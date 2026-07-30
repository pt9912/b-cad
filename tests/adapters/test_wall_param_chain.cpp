// slice-059b: die KETTE Fenster → Senke → Dienst → 3D-Sicht.
// Orakel-Zeilen §4-3, §4-4 und die Fenster-Haelfte von §4-9.
//
// Warum eine eigene Datei: §4-3/§4-4 sind die einzigen Zeilen, die den GANZEN
// Weg belegen, und der Plan sagt ausdruecklich, dass ein Beleg **am Dienst**
// nicht genuegt — die Mutation muss von der BEDIENUNG ausgeloest werden
// (Lehre slice-058 §4-10). Dafuer braucht es Fenster, Senke und
// Viewer-Surrogat am DEMSELBEN Dienst.

#include <gtest/gtest.h>

#include <QApplication>
#include <QLineEdit>
#include <QString>

#include <map>
#include <vector>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/command/view_model_mesh_source.h"
#include "adapters/ui/command/wall_param_sink.h"
#include "adapters/ui/view/main_window.h"
#include "adapters/ui/view/viewer_scene.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace command = bcad::adapters::ui::command;
namespace view = bcad::adapters::ui::view;

using view::MainWindow;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

class QtFixture {
public:
    QtFixture() : app_(argc_, static_cast<char**>(argv_)) {}

private:
    int argc_ = 1;
    char arg0_[19] = "bcad_adapter_tests";
    char* argv_[2] = {static_cast<char*>(arg0_), nullptr};
    QApplication app_;
};

QLineEdit* fieldNamed(MainWindow& window, const char* name) {
    return window.findChild<QLineEdit*>(QString::fromLatin1(name));
}

// Der Netz-INHALT der Wand — NICHT die Netz-Anzahl (die bleibt bei einer
// Parameter-Aenderung konstant) und nicht der Update-Zaehler (der bewegt sich
// auch bei einer wirkungslosen Setzung). Plan §2.4.
std::vector<double> meshVertices(const view::ViewerScene& scene,
                                 model::WallId wall) {
    const auto it = scene.wallMeshes().find(wall);
    if (it == scene.wallMeshes().end()) {
        return {};
    }
    return it->second.positions;
}

// §4-3 + §4-4: die Uebernahme im Fenster erreicht das Modell UND die 3D-Sicht.
TEST(WallParamChain, LH_FA_D3_002_UebernahmeImFensterErreichtModellUnd3D) {
    const QtFixture qt;
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const auto storey = service.building().storeys.front().id;
    const auto wall = *service.addWall(storey, seg(0, 0, 4000, 0));

    const command::ViewModelMeshSource mesh_source(service);
    view::ViewerScene scene(mesh_source);
    scene.loadAll();
    service.subscribe(scene);

    const command::WallParamSink sink(service, {});
    MainWindow window(nullptr, {}, {},
                      MainWindow::ParamActions{
                          [&sink, wall](const QString& text) {
                              sink.setThickness(wall, text.toStdString());
                          },
                          [&sink, wall](const QString& text) {
                              sink.setHeight(wall, text.toStdString());
                          }},
                      {});
    window.showWallParams(model::WallParams{
        service.building().walls.front().thickness_mm,
        service.building().walls.front().height_mm});

    const std::size_t meshes_before = scene.wallMeshes().size();
    const std::vector<double> mesh_before = meshVertices(scene, wall);
    ASSERT_FALSE(mesh_before.empty());

    // Die Bedienung: Text ins Feld, Eingabe abschliessen.
    QLineEdit* thickness = fieldNamed(window, MainWindow::kThicknessFieldName);
    ASSERT_NE(thickness, nullptr);
    thickness->setText(QStringLiteral("300"));
    emit thickness->editingFinished();

    EXPECT_DOUBLE_EQ(service.building().walls.front().thickness_mm, 300.0)
        << "die Uebernahme im Fenster muss das Modell erreichen";

    // Die 3D-Sicht folgt — an der richtigen GROESSE gemessen: der Netz-INHALT.
    // Die ANZAHL bleibt konstant und waere in beiden Armen gruen.
    EXPECT_EQ(scene.wallMeshes().size(), meshes_before)
        << "Vorbedingung: die Anzahl bewegt sich NICHT — deshalb ist sie kein "
           "taugliches Orakel";
    EXPECT_NE(meshVertices(scene, wall), mesh_before)
        << "die 3D-Darstellung muss den neuen Stand zeigen, nicht nur eine "
           "Meldung empfangen haben";

    service.unsubscribe(scene);
}

// §4-9 (Fenster-Haelfte): die Hoehe laeuft ueber ihr EIGENES Feld auf ihren
// eigenen Wert — eine Verdrahtungs-Verwechslung waere sonst still.
TEST(WallParamChain, LH_FA_WAL_003_HoehenFeldTrifftDieHoehe) {
    const QtFixture qt;
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const auto storey = service.building().storeys.front().id;
    const auto wall = *service.addWall(storey, seg(0, 0, 4000, 0));
    const double thickness_before =
        service.building().walls.front().thickness_mm;

    const command::WallParamSink sink(service, {});
    MainWindow window(nullptr, {}, {},
                      MainWindow::ParamActions{
                          [&sink, wall](const QString& text) {
                              sink.setThickness(wall, text.toStdString());
                          },
                          [&sink, wall](const QString& text) {
                              sink.setHeight(wall, text.toStdString());
                          }},
                      {});
    window.showWallParams(model::WallParams{thickness_before, 2500.0});

    QLineEdit* height = fieldNamed(window, MainWindow::kHeightFieldName);
    ASSERT_NE(height, nullptr);
    height->setText(QStringLiteral("2600"));
    emit height->editingFinished();

    EXPECT_DOUBLE_EQ(service.building().walls.front().height_mm, 2600.0);
    EXPECT_DOUBLE_EQ(service.building().walls.front().thickness_mm,
                     thickness_before)
        << "das Hoehen-Feld darf die Staerke nicht anfassen";
}

}  // namespace
