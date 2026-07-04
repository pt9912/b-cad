// Geteilte OCC-Solid-Konstruktion (slice-020b: aus occ_geometry_adapter.cpp
// ausgelagert, damit STEP-Export und Volumen/Tessellation denselben Builder
// nutzen — keine Duplikation). OCC-Typen sind hier gekapselt.

#include "adapters/geometry/occ_solids.h"

#include <cmath>
#include <stdexcept>
#include <vector>

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
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

// Grundriss-Polygon als Prisma — gemeinsamer Solid-Builder. Wirft
// `std::runtime_error` (E-GEO-002) bei degeneriertem Polygon.
TopoDS_Shape makeFootprintSolid(const model::Footprint& footprint,
                                double height_mm) {
    if (isDegenerate(footprint, height_mm)) {
        throw std::runtime_error(
            "OccSolids: degeneriertes Footprint-Polygon (E-GEO-002)");
    }
    BRepBuilderAPI_MakePolygon polygon;
    for (const model::Point2D& p : footprint.points) {
        polygon.Add(gp_Pnt(p.x_mm, p.y_mm, 0.0));
    }
    polygon.Close();
    const BRepBuilderAPI_MakeFace face(polygon.Wire());
    const gp_Vec extrusion(0.0, 0.0, height_mm);
    return BRepPrimAPI_MakePrism(face.Face(), extrusion).Shape();
}

// Schnitt-Körper eines Öffnungs-Prismas (ADR-0011 (b)): das vom Kern gelieferte
// Polygon extrudiert über `[z_min, z_max]`. Leerer Shape bei degeneriertem
// Prisma (übersprungen).
TopoDS_Shape makeCutterSolid(const model::CutPrism& cut) {
    const double z0 = cut.z_min_mm;
    const double z1 = cut.z_max_mm;
    if (!std::isfinite(z0) || !std::isfinite(z1) ||
        (z1 - z0) < model::kGeometryToleranceMm ||
        cut.polygon.points.size() < 3) {
        return TopoDS_Shape{};
    }
    BRepBuilderAPI_MakePolygon polygon;
    for (const model::Point2D& p : cut.polygon.points) {
        polygon.Add(gp_Pnt(p.x_mm, p.y_mm, z0));
    }
    polygon.Close();
    const BRepBuilderAPI_MakeFace face(polygon.Wire());
    return BRepPrimAPI_MakePrism(face.Face(), gp_Vec(0.0, 0.0, z1 - z0)).Shape();
}

}  // namespace

TopoDS_Shape makeNetSolid(const model::Footprint& footprint, double height_mm,
                          const std::vector<model::CutPrism>& cutouts) {
    TopoDS_Shape solid = makeFootprintSolid(footprint, height_mm);
    for (const model::CutPrism& cut : cutouts) {
        const TopoDS_Shape cutter = makeCutterSolid(cut);
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
        op.SetFuzzyValue(model::kGeometryToleranceMm);  // Rest-Robustheit
        op.Build();
        if (!op.IsDone() || op.Shape().IsNull()) {
            throw std::runtime_error(
                "OccSolids: Wandöffnungs-Subtraktion fehlgeschlagen (E-GEO-002)");
        }
        solid = op.Shape();
    }
    return solid;
}

TopoDS_Shape meshToSolid(const model::TriangleMesh& mesh) {
    if (mesh.empty()) {
        return TopoDS_Shape{};  // leeres Netz → leeres Shape (Aufrufer überspringt)
    }

    // Je Dreieck eine Face; koinzidente Kanten (Flat-Shading ohne geteilte
    // Vertices) verschmilzt Sewing auf das µm-kanonische Raster (023b) bei
    // Toleranz `kGeometryToleranceMm`.
    BRepBuilderAPI_Sewing sewing(model::kGeometryToleranceMm);
    const std::vector<double>& p = mesh.positions;
    const std::vector<int>& idx = mesh.indices;
    for (std::size_t t = 0; t + 3 <= idx.size(); t += 3) {
        const std::size_t i0 = static_cast<std::size_t>(idx[t]) * 3;
        const std::size_t i1 = static_cast<std::size_t>(idx[t + 1]) * 3;
        const std::size_t i2 = static_cast<std::size_t>(idx[t + 2]) * 3;
        BRepBuilderAPI_MakePolygon poly(gp_Pnt(p[i0], p[i0 + 1], p[i0 + 2]),
                                        gp_Pnt(p[i1], p[i1 + 1], p[i1 + 2]),
                                        gp_Pnt(p[i2], p[i2 + 1], p[i2 + 2]),
                                        Standard_True);  // geschlossenes Tri-Wire
        if (!poly.IsDone()) {
            continue;  // degeneriertes Dreieck überspringen
        }
        const BRepBuilderAPI_MakeFace face(poly.Wire());
        if (face.IsDone()) {
            sewing.Add(face.Face());
        }
    }
    sewing.Perform();
    const TopoDS_Shape sewn = sewing.SewedShape();
    if (sewn.IsNull()) {
        return TopoDS_Shape{};
    }

    // Genau **eine** Shell erwartet (ein zusammenhängender wasserdichter Körper).
    TopExp_Explorer exp(sewn, TopAbs_SHELL);
    if (!exp.More()) {
        return TopoDS_Shape{};  // keine Shell → offen → fail-closed
    }
    const TopoDS_Shell shell = TopoDS::Shell(exp.Current());
    exp.Next();
    if (exp.More()) {
        return TopoDS_Shape{};  // mehrere Shells → keine eine Solid → fail-closed
    }

    BRepBuilderAPI_MakeSolid mkSolid(shell);
    if (!mkSolid.IsDone()) {
        return TopoDS_Shape{};
    }
    const TopoDS_Shape solid = mkSolid.Solid();

    // fail-closed (HIGH-1): nur eine **gültige, geschlossene** Solid passiert —
    // eine offene/ungültige Vernähung wird verworfen (kein Schrott-Solid im STEP).
    if (!BRepCheck_Analyzer(solid).IsValid()) {
        return TopoDS_Shape{};
    }
    return solid;
}

TopoDS_Shape makeBoxSolid(double x0, double y0, double z0, double x1, double y1,
                          double z1) {
    if ((x1 - x0) < model::kGeometryToleranceMm ||
        (y1 - y0) < model::kGeometryToleranceMm ||
        (z1 - z0) < model::kGeometryToleranceMm) {
        return TopoDS_Shape{};  // degenerierter Quader → leer (übersprungen)
    }
    return BRepPrimAPI_MakeBox(gp_Pnt(x0, y0, z0), gp_Pnt(x1, y1, z1)).Shape();
}

}  // namespace bcad::adapters::geometry
