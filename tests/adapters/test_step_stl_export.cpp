// AK + Integration (slice-020b): STL-Export end-to-end über den ECHTEN Pfad
// `ExchangeService.exportModel` -> `ModelExporterPort` -> `StlExportAdapter`
// (geometrie-resident, ADR-0014). Re-Read-Orakel: die binäre STL-Datei wird
// geparst (80-Byte-Header + uint32-Dreieckszahl + 50 Byte/Dreieck) — ein Modell
// mit Wänden trägt > 0 Dreiecke; leeres Modell -> gültige leere STL. Negative:
// nicht beschreibbarer Zielpfad -> E-IO-001 (kein Teil-Export). STEP (B-Rep)
// folgt im zweiten Commit dieses Slices.

#include "adapters/geometry/stl_export_adapter.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/geometry/step_export_adapter.h"
#include "adapters/io/ifc_import_adapter.h"
#include "hexagon/model/building.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/services/exchange_service.h"

namespace {

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
using bcad::adapters::geometry::OccGeometryAdapter;
using bcad::adapters::geometry::StepExportAdapter;
using bcad::adapters::geometry::StlExportAdapter;
using bcad::adapters::io::IfcImportAdapter;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

// 1 Geschoss, 2 gerade Wände — tessellieren zu Prismen-Netzen (> 0 Dreiecke).
model::Building sampleBuilding() {
    model::Building b;
    b.storeys.push_back({model::StoreyId{1}, 2500.0});
    b.walls.push_back({model::WallId{1}, model::StoreyId{1}, {0.0, 0.0},
                       {5000.0, 0.0}, 240.0, 2500.0, model::WallType::Aussen});
    b.walls.push_back({model::WallId{2}, model::StoreyId{1}, {0.0, 0.0},
                       {0.0, 4000.0}, 300.0, 2500.0, model::WallType::Innen});
    return b;
}

// sampleBuilding + ein Dach gegebenen Typs/Grundrisses (seit 023b ein
// wasserdichter Volumenkörper der Dicke 200 mm) — slice-024a: das Dach wird zu
// einem B-Rep-Solid vernäht. Parametriert, damit der STEP-Test alle Typen +
// den Walm-Zeltdach-Apex deckt (MR-009 LOW-2, nicht nur Sattel).
model::Building buildingWithRoof(model::RoofType type, double width_mm,
                                 double depth_mm) {
    model::Building b = sampleBuilding();
    model::Roof roof;
    roof.id = model::RoofId{1};
    roof.storey_id = model::StoreyId{1};
    roof.type = type;
    roof.origin = {0.0, 0.0};
    roof.width_mm = width_mm;
    roof.depth_mm = depth_mm;
    roof.base_z_mm = 2500.0;
    roof.pitch_deg = 30.0;
    roof.overhang_mm = 500.0;
    roof.thickness_mm = 200.0;
    b.roofs.push_back(roof);
    return b;
}

// sampleBuilding + eine gerade Treppe mit `step_count` Stufen — slice-024b: die
// Stufen werden als analytische OCC-Box-Solids exportiert (Geländer ausgelassen).
model::Building buildingWithStair(int step_count) {
    model::Building b = sampleBuilding();
    model::Stair stair;
    stair.id = model::StairId{1};
    stair.from_storey_id = model::StoreyId{1};
    stair.to_storey_id = model::StoreyId{2};
    stair.type = model::StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = step_count;
    stair.tread_mm = 280.0;
    b.stairs.push_back(stair);
    return b;
}

// Temp-Zielpfad mit RAII-Aufräumen (Datei + .tmp-Artefakt).
struct TempPath {
    fs::path path;
    explicit TempPath(const std::string& tag, const std::string& ext = ".stl")
        : path(fs::temp_directory_path() / ("bcad_stl_" + tag + ext)) {
        clean();
    }
    ~TempPath() { clean(); }
    TempPath(const TempPath&) = delete;
    TempPath& operator=(const TempPath&) = delete;

    void clean() const {
        std::error_code ec;
        fs::remove(path, ec);
        fs::remove_all(fs::path(path.string() + ".tmp"), ec);
    }
};

// Binäres STL parsen: gültig, Dreieckszahl (uint32 @ Offset 80, LE), Dateigröße.
struct StlInfo {
    bool ok = false;
    std::uint32_t triangles = 0;
    std::uintmax_t size = 0;
};

StlInfo readStl(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    const std::vector<char> bytes((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());
    if (bytes.size() < 84) {  // 80-Byte-Header + uint32-Zähler
        return {false, 0, bytes.size()};
    }
    std::uint32_t count = 0;
    std::memcpy(&count, bytes.data() + 80, sizeof(count));  // LE (amd64)
    return {true, count, bytes.size()};
}

// slice-042c (ADR-0020, MR-006-042c-MED-1): STEP/STL serialisieren das
// KERN-berechnete `DerivedGeometry`-Bündel. Die Tests fahren über den **echten**
// `ExchangeService` (nicht adapter-direkt mit leerem Bündel), damit die starken
// Orakel (CLOSED_SHELL-Zählung, STL-Größe/-Z-Ausdehnung) die format-selektive
// Service-Berechnung **mit-verifizieren** — sonst prüften sie nur die Serialisierung
// eines vorgefertigten Bündels.
void writeStl(const model::Building& b, const fs::path& path) {
    const OccGeometryAdapter geometry;
    const StlExportAdapter stl(geometry);
    const ExchangeService service({}, {{ExchangeFormat::Stl, &stl}});
    service.exportModel(b, path, ExchangeFormat::Stl);
}
void writeStep(const model::Building& b, const fs::path& path) {
    const StepExportAdapter step;
    const ExchangeService service({}, {{ExchangeFormat::Step, &step}});
    service.exportModel(b, path, ExchangeFormat::Step);
}

// sampleBuilding + eine rechteckige Decke (Aufstandshöhe baseZ = Geschoss-Oberkante).
model::Building buildingWithSlab() {
    model::Building b = sampleBuilding();
    model::Slab slab;
    slab.id = model::SlabId{1};
    slab.storey_id = model::StoreyId{1};
    slab.type = model::SlabType::Decke;
    slab.footprint.points = {{0.0, 0.0}, {5000.0, 0.0}, {5000.0, 4000.0}, {0.0, 4000.0}};
    slab.thickness_mm = 200.0;
    b.slabs.push_back(slab);
    return b;
}

// Voll-Modell: Wände + Decke + Dach + Treppe (MR-006-042c-MED-1: prüft, dass der
// Service JEDES Bauteil mit den richtigen Parametern ableitet).
model::Building buildingFull(int step_count) {
    model::Building b = buildingWithRoof(model::RoofType::Sattel, 5000.0, 4000.0);
    model::Slab slab;
    slab.id = model::SlabId{1};
    slab.storey_id = model::StoreyId{1};
    slab.type = model::SlabType::Decke;
    slab.footprint.points = {{0.0, 0.0}, {5000.0, 0.0}, {5000.0, 4000.0}, {0.0, 4000.0}};
    slab.thickness_mm = 200.0;
    b.slabs.push_back(slab);
    model::Stair stair;
    stair.id = model::StairId{1};
    stair.from_storey_id = model::StoreyId{1};
    stair.to_storey_id = model::StoreyId{2};
    stair.type = model::StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = step_count;
    stair.tread_mm = 280.0;
    b.stairs.push_back(stair);
    return b;
}

// Max-Z aller STL-Dreiecks-Vertices (MR-006-042c-LOW-1: Koordinaten-/Extent-Sonde
// gegen die positions-blinden Zähl-Orakel — fängt einen verlorenen baseZ-Lift oder
// ein vertauschtes Storey-Feld, die 042c in die Service-Rechnung verlagert).
double stlMaxZ(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    const std::vector<char> bytes((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());
    double maxz = 0.0;
    if (bytes.size() < 84) {
        return maxz;
    }
    std::uint32_t count = 0;
    std::memcpy(&count, bytes.data() + 80, sizeof(count));
    for (std::uint32_t t = 0; t < count; ++t) {
        const std::size_t base = 84 + (50ULL * t);
        if (base + 50 > bytes.size()) {
            break;
        }
        // 12 Byte Normale, dann 3 Vertices je 3 float32; z = 3./6./9. float.
        for (int v = 0; v < 3; ++v) {
            float z = 0.0F;
            std::memcpy(&z, bytes.data() + base + 12 + (12 * v) + 8, sizeof(z));
            maxz = std::max(maxz, static_cast<double>(z));
        }
    }
    return maxz;
}

// --- Happy (LH-FA-IO-006): valide STL mit Dreiecksnetz der Bauteile ----------

TEST(StlExport, BuildingWithWallsYieldsNonEmptyMesh) {
    const TempPath out("walls");
    writeStl(sampleBuilding(), out.path);

    const StlInfo info = readStl(out.path);
    ASSERT_TRUE(info.ok);
    EXPECT_GT(info.triangles, 0U);
    // Binäres STL ist exakt: 84 Byte Kopf + 50 Byte je Dreieck.
    EXPECT_EQ(info.size, 84U + 50ULL * info.triangles);
}

// --- Boundary: 3D-leeres Modell -> gültige leere STL (Totalität) -------------

TEST(StlExport, EmptyBuildingYieldsValidEmptyStl) {
    const TempPath out("empty");
    writeStl(model::Building{}, out.path);

    const StlInfo info = readStl(out.path);
    ASSERT_TRUE(info.ok);
    EXPECT_EQ(info.triangles, 0U);
    EXPECT_EQ(info.size, 84U);  // nur Header + Zähler, keine Dreiecke
}

// --- Negative (LH-FA-IO-006): nicht beschreibbarer Zielpfad -> E-IO-001 ------

TEST(StlExport, NonWritablePathRejectedWithEIo001) {
    const TempPath out("eio001");
    // Temp-Pfad mit nicht-leerem Verzeichnis besetzen -> open scheitert (EISDIR,
    // root-fest; Muster IFC-/Persistenz-E-IO-001-Test).
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::ofstream(tmp / "block.txt") << "x"; }

    std::string what;
    try {
        writeStl(sampleBuilding(), out.path);
        FAIL() << "erwarteter E-IO-001-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
    EXPECT_NE(what.find("io_no_permission"), std::string::npos) << what;
    EXPECT_FALSE(fs::exists(out.path));  // kein Teil-Export, Zielpfad unberührt
}

// --- Integration über den ECHTEN Pfad (ExchangeService -> Port -> Adapter) ---

TEST(StlExportIntegration, ExchangeServiceWritesStlThroughRealAdapter) {
    const OccGeometryAdapter geometry;
    const StlExportAdapter stl(geometry);
    const IfcImportAdapter importer;  // export-only-Format, Importer ungenutzt
    const ExchangeService service({{ExchangeFormat::Ifc, &importer}}, {{ExchangeFormat::Stl, &stl}});

    const TempPath out("svc");
    service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Stl);
    const StlInfo info = readStl(out.path);
    ASSERT_TRUE(info.ok);
    EXPECT_GT(info.triangles, 0U);
}

TEST(StlExportIntegration, ImportOfStlRejectedAsExportOnly) {
    const OccGeometryAdapter geometry;
    const StlExportAdapter stl(geometry);
    const IfcImportAdapter importer;
    const ExchangeService service({{ExchangeFormat::Ifc, &importer}}, {{ExchangeFormat::Stl, &stl}});
    try {
        service.importModel("nicht-relevant.stl", ExchangeFormat::Stl);
        FAIL() << "STL-Import muss als export-only abgelehnt werden";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("E-IO-003"), std::string::npos) << what;
        EXPECT_NE(what.find("import_rejected"), std::string::npos) << what;
    }
}

// ====================== STEP (B-Rep, Wände + Decken) =========================
// Re-Read-Orakel ohne OCC im Test (a-check Regel C): die STEP-Datei wird als
// ISO-10303-21-Text geprüft (gültige Hülle + B-Rep-Solid-Entitäten).

std::string readText(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}

// Vorkommen einer STEP-Entität zählen (topologisch, kein bloßes „vorhanden").
std::size_t countSubstr(const std::string& hay, const std::string& needle) {
    std::size_t n = 0;
    for (std::size_t pos = hay.find(needle); pos != std::string::npos;
         pos = hay.find(needle, pos + needle.size())) {
        ++n;
    }
    return n;
}

TEST(StepExport, BuildingWithWallsYieldsBRepSolids) {
    const TempPath out("walls", ".step");
    writeStep(sampleBuilding(), out.path);

    const std::string step = readText(out.path);
    // Gültige ISO-10303-21-Hülle.
    EXPECT_NE(step.find("ISO-10303-21"), std::string::npos);
    EXPECT_NE(step.find("END-ISO-10303-21"), std::string::npos);
    // Die exportierten Bauteile sind als B-Rep-Volumenkörper enthalten.
    EXPECT_NE(step.find("BREP"), std::string::npos) << "kein B-Rep-Solid im STEP";
}

// slice-024a (LH-FA-IO-005, LH-FA-ROF-006): das Dach ist ein **geschlossenes**
// B-Rep-Solid (Mesh→Shape-Vernähung des wasserdichten 023b-Netzes). Sensor OHNE
// OCC im Test (Regel C): die topologische Wasserdichtheit zeigt sich als genau
// **eine zusätzliche** CLOSED_SHELL ggü. dem Wand-only-Modell — kein bloßes
// „nicht-leer". Wäre die Vernähung offen, würde `meshToSolid` sie fail-closed
// (BRepCheck) überspringen → kein Zuwachs → Test schlägt fehl. Über alle Typen
// + den Walm-Zeltdach-Apex (MR-009 LOW-2); zugleich Kein-Zuwachs-Regression.
TEST(StepExport, RoofYieldsClosedShellBRepSolid) {
    const TempPath wallsOut("walls_baseline", ".step");
    writeStep(sampleBuilding(), wallsOut.path);
    const std::size_t wallShells =
        countSubstr(readText(wallsOut.path), "CLOSED_SHELL");
    ASSERT_GT(wallShells, 0U) << "Basis-Erwartung: Wände sind geschlossene Solids";

    struct RoofCase {
        const char* tag;
        model::RoofType type;
        double width_mm;
        double depth_mm;
    };
    const RoofCase cases[] = {
        {"sattel", model::RoofType::Sattel, 5000.0, 4000.0},
        {"walm", model::RoofType::Walm, 5000.0, 4000.0},
        {"pult", model::RoofType::Pult, 5000.0, 4000.0},
        {"walm_zeltdach", model::RoofType::Walm, 5000.0, 5000.0},  // First=Punkt (Apex)
    };
    for (const RoofCase& c : cases) {
        const TempPath roofOut(std::string("roof_") + c.tag, ".step");
        writeStep(buildingWithRoof(c.type, c.width_mm, c.depth_mm), roofOut.path);
        const std::string step = readText(roofOut.path);

        EXPECT_NE(step.find("MANIFOLD_SOLID_BREP"), std::string::npos)
            << c.tag << ": Dach nicht als Manifold-Solid-B-Rep geschrieben";
        EXPECT_EQ(countSubstr(step, "CLOSED_SHELL"), wallShells + 1)
            << c.tag << ": Dach trägt nicht genau ein geschlossenes Solid bei";
    }
}

// slice-024b (LH-FA-IO-005, LH-FA-STR-001): die Treppen-**Stufen** sind je ein
// geschlossenes B-Rep-Box-Solid (analytisch, nicht aus dem nicht-manifolden
// stairMesh vernäht). Sensor OHNE OCC (Regel C): **genau `step_count`** zusätzliche
// CLOSED_SHELL ggü. dem Wand-only-Modell — die exakte Anzahl belegt zugleich, dass
// das **Geländer ausgelassen** ist (DoD-3; mit Geländer wären es 3·step_count).
TEST(StepExport, StairStepsYieldClosedShellBRepSolids) {
    const TempPath wallsOut("walls_baseline_stair", ".step");
    writeStep(sampleBuilding(), wallsOut.path);
    const std::size_t wallShells =
        countSubstr(readText(wallsOut.path), "CLOSED_SHELL");
    ASSERT_GT(wallShells, 0U) << "Basis-Erwartung: Wände sind geschlossene Solids";

    const int steps = 12;
    const TempPath stairOut("with_stair", ".step");
    writeStep(buildingWithStair(steps), stairOut.path);
    const std::string step = readText(stairOut.path);

    EXPECT_NE(step.find("MANIFOLD_SOLID_BREP"), std::string::npos)
        << "Treppe nicht als Manifold-Solid-B-Rep geschrieben";
    EXPECT_EQ(countSubstr(step, "CLOSED_SHELL"),
              wallShells + static_cast<std::size_t>(steps))
        << "Treppe trägt nicht genau step_count Stufen-Solids bei "
           "(Geländer müsste ausgelassen sein)";
}

TEST(StepExport, EmptyBuildingYieldsValidStepEnvelope) {
    const TempPath out("empty", ".step");
    writeStep(model::Building{}, out.path);  // 3D-leer -> kein Wurf

    const std::string step = readText(out.path);
    EXPECT_NE(step.find("ISO-10303-21"), std::string::npos);
    EXPECT_NE(step.find("END-ISO-10303-21"), std::string::npos);
}

TEST(StepExport, NonWritablePathRejectedWithEIo001) {
    const TempPath out("eio001", ".step");
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::ofstream(tmp / "block.txt") << "x"; }

    std::string what;
    try {
        writeStep(sampleBuilding(), out.path);
        FAIL() << "erwarteter E-IO-001-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
    EXPECT_NE(what.find("io_no_permission"), std::string::npos) << what;
    EXPECT_FALSE(fs::exists(out.path));  // kein Teil-Export
}

TEST(StepExportIntegration, ExchangeServiceWritesStepThroughRealAdapter) {
    const StepExportAdapter step;
    const IfcImportAdapter importer;
    const ExchangeService service({{ExchangeFormat::Ifc, &importer}}, {{ExchangeFormat::Step, &step}});

    const TempPath out("svc", ".step");
    service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Step);
    const std::string text = readText(out.path);
    EXPECT_NE(text.find("ISO-10303-21"), std::string::npos);
    EXPECT_NE(text.find("BREP"), std::string::npos);
}

// slice-042c (MR-006-042c-MED-1): das Voll-Modell (Wände + Decke + Dach + Treppe)
// über den ECHTEN Service — verifiziert, dass die format-selektive Service-Berechnung
// JEDES Bauteil mit den richtigen Parametern ableitet (nicht nur die Adapter-
// Serialisierung eines vorgefertigten Bündels). Fehlt eine Bauteil-Ableitung, weicht
// die geschlossene-Solid-Zahl ab.
TEST(StepExportIntegration, FullModelYieldsAllComponentBRepSolids) {
    const TempPath wallsOut("full_baseline", ".step");
    writeStep(sampleBuilding(), wallsOut.path);
    const std::size_t wallShells =
        countSubstr(readText(wallsOut.path), "CLOSED_SHELL");
    ASSERT_GT(wallShells, 0U);

    const int steps = 12;
    const TempPath fullOut("full_model", ".step");
    writeStep(buildingFull(steps), fullOut.path);
    const std::string step = readText(fullOut.path);
    // Wände + 1 Decke + 1 Dach + step_count Treppen-Stufen, alle als geschlossene Solids.
    EXPECT_EQ(countSubstr(step, "CLOSED_SHELL"),
              wallShells + 1 + 1 + static_cast<std::size_t>(steps))
        << "Service leitet nicht jedes Bauteil (Decke/Dach/Treppe) korrekt ab";
}

// slice-042c (MR-006-042c-LOW-1): Koordinaten-Sonde — die Decke (baseZ =
// Geschoss-Oberkante 2500) muss im STL auf ihre Aufstandshöhe gehoben sein. Ein
// verlorener baseZ-Lift / vertauschtes Storey-Feld (in der 042c-Service-Rechnung)
// bliebe von den positions-blinden Zähl-Orakeln unentdeckt.
TEST(StlExportIntegration, SlabLiftedToBaseZ) {
    const TempPath wallsOut("z_walls");
    writeStl(sampleBuilding(), wallsOut.path);
    EXPECT_LT(stlMaxZ(wallsOut.path), 2600.0);  // Wände bis zur Geschoss-Höhe 2500

    const TempPath slabOut("z_slab");
    writeStl(buildingWithSlab(), slabOut.path);
    // Decke: baseZ = 2500 (Oberkante) + Dicke 200 → Netz reicht über 2600 hinaus.
    EXPECT_GT(stlMaxZ(slabOut.path), 2600.0)
        << "Decke nicht auf ihre baseZ-Aufstandshöhe gehoben (baseZ-Lift verloren?)";
}

// slice-042c (MR-006-042c-LOW-2, MR-009-042a+042b-INFO-1): Totalität über den ECHTEN
// Service — weder ein danglender from_storey_id (→ storeyHeight-Fallback, Bauteil wird
// exportiert) noch ein degeneriertes Bauteil (→ leere/degenerierte Ableitung → OCC-Skip
// im Adapter) darf werfen. STEP und STL.
TEST(ExportTotality, DanglingStoreyAndDegenerateDoNotThrow) {
    // (a) danglender from_storey_id: eine zweite Treppe an ein NICHT existierendes
    // Geschoss (→ storeyHeight-Fallback kDefaultStoreyHeightMm).
    model::Building dangling = buildingWithStair(10);
    model::Stair orphan = dangling.stairs.front();
    orphan.id = model::StairId{2};
    orphan.from_storey_id = model::StoreyId{99};  // existiert nicht → Fallback
    dangling.stairs.push_back(orphan);

    // (b) degeneriertes Bauteil: eine Wand der Länge 0 → wallFootprint degeneriert →
    // makeNetSolid/tessellateFootprint werfen (OCC) → per-Bauteil-Skip im Adapter.
    model::Building degenerate = sampleBuilding();
    degenerate.walls.push_back({model::WallId{99}, model::StoreyId{1}, {1000.0, 1000.0},
                                {1000.0, 1000.0}, 240.0, 2500.0, model::WallType::Aussen});

    for (const model::Building& b : {dangling, degenerate}) {
        const TempPath step("total", ".step");
        const TempPath stl("total");
        EXPECT_NO_THROW(writeStep(b, step.path));
        EXPECT_NO_THROW(writeStl(b, stl.path));
    }
}

}  // namespace
