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

    // === LH-FA-DRW-001 (slice-048b): der Zug fängt wirklich ==================
    // Das ist die ZUSAMMENSPIEL-Ebene: `test_snap.cpp` belegt die Auswahl-
    // Semantik rein, kann aber nicht zeigen, dass der Canvas `snapTarget` an den
    // richtigen Stellen und mit den richtigen Argumenten ruft. Genau dieser
    // Fehler-Typ (Zusage über zwei Komponenten, belegt an einer) ist in welle-5
    // dreimal aufgetreten.
    //
    // Fit-to-Bounds bei 400x300 über die zwei Wände (BBox 0..4000 x 0..3000):
    // zoom = min(400/4000, 300/3000) * 0.9 = 0.09, Zentrum (2000,1500). Damit
    //   (0,0) mm    -> ( 20, 285) px
    //   (4000,0)    -> (380, 285) px
    //   (4000,3000) -> (380,  15) px
    // Alle Cursor-Positionen der Bestands-Fälle oben liegen >= ~70 px von jedem
    // Fang-Punkt entfernt — sie zeichnen weiter frei (Fang-Nähe: 12 px).

    // --- §4-8: der ANFANG wird gefangen (Press-Pfad) ---
    // Press 5,4 px neben der Wand-Ecke (0,0) mm; Release im Freien.
    const std::size_t before_start_snap = service.building().guide_lines.size();
    const QPointF near_origin(25, 283);   // ~5,4 px von (20,285)
    const QPointF free_center(200, 150);  // >= 111 px von jedem Fang-Punkt
    const model::Point2D expect_free_end =
        canvas.screenToModel(free_center.toPoint());
    QMouseEvent press_s(QEvent::MouseButtonPress, near_origin,
                        canvas.mapToGlobal(near_origin), Qt::LeftButton,
                        Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release_s(QEvent::MouseButtonRelease, free_center,
                          canvas.mapToGlobal(free_center), Qt::LeftButton,
                          Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&canvas, &press_s);
    QApplication::sendEvent(&canvas, &release_s);
    QApplication::processEvents();

    ASSERT_EQ(service.building().guide_lines.size(), before_start_snap + 1);
    const model::GuideLine& snapped_start = service.building().guide_lines.back();
    // EXAKT die Wand-Ecke — nicht die (nahen) Cursor-mm. Ohne Fang stünde hier
    // screenToModel(25,283) = (~55,6 mm | ~22,2 mm).
    EXPECT_DOUBLE_EQ(snapped_start.segment.start.x_mm, 0.0);
    EXPECT_DOUBLE_EQ(snapped_start.segment.start.y_mm, 0.0);
    // Das freie Ende bleibt frei (der Fang klemmt nicht alles).
    EXPECT_NEAR(snapped_start.segment.end.x_mm, expect_free_end.x_mm, 1e-6);
    EXPECT_NEAR(snapped_start.segment.end.y_mm, expect_free_end.y_mm, 1e-6);

    // --- §4-7: das ENDE wird gefangen (Release-Pfad) ---
    // Press im Freien, Release 5 px neben der Wand-Ecke (4000,3000) mm.
    const std::size_t before_end_snap = service.building().guide_lines.size();
    const QPointF free_upper(60, 60);      // >= 56 px von jedem Fang-Punkt
    const QPointF near_corner(376, 18);    // 5 px von (380,15)
    const model::Point2D expect_free_start =
        canvas.screenToModel(free_upper.toPoint());
    QMouseEvent press_e(QEvent::MouseButtonPress, free_upper,
                        canvas.mapToGlobal(free_upper), Qt::LeftButton,
                        Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release_e(QEvent::MouseButtonRelease, near_corner,
                          canvas.mapToGlobal(near_corner), Qt::LeftButton,
                          Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&canvas, &press_e);
    QApplication::sendEvent(&canvas, &release_e);
    QApplication::processEvents();

    ASSERT_EQ(service.building().guide_lines.size(), before_end_snap + 1);
    const model::GuideLine& snapped_end = service.building().guide_lines.back();
    EXPECT_NEAR(snapped_end.segment.start.x_mm, expect_free_start.x_mm, 1e-6);
    EXPECT_NEAR(snapped_end.segment.start.y_mm, expect_free_start.y_mm, 1e-6);
    // EXAKT die Wand-Ecke (4000,3000) — nicht screenToModel(376,18).
    EXPECT_DOUBLE_EQ(snapped_end.segment.end.x_mm, 4000.0);
    EXPECT_DOUBLE_EQ(snapped_end.segment.end.y_mm, 3000.0);

    // --- §4-9: Entartung DURCH den Fang → keine Hilfslinie, kein neuer Fehler ---
    // Press und Release liegen auf VERSCHIEDENEN Pixeln, aber beide in Fang-Nähe
    // DESSELBEN Punktes (4000,3000). Ohne Fang entstünde hier eine Hilfslinie;
    // mit Fang trifft der Zug die bestehende Entartungs-Ablehnung des Kerns.
    const std::size_t before_snap_deg = service.building().guide_lines.size();
    const QPointF corner_a(376, 18);
    const QPointF corner_b(384, 12);
    ASSERT_NE(canvas.screenToModel(corner_a.toPoint()).x_mm,
              canvas.screenToModel(corner_b.toPoint()).x_mm)
        << "die zwei Pixel muessen ungefangen VERSCHIEDENE mm ergeben";
    QMouseEvent press_dg(QEvent::MouseButtonPress, corner_a,
                         canvas.mapToGlobal(corner_a), Qt::LeftButton,
                         Qt::LeftButton, Qt::NoModifier);
    QMouseEvent release_dg(QEvent::MouseButtonRelease, corner_b,
                           canvas.mapToGlobal(corner_b), Qt::LeftButton,
                           Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&canvas, &press_dg);
    QApplication::sendEvent(&canvas, &release_dg);
    QApplication::processEvents();
    EXPECT_EQ(service.building().guide_lines.size(), before_snap_deg);

    service.unsubscribe(canvas);
}

}  // namespace
