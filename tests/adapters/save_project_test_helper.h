#pragma once

#include <filesystem>
#include <stdexcept>

#include "adapters/persistence/sqlite_project_repository.h"
#include "hexagon/model/building.h"
#include "hexagon/model/persisted_derivations.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/storey_query.h"
#include "hexagon/services/geometry/stair_geometry.h"

namespace bcad::adapters::persistence::test {

// Test-Aufrufer für den geweiteten `save`-Vertrag (slice-042d, ADR-0020). Steht
// stellvertretend für den künftigen `ManageProjectPort`-Save-Use-Case-Service
// (existiert noch nicht — es gibt keinen Produktions-`save`-Aufrufer; die Naht
// ist bereits richtig geschnitten). Berechnet die **kern-abgeleiteten**
// Persistenz-Skalare (`rise` je Treppe) aus **derselben** Quelle, die früher der
// Adapter nutzte (`services::stairRiseMm` + die geteilte `resolveStoreyHeight`) →
// der gebundene `rise_mm`-Wert bleibt byte-identisch. Danglendes `from_storey` →
// **E-IO** vor `save` (werfende Semantik, kein Teil-Save — beobachtbar identisch
// zum früheren adapter-lokalen `fromStoreyHeight`-Wurf, der mitten in der
// Transaktion warf und zurückrollte).
inline void saveProject(const SqliteProjectRepository& repo,
                        const hexagon::model::Building& building,
                        const std::filesystem::path& path) {
    hexagon::model::PersistedDerivations derived;
    for (const hexagon::model::Stair& stair : building.stairs) {
        const auto height =
            hexagon::model::resolveStoreyHeight(building, stair.from_storey_id);
        if (!height) {
            throw std::runtime_error("E-IO: stairs from_storey unbekannt");
        }
        derived.stairRiseMm[stair.id] =
            hexagon::services::stairRiseMm(stair, *height);
    }
    repo.save(building, derived, path);
}

}  // namespace bcad::adapters::persistence::test
