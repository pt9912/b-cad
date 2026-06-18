// AK + Roundtrip + Integration (slice-021b Commit ii): DXF-Export end-to-end
// über den ECHTEN Pfad `ExchangeService` -> `ModelExporterPort` ->
// `DxfExportAdapter` (ExchangeFormat::Dxf). AK `LH-FA-IO-004` Happy/Boundary/
// Negative. **Roundtrip** = Wand-Achsen-Anzahl je Geschoss + Achs-Lage (NICHT
// Höhe/Dicke — DXF trägt sie nicht).

#include "adapters/io/dxf_export_adapter.h"
#include "adapters/io/dxf_import_adapter.h"

#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <system_error>

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
using bcad::adapters::io::DxfExportAdapter;
using bcad::adapters::io::DxfImportAdapter;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

// Eindeutiger Temp-Zielpfad; räumt Ziel + `.tmp` (Datei/Verzeichnis) auf.
class TempPath {
public:
    explicit TempPath(const std::string& tag)
        : path(fs::temp_directory_path() / ("bcad_dxfexp_" + tag + ".dxf")) {
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

// 2 Geschosse: storey 1 (2 Wände), storey 2 (1 Wand) -> 3 Wände/2 Geschosse.
model::Building sampleBuilding() {
    model::Building b;
    b.storeys.push_back(model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    b.storeys.push_back(model::Storey{model::StoreyId{2}, model::kDefaultStoreyHeightMm});
    b.walls.push_back(makeWall(1, model::StoreyId{1}, {0.0, 0.0}, {5000.0, 0.0}));
    b.walls.push_back(makeWall(2, model::StoreyId{1}, {5000.0, 0.0}, {5000.0, 4000.0}));
    b.walls.push_back(makeWall(3, model::StoreyId{2}, {0.0, 0.0}, {3000.0, 0.0}));
    return b;
}

// --- LH-FA-IO-004 Roundtrip: Anzahl je Geschoss + Achs-Lage ----------------

TEST(DxfExport, RoundtripPreservesPerStoreyCountsAndAxes) {
    const TempPath out("rt");
    DxfExportAdapter().write(sampleBuilding(), out.path);
    ASSERT_TRUE(fs::exists(out.path));

    const model::Building back = DxfImportAdapter().read(out.path);
    EXPECT_EQ(back.storeys.size(), 2U);
    ASSERT_EQ(back.walls.size(), 3U);

    std::map<int, int> per_storey;
    for (const model::Wall& w : back.walls) {
        ++per_storey[static_cast<int>(w.storey_id)];
    }
    EXPECT_EQ(per_storey[1], 2);  // STOREY_1: 2 Wände
    EXPECT_EQ(per_storey[2], 1);  // STOREY_2: 1 Wand

    // Achs-Lage erhalten (nicht Höhe/Dicke — benannte Subset-Grenze).
    EXPECT_DOUBLE_EQ(back.walls[0].start.x_mm, 0.0);
    EXPECT_DOUBLE_EQ(back.walls[0].end.x_mm, 5000.0);
    EXPECT_DOUBLE_EQ(back.walls[1].end.y_mm, 4000.0);
}

// --- LH-FA-IO-004 Boundary: leeres Modell -> gültige, leere DXF ------------

TEST(DxfExport, EmptyBuildingRoundtripsToEmpty) {
    const TempPath out("empty");
    DxfExportAdapter().write(model::Building{}, out.path);
    EXPECT_TRUE(fs::exists(out.path));
    const model::Building back = DxfImportAdapter().read(out.path);
    EXPECT_TRUE(back.walls.empty());
    EXPECT_TRUE(back.storeys.empty());
}

// --- LH-FA-IO-004 Negative: nicht beschreibbarer Zielpfad -> E-IO-001 -------
// über den ECHTEN Pfad (ExchangeService -> Port -> Adapter). `.tmp` ist ein
// Verzeichnis -> open() scheitert (EISDIR) -> E-IO-001, kein Teil-Export.

TEST(DxfExportIntegration, NonWritablePathRejectedWithEIo001) {
    const TempPath out("reject");
    const fs::path tmp(out.path.string() + ".tmp");
    fs::create_directory(tmp);
    { std::error_code ec; (void)fs::create_directory(tmp / "block", ec); }

    const DxfExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Dxf, &exporter}});
    try {
        service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Dxf);
        FAIL() << "erwarteter E-IO-001-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string what = e.what();
        EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
        EXPECT_NE(what.find("io_no_permission"), std::string::npos) << what;
    }
    EXPECT_FALSE(fs::exists(out.path));  // kein Teil-Export
}

// --- Integration Happy: Export schreibt durch den echten Adapter -----------

TEST(DxfExportIntegration, ExchangeServiceWritesDxfThroughRealAdapter) {
    const TempPath out("svc");
    const DxfExportAdapter exporter;
    const ExchangeService service({}, {{ExchangeFormat::Dxf, &exporter}});

    service.exportModel(sampleBuilding(), out.path, ExchangeFormat::Dxf);
    ASSERT_TRUE(fs::exists(out.path));
    const model::Building back = DxfImportAdapter().read(out.path);
    EXPECT_EQ(back.walls.size(), 3U);
}

}  // namespace
