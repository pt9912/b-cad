#pragma once

#include <filesystem>
#include <functional>

#include "hexagon/model/layer.h"  // LayerId
#include "hexagon/model/wall.h"   // StoreyId

namespace bcad::hexagon::ports::driving {

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
//
// **Warum hier und nicht in `services/`** (slice-054): der Typ steht im
// Port-Vertrag, also gehört er in die Port-Schicht. Er ist dort zulässig, weil
// er nur die Standardbibliothek und `model/`-Ids braucht — die einzige Kante,
// die `ports_driving` haben darf (`.a-check.yml`, spec/architecture.md §2).
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

// Driving Port (ADR-0001): Use-Case „Projekt verwalten" — die seit dem
// Bootstrap in spec/architecture.md §1.1 deklarierte Ziel-Form, realisiert in
// slice-054.
//
// **Zuschnitt:** `architecture.md` nennt vier Aufgaben (anlegen, speichern,
// laden, versionieren); der Port deklariert die **zwei, die es gibt**
// (LH-FA-BLD-002/003). Ein Port mit unimplementierten Methoden wäre eine Lüge
// im Vertrag — „Neues Projekt" (LH-FA-BLD-001) kommt mit slice-052b,
// Versionierung (LH-FA-BLD-004) später.
//
// **Signatur-Schnitt:** die Infrastruktur (Repository, Struktur-Service) hält
// die Implementierung als Konstruktor-Abhängigkeit, NICHT der Vertrag — ein
// Driving Port darf nur `model/` sehen. Genau dadurch kann ein
// `adapters/ui/command/`-Handler diesen Port rufen, ohne den
// `ProjectRepositoryPort` oder den `StructureEditService` zu kennen (die Kante
// `ui_command → ports_driving` besteht, `ui_command → services` nicht).
//
// **Fehler kommen neutral durch** (`std::runtime_error`): fehlende oder
// korrupte Datei, unbaubare Geometrie, inkonsistentes Modell. Der Aufrufer
// entscheidet über die Anzeige; der Port kennt kein Framework.
class ManageProjectPort {
public:
    virtual ~ManageProjectPort() = default;

    // Lädt das Projekt unter `path` und ersetzt den Sitzungs-Stand
    // (LH-FA-BLD-003). Schlägt das Laden fehl, bleibt der bisherige Stand
    // unverändert (erst laden, dann ersetzen). Anschließend wird das
    // Zeichen-Ziel neu aufgelöst und über `sinks` gemeldet; der Rückgabewert
    // sagt, wie vollständig das gelang.
    //
    // **`sinks` hat bewusst KEINEN Default** (slice-053 §1.1): mit `= {}` wäre
    // ein Treiber, der die Senken vergisst, still übersetzbar gewesen und hätte
    // eine gültige Resolution geliefert — der Verlust der Zeichen-Ziel-
    // Neuauflösung (slice-047-Verify-B4) bliebe unter jedem Orakel grün. Ohne
    // Default ist das Vergessen ein **Compile-Fehler**. Wer keine Sichten hat
    // (CLI, Kern-Tests), reicht sichtbar `{}`.
    virtual DrawingTargetResolution openProject(
        const std::filesystem::path& path,
        const DrawingTargetSinks& sinks) = 0;

    // Speichert den aktuellen Sitzungs-Stand **atomar** unter `path`
    // (LH-FA-BLD-002). **Fail-closed:** ein inkonsistentes Modell (danglendes
    // `from_storey` einer Treppe) wirft **vor** dem Schreiben — kein
    // Teil-Speichern, die Zieldatei bleibt unverändert.
    virtual void saveProject(const std::filesystem::path& path) = 0;
};

}  // namespace bcad::hexagon::ports::driving
