#pragma once

#include <algorithm>

#include <QPoint>
#include <QPointF>

#include "hexagon/model/plan_view.h"
#include "hexagon/model/point2d.h"

namespace bcad::adapters::ui::view {

// Reine, display-freie 2D-Sicht-Transformation Bildschirm-Pixel ↔ Modell-mm
// (ADR-0019 Entscheidung 3): Pan (die Modell-mm im Viewport-Zentrum) + Zoom
// (Pixel pro mm). Das Modell-**+y zeigt nach oben** (Bildschirm-+y nach unten →
// y-Spiegelung; wahrt den „kein-Y-Flip"-Präzedenzfall der 2D-Exporte, slice-025b).
// **Invertierbar:** `screenToModel(modelToScreen(p)) == p`. Header-only, **ohne**
// Widget-Lebenszyklus/GL konstruier- und unit-testbar (die Testbarkeits-Naht der
// Interaktions-AK, ADR-0019 Entscheidung 7).
struct ViewTransform {
    double zoom{0.05};        // Pixel pro mm (Default: ~ganzes Haus sichtbar)
    double center_x_mm{0.0};  // Modell-mm im Viewport-Zentrum
    double center_y_mm{0.0};
    int width_px{0};
    int height_px{0};

    QPointF modelToScreen(const hexagon::model::Point2D& m) const {
        const double sx = width_px / 2.0 + (m.x_mm - center_x_mm) * zoom;
        const double sy = height_px / 2.0 - (m.y_mm - center_y_mm) * zoom;
        return QPointF(sx, sy);
    }

    hexagon::model::Point2D screenToModel(const QPoint& p) const {
        const double mx = center_x_mm + (p.x() - width_px / 2.0) / zoom;
        const double my = center_y_mm - (p.y() - height_px / 2.0) / zoom;
        return hexagon::model::Point2D{mx, my};
    }

    // Rahmt die `PlanView`-Bounding-Box mittig in den Viewport (mit Rand). Leeres
    // oder degeneriertes Modell (`has_geometry == false`, Span ≤ 0, Viewport ≤ 0)
    // → Default-Zoom + Ursprung, **kein Div-0** (Totalität — der Aufrufer zeichnet
    // eine leere Seite).
    static ViewTransform fit(const hexagon::model::PlanView& plan, int width_px,
                             int height_px) {
        ViewTransform t;
        t.width_px = width_px;
        t.height_px = height_px;
        const double span_x = plan.max_x_mm - plan.min_x_mm;
        const double span_y = plan.max_y_mm - plan.min_y_mm;
        if (!plan.has_geometry || span_x <= 0.0 || span_y <= 0.0 ||
            width_px <= 0 || height_px <= 0) {
            return t;  // Default-Zoom, Zentrum (0,0)
        }
        t.center_x_mm = (plan.min_x_mm + plan.max_x_mm) / 2.0;
        t.center_y_mm = (plan.min_y_mm + plan.max_y_mm) / 2.0;
        constexpr double kMargin = 0.9;  // 10 % Rand
        t.zoom = std::min(width_px / span_x, height_px / span_y) * kMargin;
        return t;
    }
};

}  // namespace bcad::adapters::ui::view
