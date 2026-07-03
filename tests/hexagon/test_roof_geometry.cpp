// AK-Tests Dach-Geometrie (LH-FA-ROF-001..005, spez. §1 LH-FA-ROF-001.a)
// gegen analytische Netz-Eigenschaften — display-/adapter-frei, das Dach
// ist ein im Kern berechnetes Polyeder (kein OCC).

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <utility>

#include "hexagon/model/constants.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/services/geometry/roof_geometry.h"

namespace {

namespace model = bcad::hexagon::model;
using bcad::hexagon::services::roofMesh;

struct Bounds {
    double lo{std::numeric_limits<double>::max()};
    double hi{std::numeric_limits<double>::lowest()};
    double span() const { return hi - lo; }
};

// Bounding-Box-Spanne entlang einer Achse (0=x,1=y,2=z).
Bounds axisBounds(const model::TriangleMesh& mesh, std::size_t axis) {
    Bounds b;
    for (std::size_t i = axis; i < mesh.positions.size(); i += 3) {
        b.lo = std::min(b.lo, mesh.positions[i]);
        b.hi = std::max(b.hi, mesh.positions[i]);
    }
    return b;
}

// Größte x-Koordinate unter den Vertices auf (nahe) der maximalen Höhe —
// für die First-Lage (Walm: First eingerückt).
double maxXAtTopZ(const model::TriangleMesh& mesh) {
    const double z_top = axisBounds(mesh, 2).hi;
    double x_max = std::numeric_limits<double>::lowest();
    for (std::size_t v = 0; v < mesh.positions.size(); v += 3) {
        if (std::abs(mesh.positions[v + 2] - z_top) < 1.0) {
            x_max = std::max(x_max, mesh.positions[v]);
        }
    }
    return x_max;
}

model::Roof sampleRoof(model::RoofType type) {
    model::Roof roof;
    roof.id = model::RoofId{1};
    roof.storey_id = model::StoreyId{1};
    roof.type = type;
    roof.origin = {0.0, 0.0};
    roof.width_mm = 8000.0;   // b → längere Achse
    roof.depth_mm = 6000.0;   // t
    roof.base_z_mm = 0.0;
    roof.pitch_deg = 30.0;
    roof.overhang_mm = 500.0;
    roof.thickness_mm = model::kDefaultRoofThicknessMm;  // slice-023b: Volumenkörper
    return roof;
}

// --- Slab-Invarianten (slice-023b, MR-009) ----------------------------------

using VKey = std::array<long long, 3>;

VKey vkey(const model::TriangleMesh& mesh, std::size_t vertex) {
    const std::size_t v = vertex * 3;
    return VKey{std::llround(mesh.positions[v] * 1000.0),
               std::llround(mesh.positions[v + 1] * 1000.0),
               std::llround(mesh.positions[v + 2] * 1000.0)};
}

// Geschlossene Mannigfaltigkeit: jede ungerichtete Kante (µm-kanonisch) wird von
// **genau 2** Dreiecks-Flächen genutzt → wasserdicht (kein Loch, keine
// Doppel-/non-manifold-Fläche).
bool isWatertight(const model::TriangleMesh& mesh) {
    std::map<std::pair<VKey, VKey>, int> edges;
    for (std::size_t t = 0; t + 3 <= mesh.indices.size(); t += 3) {
        const std::array<VKey, 3> k = {
            vkey(mesh, static_cast<std::size_t>(mesh.indices[t])),
            vkey(mesh, static_cast<std::size_t>(mesh.indices[t + 1])),
            vkey(mesh, static_cast<std::size_t>(mesh.indices[t + 2]))};
        for (int i = 0; i < 3; ++i) {
            const VKey& a = k[i];
            const VKey& b = k[(i + 1) % 3];
            ++edges[(a < b) ? std::make_pair(a, b) : std::make_pair(b, a)];
        }
    }
    for (const auto& entry : edges) {
        if (entry.second != 2) {
            return false;
        }
    }
    return !edges.empty();
}

// Signiertes Volumen (Divergenzsatz, Σ ⅙·v0·(v1×v2)); > 0 ⟺ Außennormalen.
double signedVolumeMm3(const model::TriangleMesh& mesh) {
    double v6 = 0.0;
    for (std::size_t t = 0; t + 3 <= mesh.indices.size(); t += 3) {
        std::array<std::array<double, 3>, 3> p{};
        for (int i = 0; i < 3; ++i) {
            const std::size_t v = static_cast<std::size_t>(mesh.indices[t + i]) * 3;
            p[static_cast<std::size_t>(i)] = {mesh.positions[v], mesh.positions[v + 1],
                                              mesh.positions[v + 2]};
        }
        const double cx = (p[1][1] * p[2][2]) - (p[1][2] * p[2][1]);
        const double cy = (p[1][2] * p[2][0]) - (p[1][0] * p[2][2]);
        const double cz = (p[1][0] * p[2][1]) - (p[1][1] * p[2][0]);
        v6 += (p[0][0] * cx) + (p[0][1] * cy) + (p[0][2] * cz);
    }
    return v6 / 6.0;
}

constexpr double kTan30 = 0.57735026918962576;  // tan(30°)

// b+2o = 9000 (länger), t+2o = 7000 (kürzer).
constexpr double kEavesShort = 7000.0;
constexpr double kEavesLong = 9000.0;

// LH-FA-ROF-003: Pultdach — eine geneigte Fläche, Hochkante (t+2o)·tan(p).
TEST(RoofGeometry_LH_FA_ROF_003, PultEineFlaecheUndHochkante) {
    const auto mesh = roofMesh(sampleRoof(model::RoofType::Pult));
    EXPECT_TRUE(isWatertight(mesh));  // geschlossener Slab
    EXPECT_NEAR(axisBounds(mesh, 2).hi, kEavesShort * kTan30, 1.0);  // Firsthöhe (Oberseite)
}

// LH-FA-ROF-001: Satteldach — zwei Flächen, First mittig, Firsthöhe
// (kürzere_eaves/2)·tan(p).
TEST(RoofGeometry_LH_FA_ROF_001, SattelZweiFlaechenUndFirsthoehe) {
    const auto mesh = roofMesh(sampleRoof(model::RoofType::Sattel));
    EXPECT_TRUE(isWatertight(mesh));
    EXPECT_NEAR(axisBounds(mesh, 2).hi, (kEavesShort / 2.0) * kTan30, 1.0);
    // First mittig: First-Vertices reichen bis an beide Giebel (x0/x1).
    EXPECT_NEAR(maxXAtTopZ(mesh), kEavesLong - 500.0 /*=x1*/, 1.0);
}

// LH-FA-ROF-002: Walmdach — vier Flächen, First kürzer als der Grundriss.
TEST(RoofGeometry_LH_FA_ROF_002, WalmVierFlaechenUndKuerzererFirst) {
    const auto mesh = roofMesh(sampleRoof(model::RoofType::Walm));
    EXPECT_TRUE(isWatertight(mesh));
    EXPECT_NEAR(axisBounds(mesh, 2).hi, (kEavesShort / 2.0) * kTan30, 1.0);
    // First um die halbe kürzere Trauf-Seite (3500) eingerückt: max-x am
    // First ≈ x1 - 3500 = 8500 - 3500 = 5000, also klar < x1 (8500).
    EXPECT_NEAR(maxXAtTopZ(mesh), 5000.0, 1.0);
    EXPECT_LT(maxXAtTopZ(mesh), 8500.0 - 1.0);
}

// LH-FA-ROF-005: Überstand — das Dach kragt um o über den Grundriss
// (Grundriss b×t = 8000×6000 an origin (0,0); Traufrechteck +o ringsum).
TEST(RoofGeometry_LH_FA_ROF_005, UeberstandKragtUeberGrundriss) {
    const auto mesh = roofMesh(sampleRoof(model::RoofType::Sattel));
    const Bounds x = axisBounds(mesh, 0);
    const Bounds y = axisBounds(mesh, 1);
    EXPECT_NEAR(x.lo, -500.0, 1e-6);
    EXPECT_NEAR(x.hi, 8500.0, 1e-6);
    EXPECT_NEAR(y.lo, -500.0, 1e-6);
    EXPECT_NEAR(y.hi, 6500.0, 1e-6);
}

// LH-FA-ROF-004: steilere Neigung → höherer First (monoton).
TEST(RoofGeometry_LH_FA_ROF_004, SteilereNeigungHoehererFirst) {
    model::Roof flat = sampleRoof(model::RoofType::Sattel);
    flat.pitch_deg = 15.0;
    model::Roof steep = sampleRoof(model::RoofType::Sattel);
    steep.pitch_deg = 45.0;
    EXPECT_GT(axisBounds(roofMesh(steep), 2).hi,
              axisBounds(roofMesh(flat), 2).hi);
}

// Volumenkörper-Invarianten (slice-023b, MR-009): der Dach-Slab ist je Typ
// **wasserdicht** (jede Kante genau 2 Flächen), **außen-orientiert** (signiertes
// Volumen > 0) und trägt eine um die Dicke versetzte **Unterseite**. Ersetzt den
// früheren „alle Flächen zeigen nach oben"-Test (gilt nur für die Oberseite).
TEST(RoofGeometry_LH_FA_ROF_006, SlabIstWasserdichtUndAussenOrientiert) {
    for (const model::RoofType type :
         {model::RoofType::Pult, model::RoofType::Sattel,
          model::RoofType::Walm}) {
        const model::Roof roof = sampleRoof(type);
        const auto mesh = roofMesh(roof);
        ASSERT_FALSE(mesh.empty());
        EXPECT_TRUE(isWatertight(mesh))
            << "Slab nicht geschlossen (Typ " << static_cast<int>(type) << ")";
        EXPECT_GT(signedVolumeMm3(mesh), 0.0) << "Slab invertiert (Innennormalen)";
        // Stärkste Sonde (MR-009 LOW-1): das signierte Volumen des vertikalen
        // Slab ist EXAKT bx·ty·d — eine einzelne invertierte Wand/Fläche oder
        // ein Loch würde es verändern (fängt lokale Defekte, die Geschlossenheit
        // + Gesamt-Vorzeichen überleben).
        const double bx = roof.width_mm + (2.0 * roof.overhang_mm);
        const double ty = roof.depth_mm + (2.0 * roof.overhang_mm);
        EXPECT_NEAR(signedVolumeMm3(mesh), bx * ty * roof.thickness_mm, 1.0)
            << "Slab-Volumen ≠ bx·ty·d (lokale Inversion / Loch)";
        EXPECT_NEAR(axisBounds(mesh, 2).lo, -roof.thickness_mm, 1e-6);  // Unterseite
    }
}

// MED-1 (Walm-Zeltdach): quadratischer Grundriss → First kollabiert zum **Apex**
// (Punkt); der Slab muss am Apex geschlossen bleiben (heikelster Fall).
TEST(RoofGeometry_LH_FA_ROF_002, WalmZeltdachApexWasserdicht) {
    model::Roof tent = sampleRoof(model::RoofType::Walm);
    tent.width_mm = 6000.0;
    tent.depth_mm = 6000.0;  // quadratisch → Zeltdach (First = Punkt)
    const auto mesh = roofMesh(tent);
    ASSERT_FALSE(mesh.empty());
    EXPECT_TRUE(isWatertight(mesh)) << "Zeltdach-Apex nicht geschlossen";
    EXPECT_GT(signedVolumeMm3(mesh), 0.0);
}

// Negative/Totalität: degenerierter (Null-Breite) oder nicht-positiv
// geneigter Grundriss → leeres Netz, kein Wurf.
TEST(RoofGeometry_LH_FA_ROF_001, DegenerierterGrundrissLeer) {
    model::Roof zero = sampleRoof(model::RoofType::Sattel);
    zero.width_mm = 0.0;
    zero.overhang_mm = 0.0;
    EXPECT_TRUE(roofMesh(zero).empty());

    model::Roof flat = sampleRoof(model::RoofType::Pult);
    flat.pitch_deg = 0.0;  // tan = 0 → kein Dach
    EXPECT_TRUE(roofMesh(flat).empty());
}

// LH-FA-ROF-006 Totalität: nicht-positive Dicke → kein Volumenkörper, leeres Netz.
TEST(RoofGeometry_LH_FA_ROF_006, NichtPositiveDickeLeer) {
    model::Roof r = sampleRoof(model::RoofType::Sattel);
    r.thickness_mm = 0.0;
    EXPECT_TRUE(roofMesh(r).empty());
}

}  // namespace
