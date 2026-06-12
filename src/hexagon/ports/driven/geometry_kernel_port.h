#pragma once

#include "hexagon/model/solid.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/model/wall.h"

namespace bcad::hexagon::ports::driven {

// Driven Port (ADR-0001/0002): Abstraktion des Geometrie-Kerns. Der Kern
// kennt nur diesen Vertrag, nicht OpenCascade. Implementierungen leben in
// `src/adapters/geometry/` (OCC-Adapter ab slice-003b). Die Rückgabewerte
// `model::Solid`/`model::TriangleMesh` sind neutral — KEIN OCC-Typ
// verlässt den Adapter.
class GeometryKernelPort {
public:
    virtual ~GeometryKernelPort() = default;

    // Extrudiert das Wand-Footprint (Segment × Stärke) auf Wandhöhe zu
    // einem Solid (LH-FA-D3-001).
    virtual model::Solid extrudeWall(const model::Wall& wall) const = 0;

    // Tesselliert das extrudierte Wand-Solid zum Dreiecksnetz für die
    // Darstellung (ADR-0009 (b): Tessellation über Port). Wirft bei
    // degenerierter Geometrie wie `extrudeWall` (E-GEO-002).
    virtual model::TriangleMesh tessellateWall(const model::Wall& wall) const = 0;
};

}  // namespace bcad::hexagon::ports::driven
