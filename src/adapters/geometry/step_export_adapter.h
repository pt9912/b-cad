#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_exporter_port.h"

namespace bcad::adapters::geometry {

// IO/Export (ADR-0014): **geometrie-residenter** `ModelExporterPort` für STEP.
// Assembliert die **B-Rep-Volumenkörper** der OCC-Solid-Bauteile — Wände und
// Decken/Fundament (Footprint-Extrusion + Öffnungs-/Aussparungs-Cutouts über den
// geteilten `makeNetSolid`) — zu einem `TopoDS_Compound` und schreibt ihn via
// OCC-DataExchange (`STEPControl_Writer`, AP214) atomar (Temp + Rename).
//
// **Benannte Lücke (slice-020b):** Dächer und Treppen sind analytische
// Dreiecksnetze ohne OCC-Solid und werden hier (noch) **nicht** geschrieben —
// für sie ist STL der verlustfreie Pfad; ihre B-Rep-Vernähung ist ein
// Folge-Inkrement (spez. §1 `LH-FA-IO-005.a`).
//
// Lebt in `src/adapters/geometry/` (OCC gekapselt, Regel C); kein OCC-Typ-Leck
// (neutraler Wurf). Nicht beschreibbarer Zielpfad → `E-IO-001` (kein Teil-Export);
// Modell ohne OCC-Solid-Bauteile → gültige, leere STEP (Totalität).
class StepExportAdapter final : public hexagon::ports::driven::ModelExporterPort {
public:
    void write(const hexagon::model::Building& building,
               const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::geometry
