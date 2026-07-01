#pragma once

#include <filesystem>

#include "hexagon/model/building.h"

namespace bcad::hexagon::ports::driving {

// Austauschformat-Diskriminator (Format-neutral, ADR-0013/0014/0015/0016). welle-4:
// IFC (Import+Export, io-resident), STEP/STL (Export-only, geometrie-resident,
// ADR-0014), DXF (Import+Export, io-resident, ADR-0015), PDF + PNG (Export-only,
// io-resident, ADR-0016 — self-rolled; PDF Vektor-Maßstabsplan slice-025b,
// PNG Raster-Grundriss slice-025c).
enum class ExchangeFormat { Ifc, Step, Stl, Dxf, Pdf, Png };

// Driving Port (ADR-0001, architecture §1.1): stößt Import/Export an, ohne das
// konkrete Format zu kennen. welle-4 (slice-019b) implementiert den Import;
// der Export (`LH-FA-IO-002`, Roundtrip) folgt in slice-019c.
class ExchangeModelPort {
public:
    virtual ~ExchangeModelPort() = default;

    // Importiert `path` im angegebenen `format` in ein Domänen-`Building`.
    // Propagiert den Adapter-Fehler (`E-IO-003`, kein Teil-Import) unverändert
    // an den Aufrufer (`LH-FA-IO-001` Negative).
    virtual model::Building importModel(const std::filesystem::path& path,
                                        ExchangeFormat format) const = 0;

    // Exportiert `building` im angegebenen `format` nach `path` (atomar, kein
    // Teil-Export). Propagiert den Adapter-Fehler (`E-IO-001`) unverändert
    // (`LH-FA-IO-002` Negative; slice-019c).
    virtual void exportModel(const model::Building& building,
                             const std::filesystem::path& path,
                             ExchangeFormat format) const = 0;
};

}  // namespace bcad::hexagon::ports::driving
