#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_exporter_port.h"

namespace bcad::adapters::geometry {

// IO/Export (ADR-0014): **geometrie-residenter** `ModelExporterPort` für STEP.
// Assembliert die **B-Rep-Volumenkörper** **aller** 3D-Bauteile zu einem
// `TopoDS_Compound` und schreibt ihn via OCC-DataExchange (`STEPControl_Writer`,
// AP214) atomar (Temp + Rename):
//   - **Wände + Decken/Fundament:** Footprint-Extrusion + Öffnungs-/Aussparungs-
//     Cutouts über den geteilten `makeNetSolid` (slice-020b).
//   - **Dächer:** das wasserdichte Dach-Netz zu **einem** Solid vernäht
//     (`meshToSolid`, fail-closed; slice-024a).
//   - **Treppen-Stufen:** analytische OCC-Box-Solids je Stufe (`makeBoxSolid` aus
//     der geteilten `stairStepBoxes`-Query; slice-024b). Das **Geländer**
//     (render-only) bleibt **ausgelassen** — nur im STL (spez. §1 `LH-FA-IO-005.a`).
//
// Lebt in `src/adapters/geometry/` (OCC gekapselt, Regel C); kein OCC-Typ-Leck
// (neutraler Wurf). Nicht beschreibbarer Zielpfad → `E-IO-001` (kein Teil-Export);
// degeneriertes/nicht geschlossen vernähbares Bauteil → übersprungen; 3D-leeres
// Modell → gültige, leere STEP (Totalität).
class StepExportAdapter final : public hexagon::ports::driven::ModelExporterPort {
public:
    void write(const hexagon::model::Building& building,
               const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::geometry
