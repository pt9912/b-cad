#pragma once

// Gemeinsames, deterministisches Geometrie-Double für Kern-Tests
// (ADR-0001 §Testbarkeit) — vorher dreifach dupliziert (Review L4).
// Seit slice-012 footprint-basiert: Volumen analytisch über die
// Shoelace-Fläche des Polygons × Höhe. Für den stumpfen
// Einzelwand-Footprint (Rechteck) ist das exakt das alte Orakel
// Länge · Stärke · Höhe — der echte OCC-Adapter wird gegen dieselben
// Werte (innerhalb der Toleranz) geprüft.

#include <cmath>
#include <cstddef>

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
        const hexagon::model::Footprint& footprint,
        double height_mm) const override {
        return hexagon::model::Solid{analyticVolume(footprint, height_mm)};
    }

    // Deterministisches Minimal-Netz: Boden-Polygon-Fächer + eine
    // Höhen-Kante — die Kern-Tests prüfen Benachrichtigungs-/Query-Fluss
    // und Bounding-Boxen, nicht die Tessellations-Qualität (die testet
    // der OCC-Adapter-Test).
    hexagon::model::TriangleMesh tessellateFootprint(
        const hexagon::model::Footprint& footprint,
        double height_mm) const override {
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
