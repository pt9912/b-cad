// Akzeptanz-Tests für das Material-System (slice-017d, LH-FA-MAT-*) — OCC-frei
// über das GeometryKernelPort-Double (ADR-0001 §Testbarkeit). Material ist eine
// pure Domänen-Eigenschaft (spez. §2.1), verwaltet über EditStructurePort,
// read-only aufgelöst über EvaluatePort:
// - MAT-001: anlegen/ändern/entfernen; Name-Pflicht; removeMaterial RESTRICT
//   (noch zugewiesenes Material nicht löschbar, ADR-0006 #5).
// - MAT-002: Bibliothek = materials()-Liste.
// - MAT-003: Zuweisung (Override) + effektives Material; "kein Material" total.
// - MAT-005/006: Kennwerte (U-Wert/Kosten) über das zugewiesene Bauteil abrufbar.
// - kein op: Material-Mutationen melden nichts (Pull-Konsum, ADR-0012-Geist).

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>

#include "analytic_geometry_double.h"
#include "hexagon/model/material.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/slab.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/ports/driving/evaluate_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace ports = bcad::hexagon::ports;
namespace services = bcad::hexagon::services;
using bcad::testing::AnalyticGeometry;

constexpr model::StoreyId kGroundStorey{1};

model::Material makeMaterial(std::string name, std::string category) {
    model::Material material;
    material.name = std::move(name);
    material.category = std::move(category);
    return material;
}

// Zählt Modell-Änderungs-Meldungen (für den „kein op"-Beleg).
class RecordingListener final : public ports::driven::ModelChangedPort {
public:
    void onModelChanged(const ports::driven::ModelChange& /*change*/) override {
        ++count;
    }
    int count{0};
};

model::WallId addWall(services::StructureEditService& svc) {
    const std::optional<model::WallId> id =
        svc.addWall(kGroundStorey, {{0.0, 0.0}, {4000.0, 0.0}});
    EXPECT_TRUE(id.has_value());
    return id.value_or(model::WallId{});
}

}  // namespace

// MAT-001: anlegen (Name-Pflicht) / ändern / entfernen (unreferenziert).
TEST(Material_LH_FA_MAT_001, AnlegenAendernEntfernen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    const std::optional<model::MaterialId> id =
        svc.addMaterial(makeMaterial("Beton", "Tragwerk"));
    ASSERT_TRUE(id.has_value());
    ASSERT_EQ(svc.materials().size(), 1U);
    EXPECT_EQ(svc.materials().front().name, "Beton");

    // Name-Pflicht (MAT-001 Negative): leer / nur Whitespace → abgelehnt.
    EXPECT_FALSE(svc.addMaterial(makeMaterial("", "X")).has_value());
    EXPECT_FALSE(svc.addMaterial(makeMaterial("   ", "X")).has_value());
    EXPECT_EQ(svc.materials().size(), 1U);

    // Ändern (Name/Kategorie/Kennwerte).
    model::Material changed = makeMaterial("Stahlbeton", "Tragwerk");
    changed.u_value = 2.3;
    EXPECT_TRUE(svc.updateMaterial(*id, changed));
    EXPECT_EQ(svc.materials().front().name, "Stahlbeton");
    ASSERT_TRUE(svc.materials().front().u_value.has_value());
    EXPECT_DOUBLE_EQ(*svc.materials().front().u_value, 2.3);
    // Update mit leerem Namen / unbekannter Id → false, unverändert.
    EXPECT_FALSE(svc.updateMaterial(*id, makeMaterial(" ", "Y")));
    EXPECT_FALSE(svc.updateMaterial(model::MaterialId{999}, changed));
    EXPECT_EQ(svc.materials().front().name, "Stahlbeton");

    // Entfernen (unreferenziert) → weg; erneut → unbekannt → false.
    EXPECT_TRUE(svc.removeMaterial(*id));
    EXPECT_TRUE(svc.materials().empty());
    EXPECT_FALSE(svc.removeMaterial(*id));
}

// MAT-001 / ADR-0006 #5: noch zugewiesenes Material ist NICHT löschbar (RESTRICT).
TEST(Material_LH_FA_MAT_001, RemoveZugewiesenAbgelehntRestrict) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::MaterialId mat =
        *svc.addMaterial(makeMaterial("Ziegel", "Wand"));
    const model::WallId wall = addWall(svc);
    ASSERT_TRUE(svc.setWallMaterial(wall, mat));

    // Zugewiesen → removeMaterial verweigert (kein stiller Verlust, ADR-0006 #5).
    EXPECT_FALSE(svc.removeMaterial(mat));
    ASSERT_EQ(svc.materials().size(), 1U);

    // Nach Abwahl → löschbar.
    ASSERT_TRUE(svc.setWallMaterial(wall, std::nullopt));
    EXPECT_TRUE(svc.removeMaterial(mat));
    EXPECT_TRUE(svc.materials().empty());
}

// MAT-002: Bibliothek listet die verfügbaren Materialien.
TEST(Material_LH_FA_MAT_002, BibliothekListet) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    svc.addMaterial(makeMaterial("Beton", "Tragwerk"));
    svc.addMaterial(makeMaterial("Holz", "Ausbau"));
    svc.addMaterial(makeMaterial("Glas", "Fenster"));
    ASSERT_EQ(svc.materials().size(), 3U);
    EXPECT_EQ(svc.materials()[1].name, "Holz");
}

// MAT-003 / MAT-005 / MAT-006: Zuweisung + effektives Material mit Kennwerten.
TEST(Material_LH_FA_MAT_003, ZuweisungEffektivesMaterialUndKennwerte) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    model::Material proto = makeMaterial("Daemmziegel", "Wand");
    proto.u_value = 0.24;       // MAT-005
    proto.cost_per_m2 = 50.0;   // MAT-006
    proto.cost_per_m3 = 120.0;  // MAT-006
    const model::MaterialId mat = *svc.addMaterial(proto);
    const model::WallId wall = addWall(svc);

    // Unzugewiesen → kein effektives Material, kein Fehler (MAT-003 Negative).
    EXPECT_FALSE(svc.effectiveMaterial(wall).has_value());

    // Zuweisen → effektives Material trägt Name + Kennwerte (MAT-005/006).
    ASSERT_TRUE(svc.setWallMaterial(wall, mat));
    const std::optional<model::Material> eff = svc.effectiveMaterial(wall);
    ASSERT_TRUE(eff.has_value());
    EXPECT_EQ(eff->name, "Daemmziegel");
    ASSERT_TRUE(eff->u_value.has_value());
    EXPECT_DOUBLE_EQ(*eff->u_value, 0.24);
    ASSERT_TRUE(eff->cost_per_m2.has_value());
    EXPECT_DOUBLE_EQ(*eff->cost_per_m2, 50.0);
    ASSERT_TRUE(eff->cost_per_m3.has_value());
    EXPECT_DOUBLE_EQ(*eff->cost_per_m3, 120.0);

    // Unbekannte Material-Id → false, Zuweisung unverändert.
    EXPECT_FALSE(svc.setWallMaterial(wall, model::MaterialId{999}));
    ASSERT_TRUE(svc.effectiveMaterial(wall).has_value());
    EXPECT_EQ(svc.effectiveMaterial(wall)->name, "Daemmziegel");
    // Unbekanntes Bauteil → false.
    EXPECT_FALSE(svc.setWallMaterial(model::WallId{999}, mat));

    // Abwahl → wieder kein effektives Material; unbekanntes Bauteil → nullopt.
    ASSERT_TRUE(svc.setWallMaterial(wall, std::nullopt));
    EXPECT_FALSE(svc.effectiveMaterial(wall).has_value());
    EXPECT_FALSE(svc.effectiveMaterial(model::WallId{999}).has_value());
}

// MAT-003: Zuweisung auch an Dach und Decke (Override, gleiche Auflösung).
TEST(Material_LH_FA_MAT_003, RoofUndSlabZuweisung) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::MaterialId mat =
        *svc.addMaterial(makeMaterial("Dachziegel", "Dach"));

    model::Roof roof;
    roof.storey_id = kGroundStorey;
    roof.type = model::RoofType::Sattel;
    roof.origin = {0.0, 0.0};
    roof.width_mm = 5000.0;
    roof.depth_mm = 4000.0;
    roof.base_z_mm = 2500.0;
    roof.pitch_deg = 30.0;
    roof.overhang_mm = 500.0;
    const model::RoofId roof_id = *svc.addRoof(roof);

    model::Slab slab;
    slab.storey_id = kGroundStorey;
    slab.type = model::SlabType::Decke;
    slab.footprint.points = {{0.0, 0.0}, {5000.0, 0.0}, {5000.0, 4000.0},
                             {0.0, 4000.0}};
    slab.thickness_mm = 200.0;
    const model::SlabId slab_id = *svc.addSlab(slab);

    ASSERT_TRUE(svc.setRoofMaterial(roof_id, mat));
    ASSERT_TRUE(svc.setSlabMaterial(slab_id, mat));
    ASSERT_TRUE(svc.effectiveMaterial(roof_id).has_value());
    EXPECT_EQ(svc.effectiveMaterial(roof_id)->name, "Dachziegel");
    ASSERT_TRUE(svc.effectiveMaterial(slab_id).has_value());
    EXPECT_EQ(svc.effectiveMaterial(slab_id)->name, "Dachziegel");

    // RESTRICT: Dach + Decke referenzieren das Material → nicht löschbar.
    EXPECT_FALSE(svc.removeMaterial(mat));
}

// kein op (ADR-0012-Geist): Material-Mutationen melden nichts; ein Bauteil-Edit
// MELDET (Listener-Sanity) — nur Material ist still. read-only stabil.
TEST(Material_LH_FA_MAT_003, KeinOpUndReadOnlyStabil) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWall(svc);
    RecordingListener listener;
    svc.subscribe(listener);
    const int before = listener.count;

    const model::MaterialId mat =
        *svc.addMaterial(makeMaterial("Putz", "Ausbau"));
    svc.setWallMaterial(wall, mat);
    model::Material upd = makeMaterial("Putz", "Ausbau");
    upd.u_value = 0.7;
    svc.updateMaterial(mat, upd);
    svc.setWallMaterial(wall, std::nullopt);
    svc.removeMaterial(mat);

    // Keine einzige Material-Meldung (kein op).
    EXPECT_EQ(listener.count, before);

    // Sanity: ein Bauteil-Edit MELDET — der Listener ist verdrahtet, nur
    // Material ist bewusst still.
    svc.setWallThickness(wall, 300.0);
    EXPECT_GT(listener.count, before);

    // read-only/total: das Modell bleibt unberührt von der Auswertung.
    EXPECT_EQ(svc.building().walls.size(), 1U);
    svc.unsubscribe(listener);
}
