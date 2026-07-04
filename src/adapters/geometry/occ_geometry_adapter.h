#pragma once

#include <vector>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/solid.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"

namespace bcad::adapters::geometry {

// Driven Adapter (ADR-0002): erfüllt `GeometryKernelPort` über
// OpenCascade. Die OCC-Typen bleiben vollständig in der `.cpp` gekapselt —
// dieser Header bindet KEIN OCC ein (ADR-0001/0002; a-check Regel C).
class OccGeometryAdapter final
    : public hexagon::ports::driven::GeometryKernelPort {
public:
    // Extrudiert das Grundriss-Polygon (Footprint-Hoheit liegt seit
    // slice-012 im Kern) auf Höhe zu einem Solid, subtrahiert die
    // Öffnungs-Schnittkörper `cutouts` (boolesch, LH-FA-DOR-004/WIN-005,
    // ADR-0011) und misst das Netto-Volumen (LH-FA-D3-001). Wirft bei
    // degeneriertem Polygon oder fehlgeschlagener Subtraktion eine
    // neutrale `std::runtime_error` (E-GEO-002) — kein OCC-Ausnahmetyp
    // verlässt den Adapter.
    hexagon::model::Solid extrudeFootprint(
        const hexagon::model::Footprint& footprint, double height_mm,
        const std::vector<hexagon::model::CutPrism>& cutouts) const override;

    // Tesselliert das ausgeschnittene Polygon-Solid zum neutralen
    // Dreiecksnetz (ADR-0009 (b): Tessellation über Port — kein OCC in
    // der GUI). Flat-Shading-Layout (eigene Vertices + Flächennormale je
    // Dreieck). Fehlerverhalten wie `extrudeFootprint` (E-GEO-002).
    hexagon::model::TriangleMesh tessellateFootprint(
        const hexagon::model::Footprint& footprint, double height_mm,
        const std::vector<hexagon::model::CutPrism>& cutouts) const override;
};

}  // namespace bcad::adapters::geometry
