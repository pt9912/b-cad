#pragma once

#include <optional>

#include <QPoint>

#include "adapters/ui/view/view_transform.h"
#include "hexagon/model/plan_view.h"
#include "hexagon/model/point2d.h"

namespace bcad::adapters::ui::view {

// Fang-Nähe in **Bildschirm-Pixeln** (LH-FA-DRW-001, slice-048b). Bewusst eine
// Widget-Konstante und **keine** §3-Bauteil-Konstante: der Fang wirkt im
// Bildschirmraum, nicht in mm — in mm wäre er bei herausgezoomter Ansicht
// unbrauchbar groß und bei hineingezoomter unbrauchbar klein. Der Wert ist ein
// **Bedien-Default, keine Zusage**: die Orakel prüfen Verhalten *relativ* zum
// Schwellwert (innerhalb/außerhalb), nie den px-Wert selbst.
inline constexpr double kSnapThresholdPx = 12.0;

// Eingabe-Quantisierung des 2D-Canvas (LH-FA-DRW-001): liefert die **exakten
// Modell-mm** des nächstgelegenen Fang-Punktes, wenn der Cursor innerhalb von
// `threshold_px` Bildschirm-Pixeln um ihn liegt — sonst `nullopt` (dann zeichnet
// der Aufrufer frei weiter, LH-FA-DRW-005).
//
// **Fang-Punkte** sind die Endpunkte **aller** Segmente der `PlanView` — also der
// Wand-Achsen **und** der Hilfslinien auf **sichtbarer** Ebene. Dass unsichtbare
// Ebenen nicht fangbar sind, ist damit **strukturell erzwungen** und keine Zusage
// dieser Funktion: `projectPlan` filtert sie **vor** der `PlanView` heraus.
//
// **Auswahl bei mehreren:** der nächstgelegene gewinnt. Bei **exakt gleicher**
// Distanz gewinnt der in der festen `PlanView`-Iterationsreihenfolge **zuerst
// besuchte** Punkt (Geschosse in Speicherreihenfolge · je Geschoss Wand-Achsen vor
// Hilfslinien — das entsteht in `projectPlan`, nicht hier · je Segment Anfang vor
// Ende). Die Grenze selbst fängt: Distanz **gleich** dem Schwellwert rastet noch ein.
//
// Display-frei und **ohne** `QWidget`/`QApplication` prüfbar (Bauform wie
// `ViewTransform`, ADR-0019 E3/E7). **Kein Geschoss-Parameter:** die Spezifikation
// definiert die Fang-Punkte über die **ganze** `PlanView` (Tie-Break-Konjunkt
// „Geschosse in Speicherreihenfolge"). Der Fang reicht damit über das dargestellte
// Geschoss hinaus — benannte Grenze (slice-048b R5), kein Fehler.
[[nodiscard]] std::optional<hexagon::model::Point2D> snapTarget(
    const hexagon::model::PlanView& plan, const ViewTransform& transform,
    QPoint cursor_px, double threshold_px);

}  // namespace bcad::adapters::ui::view
