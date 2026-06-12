#pragma once

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
// `services/wall_footprint.h`) — der Adapter extrudiert/tesselliert nur
// noch das Polygon.
class GeometryKernelPort {
public:
    virtual ~GeometryKernelPort() = default;

    // Extrudiert das Grundriss-Polygon auf Höhe (+Z) zu einem Solid und
    // misst es (LH-FA-D3-001). Wirft bei degeneriertem Polygon
    // (kollabiert, nicht-endlich) eine neutrale Ausnahme (E-GEO-002).
    virtual model::Solid extrudeFootprint(const model::Footprint& footprint,
                                          double height_mm) const = 0;

    // Tesselliert das extrudierte Polygon-Solid zum Dreiecksnetz für die
    // Darstellung (ADR-0009 (b)). Fehlerverhalten wie `extrudeFootprint`.
    virtual model::TriangleMesh tessellateFootprint(
        const model::Footprint& footprint, double height_mm) const = 0;
};

}  // namespace bcad::hexagon::ports::driven
