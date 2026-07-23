#include "adapters/ui/view/canvas_widget.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>
#include <QWheelEvent>

namespace bcad::adapters::ui::view {

CanvasWidget::CanvasWidget(PlanPull pull, GuideLineDraw draw,
                           int active_storey_id, QWidget* parent)
    : QWidget(parent),
      pull_(std::move(pull)),
      draw_(std::move(draw)),
      active_storey_id_(active_storey_id) {}

void CanvasWidget::onModelChanged(
    const hexagon::ports::driven::ModelChange& /*change*/) {
    // `op`-Mutation (Wände etc.) → neu einrahmen und Repaint einplanen
    // (`update()` ist queued — kein synchrones Rendern im Mutationspfad).
    fitted_ = false;
    update();
}

void CanvasWidget::resizeEvent(QResizeEvent* /*event*/) {
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
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    dragging_ = true;
    drag_start_px_ = event->pos();
    drag_current_px_ = event->pos();
    drag_start_mm_ = transform_.screenToModel(event->pos());  // stabil (LOW-2)
    update();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
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
    const hexagon::model::Point2D end = transform_.screenToModel(event->pos());
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
        update();
    }
    event->accept();
}

}  // namespace bcad::adapters::ui::view
