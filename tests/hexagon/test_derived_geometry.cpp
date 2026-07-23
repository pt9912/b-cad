// slice-042a (ADR-0020): das `DerivedGeometry`-Bündel ist die pure Kern-Naht,
// über die der Kern abgeleitete Geometrie an die Export-Adapter reicht — ein
// lib-freies Werttyp-Aggregat (baut ohne OCC/Qt/SQLite), default-leer und
// format-selektiv befüllbar. Diese Stufe prüft nur die **Naht-Existenz +
// Lib-Freiheit**; die format-selektive Befüllung + der STEP/STL-Konsum kommen
// mit slice-042c.

#include <gtest/gtest.h>

#include "hexagon/model/derived_geometry.h"
#include "hexagon/model/mesh_ops.h"
#include "hexagon/model/step_box.h"
#include "hexagon/model/triangle_mesh.h"

namespace model = bcad::hexagon::model;

TEST(DerivedGeometry, DefaultConstructedIsEmpty) {
    const model::DerivedGeometry derived{};
    EXPECT_TRUE(derived.walls.empty());
    EXPECT_TRUE(derived.slabs.empty());
    EXPECT_TRUE(derived.roofs.empty());
    EXPECT_TRUE(derived.stairs.empty());
}

TEST(DerivedGeometry, CarriesPurePrePrimitives) {
    model::DerivedGeometry derived{};

    model::DerivedWall wall;
    wall.height_mm = 2500.0;
    derived.walls.push_back(wall);

    model::DerivedStair stair;
    stair.rise_mm = 180.0;
    stair.boxes.push_back(model::StepBox{0.0, 300.0, 0.0, 1000.0, 0.0, 180.0});
    derived.stairs.push_back(stair);

    ASSERT_EQ(derived.walls.size(), 1U);
    EXPECT_DOUBLE_EQ(derived.walls.front().height_mm, 2500.0);
    ASSERT_EQ(derived.stairs.size(), 1U);
    ASSERT_EQ(derived.stairs.front().boxes.size(), 1U);
    EXPECT_DOUBLE_EQ(derived.stairs.front().boxes.front().z1_mm, 180.0);
    EXPECT_DOUBLE_EQ(derived.stairs.front().rise_mm, 180.0);
}

// `translateMeshZ` ist mit ADR-0020 eine reine `model/`-Util (adapter-erreichbar
// ohne `services/geometry`-Import): z-Verschiebung, x/y unberührt.
TEST(MeshOps, TranslateMeshZShiftsOnlyZ) {
    model::TriangleMesh mesh;
    mesh.positions = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};

    const model::TriangleMesh out = model::translateMeshZ(mesh, 10.0);

    ASSERT_EQ(out.positions.size(), 6U);
    EXPECT_DOUBLE_EQ(out.positions[0], 1.0);   // x unberührt
    EXPECT_DOUBLE_EQ(out.positions[1], 2.0);   // y unberührt
    EXPECT_DOUBLE_EQ(out.positions[2], 13.0);  // z + 10
    EXPECT_DOUBLE_EQ(out.positions[5], 16.0);  // z + 10
}
