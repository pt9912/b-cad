// AK-Tests Wandöffnungen (Türen/Fenster, LH-FA-DOR-*/WIN-*, ADR-0011)
// gegen das analytische GeometryKernelPort-Double (Volumen = Footprint
// minus Öffnungs-Schnittkörper). Geprüft wird die Kern-Logik: Platzieren,
// Klemmung, Folge-Meldung (`WallGeometryChanged` der Wirtswand, KEIN
// `RoomsChanged`), Transaktion (Fehlerfall) — display-/adapter-frei.

#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

#include "hexagon/model/constants.h"
#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/segment.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/services/structure_edit_service.h"
#include "analytic_geometry_double.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;
namespace driving = bcad::hexagon::ports::driving;
using bcad::testing::AnalyticGeometry;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

// Double, das auf Wunsch beim Extrudieren wirft (E-GEO-002-Klasse) —
// für die transaktionale Garantie (Muster slice-012 W3-P2).
class FailableGeometry final : public driven::GeometryKernelPort {
public:
    void setFailing(bool failing) { failing_ = failing; }
    model::Solid extrudeFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const override {
        if (failing_) {
            throw std::runtime_error("Geometrie-Operation fehlgeschlagen");
        }
        return model::Solid{
            bcad::testing::analyticVolume(footprint, height_mm, cutouts)};
    }
    model::TriangleMesh tessellateFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const override {
        return AnalyticGeometry{}.tessellateFootprint(footprint, height_mm,
                                                      cutouts);
    }

private:
    bool failing_{false};
};

// Beobachter, der die gemeldeten Ops mitschreibt (Folge-Meldungs-AK).
class RecordingListener final : public driven::ModelChangedPort {
public:
    void onModelChanged(const driven::ModelChange& change) override {
        ops.push_back(change.op);
    }
    int count(driven::ModelChangeOp op) const {
        int n = 0;
        for (const auto value : ops) {
            if (value == op) {
                ++n;
            }
        }
        return n;
    }
    std::vector<driven::ModelChangeOp> ops;
};

constexpr model::StoreyId kEg{1};

// Volumen einer freistehenden Wand (Länge·Stärke·Höhe).
double wallVolume(double length, double thickness, double height) {
    return length * thickness * height;
}

class Openings : public ::testing::Test {
protected:
    AnalyticGeometry geometry_;
    services::StructureEditService service_{geometry_};
};

// Happy (LH-FA-DOR-004): eine platzierte Tür verringert das Wandvolumen
// um das Öffnungsvolumen (Breite·Stärke·Höhe, ganz in der Wand).
TEST_F(Openings, LH_FA_DOR_004_TuerVerringertWandvolumen) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));  // 240×2500
    ASSERT_TRUE(wall.has_value());
    const double full =
        wallVolume(4000, model::kDefaultWallThicknessMm, model::kDefaultStoreyHeightMm);
    ASSERT_NEAR(service_.wallSolid(*wall).volume_mm3, full, full * 1e-9);

    const auto door = service_.addDoor(*wall, 1500.0);  // 900×2100, sill 0
    ASSERT_TRUE(door.has_value());

    const double opening = model::kDefaultDoorWidthMm *
                           model::kDefaultWallThicknessMm *
                           model::kDefaultDoorHeightMm;
    EXPECT_NEAR(service_.wallSolid(*wall).volume_mm3, full - opening,
                full * 1e-9);
}

// Happy (LH-FA-WIN-004/005): ein Fenster mit Brüstung entfernt nur sein
// Volumen (Breite·Stärke·Höhe) — die Wand unter der Brüstung bleibt.
TEST_F(Openings, LH_FA_WIN_005_FensterMitBruestung) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const double full =
        wallVolume(4000, model::kDefaultWallThicknessMm, model::kDefaultStoreyHeightMm);

    const auto win = service_.addWindow(*wall, 1500.0);  // 1200×1300, sill 900
    ASSERT_TRUE(win.has_value());

    const double opening = model::kDefaultWindowWidthMm *
                           model::kDefaultWallThicknessMm *
                           model::kDefaultWindowHeightMm;
    EXPECT_NEAR(service_.wallSolid(*wall).volume_mm3, full - opening,
                full * 1e-9);
    EXPECT_GT(service_.opening(*win).sill_height_mm, 0.0);  // Brüstung wirksam
}

// Boundary (LH-FA-DOR-003): Breite außerhalb [600,2000] wird geklemmt.
TEST_F(Openings, LH_FA_DOR_003_BreiteGeklemmt) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const auto door = service_.addDoor(*wall, 100.0);
    ASSERT_TRUE(door.has_value());

    const auto result = service_.setOpeningWidth(*door, 5000.0);  // > max
    EXPECT_EQ(result.status, driving::ParamStatus::Clamped);
    EXPECT_NEAR(result.applied_mm, model::kDoorWidthMaxMm, 1e-9);
}

// Boundary (LH-FA-DOR-001): Platzierung über das Wandende hinaus wird auf
// den gültigen Bereich geklemmt (Öffnung bleibt vollständig in der Wand).
TEST_F(Openings, LH_FA_DOR_001_PositionGeklemmt) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const auto door = service_.addDoor(*wall, 3800.0);  // 900 breit → würde überragen
    ASSERT_TRUE(door.has_value());

    // offset auf [0, 4000-900] geklemmt.
    EXPECT_NEAR(service_.opening(*door).offset_mm, 3100.0, 1e-9);
}

// Boundary (LH-FA-WIN-004): Öffnung höher als die Wand → Durchbruch
// höchstens bis zur Wandhöhe (kein Durchbruch über die Wand hinaus).
TEST_F(Openings, LH_FA_WIN_004_OeffnungAufWandhoeheGeklemmt) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    ASSERT_EQ(service_.setWallHeight(*wall, 1500.0).status,
              driving::ParamStatus::Accepted);

    const auto door = service_.addDoor(*wall, 1500.0);  // Höhe 2100 > 1500
    ASSERT_TRUE(door.has_value());

    // Entferntes Volumen ist auf die Wandhöhe begrenzt (900·240·1500).
    const double full = wallVolume(4000, model::kDefaultWallThicknessMm, 1500.0);
    const double opening =
        model::kDefaultDoorWidthMm * model::kDefaultWallThicknessMm * 1500.0;
    EXPECT_NEAR(service_.wallSolid(*wall).volume_mm3, full - opening,
                full * 1e-9);
}

// Negative (LH-FA-DOR-001): ohne Wirtswand → abgelehnt, keine Öffnung.
TEST_F(Openings, LH_FA_DOR_001_OhneWirtswandAbgelehnt) {
    const auto door = service_.addDoor(model::WallId{999}, 1000.0);
    EXPECT_FALSE(door.has_value());
    EXPECT_TRUE(service_.openings().empty());
}

// Negative (LH-FA-DOR-001): Wand zu kurz für die schmalste Tür → abgelehnt.
TEST_F(Openings, LH_FA_DOR_001_WandZuKurzAbgelehnt) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 500, 0));  // < 600 (min Tür)
    ASSERT_TRUE(wall.has_value());
    const auto door = service_.addDoor(*wall, 0.0);
    EXPECT_FALSE(door.has_value());
    EXPECT_TRUE(service_.openings().empty());
}

// Negative (LH-FA-DOR-004): entfernte Tür → die Wand schließt sich wieder
// (Volumen vollständig).
TEST_F(Openings, LH_FA_DOR_004_EntfernteTuerSchliesstWand) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const double full =
        wallVolume(4000, model::kDefaultWallThicknessMm, model::kDefaultStoreyHeightMm);
    const auto door = service_.addDoor(*wall, 1500.0);
    ASSERT_TRUE(door.has_value());

    EXPECT_TRUE(service_.removeOpening(*door));

    EXPECT_TRUE(service_.openings().empty());
    EXPECT_NEAR(service_.wallSolid(*wall).volume_mm3, full, full * 1e-9);
}

// Folge-Meldung (ADR-0011 (4)/(5)): eine Öffnungs-Mutation meldet GENAU
// die `WallGeometryChanged` der Wirtswand und KEINE `RoomsChanged`
// (Öffnung ändert weder Wandachse noch Stärke → Raumerkennung unberührt).
TEST_F(Openings, LH_FA_DOR_004_MeldetNurWallGeometryChanged) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    RecordingListener listener;
    service_.subscribe(listener);

    const auto door = service_.addDoor(*wall, 1500.0);
    ASSERT_TRUE(door.has_value());

    EXPECT_EQ(listener.count(driven::ModelChangeOp::WallGeometryChanged), 1);
    EXPECT_EQ(listener.count(driven::ModelChangeOp::RoomsChanged), 0);
    service_.unsubscribe(listener);
}

// LH-FA-DOR-002: Verschieben folgt; Position auf den gültigen Bereich
// geklemmt; meldet die Wirtswand-Geometrieänderung.
TEST_F(Openings, LH_FA_DOR_002_VerschiebenFolgtUndKlemmt) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const auto door = service_.addDoor(*wall, 500.0);
    ASSERT_TRUE(door.has_value());

    EXPECT_TRUE(service_.moveOpening(*door, 2000.0));
    EXPECT_NEAR(service_.opening(*door).offset_mm, 2000.0, 1e-9);

    // Über das Wandende hinaus → auf [0, 4000-900] geklemmt.
    EXPECT_TRUE(service_.moveOpening(*door, 9000.0));
    EXPECT_NEAR(service_.opening(*door).offset_mm, 3100.0, 1e-9);
    // Unbekannte Öffnung → false.
    EXPECT_FALSE(service_.moveOpening(model::OpeningId{999}, 0.0));
}

// LH-FA-DOR-003: Höhe außerhalb [1800,2500] wird geklemmt.
TEST_F(Openings, LH_FA_DOR_003_HoeheGeklemmt) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const auto door = service_.addDoor(*wall, 500.0);
    ASSERT_TRUE(door.has_value());

    const auto result = service_.setOpeningHeight(*door, 100.0);  // < min
    EXPECT_EQ(result.status, driving::ParamStatus::Clamped);
    EXPECT_NEAR(result.applied_mm, model::kDoorHeightMinMm, 1e-9);
}

// LH-FA-WIN-004: Brüstung eines Fensters setzbar; eine Tür hat keine
// Brüstung (Rejected).
TEST_F(Openings, LH_FA_WIN_004_BruestungNurFuerFenster) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const auto win = service_.addWindow(*wall, 200.0);
    const auto door = service_.addDoor(*wall, 2500.0);
    ASSERT_TRUE(win.has_value() && door.has_value());

    EXPECT_EQ(service_.setWindowSill(*win, 600.0).status,
              driving::ParamStatus::Accepted);
    EXPECT_NEAR(service_.opening(*win).sill_height_mm, 600.0, 1e-9);

    EXPECT_EQ(service_.setWindowSill(*door, 600.0).status,
              driving::ParamStatus::Rejected);  // Türen: keine Brüstung
    EXPECT_NEAR(service_.opening(*door).sill_height_mm, 0.0, 1e-9);
}

// LH-FA-DOR-003: der Anschlag ist eine gespeicherte, abfragbare
// Eigenschaft (welle-2: ohne eigenes Türblatt-Solid).
TEST_F(Openings, LH_FA_DOR_003_AnschlagGesetzt) {
    const auto wall = service_.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const auto door = service_.addDoor(*wall, 500.0);
    ASSERT_TRUE(door.has_value());
    ASSERT_EQ(service_.opening(*door).swing, model::SwingDirection::Left);

    service_.setDoorSwing(*door, model::SwingDirection::Right);
    EXPECT_EQ(service_.opening(*door).swing, model::SwingDirection::Right);
}

// Fehlerfall-Transaktion (W3-P2-Muster): scheitert der Solid-Bau beim
// Platzieren, bleibt das Modell unverändert und es ergeht keine Meldung.
TEST(OpeningsTransaktion, LH_FA_DOR_004_FehlschlagLaesstModellUnveraendert) {
    FailableGeometry geometry;
    services::StructureEditService service(geometry);
    const auto wall = service.addWall(kEg, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    const double before = service.wallSolid(*wall).volume_mm3;
    RecordingListener listener;
    service.subscribe(listener);

    geometry.setFailing(true);
    const auto door = service.addDoor(*wall, 1500.0);

    EXPECT_FALSE(door.has_value());
    EXPECT_TRUE(service.openings().empty());
    EXPECT_NEAR(service.wallSolid(*wall).volume_mm3, before, 1e-9);
    EXPECT_EQ(listener.ops.size(), 0U);  // keine Meldung
    service.unsubscribe(listener);
}

}  // namespace
