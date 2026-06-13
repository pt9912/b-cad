// Akzeptanz-Tests für den Domain-Kern (slice-003a), OCC-frei über ein
// GeometryKernelPort-Double (ADR-0001 §Testbarkeit). Geprüft werden
// LH-FA-BLD-001 (Default-Geschoss), LH-FA-FLR-001 (Geschoss anlegen),
// LH-FA-WAL-001 (Wand + Null-Längen-Boundary), die Klemmung/Validierung
// LH-FA-WAL-002/003 (E-VAL-001, inkl. Nicht-Endlich-Ablehnung) und die
// transaktionale Rebuild-Garantie bei Geometrie-Fehler (E-GEO-002).
// Test-Namen tragen die LH-ID.

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

#include "analytic_geometry_double.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/solid.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"
#include "hexagon/ports/driving/edit_structure_port.h"  // ParamStatus
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;
namespace driving = bcad::hexagon::ports::driving;
// Gemeinsames Geometrie-Double (Review L4: vorher dreifach dupliziert).
using bcad::testing::AnalyticGeometry;
using bcad::testing::analyticVolume;

// Steuerbares Double: `extrudeWall` wirft, wenn `failing` gesetzt ist —
// modelliert eine fehlschlagende Geometrie-Operation (E-GEO-002-Klasse),
// um die transaktionale Rebuild-Garantie zu prüfen.
class ControllableGeometry final : public driven::GeometryKernelPort {
public:
    void setFailing(bool failing) { failing_ = failing; }
    // Fehlerinjektion fuer den Mehr-Element-Pfad (LH-FA-WAL-006, W3-P2):
    // wirft beim n-ten extrude-Aufruf (1-basiert), 0 = nie.
    void failOnExtrudeCall(int call_number) { fail_on_call_ = call_number; }
    model::Solid extrudeFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const override {
        ++extrude_calls_;
        if (failing_ || extrude_calls_ == fail_on_call_) {
            throw std::runtime_error("Geometrie-Operation fehlgeschlagen");
        }
        return model::Solid{
            bcad::testing::analyticVolume(footprint, height_mm, cutouts)};
    }
    model::TriangleMesh tessellateFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const override {
        if (failing_) {
            throw std::runtime_error("Geometrie-Operation fehlgeschlagen");
        }
        return AnalyticGeometry{}.tessellateFootprint(footprint, height_mm,
                                                      cutouts);
    }

private:
    bool failing_{false};
    int fail_on_call_{0};
    mutable int extrude_calls_{0};
};

constexpr model::StoreyId kGroundStorey{1};
constexpr model::Segment kWall1000{{0.0, 0.0}, {1000.0, 0.0}};

}  // namespace

TEST(StructureEditService_LH_FA_BLD_001, NeuesGebaeudeHatGenauEinDefaultGeschoss) {
    const AnalyticGeometry geometry;
    const services::StructureEditService svc(geometry);
    ASSERT_EQ(svc.building().storeys.size(), 1U);
    EXPECT_DOUBLE_EQ(svc.building().storeys.front().height_mm,
                     model::kDefaultStoreyHeightMm);
    EXPECT_TRUE(svc.building().walls.empty());
}

TEST(StructureEditService_LH_FA_FLR_001, GeschossAnlegen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::StoreyId id = svc.addStorey(3000.0);
    ASSERT_EQ(svc.building().storeys.size(), 2U);
    EXPECT_EQ(svc.building().storeys.back().id, id);
    EXPECT_DOUBLE_EQ(svc.building().storeys.back().height_mm, 3000.0);
}

TEST(StructureEditService_LH_FA_WAL_001, WandMitDefaultsAusZweiPunkten) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000);
    ASSERT_TRUE(id.has_value());
    EXPECT_DOUBLE_EQ(svc.wall(*id).thickness_mm, model::kDefaultWallThicknessMm);
    // Default-Höhe = Geschosshöhe (parametrisch).
    EXPECT_DOUBLE_EQ(svc.wall(*id).height_mm, model::kDefaultStoreyHeightMm);
}

TEST(StructureEditService_LH_FA_WAL_001, NullLaengenWandVerworfen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::Segment degenerate{{5.0, 5.0}, {5.0, 5.0}};
    EXPECT_FALSE(svc.addWall(kGroundStorey, degenerate).has_value());
    EXPECT_TRUE(svc.building().walls.empty());
}

TEST(StructureEditService_LH_FA_WAL_002, StaerkeGrenzwerteAkzeptiert) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();

    const auto low = svc.setWallThickness(id, model::kWallThicknessMinMm);
    EXPECT_EQ(low.status, driving::ParamStatus::Accepted);
    EXPECT_DOUBLE_EQ(low.applied_mm, 50.0);

    const auto high = svc.setWallThickness(id, model::kWallThicknessMaxMm);
    EXPECT_EQ(high.status, driving::ParamStatus::Accepted);
    EXPECT_DOUBLE_EQ(high.applied_mm, 1000.0);
}

// Lastenheft-Happy-Path: Stärke = 240 mm wird akzeptiert und das Solid
// aktualisiert sich SOFORT (vgl. LH-FA-D3-002) — der Rebuild über den
// Port läuft synchron mit der Setzung.
TEST(StructureEditService_LH_FA_WAL_002, Staerke240HappyPathMitSofortigemRebuild) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();

    const auto result = svc.setWallThickness(id, 240.0);
    EXPECT_EQ(result.status, driving::ParamStatus::Accepted);
    EXPECT_DOUBLE_EQ(result.applied_mm, 240.0);
    EXPECT_DOUBLE_EQ(svc.wall(id).thickness_mm, 240.0);
    // Sofortige Geometrie-Aktualisierung: Länge 1000 · Stärke 240 ·
    // Höhe 2500 (Default-Geschosshöhe).
    EXPECT_DOUBLE_EQ(svc.wallSolid(id).volume_mm3,
                     1000.0 * 240.0 * model::kDefaultStoreyHeightMm);
}

TEST(StructureEditService_LH_FA_WAL_002, StaerkeAusserhalbWirdGeklemmt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();

    const auto under = svc.setWallThickness(id, 49.0);
    EXPECT_EQ(under.status, driving::ParamStatus::Clamped);  // E-VAL-001
    EXPECT_DOUBLE_EQ(under.applied_mm, 50.0);
    EXPECT_DOUBLE_EQ(svc.wall(id).thickness_mm, 50.0);

    const auto over = svc.setWallThickness(id, 1001.0);
    EXPECT_EQ(over.status, driving::ParamStatus::Clamped);  // E-VAL-001
    EXPECT_DOUBLE_EQ(over.applied_mm, 1000.0);
    EXPECT_DOUBLE_EQ(svc.wall(id).thickness_mm, 1000.0);
}

// Nicht-endliche Stärke (NaN/Inf) wird abgelehnt; das Modell bleibt
// gültig und unverändert (kein NaN-Wert/-Solid gespeichert).
TEST(StructureEditService_LH_FA_WAL_002, StaerkeNichtEndlichWirdAbgelehnt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();
    svc.setWallThickness(id, 300.0);  // gültiger Ausgangsstand
    const double volBefore = svc.wallSolid(id).volume_mm3;

    for (const double bad : {std::numeric_limits<double>::quiet_NaN(),
                             std::numeric_limits<double>::infinity()}) {
        const auto result = svc.setWallThickness(id, bad);
        EXPECT_EQ(result.status, driving::ParamStatus::Rejected);
        EXPECT_DOUBLE_EQ(svc.wall(id).thickness_mm, 300.0);
        EXPECT_DOUBLE_EQ(svc.wallSolid(id).volume_mm3, volBefore);
        EXPECT_TRUE(std::isfinite(svc.wallSolid(id).volume_mm3));
    }
}

TEST(StructureEditService_LH_FA_WAL_003, HoeheGrenzwerteAkzeptiert) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();

    EXPECT_EQ(svc.setWallHeight(id, model::kWallHeightMinMm).status,
              driving::ParamStatus::Accepted);
    EXPECT_EQ(svc.setWallHeight(id, model::kWallHeightMaxMm).status,
              driving::ParamStatus::Accepted);
}

TEST(StructureEditService_LH_FA_WAL_003, HoeheAusserhalbWirdGeklemmt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();

    const auto under = svc.setWallHeight(id, 499.0);
    EXPECT_EQ(under.status, driving::ParamStatus::Clamped);  // E-VAL-001
    EXPECT_DOUBLE_EQ(under.applied_mm, 500.0);

    const auto over = svc.setWallHeight(id, 10001.0);
    EXPECT_EQ(over.status, driving::ParamStatus::Clamped);  // E-VAL-001
    EXPECT_DOUBLE_EQ(over.applied_mm, 10000.0);
}

TEST(StructureEditService_LH_FA_WAL_003, HoeheNichtEndlichWirdAbgelehnt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();
    const double before = svc.wall(id).height_mm;

    const auto result =
        svc.setWallHeight(id, std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(result.status, driving::ParamStatus::Rejected);
    EXPECT_DOUBLE_EQ(svc.wall(id).height_mm, before);
}

// Rebuild über den Port: das gehaltene Solid spiegelt den (geklemmten)
// Parameter-Stand. Beweist die Driving→Service→Driven-Verkettung.
TEST(StructureEditService, RebuildSpiegeltGeklemmtenStandUeberPort) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();

    svc.setWallThickness(id, 1001.0);  // → 1000
    svc.setWallHeight(id, 3000.0);
    // Länge 1000 · Stärke 1000 · Höhe 3000.
    EXPECT_DOUBLE_EQ(svc.wallSolid(id).volume_mm3, 1000.0 * 1000.0 * 3000.0);
}

// E-GEO-002 (Finding 1): Schlägt die Extrusion fehl, bleibt das Modell
// unverändert — die Wand wird gar nicht erst übernommen (transaktional).
TEST(StructureEditService, AddWallTransaktionalBeiGeometrieFehler) {
    ControllableGeometry geometry;
    geometry.setFailing(true);
    services::StructureEditService svc(geometry);
    EXPECT_THROW((void)svc.addWall(kGroundStorey, kWall1000), std::runtime_error);
    EXPECT_TRUE(svc.building().walls.empty());  // Modell unverändert
}

// E-GEO-002 (Finding 1): Schlägt der Rebuild bei einer Parameteränderung
// fehl, bleiben Wandparameter UND altes Solid unverändert.
TEST(StructureEditService, SetThicknessTransaktionalBeiGeometrieFehler) {
    ControllableGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto id = svc.addWall(kGroundStorey, kWall1000).value();
    const double thicknessBefore = svc.wall(id).thickness_mm;
    const double volBefore = svc.wallSolid(id).volume_mm3;

    geometry.setFailing(true);
    EXPECT_THROW((void)svc.setWallThickness(id, 300.0), std::runtime_error);
    EXPECT_DOUBLE_EQ(svc.wall(id).thickness_mm, thicknessBefore);  // unverändert
    EXPECT_DOUBLE_EQ(svc.wallSolid(id).volume_mm3, volBefore);     // altes Solid
}

// LH-FA-WAL-006 / W3-P2: Transaktions-Garantie des Mehr-Element-Pfads —
// wirft der NACHBAR-Rebuild (Eckenschluss), bleibt das gesamte Modell
// unverändert und es ergeht KEINE Meldung.
TEST(StructureEditService_LH_FA_WAL_006, NachbarRebuildFehlerTransaktional) {
    ControllableGeometry geometry;
    services::StructureEditService svc(geometry);
    // Aufrufe 1-3: Wand A, Wand B (Kandidat) + Nachbar-Rebuild A.
    const auto a = svc.addWall(kGroundStorey, {{0.0, 0.0}, {1000.0, 0.0}}).value();
    const auto b = svc.addWall(kGroundStorey, {{1000.0, 0.0}, {1000.0, 1000.0}}).value();
    const double volA = svc.wallSolid(a).volume_mm3;
    const double volB = svc.wallSolid(b).volume_mm3;

    struct Counting final : driven::ModelChangedPort {
        int calls{0};
        void onModelChanged(const driven::ModelChange&) override { ++calls; }
    } listener;
    svc.subscribe(listener);
    // Aufruf 4 = Wand A (trial), Aufruf 5 = Nachbar B -> wirft.
    geometry.failOnExtrudeCall(5);
    EXPECT_THROW((void)svc.setWallThickness(a, 300.0), std::runtime_error);

    EXPECT_DOUBLE_EQ(svc.wall(a).thickness_mm, model::kDefaultWallThicknessMm);
    EXPECT_DOUBLE_EQ(svc.wallSolid(a).volume_mm3, volA);
    EXPECT_DOUBLE_EQ(svc.wallSolid(b).volume_mm3, volB);
    EXPECT_EQ(listener.calls, 0);  // keine Meldung (ADR-0008: nur Committetes)
    svc.unsubscribe(listener);
}
