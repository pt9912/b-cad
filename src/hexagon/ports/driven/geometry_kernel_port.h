#pragma once

#include <vector>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/solid.h"
#include "hexagon/model/triangle_mesh.h"

namespace bcad::hexagon::ports::driven {

// Driven Port (ADR-0001/0002): Abstraktion des Geometrie-Kerns. Der Kern
// kennt nur diesen Vertrag, nicht OpenCascade. Implementierungen leben in
// `src/adapters/geometry/` (OCC-Adapter ab slice-003b). Die Rückgabewerte
// `model::Solid`/`model::TriangleMesh` sind neutral — KEIN OCC-Typ
// verlässt den Adapter.
//
// Seit slice-012 (LH-FA-WAL-006-Teilumfang) liefert der KERN den
// Grundriss (`model::Footprint`, inkl. Eckenschluss aus Nachbar-Wissen,
// `services/geometry/wall_footprint.h`) — der Adapter extrudiert/tesselliert nur
// noch das Polygon.
//
// Seit slice-013b (LH-FA-DOR-004/WIN-005, ADR-0011 (b)): der Kern liefert
// zusätzlich die Öffnungs-Schnittkörper als `model::CutPrism` (reine
// Geometrie-Werte); der Adapter subtrahiert sie vom extrudierten
// Wand-Solid (boolesch, OCC). Eine **leere** `cutouts`-Liste ergibt das
// Verhalten vor slice-013b (reine Footprint-Extrusion). Der Adapter
// kennt keine Öffnungs-Semantik (ADR-0001/0002).
class GeometryKernelPort {
public:
    virtual ~GeometryKernelPort() = default;

    // Extrudiert das Grundriss-Polygon auf Höhe (+Z), subtrahiert die
    // Schnitt-Prismen `cutouts` und misst das Netto-Solid (LH-FA-D3-001,
    // LH-FA-DOR-004/WIN-005). Wirft bei degeneriertem Polygon oder
    // fehlgeschlagener Subtraktion eine neutrale Ausnahme (E-GEO-002).
    virtual model::Solid extrudeFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const = 0;

    // Tesselliert das ausgeschnittene Polygon-Solid zum Dreiecksnetz für
    // die Darstellung (ADR-0009 (b)). Fehlerverhalten wie `extrudeFootprint`.
    virtual model::TriangleMesh tessellateFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const = 0;
};

}  // namespace bcad::hexagon::ports::driven
