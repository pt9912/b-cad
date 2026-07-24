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

#include <array>
#include <chrono>
#include <ctime>
#include <iostream>
#include <optional>
#include <string>

#include <QApplication>
#include <QImage>
#include <QMainWindow>
#include <QTabWidget>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/geometry/step_export_adapter.h"
#include "adapters/geometry/stl_export_adapter.h"
#include "adapters/io/dxf_export_adapter.h"
#include "adapters/io/dxf_import_adapter.h"
#include "adapters/io/pdf_export_adapter.h"
#include "adapters/io/png_export_adapter.h"
#include "adapters/io/ifc_export_adapter.h"
#include "adapters/io/ifc_import_adapter.h"
#include "adapters/plugin/plugin_host.h"
#include "adapters/ui/command/edit_drawing_guide_line_sink.h"
#include "adapters/ui/command/plan_view_plan_source.h"
#include "adapters/ui/command/view_model_mesh_source.h"
#include "adapters/ui/view/canvas_widget.h"
#include "adapters/ui/view/viewer_widget.h"
#include "hexagon/model/segment.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/services/bootstrap_info.h"
#include "hexagon/services/exchange_service.h"
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
// Gibt die angelegte DRW-Hilfslinien-Ebene zurück (der 2D-Canvas nutzt sie als
// aktive Ebene, slice-043) — `nullopt` nur, wenn die Ebene nicht angelegt werden
// konnte (frischer Service: immer gesetzt).
std::optional<model::LayerId> buildAcc001KernDemo(
    services::StructureEditService& service) {
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

    // DRW (LH-FA-DRW-005/006, slice-032c): eine sichtbare Ebene + eine Hilfslinie
    // auf dem EG — erscheint im 2D-Grundriss-Export (DXF/PDF/PNG) und belegt den
    // Zeichen-Pfad im `make io-smoke`. 2D-export-only (NICHT im 3D-ViewModelPort,
    // ADR-0018 §2) → der ACC-002-Beleg (headless-3D-Render) bleibt unverändert.
    model::Layer drw_layer;
    drw_layer.name = "Hilfslinien";
    const auto layer_id = service.addLayer(drw_layer);
    if (layer_id) {
        model::GuideLine guide;
        guide.storey_id = eg;
        guide.layer_id = *layer_id;
        guide.segment = {{1000.0, 3000.0}, {7000.0, 3000.0}};
        service.addGuideLine(guide);
    }
    return layer_id;
}

// Headless-Export: bei gesetztem `flag <pfad>` das Demo-Modell exportieren und
// einen Exit-Code liefern; sonst `nullopt` (GUI-Pfad). Faltet die je Format
// identische CLI-Mechanik zusammen (hält `main` schlank).
// Export-Herkunft aus der Laufzeit (slice-046): die **einzige** Uhr-/Umgebungs-
// Berührung — der Kern/die Adapter bleiben clock-frei (Determinismus). Datum aus
// der Systemuhr, Version aus `application_banner()`; `source` bleibt leer, bis
// slice-047 ein geladenes Projekt (Basename) liefert.
model::ExportProvenance currentProvenance() {
    const std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::array<char, 32> buf{};
    std::tm tm_buf{};
    if (::localtime_r(&now, &tm_buf) != nullptr) {
        std::strftime(buf.data(), buf.size(), "%Y-%m-%d %H:%M", &tm_buf);
    }
    return {std::string(buf.data()), std::string{},
            bcad::hexagon::services::application_banner()};
}

std::optional<int> runExportIfRequested(
    const QStringList& cli, const char* flag,
    bcad::hexagon::ports::driving::ExchangeModelPort& exchange,
    services::StructureEditService& service,
    bcad::hexagon::ports::driving::ExchangeFormat format, const char* label) {
    const int index = static_cast<int>(cli.indexOf(QString::fromLatin1(flag)));
    if (index < 0 || index + 1 >= cli.size()) {
        return std::nullopt;
    }
    const std::string path = cli.at(index + 1).toStdString();
    buildAcc001KernDemo(service);  // Demo-Modell als Export-Quelle
    try {
        exchange.exportModel(service.building(), path, format, currentProvenance());
        std::cout << label << " exportiert -> " << path << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << label << "-Export fehlgeschlagen: " << e.what() << '\n';
        return 1;
    }
}

// Plugin-Laden beim Start (ADR-0017, slice-026b): jedes `--plugin <pfad>`
// läuft fail-closed durch den Host — Annahme wie Ablehnung sind sichtbar
// (E-PLG-001-Meldung), eine Ablehnung stürzt nicht ab und ändert das
// Modell nicht. Entladen übernimmt der Host beim Beenden (Destruktor).
void loadPluginsFromCli(const QStringList& cli,
                        bcad::adapters::plugin::PluginHost& host) {
    for (int i = 0; i < cli.size(); ++i) {
        if (cli.at(i) != QStringLiteral("--plugin")) {
            continue;
        }
        if (i + 1 >= cli.size()) {
            // Fehlbedienung nicht schweigend schlucken (Review-LOW-2).
            std::cerr << "--plugin ohne Pfad-Argument ignoriert\n";
            break;
        }
        const auto result = host.load(cli.at(i + 1).toStdString());
        (result.ok ? std::cout : std::cerr) << result.message << '\n';
    }
}

}  // namespace

int main(int argc, char** argv) {
    const QApplication app(argc, argv);

    std::cout << bcad::hexagon::services::application_banner() << '\n';

    const bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    // MeshSource-Naht (slice-029): der driving-Pull der ui liegt in
    // ui/command/; deklariert NACH service und VOR window/viewer — das
    // Widget hält eine nicht-besitzende Referenz, die Quelle muss es
    // überleben.
    const bcad::adapters::ui::command::ViewModelMeshSource mesh_source(service);

    // IFC-Austausch-Use-Case verdrahten (ADR-0013, slice-019b/c): Driven-
    // Adapter `IfcImportAdapter`/`IfcExportAdapter` -> Driving-Port-Service
    // `ExchangeService` (Composition Root, ADR-0001). Headless nutzbar über
    // `--import-ifc <pfad>` / `--export-ifc <pfad>`; die GUI-Anbindung folgt
    // mit der IO-Oberfläche.
    bcad::adapters::io::IfcImportAdapter ifc_importer;
    bcad::adapters::io::IfcExportAdapter ifc_exporter;
    bcad::adapters::io::DxfImportAdapter dxf_importer;  // io-resident (ADR-0015)
    bcad::adapters::io::DxfExportAdapter dxf_exporter;  // io-resident (ADR-0015)
    bcad::adapters::geometry::StlExportAdapter stl_exporter(geometry);  // geometrie-resident (ADR-0014)
    bcad::adapters::geometry::StepExportAdapter step_exporter;          // geometrie-resident (ADR-0014)
    bcad::adapters::io::PdfExportAdapter pdf_exporter;  // io-resident, export-only (ADR-0016)
    bcad::adapters::io::PngExportAdapter png_exporter;  // io-resident, export-only (ADR-0016)
    // Symmetrische Importer-/Exporter-Registries (slice-021b): IFC + DXF
    // bidirektional (io-resident), STEP/STL export-only (geometrie-resident),
    // PDF/PNG export-only (io-resident, ADR-0016 — nur in der ExporterMap).
    services::ExchangeService exchange(
        {{bcad::hexagon::ports::driving::ExchangeFormat::Ifc, &ifc_importer},
         {bcad::hexagon::ports::driving::ExchangeFormat::Dxf, &dxf_importer}},
        {{bcad::hexagon::ports::driving::ExchangeFormat::Ifc, &ifc_exporter},
         {bcad::hexagon::ports::driving::ExchangeFormat::Step, &step_exporter},
         {bcad::hexagon::ports::driving::ExchangeFormat::Stl, &stl_exporter},
         {bcad::hexagon::ports::driving::ExchangeFormat::Dxf, &dxf_exporter},
         {bcad::hexagon::ports::driving::ExchangeFormat::Pdf, &pdf_exporter},
         {bcad::hexagon::ports::driving::ExchangeFormat::Png, &png_exporter}});

    // Plugin-Host (ADR-0017): zweiter Driving-Weg in denselben Kern —
    // der Kontext vermittelt EditStructurePort + EvaluatePort (beide vom
    // StructureEditService getragen). Lebt bis Programmende; sein
    // Destruktor entlädt aktive Plugins kontrolliert.
    bcad::adapters::plugin::PluginHost plugin_host(service, service);

    const QStringList cli = QApplication::arguments();
    loadPluginsFromCli(cli, plugin_host);

    const int import_index =
        static_cast<int>(cli.indexOf(QStringLiteral("--import-ifc")));
    if (import_index >= 0 && import_index + 1 < cli.size()) {
        const std::string ifc_path = cli.at(import_index + 1).toStdString();
        try {
            const model::Building imported = exchange.importModel(
                ifc_path, bcad::hexagon::ports::driving::ExchangeFormat::Ifc);
            std::cout << "IFC importiert: " << imported.storeys.size()
                      << " Geschosse, " << imported.walls.size() << " Wände\n";
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "IFC-Import fehlgeschlagen: " << e.what() << '\n';
            return 1;
        }
    }

    // Headless-Import DXF (Parität zu --import-ifc, ADR-0015 — io-resident).
    const int dxf_import_index =
        static_cast<int>(cli.indexOf(QStringLiteral("--import-dxf")));
    if (dxf_import_index >= 0 && dxf_import_index + 1 < cli.size()) {
        const std::string dxf_path = cli.at(dxf_import_index + 1).toStdString();
        try {
            const model::Building imported = exchange.importModel(
                dxf_path, bcad::hexagon::ports::driving::ExchangeFormat::Dxf);
            std::cout << "DXF importiert: " << imported.storeys.size()
                      << " Geschosse, " << imported.walls.size() << " Wände\n";
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "DXF-Import fehlgeschlagen: " << e.what() << '\n';
            return 1;
        }
    }

    // Headless-Export je Format (IFC io-resident, STEP/STL geometrie-resident,
    // DXF/PDF/PNG io-resident) — Tabelle statt wiederholter if-Blöcke.
    using bcad::hexagon::ports::driving::ExchangeFormat;
    struct ExportOption {
        const char* flag;
        ExchangeFormat format;
        const char* label;
    };
    const std::array<ExportOption, 6> export_options = {{
        {"--export-ifc", ExchangeFormat::Ifc, "IFC"},
        {"--export-stl", ExchangeFormat::Stl, "STL"},
        {"--export-step", ExchangeFormat::Step, "STEP"},
        {"--export-dxf", ExchangeFormat::Dxf, "DXF"},
        {"--export-pdf", ExchangeFormat::Pdf, "PDF"},
        {"--export-png", ExchangeFormat::Png, "PNG"},
    }};
    for (const ExportOption& opt : export_options) {
        if (const auto rc = runExportIfRequested(cli, opt.flag, exchange, service,
                                                 opt.format, opt.label)) {
            return *rc;
        }
    }

    // ADR-0009 (e): Konstruktor-Injektion der Driving-Port-Referenz, dann
    // Beobachter-Lebenszyklus. Der 3D-Viewer akkumuliert seinen Szenen-Stand über
    // die ADR-0008-Meldungen → subscribe VOR dem Modell-Aufbau.
    auto* viewer = new bcad::adapters::ui::view::ViewerWidget(mesh_source);
    service.subscribe(*viewer);
    const auto drw_layer = buildAcc001KernDemo(service);  // Meldungen in die Szene

    // 2D-Canvas (ADR-0019, slice-043): Read/Schreib laufen PORT-FREI über
    // ui/command/-Objekte, die der Composition-Root als std::function in den
    // view/-Canvas verdrahtet (Option A — kein command/->view/-Include). Der
    // Canvas PULLT seinen Stand (kein Akkumulieren) → subscribe genügt für
    // spätere op-Mutationen. Aktives Geschoss = EG (front); aktive Ebene = die
    // Demo-Hilfslinien-Ebene (v1 fix; interaktive Auswahl ist ein ADR-0019-Re-Eval).
    const auto active_storey = service.building().storeys.front().id;
    const model::LayerId canvas_layer = [&]() -> model::LayerId {
        if (drw_layer) {
            return *drw_layer;
        }
        model::Layer fallback;
        fallback.name = "Canvas";
        return *service.addLayer(fallback);
    }();
    const bcad::adapters::ui::command::PlanViewPlanSource plan_source(service);
    const bcad::adapters::ui::command::EditDrawingGuideLineSink guide_sink(
        service, active_storey, canvas_layer);
    auto* canvas = new bcad::adapters::ui::view::CanvasWidget(
        [&plan_source]() { return plan_source.planView(); },
        [&guide_sink](model::Point2D a, model::Point2D b) {
            return guide_sink.addGuideLine(a, b);
        },
        static_cast<int>(active_storey));
    service.subscribe(*canvas);

    // Umschalt-Layout 3D↔2D (ADR-0019 E7): der Viewer ist Tab 0 (Default-Sicht),
    // sein GL-Kontext initialisiert beim show() → der ACC-002-Beleg bleibt heil.
    QMainWindow window;
    auto* tabs = new QTabWidget;
    tabs->addTab(viewer, QStringLiteral("3D"));
    tabs->addTab(canvas, QStringLiteral("2D"));
    window.setCentralWidget(tabs);  // Qt übernimmt das Widget-Ownership
    window.resize(1280, 800);
    window.setWindowTitle(QStringLiteral("b-cad"));

    const QStringList args = QApplication::arguments();
    const int beleg_index = static_cast<int>(args.indexOf(
        QStringLiteral("--acc-002-beleg")));
    int result = 0;
    if (beleg_index >= 0 && beleg_index + 1 < args.size()) {
        // Headless-Beleg (ADR-0009 (f)/ADR-0010): Fenster anzeigen
        // (initialisiert den GL-Kontext unter Xvfb), rendern, grabben. Der
        // Viewer muss die aktuelle Tab-Seite sein, sonst initialisiert das
        // QOpenGLWidget beim show() seinen GL-Kontext nicht (Plan-Review-MED-1).
        tabs->setCurrentWidget(viewer);
        window.show();
        QApplication::processEvents();
        const QImage image = viewer->grabFramebuffer();
        const QString& path = args.at(beleg_index + 1);
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

    service.unsubscribe(*canvas);  // vor der Widget-Zerstörung (ADR-0008 #5)
    service.unsubscribe(*viewer);
    return result;
}
