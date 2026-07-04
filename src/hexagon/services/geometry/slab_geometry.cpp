#include "hexagon/services/geometry/slab_geometry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "hexagon/model/constants.h"

namespace bcad::hexagon::services {

namespace {

// Vorzeichenlose Polygonfläche (Shoelace) — für die Degenerations-Prüfung.
double absPolygonArea(const model::Footprint& fp) {
    double twice = 0.0;
    const std::size_t n = fp.points.size();
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        twice += (fp.points[j].x_mm * fp.points[i].y_mm) -
                 (fp.points[i].x_mm * fp.points[j].y_mm);
    }
    return std::abs(twice) * 0.5;
}

// Punkt strikt im einfachen Polygon (Ray-Casting). Randlagen zählen NICHT
// als innen — ein Ausschnitt-Stützpunkt auf der Umrisskante wäre Boolean-
// koplanar und wird damit ausgeschlossen.
bool pointInPolygon(const model::Footprint& fp, model::Point2D p) {
    bool inside = false;
    const std::size_t n = fp.points.size();
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const model::Point2D& a = fp.points[i];
        const model::Point2D& b = fp.points[j];
        const bool crosses = (a.y_mm > p.y_mm) != (b.y_mm > p.y_mm);
        if (!crosses) {
            continue;
        }
        const double x_at =
            a.x_mm + ((p.y_mm - a.y_mm) * (b.x_mm - a.x_mm) / (b.y_mm - a.y_mm));
        if (p.x_mm < x_at) {
            inside = !inside;
        }
    }
    return inside;
}

}  // namespace

double slabBaseZ(const model::Slab& slab, double storey_height_mm) {
    switch (slab.type) {
        case model::SlabType::Decke:
            return storey_height_mm;  // auf der Geschoss-Oberkante
        case model::SlabType::Bodenplatte:
        case model::SlabType::Fundament:
            return -slab.thickness_mm;  // Oberkante auf Höhe 0, Dicke nach unten
    }
    return 0.0;
}

std::vector<model::CutPrism> slabCutPrisms(const model::Slab& slab) {
    std::vector<model::CutPrism> prisms;
    prisms.reserve(slab.cutouts.size());
    const double eps = model::kOpeningCutOvershootMm;
    for (const model::Footprint& cut : slab.cutouts) {
        model::CutPrism prism;
        prism.polygon = cut;
        prism.z_min_mm = -eps;                       // relativ zum Solid [0,Dicke]
        prism.z_max_mm = slab.thickness_mm + eps;    // Überstand volumen-neutral
        prisms.push_back(prism);
    }
    return prisms;
}

bool cutoutInsideSlab(const model::Slab& slab,
                      const model::Footprint& cutout) {
    if (cutout.points.size() < 3) {
        return false;
    }
    for (const model::Point2D& pt : cutout.points) {
        if (!std::isfinite(pt.x_mm) || !std::isfinite(pt.y_mm)) {
            return false;  // NaN/Inf-Stützpunkt
        }
    }
    if (absPolygonArea(cutout) <
        (model::kGeometryToleranceMm * model::kGeometryToleranceMm)) {
        return false;  // degeneriert (Fläche ~ 0)
    }
    // „Auf den Platten-Umriss begrenzt" (spez. §1 LH-FA-SLB-001.a): jeder
    // Ausschnitt-Stützpunkt muss strikt innen liegen. Damit ist der Boolean
    // koplanar-frei (innenliegendes Loch, kein lateraler Überstand nötig —
    // anders als bei der Wandöffnung, die die Wand zwangsläufig durchspannt).
    return std::ranges::all_of(cutout.points, [&](const model::Point2D& pt) {
        return pointInPolygon(slab.footprint, pt);
    });
}

model::TriangleMesh translateMeshZ(model::TriangleMesh mesh, double dz) {
    for (std::size_t i = 2; i < mesh.positions.size(); i += 3) {
        mesh.positions[i] += dz;
    }
    return mesh;
}

}  // namespace bcad::hexagon::services
