// AK + Integration (slice-019c): IFC-Export end-to-end über den ECHTEN Pfad
// `ExchangeService.exportModel` -> `ModelExporterPort` -> `IfcExportAdapter`,
// verifiziert per **Roundtrip** gegen den echten 019b-Importer (Export ->
// Import erhält Geschoss-/Wand-Anzahl, LH-FA-IO-002 Happy / ACC-003) +
// E-IO-001-Negative (nicht beschreibbarer Zielpfad, kein Teil-Export).

#include "adapters/io/ifc_export_adapter.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "adapters/io/ifc_import_adapter.h"
#include "hexagon/model/building.h"
#include "hexagon/model/constants.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/services/exchange_service.h"

namespace {

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
using bcad::adapters::io::IfcExportAdapter;
using bcad::adapters::io::IfcImportAdapter;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

// 2 Geschosse (Höhen 2500/3000), 2 gerade Wände in verschiedenen Geschossen.
model::Building sampleBuilding() {
    model::Building b;
    b.storeys.push_back({model::StoreyId{1}, 2500.0});
    b.storeys.push_back({model::StoreyId{2}, 3000.0});
    b.walls.push_back({model::WallId{1}, model::StoreyId{1}, {0.0, 0.0},
                       {5000.0, 0.0}, 240.0, 2500.0, model::WallType::Aussen});
    b.walls.push_back({model::WallId{2}, model::StoreyId{2}, {0.0, 0.0},
                       {0.0, 4000.0}, 300.0, 3000.0, model::WallType::Innen});
    return b;
}

// Temp-Zielpfad mit RAII-Aufräumen (Datei + .tmp-Artefakt).
struct TempPath {
    fs::path path;
    explicit TempPath(const std::string& tag)
        : path(fs::temp_directory_path() / ("bcad_ifcexp_" + tag + ".ifc")) {
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

// --- Roundtrip (LH-FA-IO-002 Happy / ACC-003): Export -> 019b-Import ---------

TEST(IfcExport, RoundtripPreservesCounts) {
    const TempPath out("counts");
    const IfcExportAdapter exporter;
    exporter.write(sampleBuilding(), model::DerivedGeometry{}, out.path);

    const IfcImportAdapter importer;
    const model::Building back = importer.read(out.path);
    EXPECT_EQ(back.storeys.size(), 2U);  // == Quelle
    EXPECT_EQ(back.walls.size(), 2U);    // == Quelle
}

TEST(IfcExport, RoundtripPreservesWallGeometry) {
    const TempPath out("geom");
    const IfcExportAdapter exporter;
    exporter.write(sampleBuilding(), model::DerivedGeometry{}, out.path);
    const IfcImportAdapter importer;
    const model::Building b = importer.read(out.path);

    ASSERT_EQ(b.walls.size(), 2U);
    // Identitäts-robust statt positionsabhängig (Review MED-1): Wände per Dicke
    // unterscheiden, nicht per `walls[0]/[1]` — würde der Writer umordnen, träfe
    // ein positionsbasierter Test still die falsche Wand. Werte exakt (über
    // spfReal/parse round-trip-stabil).
    const model::Wall* w240 = nullptr;
    const model::Wall* w300 = nullptr;
    for (const model::Wall& wall : b.walls) {
        if (wall.thickness_mm == 240.0) {
            w240 = &wall;
        } else if (wall.thickness_mm == 300.0) {
            w300 = &wall;
        }
    }
    ASSERT_NE(w240, nullptr);
    ASSERT_NE(w300, nullptr);
    // Wand 240: Achse (0,0)->(5000,0), Höhe 2500.
    EXPECT_DOUBLE_EQ(w240->start.x_mm, 0.0);
    EXPECT_DOUBLE_EQ(w240->end.x_mm, 5000.0);
    EXPECT_DOUBLE_EQ(w240->height_mm, 2500.0);
    // Wand 300: Achse (0,0)->(0,4000), Höhe 3000.
    EXPECT_DOUBLE_EQ(w300->end.y_mm, 4000.0);
    EXPECT_DOUBLE_EQ(w300->height_mm, 3000.0);

    ASSERT_EQ(b.storeys.size(), 2U);
    // Höhe = Elevation-Differenz: unteres Geschoss exakt (2500); das oberste
    // bekommt beim Import den Default (2500) — Roundtrip-Treue = Anzahl, nicht
    // oberste Höhe (benannte Subset-Grenze, spez. §1).
    EXPECT_DOUBLE_EQ(b.storeys[0].height_mm, 2500.0);
    EXPECT_DOUBLE_EQ(b.storeys[1].height_mm, model::kDefaultStoreyHeightMm);
    EXPECT_EQ(w240->storey_id, b.storeys[0].id);  // 240er-Wand im unteren Geschoss
    EXPECT_EQ(w300->storey_id, b.storeys[1].id);  // 300er-Wand im oberen Geschoss
}

TEST(IfcExport, EmptyBuildingRoundtripsToEmpty) {
    const TempPath out("empty");
    const IfcExportAdapter exporter;
    exporter.write(model::Building{}, model::DerivedGeometry{}, out.path);
    const IfcImportAdapter importer;
    const model::Building back = importer.read(out.path);
    EXPECT_TRUE(back.storeys.empty());
    EXPECT_TRUE(back.walls.empty());
}

// --- Negative (LH-FA-IO-002): nicht beschreibbarer Zielpfad -> E-IO-001 ------

TEST(IfcExport, NonWritablePathRejectedWithEIo001) {
    const TempPath out("eio001");
    // Temp-Pfad mit nicht-leerem Verzeichnis besetzen -> open scheitert (EISDIR,
    // root-fest; Muster Persistenz-E-IO-001-Test).
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::ofstream(tmp / "block.txt") << "x"; }

    const IfcExportAdapter exporter;
    std::string what;
    try {
        exporter.write(sampleBuilding(), model::DerivedGeometry{}, out.path);
        FAIL() << "erwarteter E-IO-001-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
    EXPECT_NE(what.find("io_no_permission"), std::string::npos) << what;
    EXPECT_FALSE(fs::exists(out.path));  // kein Teil-Export, Zielpfad unberührt
}

// --- Integration über den ECHTEN Pfad (ExchangeService -> Port -> Adapter) ---

TEST(IfcExportIntegration, ExchangeServiceRoundtripThroughRealAdapters) {
    const TempPath out("svc");
    const IfcImportAdapter importer;
    const IfcExportAdapter exporter;
    const ExchangeService service({{ExchangeFormat::Ifc, &importer}}, {{ExchangeFormat::Ifc, &exporter}});

    service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Ifc);
    const model::Building back =
        service.importModel(out.path, ExchangeFormat::Ifc);
    EXPECT_EQ(back.storeys.size(), 2U);
    EXPECT_EQ(back.walls.size(), 2U);
}

TEST(IfcExportIntegration, ExchangeServicePropagatesEIo001) {
    const TempPath out("svc_reject");
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::ofstream(tmp / "block.txt") << "x"; }

    const IfcImportAdapter importer;
    const IfcExportAdapter exporter;
    const ExchangeService service({{ExchangeFormat::Ifc, &importer}}, {{ExchangeFormat::Ifc, &exporter}});
    try {
        service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Ifc);
        FAIL() << "erwarteter E-IO-001-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
        EXPECT_NE(what.find("io_no_permission"), std::string::npos) << what;
    }
}

}  // namespace
