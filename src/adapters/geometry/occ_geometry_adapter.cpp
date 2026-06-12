// Driven-Adapter (Geometrie) — OpenCascade-Implementierung des
// `GeometryKernelPort` (ADR-0002). OCC-Typen sind hier gekapselt; nach
// außen gehen nur `model::Solid`/`model::TriangleMesh` (neutral).
// Extrusion = Wand-Footprint (Segment × Stärke) als Rechteck-Fläche, per
// Prisma auf Wandhöhe in +Z gezogen; Volumen über `BRepGProp` gemessen
// (LH-FA-D3-001). Tessellation = `BRepMesh_IncrementalMesh` über dem
// Prisma, Flat-Shading-Layout (ADR-0009 (b)).

#include "adapters/geometry/occ_geometry_adapter.h"

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Failure.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include "hexagon/model/constants.h"

namespace bcad::adapters::geometry {

namespace model = hexagon::model;

namespace {

// Wand-Footprint als Prisma — gemeinsamer Solid-Builder für Extrusion
// (Volumen) und Tessellation. Wirft `std::runtime_error` (E-GEO-002)
// bei degeneriertem Segment.
TopoDS_Shape makeWallSolid(const model::Wall& wall) {
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

    const gp_Pnt p1(wall.start.x_mm + (nx * half), wall.start.y_mm + (ny * half), 0.0);
    const gp_Pnt p2(wall.end.x_mm + (nx * half), wall.end.y_mm + (ny * half), 0.0);
    const gp_Pnt p3(wall.end.x_mm - (nx * half), wall.end.y_mm - (ny * half), 0.0);
    const gp_Pnt p4(wall.start.x_mm - (nx * half), wall.start.y_mm - (ny * half), 0.0);

    BRepBuilderAPI_MakePolygon polygon(p1, p2, p3, p4, Standard_True);
    BRepBuilderAPI_MakeFace face(polygon.Wire());
    const gp_Vec extrusion(0.0, 0.0, wall.height_mm);
    return BRepPrimAPI_MakePrism(face.Face(), extrusion).Shape();
}

// Hängt ein Dreieck im Flat-Shading-Layout an: drei eigene Vertices mit
// der Flächennormale (TriangleMesh-Konvention, triangle_mesh.h).
void appendTriangle(model::TriangleMesh& mesh, const gp_Pnt& a, const gp_Pnt& b,
                    const gp_Pnt& c) {
    const gp_Vec ab(a, b);
    const gp_Vec ac(a, c);
    gp_Vec normal = ab.Crossed(ac);
    const double magnitude = normal.Magnitude();
    if (magnitude < 1e-12) {
        return;  // degeneriertes Dreieck — trägt keine Fläche
    }
    normal.Divide(magnitude);

    for (const gp_Pnt& p : {a, b, c}) {
        mesh.indices.push_back(mesh.vertexCount());
        mesh.positions.insert(mesh.positions.end(), {p.X(), p.Y(), p.Z()});
        mesh.normals.insert(mesh.normals.end(),
                            {normal.X(), normal.Y(), normal.Z()});
    }
}

}  // namespace

model::Solid OccGeometryAdapter::extrudeWall(const model::Wall& wall) const {
    try {
        const TopoDS_Shape solid = makeWallSolid(wall);
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

model::TriangleMesh OccGeometryAdapter::tessellateWall(
    const model::Wall& wall) const {
    try {
        const TopoDS_Shape solid = makeWallSolid(wall);
        // Lineare Ablenkung 1 mm: Wand-Prismen sind eben — die
        // Triangulation ist exakt, der Wert unkritisch (ADR-0009:
        // Granularität wird erst mit einem Latenz-Budget Thema).
        BRepMesh_IncrementalMesh mesher(solid, /*theLinDeflection=*/1.0);

        model::TriangleMesh mesh;
        for (TopExp_Explorer faces(solid, TopAbs_FACE); faces.More();
             faces.Next()) {
            const TopoDS_Face face = TopoDS::Face(faces.Current());
            TopLoc_Location location;
            const Handle(Poly_Triangulation) triangulation =
                BRep_Tool::Triangulation(face, location);
            if (triangulation.IsNull()) {
                continue;
            }
            const bool reversed = (face.Orientation() == TopAbs_REVERSED);
            for (int t = 1; t <= triangulation->NbTriangles(); ++t) {
                Standard_Integer i1 = 0;
                Standard_Integer i2 = 0;
                Standard_Integer i3 = 0;
                triangulation->Triangle(t).Get(i1, i2, i3);
                if (reversed) {
                    std::swap(i2, i3);  // Außen-Orientierung erhalten
                }
                const gp_Pnt a =
                    triangulation->Node(i1).Transformed(location.Transformation());
                const gp_Pnt b =
                    triangulation->Node(i2).Transformed(location.Transformation());
                const gp_Pnt c =
                    triangulation->Node(i3).Transformed(location.Transformation());
                appendTriangle(mesh, a, b, c);
            }
        }
        if (mesh.empty()) {
            throw std::runtime_error(
                "OccGeometryAdapter: Tessellation lieferte kein Netz (E-GEO-002)");
        }
        return mesh;
    } catch (const Standard_Failure& failure) {
        throw std::runtime_error(
            std::string("OccGeometryAdapter: Tessellation fehlgeschlagen (E-GEO-002): ") +
            failure.GetMessageString());
    }
}

}  // namespace bcad::adapters::geometry
