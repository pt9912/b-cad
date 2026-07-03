#include "hexagon/services/wall_footprint.h"

#include <cmath>
#include <optional>

#include "hexagon/model/constants.h"

namespace bcad::hexagon::services {

namespace {

struct Vec2 {
    double x{};
    double y{};
};

Vec2 direction(model::Point2D from, model::Point2D to) {
    return Vec2{to.x_mm - from.x_mm, to.y_mm - from.y_mm};
}

double length(Vec2 v) { return std::sqrt((v.x * v.x) + (v.y * v.y)); }

Vec2 normalized(Vec2 v) {
    const double len = length(v);
    return Vec2{v.x / len, v.y / len};
}

Vec2 leftNormal(Vec2 v) { return Vec2{-v.y, v.x}; }

double cross(Vec2 a, Vec2 b) { return (a.x * b.y) - (a.y * b.x); }

bool samePoint(model::Point2D a, model::Point2D b) {
    return std::abs(a.x_mm - b.x_mm) < model::kGeometryToleranceMm &&
           std::abs(a.y_mm - b.y_mm) < model::kGeometryToleranceMm;
}

// Schnitt zweier Geraden (Punkt + Richtung); leer bei (nahezu)
// parallelen Richtungen.
std::optional<model::Point2D> intersect(model::Point2D p, Vec2 d,
                                        model::Point2D q, Vec2 e) {
    const double denom = cross(d, e);
    if (std::abs(denom) < 1e-9) {
        return std::nullopt;
    }
    const Vec2 pq = direction(p, q);
    const double t = cross(pq, e) / denom;
    return model::Point2D{p.x_mm + (t * d.x), p.y_mm + (t * d.y)};
}

model::Point2D offsetPoint(model::Point2D p, Vec2 n, double dist) {
    return model::Point2D{p.x_mm + (n.x * dist), p.y_mm + (n.y * dist)};
}

// Die genau-eine Nachbar-Wand am Punkt `corner` (gleiches Geschoss,
// Toleranz) — leer bei Grad ≠ 2 (LH-FA-WAL-006-Abgrenzung).
const model::Wall* soleNeighborAt(const model::Wall& wall,
                                  model::Point2D corner,
                                  const std::vector<model::Wall>& walls) {
    const model::Wall* neighbor = nullptr;
    for (const model::Wall& other : walls) {
        if (other.id == wall.id || other.storey_id != wall.storey_id) {
            continue;
        }
        if (!samePoint(other.start, corner) && !samePoint(other.end, corner)) {
            continue;
        }
        if (neighbor != nullptr) {
            return nullptr;  // Grad >= 3 → stumpf
        }
        neighbor = &other;
    }
    return neighbor;
}

// Eck-Konstruktion an einem Wand-Ende (spez. §1): Seitenkanten beider
// Wände im Schnittpunkt verbunden; Begrenzung WALL_MITER_LIMIT =
// max(Stärken) über den Endpunkt hinaus, sonst (und bei kollinearen
// Richtungen) Rückfall auf stumpfes Ende.
struct CornerPair {
    model::Point2D left;
    model::Point2D right;
};

CornerPair cornerAt(const model::Wall& wall, model::Point2D corner,
                    model::Point2D far_end,
                    const std::vector<model::Wall>& walls) {
    const Vec2 u = normalized(direction(far_end, corner));  // zeigt auf Ecke
    const Vec2 nu = leftNormal(u);
    const double hu = wall.thickness_mm * 0.5;
    const CornerPair butt{offsetPoint(corner, nu, hu),
                          offsetPoint(corner, nu, -hu)};

    const model::Wall* nb = soleNeighborAt(wall, corner, walls);
    if (nb == nullptr) {
        return butt;
    }
    const model::Point2D nb_far =
        samePoint(nb->start, corner) ? nb->end : nb->start;
    const Vec2 v = normalized(direction(corner, nb_far));  // zeigt von Ecke weg
    const Vec2 nv = leftNormal(v);
    const double hv = nb->thickness_mm * 0.5;

    const auto left = intersect(offsetPoint(corner, nu, hu), u,
                                offsetPoint(corner, nv, hv), v);
    const auto right = intersect(offsetPoint(corner, nu, -hu), u,
                                 offsetPoint(corner, nv, -hv), v);
    if (!left.has_value() || !right.has_value()) {
        return butt;  // kollinear (gleiche oder ungleiche Stärke) → stumpf
    }
    // Begrenzung (spez. §3 WALL_MITER_LIMIT): höchstens die größere der
    // beiden Wandstärken über den gemeinsamen Endpunkt hinaus.
    const double limit = std::max(wall.thickness_mm, nb->thickness_mm);
    if (length(direction(corner, *left)) > limit ||
        length(direction(corner, *right)) > limit) {
        return butt;  // Spitzwinkel → kein Eck-Sporn
    }
    return CornerPair{*left, *right};
}

}  // namespace

model::Footprint buttFootprint(const model::Wall& wall) {
    const Vec2 d = direction(wall.start, wall.end);
    const double len = length(d);
    if (!std::isfinite(len) || len < model::kGeometryToleranceMm) {
        // Degeneriertes Segment: kollabiertes Polygon — der
        // Geometrie-Adapter meldet das als E-GEO-002.
        return model::Footprint{{wall.start, wall.end, wall.start, wall.end}};
    }
    const Vec2 n = leftNormal(normalized(d));
    const double h = wall.thickness_mm * 0.5;
    return model::Footprint{{offsetPoint(wall.start, n, h),
                             offsetPoint(wall.end, n, h),
                             offsetPoint(wall.end, n, -h),
                             offsetPoint(wall.start, n, -h)}};
}

model::Footprint wallFootprint(const model::Wall& wall,
                               const std::vector<model::Wall>& walls) {
    const Vec2 d = direction(wall.start, wall.end);
    const double len = length(d);
    if (!std::isfinite(len) || len < model::kGeometryToleranceMm) {
        return buttFootprint(wall);
    }
    // Ende-Ecke (bei wall.end): u zeigt start→end auf die Ecke.
    const CornerPair at_end = cornerAt(wall, wall.end, wall.start, walls);
    // Start-Ecke: u zeigt end→start; deren "links" ist die andere Seite.
    const CornerPair at_start = cornerAt(wall, wall.start, wall.end, walls);

    // Umlauf: start-links → end-links → end-rechts → start-rechts,
    // bezogen auf die Richtung start→end. `at_start` ist relativ zur
    // Gegenrichtung berechnet — links/rechts tauschen.
    return model::Footprint{
        {at_start.right, at_end.left, at_end.right, at_start.left}};
}

}  // namespace bcad::hexagon::services
