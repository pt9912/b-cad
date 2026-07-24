// AK + voll-Decode-Orakel + Integration (slice-025c): PNG-Export end-to-end über
// den ECHTEN Pfad `ExchangeService` -> `ModelExporterPort` -> `PngExportAdapter`
// (ExchangeFormat::Png). AK `LH-FA-IO-008` Happy/Boundary/Negative + export-only.
//
// Voll-Decode-Orakel (Review-MED-1/2/3, ADR-0016): ein im Test EIGENSTÄNDIG
// implementierter PNG-Decoder (KEINE Wiederverwendung der png_writer-Helfer —
// sonst prüft der Encoder sich selbst): Signatur, IHDR, CRC-32 je Chunk (Polynom
// 0xEDB88320 über Typ+Daten), zlib-2-Byte-Header, IDAT-Inflate (stored-Blöcke),
// jede Scanline Filterbyte 0, Adler-32 (mod 65521 über Roh-Scanlines), Tinte +
// Geschoss-Farben.

#include "adapters/io/png_export_adapter.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"
#include "hexagon/model/wall_type.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/services/exchange_service.h"
#include "hexagon/services/geometry/plan_projection.h"

namespace {

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
using bcad::adapters::io::PngExportAdapter;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

// slice-042b: PNG serialisiert die kern-gelieferte `PlanView` aus dem Bündel; die
// Direkt-Tests speisen sie aus DERSELBEN `projectPlan`-Quelle (byte-identisch;
// MR-006-LOW-2).
void writePng(const model::Building& b, const fs::path& path) {
    model::DerivedGeometry derived;
    derived.plan = bcad::hexagon::services::projectPlan(b);
    PngExportAdapter().write(b, derived, path);
}

constexpr int kCanvasWidthPx = 800;   // Spiegel der Adapter-Konstante
constexpr int kCanvasHeightPx = 600;

// --- Fixtures ---------------------------------------------------------------

class TempPath {
public:
    explicit TempPath(const std::string& tag)
        : path(fs::temp_directory_path() / ("bcad_pngexp_" + tag + ".png")) {
        std::error_code ec;
        fs::remove(path, ec);
    }
    ~TempPath() {
        std::error_code ec;
        fs::remove(path, ec);
        fs::remove_all(fs::path(path.string() + ".tmp"), ec);
    }
    TempPath(const TempPath&) = delete;
    TempPath& operator=(const TempPath&) = delete;
    fs::path path;
};

model::Wall makeWall(int id, model::StoreyId storey, model::Point2D start,
                     model::Point2D end) {
    model::Wall wall;
    wall.id = model::WallId{id};
    wall.storey_id = storey;
    wall.start = start;
    wall.end = end;
    wall.thickness_mm = model::kDefaultWallThicknessMm;
    wall.height_mm = model::kDefaultStoreyHeightMm;
    wall.type = model::WallType::Innen;
    return wall;
}

model::Building sampleBuilding() {
    model::Building b;
    b.storeys.push_back(model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    b.storeys.push_back(model::Storey{model::StoreyId{2}, model::kDefaultStoreyHeightMm});
    b.walls.push_back(makeWall(1, model::StoreyId{1}, {0.0, 0.0}, {5000.0, 0.0}));
    b.walls.push_back(makeWall(2, model::StoreyId{1}, {5000.0, 0.0}, {5000.0, 4000.0}));
    b.walls.push_back(makeWall(3, model::StoreyId{2}, {0.0, 0.0}, {3000.0, 4000.0}));
    return b;
}

// slice-032c (LH-FA-DRW-005/006, ADR-0018): ein Geschoss mit einer Wand-BOX
// (2D-Ausdehnung → nicht-degenerierte BBox) + 1 Ebene + 1 Hilfslinie INNERHALB
// der Box. Weil die Hilfslinie in der Wand-BBox liegt, ist die BBox (und damit
// der Maßstab) für sichtbar/unsichtbar IDENTISCH — der einzige Unterschied ist
// die zusätzliche Hilfslinien-Tinte (sauberer Erscheint/Fehlt-Vergleich).
model::Building drwBuilding(bool layer_visible) {
    model::Building b;
    b.storeys.push_back(model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    b.walls.push_back(makeWall(1, model::StoreyId{1}, {0.0, 0.0}, {5000.0, 0.0}));
    b.walls.push_back(makeWall(2, model::StoreyId{1}, {5000.0, 0.0}, {5000.0, 4000.0}));
    b.walls.push_back(makeWall(3, model::StoreyId{1}, {5000.0, 4000.0}, {0.0, 4000.0}));
    b.walls.push_back(makeWall(4, model::StoreyId{1}, {0.0, 4000.0}, {0.0, 0.0}));
    model::Layer layer;
    layer.id = model::LayerId{1};
    layer.name = "Achsen";
    layer.visible = layer_visible;
    b.layers.push_back(layer);
    model::GuideLine guide;
    guide.id = model::GuideLineId{1};
    guide.storey_id = model::StoreyId{1};
    guide.layer_id = model::LayerId{1};
    guide.segment = {{1000.0, 1000.0}, {4000.0, 3000.0}};  // innerhalb der Box
    b.guide_lines.push_back(guide);
    return b;
}

std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// --- EIGENSTÄNDIGE Prüfsummen (Review-MED-1: NICHT png_writer aufrufen) -----

std::uint32_t oracleCrc32(const std::string& data) {
    std::uint32_t crc = 0xFFFFFFFFU;
    for (const char ch : data) {
        crc ^= static_cast<unsigned char>(ch);
        for (int k = 0; k < 8; ++k) {
            crc = (crc >> 1U) ^ ((crc & 1U) != 0U ? 0xEDB88320U : 0U);
        }
    }
    return crc ^ 0xFFFFFFFFU;
}

std::uint32_t oracleAdler32(const std::string& data) {
    std::uint32_t a = 1U;
    std::uint32_t b = 0U;
    for (const char ch : data) {
        a = (a + static_cast<unsigned char>(ch)) % 65521U;
        b = (b + a) % 65521U;
    }
    return (b << 16U) | a;
}

std::uint32_t u32be(const std::string& s, std::size_t pos) {
    return (static_cast<std::uint32_t>(static_cast<unsigned char>(s[pos])) << 24U) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(s[pos + 1])) << 16U) |
           (static_cast<std::uint32_t>(static_cast<unsigned char>(s[pos + 2])) << 8U) |
           static_cast<std::uint32_t>(static_cast<unsigned char>(s[pos + 3]));
}

std::uint16_t u16le(const std::string& s, std::size_t pos) {
    return static_cast<std::uint16_t>(
        static_cast<unsigned char>(s[pos]) |
        (static_cast<unsigned char>(s[pos + 1]) << 8U));
}

// --- PNG-Chunk-Parser (verifiziert Signatur + CRC-32 je Chunk) --------------

struct Chunk {
    std::string type;
    std::string data;
};

std::vector<Chunk> parseChunks(const std::string& png) {
    const std::array<unsigned char, 8> sig = {137, 80, 78, 71, 13, 10, 26, 10};
    for (std::size_t i = 0; i < sig.size(); ++i) {
        EXPECT_EQ(static_cast<unsigned char>(png[i]), sig[i]) << "Signatur-Byte " << i;
    }
    std::vector<Chunk> chunks;
    std::size_t pos = 8;
    while (pos + 12 <= png.size()) {
        const std::uint32_t len = u32be(png, pos);
        const std::string type = png.substr(pos + 4, 4);
        const std::string data = png.substr(pos + 8, len);
        const std::uint32_t crc = u32be(png, pos + 8 + len);
        EXPECT_EQ(crc, oracleCrc32(type + data)) << "CRC-32 Chunk " << type;
        chunks.push_back({type, data});
        pos += 12 + len;
        if (type == "IEND") {
            break;
        }
    }
    return chunks;
}

// zlib-Strom (stored-DEFLATE) inflaten + Header/Adler prüfen (Review-MED-3).
std::string inflateStored(const std::string& zlib) {
    EXPECT_GE(zlib.size(), 6U);
    if (zlib.size() < 6U) {
        return {};
    }
    const auto cmf = static_cast<unsigned char>(zlib[0]);
    const auto flg = static_cast<unsigned char>(zlib[1]);
    EXPECT_EQ(cmf & 0x0FU, 8U) << "zlib CM != 8";                       // Deflate
    EXPECT_EQ(((cmf << 8U) | flg) % 31U, 0U) << "zlib FCHECK ungültig";  // MED-3
    EXPECT_EQ(flg & 0x20U, 0U) << "zlib FDICT gesetzt";
    std::string raw;
    std::size_t pos = 2;
    while (true) {
        const auto header = static_cast<unsigned char>(zlib[pos]);
        EXPECT_EQ((header >> 1U) & 0x03U, 0U) << "BTYPE != stored";
        const bool is_final = (header & 0x01U) != 0U;
        const std::uint16_t len = u16le(zlib, pos + 1);
        const std::uint16_t nlen = u16le(zlib, pos + 3);
        EXPECT_EQ(nlen, static_cast<std::uint16_t>(~len)) << "NLEN != ~LEN";
        raw += zlib.substr(pos + 5, len);
        pos += 5U + len;
        if (is_final) {
            break;
        }
    }
    const std::uint32_t adler = u32be(zlib, pos);
    EXPECT_EQ(adler, oracleAdler32(raw)) << "Adler-32 stimmt nicht";
    return raw;
}

// --- LH-FA-IO-008 Happy: voll-Decode + IHDR + Scanline-Filter + Adler --------

TEST(PngExport, FullDecodeStructureAndInk) {
    const TempPath out("full");
    const PngExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Png, &exporter}});
    service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Png);
    ASSERT_TRUE(fs::exists(out.path));
    const std::string png = readFile(out.path);

    const std::vector<Chunk> chunks = parseChunks(png);  // Signatur + CRC je Chunk
    ASSERT_GE(chunks.size(), 3U);
    EXPECT_EQ(chunks.front().type, "IHDR");
    EXPECT_EQ(chunks.back().type, "IEND");

    // IHDR: Dimensionen + 8-bit RGB, keine Interlace/Filter.
    const std::string& ihdr = chunks.front().data;
    ASSERT_EQ(ihdr.size(), 13U);
    EXPECT_EQ(u32be(ihdr, 0), static_cast<std::uint32_t>(kCanvasWidthPx));
    EXPECT_EQ(u32be(ihdr, 4), static_cast<std::uint32_t>(kCanvasHeightPx));
    EXPECT_EQ(static_cast<unsigned char>(ihdr[8]), 8U);   // bit depth
    EXPECT_EQ(static_cast<unsigned char>(ihdr[9]), 2U);   // color type RGB
    EXPECT_EQ(static_cast<unsigned char>(ihdr[12]), 0U);  // interlace

    // IDAT konkatenieren -> inflaten -> Roh-Scanlines.
    std::string idat;
    for (const Chunk& c : chunks) {
        if (c.type == "IDAT") {
            idat += c.data;
        }
    }
    const std::string raw = inflateStored(idat);
    const std::size_t stride = 1U + (static_cast<std::size_t>(kCanvasWidthPx) * 3U);
    ASSERT_EQ(raw.size(), static_cast<std::size_t>(kCanvasHeightPx) * stride);

    // Jede Scanline beginnt mit Filterbyte 0 (None).
    for (int y = 0; y < kCanvasHeightPx; ++y) {
        EXPECT_EQ(static_cast<unsigned char>(raw[static_cast<std::size_t>(y) * stride]), 0U)
            << "Scanline " << y << " Filterbyte";
    }

    // Tinte vorhanden + Geschoss-Farben unterscheidbar (Review-LOW-1: >=2 Farben).
    std::set<std::array<unsigned char, 3>> colors;
    for (int y = 0; y < kCanvasHeightPx; ++y) {
        const std::size_t rowbase = (static_cast<std::size_t>(y) * stride) + 1U;
        for (int x = 0; x < kCanvasWidthPx; ++x) {
            const std::size_t i = rowbase + (static_cast<std::size_t>(x) * 3U);
            const std::array<unsigned char, 3> rgb = {
                static_cast<unsigned char>(raw[i]),
                static_cast<unsigned char>(raw[i + 1]),
                static_cast<unsigned char>(raw[i + 2])};
            if (rgb != std::array<unsigned char, 3>{255, 255, 255}) {
                colors.insert(rgb);
            }
        }
    }
    EXPECT_FALSE(colors.empty()) << "keine Tinte";
    EXPECT_GE(colors.size(), 2U) << "Geschoss-Farben nicht unterscheidbar";
}

// Zahl der Nicht-weiß-Pixel (Tinte) eines dekodierten PNG (nutzt die
// parseChunks/inflateStored-Decoder-Helfer oben).
int inkPixels(const std::string& png) {
    std::string idat;
    for (const Chunk& chunk : parseChunks(png)) {
        if (chunk.type == "IDAT") {
            idat += chunk.data;
        }
    }
    const std::string raw = inflateStored(idat);
    const std::size_t stride = 1U + (static_cast<std::size_t>(kCanvasWidthPx) * 3U);
    int ink = 0;
    for (int y = 0; y < kCanvasHeightPx; ++y) {
        const std::size_t rowbase = (static_cast<std::size_t>(y) * stride) + 1U;
        for (int x = 0; x < kCanvasWidthPx; ++x) {
            const std::size_t i = rowbase + (static_cast<std::size_t>(x) * 3U);
            if (!(static_cast<unsigned char>(raw[i]) == 255U &&
                  static_cast<unsigned char>(raw[i + 1]) == 255U &&
                  static_cast<unsigned char>(raw[i + 2]) == 255U)) {
                ++ink;
            }
        }
    }
    return ink;
}

// --- LH-FA-DRW-005 (ADR-0018): Hilfslinie im PNG-Grundriss, Ebenen-Filter -----
// Gleiche Wand-BBox in beiden Fällen (Hilfslinie innerhalb) → identischer
// Maßstab; der Unterschied ist allein die Hilfslinien-Tinte.

// DRW-005 Happy (Export) + DRW-005-Negative (unsichtbar → fehlt): eine sichtbare
// Hilfslinie fügt Tinte hinzu; eine unsichtbare Ebene nicht.
TEST(PngExport, LH_FA_DRW_005_SichtbareHilfslinieMehrTinte) {
    const TempPath vis("drw_vis");
    const TempPath inv("drw_inv");
    writePng(drwBuilding(/*layer_visible=*/true), vis.path);
    writePng(drwBuilding(/*layer_visible=*/false), inv.path);
    const int ink_visible = inkPixels(readFile(vis.path));
    const int ink_hidden = inkPixels(readFile(inv.path));
    EXPECT_GT(ink_visible, ink_hidden) << "sichtbare Hilfslinie fügt keine Tinte hinzu";
}

// slice-046b (LH-FA-IO-008): sichtbarer Provenance-Titelblock (5×7-Bitmap-Font) +
// injizierte tEXt. Ink-Sonde (dasselbe Modell mit-vs-leer, Fußzeile im weißen
// Unterrand → streng), Unterscheidbarkeit, tEXt-Präsenz, kein tIME, Langer-String
// out-of-bounds-sicher. Das Voll-Decode-Orakel oben bleibt unberührt.
TEST(PngExport, VisibleProvenanceFooterAndText) {
    const model::ExportProvenance prov{"1970-01-01 00:00", "golden.bcad", "b-cad test"};
    model::DerivedGeometry derived;
    derived.plan = bcad::hexagon::services::projectPlan(sampleBuilding());

    // (a) Ink-Sonde: Herkunft-Fußzeile fügt Tinte hinzu.
    const TempPath with_prov("prov_with");
    const TempPath without_prov("prov_without");
    PngExportAdapter().write(sampleBuilding(), derived, with_prov.path, prov);
    PngExportAdapter().write(sampleBuilding(), derived, without_prov.path);  // leer
    EXPECT_GT(inkPixels(readFile(with_prov.path)), inkPixels(readFile(without_prov.path)))
        << "sichtbare Provenance-Fußzeile fügt keine Tinte hinzu";

    // (b) tEXt Date/Source/Version präsent, kein tIME.
    const std::string png = readFile(with_prov.path);
    bool date = false;
    bool source = false;
    bool version = false;
    for (const Chunk& c : parseChunks(png)) {
        if (c.type == "tEXt") {
            if (c.data == std::string("Date").append(1, '\0').append("1970-01-01 00:00")) {
                date = true;
            }
            if (c.data == std::string("Source").append(1, '\0').append("golden.bcad")) {
                source = true;
            }
            if (c.data == std::string("Version").append(1, '\0').append("b-cad test")) {
                version = true;
            }
        }
        EXPECT_NE(c.type, "tIME") << "dynamischer tIME-Chunk verboten";
    }
    EXPECT_TRUE(date) << "tEXt Date fehlt";
    EXPECT_TRUE(source) << "tEXt Source fehlt";
    EXPECT_TRUE(version) << "tEXt Version fehlt";

    // (c) Unterscheidbarkeit: verschiedene Herkunft → verschiedene Bytes.
    const TempPath other("prov_other");
    const model::ExportProvenance prov2{"2026-07-24 14:32", "haus.bcad", "b-cad test"};
    PngExportAdapter().write(sampleBuilding(), derived, other.path, prov2);
    EXPECT_NE(readFile(with_prov.path), readFile(other.path))
        << "verschiedene Herkunft muss unterscheidbares PNG ergeben";

    // (d) langer String → kein Wurf/OOB (drawText geklemmt, by construction).
    const TempPath long_path("prov_long");
    const model::ExportProvenance long_prov{std::string(500, 'X'), std::string{}, std::string{}};
    EXPECT_NO_THROW(
        PngExportAdapter().write(sampleBuilding(), derived, long_path.path, long_prov));
}

// --- LH-FA-IO-008 Boundary (leer): weißes, valides PNG ----------------------

// slice-045 (LH-FA-IO-008): statische tEXt-Metadaten (Software + Title) —
// benutzer-sichtbar, konsistent mit STEP/IFC/PDF ('b-cad'). BEWUSST kein
// tIME-Chunk (dynamisch) → byte-deterministisch (Byte-Golden tragfähig). Der
// Titel trägt HIGH-1, daher direkt asserted; das Voll-Decode-Orakel oben bleibt
// unberührt (parseChunks ist positions-agnostisch, front==IHDR/back==IEND).
TEST(PngExport, StaticTextMetadataNoTime) {
    const TempPath out("text");
    writePng(sampleBuilding(), out.path);
    const std::string png = readFile(out.path);
    const std::vector<Chunk> chunks = parseChunks(png);  // CRC-prüft jeden Chunk

    bool software = false;
    bool title = false;
    bool text_before_idat = true;
    bool seen_idat = false;
    for (const Chunk& c : chunks) {
        if (c.type == "IDAT") {
            seen_idat = true;
        }
        if (c.type == "tEXt") {
            if (seen_idat) {
                text_before_idat = false;  // tEXt nach IDAT wäre falsch platziert
            }
            if (c.data == std::string("Software").append(1, '\0').append("b-cad")) {
                software = true;
            }
            if (c.data ==
                std::string("Title").append(1, '\0').append("b-cad Plan-Export")) {
                title = true;
            }
        }
        // Negative-Determinismus-Sonde: kein dynamischer tIME-Chunk.
        EXPECT_NE(c.type, "tIME") << "dynamischer tIME-Chunk verboten (Determinismus)";
    }
    EXPECT_TRUE(software) << "tEXt Software=b-cad fehlt";
    EXPECT_TRUE(title) << "tEXt Title=b-cad Plan-Export fehlt";
    EXPECT_TRUE(text_before_idat) << "tEXt muss vor IDAT stehen";

    // Zwei Läufe byte-identisch (Determinismus der statischen tEXt).
    const TempPath out2("text2");
    writePng(sampleBuilding(), out2.path);
    EXPECT_EQ(readFile(out2.path), png);
}

TEST(PngExport, EmptyBuildingProducesValidWhitePng) {
    const TempPath out("empty");
    writePng(model::Building{}, out.path);
    const std::string png = readFile(out.path);
    const std::vector<Chunk> chunks = parseChunks(png);
    ASSERT_GE(chunks.size(), 3U);
    std::string idat;
    for (const Chunk& c : chunks) {
        if (c.type == "IDAT") {
            idat += c.data;
        }
    }
    const std::string raw = inflateStored(idat);  // Adler/Filter via inflateStored
    const std::size_t stride = 1U + (static_cast<std::size_t>(kCanvasWidthPx) * 3U);
    EXPECT_EQ(raw.size(), static_cast<std::size_t>(kCanvasHeightPx) * stride);
    // keine Tinte: alle Pixel weiß.
    bool all_white = true;
    for (int y = 0; y < kCanvasHeightPx && all_white; ++y) {
        const std::size_t rowbase = (static_cast<std::size_t>(y) * stride) + 1U;
        for (std::size_t k = 0; k < static_cast<std::size_t>(kCanvasWidthPx) * 3U; ++k) {
            if (static_cast<unsigned char>(raw[rowbase + k]) != 255U) {
                all_white = false;
                break;
            }
        }
    }
    EXPECT_TRUE(all_white);
}

// --- LH-FA-IO-008 Boundary (degenerierte BBox, Review-MED-2) ----------------
// Geometrie vorhanden, aber Breite 0 (rein vertikale Wand) -> Fit-to-Canvas-Guard
// greift, kein Div-durch-0/Absturz, valides PNG.

TEST(PngExport, DegenerateBBoxNoDivByZero) {
    const TempPath out("degen");
    model::Building b;
    b.storeys.push_back(model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    b.walls.push_back(makeWall(1, model::StoreyId{1}, {1000.0, 0.0}, {1000.0, 5000.0}));

    writePng(b, out.path);
    const std::string png = readFile(out.path);
    const std::vector<Chunk> chunks = parseChunks(png);
    EXPECT_EQ(chunks.front().type, "IHDR");
    std::string idat;
    for (const Chunk& c : chunks) {
        if (c.type == "IDAT") {
            idat += c.data;
        }
    }
    const std::string raw = inflateStored(idat);  // valider Strom trotz Breite 0
    const std::size_t stride = 1U + (static_cast<std::size_t>(kCanvasWidthPx) * 3U);
    EXPECT_EQ(raw.size(), static_cast<std::size_t>(kCanvasHeightPx) * stride);
}

// --- LH-FA-IO-008 Negative: nicht beschreibbarer Zielpfad -> E-IO-001 -------

TEST(PngExportIntegration, NonWritablePathRejectedWithEIo001) {
    const TempPath out("reject");
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::error_code ec; (void)fs::create_directory(tmp / "block", ec); }

    const PngExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Png, &exporter}});
    try {
        service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Png);
        FAIL() << "erwarteter E-IO-001-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
        EXPECT_NE(what.find("io_no_permission"), std::string::npos) << what;
    }
    EXPECT_FALSE(fs::exists(out.path));  // kein Teil-Export
}

// --- Export-only: Import-Request für PNG -> E-IO-003 ------------------------

TEST(PngExportIntegration, ImportRejectedExportOnly) {
    const PngExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Png, &exporter}});
    try {
        (void)service.importModel(fs::path("irrelevant.png"), ExchangeFormat::Png);
        FAIL() << "PNG-Import müsste als export-only abgewiesen werden";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("E-IO-003"), std::string::npos) << what;
        EXPECT_NE(what.find("import_rejected"), std::string::npos) << what;
    }
}

}  // namespace
