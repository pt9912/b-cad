// Akzeptanz-Tests für die Volumen-Auswertung (slice-017c, EVL-002) — OCC-frei
// über ein GeometryKernelPort-Double (ADR-0001 §Testbarkeit). Geprüft wird
// LH-FA-EVL-002 (Netto-MATERIAL-Volumen = Bauteil-Volumen abzüglich Öffnungen,
// in m³) gegen spez. §1 LH-FA-EVL-001.a:
// - Wand: Footprint-Fläche (Shoelace) · Höhe − geklemmtes Öffnungsvolumen
//   (width · Wandstärke · clamped_height; Brüstung/Oberkante geklemmt).
// - Decke/Fundament: (Fläche − gültige Ausschnitte) · Dicke.
// - Treppe: Σ Stufenkörper = tread·width·h·(step_count+1)/2, GELÄNDER-FREI.
// - Dach welle-3 ausgenommen (dicke-loses Modell) — ein Dach ändert total_m3
//   NICHT.
// Die Orakel sind unabhängige, von Hand gerechnete Soll-Werte (konkrete
// mm-Maße → glatte m³); analytisch im Kern, KEIN solids_[].volume_mm3
// (das wäre die OCC-Adapter-Messung — ADR-0012 #4).

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>

#include "analytic_geometry_double.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/slab.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/volume_report.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driving/evaluate_port.h"
#include "hexagon/services/structure_edit_service.h"
#include "hexagon/services/volume_geometry.h"

namespace {

namespace model = bcad::hexagon::model;
namespace ports = bcad::hexagon::ports;
namespace services = bcad::hexagon::services;
using bcad::testing::AnalyticGeometry;

constexpr model::StoreyId kGroundStorey{1};  // Default-Geschoss, Höhe 2500 mm
// Volumina in m³; Netto-Volumina sind exakte Integer-mm³, ÷1e9 → glatte m³ —
// eine großzügige fp-Toleranz genügt.
constexpr double kVolumeToleranceM3 = 1e-6;

// Achsenparalleles Rechteck-Polygon (CCW).
model::Footprint rectFootprint(double x0, double y0, double x1, double y1) {
    return model::Footprint{{{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}}};
}

// Freistehende Wand (Segment; Default-Stärke 240, Höhe = Geschosshöhe 2500) —
// stumpfer Rechteck-Footprint (keine Nachbarn → kein Miter), damit das Orakel
// exakt Länge·Stärke·Höhe ist. Gibt die Id zurück; scheitert sauber.
model::WallId addFreestandingWall(services::StructureEditService& svc,
                                  model::Segment seg) {
    const std::optional<model::WallId> id = svc.addWall(kGroundStorey, seg);
    EXPECT_TRUE(id.has_value()) << "addWall hat die Wand verworfen";
    return id.value_or(model::WallId{});
}

}  // namespace

// EVL-002 Happy (Wand): freistehende Wand → walls_m3 = Länge·Stärke·Höhe.
TEST(Evaluate_LH_FA_EVL_002, FreistehendeWandMaterialvolumen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addFreestandingWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});

    // 4000·240·2500 = 2 400 000 000 mm³ = 2,4 m³.
    const model::VolumeReport v = svc.volume();
    EXPECT_NEAR(v.walls_m3, 2.4, kVolumeToleranceM3);
    EXPECT_NEAR(v.slabs_m3, 0.0, kVolumeToleranceM3);
    EXPECT_NEAR(v.stairs_m3, 0.0, kVolumeToleranceM3);
    EXPECT_NEAR(v.total_m3, 2.4, kVolumeToleranceM3);
}

// EVL-002 Wand mit Tür: das real entfernte, geklemmte Öffnungsvolumen wird
// abgezogen (Tür: Brüstung 0, voll in [0,Wandhöhe]).
TEST(Evaluate_LH_FA_EVL_002, WandMitTuerVolumenAbgezogen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall =
        addFreestandingWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});
    ASSERT_TRUE(svc.addDoor(wall, 1000.0).has_value());

    // Brutto 2,4 m³ − Tür (Default 900 breit · 240 Stärke · 2100 hoch,
    // Brüstung 0): 900·240·2100 = 453 600 000 mm³ = 0,4536 m³.
    // Netto = 2,4 − 0,4536 = 1,9464 m³.
    const model::VolumeReport v = svc.volume();
    EXPECT_NEAR(v.walls_m3, 1.9464, kVolumeToleranceM3);
    EXPECT_NEAR(v.total_m3, 1.9464, kVolumeToleranceM3);
}

// EVL-002 Boundary (Öffnungs-Klemmung): Fenster, dessen Oberkante (sill+height)
// über die Wandhöhe reicht → clamped_height = Wandhöhe − sill, nicht height.
TEST(Evaluate_LH_FA_EVL_002, FensterOberkanteUeberWandhoeheGeklemmt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall =
        addFreestandingWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});
    const std::optional<model::OpeningId> win = svc.addWindow(wall, 1000.0);
    ASSERT_TRUE(win.has_value());
    // Brüstung auf 2000 (Max): sill+height = 2000+1300 = 3300 > Wandhöhe 2500
    // → clamped_height = 2500 − 2000 = 500 (Oberkante geklemmt).
    svc.setWindowSill(*win, 2000.0);

    // Default-Fenster 1200 breit · 240 · 500 = 144 000 000 mm³ = 0,144 m³.
    // Netto = 2,4 − 0,144 = 2,256 m³.
    const model::VolumeReport v = svc.volume();
    EXPECT_NEAR(v.walls_m3, 2.256, kVolumeToleranceM3);
}

// EVL-002 Decke + Aussparung (über den Service): die Aussparung reduziert das
// Platten-Volumen.
TEST(Evaluate_LH_FA_EVL_002, DeckeMitAussparung) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    model::Slab proto;
    proto.storey_id = kGroundStorey;
    proto.type = model::SlabType::Decke;
    proto.footprint = rectFootprint(0.0, 0.0, 5000.0, 4000.0);
    proto.thickness_mm = 200.0;
    const std::optional<model::SlabId> id = svc.addSlab(proto);
    ASSERT_TRUE(id.has_value());
    ASSERT_TRUE(
        svc.addSlabCutout(*id, rectFootprint(1000.0, 1000.0, 2000.0, 2000.0)));

    // (5000·4000 − 1000·1000)·200 = 19 000 000·200 = 3 800 000 000 mm³ = 3,8 m³.
    const model::VolumeReport v = svc.volume();
    EXPECT_NEAR(v.slabs_m3, 3.8, kVolumeToleranceM3);
    EXPECT_NEAR(v.total_m3, 3.8, kVolumeToleranceM3);
}

// EVL-002 Aussparungs-Gate (LOW-1): gespeicherte Cutouts sind über
// addSlabCutout vorgefiltert — der „Außen-Ausschnitt → kein Abzug"-Beleg trifft
// daher die PURE slabNetVolumeMm3 mit einem handgebauten Außen-Cutout (der über
// addSlabCutout gar nicht erst in die Platte gelangte).
TEST(Evaluate_LH_FA_EVL_002, SlabAussenAusschnittWirdNichtAbgezogen) {
    model::Slab slab;
    slab.type = model::SlabType::Decke;
    slab.thickness_mm = 200.0;
    slab.footprint = rectFootprint(0.0, 0.0, 5000.0, 4000.0);

    // Außenliegender Ausschnitt → cutoutInsideSlab false → KEIN Abzug (4,0 m³).
    slab.cutouts = {rectFootprint(6000.0, 6000.0, 7000.0, 7000.0)};
    EXPECT_NEAR(services::slabNetVolumeMm3(slab) / 1.0e9, 4.0,
                kVolumeToleranceM3);

    // Innenliegender Ausschnitt → abgezogen (3,8 m³).
    slab.cutouts = {rectFootprint(1000.0, 1000.0, 2000.0, 2000.0)};
    EXPECT_NEAR(services::slabNetVolumeMm3(slab) / 1.0e9, 3.8,
                kVolumeToleranceM3);
}

// EVL-002 Treppe: Σ Stufenkörper, geländer-frei (Wert hängt NICHT von der
// Render-Streifenstärke kRailThicknessMm ab — der MR-009-Sentinel; mit Geländer
// läge der Wert höher).
TEST(Evaluate_LH_FA_EVL_002, TreppeStufenkoerperGelaenderFrei) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::StoreyId upper = svc.addStorey(model::kDefaultStoreyHeightMm);
    model::Stair proto;
    proto.from_storey_id = kGroundStorey;  // Höhe 2500
    proto.to_storey_id = upper;
    proto.type = model::StairType::Gerade;
    proto.start = {0.0, 0.0};
    proto.width_mm = 1000.0;
    proto.step_count = 15;
    proto.tread_mm = 280.0;
    ASSERT_TRUE(svc.addStair(proto).has_value());

    // tread·width·h·(n+1)/2 = 280·1000·2500·16/2 = 280·1000·2500·8
    //   = 5 600 000 000 mm³ = 5,6 m³.
    const model::VolumeReport v = svc.volume();
    EXPECT_NEAR(v.stairs_m3, 5.6, kVolumeToleranceM3);
    EXPECT_NEAR(v.total_m3, 5.6, kVolumeToleranceM3);
}

// EVL-002 gemischtes Gebäude: total_m3 = Summe der Bauteiltyp-Subtotale; ein
// Dach im Modell ändert das Material-Volumen NICHT (welle-3-Ausnahme).
TEST(Evaluate_LH_FA_EVL_002, GemischtesGebaeudeSummeUndDachAusgenommen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addFreestandingWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});  // 2,4 m³
    model::Slab slab;
    slab.storey_id = kGroundStorey;
    slab.type = model::SlabType::Decke;
    slab.footprint = rectFootprint(0.0, 0.0, 5000.0, 4000.0);
    slab.thickness_mm = 200.0;                              // 4,0 m³
    ASSERT_TRUE(svc.addSlab(slab).has_value());
    const model::StoreyId upper = svc.addStorey(model::kDefaultStoreyHeightMm);
    model::Stair stair;
    stair.from_storey_id = kGroundStorey;
    stair.to_storey_id = upper;
    stair.start = {6000.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 15;
    stair.tread_mm = 280.0;                                 // 5,6 m³
    ASSERT_TRUE(svc.addStair(stair).has_value());

    const model::VolumeReport before = svc.volume();
    EXPECT_NEAR(before.walls_m3, 2.4, kVolumeToleranceM3);
    EXPECT_NEAR(before.slabs_m3, 4.0, kVolumeToleranceM3);
    EXPECT_NEAR(before.stairs_m3, 5.6, kVolumeToleranceM3);
    EXPECT_NEAR(before.total_m3, 12.0, kVolumeToleranceM3);

    // Ein Dach ändert das Material-Volumen NICHT (dicke-loses Modell).
    model::Roof roof;
    roof.storey_id = kGroundStorey;
    roof.type = model::RoofType::Sattel;
    roof.origin = {0.0, 0.0};
    roof.width_mm = 5000.0;
    roof.depth_mm = 4000.0;
    roof.base_z_mm = 2500.0;
    roof.pitch_deg = 30.0;
    roof.overhang_mm = 500.0;
    ASSERT_TRUE(svc.addRoof(roof).has_value());

    const model::VolumeReport after = svc.volume();
    EXPECT_NEAR(after.total_m3, before.total_m3, kVolumeToleranceM3);
    EXPECT_NEAR(after.total_m3, 12.0, kVolumeToleranceM3);
}

// EVL-002 Boundary: leeres Modell → alle Felder 0, kein Wurf; und LOW-2:
// Treppe mit unbekanntem from_storey → storeyHeight 0 → Volumen 0 (Totalität).
TEST(Evaluate_LH_FA_EVL_002, LeeresModellNullUndTreppeUnbekanntesGeschoss) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    const model::VolumeReport empty = svc.volume();
    EXPECT_DOUBLE_EQ(empty.total_m3, 0.0);
    EXPECT_DOUBLE_EQ(empty.walls_m3, 0.0);
    EXPECT_DOUBLE_EQ(empty.slabs_m3, 0.0);
    EXPECT_DOUBLE_EQ(empty.stairs_m3, 0.0);

    model::Stair stair;
    stair.from_storey_id = model::StoreyId{999};
    stair.to_storey_id = model::StoreyId{1000};
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 15;
    stair.tread_mm = 280.0;
    // from_storey-Höhe 0 (unbekannt) → 0, kein Wurf.
    EXPECT_DOUBLE_EQ(services::stairNetVolumeMm3(stair, 0.0), 0.0);
}

// read-only/stabil (ADR-0012): zwei volume()-Abfragen identisch; das Modell
// bleibt unverändert (kein versteckter Re-Detect). Zugriff über den Port-Typ.
TEST(Evaluate_LH_FA_EVL_002, VolumenIstReadOnlyUndStabil) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addFreestandingWall(svc, {{0.0, 0.0}, {4000.0, 0.0}});

    const ports::driving::EvaluatePort& port = svc;
    const std::size_t walls_before = svc.building().walls.size();

    const model::VolumeReport first = port.volume();
    const model::VolumeReport second = port.volume();
    EXPECT_DOUBLE_EQ(first.total_m3, second.total_m3);
    EXPECT_DOUBLE_EQ(first.walls_m3, second.walls_m3);
    EXPECT_EQ(svc.building().walls.size(), walls_before);
}
