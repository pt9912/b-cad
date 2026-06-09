#pragma once

#include "hexagon/model/solid.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"

namespace bcad::adapters::geometry {

// Driven Adapter (ADR-0002): erfüllt `GeometryKernelPort` über
// OpenCascade. Die OCC-Typen bleiben vollständig in der `.cpp` gekapselt —
// dieser Header bindet KEIN OCC ein (ADR-0001/0002; arch-check Regel C).
class OccGeometryAdapter final
    : public hexagon::ports::driven::GeometryKernelPort {
public:
    // Extrudiert das Wand-Footprint (Segment × Stärke) auf Wandhöhe zu
    // einem Solid und misst dessen Volumen (LH-FA-D3-001). Wirft bei
    // degenerierter/fehlgeschlagener Geometrie eine neutrale
    // `std::runtime_error` (E-GEO-002) — kein OCC-Ausnahmetyp verlässt
    // den Adapter.
    hexagon::model::Solid extrudeWall(
        const hexagon::model::Wall& wall) const override;
};

}  // namespace bcad::adapters::geometry
