// AK-Tests Platten-Geometrie (LH-FA-SLB-*/FND-*, spez. §1 LH-FA-SLB-001.a)
// — display-/adapter-frei. Die pure Geometrie (base_z je Typ,
// Cutout-Prismen relativ, Mesh-Translation) wird direkt geprüft; das
// Volumen/Netz über das analytische GeometryKernelPort-Double.

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

#include "hexagon/model/constants.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/slab.h"
#include "hexagon/services/geometry/slab_geometry.h"
#include "analytic_geometry_double.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
using bcad::testing::AnalyticGeometry;

model::Footprint rect(double x0, double y0, double x1, double y1) {
    return model::Footprint{{{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}}};
}

model::Slab sampleSlab(model::SlabType type) {
    model::Slab slab;
    slab.id = model::SlabId{1};
    slab.storey_id = model::StoreyId{1};
    slab.type = type;
    slab.footprint = rect(0.0, 0.0, 5000.0, 4000.0);
    slab.thickness_mm = 200.0;
    return slab;
}

double minZ(const model::TriangleMesh& mesh) {
    double lo = std::numeric_limits<double>::max();
    for (std::size_t i = 2; i < mesh.positions.size(); i += 3) {
        lo = std::min(lo, mesh.positions[i]);
    }
    return lo;
}

// LH-FA-SLB-001 / LH-FA-FND-001/003: base_z je Typ (Ein-Geschoss).
TEST(SlabGeometry_LH_FA_SLB_001, BaseZJeTyp) {
    EXPECT_DOUBLE_EQ(services::slabBaseZ(sampleSlab(model::SlabType::Decke),
                                         2500.0),
                     2500.0);  // Decke auf Geschoss-Oberkante
    EXPECT_DOUBLE_EQ(services::slabBaseZ(sampleSlab(model::SlabType::Bodenplatte),
                                         2500.0),
                     -200.0);  // Oberkante 0, Dicke nach unten
    EXPECT_DOUBLE_EQ(services::slabBaseZ(sampleSlab(model::SlabType::Fundament),
                                         2500.0),
                     -200.0);
}

// LH-FA-SLB-003: Ausschnitt-Prismen liegen RELATIV zum Solid [0,Dicke]
// (nicht base_z) — die Reihenfolge-Falle (MED-2) ist damit ausgeschlossen.
TEST(SlabGeometry_LH_FA_SLB_003, CutPrismenRelativZumSolid) {
    model::Slab slab = sampleSlab(model::SlabType::Decke);
    slab.cutouts = {rect(1000.0, 1000.0, 2000.0, 2000.0)};
    const auto prisms = services::slabCutPrisms(slab);
    ASSERT_EQ(prisms.size(), 1U);
    EXPECT_LT(prisms[0].z_min_mm, 0.0);                  // −ε, nicht base_z
    EXPECT_GT(prisms[0].z_max_mm, slab.thickness_mm);    // Dicke+ε
    EXPECT_LT(prisms[0].z_max_mm, slab.thickness_mm + 100.0);
}

// LH-FA-SLB-003: der Ausschnitt verringert das Platten-Volumen um das
// Ausschnitt-Volumen (Volumen ist base_z-invariant — über das Double).
TEST(SlabGeometry_LH_FA_SLB_003, AusschnittVerringertVolumen) {
    const AnalyticGeometry geometry;
    model::Slab slab = sampleSlab(model::SlabType::Decke);
    const double full =
        geometry.extrudeFootprint(slab.footprint, slab.thickness_mm, {})
            .volume_mm3;

    slab.cutouts = {rect(1000.0, 1000.0, 2000.0, 2000.0)};  // 1000×1000
    const double net = geometry
                           .extrudeFootprint(slab.footprint, slab.thickness_mm,
                                             services::slabCutPrisms(slab))
                           .volume_mm3;

    const double opening = 1000.0 * 1000.0 * slab.thickness_mm;
    EXPECT_NEAR(full - net, opening, full * 1e-9);
}

// LH-FA-SLB-001: das Platten-Netz wird um base_z verschoben — die
// Unterkante liegt nach der Translation auf der Aufstandshöhe.
TEST(SlabGeometry_LH_FA_SLB_001, NetzAufAufstandshoeheVerschoben) {
    const AnalyticGeometry geometry;
    const model::Slab slab = sampleSlab(model::SlabType::Decke);
    model::TriangleMesh mesh = geometry.tessellateFootprint(
        slab.footprint, slab.thickness_mm, services::slabCutPrisms(slab));
    ASSERT_DOUBLE_EQ(minZ(mesh), 0.0);  // vor Translation: Solid bei z=0

    const double base_z = services::slabBaseZ(slab, 2500.0);
    mesh = services::translateMeshZ(std::move(mesh), base_z);

    EXPECT_DOUBLE_EQ(minZ(mesh), 2500.0);  // Decke-Unterkante auf Geschoss-OK
}

// LH-FA-SLB-003 (Code-Review slice-015b H1): „auf den Platten-Umriss
// begrenzt" — nur ein nicht-degenerierter, vollständig innenliegender
// Ausschnitt wird akzeptiert; rand-/außenliegende, degenerierte und
// nicht-endliche werden abgelehnt (koplanar-freier Boolean).
TEST(SlabGeometry_LH_FA_SLB_003, CutoutNurInnenliegendAkzeptiert) {
    const model::Slab slab = sampleSlab(model::SlabType::Decke);  // 0,0..5000,4000

    // Innenliegend → akzeptiert.
    EXPECT_TRUE(services::cutoutInsideSlab(
        slab, rect(1000.0, 1000.0, 2000.0, 2000.0)));

    // Über den Umriss hinaus (x bis 6000 > 5000) → abgelehnt.
    EXPECT_FALSE(services::cutoutInsideSlab(
        slab, rect(4000.0, 1000.0, 6000.0, 2000.0)));

    // Stützpunkt auf der Umrisskante (x=5000, nicht strikt innen) → abgelehnt.
    EXPECT_FALSE(services::cutoutInsideSlab(
        slab, rect(3000.0, 1000.0, 5000.0, 2000.0)));

    // Degeneriert (Fläche 0) → abgelehnt.
    EXPECT_FALSE(services::cutoutInsideSlab(
        slab, rect(1000.0, 1000.0, 1000.0, 1000.0)));

    // Zu wenige Stützpunkte → abgelehnt.
    EXPECT_FALSE(services::cutoutInsideSlab(
        slab, model::Footprint{{{1000.0, 1000.0}, {2000.0, 2000.0}}}));

    // Nicht-endliche Koordinate → abgelehnt.
    model::Footprint nan_cut = rect(1000.0, 1000.0, 2000.0, 2000.0);
    nan_cut.points[1].x_mm = std::numeric_limits<double>::quiet_NaN();
    EXPECT_FALSE(services::cutoutInsideSlab(slab, nan_cut));
}

}  // namespace
