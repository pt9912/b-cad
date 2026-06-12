// Composition Root von b-cad (ADR-0001, ADR-0009 (e)).
//
// Verdrahtet Kern + Adapter: OCC-Geometrie (driven) in den
// `StructureEditService`, den Qt-Viewer (driving) an die Driving Ports
// (`ViewModelPort`) und als Beobachter an den ADR-0008-Vertrag
// (`subscribe` nach Konstruktion, `unsubscribe` vor Zerstörung; main
// besitzt den Viewer, der Service hält nur die nicht-besitzende
// Referenz).
//
// `--acc-002-beleg <pfad.png>`: rendert headless (Xvfb + Mesa/llvmpipe,
// ADR-0010; Aufruf via `make acc-002-beleg`) das ACC-001-Kern-Demo-
// Projekt und schreibt das Beleg-Bild — manueller Abnahme-Schritt,
// kein Gate (ADR-0009 (f)).

#include <iostream>
#include <string>

#include <QApplication>
#include <QImage>
#include <QMainWindow>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/viewer_widget.h"
#include "hexagon/model/segment.h"
#include "hexagon/services/bootstrap_info.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

// ACC-001-Kern-Demo: Einfamilienhaus in Grundzügen — EG (8 m × 6 m,
// Trennwand → zwei Räume, LH-FA-ROM-001) + OG. Eine abschließende
// Parameteränderung lässt den dargestellten Stand der
// Echtzeit-Mechanik folgen (sichtbare Hälfte LH-FA-D3-002).
void buildAcc001KernDemo(services::StructureEditService& service) {
    const auto eg = service.building().storeys.front().id;
    service.addWall(eg, seg(0, 0, 8000, 0));
    service.addWall(eg, seg(8000, 0, 8000, 6000));
    service.addWall(eg, seg(8000, 6000, 0, 6000));
    service.addWall(eg, seg(0, 6000, 0, 0));
    const auto partition = service.addWall(eg, seg(3000, 0, 3000, 6000));

    const auto og = service.addStorey(2700.0);
    service.addWall(og, seg(0, 0, 8000, 0));
    service.addWall(og, seg(8000, 0, 8000, 6000));
    service.addWall(og, seg(8000, 6000, 0, 6000));
    service.addWall(og, seg(0, 6000, 0, 0));

    if (partition.has_value()) {
        service.setWallThickness(*partition, 115.0);  // committete Mutation
    }
}

}  // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    std::cout << bcad::hexagon::services::application_banner() << '\n';

    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);

    // ADR-0009 (e): Konstruktor-Injektion der Driving-Port-Referenz,
    // dann Beobachter-Lebenszyklus.
    QMainWindow window;
    auto* viewer = new bcad::adapters::ui::ViewerWidget(service);
    window.setCentralWidget(viewer);  // Qt übernimmt das Widget-Ownership
    window.resize(1280, 800);
    window.setWindowTitle(QStringLiteral("b-cad"));

    service.subscribe(*viewer);
    buildAcc001KernDemo(service);  // Meldungen laufen bereits in die Szene

    const QStringList args = QApplication::arguments();
    const int beleg_index = static_cast<int>(args.indexOf(
        QStringLiteral("--acc-002-beleg")));
    int result = 0;
    if (beleg_index >= 0 && beleg_index + 1 < args.size()) {
        // Headless-Beleg (ADR-0009 (f)/ADR-0010): Fenster anzeigen
        // (initialisiert den GL-Kontext unter Xvfb), rendern, grabben.
        window.show();
        QApplication::processEvents();
        const QImage image = viewer->grabFramebuffer();
        const QString path = args.at(beleg_index + 1);
        if (image.isNull() || !image.save(path)) {
            std::cerr << "acc-002-beleg: Rendern/Speichern fehlgeschlagen: "
                      << path.toStdString() << '\n';
            result = 1;
        } else {
            std::cout << "acc-002-beleg geschrieben: " << path.toStdString()
                      << " (" << image.width() << "x" << image.height()
                      << ", " << viewer->scene().wallMeshes().size()
                      << " Wand-Netze)\n";
        }
    } else {
        window.show();
        result = QApplication::exec();
    }

    service.unsubscribe(*viewer);  // vor der Widget-Zerstörung (ADR-0008 #5)
    return result;
}
