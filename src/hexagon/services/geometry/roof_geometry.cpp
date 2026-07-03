#include "hexagon/services/geometry/roof_geometry.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <map>
#include <utility>

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

// --- Oberseiten-Schale je Typ (die geneigten Dachflächen, z = Höhe) ---------
// Liefert ein **offenes** Netz der Oberseite (wie bis slice-023a); die
// Schließung zum Volumenkörper-Slab erfolgt in closeSlabDownward.
model::TriangleMesh buildTopShell(const model::Roof& roof) {
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
        ty < model::kGeometryToleranceMm || !std::isfinite(roof.pitch_deg) ||
        roof.pitch_deg <= 0.0 || roof.pitch_deg >= 90.0) {
        return mesh;  // degeneriert / nicht-bauliche Neigung → leeres Netz (total)
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
            const double high = z0 + (ty * tan_p);
            const V3 high_d{x0, y1, high};
            const V3 high_c{x1, y1, high};
            appendQuad(mesh, corner_a, corner_b, high_c, high_d);
            break;
        }
        case model::RoofType::Sattel: {
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
            if (bx >= ty) {
                const double z_ridge = z0 + ((ty / 2.0) * tan_p);
                const double y_mid = (y0 + y1) / 2.0;
                const double inset = ty / 2.0;
                double rx0 = x0 + inset;
                double rx1 = x1 - inset;
                if (rx1 <= rx0) {  // (nahezu) quadratisch → Zeltdach (First = Punkt)
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
                if (ry1 <= ry0) {  // (nahezu) quadratisch → Zeltdach (First = Punkt)
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

// --- Volumenkörper-Schluss: vertikaler Schräg-Slab der Dicke d --------------
// (slice-023b, spez. §1 LH-FA-ROF-001.a): Oberseite + um d **vertikal nach
// unten** versetzte Unterseite (umgekehrte Wicklung) + Seitenwände entlang der
// **Rand-Kanten** (Kante, die von genau EINER Oberseiten-Fläche genutzt wird).
// Ergebnis: geschlossene, außen-orientierte Mannigfaltigkeit (jede Kante von
// genau 2 Flächen). Grat/First/Hip sind innen (2 Flächen) → keine Wand.

using EdgeKey = std::array<std::int64_t, 3>;

// Koordinaten-Kanonisierung auf ein µm-Raster (Muster slice-016b): exakt
// gleiche analytische Vertices (geteilte First-/Hip-Punkte) treffen sich.
EdgeKey vertexKey(const V3& p) {
    return EdgeKey{static_cast<std::int64_t>(std::llround(p.x * 1000.0)),
                   static_cast<std::int64_t>(std::llround(p.y * 1000.0)),
                   static_cast<std::int64_t>(std::llround(p.z * 1000.0))};
}

std::pair<EdgeKey, EdgeKey> undirectedEdge(const V3& a, const V3& b) {
    const EdgeKey ka = vertexKey(a);
    const EdgeKey kb = vertexKey(b);
    return (ka < kb) ? std::make_pair(ka, kb) : std::make_pair(kb, ka);
}

V3 vertexAt(const model::TriangleMesh& top, int idx) {
    const std::size_t i = static_cast<std::size_t>(idx) * 3;
    return V3{top.positions[i], top.positions[i + 1], top.positions[i + 2]};
}

model::TriangleMesh closeSlabDownward(const model::TriangleMesh& top, double d) {
    model::TriangleMesh mesh;
    const auto down = [d](const V3& p) { return V3{p.x, p.y, p.z - d}; };

    // 1. Oberseite (unverändert) + 2. Unterseite (versetzt, umgekehrte Wicklung).
    for (std::size_t t = 0; t + 3 <= top.indices.size(); t += 3) {
        const V3 a = vertexAt(top, top.indices[t]);
        const V3 b = vertexAt(top, top.indices[t + 1]);
        const V3 c = vertexAt(top, top.indices[t + 2]);
        appendTriangle(mesh, a, b, c);                    // Oberseite (Normale oben)
        appendTriangle(mesh, down(c), down(b), down(a));  // Unterseite (Normale unten)
    }

    // 3. Rand-Kanten zählen (jede Kante einer Oberseiten-Fläche).
    std::map<std::pair<EdgeKey, EdgeKey>, int> edge_count;
    for (std::size_t t = 0; t + 3 <= top.indices.size(); t += 3) {
        const std::array<V3, 3> v = {vertexAt(top, top.indices[t]),
                                     vertexAt(top, top.indices[t + 1]),
                                     vertexAt(top, top.indices[t + 2])};
        for (int i = 0; i < 3; ++i) {
            ++edge_count[undirectedEdge(v[i], v[(i + 1) % 3])];
        }
    }

    // 4. Seitenwand je Rand-Kante (Zählung == 1), außen-orientiert: für eine
    // CCW-von-oben-Oberseiten-Fläche liegt das Innere links der gerichteten
    // Kante p→q → Wand (p, p↓, q↓, q) zeigt nach außen (rechts).
    for (std::size_t t = 0; t + 3 <= top.indices.size(); t += 3) {
        const std::array<V3, 3> v = {vertexAt(top, top.indices[t]),
                                     vertexAt(top, top.indices[t + 1]),
                                     vertexAt(top, top.indices[t + 2])};
        for (int i = 0; i < 3; ++i) {
            const V3& p = v[i];
            const V3& q = v[(i + 1) % 3];
            if (edge_count[undirectedEdge(p, q)] == 1) {
                appendQuad(mesh, p, down(p), down(q), q);
            }
        }
    }
    return mesh;
}

}  // namespace

model::TriangleMesh roofMesh(const model::Roof& roof) {
    const model::TriangleMesh top = buildTopShell(roof);
    const double d = roof.thickness_mm;
    // Totalität: degenerierte Oberseite oder nicht-positive Dicke → leeres Netz
    // (kein Volumenkörper, kein Wurf — wie das bisherige Flächenmodell).
    if (top.empty() || !std::isfinite(d) || d <= 0.0) {
        return model::TriangleMesh{};
    }
    return closeSlabDownward(top, d);
}

}  // namespace bcad::hexagon::services
