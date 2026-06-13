#include "hexagon/services/roof_geometry.h"

#include <array>
#include <cmath>

#include "hexagon/model/constants.h"

namespace bcad::hexagon::services {

namespace {

constexpr double kPi = 3.14159265358979323846;

struct V3 {
    double x;
    double y;
    double z;
};

// Hängt ein Dreieck im Flat-Shading-Layout an (eigene Vertices +
// Flächennormale; triangle_mesh.h-Konvention). Degenerierte Dreiecke
// (kollineare Punkte) werden übersprungen.
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

void appendQuad(model::TriangleMesh& mesh, const V3& a, const V3& b,
                const V3& c, const V3& d) {
    appendTriangle(mesh, a, b, c);
    appendTriangle(mesh, a, c, d);
}

}  // namespace

model::TriangleMesh roofMesh(const model::Roof& roof) {
    model::TriangleMesh mesh;

    const double o = roof.overhang_mm;
    const double x0 = roof.origin.x_mm - o;
    const double x1 = roof.origin.x_mm + roof.width_mm + o;
    const double y0 = roof.origin.y_mm - o;
    const double y1 = roof.origin.y_mm + roof.depth_mm + o;
    const double bx = x1 - x0;  // Trauf-Ausdehnung x (B_eaves)
    const double ty = y1 - y0;  // Trauf-Ausdehnung y (T_eaves)
    const double z0 = roof.base_z_mm;

    if (!std::isfinite(bx) || !std::isfinite(ty) || !std::isfinite(z0) ||
        !std::isfinite(o) || bx < model::kGeometryToleranceMm ||
        ty < model::kGeometryToleranceMm || !std::isfinite(roof.pitch_deg)) {
        return mesh;  // degeneriert → leeres Netz (total)
    }
    const double tan_p = std::tan(roof.pitch_deg * kPi / 180.0);
    if (!std::isfinite(tan_p) || tan_p <= 0.0) {
        return mesh;
    }

    // Trauf-Ecken (z = Traufhöhe).
    const V3 corner_a{x0, y0, z0};
    const V3 corner_b{x1, y0, z0};
    const V3 corner_c{x1, y1, z0};
    const V3 corner_d{x0, y1, z0};

    switch (roof.type) {
        case model::RoofType::Pult: {
            // Eine geneigte Fläche: Niedrigkante bei y0, Hochkante bei y1
            // mit Höhe (t+2o)·tan(p).
            const double high = z0 + (ty * tan_p);
            const V3 high_d{x0, y1, high};
            const V3 high_c{x1, y1, high};
            appendQuad(mesh, corner_a, corner_b, high_c, high_d);
            break;
        }
        case model::RoofType::Sattel: {
            // First mittig entlang der längeren Trauf-Achse; zwei Flächen.
            if (bx >= ty) {
                const double z_ridge = z0 + ((ty / 2.0) * tan_p);
                const double y_mid = (y0 + y1) / 2.0;
                const V3 r1{x0, y_mid, z_ridge};
                const V3 r2{x1, y_mid, z_ridge};
                appendQuad(mesh, corner_a, corner_b, r2, r1);  // y0-Fläche
                appendQuad(mesh, corner_c, corner_d, r1, r2);  // y1-Fläche
            } else {
                const double z_ridge = z0 + ((bx / 2.0) * tan_p);
                const double x_mid = (x0 + x1) / 2.0;
                const V3 r1{x_mid, y0, z_ridge};
                const V3 r2{x_mid, y1, z_ridge};
                appendQuad(mesh, corner_b, corner_c, r2, r1);  // x1-Fläche
                appendQuad(mesh, corner_d, corner_a, r1, r2);  // x0-Fläche
            }
            break;
        }
        case model::RoofType::Walm: {
            // Wie Sattel, zusätzlich an den Giebelseiten abgewalmt; First
            // um den Einrückbetrag = halbe kürzere Trauf-Seite kürzer.
            if (bx >= ty) {
                const double z_ridge = z0 + ((ty / 2.0) * tan_p);
                const double y_mid = (y0 + y1) / 2.0;
                const double inset = ty / 2.0;
                double rx0 = x0 + inset;
                double rx1 = x1 - inset;
                if (rx1 < rx0) {  // (nahezu) quadratisch → Zeltdach (First = Punkt)
                    rx0 = (x0 + x1) / 2.0;
                    rx1 = rx0;
                }
                const V3 r1{rx0, y_mid, z_ridge};
                const V3 r2{rx1, y_mid, z_ridge};
                appendQuad(mesh, corner_a, corner_b, r2, r1);  // y0-Trapez
                appendQuad(mesh, corner_c, corner_d, r1, r2);  // y1-Trapez
                appendTriangle(mesh, corner_d, corner_a, r1);  // x0-Walm
                appendTriangle(mesh, corner_b, corner_c, r2);  // x1-Walm
            } else {
                const double z_ridge = z0 + ((bx / 2.0) * tan_p);
                const double x_mid = (x0 + x1) / 2.0;
                const double inset = bx / 2.0;
                double ry0 = y0 + inset;
                double ry1 = y1 - inset;
                if (ry1 < ry0) {
                    ry0 = (y0 + y1) / 2.0;
                    ry1 = ry0;
                }
                const V3 r1{x_mid, ry0, z_ridge};
                const V3 r2{x_mid, ry1, z_ridge};
                appendQuad(mesh, corner_b, corner_c, r2, r1);  // x1-Trapez
                appendQuad(mesh, corner_d, corner_a, r1, r2);  // x0-Trapez
                appendTriangle(mesh, corner_a, corner_b, r1);  // y0-Walm
                appendTriangle(mesh, corner_c, corner_d, r2);  // y1-Walm
            }
            break;
        }
    }
    return mesh;
}

}  // namespace bcad::hexagon::services
