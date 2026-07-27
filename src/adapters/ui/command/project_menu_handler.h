#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <utility>

#include "hexagon/model/building.h"

#include "hexagon/ports/driving/manage_project_port.h"
#include "hexagon/ports/driving/project_session_port.h"

namespace bcad::adapters::ui::command {

// Was die Datei-Menü-Aktionen **tun** (slice-053) — der erste Treiber-Adapter
// am `ManageProjectPort` (slice-054) neben dem Composition-Root. Genau dafür
// wurde der Port geschnitten: dieser Handler sieht **weder** den
// `StructureEditService` **noch** den `ProjectRepositoryPort`, und braucht
// beide auch nicht. `.a-check.yml` erlaubt `ui_command → ports_driving` und
// verbietet `ui_command → services`/`ports_driven`; erst mit dem Port ist diese
// Klasse überhaupt baubar (Plan-Review Lauf 1, HIGH-1).
//
// **Nicht hier:** die Dateiauswahl (modaler `QFileDialog`), die Meldungstexte
// und der Fenstertitel — sie bleiben im Composition-Root. Der Handler bekommt
// einen fertigen Pfad und gibt das Ergebnis zurück; er kennt kein Qt.
//
// **Die Senken** (`DrawingTargetSinks`) hält er als Wert und **reicht sie bei
// jedem Öffnen durch** — das ist seine zweite Verantwortung neben dem Aufruf.
// Ohne sie zeigen die Sichten nach einem Projekt-Laden auf Ids des ALTEN
// Modells (slice-047-Verify-B4). Der Port hat für sie bewusst keinen Default,
// damit das Vergessen ein Compile-Fehler ist; das Durchreichen **leerer**
// Senken fängt nur ein Orakel (slice-053 §3-Zeile 6).
class ProjectMenuHandler {
public:
    // slice-052a: `session` und `current` kommen dazu. `current` liefert den
    // **aktuellen** Modell-Stand — der Handler hält ihn nicht, er fragt ihn ab
    // (port-freie Naht, Muster ADR-0019 Option A: `ui_command` darf den
    // `StructureEditService` nicht sehen).
    using BuildingPull = std::function<const hexagon::model::Building&()>;

    ProjectMenuHandler(hexagon::ports::driving::ManageProjectPort& project,
                       hexagon::ports::driving::ProjectSessionPort& session,
                       BuildingPull current,
                       hexagon::ports::driving::DrawingTargetSinks sinks)
        : project_(project),
          session_(session),
          current_(std::move(current)),
          sinks_(std::move(sinks)) {}

    // Öffnet `path` (LH-FA-BLD-003) und meldet, wie vollständig das
    // Zeichen-Ziel danach aufgelöst werden konnte. Fehler kommen als neutrale
    // `std::runtime_error` durch — der Aufrufer zeigt sie an.
    hexagon::ports::driving::DrawingTargetResolution open(
        const std::filesystem::path& path);

    // Speichert den Sitzungs-Stand unter `path` (LH-FA-BLD-002); wirft neutral.
    void saveAs(const std::filesystem::path& path);


    // --- slice-052a ---------------------------------------------------------

    // Verdikt vor einer verwerfenden Aktion (Öffnen, Fenster schließen): muss
    // der Benutzer gefragt werden? **Die Entscheidung fällt im Kern** — der
    // Handler holt sie, er bildet sie nicht.
    hexagon::ports::driving::DiscardVerdict verdictForDiscard() const;

    // Wohin „Speichern" schreibt (bekannte Datei / Ziel-Abfrage).
    hexagon::ports::driving::SaveTarget saveTarget() const;

    // Was aus der Benutzer-Antwort folgt — ebenfalls Kern-Auswertung.
    hexagon::ports::driving::DiscardOutcome evaluate(
        hexagon::ports::driving::DiscardAnswer answer) const;

    // „Speichern" auf die bekannte Datei (LH-FA-BLD-002). Gibt `false` zurück,
    // wenn kein Pfad bekannt ist — dann muss der Aufrufer „Speichern unter…"
    // anbieten. Schreibfehler kommen als Wurf durch.
    bool save();

    // Darf die auslösende Aktion (Öffnen, Fenster schließen) laufen?
    //
    // **Hier liegt die Komposition — nicht in der Verdrahtung.** Der
    // Composition-Root reicht nur zwei Fragen herein: `ask` stellt die
    // Rückfrage, `ask_target` erfragt ein Speicher-Ziel. Beides sind Dialoge,
    // also die benannte Grenze. Alles dazwischen — Verdikt holen, Antwort
    // auswerten, ggf. speichern, Fehler behandeln — läuft hier und ist damit
    // orakel-gedeckt. Läge es in `main.cpp`, wäre es sensorlos; genau diese
    // Klasse Finding hat die ganze Kette erzeugt.
    //
    // `false` heißt: die auslösende Aktion **unterbleibt**.
    using DiscardAsk = std::function<hexagon::ports::driving::DiscardAnswer()>;
    using SaveTargetAsk =
        std::function<std::optional<std::filesystem::path>()>;
    bool mayDiscard(const DiscardAsk& ask, const SaveTargetAsk& ask_target);

    // slice-052b: legt ein neues, leeres Projekt an (LH-FA-BLD-001) —
    // **hinter** der Rückfrage. Gibt `false` zurück, wenn der Benutzer
    // abgebrochen hat (oder ein vorgeschaltetes Speichern scheiterte); dann
    // bleibt das alte Projekt **vollständig** stehen.
    //
    // Die Rückfrage-Kette ist dieselbe wie beim Öffnen und beim Schließen —
    // `mayDiscard` wird **nicht** ein zweites Mal geschrieben.
    bool newProject(const DiscardAsk& ask, const SaveTargetAsk& ask_target);

private:
    hexagon::ports::driving::ManageProjectPort& project_;
    hexagon::ports::driving::ProjectSessionPort& session_;
    BuildingPull current_;
    hexagon::ports::driving::DrawingTargetSinks sinks_;
};

}  // namespace bcad::adapters::ui::command
