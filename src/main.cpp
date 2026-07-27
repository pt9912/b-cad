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
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
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
#include "adapters/persistence/sqlite_project_repository.h"
#include "adapters/plugin/plugin_host.h"
#include "adapters/ui/command/edit_drawing_guide_line_sink.h"
#include "adapters/ui/command/plan_view_plan_source.h"
#include "adapters/ui/command/project_menu_handler.h"
#include "adapters/ui/command/view_model_mesh_source.h"
#include "adapters/ui/view/canvas_widget.h"
#include "adapters/ui/view/main_window.h"
#include "adapters/ui/view/viewer_widget.h"
#include "hexagon/model/segment.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/ports/driving/manage_project_port.h"
#include "hexagon/ports/driving/project_session_port.h"
#include "hexagon/services/bootstrap_info.h"
#include "hexagon/services/exchange_service.h"
#include "hexagon/services/manage_project.h"
#include "hexagon/services/project_session.h"
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
model::ExportProvenance currentProvenance(const std::string& source) {
    const std::time_t now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::array<char, 32> buf{};
    std::tm tm_buf{};
    if (::localtime_r(&now, &tm_buf) != nullptr) {
        std::strftime(buf.data(), buf.size(), "%Y-%m-%d %H:%M", &tm_buf);
    }
    // slice-047a: `source` = Basename der geöffneten Projektdatei (leer ohne --open).
    return {std::string(buf.data()), source,
            bcad::hexagon::services::application_banner()};
}

// slice-047a: exportiert ein **übergebenes** `Building` + `ExportProvenance` (das
// Modell — Demo oder via --open geladen — und die Herkunft bestimmt `main` einmal).
std::optional<int> runExportIfRequested(
    const QStringList& cli, const char* flag,
    bcad::hexagon::ports::driving::ExchangeModelPort& exchange,
    const model::Building& model,
    const model::ExportProvenance& provenance,
    bcad::hexagon::ports::driving::ExchangeFormat format, const char* label) {
    const int index = static_cast<int>(cli.indexOf(QString::fromLatin1(flag)));
    if (index < 0 || index + 1 >= cli.size()) {
        return std::nullopt;
    }
    const std::string path = cli.at(index + 1).toStdString();
    try {
        exchange.exportModel(model, path, format, provenance);
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

// slice-047a: headless CLI-Modus. Verarbeitet Projekt-Persistenz (`--save`/`--open`)
// und Export (`--export-*`) und liefert den Exit-Code; **nullopt** → kein CLI-Op → GUI.
// Ausgelagert aus `main` (Kognitive-Komplexität, lint-Gate). Das via `--open` geladene
// (oder Demo-)`Building` ist die Quelle; sein Basename füllt die Provenance-Quelle.
std::optional<int> runHeadlessCli(
    const QStringList& cli,
    bcad::hexagon::ports::driving::ExchangeModelPort& exchange,
    services::StructureEditService& service) {
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
    const int open_index = static_cast<int>(cli.indexOf(QStringLiteral("--open")));
    const int save_index = static_cast<int>(cli.indexOf(QStringLiteral("--save")));
    const bool have_open = open_index >= 0 && open_index + 1 < cli.size();
    const bool have_save = save_index >= 0 && save_index + 1 < cli.size();
    bool have_export = false;
    for (const ExportOption& opt : export_options) {
        if (cli.indexOf(QString::fromLatin1(opt.flag)) >= 0) {
            have_export = true;
        }
    }
    if (!(have_open || have_save || have_export)) {
        return std::nullopt;  // kein CLI-Op → GUI
    }

    const bcad::adapters::persistence::SqliteProjectRepository repository;
    model::Building work_model;
    std::string source;
    if (have_open) {
        const std::string open_path = cli.at(open_index + 1).toStdString();
        try {
            work_model = repository.load(open_path);
            source = std::filesystem::path(open_path).filename().string();
            std::cout << "Projekt geöffnet <- " << open_path << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Öffnen fehlgeschlagen: " << e.what() << '\n';
            return 1;
        }
    } else {
        buildAcc001KernDemo(service);  // Demo als Export-/Save-Quelle (kein --open)
        work_model = service.building();
    }
    const model::ExportProvenance provenance = currentProvenance(source);

    if (have_save) {
        const std::string save_path = cli.at(save_index + 1).toStdString();
        try {
            bcad::hexagon::services::saveProject(repository, work_model, save_path);
            std::cout << "Projekt gespeichert -> " << save_path << '\n';
        } catch (const std::exception& e) {
            std::cerr << "Speichern fehlgeschlagen: " << e.what() << '\n';
            return 1;
        }
    }

    for (const ExportOption& opt : export_options) {
        if (const auto rc = runExportIfRequested(cli, opt.flag, exchange, work_model,
                                                 provenance, opt.format, opt.label)) {
            return rc;  // optional direkt zurück (kein int-Round-Trip, bugprone-*)
        }
    }
    return 0;  // headless: Persistenz/Export erledigt, keine GUI
}

// Datei-Menue "Oeffnen/Speichern unter" (LH-FA-BLD-002/003, slice-047b,
// ADR-0009). Der modale QFileDialog lebt hier im coverage-ausgenommenen main;
// die eigentliche Arbeit machen die HANDLER (openProject/saveProject), die
// headless getestet sind (Plan-Review MED-1/MED-2).
//
// Die Neu-Aufloesung der beim Demo-Bau EINGEFRORENEN Ids (Plan-Review HIGH-3)
// liegt NICHT mehr hier, sondern in `services::openProject` selbst: als Lambda
// in diesem coverage-ausgenommenen main wurde sie von keinem Sensor ausgefuehrt
// (Verify-Finding B4 — Aufruf entfernt, 280/280 blieben gruen). main verdrahtet
// nur noch die Senken und uebersetzt das Ergebnis in einen Hinweis-Text.
//
// slice-054: das Menue spricht die Use-Cases ueber den `ManageProjectPort` an,
// nicht mehr ueber die freien Service-Funktionen. Es sieht damit weder den
// `StructureEditService` noch den `ProjectRepositoryPort` — genau die Sicht,
// die ein `adapters/ui/command/`-Handler haben wird (slice-053).
// slice-053: `main` baut nur noch die AKTIONEN — was sie tun, liegt im
// `ui/command/`-Handler, wo es gerufen und geprueft werden kann; das Fenster
// ist eine `ui/view/`-Adapter-Klasse. Hier bleiben genau die fuenf benannten
// Grenz-Punkte (Plan §3): die modalen Dialoge, die Meldungstexte, der
// Fenstertitel, die `.bcad`-Suffix-Ergaenzung und der Fenster-Aufbau.
//
// Das Eltern-Widget der Dialoge kommt beim Ausloesen vom Fenster selbst — die
// Aktionen existieren vor ihm (es nimmt sie im Konstruktor).
// slice-052a: Ziel-Dialog + Suffix — beides gehoert zur benannten Grenze
// (Plan §6): Dialog und Meldungstext bleiben hier, die ENTSCHEIDUNG nicht.
std::optional<std::string> askSaveTarget(QWidget* parent) {
    QString path = QFileDialog::getSaveFileName(
        parent, QStringLiteral("Projekt speichern"), QString(),
        QStringLiteral("b-cad-Projekt (*.bcad);;Alle Dateien (*)"));
    if (path.isEmpty()) {
        return std::nullopt;  // abgebrochen
    }
    // Ohne Endung entstuende eine Datei, die der Oeffnen-Dialog mit seinem
    // Vorgabefilter nicht mehr anzeigt (Code-Review LOW-4).
    if (QFileInfo(path).suffix().isEmpty()) {
        path += QStringLiteral(".bcad");
    }
    return path.toStdString();
}

// slice-052a: die Rueckfrage vor Datenverlust. `main` STELLT sie und reicht die
// Antwort zurueck — WAS daraus folgt, wertet der Kern aus (Orakel-Zeile 12).
bcad::hexagon::ports::driving::DiscardAnswer askDiscard(QWidget* parent) {
    using Answer = bcad::hexagon::ports::driving::DiscardAnswer;
    const auto button = QMessageBox::question(
        parent, QStringLiteral("Ungesicherte Aenderungen"),
        QStringLiteral("Das Projekt hat ungesicherte Aenderungen. "
                       "Vor dem Fortfahren speichern?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
    switch (button) {
        case QMessageBox::Save:
            return Answer::Save;
        case QMessageBox::Discard:
            return Answer::Discard;
        default:
            return Answer::Cancel;
    }
}

// slice-052a: `main` reicht dem Handler nur die zwei DIALOGE herein — die
// Komposition (Verdikt holen, Antwort auswerten, ggf. speichern, Fehler
// behandeln) liegt im Handler und ist dort orakel-gedeckt. Laege sie hier,
// waere sie sensorlos; genau diese Klasse Finding hat die Kette erzeugt.
bool mayDiscardSession(QWidget* parent,
                       bcad::adapters::ui::command::ProjectMenuHandler& handler) {
    return handler.mayDiscard(
        [parent]() { return askDiscard(parent); },
        [parent]() -> std::optional<std::filesystem::path> {
            const std::optional<std::string> target = askSaveTarget(parent);
            if (!target) {
                return std::nullopt;
            }
            return std::filesystem::path{*target};
        });
}

// „Speichern": in die gemerkte Datei; ist keine bekannt, wird das Ziel einmalig
// erfragt. Die Ziel-WAHL trifft der Kern (`saveTarget()`), nicht dieser Code.
void saveToKnownOrAsk(QWidget* parent,
                      bcad::adapters::ui::command::ProjectMenuHandler& handler) {
    try {
        if (handler.save()) {
            return;  // in die gemerkte Datei geschrieben
        }
        const std::optional<std::string> target = askSaveTarget(parent);
        if (target) {
            handler.saveAs(*target);
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(parent, QStringLiteral("Speichern fehlgeschlagen"),
                              QString::fromStdString(e.what()));
    }
}

bcad::adapters::ui::view::MainWindow::FileActions makeFileActions(
    bcad::adapters::ui::command::ProjectMenuHandler& handler) {
    // Ein unvollstaendig aufgeloestes Ziel ist kein Fehler, aber der Benutzer
    // muss es erfahren — sonst stuende er vor einer leeren Flaeche bzw. vor
    // einem Zeichnen, das ohne sichtbaren Grund abgelehnt wird (Code-Review
    // LOW-1). Leerer Text = alles aufgeloest.
    const auto hint_for =
        [](bcad::hexagon::ports::driving::DrawingTargetResolution r) -> QString {
        using Res = bcad::hexagon::ports::driving::DrawingTargetResolution;
        switch (r) {
            case Res::NoStorey:
                return QStringLiteral(
                    "Das geoeffnete Projekt enthaelt kein Geschoss — die "
                    "Zeichenflaeche bleibt leer.");
            case Res::NoLayer:
                return QStringLiteral(
                    "Das geoeffnete Projekt enthaelt keine Zeichen-Ebene — "
                    "Hilfslinien koennen erst nach dem Anlegen einer Ebene "
                    "gezeichnet werden.");
            case Res::Resolved:
                break;
        }
        return {};
    };

    bcad::adapters::ui::view::MainWindow::FileActions actions;

    actions.open = [&handler, hint_for](QWidget* parent) {
        // slice-052a: erst fragen, dann verwerfen (LH-FA-BLD-003).
        if (!mayDiscardSession(parent, handler)) {
            return;
        }
        const QString path = QFileDialog::getOpenFileName(
            parent, QStringLiteral("Projekt oeffnen"), QString(),
            QStringLiteral("b-cad-Projekt (*.bcad);;Alle Dateien (*)"));
        if (path.isEmpty()) {
            return;  // abgebrochen — kein Zustandswechsel
        }
        try {
            const QString hint = hint_for(handler.open(path.toStdString()));
            if (parent != nullptr) {
                parent->setWindowTitle(
                    QStringLiteral("b-cad — %1").arg(QFileInfo(path).fileName()));
            }
            if (!hint.isEmpty()) {
                QMessageBox::information(
                    parent, QStringLiteral("Projekt geoeffnet"), hint);
            }
        } catch (const std::exception& e) {
            // Fehler benutzer-sichtbar, kein Crash; das Modell ist
            // unveraendert (openProject laedt erst, ersetzt dann).
            QMessageBox::critical(parent,
                                  QStringLiteral("Oeffnen fehlgeschlagen"),
                                  QString::fromStdString(e.what()));
        }
    };

    // slice-052a: "Speichern" schreibt in die GEMERKTE Datei; ist keine
    // bekannt, wird das Ziel erfragt. Die Ziel-WAHL trifft der Kern
    // (`saveTarget()`), nicht dieser Handler-Aufrufer.
    actions.save = [&handler](QWidget* parent) {
        saveToKnownOrAsk(parent, handler);
    };

    actions.save_as = [&handler](QWidget* parent) {
        const std::optional<std::string> target = askSaveTarget(parent);
        if (!target) {
            return;
        }
        try {
            handler.saveAs(*target);
        } catch (const std::exception& e) {
            QMessageBox::critical(parent,
                                  QStringLiteral("Speichern fehlgeschlagen"),
                                  QString::fromStdString(e.what()));
        }
    };

    return actions;
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

    // slice-047a: headless CLI (--open/--save/--export) — ausgelagert (hält die
    // main-Kognitive-Komplexität unter der lint-Schwelle). nullopt → GUI unten.
    if (const auto rc = runHeadlessCli(cli, exchange, service)) {
        return *rc;
    }

    // Persistenz-Adapter fuer das GUI-Datei-Menue (slice-047b). Die CLI haelt
    // ihre eigene Instanz in runHeadlessCli; der Adapter ist zustandslos.
    const bcad::adapters::persistence::SqliteProjectRepository repository;

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
    // NICHT const: nach einem Projekt-Laden muss das Ziel-Geschoss/die Ebene
    // neu gesetzt werden (slice-047b, s. u.).
    bcad::adapters::ui::command::EditDrawingGuideLineSink guide_sink(
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
    auto* tabs = new QTabWidget;
    tabs->addTab(viewer, QStringLiteral("3D"));
    tabs->addTab(canvas, QStringLiteral("2D"));

    // slice-052a: der Sitzungs-Zustand. Die Baseline ist der Stand NACH dem
    // Start-Aufbau (Orakel-Zeile 14) — mit einem leeren `Building` waere die
    // frische Sitzung sofort "ungesichert" und jede Rueckfrage falsch.
    bcad::hexagon::services::ProjectSessionService session(service.building());

    // slice-054: die Projekt-Use-Cases hinter ihrem Driving Port. Der
    // Composition-Root verdrahtet die Infrastruktur EINMAL hier; der Handler
    // darunter sieht nur noch den Vertrag. Der Use-Case meldet der Sitzung
    // selbst, wenn ein Oeffnen/Speichern GELUNGEN ist.
    bcad::hexagon::services::ManageProjectService manage_project(
        service, repository, &session);

    // slice-053: die Sichten als Senken (port-frei, Muster ADR-0019 Option A).
    // Sie gehoeren jetzt dem HANDLER — er reicht sie bei jedem Oeffnen durch,
    // damit das Zeichen-Ziel nach einem Projekt-Laden auf die Ids des GELADENEN
    // Stands zeigt (slice-047-Verify-B4). Der Port hat dafuer bewusst keinen
    // Default: Vergessen waere ein Compile-Fehler.
    bcad::adapters::ui::command::ProjectMenuHandler project_handler(
        manage_project, session,
        [&service]() -> const model::Building& { return service.building(); },
        bcad::hexagon::ports::driving::DrawingTargetSinks{
            [canvas](model::StoreyId storey) {
                canvas->setActiveStorey(static_cast<int>(storey));
            },
            [&guide_sink](model::StoreyId storey, model::LayerId layer) {
                guide_sink.setTarget(storey, layer);
            },
        });

    // slice-053: das Fenster ist eine Adapter-Klasse. `main` uebergibt ihm die
    // fertigen Tabs und die Aktionen; beim Ausloesen reicht es sich selbst als
    // Eltern-Widget der Dialoge durch.
    // slice-052a: die von slice-053 gelieferte, bis hierher UNBESETZTE
    // CloseGuard-Naht wird besetzt — Rueckfrage vor Datenverlust beim
    // Fenster-Schliessen (Orakel-Zeile 13).
    // Der Waechter braucht ein Eltern-Fenster fuer seinen Dialog, existiert aber
    // vor dem Fenster (es nimmt ihn im Konstruktor) — deshalb der nachgereichte
    // Zeiger. Die CloseGuard-SIGNATUR bleibt unveraendert: slice-052a besetzt
    // eine vorhandene Naht, es baut die Fenster-Klasse nicht um.
    QWidget* close_dialog_parent = nullptr;
    bcad::adapters::ui::view::MainWindow window(
        tabs, makeFileActions(project_handler),
        [&close_dialog_parent, &project_handler]() {
            return mayDiscardSession(close_dialog_parent, project_handler);
        });
    close_dialog_parent = &window;
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
