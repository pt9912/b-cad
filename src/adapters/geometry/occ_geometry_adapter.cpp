// Driven-Adapter (Geometrie) — OpenCascade-Implementierung des
// `GeometryKernelPort` (ADR-0002). OCC-Typen sind hier gekapselt; nach
// außen gehen nur `model::Solid`/`model::TriangleMesh` (neutral).
// Seit slice-012 (LH-FA-WAL-006-Teilumfang) liefert der Kern das
// Grundriss-Polygon (`model::Footprint`) — der Adapter extrudiert es
// per Prisma in +Z (Volumen über `BRepGProp`, LH-FA-D3-001) bzw.
// tesselliert es (`BRepMesh_IncrementalMesh`, Flat-Shading-Layout,
// ADR-0009 (b)).

#include "adapters/geometry/occ_geometry_adapter.h"

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <BRepAlgoAPI_Cut.hxx>
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
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#include "hexagon/model/constants.h"

namespace bcad::adapters::geometry {

namespace model = hexagon::model;

namespace {

// Shoelace-Fläche (vorzeichenlos) — Degenerations-Prüfung des Polygons.
double polygonArea(const model::Footprint& footprint) {
    double twice_area = 0.0;
    const std::size_t n = footprint.points.size();
    for (std::size_t i = 0; i < n; ++i) {
        const model::Point2D& a = footprint.points[i];
        const model::Point2D& b = footprint.points[(i + 1) % n];
        twice_area += (a.x_mm * b.y_mm) - (b.x_mm * a.y_mm);
    }
    return std::abs(twice_area) * 0.5;
}

bool isDegenerate(const model::Footprint& footprint, double height_mm) {
    if (footprint.points.size() < 3 || !std::isfinite(height_mm) ||
        height_mm <= 0.0) {
        return true;
    }
    for (const model::Point2D& p : footprint.points) {
        if (!std::isfinite(p.x_mm) || !std::isfinite(p.y_mm)) {
            return true;
        }
    }
    return polygonArea(footprint) <
           model::kGeometryToleranceMm * model::kGeometryToleranceMm;
}

// Grundriss-Polygon als Prisma — gemeinsamer Solid-Builder für
// Extrusion (Volumen) und Tessellation. Wirft `std::runtime_error`
// (E-GEO-002) bei degeneriertem Polygon.
TopoDS_Shape makeFootprintSolid(const model::Footprint& footprint,
                                double height_mm) {
    if (isDegenerate(footprint, height_mm)) {
        throw std::runtime_error(
            "OccGeometryAdapter: degeneriertes Footprint-Polygon (E-GEO-002)");
    }
    BRepBuilderAPI_MakePolygon polygon;
    for (const model::Point2D& p : footprint.points) {
        polygon.Add(gp_Pnt(p.x_mm, p.y_mm, 0.0));
    }
    polygon.Close();
    BRepBuilderAPI_MakeFace face(polygon.Wire());
    const gp_Vec extrusion(0.0, 0.0, height_mm);
    return BRepPrimAPI_MakePrism(face.Face(), extrusion).Shape();
}

// Schnitt-Körper eines Öffnungs-Prismas (ADR-0011 (b)): Polygon über
// `[z_min, z_max]`. An den Wand-Boundary-Höhen (0 / Wandhöhe) steht der
// Körper leicht über (kOvershootMm) — der Überstand liegt AUSSERHALB der
// Wand und ändert das Netto-Volumen nicht, vermeidet aber koplanare
// Deck-/Boden-Flächen beim Boolean. Leerer Shape (übersprungen) bei
// degeneriertem Prisma.
TopoDS_Shape makeCutterSolid(const model::CutPrism& cut, double wall_height_mm) {
    double z0 = cut.z_min_mm;
    double z1 = cut.z_max_mm;
    if (!std::isfinite(z0) || !std::isfinite(z1) ||
        (z1 - z0) < model::kGeometryToleranceMm ||
        cut.polygon.points.size() < 3) {
        return TopoDS_Shape{};
    }
    constexpr double kOvershootMm = 1.0;
    if (z0 <= model::kGeometryToleranceMm) {
        z0 -= kOvershootMm;
    }
    if (z1 >= wall_height_mm - model::kGeometryToleranceMm) {
        z1 += kOvershootMm;
    }
    BRepBuilderAPI_MakePolygon polygon;
    for (const model::Point2D& p : cut.polygon.points) {
        polygon.Add(gp_Pnt(p.x_mm, p.y_mm, z0));
    }
    polygon.Close();
    BRepBuilderAPI_MakeFace face(polygon.Wire());
    return BRepPrimAPI_MakePrism(face.Face(), gp_Vec(0.0, 0.0, z1 - z0)).Shape();
}

// Wand-Solid minus Öffnungs-Schnittkörper (boolesch, OCC). Total: wirft
// `std::runtime_error` (E-GEO-002) bei degeneriertem Polygon oder
// fehlgeschlagener Subtraktion — kein OCC-Typ verlässt den Adapter.
TopoDS_Shape makeNetSolid(const model::Footprint& footprint, double height_mm,
                          const std::vector<model::CutPrism>& cutouts) {
    TopoDS_Shape solid = makeFootprintSolid(footprint, height_mm);
    for (const model::CutPrism& cut : cutouts) {
        const TopoDS_Shape cutter = makeCutterSolid(cut, height_mm);
        if (cutter.IsNull()) {
            continue;
        }
        BRepAlgoAPI_Cut op;
        TopTools_ListOfShape args;
        args.Append(solid);
        TopTools_ListOfShape tools;
        tools.Append(cutter);
        op.SetArguments(args);
        op.SetTools(tools);
        op.SetFuzzyValue(model::kGeometryToleranceMm);  // koplanare Flächen
        op.Build();
        if (!op.IsDone()) {
            throw std::runtime_error(
                "OccGeometryAdapter: Wandöffnungs-Subtraktion fehlgeschlagen (E-GEO-002)");
        }
        solid = op.Shape();
    }
    return solid;
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

model::Solid OccGeometryAdapter::extrudeFootprint(
    const model::Footprint& footprint, double height_mm,
    const std::vector<model::CutPrism>& cutouts) const {
    try {
        const TopoDS_Shape solid = makeNetSolid(footprint, height_mm, cutouts);
        GProp_GProps properties;
        BRepGProp::VolumeProperties(solid, properties);
        return model::Solid{std::abs(properties.Mass())};
    } catch (const Standard_Failure& failure) {
        // OCC-Ausnahme in einen neutralen Fehler übersetzen — kein
        // OCC-Typ verlässt den Adapter (ADR-0001/0002, E-GEO-002).
        throw std::runtime_error(
            std::string("OccGeometryAdapter: Extrusion fehlgeschlagen (E-GEO-002): ") +
            failure.GetMessageString());
    }
}

model::TriangleMesh OccGeometryAdapter::tessellateFootprint(
    const model::Footprint& footprint, double height_mm,
    const std::vector<model::CutPrism>& cutouts) const {
    try {
        const TopoDS_Shape solid = makeNetSolid(footprint, height_mm, cutouts);
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
