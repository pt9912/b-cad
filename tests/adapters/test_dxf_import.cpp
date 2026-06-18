// AK + Integration (slice-021b Commit ii): DXF-Import end-to-end über den
// ECHTEN Pfad `ExchangeService` -> `ModelImporterPort` -> `DxfImportAdapter`
// (ExchangeFormat::Dxf). AK `LH-FA-IO-003` Happy/Boundary/Negative + Subset-Skip
// + MED-2-Mitte + LWPOLYLINE (LOW-1). Fixtures sind Raw-String-DXF in eine
// Temp-Datei geschrieben (der Port liest einen Pfad).

#include "adapters/io/dxf_import_adapter.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/constants.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/services/exchange_service.h"

namespace {

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
using bcad::adapters::io::DxfImportAdapter;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

// Temp-DXF-Datei aus Roh-Inhalt; entfernt sich selbst.
class TempDxf {
public:
    TempDxf(const std::string& tag, const std::string& content)
        : path(fs::temp_directory_path() / ("bcad_dxfimp_" + tag + ".dxf")) {
        std::ofstream(path, std::ios::binary) << content;
    }
    ~TempDxf() {
        std::error_code ec;
        fs::remove(path, ec);
    }
    TempDxf(const TempDxf&) = delete;
    TempDxf& operator=(const TempDxf&) = delete;
    fs::path path;
};

// 2 Geschoss-Layer: STOREY_1 (2 Wände), STOREY_2 (1 Wand) -> 3 Wände/2 Geschosse.
std::string sampleDxf() {
    return "0\nSECTION\n2\nENTITIES\n"
           "0\nLINE\n8\nSTOREY_1\n10\n0.0\n20\n0.0\n11\n5000.0\n21\n0.0\n30\n0.0\n31\n0.0\n"
           "0\nLINE\n8\nSTOREY_1\n10\n5000.0\n20\n0.0\n11\n5000.0\n21\n4000.0\n"
           "0\nLINE\n8\nSTOREY_2\n10\n0.0\n20\n0.0\n11\n3000.0\n21\n0.0\n"
           "0\nENDSEC\n0\nEOF\n";
}

// --- LH-FA-IO-003 Happy: Anzahl stimmt, Default-Höhe/-Dicke ----------------

TEST(DxfImport, HappyPathCountsMatchSourceWithDefaults) {
    const TempDxf fx("happy", sampleDxf());
    const model::Building b = DxfImportAdapter().read(fx.path);
    ASSERT_EQ(b.walls.size(), 3U);
    EXPECT_EQ(b.storeys.size(), 2U);
    EXPECT_DOUBLE_EQ(b.walls[0].thickness_mm, model::kDefaultWallThicknessMm);
    EXPECT_DOUBLE_EQ(b.walls[0].height_mm, model::kDefaultStoreyHeightMm);
    EXPECT_DOUBLE_EQ(b.walls[0].end.x_mm, 5000.0);
}

// --- LH-FA-IO-003 Boundary: leer / strukturlos -> leeres Modell (kein Wurf) --

TEST(DxfImport, EmptyFileYieldsEmptyModel) {
    const TempDxf fx("empty", "");
    const model::Building b = DxfImportAdapter().read(fx.path);
    EXPECT_TRUE(b.walls.empty());
    EXPECT_TRUE(b.storeys.empty());
}

// MED-2-Mitte: wohlgeformte Gruppencode-Paare OHNE ENTITIES -> leeres Modell.
TEST(DxfImport, StructurelessButWellFormedYieldsEmptyModel) {
    const TempDxf fx("structureless",
                     "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1009\n"
                     "0\nENDSEC\n0\nEOF\n");
    const model::Building b = DxfImportAdapter().read(fx.path);
    EXPECT_TRUE(b.walls.empty());
}

// --- LH-FA-IO-003 Negative: nicht-DXF -> E-IO-003, kein Teil-Import ---------

TEST(DxfImport, NonDxfFileRejectedWithEIo003) {
    const TempDxf fx("nondxf", "this is not dxf\nat all\n");
    try {
        DxfImportAdapter().read(fx.path);
        FAIL() << "erwarteter E-IO-003-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("E-IO-003"), std::string::npos);
        EXPECT_NE(msg.find("import_rejected"), std::string::npos);
    }
}

// --- Subset-Skip: Fremd-Entitäten übersprungen, nicht abgelehnt ------------

TEST(DxfImport, SubsetForeignEntitiesAreSkippedNotRejected) {
    const TempDxf fx("subset",
                     "0\nSECTION\n2\nENTITIES\n"
                     "0\nCIRCLE\n8\nSTOREY_1\n10\n0.0\n20\n0.0\n40\n100.0\n"
                     "0\nLINE\n8\nSTOREY_1\n10\n0.0\n20\n0.0\n11\n1000.0\n21\n0.0\n"
                     "0\nENDSEC\n0\nEOF\n");
    const model::Building b = DxfImportAdapter().read(fx.path);
    EXPECT_EQ(b.walls.size(), 1U);  // CIRCLE übersprungen, LINE bleibt
}

// --- LWPOLYLINE (LOW-1): n Vertices -> n-1 gerade Wände ---------------------

TEST(DxfImport, LwpolylineYieldsSegmentWalls) {
    const TempDxf fx("lwpoly",
                     "0\nSECTION\n2\nENTITIES\n"
                     "0\nLWPOLYLINE\n8\nSTOREY_1\n90\n3\n"
                     "10\n0.0\n20\n0.0\n10\n1000.0\n20\n0.0\n10\n1000.0\n20\n2000.0\n"
                     "0\nENDSEC\n0\nEOF\n");
    const model::Building b = DxfImportAdapter().read(fx.path);
    ASSERT_EQ(b.walls.size(), 2U);  // 3 Vertices -> 2 Segmente -> 2 Wände
    // MED-2: Achs-Lage + Vertex-Reihenfolge (fängt invertiertes/x-y-vertauschtes Mapping).
    EXPECT_DOUBLE_EQ(b.walls[0].start.x_mm, 0.0);
    EXPECT_DOUBLE_EQ(b.walls[0].start.y_mm, 0.0);
    EXPECT_DOUBLE_EQ(b.walls[0].end.x_mm, 1000.0);
    EXPECT_DOUBLE_EQ(b.walls[0].end.y_mm, 0.0);
    EXPECT_DOUBLE_EQ(b.walls[1].start.x_mm, 1000.0);
    EXPECT_DOUBLE_EQ(b.walls[1].end.y_mm, 2000.0);
}

// MED-1: eine teil-koordinierte LINE (fehlendes Endpunkt-Paar) wird
// übersprungen — keine (0,0)-Wand, keine Ablehnung.
TEST(DxfImport, PartiallyCoordinatedLineIsSkipped) {
    const TempDxf fx("partial",
                     "0\nSECTION\n2\nENTITIES\n"
                     "0\nLINE\n8\nSTOREY_1\n10\n0.0\n20\n0.0\n"  // 11/21 fehlen
                     "0\nLINE\n8\nSTOREY_1\n10\n0.0\n20\n0.0\n11\n1000.0\n21\n0.0\n"
                     "0\nENDSEC\n0\nEOF\n");
    const model::Building b = DxfImportAdapter().read(fx.path);
    EXPECT_EQ(b.walls.size(), 1U);  // unvollständige LINE übersprungen
}

// --- Integration über den ECHTEN Pfad (ExchangeService -> Port -> Adapter) ---

TEST(DxfImportIntegration, ExchangeServiceImportsThroughRealAdapter) {
    const TempDxf fixture("svc_happy", sampleDxf());
    const DxfImportAdapter adapter;
    const ExchangeService service({{ExchangeFormat::Dxf, &adapter}}, {});

    const model::Building b =
        service.importModel(fixture.path, ExchangeFormat::Dxf);
    EXPECT_EQ(b.storeys.size(), 2U);
    EXPECT_EQ(b.walls.size(), 3U);
}

TEST(DxfImportIntegration, ExchangeServicePropagatesEIo003) {
    const TempDxf fixture("svc_reject", "<not-dxf-bytes>\nrandom text\n");
    const DxfImportAdapter adapter;
    const ExchangeService service({{ExchangeFormat::Dxf, &adapter}}, {});
    try {
        service.importModel(fixture.path, ExchangeFormat::Dxf);
        FAIL() << "erwarteter E-IO-003-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("E-IO-003"), std::string::npos);
        EXPECT_NE(msg.find("import_rejected"), std::string::npos);
    }
}

}  // namespace
