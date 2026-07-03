// AK-Tests Treppen-Geometrie (LH-FA-STR-001..004, spez. §1 LH-FA-STR-001.a)
// gegen analytische Netz-Eigenschaften — display-/adapter-frei, die Treppe ist
// ein im Kern berechnetes Polyeder (kein OCC). Die rise ist abgeleitet
// (Geschosshöhe/step_count, LOW-2: kein Elevation-Lookup); das Geländer wird mit
// ZWEI Sonden belegt (Handlaufhöhe + folgt dem Lauf, MED-2 gegen Spike-Fehlpass).

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <limits>
#include <set>

#include "hexagon/model/constants.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/services/geometry/stair_geometry.h"

namespace {

namespace model = bcad::hexagon::model;
using bcad::hexagon::services::stairMesh;
using bcad::hexagon::services::stairRiseMm;
using bcad::hexagon::services::stairRunLengthMm;
using bcad::hexagon::services::stairStepBoxes;
using bcad::hexagon::services::StepBox;

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

// Maximale z-Koordinate unter den Vertices mit x < x_threshold — belegt, dass
// das Geländer dem Lauf folgt (auch am unteren Lauf vorhanden), nicht nur ein
// einzelner hoher Spike am Top ist (MED-2 Sonde b).
double maxZForXBelow(const model::TriangleMesh& mesh, double x_threshold) {
    double z_max = std::numeric_limits<double>::lowest();
    for (std::size_t v = 0; v < mesh.positions.size(); v += 3) {
        if (mesh.positions[v] < x_threshold) {
            z_max = std::max(z_max, mesh.positions[v + 2]);
        }
    }
    return z_max;
}

bool hasVertexNearZ(const model::TriangleMesh& mesh, double z, double tol) {
    for (std::size_t v = 2; v < mesh.positions.size(); v += 3) {
        if (std::abs(mesh.positions[v] - z) < tol) {
            return true;
        }
    }
    return false;
}

constexpr double kStoreyHeight = 2500.0;

model::Stair sampleStair() {
    model::Stair stair;
    stair.id = model::StairId{1};
    stair.from_storey_id = model::StoreyId{1};
    stair.to_storey_id = model::StoreyId{2};
    stair.type = model::StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 15;
    stair.tread_mm = 280.0;
    return stair;
}

// slice-024b (LH-FA-STR-001): `stairStepBoxes` ist die **eine Box-Wahrheit** für
// `stairMesh` (Darstellung/STL) UND den STEP-Export. Direkt geprüft: Anzahl ==
// step_count, Extents je Stufe == der dokumentierten Formel (ohne Geländer);
// degeneriert → leer (Totalität).
TEST(StairGeometry_LH_FA_STR_001, StepBoxesTragenDieStufenWahrheit) {
    const model::Stair stair = sampleStair();
    const std::vector<StepBox> boxes = stairStepBoxes(stair, kStoreyHeight);

    ASSERT_EQ(boxes.size(), static_cast<std::size_t>(stair.step_count));
    const double rise = kStoreyHeight / stair.step_count;
    for (int i = 0; i < stair.step_count; ++i) {
        const StepBox& b = boxes[static_cast<std::size_t>(i)];
        EXPECT_DOUBLE_EQ(b.x0_mm, stair.start.x_mm + (i * stair.tread_mm)) << i;
        EXPECT_DOUBLE_EQ(b.x1_mm, stair.start.x_mm + ((i + 1) * stair.tread_mm)) << i;
        EXPECT_DOUBLE_EQ(b.y0_mm, stair.start.y_mm);
        EXPECT_DOUBLE_EQ(b.y1_mm, stair.start.y_mm + stair.width_mm);
        EXPECT_DOUBLE_EQ(b.z0_mm, 0.0);
        EXPECT_DOUBLE_EQ(b.z1_mm, (i + 1) * rise);  // volle Höhe von 0 → top
    }

    // Degeneriert (step_count < 1) → leere Liste (kein Wurf), wie stairMesh.
    model::Stair bad = stair;
    bad.step_count = 0;
    EXPECT_TRUE(stairStepBoxes(bad, kStoreyHeight).empty());
}

// LH-FA-STR-001: rise abgeleitet (Geschosshöhe/step_count) und die Treppe
// verbindet beide Ebenen (oberste Stufe auf Geschosshöhe); +x-Aufstieg.
TEST(StairGeometry_LH_FA_STR_001, RiseAbgeleitetUndVerbindetGeschosse) {
    const model::Stair stair = sampleStair();
    const double rise = stairRiseMm(stair, kStoreyHeight);
    EXPECT_DOUBLE_EQ(rise, kStoreyHeight / stair.step_count);
    // Oberste Stufe erreicht die obere Geschossebene = Geschosshöhe (LOW-2: kein
    // Elevation-Lookup, Ein-Geschoss-Annahme).
    EXPECT_DOUBLE_EQ(rise * stair.step_count, kStoreyHeight);

    const model::TriangleMesh mesh = stairMesh(stair, kStoreyHeight);
    EXPECT_FALSE(mesh.empty());
    const Bounds z = axisBounds(mesh, 2);
    EXPECT_NEAR(z.lo, 0.0, 1e-9);  // Fuß auf base_z=0
    EXPECT_TRUE(hasVertexNearZ(mesh, kStoreyHeight, 1e-6));  // Stufe auf OK

    // +x-Aufstieg ab Startpunkt; x-Spanne = Lauflänge.
    const Bounds x = axisBounds(mesh, 0);
    EXPECT_NEAR(x.lo, stair.start.x_mm, 1e-9);
    EXPECT_NEAR(x.span(), stairRunLengthMm(stair), 1e-6);
    EXPECT_DOUBLE_EQ(stairRunLengthMm(stair), stair.step_count * stair.tread_mm);
}

// LH-FA-STR-002: die Stufenanzahl spiegelt sich im Netz (mehr Stufen → mehr
// Geometrie) und steuert die abgeleitete Steigung.
TEST(StairGeometry_LH_FA_STR_002, StufenanzahlSpiegeltSichImNetz) {
    model::Stair few = sampleStair();
    few.step_count = 10;
    model::Stair many = sampleStair();
    many.step_count = 20;

    const model::TriangleMesh mesh_few = stairMesh(few, kStoreyHeight);
    const model::TriangleMesh mesh_many = stairMesh(many, kStoreyHeight);
    EXPECT_GT(mesh_many.triangleCount(), mesh_few.triangleCount());

    // Abgeleitete Steigung sinkt mit mehr Stufen; erste Stufenoberkante = rise.
    EXPECT_GT(stairRiseMm(few, kStoreyHeight), stairRiseMm(many, kStoreyHeight));
    EXPECT_TRUE(hasVertexNearZ(mesh_few, kStoreyHeight / 10.0, 1e-6));
}

// LH-FA-STR-003: die Laufbreite spiegelt sich als y-Ausdehnung des Netzes.
TEST(StairGeometry_LH_FA_STR_003, LaufbreiteImNetz) {
    model::Stair narrow = sampleStair();
    narrow.width_mm = 1000.0;
    model::Stair wide = sampleStair();
    wide.width_mm = 1600.0;

    EXPECT_NEAR(axisBounds(stairMesh(narrow, kStoreyHeight), 1).span(), 1000.0, 1e-6);
    EXPECT_NEAR(axisBounds(stairMesh(wide, kStoreyHeight), 1).span(), 1600.0, 1e-6);
}

// LH-FA-STR-004: Geländer mit ZWEI Sonden (MED-2) — (a) Handlaufhöhe über der
// obersten Stufe, (b) Geländer folgt dem Lauf (auch bei kleinem x vorhanden,
// kein Top-Spike).
TEST(StairGeometry_LH_FA_STR_004, GelaenderZweiSonden) {
    const model::Stair stair = sampleStair();
    const model::TriangleMesh mesh = stairMesh(stair, kStoreyHeight);

    // (a) Oberkante des Netzes ≈ Geschosshöhe + Handlaufhöhe.
    EXPECT_NEAR(axisBounds(mesh, 2).hi,
                kStoreyHeight + model::kStairRailingHeightMm, 1e-6);

    // (b) am unteren Lauf (x < 2·tread) reicht das Netz bereits über die
    // Handlaufhöhe — ein bloßer Top-Spike läge hier nur bei ~2·rise (< rail_h).
    const double low_x = stair.start.x_mm + (2.0 * stair.tread_mm);
    EXPECT_GT(maxZForXBelow(mesh, low_x), model::kStairRailingHeightMm);
}

// LH-FA-STR-001 Negative: degenerierte Parameter → leeres Netz, kein Wurf
// (Totalität).
TEST(StairGeometry_LH_FA_STR_001, NegativeDegeneriertLeeresNetz) {
    model::Stair zero_steps = sampleStair();
    zero_steps.step_count = 0;
    EXPECT_TRUE(stairMesh(zero_steps, kStoreyHeight).empty());

    model::Stair zero_width = sampleStair();
    zero_width.width_mm = 0.0;
    EXPECT_TRUE(stairMesh(zero_width, kStoreyHeight).empty());

    model::Stair zero_tread = sampleStair();
    zero_tread.tread_mm = 0.0;
    EXPECT_TRUE(stairMesh(zero_tread, kStoreyHeight).empty());

    EXPECT_TRUE(stairMesh(sampleStair(), 0.0).empty());  // keine Geschosshöhe

    model::Stair nan_start = sampleStair();
    nan_start.start = {std::numeric_limits<double>::quiet_NaN(), 0.0};
    EXPECT_TRUE(stairMesh(nan_start, kStoreyHeight).empty());

    EXPECT_DOUBLE_EQ(stairRiseMm(zero_steps, kStoreyHeight), 0.0);  // Div-Schutz
}

// LH-FA-STR-001 (Code-Review M2): das Netz ist ein geschlossener, konsistent
// orientierter Körper — die Summe der flächengewichteten Außennormalen
// (∑ ½·(b−a)×(c−a) je Dreieck) verschwindet (Divergenzsatz; jeder geschlossene
// Quader summiert zu 0). Fängt eine **invertierte** Box-Fläche (014b-Defekt-
// klasse), die Bounding-Box-/Vertex-Sonden NICHT sehen.
TEST(StairGeometry_LH_FA_STR_001, NetzGeschlossenUndKonsistentOrientiert) {
    const model::TriangleMesh mesh = stairMesh(sampleStair(), kStoreyHeight);
    ASSERT_FALSE(mesh.empty());
    double sx = 0.0;
    double sy = 0.0;
    double sz = 0.0;
    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const auto pos = [&](int tri_offset, std::size_t axis) {
            return mesh.positions[(static_cast<std::size_t>(
                                       mesh.indices[i + tri_offset]) *
                                   3) +
                                  axis];
        };
        const double abx = pos(1, 0) - pos(0, 0);
        const double aby = pos(1, 1) - pos(0, 1);
        const double abz = pos(1, 2) - pos(0, 2);
        const double acx = pos(2, 0) - pos(0, 0);
        const double acy = pos(2, 1) - pos(0, 1);
        const double acz = pos(2, 2) - pos(0, 2);
        sx += 0.5 * ((aby * acz) - (abz * acy));
        sy += 0.5 * ((abz * acx) - (abx * acz));
        sz += 0.5 * ((abx * acy) - (aby * acx));
    }
    // ≈ 0 mm²; eine invertierte Fläche läge bei ~Flächeninhalt (≫ 1).
    EXPECT_LT(std::sqrt((sx * sx) + (sy * sy) + (sz * sz)), 1.0);
}

// LH-FA-STR-002 (Code-Review M3): alle `step_count` Stufen sind vorhanden und
// **bündig** — die distinkten x-Stufenkanten sind genau `step_count + 1` (kein
// Spalt, keine fehlende/zusätzliche Stufe). Bounding-Box-/Dreiecks-Anzahl-
// Sonden sehen das nicht.
TEST(StairGeometry_LH_FA_STR_002, StufenBuendigKeineLuecke) {
    const model::Stair stair = sampleStair();  // step_count=15, tread=280, x=0
    const model::TriangleMesh mesh = stairMesh(stair, kStoreyHeight);
    std::set<long long> x_edges;
    for (std::size_t v = 0; v < mesh.positions.size(); v += 3) {
        x_edges.insert(std::llround(mesh.positions[v] * 1000.0));  // µm-Raster
    }
    EXPECT_EQ(static_cast<int>(x_edges.size()), stair.step_count + 1);
}

}  // namespace
