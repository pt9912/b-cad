// AK-Tests Dach-Geometrie (LH-FA-ROF-001..005, spez. §1 LH-FA-ROF-001.a)
// gegen analytische Netz-Eigenschaften — display-/adapter-frei, das Dach
// ist ein im Kern berechnetes Polyeder (kein OCC).

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <limits>

#include "hexagon/model/constants.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/services/roof_geometry.h"

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
    return roof;
}

constexpr double kTan30 = 0.57735026918962576;  // tan(30°)

// b+2o = 9000 (länger), t+2o = 7000 (kürzer).
constexpr double kEavesShort = 7000.0;
constexpr double kEavesLong = 9000.0;

// LH-FA-ROF-003: Pultdach — eine geneigte Fläche, Hochkante (t+2o)·tan(p).
TEST(RoofGeometry_LH_FA_ROF_003, PultEineFlaecheUndHochkante) {
    const auto mesh = roofMesh(sampleRoof(model::RoofType::Pult));
    EXPECT_EQ(mesh.triangleCount(), 2);  // eine Fläche = 2 Dreiecke
    EXPECT_NEAR(axisBounds(mesh, 2).span(), kEavesShort * kTan30, 1.0);
}

// LH-FA-ROF-001: Satteldach — zwei Flächen, First mittig, Firsthöhe
// (kürzere_eaves/2)·tan(p).
TEST(RoofGeometry_LH_FA_ROF_001, SattelZweiFlaechenUndFirsthoehe) {
    const auto mesh = roofMesh(sampleRoof(model::RoofType::Sattel));
    EXPECT_EQ(mesh.triangleCount(), 4);  // zwei Flächen = 4 Dreiecke
    EXPECT_NEAR(axisBounds(mesh, 2).span(), (kEavesShort / 2.0) * kTan30, 1.0);
    // First mittig: First-Vertices reichen bis an beide Giebel (x0/x1).
    EXPECT_NEAR(maxXAtTopZ(mesh), kEavesLong - 500.0 /*=x1*/, 1.0);
}

// LH-FA-ROF-002: Walmdach — vier Flächen, First kürzer als der Grundriss.
TEST(RoofGeometry_LH_FA_ROF_002, WalmVierFlaechenUndKuerzererFirst) {
    const auto mesh = roofMesh(sampleRoof(model::RoofType::Walm));
    EXPECT_EQ(mesh.triangleCount(), 6);  // 2 Trapeze (4) + 2 Walme (2)
    EXPECT_NEAR(axisBounds(mesh, 2).span(), (kEavesShort / 2.0) * kTan30, 1.0);
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
    EXPECT_GT(axisBounds(roofMesh(steep), 2).span(),
              axisBounds(roofMesh(flat), 2).span());
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

}  // namespace
