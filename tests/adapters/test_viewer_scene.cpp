// AK-Tests Viewer-Szene (sichtbare Hälfte LH-FA-D3-002 + statische
// Darstellung LH-FA-D3-001) — display-frei über das Szenen-Surrogat
// (ADR-0009 (f)): echter Kern + echter OCC-Adapter, geprüft wird der
// Szenen-Endzustand (gehaltene Netze + Zähler wirksamer Updates).

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/viewer_scene.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/segment.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

using bcad::adapters::geometry::OccGeometryAdapter;
using bcad::adapters::ui::ViewerScene;
namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

// Bounding-Box-Ausdehnung eines Netzes entlang einer Achse (0=x,1=y,2=z).
double meshExtent(const model::TriangleMesh& mesh, int axis) {
    double lo = std::numeric_limits<double>::max();
    double hi = std::numeric_limits<double>::lowest();
    for (std::size_t i = static_cast<std::size_t>(axis);
         i < mesh.positions.size(); i += 3) {
        lo = std::min(lo, mesh.positions[i]);
        hi = std::max(hi, mesh.positions[i]);
    }
    return hi - lo;
}

class ViewerSceneAk : public ::testing::Test {
protected:
    OccGeometryAdapter geometry_;
    services::StructureEditService service_{geometry_};
    ViewerScene scene_{service_};
    model::StoreyId eg_{service_.building().storeys.front().id};
};

// Split-Hälfte (i) — statische Darstellung: die Szene zeigt den
// extrudierten Stand aller Wände nach Initial-Laden.
TEST_F(ViewerSceneAk, LH_FA_D3_001_StatischeDarstellungZeigtExtrudiertenStand) {
    service_.addWall(eg_, seg(0, 0, 4000, 0));
    service_.addWall(eg_, seg(4000, 0, 4000, 3000));

    scene_.loadAll();

    ASSERT_EQ(scene_.wallMeshes().size(), 2U);
    for (const auto& [id, mesh] : scene_.wallMeshes()) {
        EXPECT_GT(mesh.triangleCount(), 0) << "Wand ohne Netz";
        // Extrusion in +Z: Netz-Höhe == Wandhöhe (Default = Geschosshöhe).
        EXPECT_NEAR(meshExtent(mesh, 2), model::kDefaultStoreyHeightMm, 0.5);
    }
}

// Happy (ii): committete Parameteränderung → dargestellter Stand folgt
// ohne expliziten Benutzer-/Reload-Schritt (nur über den Callback).
TEST_F(ViewerSceneAk, LH_FA_D3_002_Happy_ParameterAenderungFolgtOhneBenutzerSchritt) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);

    const auto result = service_.setWallThickness(*wall, 800.0);
    ASSERT_EQ(result.status, bcad::hexagon::ports::driving::ParamStatus::Accepted);

    // Kein loadAll(): der Stand kam ausschließlich über Push-Notify+Pull.
    const auto& mesh = scene_.wallMeshes().at(*wall);
    EXPECT_NEAR(meshExtent(mesh, 1), 800.0, 0.5);  // Stärke quer zur Wand (y)
    EXPECT_GE(scene_.effectiveUpdates(), 1);
    service_.unsubscribe(scene_);
}

// Boundary (ii): geklemmte Änderung → dargestellter Stand = Grenzwert;
// die Mehrfach-Meldung einer Mutation (Wand-Op + RoomsChanged) erzeugt
// genau EIN wirksames Szenen-Update (kein Flackern; Welle-1-Lerneintrag).
TEST_F(ViewerSceneAk, LH_FA_D3_002_Boundary_GeklemmtUndGenauEinUpdateJeMutation) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);

    const auto result = service_.setWallThickness(*wall, 5000.0);  // > max
    EXPECT_EQ(result.status, bcad::hexagon::ports::driving::ParamStatus::Clamped);
    EXPECT_NEAR(result.applied_mm, model::kWallThicknessMaxMm, 1e-9);

    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1),
                model::kWallThicknessMaxMm, 0.5);
    EXPECT_EQ(scene_.effectiveUpdates(), 1)
        << "Wand-Op + RoomsChanged dürfen nur ein wirksames Update erzeugen";
    service_.unsubscribe(scene_);
}

// Boundary (ii): identische Mehrfach-Meldung → idempotenter
// Szenen-Endzustand (je Meldung höchstens ein wirksames Update, der
// Stand kippt nicht).
TEST_F(ViewerSceneAk, LH_FA_D3_002_Boundary_MehrfachMeldungIdempotent) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();

    const bcad::hexagon::ports::driven::ModelChange change{
        .op = bcad::hexagon::ports::driven::ModelChangeOp::WallThicknessChanged,
        .storey_id = eg_,
        .wall_id = *wall};
    scene_.onModelChanged(change);
    const double extent_first = meshExtent(scene_.wallMeshes().at(*wall), 1);
    scene_.onModelChanged(change);  // identische Wiederholung

    EXPECT_EQ(scene_.wallMeshes().size(), 1U);
    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1), extent_first, 1e-9);
    EXPECT_EQ(scene_.effectiveUpdates(), 2);  // je Meldung genau eines
}

// Negative (ii): verworfene Mutation (Rejected) → keine Meldung, die
// Darstellung bleibt unverändert.
TEST_F(ViewerSceneAk, LH_FA_D3_002_Negative_RejectedAendertDarstellungNicht) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);
    const double extent_before = meshExtent(scene_.wallMeshes().at(*wall), 1);

    const auto result = service_.setWallThickness(
        *wall, std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(result.status, bcad::hexagon::ports::driving::ParamStatus::Rejected);

    EXPECT_EQ(scene_.effectiveUpdates(), 0);
    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1), extent_before, 1e-9);
    service_.unsubscribe(scene_);
}

// Negative (ii): nach unsubscribe folgt die Darstellung nicht mehr.
TEST_F(ViewerSceneAk, LH_FA_D3_002_Negative_UnsubscribeBeendetFolgen) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);
    service_.unsubscribe(scene_);

    service_.setWallThickness(*wall, 800.0);

    EXPECT_EQ(scene_.effectiveUpdates(), 0);
    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1),
                model::kDefaultWallThicknessMm, 0.5);
}

}  // namespace
