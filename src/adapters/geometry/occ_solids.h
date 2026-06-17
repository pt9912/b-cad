#pragma once

#include <vector>

#include <TopoDS_Shape.hxx>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"

// Geteilte OCC-Solid-Konstruktion (geometrie-intern, Regel C — diese Datei
// trägt OCC-`.hxx` und darf NUR von `src/adapters/geometry/` inkludiert werden).
// Gemeinsamer Builder für Volumen/Tessellation (`OccGeometryAdapter`) und den
// STEP-Export (B-Rep-Solids, slice-020b). Reine Geometrie — kein Domänen-Wissen.
namespace bcad::adapters::geometry {

// Grundriss-Polygon auf Höhe (+Z) extrudiert, minus die Öffnungs-Schnittkörper
// `cutouts` (boolesch, OCC). Wirft `std::runtime_error` (E-GEO-002) bei
// degeneriertem Polygon oder fehlgeschlagener Subtraktion — kein OCC-Typ-Leck
// über die Adapter-Grenze hinaus.
TopoDS_Shape makeNetSolid(const hexagon::model::Footprint& footprint,
                          double height_mm,
                          const std::vector<hexagon::model::CutPrism>& cutouts);

}  // namespace bcad::adapters::geometry
