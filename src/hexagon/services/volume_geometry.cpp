#include "hexagon/services/volume_geometry.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "hexagon/model/constants.h"
#include "hexagon/services/slab_geometry.h"   // cutoutInsideSlab
#include "hexagon/services/wall_footprint.h"  // wallFootprint

namespace bcad::hexagon::services {

double polygonArea(const model::Footprint& footprint) {
    double twice = 0.0;
    const std::size_t n = footprint.points.size();
    for (std::size_t i = 0; i < n; ++i) {
        const model::Point2D& a = footprint.points[i];
        const model::Point2D& b = footprint.points[(i + 1) % n];
        twice += (a.x_mm * b.y_mm) - (b.x_mm * a.y_mm);
    }
    return std::abs(twice) * 0.5;
}

namespace {

// Real entferntes, geklemmtes Volumen einer Öffnung (spez. §1 EVL-001.a /
// LH-FA-WIN-004): width · Wandstärke · clamped_height,
// clamped_height = min(sill+height, Wandhöhe) − max(0, sill). Reicht die
// Öffnung nicht in [0, Wandhöhe] (z. B. Brüstung über Wandhöhe), ist das
// real entfernte Volumen 0. Quer belegt die Öffnung stets die volle
// Wandstärke (kein eigenes thickness-Feld, ADR-0011).
double clampedOpeningVolumeMm3(const model::Opening& opening,
                               const model::Wall& wall) {
    const double z_min = std::max(0.0, opening.sill_height_mm);
    const double z_max =
        std::min(opening.sill_height_mm + opening.height_mm, wall.height_mm);
    const double clamped_height = z_max - z_min;
    if (clamped_height <= 0.0) {
        return 0.0;
    }
    return opening.width_mm * wall.thickness_mm * clamped_height;
}

}  // namespace

double wallNetVolumeMm3(const model::Wall& wall,
                        const std::vector<model::Wall>& walls,
                        const std::vector<model::Opening>& openings) {
    const double gross =
        polygonArea(wallFootprint(wall, walls)) * wall.height_mm;
    double removed = 0.0;
    for (const model::Opening& o : openings) {
        if (o.wall_id == wall.id) {
            removed += clampedOpeningVolumeMm3(o, wall);
        }
    }
    return std::max(0.0, gross - removed);
}

double slabNetVolumeMm3(const model::Slab& slab) {
    double net_area = polygonArea(slab.footprint);
    for (const model::Footprint& cut : slab.cutouts) {
        if (cutoutInsideSlab(slab, cut)) {
            net_area -= polygonArea(cut);
        }
    }
    return std::max(0.0, net_area) * slab.thickness_mm;
}

double stairNetVolumeMm3(const model::Stair& stair,
                         double from_storey_height_mm) {
    const double width = stair.width_mm;
    const double tread = stair.tread_mm;
    const int steps = stair.step_count;
    if (steps < 1 || !std::isfinite(width) || !std::isfinite(tread) ||
        !std::isfinite(from_storey_height_mm) ||
        width < model::kGeometryToleranceMm ||
        tread < model::kGeometryToleranceMm ||
        from_storey_height_mm < model::kGeometryToleranceMm) {
        return 0.0;  // Totalität (Muster stairMesh/stairRiseMm)
    }
    // Σ_{i=1..n} (tread · width · i · rise), rise = h / n
    //   = tread · width · rise · n(n+1)/2 = tread · width · h · (n+1)/2.
    return tread * width * from_storey_height_mm *
           ((static_cast<double>(steps) + 1.0) / 2.0);
}

}  // namespace bcad::hexagon::services
