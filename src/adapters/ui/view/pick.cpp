// Treffer-Prüfung des 2D-Canvas (ADR-0021 E3/E16, slice-059a): reine
// Auswahl-Funktion, display-frei. Sie misst in **Bildschirm-Pixeln** — wie der
// Fang und aus demselben Grund: eine mm-Toleranz wäre herausgezoomt unbrauchbar
// groß und hineingezoomt unbrauchbar klein.

#include "adapters/ui/view/pick.h"

#include <algorithm>
#include <cmath>

#include <QPointF>

namespace bcad::adapters::ui::view {
namespace {

// Bildschirm-Distanz Cursor ↔ **Strecke** (nicht Endpunkt): Projektion des
// Cursors auf das Segment, geklemmt auf [0,1] — außerhalb der Strecke ist der
// nächste Punkt ihr Endpunkt. Gerechnet wird in Bildschirm-Koordinaten, damit
// die Toleranz zoom-unabhängig bleibt.
double distanceToSegmentPx(const QPointF& a, const QPointF& b, QPoint cursor) {
    const double dx = b.x() - a.x();
    const double dy = b.y() - a.y();
    const double length_squared = (dx * dx) + (dy * dy);
    if (length_squared <= 0.0) {
        // Entartetes Segment (beide Enden auf demselben Pixel): Punkt-Abstand.
        return std::hypot(cursor.x() - a.x(), cursor.y() - a.y());
    }
    const double t = std::clamp(
        (((cursor.x() - a.x()) * dx) + ((cursor.y() - a.y()) * dy)) /
            length_squared,
        0.0, 1.0);
    const double closest_x = a.x() + (t * dx);
    const double closest_y = a.y() + (t * dy);
    return std::hypot(cursor.x() - closest_x, cursor.y() - closest_y);
}

}  // namespace

std::optional<hexagon::model::WallId> pickWall(
    const hexagon::model::PlanView& plan, const ViewTransform& transform,
    QPoint cursor_px, double threshold_px,
    hexagon::model::StoreyId active_storey) {
    std::optional<hexagon::model::WallId> best;
    double best_distance = 0.0;

    for (const hexagon::model::StoreyPlan& storey : plan.storeys) {
        if (storey.storey_id != static_cast<int>(active_storey)) {
            continue;  // ADR-0021 E16: nur das DARGESTELLTE Geschoss
        }
        for (const hexagon::model::PlanSegment& segment : storey.segments) {
            if (!segment.origin.has_value() ||
                segment.origin->kind !=
                    hexagon::model::PlanSegmentKind::WallAxis) {
                continue;  // Hilfslinien (und herkunftslose Segmente) sind
                           // keine Bauteile und damit nicht wählbar
            }
            const double distance = distanceToSegmentPx(
                transform.modelToScreen({segment.x1_mm, segment.y1_mm}),
                transform.modelToScreen({segment.x2_mm, segment.y2_mm}),
                cursor_px);
            if (distance > threshold_px) {
                continue;  // außerhalb der Treffer-Nähe
            }
            // STRIKT kleiner: bei gleicher Distanz behält die zuerst besuchte
            // Achse den Zuschlag (Tie-Break der Spezifikation, ADR-0021 E3).
            if (!best.has_value() || distance < best_distance) {
                best = static_cast<hexagon::model::WallId>(segment.origin->id);
                best_distance = distance;
            }
        }
    }
    return best;
}

}  // namespace bcad::adapters::ui::view
