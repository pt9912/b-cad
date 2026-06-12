// Adapter-Test (slice-003b): beweist echte OCC-Extrusion (LH-FA-D3-001).
// Linkt gegen bcad_adapters (reale OpenCascade-Linkage) und prüft, dass
// das gemessene Solid-Volumen dem analytischen Wert (Länge·Stärke·Höhe)
// innerhalb einer engen Toleranz entspricht und reproduzierbar ist.

#include <gtest/gtest.h>

#include <stdexcept>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "hexagon/model/constants.h"
#include "hexagon/services/wall_footprint.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
using bcad::adapters::geometry::OccGeometryAdapter;
using bcad::hexagon::services::buttFootprint;

// Portierung slice-012 (W3-Q1): der Adapter extrudiert das vom Kern
// gelieferte Footprint-Polygon — Orakel-Werte unveraendert
// (Laenge·Staerke·Hoehe fuer den stumpfen Rechteck-Footprint).
bcad::hexagon::model::Solid extrude(const OccGeometryAdapter& adapter,
                                    const model::Wall& wall) {
    return adapter.extrudeFootprint(buttFootprint(wall), wall.height_mm);
}

model::Wall makeWall(model::Point2D start, model::Point2D end,
                     double thickness_mm, double height_mm) {
    model::Wall wall{};
    wall.start = start;
    wall.end = end;
    wall.thickness_mm = thickness_mm;
    wall.height_mm = height_mm;
    return wall;
}

}  // namespace

TEST(OccGeometryAdapter_LH_FA_D3_001, VolumenEntsprichtAnalytischemWert) {
    const OccGeometryAdapter adapter;
    const auto wall = makeWall({0.0, 0.0}, {1000.0, 0.0}, 240.0, 2500.0);

    const auto solid = extrude(adapter, wall);

    const double expected = 1000.0 * 240.0 * 2500.0;  // Länge·Stärke·Höhe
    EXPECT_NEAR(solid.volume_mm3, expected, expected * 1e-6);
}

TEST(OccGeometryAdapter_LH_FA_D3_001, VolumenUnabhaengigVonSegmentRichtung) {
    const OccGeometryAdapter adapter;
    // Diagonales Segment, Länge 1000 (3-4-5: 600/800).
    const auto wall = makeWall({0.0, 0.0}, {600.0, 800.0}, 300.0, 2700.0);

    const auto solid = extrude(adapter, wall);

    const double expected = 1000.0 * 300.0 * 2700.0;
    EXPECT_NEAR(solid.volume_mm3, expected, expected * 1e-6);
}

TEST(OccGeometryAdapter_LH_FA_D3_001, ExtrusionIstDeterministisch) {
    const OccGeometryAdapter adapter;
    const auto wall = makeWall({10.0, 20.0}, {10.0, 1020.0}, 300.0, 2700.0);

    const auto first = extrude(adapter, wall);
    const auto second = extrude(adapter, wall);

    EXPECT_DOUBLE_EQ(first.volume_mm3, second.volume_mm3);
}

// E-GEO-002 (Finding 2): degeneriertes Polygon (kollabiertes Segment)
// -> neutrale Ausnahme (kein OCC-Typ verlässt den Adapter).
TEST(OccGeometryAdapter_LH_FA_D3_001, DegeneriertesFootprintWirftNeutral) {
    const OccGeometryAdapter adapter;
    const auto wall = makeWall({5.0, 5.0}, {5.0, 5.0}, 240.0, 2500.0);  // Länge 0
    EXPECT_THROW((void)extrude(adapter, wall), std::runtime_error);
}

// Integration: volle Kette Driving -> Service -> Driven -> OCC mit dem
// realen Adapter. Belegt, dass der Service das echte OCC-Volumen hält.
// (Die transaktionale E-GEO-002-Garantie selbst ist adapter-agnostisch
// in slice-003a mit einem werfenden Port-Double getestet.)
TEST(StructureEditServiceMitOcc_LH_FA_D3_001, EndToEndRealeExtrusion) {
    const OccGeometryAdapter geometry;
    services::StructureEditService svc(geometry);
    const auto id =
        svc.addWall(model::StoreyId{1}, model::Segment{{0.0, 0.0}, {1000.0, 0.0}});
    ASSERT_TRUE(id.has_value());
    svc.setWallThickness(*id, 240.0);

    const double expected = 1000.0 * 240.0 * model::kDefaultStoreyHeightMm;
    EXPECT_NEAR(svc.wallSolid(*id).volume_mm3, expected, expected * 1e-6);
}
