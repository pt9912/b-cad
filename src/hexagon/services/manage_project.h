#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/project_repository_port.h"

namespace bcad::hexagon::services {

// Save-Use-Case (slice-047a, der vorgesehene `ManageProjectPort`): baut die
// **kern-abgeleiteten** write-derived Persistenz-Skalare (`PersistedDerivations`:
// `rise` je Treppe, aus `resolveStoreyHeight` + `stairRiseMm`) aus dem `Building`
// und speichert **atomar** über den `ProjectRepositoryPort` (der Adapter
// serialisiert nur, ADR-0020). **Fail-closed:** ein danglendes `from_storey`
// wirft neutral **vor** dem `save` — **kein** Teil-Speichern, Zieldatei unverändert.
//
// Promotet aus dem früheren Test-Helfer (`save_project_test_helper.h`) in die
// Produktion (framework-frei; nutzt nur den Port + Kern-Geometrie, ADR-0001) —
// die **einzige** Save-Ableitungs-Logik, von CLI (slice-047a) und GUI (047b)
// **geteilt**, damit sie testbar außerhalb des coverage-ausgenommenen main/GUI liegt.
void saveProject(const ports::driven::ProjectRepositoryPort& repository,
                 const model::Building& building,
                 const std::filesystem::path& path);

}  // namespace bcad::hexagon::services
