// Interaktions-AK der 2D-Zeichenfläche (ADR-0019, slice-043) — headless
// (ADR-0010, Xvfb). Zwei Ebenen: (1) die reine `ViewTransform`-Naht
// (`screenToModel`/`modelToScreen`) **ohne** Widget/GL; (2) der Maus-Zug über den
// echten `QMouseEvent`-Pfad → **display-frei** über den Surrogat-Zustand
// (`service.building().guide_lines`) mit den `screenToModel`-gemappten mm — nicht
// über den Framebuffer (Muster `test_viewer_widget.cpp`).

#include <gtest/gtest.h>

#include <QApplication>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/command/edit_drawing_guide_line_sink.h"
#include "adapters/ui/command/plan_view_plan_source.h"
#include "adapters/ui/view/canvas_widget.h"
#include "adapters/ui/view/view_transform.h"
#include "hexagon/model/plan_view.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/segment.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace view = bcad::adapters::ui::view;
namespace command = bcad::adapters::ui::command;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

// (1) Reine Transformations-Naht — invertierbar, +y-oben, kein Div-0. Braucht
// KEIN QApplication/GL (die Testbarkeits-Achse aus ADR-0019 E3/E7).
TEST(CanvasViewTransform, RoundtripPlusBekannteAbbildung) {
    view::ViewTransform t;
    t.zoom = 0.1;  // px pro mm
    t.center_x_mm = 4000.0;
    t.center_y_mm = 3000.0;
    t.width_px = 400;
    t.height_px = 300;

    // Zentrum-mm → Viewport-Mitte.
    const QPointF c = t.modelToScreen({4000.0, 3000.0});
    EXPECT_NEAR(c.x(), 200.0, 1e-9);
    EXPECT_NEAR(c.y(), 150.0, 1e-9);

    // +y-Modell zeigt nach OBEN (kleineres Screen-y als das Zentrum).
    EXPECT_LT(t.modelToScreen({4000.0, 4000.0}).y(), c.y());

    // Roundtrip screenToModel(modelToScreen(p)) ≈ p (Toleranz = 1 Pixel in mm,
    // da modelToScreen sub-pixel liefert, screenToModel ganze Pixel nimmt).
    for (const model::Point2D p : {model::Point2D{0.0, 0.0},
                                   model::Point2D{4000.0, 3000.0},
                                   model::Point2D{8123.0, 6543.0}}) {
        const model::Point2D back = t.screenToModel(t.modelToScreen(p).toPoint());
        EXPECT_NEAR(back.x_mm, p.x_mm, 1.0 / t.zoom);
        EXPECT_NEAR(back.y_mm, p.y_mm, 1.0 / t.zoom);
    }

    // Fit: leeres Modell (has_geometry == false) → Default-Zoom, kein Div-0.
    const view::ViewTransform ft = view::ViewTransform::fit(model::PlanView{}, 400, 300);
    EXPECT_GT(ft.zoom, 0.0);
}

// (2) Der Maus-Zug erzeugt eine Hilfslinie mit den gemappten mm; der entartete
// Zug (Anfang == Ende) erzeugt keine (LH-FA-DRW-005). Ein QApplication für beide
// Fälle (Qt erlaubt nur eine Instanz pro Prozess).
TEST(CanvasWidgetInteraction, LH_FA_DRW_005_MausZugErzeugtHilfslinie) {
    int argc = 1;
    char arg0[] = "bcad_adapter_tests";
    char* argv[] = {static_cast<char*>(arg0), nullptr};
    QApplication app(argc, static_cast<char**>(argv));

    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const auto eg = service.building().storeys.front().id;
    service.addWall(eg, seg(0, 0, 4000, 0));       // Grundriss-Geometrie → BBox
    service.addWall(eg, seg(4000, 0, 4000, 3000));
    model::Layer layer;
    layer.name = "Zeichenebene";
    const auto layer_id = service.addLayer(layer);
    ASSERT_TRUE(layer_id.has_value());

    const command::PlanViewPlanSource plan_source(service);
    const command::EditDrawingGuideLineSink guide_sink(service, eg, *layer_id);
    view::CanvasWidget canvas(
        [&plan_source]() { return plan_source.planView(); },
        [&guide_sink](model::Point2D a, model::Point2D b) {
            return guide_sink.addGuideLine(a, b);
        },
        static_cast<int>(eg));
    service.subscribe(canvas);
    canvas.resize(400, 300);
    canvas.show();
    QApplication::processEvents();  // Paint → Fit-to-Bounds der Transformation

    // --- Happy: Links-Zug A → B legt genau eine Hilfslinie an ---
    const std::size_t before = service.building().guide_lines.size();
    const QPointF a(100, 100);
    const QPointF b(300, 200);
    const model::Point2D expect_start = canvas.screenToModel(a.toPoint());
    const model::Point2D expect_end = canvas.screenToModel(b.toPoint());

    QMouseEvent press(QEvent::MouseButtonPress, a, canvas.mapToGlobal(a),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove, b, canvas.mapToGlobal(b), Qt::NoButton,
                     Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease, b, canvas.mapToGlobal(b),
                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&canvas, &press);
    QApplication::sendEvent(&canvas, &move);
    QApplication::sendEvent(&canvas, &release);
    QApplication::processEvents();

    ASSERT_EQ(service.building().guide_lines.size(), before + 1);
    const model::GuideLine& gl = service.building().guide_lines.back();
    EXPECT_NEAR(gl.segment.start.x_mm, expect_start.x_mm, 1e-6);
    EXPECT_NEAR(gl.segment.start.y_mm, expect_start.y_mm, 1e-6);
    EXPECT_NEAR(gl.segment.end.x_mm, expect_end.x_mm, 1e-6);
    EXPECT_NEAR(gl.segment.end.y_mm, expect_end.y_mm, 1e-6);
    EXPECT_EQ(static_cast<int>(gl.storey_id), static_cast<int>(eg));
    EXPECT_EQ(static_cast<int>(gl.layer_id), static_cast<int>(*layer_id));

    // --- Boundary/Negative: entarteter Zug (Anfang == Ende) → keine Hilfslinie ---
    const std::size_t before_deg = service.building().guide_lines.size();
    const QPointF d(150, 150);
    QMouseEvent press_d(QEvent::MouseButtonPress, d, canvas.mapToGlobal(d),
                        Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release_d(QEvent::MouseButtonRelease, d, canvas.mapToGlobal(d),
                          Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&canvas, &press_d);
    QApplication::sendEvent(&canvas, &release_d);
    QApplication::processEvents();
    EXPECT_EQ(service.building().guide_lines.size(), before_deg);  // unverändert

    service.unsubscribe(canvas);
}

}  // namespace
