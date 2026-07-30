#pragma once

#include <optional>

#include <QPoint>

#include "adapters/ui/view/view_transform.h"
#include "hexagon/model/plan_view.h"
#include "hexagon/model/wall.h"  // WallId

namespace bcad::adapters::ui::view {

// Treffer-Nähe in **Bildschirm-Pixeln** (ADR-0021 E3, slice-059a). Eigene
// Konstante, nicht die des Fangs: dort geht es um Endpunkte, hier um die ganze
// Achse — und eine Trefferfläche darf enger sein als eine Einrast-Nähe. Wie
// dort ein **Bedien-Default, keine Zusage**: die Orakel prüfen Verhalten
// *relativ* zum Schwellwert (innerhalb/außerhalb), nie den px-Wert selbst.
inline constexpr double kPickThresholdPx = 6.0;

// Treffer-Prüfung des 2D-Canvas (ADR-0021 E3/E16): liefert die `WallId` der
// **nächstgelegenen** Wand-Achse, deren Abstand zur Cursor-Position höchstens
// `threshold_px` beträgt — sonst `nullopt`.
//
// **Drei Unterschiede zum Fang** (`snapTarget`), alle aus ADR-0021 E16:
//
//  1. **Abstand zum SEGMENT, nicht zu den Endpunkten.** Eine Wand ist eine
//     Strecke; ein Klick auf ihre Mitte muss sie treffen.
//  2. **Nur das dargestellte Geschoss** (`active_storey_id`). Der Fang ist
//     geschoss-übergreifend, weil er eine Aussage über eine **Koordinate** ist;
//     Auswählen ist eine Aussage über ein **Ding, auf das der Benutzer zeigt**,
//     und er zeigt nur auf Sichtbares. Ohne diese Beschränkung träfe ein Klick
//     bei deckungsgleichen Achsen **deterministisch** die unsichtbare Wand des
//     anderen Geschosses — das mitgelieferte Demo-Modell legt in EG und OG
//     dieselben vier Außenwände an.
//  3. **Nur Wand-Achsen** (`PlanSegmentKind::WallAxis`). Hilfslinien sind
//     Zeichenhilfen, keine Bauteile; ein Segment **ohne** Herkunft ist ebenfalls
//     kein Treffer (die Projektion liefert seit slice-057 immer eine — ein
//     leeres `origin` wäre ein Fehler, kein Wand-Kandidat).
//
// **Auswahl bei mehreren:** der nächstgelegene gewinnt. Bei **exakt gleicher**
// Distanz gewinnt der in der festen `PlanView`-Iterationsreihenfolge **zuerst
// besuchte** — dieselbe Regel wie beim Fang, aus demselben Grund
// (Determinismus statt Zufall; ADR-0021 E3). Die Grenze selbst trifft:
// Distanz **gleich** dem Schwellwert zählt noch als Treffer.
//
// **Das Geschoss kommt als starke Id**, nicht als `int`: `StoreyPlan.storey_id`
// ist ein blanker `int`, und neben einem `double`-Schwellwert wären zwei
// vertauschbare Zahlen-Parameter eine stille Fehlerquelle — der Lint-Gate sagt
// dasselbe. Es ist dieselbe Begründung, mit der die **Rückgabe** eine `WallId`
// ist: sie gilt für Eingaben genauso.
//
// Display-frei und **ohne** `QWidget`/`QApplication` prüfbar (Bauform
// `snapTarget`, ADR-0019 E3/E7).
[[nodiscard]] std::optional<hexagon::model::WallId> pickWall(
    const hexagon::model::PlanView& plan, const ViewTransform& transform,
    QPoint cursor_px, double threshold_px,
    hexagon::model::StoreyId active_storey);

}  // namespace bcad::adapters::ui::view
