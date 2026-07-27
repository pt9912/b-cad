#pragma once

#include <filesystem>
#include <utility>

#include "hexagon/ports/driving/manage_project_port.h"

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
    ProjectMenuHandler(hexagon::ports::driving::ManageProjectPort& project,
                       hexagon::ports::driving::DrawingTargetSinks sinks)
        : project_(project), sinks_(std::move(sinks)) {}

    // Öffnet `path` (LH-FA-BLD-003) und meldet, wie vollständig das
    // Zeichen-Ziel danach aufgelöst werden konnte. Fehler kommen als neutrale
    // `std::runtime_error` durch — der Aufrufer zeigt sie an.
    hexagon::ports::driving::DrawingTargetResolution open(
        const std::filesystem::path& path);

    // Speichert den Sitzungs-Stand unter `path` (LH-FA-BLD-002); wirft neutral.
    void saveAs(const std::filesystem::path& path);

private:
    hexagon::ports::driving::ManageProjectPort& project_;
    hexagon::ports::driving::DrawingTargetSinks sinks_;
};

}  // namespace bcad::adapters::ui::command
