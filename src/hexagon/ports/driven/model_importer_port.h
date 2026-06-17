#pragma once

#include <filesystem>

#include "hexagon/model/building.h"

namespace bcad::hexagon::ports::driven {

// Driven Port (ADR-0001, ADR-0013): liest ein externes Modell (IFC/DXF) in
// Domänen-Bauteile. Implementierungen kapseln das Dateiformat vollständig im
// Adapter — dieser Vertrag kennt nur `model::Building` und
// `std::filesystem::path`, keine Format-Typen (kein SPF/IFC-Symbol im Kern).
//
// Atomar (ADR-0013 #3): die Implementierung baut zuerst ein vollständiges
// In-Memory-`Building` und liefert es erst nach fehlerfreiem Parsen; bei
// Parse-/Format-Fehler oder fehlender tragender Pflicht-Referenz wirft sie
// eine **neutrale** `std::runtime_error` (kein Teil-Import). Der Adapter stellt
// dabei den Spec-Fehlercode `E-IO-003` (`event=import_rejected`) der Nachricht
// voran — eine Fehler-Zeichenkette ist kein Backend-Typ (Muster `ioCodeFor`
// der Persistenz, ADR-0001).
class ModelImporterPort {
public:
    virtual ~ModelImporterPort() = default;

    // Liest die Datei `path` in ein Domänen-`Building`. Wirft bei nicht
    // erkanntem/invalidem Format eine neutrale `std::runtime_error`.
    virtual model::Building read(const std::filesystem::path& path) const = 0;
};

}  // namespace bcad::hexagon::ports::driven
