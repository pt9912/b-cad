#pragma once

#include <filesystem>

#include "hexagon/model/building.h"

namespace bcad::hexagon::ports::driven {

// Driven Port (ADR-0001, ADR-0013): schreibt ein Domänen-`Building` in ein
// externes Format (IFC/DXF/STEP/…). Implementierungen kapseln das Dateiformat
// vollständig im Adapter — dieser Vertrag kennt nur `model::Building` und
// `std::filesystem::path`, keine Format-Typen (Spiegel von `ModelImporterPort`).
//
// **Atomar** (ADR-0013 #3): die Implementierung schreibt in eine Temp-Datei und
// ersetzt den Zielpfad erst nach Erfolg (Rename) — bei nicht beschreibbarem
// Zielpfad bleibt der vorherige Stand intakt, **kein** Teil-Export. Geworfen
// wird eine **neutrale** `std::runtime_error`; der Adapter stellt den
// Spec-Fehlercode `E-IO-001` (`event=io_no_permission`) der Nachricht voran
// (eine Fehler-Zeichenkette ist kein Backend-Typ, ADR-0001).
class ModelExporterPort {
public:
    virtual ~ModelExporterPort() = default;

    // Schreibt das Modell **atomar** nach `path`. Wirft bei nicht
    // beschreibbarem Zielpfad eine neutrale `std::runtime_error`.
    virtual void write(const model::Building& building,
                       const std::filesystem::path& path) const = 0;
};

}  // namespace bcad::hexagon::ports::driven
