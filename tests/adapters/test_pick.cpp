// slice-059a: die Treffer-Pruefung als reine, display-freie Funktion
// (ADR-0021 E3/E16) — Orakel-Zeilen §4-1..4a.
//
// Bauform test_snap.cpp: KEIN QApplication, KEIN Widget, kein GL. Die
// `PlanView` wird von Hand gebaut, die `ViewTransform` gesetzt — damit ist die
// Iterationsreihenfolge vollstaendig steuerbar, und genau darauf kommt es hier
// an: drei der fuenf Zeilen haengen an einer Fixture-Vorbedingung, die der Plan
// ausgeschrieben hat (§4-3, §4-4, §4-4a).

#include <gtest/gtest.h>

#include <QPoint>

#include "adapters/ui/view/pick.h"
#include "adapters/ui/view/view_transform.h"
#include "hexagon/model/plan_view.h"

namespace {

namespace model = bcad::hexagon::model;
namespace view = bcad::adapters::ui::view;

// Eine 1:1-Abbildung mm -> px mit Ursprung in der Viewport-Mitte: zoom 1,0,
// Zentrum (0,0), 400x300. Damit ist ein mm ein Pixel, und die Distanzen der
// Orakel sind ohne Umrechnung lesbar.
view::ViewTransform unitTransform() {
    view::ViewTransform t;
    t.zoom = 1.0;
    t.center_x_mm = 0.0;
    t.center_y_mm = 0.0;
    t.width_px = 400;
    t.height_px = 300;
    return t;
}

model::PlanSegment wallAxis(double x1, double y1, double x2, double y2,
                            int wall_id) {
    model::PlanSegment s;
    s.x1_mm = x1;
    s.y1_mm = y1;
    s.x2_mm = x2;
    s.y2_mm = y2;
    s.origin = model::PlanSegmentOrigin{model::PlanSegmentKind::WallAxis,
                                        wall_id};
    return s;
}

model::PlanSegment guideLine(double x1, double y1, double x2, double y2,
                             int guide_id) {
    model::PlanSegment s;
    s.x1_mm = x1;
    s.y1_mm = y1;
    s.x2_mm = x2;
    s.y2_mm = y2;
    s.origin = model::PlanSegmentOrigin{model::PlanSegmentKind::GuideLine,
                                        guide_id};
    return s;
}

model::PlanView planWith(std::vector<model::StoreyPlan> storeys) {
    model::PlanView plan;
    plan.has_geometry = true;
    plan.storeys = std::move(storeys);
    return plan;
}

// §4-1: rein, display-frei, Abstand zum SEGMENT; die Grenze faengt; leere Sicht
// ⇒ kein Treffer.
TEST(PickWall, ADR_0021_E3_AbstandZumSegmentNichtZumEndpunkt) {
    const view::ViewTransform t = unitTransform();
    // Achse von (0,0) bis (4000,0) — 4 m lang. In Bildschirm-Koordinaten
    // (zoom 1) laeuft sie waagerecht durch die Viewport-Mitte.
    const model::PlanView plan =
        planWith({model::StoreyPlan{1, {wallAxis(0, 0, 4000, 0, 7)}}});

    // Die MITTE der Achse: mit Endpunkt-Abstand (Bauform snap) waere sie
    // 2000 px von beiden Enden entfernt und traefe NICHT.
    const QPoint middle = t.modelToScreen({2000.0, 0.0}).toPoint();
    const auto hit = view::pickWall(plan, t, middle, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1));
    ASSERT_TRUE(hit.has_value()) << "ein Klick auf die Mitte muss treffen";
    EXPECT_EQ(static_cast<int>(*hit), 7);

    // Die GRENZE faengt: Distanz == Schwellwert zaehlt noch als Treffer.
    const QPoint on_threshold =
        middle + QPoint(0, static_cast<int>(view::kPickThresholdPx));
    EXPECT_TRUE(
        view::pickWall(plan, t, on_threshold, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1))
            .has_value());
    const QPoint beyond =
        middle + QPoint(0, static_cast<int>(view::kPickThresholdPx) + 1);
    EXPECT_FALSE(
        view::pickWall(plan, t, beyond, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1)).has_value());

    // Leere Sicht ⇒ kein Treffer (total, kein Wurf).
    EXPECT_FALSE(view::pickWall(model::PlanView{}, t, middle,
                                view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1))
                     .has_value());
}

// §4-2: nur Wand-Achsen sind waehlbar.
TEST(PickWall, ADR_0021_E3_HilfslinienSindNichtWaehlbar) {
    const view::ViewTransform t = unitTransform();
    // Die Hilfslinie liegt GENAU dort, wo geklickt wird; die Wand weit weg.
    const model::PlanView plan = planWith({model::StoreyPlan{
        1,
        {guideLine(0, 0, 4000, 0, 3), wallAxis(0, 1000, 4000, 1000, 7)}}});

    const QPoint on_guide = t.modelToScreen({2000.0, 0.0}).toPoint();
    EXPECT_FALSE(
        view::pickWall(plan, t, on_guide, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1)).has_value())
        << "eine Hilfslinie ist kein Bauteil und darf nicht waehlbar sein";

    // Und ein Segment OHNE Herkunft ebenso wenig (die Projektion liefert seit
    // slice-057 immer eine — ein leeres origin waere ein Fehler).
    model::PlanSegment anonymous;
    anonymous.x1_mm = 0.0;
    anonymous.y1_mm = 0.0;
    anonymous.x2_mm = 4000.0;
    anonymous.y2_mm = 0.0;
    const model::PlanView anonymous_plan =
        planWith({model::StoreyPlan{1, {anonymous}}});
    EXPECT_FALSE(view::pickWall(anonymous_plan, t, on_guide,
                                view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1))
                     .has_value());
}

// §4-3: nur das dargestellte Geschoss — die von ADR-0021 E16 benannte
// symptomlose Fehlerklasse.
//
// DREI Fixture-Vorbedingungen, alle tragend (Plan §4-3):
//   (a) das AKTIVE Geschoss ist das ZWEITE in der Iterationsreihenfolge —
//       bei gleicher Distanz gewinnt sonst der zuerst besuchte, und die
//       Gegenprobe lieferte dieselbe Id wie die korrekte Implementierung;
//   (b) die zwei deckungsgleichen Achsen tragen VERSCHIEDENE Wand-Ids;
//   (c) die Pruefung entscheidet bei gleicher Distanz fuer den zuerst
//       Besuchten (belegt in §4-4a).
TEST(PickWall, ADR_0021_E16_NurDasDargestellteGeschoss) {
    const view::ViewTransform t = unitTransform();
    const model::PlanView plan = planWith({
        model::StoreyPlan{1, {wallAxis(0, 0, 4000, 0, 11)}},   // EG, zuerst
        model::StoreyPlan{2, {wallAxis(0, 0, 4000, 0, 22)}},   // OG, AKTIV
    });

    const QPoint on_axis = t.modelToScreen({2000.0, 0.0}).toPoint();
    const auto hit = view::pickWall(plan, t, on_axis, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(2));
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(static_cast<int>(*hit), 22)
        << "getroffen werden muss die Wand des AKTIVEN Geschosses — ohne "
           "Geschoss-Filter gewaenne die zuerst besuchte (Id 11), also die "
           "unsichtbare";

    // Und im anderen Geschoss trifft derselbe Klick die andere Wand.
    const auto other = view::pickWall(plan, t, on_axis, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1));
    ASSERT_TRUE(other.has_value());
    EXPECT_EQ(static_cast<int>(*other), 11);

    // Ein Geschoss, das es nicht gibt ⇒ kein Treffer.
    EXPECT_FALSE(
        view::pickWall(plan, t, on_axis, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(99)).has_value());
}

// §4-4: hoechstens eine, die naechstgelegene.
// Fixture-Vorbedingung: die NAEHERE Achse kommt in der Iteration SPAETER —
// sonst waere auch eine Reihenfolge-Auswahl zufaellig richtig.
TEST(PickWall, ADR_0021_E3_DieNaechstgelegeneGewinnt) {
    const view::ViewTransform t = unitTransform();
    const model::PlanView plan = planWith({model::StoreyPlan{
        1,
        {wallAxis(0, 5, 4000, 5, 11),      // 5 px entfernt, ZUERST besucht
         wallAxis(0, 1, 4000, 1, 22)}}});  // 1 px entfernt, SPAETER besucht

    const QPoint cursor = t.modelToScreen({2000.0, 0.0}).toPoint();
    const auto hit = view::pickWall(plan, t, cursor, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1));
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(static_cast<int>(*hit), 22)
        << "die naehere Achse gewinnt — nicht die zuerst besuchte";

    // Ausserhalb der Reichweite: kein Treffer (beide Achsen zu weit).
    const QPoint far = t.modelToScreen({2000.0, -100.0}).toPoint();
    EXPECT_FALSE(
        view::pickWall(plan, t, far, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1)).has_value());
}

// §4-4a: bei gleicher Distanz gewinnt der ZUERST Besuchte (ADR-0021 E3,
// wortgleich in der Spezifikation). Ohne diese Zeile haette eine ENTSCHIEDENE
// Regel keinen Sensor — und §4-3 haengt an ihr, ohne sie zu pruefen.
TEST(PickWall, ADR_0021_E3_TieBreakZuerstBesuchterGewinnt) {
    const view::ViewTransform t = unitTransform();
    // Zwei Achsen in EXAKT gleicher Distanz: symmetrisch ober- und unterhalb.
    const model::PlanView plan = planWith({model::StoreyPlan{
        1, {wallAxis(0, 3, 4000, 3, 11), wallAxis(0, -3, 4000, -3, 22)}}});

    const QPoint cursor = t.modelToScreen({2000.0, 0.0}).toPoint();
    const auto hit = view::pickWall(plan, t, cursor, view::kPickThresholdPx,
                     static_cast<model::StoreyId>(1));
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(static_cast<int>(*hit), 11)
        << "bei gleicher Distanz behaelt die ZUERST besuchte Achse den "
           "Zuschlag (strikt <, nicht <=)";
}

}  // namespace
