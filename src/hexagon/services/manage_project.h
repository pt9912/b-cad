#pragma once

#include <filesystem>
#include <functional>

#include "hexagon/model/building.h"
#include "hexagon/model/layer.h"  // LayerId
#include "hexagon/model/wall.h"   // StoreyId
#include "hexagon/ports/driven/project_repository_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace bcad::hexagon::services {

// Senken für das **Zeichen-Ziel** einer laufenden Sitzung (slice-047,
// Verify-Finding B4). Beim Aufbau der Sitzung frieren die Sichten das aktive
// Geschoss und die Hilfslinien-Ebene **by-value** ein; nach einem Projekt-Laden
// stammen diese Ids aus dem alten Modell und sind i. d. R. ungültig — der
// Grundriss bliebe leer und jedes Hilfslinien-Zeichnen würde abgelehnt.
// `openProject` löst das Ziel darum **selbst** neu auf und meldet es hier.
//
// **Port-frei** (Muster ADR-0019 Option A): zwei `std::function`-Callables statt
// eines Ports — der Kern kennt weder `view/` noch `command/`, und der
// Composition-Root verdrahtet die konkreten Sicht-Objekte. Nicht gesetzte
// Callables sind zulässig (CLI/Tests ohne Sichten).
struct DrawingTargetSinks {
    std::function<void(model::StoreyId)> set_active_storey;
    std::function<void(model::StoreyId, model::LayerId)> set_draw_target;
};

// Ergebnis der Neu-Auflösung. Ein unvollständiges Ziel ist **kein Fehler** (die
// Datei ist in Ordnung), aber benutzer-relevant: der Aufrufer meldet es, statt
// das Fehlende still anzulegen — Öffnen ist lesend (spez. §1 `LH-FA-BLD-003.a`,
// Code-Review MEDIUM-8).
enum class DrawingTargetResolution {
    Resolved,   // Geschoss + Ebene gesetzt
    NoStorey,   // Projekt ohne Geschoss — Zeichenfläche bleibt leer
    NoLayer,    // Geschoss gesetzt, aber keine Ebene für Hilfslinien
};

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
//
// **Teil des Use-Case, nicht des Aufrufers** (slice-047, Verify-Finding B4): nach
// dem Tausch löst `openProject` das **Zeichen-Ziel** neu auf und meldet es über
// `sinks`. Vorher lag dieser Schritt als Lambda im coverage-ausgenommenen `main`
// und wurde von **keinem** Sensor ausgeführt — sein Verlust blieb unbemerkt
// (Gegenprobe des Verifiers: Aufruf entfernt → 280/280 grün). Jetzt fällt er mit
// dem Use-Case zusammen und ist damit orakel-gedeckt.
DrawingTargetResolution openProject(
    StructureEditService& service,
    const ports::driven::ProjectRepositoryPort& repository,
    const std::filesystem::path& path, const DrawingTargetSinks& sinks = {});

}  // namespace bcad::hexagon::services
