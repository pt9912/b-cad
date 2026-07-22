// Unit-Tests der geteilten 2D-Plan-Projektion (`plan_geometry`, slice-032c):
// der Sichtbarkeits-Filter (`visibleLayerIds`) + die **Bounding-Box-Erweiterung**
// um sichtbare Hilfslinien (LH-FA-DRW-005/006, ADR-0018). Das ist der von PDF/PNG
// GETEILTE Projektions-Pfad; die format-Decode-Orakel prüfen ihn nur indirekt.
// Hier direkt: (a) eine Hilfslinie AUSSERHALB der Wand-Ausdehnung erweitert die
// BBox (sonst würde sie in PDF/PNG abgeschnitten — Code-Review-MED-1); (b) die
// Koordinaten sind exakt (distinkte Werte fangen einen start/end- oder x/y-Swap
// — Lehre slice-032b-MED-1, Code-Review-LOW-1).

#include "adapters/io/plan_geometry.h"

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/constants.h"

namespace {

namespace model = bcad::hexagon::model;
using bcad::adapters::io::PlanView;
using bcad::adapters::io::projectPlan;
using bcad::adapters::io::visibleLayerIds;

// 1 Geschoss, 1 kurze Wand bei y=0 (x≤1000); 1 Ebene; 1 Hilfslinie WEIT
// AUSSERHALB der Wand-Ausdehnung, mit distinkten Koordinaten (x≠y, start≠end).
model::Building drwBuilding(bool layer_visible) {
    model::Building b;
    b.storeys.push_back(
        model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    model::Wall wall;
    wall.id = model::WallId{1};
    wall.storey_id = model::StoreyId{1};
    wall.start = {0.0, 0.0};
    wall.end = {1000.0, 0.0};
    b.walls.push_back(wall);
    model::Layer layer;
    layer.id = model::LayerId{1};
    layer.name = "Achsen";
    layer.visible = layer_visible;
    b.layers.push_back(layer);
    model::GuideLine guide;
    guide.id = model::GuideLineId{1};
    guide.storey_id = model::StoreyId{1};
    guide.layer_id = model::LayerId{1};
    guide.segment = {{2000.0, 3000.0}, {5000.0, 7000.0}};  // außerhalb der Wand
    b.guide_lines.push_back(guide);
    return b;
}

}  // namespace

// DRW-005: sichtbare Hilfslinie ist im projizierten Grundriss UND die BBox ist um
// ihre Endpunkte erweitert (MED-1: ohne die Erweiterung würde sie abgeschnitten).
TEST(PlanGeometry, LH_FA_DRW_005_SichtbareHilfslinieImPlanUndBBox) {
    const PlanView view = projectPlan(drwBuilding(/*layer_visible=*/true));
    ASSERT_EQ(view.storeys.size(), 1U);
    ASSERT_EQ(view.storeys[0].segments.size(), 2U);  // Wand + Hilfslinie

    // Koordinaten-Treue der Hilfslinie (LOW-1: fängt start/end- + x/y-Swap).
    const auto& guide_seg = view.storeys[0].segments[1];  // nach der Wand
    EXPECT_DOUBLE_EQ(guide_seg.x1_mm, 2000.0);
    EXPECT_DOUBLE_EQ(guide_seg.y1_mm, 3000.0);
    EXPECT_DOUBLE_EQ(guide_seg.x2_mm, 5000.0);
    EXPECT_DOUBLE_EQ(guide_seg.y2_mm, 7000.0);

    // BBox um die Hilfslinien-Endpunkte erweitert (Wand allein: x≤1000, y=0).
    EXPECT_TRUE(view.has_geometry);
    EXPECT_DOUBLE_EQ(view.min_x_mm, 0.0);
    EXPECT_DOUBLE_EQ(view.min_y_mm, 0.0);
    EXPECT_DOUBLE_EQ(view.max_x_mm, 5000.0);
    EXPECT_DOUBLE_EQ(view.max_y_mm, 7000.0);
}

// DRW-005-Negative / DRW-006-Happy: unsichtbare Ebene → Hilfslinie NICHT im Plan,
// BBox bleibt wand-only (kein Leck in die Projektion).
TEST(PlanGeometry, LH_FA_DRW_005_UnsichtbareEbeneGefiltert) {
    const PlanView view = projectPlan(drwBuilding(/*layer_visible=*/false));
    ASSERT_EQ(view.storeys.size(), 1U);
    ASSERT_EQ(view.storeys[0].segments.size(), 1U);  // nur die Wand
    EXPECT_DOUBLE_EQ(view.max_x_mm, 1000.0);  // wand-only
    EXPECT_DOUBLE_EQ(view.max_y_mm, 0.0);
}

// LH-FA-DRW-006: visibleLayerIds trägt nur die sichtbaren Ebenen.
TEST(PlanGeometry, LH_FA_DRW_006_VisibleLayerIdsNurSichtbar) {
    model::Building b = drwBuilding(/*layer_visible=*/true);
    model::Layer hidden;
    hidden.id = model::LayerId{2};
    hidden.name = "Skizze";
    hidden.visible = false;
    b.layers.push_back(hidden);
    const auto visible = visibleLayerIds(b);
    EXPECT_TRUE(visible.contains(1));
    EXPECT_FALSE(visible.contains(2));
}
