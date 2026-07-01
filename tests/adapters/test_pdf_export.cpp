// AK + voll-Decode-Orakel + Integration (slice-025b): PDF-Export end-to-end über
// den ECHTEN Pfad `ExchangeService` -> `ModelExporterPort` -> `PdfExportAdapter`
// (ExchangeFormat::Pdf). AK `LH-FA-IO-007` Happy/Boundary/Negative.
//
// Voll-Decode-Orakel (Review-MED-1, ADR-0016): kein oberflächliches "nicht-leer",
// sondern (a) Byte-Konsistenz (%PDF-Header, xref-Offsets -> "N 0 obj",
// Content-`/Length` == reale Stream-Länge, trailer/startxref/%%EOF), (b)
// Objektgraph/Reader-Öffenbarkeit (Catalog->Pages->Kids, je Seite /MediaBox==A4,
// /Contents->Stream, /Resources->/Font->/Helvetica), (c) Inhalt je Geschoss-Seite
// (Linien-Operatoren == Wandzahl des Geschosses — Geschoss-Trennung, Review-LOW-3).
// Maßstabs-Sonde belegt die Maßstäblichkeit (ACC-004).

#include "adapters/io/pdf_export_adapter.h"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
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

namespace {

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
using bcad::adapters::io::PdfExportAdapter;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

// mm -> pt bei festem Maßstab 1:100 (Spiegel der Adapter-Konstante).
constexpr double kMmToPt = (72.0 / 25.4) / 100.0;

// --- Fixtures ---------------------------------------------------------------

class TempPath {
public:
    explicit TempPath(const std::string& tag)
        : path(fs::temp_directory_path() / ("bcad_pdfexp_" + tag + ".pdf")) {
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

// 2 Geschosse: storey 1 (2 Wände), storey 2 (1 Wand).
model::Building sampleBuilding() {
    model::Building b;
    b.storeys.push_back(model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    b.storeys.push_back(model::Storey{model::StoreyId{2}, model::kDefaultStoreyHeightMm});
    b.walls.push_back(makeWall(1, model::StoreyId{1}, {0.0, 0.0}, {5000.0, 0.0}));
    b.walls.push_back(makeWall(2, model::StoreyId{1}, {5000.0, 0.0}, {5000.0, 4000.0}));
    b.walls.push_back(makeWall(3, model::StoreyId{2}, {0.0, 0.0}, {3000.0, 0.0}));
    return b;
}

std::string readFile(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// --- PDF-Parse-Helfer (voll-Decode-Orakel) ---------------------------------

int countOccurrences(const std::string& hay, const std::string& needle) {
    int n = 0;
    for (std::size_t p = hay.find(needle); p != std::string::npos;
         p = hay.find(needle, p + needle.size())) {
        ++n;
    }
    return n;
}

// Body von Objekt `num` ("num 0 obj" ... "endobj"), oder "" falls fehlend.
std::string objectBody(const std::string& pdf, int num) {
    const std::string header = "\n" + std::to_string(num) + " 0 obj\n";
    const std::size_t start = pdf.find(header);
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t body = start + header.size();
    const std::size_t end = pdf.find("endobj", body);
    if (end == std::string::npos) {
        return "";
    }
    return pdf.substr(body, end - body);
}

// Content-Stream von Objekt `num` mit Prüfung `/Length` == reale Stream-Länge.
std::string streamOfObject(const std::string& pdf, int num) {
    const std::string body = objectBody(pdf, num);
    const std::size_t lp = body.find("/Length ");
    EXPECT_NE(lp, std::string::npos) << "obj " << num << " ohne /Length";
    const std::size_t declared =
        static_cast<std::size_t>(std::stoul(body.substr(lp + 8)));
    const std::size_t sp = body.find("stream\n");
    EXPECT_NE(sp, std::string::npos);
    const std::size_t content_start = sp + std::string("stream\n").size();
    const std::size_t es = body.find("\nendstream", content_start);
    EXPECT_NE(es, std::string::npos);
    const std::string content = body.substr(content_start, es - content_start);
    EXPECT_EQ(content.size(), declared)
        << "obj " << num << ": /Length != reale Stream-Länge";
    return content;
}

// (a) Byte-Konsistenz: startxref -> xref, jede 'n'-Zeile -> "<i> 0 obj".
// Gibt die Objektzahl zurück (aus der xref-Größe).
int verifyXrefOffsets(const std::string& pdf) {
    const std::size_t sx = pdf.rfind("startxref");
    EXPECT_NE(sx, std::string::npos);
    const std::size_t numpos = pdf.find('\n', sx) + 1;
    const std::size_t xref_off =
        static_cast<std::size_t>(std::stoul(pdf.substr(numpos)));
    EXPECT_EQ(pdf.compare(xref_off, 4, "xref"), 0) << "startxref zeigt nicht auf xref";

    // "xref\n0 M\n"
    const std::size_t after_kw = xref_off + std::string("xref\n").size();
    const std::size_t nl = pdf.find('\n', after_kw);
    std::istringstream head(pdf.substr(after_kw, nl - after_kw));
    int first = 0;
    int size = 0;
    head >> first >> size;
    EXPECT_EQ(first, 0);
    const std::size_t entries = nl + 1;
    const std::size_t kEntry = 20;  // Byte je xref-Eintrag
    for (int i = 1; i < size; ++i) {
        const std::string entry =
            pdf.substr(entries + (static_cast<std::size_t>(i) * kEntry), kEntry);
        EXPECT_EQ(entry.substr(17, 1), "n") << "xref-Eintrag " << i;
        const std::size_t obj_off =
            static_cast<std::size_t>(std::stoul(entry.substr(0, 10)));
        const std::string expect = std::to_string(i) + " 0 obj";
        EXPECT_EQ(pdf.compare(obj_off, expect.size(), expect), 0)
            << "xref-Offset " << i << " zeigt nicht auf Objekt-Header";
    }
    return size - 1;  // Objekt 0 ist das freie Objekt
}

// Kids-Objektnummern aus dem Seitenbaum (obj 2).
std::vector<int> pageObjects(const std::string& pdf) {
    const std::string pages = objectBody(pdf, 2);
    EXPECT_NE(pages.find("/Type /Pages"), std::string::npos);
    const std::size_t ob = pages.find('[');
    const std::size_t cb = pages.find(']', ob);
    std::istringstream ks(pages.substr(ob + 1, cb - ob - 1));
    std::vector<int> kids;
    int num = 0;
    std::string zero;
    std::string r;
    while (ks >> num >> zero >> r) {  // "N 0 R"
        kids.push_back(num);
    }
    return kids;
}

// Erstes Achs-Segment (m/l) eines Content-Streams als (x1,y1,x2,y2) in pt.
struct Seg {
    double x1{}, y1{}, x2{}, y2{};
};
Seg firstSegment(const std::string& content) {
    const std::size_t mp = content.find(" m\n");
    const std::size_t line_start = content.rfind('\n', mp) + 1;
    std::istringstream a(content.substr(line_start, mp - line_start));
    Seg s;
    a >> s.x1 >> s.y1;
    const std::size_t lp = content.find(" l\n", mp);
    std::istringstream b(content.substr(mp + 3, lp - (mp + 3)));
    b >> s.x2 >> s.y2;
    return s;
}

// --- LH-FA-IO-007 Happy: Struktur + Objektgraph + Inhalt je Geschoss --------

TEST(PdfExport, StructureObjectGraphAndPerStoreyContent) {
    const TempPath out("struct");
    const PdfExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Pdf, &exporter}});
    service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Pdf);
    ASSERT_TRUE(fs::exists(out.path));
    const std::string pdf = readFile(out.path);

    // (a) Byte-Konsistenz.
    EXPECT_EQ(pdf.compare(0, 7, "%PDF-1."), 0);
    EXPECT_NE(pdf.find("%%EOF"), std::string::npos);
    const int objects = verifyXrefOffsets(pdf);
    EXPECT_EQ(objects, 7);  // Katalog+Seitenbaum+Font + 2 Seiten * 2

    // (b) Objektgraph / Reader-Öffenbarkeit.
    EXPECT_NE(objectBody(pdf, 1).find("/Type /Catalog"), std::string::npos);
    EXPECT_NE(objectBody(pdf, 1).find("/Pages 2 0 R"), std::string::npos);
    EXPECT_NE(objectBody(pdf, 3).find("/BaseFont /Helvetica"), std::string::npos);
    const std::vector<int> kids = pageObjects(pdf);
    ASSERT_EQ(kids.size(), 2U);
    for (const int page_obj : kids) {
        const std::string page = objectBody(pdf, page_obj);
        EXPECT_NE(page.find("/Type /Page"), std::string::npos);
        EXPECT_NE(page.find("/MediaBox [0 0 595.000 842.000]"), std::string::npos);
        EXPECT_NE(page.find("/Contents "), std::string::npos);
        EXPECT_NE(page.find("/Font << /F1 3 0 R >>"), std::string::npos);
    }

    // (c) Inhalt je Geschoss-Seite: Linien == Wandzahl, Rahmen + Label je Seite.
    const std::string page0 = streamOfObject(pdf, 5);  // Geschoss 1: 2 Wände
    const std::string page1 = streamOfObject(pdf, 7);  // Geschoss 2: 1 Wand
    EXPECT_EQ(countOccurrences(page0, " l\n"), 2);
    EXPECT_EQ(countOccurrences(page1, " l\n"), 1);
    for (const std::string& page : {page0, page1}) {
        EXPECT_EQ(countOccurrences(page, " re\n"), 1);  // Rahmen
        EXPECT_EQ(countOccurrences(page, " Tj\n"), 1);  // "M 1:100"-Label
        EXPECT_NE(page.find("(M 1:100)"), std::string::npos);
    }
}

// --- LH-FA-IO-007 Maßstabs-Sonde: maßstäblich (ACC-004) ---------------------

TEST(PdfExport, ScaleFidelityKnownEdge) {
    const TempPath out("scale");
    model::Building b;
    b.storeys.push_back(model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    b.walls.push_back(makeWall(1, model::StoreyId{1}, {0.0, 0.0}, {5000.0, 0.0}));

    PdfExportAdapter().write(b, out.path);
    const std::string content = streamOfObject(readFile(out.path), 5);
    const Seg s = firstSegment(content);
    const double length = std::hypot(s.x2 - s.x1, s.y2 - s.y1);
    // 5000 mm bei 1:100 -> 5000 * (72/25.4)/100 pt.
    EXPECT_NEAR(length, 5000.0 * kMmToPt, 0.5);
}

// --- LH-FA-IO-007 Boundary: leeres Modell -> gültige (leere) PDF ------------

TEST(PdfExport, EmptyBuildingProducesValidPdf) {
    const TempPath out("empty");
    PdfExportAdapter().write(model::Building{}, out.path);
    ASSERT_TRUE(fs::exists(out.path));
    const std::string pdf = readFile(out.path);
    EXPECT_EQ(pdf.compare(0, 7, "%PDF-1."), 0);
    EXPECT_NE(pdf.find("%%EOF"), std::string::npos);
    EXPECT_EQ(verifyXrefOffsets(pdf), 5);  // eine leere Seite (3 + 2*1)
    ASSERT_EQ(pageObjects(pdf).size(), 1U);
    const std::string page = streamOfObject(pdf, 5);
    EXPECT_EQ(countOccurrences(page, " l\n"), 0);  // keine Wände
    EXPECT_EQ(countOccurrences(page, " re\n"), 1);  // aber gültiger Rahmen
}

// --- LH-FA-IO-007 Negative: nicht beschreibbarer Zielpfad -> E-IO-001 -------
// über den ECHTEN Pfad; `.tmp` ist ein nicht-leeres Verzeichnis -> Rename/open
// scheitert -> E-IO-001, kein Teil-Export.

TEST(PdfExportIntegration, NonWritablePathRejectedWithEIo001) {
    const TempPath out("reject");
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::error_code ec; (void)fs::create_directory(tmp / "block", ec); }

    const PdfExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Pdf, &exporter}});
    try {
        service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Pdf);
        FAIL() << "erwarteter E-IO-001-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
        EXPECT_NE(what.find("io_no_permission"), std::string::npos) << what;
    }
    EXPECT_FALSE(fs::exists(out.path));  // kein Teil-Export
}

// --- Export-only: Import-Request für PDF -> E-IO-003 (Lookup-Miss) ----------

TEST(PdfExportIntegration, ImportRejectedExportOnly) {
    const PdfExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Pdf, &exporter}});
    try {
        (void)service.importModel(fs::path("irrelevant.pdf"), ExchangeFormat::Pdf);
        FAIL() << "PDF-Import müsste als export-only abgewiesen werden";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("E-IO-003"), std::string::npos) << what;
        EXPECT_NE(what.find("import_rejected"), std::string::npos) << what;
    }
}

}  // namespace
