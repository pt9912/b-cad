// Unit-Tests der geteilten 2D-Plan-Projektion (`services::projectPlan`, seit
// slice-042b im Kern; ADR-0019/ADR-0020): der Sichtbarkeits-Filter
// (`model::visibleLayerIds`) + die **Bounding-Box-Erweiterung** um sichtbare
// Hilfslinien (LH-FA-DRW-005/006, ADR-0018). Das ist die von PDF/PNG GETEILTE
// Kern-Projektion (über das `DerivedGeometry`-Bündel); die format-Decode-Orakel
// prüfen sie nur indirekt. Hier direkt: (a) eine Hilfslinie AUSSERHALB der
// Wand-Ausdehnung erweitert die BBox (sonst würde sie in PDF/PNG abgeschnitten —
// Code-Review-MED-1); (b) die Koordinaten sind exakt (distinkte Werte fangen einen
// start/end- oder x/y-Swap — Lehre slice-032b-MED-1, Code-Review-LOW-1).

#include "hexagon/services/geometry/plan_projection.h"

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/layer_visibility.h"
#include "hexagon/model/plan_view.h"

namespace {

namespace model = bcad::hexagon::model;
using bcad::hexagon::model::PlanView;
using bcad::hexagon::model::visibleLayerIds;
using bcad::hexagon::services::projectPlan;

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

// --- slice-057 (ADR-0021 E11): Herkunft je Segment ---------------------------
// Die Projektion mischt Wand-Achsen und sichtbare Hilfslinien in EINE Liste;
// ohne Herkunft haette eine Treffer-Pruefung nur anonyme Koordinaten. Gebaut
// wird das Building HIER von Hand — nur so lassen sich Ids vergeben, die NICHT
// der Reihenfolge entsprechen (der Service vergibt sie fortlaufend).
model::Building originBuilding() {
    model::Building b;
    b.storeys.push_back(
        model::Storey{model::StoreyId{1}, model::kDefaultStoreyHeightMm});
    // Ids bewusst NICHT 1,2 und NICHT aufsteigend: ein Laufindex statt der
    // echten Id faellt damit auf (Orakel 3).
    model::Wall w1;
    w1.id = model::WallId{7};
    w1.storey_id = model::StoreyId{1};
    w1.start = {0.0, 0.0};
    w1.end = {1000.0, 0.0};
    b.walls.push_back(w1);
    model::Wall w2 = w1;
    w2.id = model::WallId{3};
    w2.start = {1000.0, 0.0};
    w2.end = {1000.0, 500.0};
    b.walls.push_back(w2);
    model::Layer layer;
    layer.id = model::LayerId{1};
    layer.name = "Achsen";
    layer.visible = true;
    b.layers.push_back(layer);
    model::GuideLine g;
    g.id = model::GuideLineId{42};
    g.storey_id = model::StoreyId{1};
    g.layer_id = model::LayerId{1};
    g.segment = {{2000.0, 3000.0}, {5000.0, 7000.0}};
    b.guide_lines.push_back(g);
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

// LH-FA-DRW-001 (slice-048b): je Geschoss stehen die WAND-ACHSEN VOR den
// Hilfslinien. Das ist der zweite Konjunkt des Fang-Tie-Breaks — und er ist NUR
// hier belegbar: `PlanSegment` trägt keinen Unterscheider Wand↔Hilfslinie, die
// Fang-Auswahl (`snapTarget`) sieht eine flache Liste und kann allein
// "zuerst besucht gewinnt" zusichern. DASS diese Reihenfolge Wand-Achsen zuerst
// führt, entsteht in `projectPlan`. Mehrere Wände UND mehrere Hilfslinien,
// damit ein blosses Vertauschen einzelner Elemente nicht durchrutscht.
TEST(PlanGeometry, LH_FA_DRW_001_WandAchsenVorHilfslinien) {
    model::Building b = drwBuilding(/*layer_visible=*/true);
    model::Wall second_wall;
    second_wall.id = model::WallId{2};
    second_wall.storey_id = model::StoreyId{1};
    second_wall.start = {1000.0, 0.0};
    second_wall.end = {1000.0, 500.0};
    b.walls.push_back(second_wall);
    model::GuideLine second_guide;
    second_guide.id = model::GuideLineId{2};
    second_guide.storey_id = model::StoreyId{1};
    second_guide.layer_id = model::LayerId{1};
    second_guide.segment = {{8000.0, 9000.0}, {8500.0, 9500.0}};
    b.guide_lines.push_back(second_guide);

    const PlanView view = projectPlan(b);
    ASSERT_EQ(view.storeys.size(), 1U);
    const auto& segs = view.storeys[0].segments;
    ASSERT_EQ(segs.size(), 4U);  // 2 Wände + 2 Hilfslinien

    // [0],[1] sind die Wände (Modell-Reihenfolge), [2],[3] die Hilfslinien.
    EXPECT_DOUBLE_EQ(segs[0].x2_mm, 1000.0);  // Wand 1: (0,0)->(1000,0)
    EXPECT_DOUBLE_EQ(segs[0].y2_mm, 0.0);
    EXPECT_DOUBLE_EQ(segs[1].x1_mm, 1000.0);  // Wand 2: (1000,0)->(1000,500)
    EXPECT_DOUBLE_EQ(segs[1].y2_mm, 500.0);
    EXPECT_DOUBLE_EQ(segs[2].x1_mm, 2000.0);  // Hilfslinie 1
    EXPECT_DOUBLE_EQ(segs[2].y1_mm, 3000.0);
    EXPECT_DOUBLE_EQ(segs[3].x1_mm, 8000.0);  // Hilfslinie 2
    EXPECT_DOUBLE_EQ(segs[3].y1_mm, 9000.0);
}

// Orakel 1+3: richtige ART je Segment, und die ECHTE Id — nicht der Laufindex.
TEST(PlanGeometry, ADR0021_HerkunftTraegtArtUndEchteId) {
    const PlanView view = projectPlan(originBuilding());
    ASSERT_EQ(view.storeys.size(), 1U);
    const auto& segs = view.storeys[0].segments;
    ASSERT_EQ(segs.size(), 3U);  // 2 Waende + 1 Hilfslinie

    ASSERT_TRUE(segs[0].origin.has_value());
    EXPECT_EQ(segs[0].origin->kind, model::PlanSegmentKind::WallAxis);
    EXPECT_EQ(segs[0].origin->id, 7);   // Laufindex waere 0
    ASSERT_TRUE(segs[1].origin.has_value());
    EXPECT_EQ(segs[1].origin->kind, model::PlanSegmentKind::WallAxis);
    EXPECT_EQ(segs[1].origin->id, 3);   // Laufindex waere 1; auch nicht sortiert
    ASSERT_TRUE(segs[2].origin.has_value());
    EXPECT_EQ(segs[2].origin->kind, model::PlanSegmentKind::GuideLine);
    EXPECT_EQ(segs[2].origin->id, 42);  // Laufindex waere 2
}

// Orakel 2: die Projektion liefert NIE ein Segment ohne Herkunft. Das ist die
// Zusage, die das `optional` ueberhaupt erst rechtfertigt (slice-057 §2.1):
// ein vergessenes Feld ist LEER, und Leere ist pruefbar — als Wert waere es
// still als Wand-Achse beschriftet gewesen.
TEST(PlanGeometry, ADR0021_KeinSegmentOhneHerkunft) {
    for (const model::Building& b :
         {originBuilding(), drwBuilding(true), drwBuilding(false)}) {
        const PlanView view = projectPlan(b);
        for (const auto& storey : view.storeys) {
            for (const auto& seg : storey.segments) {
                EXPECT_TRUE(seg.origin.has_value())
                    << "Segment ohne Herkunft in Geschoss " << storey.storey_id;
            }
        }
    }
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
