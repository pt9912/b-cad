#include "hexagon/services/manage_project.h"

#include <stdexcept>

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

}  // namespace bcad::hexagon::services
