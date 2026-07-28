// Auswahl-Semantik des Fangens (LH-FA-DRW-001, ADR-0019; slice-048b) — die
// reine, display-freie Funktion `snapTarget`, **ohne** QApplication/Widget/GL
// (Bauform wie `ViewTransform`, ADR-0019 E3/E7). Geprüft wird hier die
// **Auswahl**: exaktes Einrasten, Schwellwert, nächstgelegener Punkt, Tie-Break
// (Segment-Anfang vor Ende, Geschosse in Speicherreihenfolge).
//
// **Was hier NICHT geprüft werden kann** (bewusst benannt, damit die Deckung
// nicht überschätzt wird): (a) dass der Canvas die Funktion überhaupt und an den
// richtigen Stellen ruft — das belegt `test_canvas_widget.cpp` am `CanvasWidget`;
// (b) dass die Iterationsreihenfolge Wand-Achsen **vor** Hilfslinien führt — das
// entsteht in `projectPlan` und wird an `test_plan_projection.cpp` belegt
// (`PlanSegment` trägt keinen Unterscheider Wand↔Hilfslinie).

#include "adapters/ui/view/snap.h"

#include <optional>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <QPoint>

#include "adapters/ui/view/view_transform.h"
#include "hexagon/model/plan_view.h"
#include "hexagon/model/point2d.h"

namespace {

namespace model = bcad::hexagon::model;
namespace view = bcad::adapters::ui::view;

// Zoom 1 px/mm, Zentrum (0,0), 200x200-Viewport → modelToScreen(x,y) =
// (100 + x, 100 - y). Damit ist jede Pixel-Distanz unten von Hand nachrechenbar.
view::ViewTransform unitTransform() {
    view::ViewTransform t;
    t.zoom = 1.0;
    t.center_x_mm = 0.0;
    t.center_y_mm = 0.0;
    t.width_px = 200;
    t.height_px = 200;
    return t;
}

model::PlanView planOf(std::vector<model::StoreyPlan> storeys) {
    model::PlanView plan;
    plan.storeys = std::move(storeys);
    plan.has_geometry = !plan.storeys.empty();
    return plan;
}

model::StoreyPlan storeyOf(int storey_id, std::vector<model::PlanSegment> segs) {
    model::StoreyPlan sp;
    sp.storey_id = storey_id;
    sp.segments = std::move(segs);
    return sp;
}

constexpr double kThreshold = 12.0;

}  // namespace

// §4-1: innerhalb der Fang-Nähe rastet es EXAKT ein — die zurückgegebenen mm
// sind die des Fang-Ziels, nicht die (nahen) Cursor-mm. Distinkte, nicht-runde
// Koordinaten mit x≠y fangen zusätzlich einen x/y- oder start/end-Swap.
TEST(Snap, LH_FA_DRW_001_InnerhalbRastetExaktEin) {
    // Achsen-Endpunkt (10.5, -20.25) mm → Bildschirm (110.5, 120.25).
    const model::PlanView plan = planOf(
        {storeyOf(1, {model::PlanSegment{10.5, -20.25, 50.0, 0.0}})});
    const view::ViewTransform t = unitTransform();

    // Cursor 3,05 px daneben → innerhalb.
    const std::optional<model::Point2D> got =
        view::snapTarget(plan, t, QPoint(113, 122), kThreshold);
    ASSERT_TRUE(got.has_value());
    // GLEICHHEIT, nicht Nähe: die Cursor-mm wären (13, -22).
    EXPECT_DOUBLE_EQ(got->x_mm, 10.5);
    EXPECT_DOUBLE_EQ(got->y_mm, -20.25);
}

// §4-2: außerhalb der Fang-Nähe wird NICHT gefangen (nullopt → der Aufrufer
// zeichnet frei). Die Grenze selbst gehört noch dazu (Distanz == Schwellwert
// rastet ein, Distanz > Schwellwert nicht) — beide Seiten geprüft, sonst wäre
// eine `>=`/`>`-Verwechslung unsichtbar.
TEST(Snap, LH_FA_DRW_001_AusserhalbFaengtNicht) {
    // Endpunkt (0,0) mm → Bildschirm (100,100); zweiter Endpunkt weit weg.
    const model::PlanView plan =
        planOf({storeyOf(1, {model::PlanSegment{0.0, 0.0, 500.0, 0.0}})});
    const view::ViewTransform t = unitTransform();

    EXPECT_FALSE(
        view::snapTarget(plan, t, QPoint(150, 150), kThreshold).has_value());

    // Genau auf der Grenze (12 px) → fängt noch.
    const std::optional<model::Point2D> on_edge =
        view::snapTarget(plan, t, QPoint(112, 100), kThreshold);
    ASSERT_TRUE(on_edge.has_value());
    EXPECT_DOUBLE_EQ(on_edge->x_mm, 0.0);

    // Einen Pixel weiter (13 px) → fängt nicht mehr.
    EXPECT_FALSE(
        view::snapTarget(plan, t, QPoint(113, 100), kThreshold).has_value());
}

// §4-3: liegen MEHRERE Fang-Punkte in Reichweite, gewinnt der nächstgelegene —
// auch wenn er SPÄTER besucht wird ("erster Treffer gewinnt" wäre damit rot).
TEST(Snap, LH_FA_DRW_001_NaechstgelegenerGewinnt) {
    // Zuerst besucht: (-8,0) mm → (92,100), 8 px vom Cursor (100,100).
    // Danach:         (3,0)  mm → (103,100), 3 px — näher.
    const model::PlanView plan = planOf({storeyOf(
        1, {model::PlanSegment{-8.0, 0.0, -300.0, 0.0},
            model::PlanSegment{3.0, 0.0, 300.0, 0.0}})});
    const std::optional<model::Point2D> got = view::snapTarget(
        plan, unitTransform(), QPoint(100, 100), kThreshold);
    ASSERT_TRUE(got.has_value());
    EXPECT_DOUBLE_EQ(got->x_mm, 3.0);
}

// §4-4: bei EXAKT gleicher Distanz gewinnt der zuerst besuchte Punkt — je
// Segment also der ANFANG vor dem Ende. Gegenprobe: `<=` statt `<` beim Minimum
// (oder umgedrehte Iteration) ⇒ das Ende gewönne ⇒ rot.
TEST(Snap, LH_FA_DRW_001_TieBreakAnfangVorEnde) {
    // Anfang (-10,0) → (90,100), Ende (10,0) → (110,100): beide 10 px vom
    // Cursor (100,100) entfernt.
    const model::PlanView plan =
        planOf({storeyOf(1, {model::PlanSegment{-10.0, 0.0, 10.0, 0.0}})});
    const std::optional<model::Point2D> got = view::snapTarget(
        plan, unitTransform(), QPoint(100, 100), kThreshold);
    ASSERT_TRUE(got.has_value());
    EXPECT_DOUBLE_EQ(got->x_mm, -10.0);  // der Anfang, nicht das Ende
}

// §4-5: ALLE Geschosse der PlanView sind Kandidaten (kein Filter auf das
// dargestellte — slice-048b R5/HIGH-1), und bei gleicher Distanz gewinnt das
// FRÜHER gespeicherte Geschoss.
TEST(Snap, LH_FA_DRW_001_AlleGeschosseKandidatenUndReihenfolge) {
    const view::ViewTransform t = unitTransform();

    // (a) Der einzige nahe Punkt liegt im ZWEITEN Geschoss → er muss gewinnen.
    //     Eine auf das erste Geschoss beschränkte Schleife lieferte nullopt.
    const model::PlanView two = planOf(
        {storeyOf(1, {model::PlanSegment{-400.0, 0.0, -300.0, 0.0}}),
         storeyOf(2, {model::PlanSegment{4.0, -3.0, 300.0, 0.0}})});
    const std::optional<model::Point2D> from_second =
        view::snapTarget(two, t, QPoint(104, 103), kThreshold);
    ASSERT_TRUE(from_second.has_value());
    EXPECT_DOUBLE_EQ(from_second->x_mm, 4.0);
    EXPECT_DOUBLE_EQ(from_second->y_mm, -3.0);

    // (b) Gleich weit entfernte Punkte in Geschoss 1 und 2 → Geschoss 1 gewinnt
    //     (Speicherreihenfolge). Umgedrehte Geschoss-Schleife ⇒ rot.
    const model::PlanView tie = planOf(
        {storeyOf(1, {model::PlanSegment{-10.0, 0.0, -400.0, 0.0}}),
         storeyOf(2, {model::PlanSegment{10.0, 0.0, 400.0, 0.0}})});
    const std::optional<model::Point2D> earlier =
        view::snapTarget(tie, t, QPoint(100, 100), kThreshold);
    ASSERT_TRUE(earlier.has_value());
    EXPECT_DOUBLE_EQ(earlier->x_mm, -10.0);
}

// Totalität: leere PlanView (kein Geschoss, kein Segment) → nullopt, kein
// Sonderfall im Aufrufer nötig.
TEST(Snap, LH_FA_DRW_001_LeererPlanFaengtNicht) {
    EXPECT_FALSE(view::snapTarget(model::PlanView{}, unitTransform(),
                                  QPoint(100, 100), kThreshold)
                     .has_value());
}
