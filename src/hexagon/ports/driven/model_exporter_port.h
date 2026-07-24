#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/model/derived_geometry.h"
#include "hexagon/model/export_provenance.h"

namespace bcad::hexagon::ports::driven {

// Driven Port (ADR-0001, ADR-0013): schreibt ein Domänen-`Building` in ein
// externes Format (IFC/DXF/STEP/…). Implementierungen kapseln das Dateiformat
// vollständig im Adapter — dieser Vertrag kennt nur `model::Building`, das
// kern-berechnete `model::DerivedGeometry`-Bündel und `std::filesystem::path`,
// keine Format-Typen (Spiegel von `ModelImporterPort`).
//
// **`DerivedGeometry` (ADR-0020):** driven Adapter serialisieren nur, sie leiten
// **keine** Domänen-Geometrie ab — der Kern berechnet sie und reicht sie als
// pures Werttyp-Bündel (format-selektiv befüllt; leer für Formate, die nichts
// ableiten). Der Adapter tut nur Serialisierung + backend-nötige Montage
// (OCC-B-Rep aus pre-OCC-Primitiven, Tessellation über den `GeometryKernelPort`).
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

    // Schreibt das Modell **atomar** nach `path`, unter Nutzung der kern-
    // gelieferten `derived`-Geometrie (format-selektiv; ggf. leer). Wirft bei
    // nicht beschreibbarem Zielpfad eine neutrale `std::runtime_error`.
    // `provenance` (slice-046): injizierte Export-Herkunft (Datum/Quelle/Version),
    // sichtbar/eingebettet gerendert. **Optional** — leer lässt den Adapter auf sein
    // deterministisches Sentinel-Verhalten (044a/045) zurückfallen.
    virtual void write(const model::Building& building,
                       const model::DerivedGeometry& derived,
                       const std::filesystem::path& path,
                       const model::ExportProvenance& provenance = {}) const = 0;
};

}  // namespace bcad::hexagon::ports::driven
