// Akzeptanz-Tests für die EVL-Listen (slice-017f) — OCC-frei über ein
// GeometryKernelPort-Double (ADR-0001 §Testbarkeit). Geprüft gegen spez. §1
// LH-FA-EVL-001.a (Listen = Aggregation):
// - LH-FA-EVL-004 Materialliste: je effektivem Material die Menge = Σ Netto-
//   Volumen (m³) über Wand + Decke; Bauteile ohne Material nicht gruppiert
//   (Boundary); Dach ausgenommen (Volumen welle-3 zurückgestellt, Lücke).
// - LH-FA-EVL-005/006 Tür-/Fensterlisten: die Öffnungen mit ihren Maßen.
// Volumen-Orakel: freistehende Rechteck-Wand = Länge·Stärke·Höhe (017c).

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "analytic_geometry_double.h"
#include "hexagon/model/material.h"
#include "hexagon/model/material_line.h"
#include "hexagon/model/opening_line.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/slab.h"
#include "hexagon/ports/driving/evaluate_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace ports = bcad::hexagon::ports;
namespace services = bcad::hexagon::services;
using bcad::testing::AnalyticGeometry;

constexpr model::StoreyId kGround{1};  // Default-Geschoss, Höhe 2500 mm
constexpr double kTolM3 = 1e-6;

model::Material makeMat(std::string name, std::string category) {
    model::Material material;
    material.name = std::move(name);
    material.category = std::move(category);
    return material;
}

// Freistehende Wand (Default-Stärke 240, Höhe 2500) → stumpfer Footprint, Orakel
// exakt Länge·Stärke·Höhe.
model::WallId addWall(services::StructureEditService& svc, model::Segment seg) {
    const std::optional<model::WallId> id = svc.addWall(kGround, seg);
    EXPECT_TRUE(id.has_value());
    return id.value_or(model::WallId{});
}

// Decke 5000×4000 × 200 mm = 4,0 m³.
model::SlabId addDecke(services::StructureEditService& svc) {
    model::Slab slab;
    slab.storey_id = kGround;
    slab.type = model::SlabType::Decke;
    slab.footprint.points = {{0.0, 3000.0}, {5000.0, 3000.0}, {5000.0, 7000.0},
                             {0.0, 7000.0}};
    slab.thickness_mm = 200.0;
    const std::optional<model::SlabId> id = svc.addSlab(slab);
    EXPECT_TRUE(id.has_value());
    return id.value_or(model::SlabId{});
}

}  // namespace

// EVL-004: Materialliste gruppiert je effektivem Material; Menge = Σ Volumen.
TEST(Evaluate_LH_FA_EVL_004, MateriallisteGruppiertVolumen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::MaterialId mat1 = *svc.addMaterial(makeMat("Beton", "Tragwerk"));
    const model::MaterialId mat2 = *svc.addMaterial(makeMat("Ziegel", "Wand"));

    const model::WallId w1 = addWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});  // 2,4 m³
    ASSERT_TRUE(svc.setWallMaterial(w1, mat1));
    const model::WallId w2 =
        addWall(svc, {{0.0, 1000.0}, {4000.0, 1000.0}});  // 2,4 m³
    ASSERT_TRUE(svc.setWallMaterial(w2, mat2));
    addWall(svc, {{0.0, 2000.0}, {4000.0, 2000.0}});  // ohne Material
    const model::SlabId slab = addDecke(svc);          // 4,0 m³
    ASSERT_TRUE(svc.setSlabMaterial(slab, mat1));

    const model::MaterialReport report = svc.materialList();
    ASSERT_EQ(report.lines.size(), 2U);  // mat1, mat2 (nach MaterialId sortiert)

    // mat1 (Beton): Wand1 (2,4) + Decke (4,0) = 6,4 m³, 2 Bauteile.
    EXPECT_EQ(report.lines[0].material.name, "Beton");
    EXPECT_EQ(report.lines[0].component_count, 2);
    EXPECT_NEAR(report.lines[0].quantity_m3, 6.4, kTolM3);
    // mat2 (Ziegel): Wand2 (2,4) = 2,4 m³, 1 Bauteil.
    EXPECT_EQ(report.lines[1].material.name, "Ziegel");
    EXPECT_EQ(report.lines[1].component_count, 1);
    EXPECT_NEAR(report.lines[1].quantity_m3, 2.4, kTolM3);
    // total = 6,4 + 2,4 = 8,8 m³ (Wand3 ohne Material zählt nicht).
    EXPECT_NEAR(report.total_m3, 8.8, kTolM3);
}

// EVL-004 Boundary: Bauteil ohne Material nicht gruppiert; Dach mit Material
// erscheint NICHT (Volumen welle-3 zurückgestellt — benannte Lücke).
TEST(Evaluate_LH_FA_EVL_004, OhneMaterialNichtGruppiertUndDachLuecke) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::MaterialId mat =
        *svc.addMaterial(makeMat("Dachziegel", "Dach"));

    addWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});  // ohne Material
    model::Roof roof;
    roof.storey_id = kGround;
    roof.type = model::RoofType::Sattel;
    roof.origin = {0.0, 0.0};
    roof.width_mm = 5000.0;
    roof.depth_mm = 4000.0;
    roof.base_z_mm = 2500.0;
    roof.pitch_deg = 30.0;
    roof.overhang_mm = 500.0;
    const model::RoofId roof_id = *svc.addRoof(roof);
    ASSERT_TRUE(svc.setRoofMaterial(roof_id, mat));  // Dach IST material-tragend

    const model::MaterialReport report = svc.materialList();
    EXPECT_TRUE(report.lines.empty());  // Wand unzugewiesen + Dach ausgenommen
    EXPECT_DOUBLE_EQ(report.total_m3, 0.0);
}

// EVL-005: Türliste mit Maßen; Anzahl = Listengröße.
TEST(Evaluate_LH_FA_EVL_005, TuerlisteMitMassen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWall(svc, {{0.0, 0.0}, {6000.0, 0.0}});
    ASSERT_TRUE(svc.addDoor(wall, 1000.0).has_value());
    ASSERT_TRUE(svc.addDoor(wall, 3000.0).has_value());

    const std::vector<model::DoorLine> doors = svc.doorList();
    ASSERT_EQ(doors.size(), 2U);  // Anzahl = 2
    EXPECT_DOUBLE_EQ(doors[0].width_mm, 900.0);   // Default-Tür
    EXPECT_DOUBLE_EQ(doors[0].height_mm, 2100.0);
    EXPECT_TRUE(svc.windowList().empty());  // keine Fenster
}

// EVL-006: Fensterliste mit Maßen inkl. Brüstung.
TEST(Evaluate_LH_FA_EVL_006, FensterlisteMitMassenUndBruestung) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWall(svc, {{0.0, 0.0}, {6000.0, 0.0}});
    ASSERT_TRUE(svc.addWindow(wall, 1000.0).has_value());

    const std::vector<model::WindowLine> windows = svc.windowList();
    ASSERT_EQ(windows.size(), 1U);
    EXPECT_DOUBLE_EQ(windows[0].width_mm, 1200.0);       // Default-Fenster
    EXPECT_DOUBLE_EQ(windows[0].height_mm, 1300.0);
    EXPECT_DOUBLE_EQ(windows[0].sill_height_mm, 900.0);  // Brüstung
    EXPECT_TRUE(svc.doorList().empty());  // keine Türen
}

// Boundary: leeres Modell → alle Listen leer; read-only/stabil.
TEST(Evaluate_LH_FA_EVL_004, LeerUndReadOnly) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    EXPECT_TRUE(svc.materialList().lines.empty());
    EXPECT_DOUBLE_EQ(svc.materialList().total_m3, 0.0);
    EXPECT_TRUE(svc.doorList().empty());
    EXPECT_TRUE(svc.windowList().empty());

    addWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});
    const ports::driving::EvaluatePort& port = svc;
    const std::size_t walls_before = svc.building().walls.size();
    const model::MaterialReport first = port.materialList();
    const model::MaterialReport second = port.materialList();
    EXPECT_EQ(first.lines.size(), second.lines.size());
    EXPECT_DOUBLE_EQ(first.total_m3, second.total_m3);
    EXPECT_EQ(svc.building().walls.size(), walls_before);
}

// MAT-006: Kosten je Material = Menge × cost_per_m3 + Summe; Material ohne
// Kostenkennwert → cost nullopt (nicht 0) und trägt 0 zur Summe bei.
TEST(Evaluate_LH_FA_MAT_006, KostenJeMaterialUndSumme) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    model::Material m1 = makeMat("Beton", "Tragwerk");
    m1.cost_per_m3 = 100.0;  // Kostenkennwert
    const model::MaterialId mat1 = *svc.addMaterial(m1);
    const model::MaterialId mat2 =
        *svc.addMaterial(makeMat("Ziegel", "Wand"));  // ohne Kennwert

    const model::WallId w1 = addWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});  // 2,4 m³
    ASSERT_TRUE(svc.setWallMaterial(w1, mat1));
    const model::WallId w2 =
        addWall(svc, {{0.0, 1000.0}, {4000.0, 1000.0}});  // 2,4 m³
    ASSERT_TRUE(svc.setWallMaterial(w2, mat2));

    const model::MaterialReport report = svc.materialList();
    ASSERT_EQ(report.lines.size(), 2U);
    // mat1 (Beton): 2,4 m³ × 100 = 240 Kosten.
    ASSERT_TRUE(report.lines[0].cost.has_value());
    EXPECT_NEAR(*report.lines[0].cost, 240.0, kTolM3);
    // mat2 (Ziegel): kein Kostenkennwert → nullopt, NICHT 0.
    EXPECT_FALSE(report.lines[1].cost.has_value());
    // total_cost = nur mat1 = 240 (mat2 trägt 0 bei).
    EXPECT_NEAR(report.total_cost, 240.0, kTolM3);
}
