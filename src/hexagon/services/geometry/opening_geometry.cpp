#include "hexagon/services/opening_geometry.h"

#include <algorithm>
#include <cmath>

#include "hexagon/model/constants.h"

namespace bcad::hexagon::services {

namespace {

double segmentLength(const model::Wall& wall) {
    const double dx = wall.end.x_mm - wall.start.x_mm;
    const double dy = wall.end.y_mm - wall.start.y_mm;
    return std::sqrt((dx * dx) + (dy * dy));
}

}  // namespace

std::optional<model::CutPrism> openingCutPrism(const model::Opening& opening,
                                               const model::Wall& wall) {
    const double length = segmentLength(wall);
    if (!std::isfinite(length) || length < model::kGeometryToleranceMm ||
        !std::isfinite(opening.offset_mm) || !std::isfinite(opening.width_mm) ||
        !std::isfinite(opening.height_mm) ||
        !std::isfinite(opening.sill_height_mm) ||
        opening.width_mm < model::kGeometryToleranceMm) {
        return std::nullopt;
    }

    // Höhen-Bereich innerhalb der Wand, Oberkante auf die Wandhöhe
    // geklemmt (kein Durchbruch über die Wand hinaus, LH-FA-WIN-004).
    const double z_min = std::max(0.0, opening.sill_height_mm);
    const double z_max = std::min(opening.sill_height_mm + opening.height_mm,
                                  wall.height_mm);
    if (z_max - z_min < model::kGeometryToleranceMm) {
        return std::nullopt;  // Brüstung ≥ Wandhöhe o. ä. — kein Schnitt
    }

    // Überstand über die Wandgrenzen (spez. §1): lateral je Seite und an
    // den Boundary-Höhen (Standfläche/Wandkrone) — vermeidet koplanare
    // Flächen beim Boolean. Liegt außerhalb der Wand → das real entfernte
    // Volumen (Footprint ∩ Cutter, z ∩ [0, Wandhöhe]) bleibt unverändert.
    const double ov = model::kOpeningCutOvershootMm;
    const double z0 = (z_min <= model::kGeometryToleranceMm) ? z_min - ov : z_min;
    const double z1 =
        (z_max >= wall.height_mm - model::kGeometryToleranceMm) ? z_max + ov : z_max;

    // Achsen-Einheitsvektor d und Quer-Normale n der Wand.
    const double dir_x = (wall.end.x_mm - wall.start.x_mm) / length;
    const double dir_y = (wall.end.y_mm - wall.start.y_mm) / length;
    const double nrm_x = -dir_y;
    const double nrm_y = dir_x;
    const double half = (wall.thickness_mm * 0.5) + ov;  // lateraler Überstand

    // Begrenzung der Öffnung auf die Wandlänge (defensiv — der Service
    // klemmt die Position bereits; opening_geometry bleibt dennoch total).
    const double near = std::clamp(opening.offset_mm, 0.0, length);
    const double far = std::clamp(opening.offset_mm + opening.width_mm, near,
                                  length);
    if (far - near < model::kGeometryToleranceMm) {
        return std::nullopt;
    }

    const auto corner = [&](double along, double across) {
        return model::Point2D{
            wall.start.x_mm + (dir_x * along) + (nrm_x * across),
            wall.start.y_mm + (dir_y * along) + (nrm_y * across)};
    };

    model::CutPrism prism;
    prism.polygon.points = {corner(near, half), corner(far, half),
                            corner(far, -half), corner(near, -half)};
    prism.z_min_mm = z0;
    prism.z_max_mm = z1;
    return prism;
}

std::vector<model::CutPrism> wallCutPrisms(
    const model::Wall& wall, const std::vector<model::Opening>& openings) {
    std::vector<model::CutPrism> prisms;
    for (const model::Opening& opening : openings) {
        if (opening.wall_id != wall.id) {
            continue;
        }
        if (std::optional<model::CutPrism> prism = openingCutPrism(opening, wall)) {
            prisms.push_back(std::move(*prism));
        }
    }
    return prisms;
}

}  // namespace bcad::hexagon::services
