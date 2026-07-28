#include "adapters/ui/view/canvas_widget.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QEvent>
#include <QResizeEvent>
#include <QWheelEvent>

#include "adapters/ui/view/snap.h"

namespace bcad::adapters::ui::view {

CanvasWidget::CanvasWidget(PlanPull pull, GuideLineDraw draw,
                           int active_storey_id, QWidget* parent)
    : QWidget(parent),
      pull_(std::move(pull)),
      draw_(std::move(draw)),
      active_storey_id_(active_storey_id) {
    // Ohne Maus-Verfolgung stellt Qt OHNE gedrückte Taste kein Move-Ereignis zu
    // — die Fang-Anzeige (slice-055) wäre im Produkt tot, während ein Test, der
    // Ereignisse synthetisiert, grün bliebe. Deshalb ist die Eigenschaft selbst
    // eine Zusage (§4-5), nicht nur ihre Wirkung.
    setMouseTracking(true);
}

void CanvasWidget::setActiveStorey(int active_storey_id) {
    active_storey_id_ = active_storey_id;
    invalidateSnapPreview();
    // Derselbe Pfad wie im Notify-Callback: neu einrahmen + Repaint einplanen
    // (`update()` ist queued), damit der Wechsel ohne weitere Modell-Meldung
    // sichtbar wird.
    fitted_ = false;
    update();
}

void CanvasWidget::onModelChanged(
    const hexagon::ports::driven::ModelChange& /*change*/) {
    // `op`-Mutation (Wände etc.) → neu einrahmen und Repaint einplanen
    // (`update()` ist queued — kein synchrones Rendern im Mutationspfad).
    invalidateSnapPreview();
    fitted_ = false;
    update();
}

void CanvasWidget::resizeEvent(QResizeEvent* /*event*/) {
    invalidateSnapPreview();
    transform_.width_px = width();
    transform_.height_px = height();
    fitted_ = false;  // beim nächsten Paint neu einrahmen
}

void CanvasWidget::paintEvent(QPaintEvent* /*event*/) {
    const hexagon::model::PlanView plan = pull_();
    if (!fitted_) {
        transform_ = ViewTransform::fit(plan, width(), height());
        fitted_ = true;
    }

    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    // Nur das aktive Geschoss (ADR-0019; StoreyPlan.storey_id ist plain int,
    // StoreyId ist enum class → Cast beim Vergleich).
    painter.setPen(QPen(Qt::black, 1));
    for (const hexagon::model::StoreyPlan& sp : plan.storeys) {
        if (sp.storey_id != active_storey_id_) {
            continue;
        }
        for (const hexagon::model::PlanSegment& s : sp.segments) {
            painter.drawLine(transform_.modelToScreen({s.x1_mm, s.y1_mm}),
                             transform_.modelToScreen({s.x2_mm, s.y2_mm}));
        }
    }

    // Die in-Arbeit-Linie während des Ziehens (nur UI-Feedback).
    if (dragging_) {
        painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter.drawLine(drag_start_px_, drag_current_px_);
    }

    // Fang-Anzeige (LH-FA-DRW-001, slice-055): ein Marker auf dem Punkt, auf den
    // eingerastet WÜRDE — vor dem Klick. Form/Farbe/Größe sind bewusst KEINE
    // Zusage (das Lastenheft sagt nur "erkennbar"); die Tinten-Sonde des Orakels
    // prüft, DASS gezeichnet wird, nicht WIE.
    if (snap_preview_.has_value()) {
        constexpr int kMarkerRadiusPx = 5;
        const QPointF center = transform_.modelToScreen(*snap_preview_);
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, kMarkerRadiusPx, kMarkerRadiusPx);
    }
}

void CanvasWidget::updateSnapPreview(const QPoint& cursor_px) {
    // Außerhalb der Fläche gibt es nichts anzuzeigen. Der Fall ist NICHT von
    // `leaveEvent` gedeckt: bei gedrückter Taste stellt Qt kein `Leave` zu und
    // liefert Move-Ereignisse mit Koordinaten außerhalb `rect()` weiter.
    std::optional<hexagon::model::Point2D> next;
    if (rect().contains(cursor_px)) {
        const hexagon::model::PlanView plan = pull_();
        next = snapTarget(plan, transform_, cursor_px, kSnapThresholdPx);
    }
    if (next == snap_preview_) {
        return;  // nichts zu zeichnen, kein Repaint einplanen
    }
    snap_preview_ = next;
    update();
}

void CanvasWidget::invalidateSnapPreview() {
    // Der Kandidat liegt in mm; ändert sich die Abbildung ohne Zeiger-Ereignis
    // (Zoom/Resize/Geschoss/Modell-Meldung), zeigte er danach auf die falsche
    // Bildschirmstelle. Er wird VERWORFEN statt umgerechnet — die nächste
    // Zeiger-Bewegung baut ihn neu auf. (Die `drag_start_mm_`-Lehre aus slice-043,
    // umgekehrt: dort MUSS ein mm-Wert überleben, hier muss er fallen.)
    if (!snap_preview_.has_value()) {
        return;
    }
    snap_preview_.reset();
    update();
}

void CanvasWidget::leaveEvent(QEvent* event) {
    invalidateSnapPreview();
    QWidget::leaveEvent(event);
}

hexagon::model::Point2D CanvasWidget::snappedModelPos(
    const QPoint& cursor_px) const {
    const hexagon::model::PlanView plan = pull_();
    const std::optional<hexagon::model::Point2D> target =
        snapTarget(plan, transform_, cursor_px, kSnapThresholdPx);
    return target.value_or(transform_.screenToModel(cursor_px));
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    dragging_ = true;
    updateSnapPreview(event->pos());
    drag_start_px_ = event->pos();
    drag_current_px_ = event->pos();
    // Gefangene mm festhalten, nicht das Pixel und nicht den ungefangenen Wert
    // (slice-048b R3): Zoom/Resize mitten im Zug dürfen den Anfang nicht bewegen.
    drag_start_mm_ = snappedModelPos(event->pos());  // stabil (LOW-2)
    update();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    // Die Fang-Anzeige wird in BEIDEN Phasen gepflegt: ohne gedrückte Taste
    // (der Anfang wird gesetzt) und während des Zugs (das Ende wird geführt) —
    // der Fang wirkt an beiden Stellen, also muss die Zusage beide decken.
    updateSnapPreview(event->pos());
    if (!dragging_) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    drag_current_px_ = event->pos();
    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (!dragging_ || event->button() != Qt::LeftButton) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    dragging_ = false;
    const hexagon::model::Point2D start = drag_start_mm_;  // bei Press gemappt
    const hexagon::model::Point2D end = snappedModelPos(event->pos());
    // Der Kern lehnt den entarteten Zug (Anfang == Ende) ab (kein Wert, Modell
    // unverändert) — der Canvas verlässt sich darauf, klemmt nichts selbst
    // (E-VAL-001-Rejection-Lesart). Erfolg wie Ablehnung: Repaint (die
    // in-Arbeit-Linie verschwindet; bei Erfolg erscheint die neue Hilfslinie
    // aus der frisch gepullten `PlanView` = Selbst-Refresh, kein `op`).
    if (draw_) {
        draw_(start, end);
    }
    update();
}

void CanvasWidget::wheelEvent(QWheelEvent* event) {
    const double steps = event->angleDelta().y() / 120.0;
    if (steps != 0.0) {
        // Zoom um den Viewport-Mittelpunkt, geklemmt (px/mm; wie der 3D-Viewer
        // seinen Zoom klemmt — MR-009-LOW-1: verhindert Zoom→0 → absurde mm).
        constexpr double kMinZoom = 1e-4;
        constexpr double kMaxZoom = 100.0;
        transform_.zoom =
            std::clamp(transform_.zoom * std::pow(1.15, steps), kMinZoom, kMaxZoom);
        invalidateSnapPreview();
        update();
    }
    event->accept();
}

}  // namespace bcad::adapters::ui::view
