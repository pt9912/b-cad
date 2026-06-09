// Driven-Adapter (Geometrie) — OpenCascade-Implementierung des
// `GeometryKernelPort` (ADR-0002). OCC-Typen sind hier gekapselt; nach
// außen geht nur `model::Solid` (neutral). Extrusion = Wand-Footprint
// (Segment × Stärke) als Rechteck-Fläche, per Prisma auf Wandhöhe in +Z
// gezogen; Volumen über `BRepGProp` gemessen (LH-FA-D3-001).

#include "adapters/geometry/occ_geometry_adapter.h"

#include <cmath>
#include <stdexcept>
#include <string>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GProp_GProps.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include "hexagon/model/constants.h"

namespace bcad::adapters::geometry {

namespace model = hexagon::model;

model::Solid OccGeometryAdapter::extrudeWall(const model::Wall& wall) const {
    const double dx = wall.end.x_mm - wall.start.x_mm;
    const double dy = wall.end.y_mm - wall.start.y_mm;
    const double length = std::sqrt((dx * dx) + (dy * dy));
    if (!std::isfinite(length) || length < model::kGeometryToleranceMm) {
        throw std::runtime_error("OccGeometryAdapter: degeneriertes Segment (E-GEO-002)");
    }

    // Einheits-Normale auf das Segment in der XY-Ebene; Footprint-Eckpunkte
    // um die halbe Stärke nach beiden Seiten versetzt.
    const double nx = -dy / length;
    const double ny = dx / length;
    const double half = wall.thickness_mm * 0.5;

    try {
        const gp_Pnt p1(wall.start.x_mm + (nx * half), wall.start.y_mm + (ny * half), 0.0);
        const gp_Pnt p2(wall.end.x_mm + (nx * half), wall.end.y_mm + (ny * half), 0.0);
        const gp_Pnt p3(wall.end.x_mm - (nx * half), wall.end.y_mm - (ny * half), 0.0);
        const gp_Pnt p4(wall.start.x_mm - (nx * half), wall.start.y_mm - (ny * half), 0.0);

        BRepBuilderAPI_MakePolygon polygon(p1, p2, p3, p4, Standard_True);
        BRepBuilderAPI_MakeFace face(polygon.Wire());
        const gp_Vec extrusion(0.0, 0.0, wall.height_mm);
        const TopoDS_Shape solid =
            BRepPrimAPI_MakePrism(face.Face(), extrusion).Shape();

        GProp_GProps properties;
        BRepGProp::VolumeProperties(solid, properties);
        return model::Solid{properties.Mass()};
    } catch (const Standard_Failure& failure) {
        // OCC-Ausnahme in einen neutralen Fehler übersetzen — kein
        // OCC-Typ verlässt den Adapter (ADR-0001/0002, E-GEO-002).
        throw std::runtime_error(
            std::string("OccGeometryAdapter: Extrusion fehlgeschlagen (E-GEO-002): ") +
            failure.GetMessageString());
    }
}

}  // namespace bcad::adapters::geometry
