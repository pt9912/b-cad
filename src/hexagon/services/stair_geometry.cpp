#include "hexagon/services/stair_geometry.h"

#include <algorithm>
#include <cmath>

#include "hexagon/model/constants.h"

namespace bcad::hexagon::services {

namespace {

// Geländer-Streifenstärke (Render-Detail, kein spez. Parameter): dünn an der
// Lauf-Seite, gegen sehr schmale Treppen auf width/4 begrenzt.
constexpr double kRailThicknessMm = 50.0;

struct V3 {
    double x;
    double y;
    double z;
};

// Flat-Shading-Dreieck (eigene Vertices + Flächennormale; triangle_mesh.h).
// Degenerierte Dreiecke (kollinear) werden übersprungen. Muster roof_geometry.
void appendTriangle(model::TriangleMesh& mesh, const V3& a, const V3& b,
                    const V3& c) {
    const double ux = b.x - a.x;
    const double uy = b.y - a.y;
    const double uz = b.z - a.z;
    const double vx = c.x - a.x;
    const double vy = c.y - a.y;
    const double vz = c.z - a.z;
    double nx = (uy * vz) - (uz * vy);
    double ny = (uz * vx) - (ux * vz);
    double nz = (ux * vy) - (uy * vx);
    const double mag = std::sqrt((nx * nx) + (ny * ny) + (nz * nz));
    if (mag < 1e-9) {
        return;
    }
    nx /= mag;
    ny /= mag;
    nz /= mag;
    for (const V3& p : {a, b, c}) {
        mesh.indices.push_back(mesh.vertexCount());
        mesh.positions.insert(mesh.positions.end(), {p.x, p.y, p.z});
        mesh.normals.insert(mesh.normals.end(), {nx, ny, nz});
    }
}

void appendQuad(model::TriangleMesh& mesh, const V3& a, const V3& b, const V3& c,
                const V3& d) {
    appendTriangle(mesh, a, b, c);
    appendTriangle(mesh, a, c, d);
}

// Achsen-paralleler Quader [x0,x1]×[y0,y1]×[z0,z1] als 6 Außenflächen
// (Flat-Shading, nach außen gewickelt).
void appendBox(model::TriangleMesh& mesh, double x0, double x1, double y0,
               double y1, double z0, double z1) {
    const V3 a{x0, y0, z0};
    const V3 b{x1, y0, z0};
    const V3 c{x1, y1, z0};
    const V3 d{x0, y1, z0};
    const V3 e{x0, y0, z1};
    const V3 f{x1, y0, z1};
    const V3 g{x1, y1, z1};
    const V3 h{x0, y1, z1};
    appendQuad(mesh, e, f, g, h);  // +z (oben)
    appendQuad(mesh, a, d, c, b);  // -z (unten)
    appendQuad(mesh, a, b, f, e);  // -y (vorn)
    appendQuad(mesh, d, h, g, c);  // +y (hinten)
    appendQuad(mesh, b, c, g, f);  // +x (rechts)
    appendQuad(mesh, a, e, h, d);  // -x (links)
}

}  // namespace

double stairRiseMm(const model::Stair& stair, double from_storey_height_mm) {
    if (stair.step_count <= 0) {
        return 0.0;
    }
    return from_storey_height_mm / stair.step_count;
}

double stairRunLengthMm(const model::Stair& stair) {
    if (stair.step_count < 1 || !std::isfinite(stair.tread_mm)) {
        return 0.0;  // Totalitäts-Kontrakt wie stairMesh/stairRiseMm (M1)
    }
    return stair.step_count * stair.tread_mm;
}

model::TriangleMesh stairMesh(const model::Stair& stair,
                              double from_storey_height_mm) {
    model::TriangleMesh mesh;

    const double width = stair.width_mm;
    const double tread = stair.tread_mm;
    const int steps = stair.step_count;
    if (steps < 1 || !std::isfinite(width) || !std::isfinite(tread) ||
        !std::isfinite(from_storey_height_mm) || !std::isfinite(stair.start.x_mm) ||
        !std::isfinite(stair.start.y_mm) || width < model::kGeometryToleranceMm ||
        tread < model::kGeometryToleranceMm ||
        from_storey_height_mm < model::kGeometryToleranceMm) {
        return mesh;  // degeneriert → leeres Netz (total)
    }

    const double rise = from_storey_height_mm / steps;
    const double x_start = stair.start.x_mm;
    const double y0 = stair.start.y_mm;
    const double y1 = y0 + width;
    const double rail_t = std::min(kRailThicknessMm, width / 4.0);
    const double rail_h = model::kStairRailingHeightMm;

    for (int i = 0; i < steps; ++i) {
        const double sx0 = x_start + (i * tread);
        const double sx1 = x_start + ((i + 1) * tread);
        const double top = (i + 1) * rise;
        // Solide Stufe vom Boden bis zur Stufenoberkante (+x-Aufstieg).
        appendBox(mesh, sx0, sx1, y0, y1, 0.0, top);
        // Geländer beidseitig: folgt der Stufenfolge bis rail_h über die Stufe
        // (STR-004; an kleinem x vorhanden → kein Top-Spike).
        appendBox(mesh, sx0, sx1, y0, y0 + rail_t, top, top + rail_h);
        appendBox(mesh, sx0, sx1, y1 - rail_t, y1, top, top + rail_h);
    }

    return mesh;
}

}  // namespace bcad::hexagon::services
