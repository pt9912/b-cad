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

// Volumen eines Öffnungs-Schnittkörpers, das TATSÄCHLICH aus der Wand
// entfernt wird: Grundfläche × Höhen-Überlappung mit der Wand
// `[0, height_mm]` (Oberkante auf Wandhöhe geklemmt, LH-FA-WIN-004).
// Exaktes Orakel zum flush-genauen Kern-Schnittkörper (opening_geometry).
inline double cutVolume(const hexagon::model::CutPrism& cut, double height_mm) {
    const double z0 = std::max(0.0, cut.z_min_mm);
    const double z1 = std::min(height_mm, cut.z_max_mm);
    if (z1 <= z0) {
        return 0.0;
    }
    return shoelaceArea(cut.polygon) * (z1 - z0);
}

// Netto-Wandvolumen: Footprint-Extrusion minus Öffnungs-Schnittkörper
// (LH-FA-DOR-004/WIN-005). Leere Cutouts ⇒ identisch zum Stand vor
// slice-013b (Regressions-Orakel).
inline double analyticVolume(const hexagon::model::Footprint& footprint,
                             double height_mm,
                             const std::vector<hexagon::model::CutPrism>& cutouts) {
    double volume = shoelaceArea(footprint) * height_mm;
    for (const hexagon::model::CutPrism& cut : cutouts) {
        volume -= cutVolume(cut, height_mm);
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
