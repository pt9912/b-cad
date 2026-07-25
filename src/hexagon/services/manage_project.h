#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/project_repository_port.h"
#include "hexagon/services/structure_edit_service.h"

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

// Open-Use-Case (slice-047b) — symmetrische Gegen-Naht zu `saveProject`: lädt
// über den `ProjectRepositoryPort` und übergibt das Ergebnis dem Service, der
// das Modell **samt abgeleiteter Zustände** ersetzt (`replaceBuilding`).
// Fehler (fehlende/korrupte Datei, unbaubare Geometrie) kommen als neutrale
// `std::runtime_error` durch — der Aufrufer entscheidet über die Anzeige;
// bei einem Fehler bleibt der bisherige Modell-Stand unverändert.
//
// Zweck der Naht (wie bei `saveProject`): der Lade-Pfad ist damit **testbar
// außerhalb** des coverage-ausgenommenen `main`/GUI — der Datei-Dialog bleibt
// die einzige untestbare Schicht darüber.
void openProject(StructureEditService& service,
                 const ports::driven::ProjectRepositoryPort& repository,
                 const std::filesystem::path& path);

}  // namespace bcad::hexagon::services
