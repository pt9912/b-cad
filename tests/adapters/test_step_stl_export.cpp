// AK + Integration (slice-020b): STL-Export end-to-end über den ECHTEN Pfad
// `ExchangeService.exportModel` -> `ModelExporterPort` -> `StlExportAdapter`
// (geometrie-resident, ADR-0014). Re-Read-Orakel: die binäre STL-Datei wird
// geparst (80-Byte-Header + uint32-Dreieckszahl + 50 Byte/Dreieck) — ein Modell
// mit Wänden trägt > 0 Dreiecke; leeres Modell -> gültige leere STL. Negative:
// nicht beschreibbarer Zielpfad -> E-IO-001 (kein Teil-Export). STEP (B-Rep)
// folgt im zweiten Commit dieses Slices.

#include "adapters/geometry/stl_export_adapter.h"

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

// --- Happy (LH-FA-IO-006): valide STL mit Dreiecksnetz der Bauteile ----------

TEST(StlExport, BuildingWithWallsYieldsNonEmptyMesh) {
    const OccGeometryAdapter geometry;
    const StlExportAdapter exporter(geometry);
    const TempPath out("walls");
    exporter.write(sampleBuilding(), out.path);

    const StlInfo info = readStl(out.path);
    ASSERT_TRUE(info.ok);
    EXPECT_GT(info.triangles, 0U);
    // Binäres STL ist exakt: 84 Byte Kopf + 50 Byte je Dreieck.
    EXPECT_EQ(info.size, 84U + 50ULL * info.triangles);
}

// --- Boundary: 3D-leeres Modell -> gültige leere STL (Totalität) -------------

TEST(StlExport, EmptyBuildingYieldsValidEmptyStl) {
    const OccGeometryAdapter geometry;
    const StlExportAdapter exporter(geometry);
    const TempPath out("empty");
    exporter.write(model::Building{}, out.path);

    const StlInfo info = readStl(out.path);
    ASSERT_TRUE(info.ok);
    EXPECT_EQ(info.triangles, 0U);
    EXPECT_EQ(info.size, 84U);  // nur Header + Zähler, keine Dreiecke
}

// --- Negative (LH-FA-IO-006): nicht beschreibbarer Zielpfad -> E-IO-001 ------

TEST(StlExport, NonWritablePathRejectedWithEIo001) {
    const OccGeometryAdapter geometry;
    const StlExportAdapter exporter(geometry);
    const TempPath out("eio001");
    // Temp-Pfad mit nicht-leerem Verzeichnis besetzen -> open scheitert (EISDIR,
    // root-fest; Muster IFC-/Persistenz-E-IO-001-Test).
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::ofstream(tmp / "block.txt") << "x"; }

    std::string what;
    try {
        exporter.write(sampleBuilding(), out.path);
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
// Re-Read-Orakel ohne OCC im Test (arch-check Regel C): die STEP-Datei wird als
// ISO-10303-21-Text geprüft (gültige Hülle + B-Rep-Solid-Entitäten).

std::string readText(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}

TEST(StepExport, BuildingWithWallsYieldsBRepSolids) {
    const StepExportAdapter exporter;
    const TempPath out("walls", ".step");
    exporter.write(sampleBuilding(), out.path);

    const std::string step = readText(out.path);
    // Gültige ISO-10303-21-Hülle.
    EXPECT_NE(step.find("ISO-10303-21"), std::string::npos);
    EXPECT_NE(step.find("END-ISO-10303-21"), std::string::npos);
    // Die exportierten Bauteile sind als B-Rep-Volumenkörper enthalten.
    EXPECT_NE(step.find("BREP"), std::string::npos) << "kein B-Rep-Solid im STEP";
}

TEST(StepExport, EmptyBuildingYieldsValidStepEnvelope) {
    const StepExportAdapter exporter;
    const TempPath out("empty", ".step");
    exporter.write(model::Building{}, out.path);  // 3D-leer -> kein Wurf

    const std::string step = readText(out.path);
    EXPECT_NE(step.find("ISO-10303-21"), std::string::npos);
    EXPECT_NE(step.find("END-ISO-10303-21"), std::string::npos);
}

TEST(StepExport, NonWritablePathRejectedWithEIo001) {
    const StepExportAdapter exporter;
    const TempPath out("eio001", ".step");
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::ofstream(tmp / "block.txt") << "x"; }

    std::string what;
    try {
        exporter.write(sampleBuilding(), out.path);
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

}  // namespace
