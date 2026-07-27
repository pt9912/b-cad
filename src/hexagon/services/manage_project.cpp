#include "hexagon/services/manage_project.h"

#include <stdexcept>
#include <utility>

#include "hexagon/model/persisted_derivations.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/storey_query.h"
#include "hexagon/services/geometry/stair_geometry.h"

namespace bcad::hexagon::services {

void saveProject(const ports::driven::ProjectRepositoryPort& repository,
                 const model::Building& building,
                 const std::filesystem::path& path) {
    model::PersistedDerivations derived;
    for (const model::Stair& stair : building.stairs) {
        // Pro Treppe wird die Stufenhöhe ("rise") mitgespeichert. Sie ergibt sich aus
        // der Höhe des Geschosses, VON dem die Treppe ausgeht (`from_storey_id`).
        // Referenziert die Treppe ein Geschoss, das im Modell nicht existiert, lässt
        // sich die rise nicht berechnen — das Projekt ist inkonsistent. Dann brechen
        // wir mit einem neutralen Fehler ab, BEVOR gespeichert wird (kein halb
        // geschriebenes Projekt; dieselbe fail-closed-Semantik wie slice-042d).
        const auto storey_height =
            model::resolveStoreyHeight(building, stair.from_storey_id);
        if (!storey_height) {
            throw std::runtime_error("E-IO: Ausgangs-Geschoss der Treppe unbekannt");
        }
        derived.stairRiseMm[stair.id] = stairRiseMm(stair, *storey_height);
    }
    repository.save(building, derived, path);
}

DrawingTargetResolution openProject(
    StructureEditService& service,
    const ports::driven::ProjectRepositoryPort& repository,
    const std::filesystem::path& path, const DrawingTargetSinks& sinks) {
    // Laden (wirft neutral bei fehlender/korrupter Datei) und erst DANACH
    // ersetzen: schlaegt das Laden fehl, hat der Service nichts gesehen und
    // der bisherige Stand bleibt unveraendert.
    model::Building loaded = repository.load(path);
    service.replaceBuilding(std::move(loaded));

    // Zeichen-Ziel neu aufloesen (slice-047b HIGH-3): die Sichten halten das
    // aktive Geschoss / die Hilfslinien-Ebene by-value aus dem ALTEN Modell.
    //
    // WICHTIG: hier wird nichts angelegt. Frueher legte der GUI-Pfad eine Ebene
    // "Canvas" an, wenn die geoeffnete Datei keine hatte — damit wich der Stand
    // im Speicher vom Dateiinhalt ab und ein anschliessendes Speichern schrieb
    // die Zusatz-Ebene mit (Code-Review MEDIUM-8, gegen LH-FA-BLD-003
    // "vollstaendig wiederhergestellt"). Fehlt etwas, meldet der Rueckgabewert
    // es dem Aufrufer, der es dem Benutzer zeigt.
    const model::Building& current = service.building();
    if (current.storeys.empty()) {
        return DrawingTargetResolution::NoStorey;
    }
    const model::StoreyId storey = current.storeys.front().id;
    if (sinks.set_active_storey) {
        sinks.set_active_storey(storey);
    }
    if (current.layers.empty()) {
        // Geschoss ist gesetzt (der Grundriss zeichnet), nur das Hilfslinien-
        // Ziel bleibt unaufgeloest — der Sink lehnt das Zeichnen dann
        // vertragsgemaess ab, bis der Benutzer selbst eine Ebene anlegt.
        return DrawingTargetResolution::NoLayer;
    }
    if (sinks.set_draw_target) {
        sinks.set_draw_target(storey, current.layers.front().id);
    }
    return DrawingTargetResolution::Resolved;
}

// --- ManageProjectService: die Port-Naht (slice-054) ----------------------
//
// Beide Methoden delegieren an die freien Funktionen oben. Der einzige
// Unterschied ist, WOHER die Abhängigkeiten kommen: aus den
// Konstruktor-Referenzen statt aus der Signatur. Genau das macht den Vertrag
// adapter-tauglich (Plan §2.1).

DrawingTargetResolution ManageProjectService::openProject(
    const std::filesystem::path& path, const DrawingTargetSinks& sinks) {
    return services::openProject(service_, repository_, path, sinks);
}

void ManageProjectService::saveProject(const std::filesystem::path& path) {
    // Der zu speichernde Stand ist der des Struktur-Service — nicht ein von
    // aussen gereichtes `Building`. Deshalb kommt ein `ui_command`-Handler ohne
    // `StructureEditService` aus.
    services::saveProject(repository_, service_.building(), path);
}

}  // namespace bcad::hexagon::services
