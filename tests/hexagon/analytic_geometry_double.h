#pragma once

// Gemeinsames, deterministisches Geometrie-Double für Kern-Tests
// (ADR-0001 §Testbarkeit) — vorher dreifach dupliziert (Review L4).
// Seit slice-012 footprint-basiert: Volumen analytisch über die
// Shoelace-Fläche des Polygons × Höhe. Für den stumpfen
// Einzelwand-Footprint (Rechteck) ist das exakt das alte Orakel
// Länge · Stärke · Höhe — der echte OCC-Adapter wird gegen dieselben
// Werte (innerhalb der Toleranz) geprüft.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/solid.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"

namespace bcad::testing {

inline double shoelaceArea(const hexagon::model::Footprint& footprint) {
    double twice_area = 0.0;
    const std::size_t n = footprint.points.size();
    for (std::size_t i = 0; i < n; ++i) {
        const auto& a = footprint.points[i];
        const auto& b = footprint.points[(i + 1) % n];
        twice_area += (a.x_mm * b.y_mm) - (b.x_mm * a.y_mm);
    }
    return std::abs(twice_area) * 0.5;
}

inline double analyticVolume(const hexagon::model::Footprint& footprint,
                             double height_mm) {
    return shoelaceArea(footprint) * height_mm;
}

// Doppelte vorzeichenbehaftete Fläche eines Polygons (Shoelace).
inline double signedDoubleArea(const std::vector<hexagon::model::Point2D>& p) {
    double acc = 0.0;
    const std::size_t n = p.size();
    for (std::size_t i = 0; i < n; ++i) {
        const auto& u = p[i];
        const auto& v = p[(i + 1) % n];
        acc += (u.x_mm * v.y_mm) - (v.x_mm * u.y_mm);
    }
    return acc;
}

// Schnittpunkt der Strecke (s,e) mit der Geraden (a,b).
inline hexagon::model::Point2D lineIntersect(hexagon::model::Point2D s,
                                             hexagon::model::Point2D e,
                                             hexagon::model::Point2D a,
                                             hexagon::model::Point2D b) {
    const double a1 = e.y_mm - s.y_mm;
    const double b1 = s.x_mm - e.x_mm;
    const double c1 = (a1 * s.x_mm) + (b1 * s.y_mm);
    const double a2 = b.y_mm - a.y_mm;
    const double b2 = a.x_mm - b.x_mm;
    const double c2 = (a2 * a.x_mm) + (b2 * a.y_mm);
    const double det = (a1 * b2) - (a2 * b1);
    return {((b2 * c1) - (b1 * c2)) / det, ((a1 * c2) - (a2 * c1)) / det};
}

// Fläche des Schnitts `subject ∩ clip` (Sutherland-Hodgman; `clip` muss
// konvex sein — das Cutter-Polygon ist ein Rechteck). So misst das Orakel
// das TATSÄCHLICH aus der Wand entfernte Footprint-Areal, auch wenn der
// Cutter lateral über die Wand übersteht (spez. §1) — ehrlich gegenüber
// dem, was OCC real subtrahiert.
inline double intersectionArea(std::vector<hexagon::model::Point2D> subject,
                               std::vector<hexagon::model::Point2D> clip) {
    if (signedDoubleArea(clip) < 0.0) {
        std::reverse(clip.begin(), clip.end());  // Clip auf CCW
    }
    const std::size_t m = clip.size();
    for (std::size_t i = 0; i < m && !subject.empty(); ++i) {
        const auto a = clip[i];
        const auto b = clip[(i + 1) % m];
        const auto inside = [&](hexagon::model::Point2D pt) {
            return (((b.x_mm - a.x_mm) * (pt.y_mm - a.y_mm)) -
                    ((b.y_mm - a.y_mm) * (pt.x_mm - a.x_mm))) >= -1e-9;
        };
        const std::vector<hexagon::model::Point2D> in = subject;
        subject.clear();
        const std::size_t k = in.size();
        for (std::size_t j = 0; j < k; ++j) {
            const auto cur = in[j];
            const auto prev = in[(j + k - 1) % k];
            if (inside(cur)) {
                if (!inside(prev)) {
                    subject.push_back(lineIntersect(prev, cur, a, b));
                }
                subject.push_back(cur);
            } else if (inside(prev)) {
                subject.push_back(lineIntersect(prev, cur, a, b));
            }
        }
    }
    return std::abs(signedDoubleArea(subject)) * 0.5;
}

// Volumen, das eine Öffnung TATSÄCHLICH aus der Wand entfernt:
// (Footprint ∩ Cutter-Polygon) × Höhen-Überlappung mit `[0, height_mm]`
// (Oberkante auf Wandhöhe, LH-FA-WIN-004; lateraler/Boundary-Überstand
// des Cutters wird durch den Schnitt mit dem Footprint herausgeklemmt).
inline double cutVolume(const hexagon::model::Footprint& footprint,
                        const hexagon::model::CutPrism& cut, double height_mm) {
    const double z0 = std::max(0.0, cut.z_min_mm);
    const double z1 = std::min(height_mm, cut.z_max_mm);
    if (z1 <= z0) {
        return 0.0;
    }
    return intersectionArea(footprint.points, cut.polygon.points) * (z1 - z0);
}

// Netto-Wandvolumen: Footprint-Extrusion minus Öffnungs-Schnittkörper
// (LH-FA-DOR-004/WIN-005). Leere Cutouts ⇒ identisch zum Stand vor
// slice-013b (Regressions-Orakel).
inline double analyticVolume(const hexagon::model::Footprint& footprint,
                             double height_mm,
                             const std::vector<hexagon::model::CutPrism>& cutouts) {
    double volume = shoelaceArea(footprint) * height_mm;
    for (const hexagon::model::CutPrism& cut : cutouts) {
        volume -= cutVolume(footprint, cut, height_mm);
    }
    return volume;
}

// Altes Einzelwand-Orakel (Länge · Stärke · Höhe) — für Tests, die eine
// freistehende Wand prüfen (stumpfer Rechteck-Footprint).
inline double analyticVolume(const hexagon::model::Wall& w) {
    const double dx = w.end.x_mm - w.start.x_mm;
    const double dy = w.end.y_mm - w.start.y_mm;
    const double length = std::sqrt((dx * dx) + (dy * dy));
    return length * w.thickness_mm * w.height_mm;
}

class AnalyticGeometry final : public hexagon::ports::driven::GeometryKernelPort {
public:
    hexagon::model::Solid extrudeFootprint(
        const hexagon::model::Footprint& footprint, double height_mm,
        const std::vector<hexagon::model::CutPrism>& cutouts) const override {
        return hexagon::model::Solid{
            analyticVolume(footprint, height_mm, cutouts)};
    }

    // Deterministisches Minimal-Netz: Boden-Polygon-Fächer + eine
    // Höhen-Kante — die Kern-Tests prüfen Benachrichtigungs-/Query-Fluss
    // und Bounding-Boxen, nicht die Tessellations-Qualität (die testet
    // der OCC-Adapter-Test).
    hexagon::model::TriangleMesh tessellateFootprint(
        const hexagon::model::Footprint& footprint, double height_mm,
        const std::vector<hexagon::model::CutPrism>& /*cutouts*/) const override {
        // Surrogat-Netz: Bounding-Box-tragend; die Öffnungs-Geometrie
        // prüft der OCC-Adapter-Test, nicht das Kern-Double.
        hexagon::model::TriangleMesh mesh;
        const std::size_t n = footprint.points.size();
        for (std::size_t i = 1; i + 1 < n; ++i) {
            for (const std::size_t k : {std::size_t{0}, i, i + 1}) {
                const auto& p = footprint.points[k];
                mesh.indices.push_back(mesh.vertexCount());
                mesh.positions.insert(mesh.positions.end(),
                                      {p.x_mm, p.y_mm, 0.0});
                mesh.normals.insert(mesh.normals.end(), {0.0, 0.0, 1.0});
            }
        }
        // Höhen-Kante, damit die Netz-Bounding-Box die Extrusion trägt.
        if (!footprint.points.empty()) {
            const auto& p = footprint.points.front();
            for (const double z : {0.0, height_mm, height_mm}) {
                mesh.indices.push_back(mesh.vertexCount());
                mesh.positions.insert(mesh.positions.end(),
                                      {p.x_mm, p.y_mm, z});
                mesh.normals.insert(mesh.normals.end(), {0.0, 0.0, 1.0});
            }
        }
        return mesh;
    }
};

}  // namespace bcad::testing
