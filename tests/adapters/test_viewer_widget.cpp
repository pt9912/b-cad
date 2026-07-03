// Widget-Smoke headless (ADR-0009 (f), präzisiert durch ADR-0010): das
// Qt-Fenster rendert den Szenen-Stand ohne physisches Display — belegt
// den Beleg-Pfad (`grabFramebuffer`) und den Callback→Repaint-Weg. Die
// QPA-Plattform wählt der Harness (Dockerfile-Stage: Xvfb +
// Mesa/llvmpipe — die offscreen-QPA trägt kein GL, ADR-0010), nicht
// dieser Test. Die AK-Logik selbst prüft test_viewer_scene.cpp gegen
// das Surrogat.

#include <gtest/gtest.h>

#include <QApplication>
#include <QImage>
#include <QMouseEvent>
#include <QWheelEvent>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/command/view_model_mesh_source.h"
#include "adapters/ui/view/viewer_widget.h"
#include "hexagon/model/segment.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

TEST(ViewerWidgetSmoke, LH_FA_D3_002_HeadlessRenderUndCallbackRepaint) {
    int argc = 1;
    char arg0[] = "bcad_adapter_tests";
    char* argv[] = {static_cast<char*>(arg0), nullptr};
    QApplication app(argc, static_cast<char**>(argv));

    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const auto eg = service.building().storeys.front().id;
    service.addWall(eg, seg(0, 0, 4000, 0));

    bcad::adapters::ui::command::ViewModelMeshSource mesh_source(service);
    bcad::adapters::ui::view::ViewerWidget viewer(mesh_source);
    service.subscribe(viewer);
    viewer.resize(320, 240);
    viewer.show();
    QApplication::processEvents();

    // Mutation über den ADR-0008-Pfad — Szene folgt, Repaint geplant.
    service.addWall(eg, seg(4000, 0, 4000, 3000));
    QApplication::processEvents();
    EXPECT_EQ(viewer.scene().wallMeshes().size(), 2U);

    // Orbit-/Zoom-Pfad (Events statt Interaktion).
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(50, 50),
                      viewer.mapToGlobal(QPointF(50, 50)), Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove, QPointF(80, 60),
                     viewer.mapToGlobal(QPointF(80, 60)), Qt::NoButton,
                     Qt::LeftButton, Qt::NoModifier);
    QWheelEvent wheel(QPointF(50, 50), viewer.mapToGlobal(QPointF(50, 50)),
                      QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier,
                      Qt::NoScrollPhase, false);
    QApplication::sendEvent(&viewer, &press);
    QApplication::sendEvent(&viewer, &move);
    QApplication::sendEvent(&viewer, &wheel);
    QApplication::processEvents();

    // Beleg-Pfad (ACC-002, manueller Abnahme-Schritt nutzt denselben Weg).
    const QImage image = viewer.grabFramebuffer();
    ASSERT_FALSE(image.isNull());
    EXPECT_GE(image.width(), 320);

    // Es wurde tatsächlich Geometrie gerendert: nicht alle Pixel tragen
    // die Hintergrundfarbe.
    const QRgb background = image.pixel(1, 1);
    bool any_foreground = false;
    for (int y = 0; y < image.height() && !any_foreground; ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixel(x, y) != background) {
                any_foreground = true;
                break;
            }
        }
    }
    EXPECT_TRUE(any_foreground) << "Framebuffer enthält nur Hintergrund";

    service.unsubscribe(viewer);
}

}  // namespace
