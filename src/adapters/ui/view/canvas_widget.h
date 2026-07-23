#pragma once

#include <functional>
#include <optional>

#include <QPoint>
#include <QWidget>

#include "adapters/ui/view/view_transform.h"
#include "hexagon/model/guide_line.h"  // GuideLineId
#include "hexagon/model/plan_view.h"
#include "hexagon/model/point2d.h"
#include "hexagon/ports/driven/model_changed_port.h"

class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QWheelEvent;

namespace bcad::adapters::ui::view {

// Interaktive 2D-Zeichenfläche (ADR-0019): ein eigenes `view/`-`QWidget`
// (**nicht** der orbit-verdrahtete 3D-`ViewerWidget`), das die 2D-Grundriss-
// Projektion (`PlanView`) des **aktiven Geschosses** zeichnet und Hilfslinien
// per **Links-Zug** ziehen lässt (Press = Anfang, Release = Ende, gemappte mm).
//
// **Port-frei (Plan-Review-Entscheidung Option A):** das Widget hält **keinen**
// Driving-Port und **keinen** `view/`-Sink-Header, sondern
// zwei `std::function`-Callables (Read-Pull `PlanView`, Schreib `addGuideLine`),
// die der Composition-Root aus `ui/command/`-Objekten verdrahtet — **kein**
// `command/ → view/`-Include, der Regel-B-`adapter_sink` bleibt unverändert.
//
// **Refresh:** Beobachter des `ModelChangedPort` (Repaint + Neu-Einrahmen nach
// `op`-Mutationen, z. B. Wand-Änderung); nach dem **eigenen** erfolgreichen
// `addGuideLine` repaintet er **selbst** (Selbst-Refresh, **kein** `op` —
// ADR-0018 §2 „kein op" bleibt unrevidiert). **v1 zeichnet frei** (kein
// Fang/Raster/Winkel — eigene spätere Slices).
class CanvasWidget final : public QWidget,
                           public hexagon::ports::driven::ModelChangedPort {
public:
    using PlanPull = std::function<hexagon::model::PlanView()>;
    using GuideLineDraw =
        std::function<std::optional<hexagon::model::GuideLineId>(
            hexagon::model::Point2D, hexagon::model::Point2D)>;

    CanvasWidget(PlanPull pull, GuideLineDraw draw, int active_storey_id,
                 QWidget* parent = nullptr);

    // ADR-0008-Callback: `op`-Mutation → neu einrahmen + Repaint einplanen.
    void onModelChanged(
        const hexagon::ports::driven::ModelChange& change) override;

    // Testbare, display-freie Transformations-Naht (ADR-0019 E3/E7).
    hexagon::model::Point2D screenToModel(const QPoint& p) const {
        return transform_.screenToModel(p);
    }
    const ViewTransform& transform() const { return transform_; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    PlanPull pull_;
    GuideLineDraw draw_;
    int active_storey_id_{};
    ViewTransform transform_{};
    bool fitted_{false};  // Fit-to-Bounds beim nächsten Paint nötig?
    bool dragging_{false};
    QPoint drag_start_px_{};   // nur für die visuelle in-Arbeit-Linie
    QPoint drag_current_px_{};
    // Der Zug-Startpunkt in Modell-mm (bei Press gemappt) — überlebt eine
    // Transformations-Änderung (Zoom/Resize) mitten im Zug; die committete
    // Hilfslinie nutzt IHN, nicht die Neu-Abbildung des alten Pixels (MR-009-LOW-2).
    hexagon::model::Point2D drag_start_mm_{};
};

}  // namespace bcad::adapters::ui::view
