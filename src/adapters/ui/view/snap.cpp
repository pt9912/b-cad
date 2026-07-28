// Eingabe-Quantisierung des 2D-Canvas (LH-FA-DRW-001, slice-048b): reine
// Auswahl-Funktion, display-frei. Sie **kopiert** die mm des Fang-Ziels aus der
// `PlanView` unverändert — keine Rundung, keine Umrechnung; nur so trägt die
// erzeugte Hilfslinie **exakt** die Position des Ziels (und überlebt sie den
// Round-Trip identisch).

#include "adapters/ui/view/snap.h"

#include <array>
#include <cmath>

#include <QPointF>

namespace bcad::adapters::ui::view {
namespace {

// Bildschirm-Distanz Cursor ↔ Modell-Punkt (der Fang misst in **px**, nicht in
// mm — sonst wäre er zoom-abhängig unbrauchbar, slice-048b R2).
double distancePx(const ViewTransform& transform,
                  const hexagon::model::Point2D& point, QPoint cursor_px) {
    const QPointF screen = transform.modelToScreen(point);
    return std::hypot(screen.x() - cursor_px.x(), screen.y() - cursor_px.y());
}

}  // namespace

std::optional<hexagon::model::Point2D> snapTarget(
    const hexagon::model::PlanView& plan, const ViewTransform& transform,
    QPoint cursor_px, double threshold_px) {
    std::optional<hexagon::model::Point2D> best;
    double best_distance = 0.0;

    for (const hexagon::model::StoreyPlan& storey : plan.storeys) {
        for (const hexagon::model::PlanSegment& segment : storey.segments) {
            // Anfang VOR Ende — der dritte Tie-Break-Konjunkt der Spezifikation.
            const std::array<hexagon::model::Point2D, 2> endpoints{
                hexagon::model::Point2D{segment.x1_mm, segment.y1_mm},
                hexagon::model::Point2D{segment.x2_mm, segment.y2_mm}};
            for (const hexagon::model::Point2D& candidate : endpoints) {
                const double distance =
                    distancePx(transform, candidate, cursor_px);
                if (distance > threshold_px) {
                    continue;  // außerhalb der Fang-Nähe: kein Kandidat
                }
                // STRIKT kleiner: bei gleicher Distanz behält der zuerst
                // besuchte Punkt den Zuschlag (Tie-Break der Spezifikation).
                if (!best.has_value() || distance < best_distance) {
                    best = candidate;
                    best_distance = distance;
                }
            }
        }
    }
    return best;
}

}  // namespace bcad::adapters::ui::view
