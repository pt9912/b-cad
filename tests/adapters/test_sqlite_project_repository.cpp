// Round-Trip-Tests für den SQLite-Persistenz-Adapter (BLD-002 Happy /
// BLD-003): speichern → laden → feldgleiches Domänen-Modell. Crash-Recovery
// (kill -9, LH-QA-005) + E-IO-002 (Medium voll) sind slice-008b.

#include "adapters/persistence/sqlite_project_repository.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
#include <sqlite3.h>

#include "save_project_test_helper.h"  // saveProject (kern-gelieferter rise, 042d)
#include "hexagon/model/building.h"
#include "hexagon/model/constants.h"

namespace {

namespace fs = std::filesystem;
using bcad::adapters::persistence::SqliteProjectRepository;
using bcad::adapters::persistence::test::saveProject;
using bcad::hexagon::model::Building;
using bcad::hexagon::model::Storey;
using bcad::hexagon::model::StoreyId;
using bcad::hexagon::model::Opening;
using bcad::hexagon::model::OpeningId;
using bcad::hexagon::model::OpeningKind;
using bcad::hexagon::model::Roof;
using bcad::hexagon::model::RoofId;
using bcad::hexagon::model::RoofType;
using bcad::hexagon::model::Footprint;
using bcad::hexagon::model::Slab;
using bcad::hexagon::model::SlabId;
using bcad::hexagon::model::SlabType;
using bcad::hexagon::model::Stair;
using bcad::hexagon::model::StairId;
using bcad::hexagon::model::StairType;
using bcad::hexagon::model::SwingDirection;
using bcad::hexagon::model::Material;
using bcad::hexagon::model::MaterialId;
using bcad::hexagon::model::Wall;
using bcad::hexagon::model::WallId;
using bcad::hexagon::model::WallType;
using bcad::hexagon::model::Layer;
using bcad::hexagon::model::LayerId;
using bcad::hexagon::model::GuideLine;
using bcad::hexagon::model::GuideLineId;

Building sampleBuilding() {
    Building building;
    building.storeys.push_back({StoreyId{1}, 2500.0});
    building.storeys.push_back({StoreyId{2}, 3000.0});
    building.walls.push_back(
        {WallId{1}, StoreyId{1}, {0.0, 0.0}, {5000.0, 0.0}, 240.0, 2500.0,
         WallType::Aussen});
    building.walls.push_back(
        {WallId{2}, StoreyId{1}, {5000.0, 0.0}, {5000.0, 4000.0}, 115.0,
         2500.0, WallType::Innen});
    building.walls.push_back(
        {WallId{3}, StoreyId{2}, {0.0, 0.0}, {3000.0, 0.0}, 300.0, 3000.0,
         WallType::Trag});
    return building;
}

// Gebäude mit Material-Bibliothek + Zuweisung (slice-017e): ein „volles"
// Material (Kennwerte gesetzt) + ein „leeres" (alle optionalen Felder NULL —
// der NULL-vs-0.0-Sentinel); zugewiesen an Wand/Dach/Decke, eine Wand bleibt
// unzugewiesen.
Building materialBuilding() {
    Building b;
    b.storeys.push_back({StoreyId{1}, 2500.0});

    Material full;
    full.id = MaterialId{1};
    full.name = "Stahlbeton";
    full.category = "Tragwerk";
    full.u_value = 2.3;
    full.cost_per_m2 = 45.5;
    full.cost_per_m3 = 130.0;
    full.color_hex = "#888888";
    b.materials.push_back(full);

    Material bare;  // keine Kennwerte/Texte → alle optionalen Felder nullopt
    bare.id = MaterialId{2};
    bare.name = "Putz";
    bare.category = "Ausbau";
    b.materials.push_back(bare);

    Wall w1{WallId{1}, StoreyId{1}, {0.0, 0.0}, {5000.0, 0.0}, 240.0, 2500.0,
            WallType::Aussen};
    w1.material_id = MaterialId{1};
    b.walls.push_back(w1);
    b.walls.push_back({WallId{2}, StoreyId{1}, {0.0, 0.0}, {4000.0, 0.0}, 115.0,
                       2500.0, WallType::Innen});  // material_id nullopt

    Roof roof;
    roof.id = RoofId{1};
    roof.storey_id = StoreyId{1};
    roof.type = RoofType::Sattel;
    roof.origin = {0.0, 0.0};
    roof.width_mm = 5000.0;
    roof.depth_mm = 4000.0;
    roof.base_z_mm = 2500.0;
    roof.pitch_deg = 30.0;
    roof.overhang_mm = 500.0;
    roof.material_id = MaterialId{2};
    b.roofs.push_back(roof);

    Slab slab;
    slab.id = SlabId{1};
    slab.storey_id = StoreyId{1};
    slab.type = SlabType::Decke;
    slab.footprint.points = {{0.0, 0.0}, {5000.0, 0.0}, {5000.0, 4000.0},
                             {0.0, 4000.0}};
    slab.thickness_mm = 200.0;
    slab.material_id = MaterialId{1};
    b.slabs.push_back(slab);
    return b;
}

fs::path tempPath(const char* name) { return fs::temp_directory_path() / name; }

TEST(SqliteProjectRepository_LH_FA_BLD_002_003, RoundTripErhaeltModell) {
    const SqliteProjectRepository repo;
    const Building original = sampleBuilding();
    const fs::path path = tempPath("bcad_roundtrip.bcad");
    fs::remove(path);

    saveProject(repo, original, path);
    const Building loaded = repo.load(path);

    ASSERT_EQ(loaded.storeys.size(), original.storeys.size());
    for (std::size_t i = 0; i < original.storeys.size(); ++i) {
        EXPECT_EQ(static_cast<int>(loaded.storeys[i].id),
                  static_cast<int>(original.storeys[i].id));
        EXPECT_DOUBLE_EQ(loaded.storeys[i].height_mm,
                         original.storeys[i].height_mm);
    }

    ASSERT_EQ(loaded.walls.size(), original.walls.size());
    for (std::size_t i = 0; i < original.walls.size(); ++i) {
        const Wall& want = original.walls[i];
        const Wall& got = loaded.walls[i];
        EXPECT_EQ(static_cast<int>(got.id), static_cast<int>(want.id));
        EXPECT_EQ(static_cast<int>(got.storey_id),
                  static_cast<int>(want.storey_id));
        EXPECT_DOUBLE_EQ(got.start.x_mm, want.start.x_mm);
        EXPECT_DOUBLE_EQ(got.start.y_mm, want.start.y_mm);
        EXPECT_DOUBLE_EQ(got.end.x_mm, want.end.x_mm);
        EXPECT_DOUBLE_EQ(got.end.y_mm, want.end.y_mm);
        EXPECT_DOUBLE_EQ(got.thickness_mm, want.thickness_mm);
        EXPECT_DOUBLE_EQ(got.height_mm, want.height_mm);
        EXPECT_EQ(static_cast<int>(got.type), static_cast<int>(want.type));
    }
    fs::remove(path);
}

TEST(SqliteProjectRepository_LH_FA_BLD_002_003, LeeresProjektRoundTrip) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_empty.bcad");
    fs::remove(path);

    saveProject(repo, Building{}, path);
    const Building loaded = repo.load(path);

    EXPECT_TRUE(loaded.storeys.empty());
    EXPECT_TRUE(loaded.walls.empty());
    EXPECT_TRUE(loaded.slabs.empty());  // Leer-Pfad der slabs-Entity (slice-015c)
    EXPECT_TRUE(loaded.stairs.empty());  // Leer-Pfad der stairs-Entity (slice-016c)
    fs::remove(path);
}

TEST(SqliteProjectRepository_LH_FA_BLD_002_003, SaveErsetztSauberOhneTempRest) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_overwrite.bcad");
    fs::remove(path);

    saveProject(repo, sampleBuilding(), path);
    Building second;
    second.storeys.push_back({StoreyId{7}, 2750.0});
    saveProject(repo, second, path);

    const Building loaded = repo.load(path);
    ASSERT_EQ(loaded.storeys.size(), 1U);
    EXPECT_EQ(static_cast<int>(loaded.storeys[0].id), 7);
    EXPECT_TRUE(loaded.walls.empty());
    EXPECT_FALSE(fs::exists(fs::path(path.string() + ".tmp")));
    fs::remove(path);
}

// LH-FA-MAT-001/003 (ADR-0006, slice-017e): Material-Bibliothek + Zuweisung
// überleben den Round-Trip; NULL-Korrektheit (kein Wert ≠ 0.0).
TEST(SqliteProjectRepository_LH_FA_MAT, RoundTripErhaeltBibliothekUndZuweisung) {
    const SqliteProjectRepository repo;
    const Building original = materialBuilding();
    const fs::path path = tempPath("bcad_material.bcad");
    fs::remove(path);

    saveProject(repo, original, path);
    const Building loaded = repo.load(path);

    // (a) Bibliothek feldgleich (volles Material).
    ASSERT_EQ(loaded.materials.size(), 2U);
    const Material& m1 = loaded.materials[0];
    EXPECT_EQ(static_cast<int>(m1.id), 1);
    EXPECT_EQ(m1.name, "Stahlbeton");
    EXPECT_EQ(m1.category, "Tragwerk");
    ASSERT_TRUE(m1.u_value.has_value());
    EXPECT_DOUBLE_EQ(*m1.u_value, 2.3);
    ASSERT_TRUE(m1.cost_per_m2.has_value());
    EXPECT_DOUBLE_EQ(*m1.cost_per_m2, 45.5);
    ASSERT_TRUE(m1.cost_per_m3.has_value());
    EXPECT_DOUBLE_EQ(*m1.cost_per_m3, 130.0);
    ASSERT_TRUE(m1.color_hex.has_value());
    EXPECT_EQ(*m1.color_hex, "#888888");

    // (b) NULL-Korrektheit (Code-Review-Sentinel): Material ohne Kennwerte →
    // nullopt, NICHT 0.0 (sqlite3_column_double gäbe für NULL still 0.0).
    const Material& m2 = loaded.materials[1];
    EXPECT_EQ(m2.name, "Putz");
    EXPECT_FALSE(m2.u_value.has_value());
    EXPECT_FALSE(m2.cost_per_m2.has_value());
    EXPECT_FALSE(m2.cost_per_m3.has_value());
    EXPECT_FALSE(m2.color_hex.has_value());
    EXPECT_FALSE(m2.texture_path.has_value());

    // (a) Zuweisung erhalten + manuelle Auflösung gegen loaded.materials (F1:
    // effectiveMaterial ist eine Service-Methode, nicht auf dem Building).
    ASSERT_EQ(loaded.walls.size(), 2U);
    ASSERT_TRUE(loaded.walls[0].material_id.has_value());
    EXPECT_EQ(static_cast<int>(*loaded.walls[0].material_id), 1);
    const auto resolved = std::find_if(
        loaded.materials.begin(), loaded.materials.end(),
        [&](const Material& m) { return m.id == *loaded.walls[0].material_id; });
    ASSERT_NE(resolved, loaded.materials.end());
    EXPECT_EQ(resolved->name, "Stahlbeton");

    // (c) Bauteil ohne Material → nullopt (nicht MaterialId{0}).
    EXPECT_FALSE(loaded.walls[1].material_id.has_value());

    // (d) Dach + Decke: material_id erhalten.
    ASSERT_EQ(loaded.roofs.size(), 1U);
    ASSERT_TRUE(loaded.roofs[0].material_id.has_value());
    EXPECT_EQ(static_cast<int>(*loaded.roofs[0].material_id), 2);
    ASSERT_EQ(loaded.slabs.size(), 1U);
    ASSERT_TRUE(loaded.slabs[0].material_id.has_value());
    EXPECT_EQ(static_cast<int>(*loaded.slabs[0].material_id), 1);

    fs::remove(path);
}

// (e) Leere Bibliothek + unzugewiesene Bauteile round-trippen sauber.
TEST(SqliteProjectRepository_LH_FA_MAT, LeereBibliothekUndUnzugewiesen) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_material_empty.bcad");
    fs::remove(path);

    saveProject(repo, sampleBuilding(), path);  // Wände, keine Materialien
    const Building loaded = repo.load(path);

    EXPECT_TRUE(loaded.materials.empty());
    ASSERT_FALSE(loaded.walls.empty());
    for (const Wall& wall : loaded.walls) {
        EXPECT_FALSE(wall.material_id.has_value());  // unzugewiesen → nullopt
    }
    fs::remove(path);
}

// LH-FA-DRW-005/006 (ADR-0018, slice-032b): Ebenen + Hilfslinien überleben den
// Round-Trip feldgleich — layers (name/visible/locked/Farbe, NULL-korrekt) +
// guide_lines (Endpunkte exakt, storey_id/layer_id erhalten). Das ist die
// benutzer-beobachtbare Fundament-Stufe (Persistenz-Round-Trip; die Export-
// Sichtbarkeit ist slice-032c).
Building drawingBuilding() {
    Building building;
    building.storeys.push_back({StoreyId{1}, 2500.0});
    building.storeys.push_back({StoreyId{2}, 3000.0});
    Layer visible;
    visible.id = LayerId{1};
    visible.name = "Achsen";
    visible.visible = true;
    visible.locked = false;
    visible.color_hex = "#00ff00";
    building.layers.push_back(visible);
    Layer hidden;
    hidden.id = LayerId{2};
    hidden.name = "Skizze";
    hidden.visible = false;
    hidden.locked = true;  // color_hex bleibt nullopt (NULL-Korrektheit)
    building.layers.push_back(hidden);
    building.guide_lines.push_back({GuideLineId{1}, StoreyId{1}, LayerId{1},
                                    {{100.0, 200.0}, {900.0, 200.0}}});
    // storey_id ≠ layer_id (Geschoss 2, Ebene 1) — fängt einen storey_id↔layer_id-
    // Bind/Spalten-Swap, der bei gleichen Werten unentdeckt bliebe (Review-MED-1).
    building.guide_lines.push_back({GuideLineId{2}, StoreyId{2}, LayerId{1},
                                    {{0.0, 0.0}, {0.0, 1500.0}}});
    return building;
}

TEST(SqliteProjectRepository_LH_FA_DRW, RoundTripErhaeltEbeneUndHilfslinie) {
    const SqliteProjectRepository repo;
    const Building original = drawingBuilding();
    const fs::path path = tempPath("bcad_drawing.bcad");
    fs::remove(path);

    saveProject(repo, original, path);
    const Building loaded = repo.load(path);

    // Ebenen feldgleich (inkl. NULL-Farbe der zweiten Ebene).
    ASSERT_EQ(loaded.layers.size(), 2U);
    const Layer& l1 = loaded.layers[0];
    EXPECT_EQ(static_cast<int>(l1.id), 1);
    EXPECT_EQ(l1.name, "Achsen");
    EXPECT_TRUE(l1.visible);
    EXPECT_FALSE(l1.locked);
    ASSERT_TRUE(l1.color_hex.has_value());
    EXPECT_EQ(*l1.color_hex, "#00ff00");
    const Layer& l2 = loaded.layers[1];
    EXPECT_EQ(l2.name, "Skizze");
    EXPECT_FALSE(l2.visible);
    EXPECT_TRUE(l2.locked);
    EXPECT_FALSE(l2.color_hex.has_value());  // NULL → nullopt (nicht "")

    // Hilfslinien feldgleich: Endpunkte exakt + storey_id/layer_id erhalten.
    ASSERT_EQ(loaded.guide_lines.size(), 2U);
    const GuideLine& g1 = loaded.guide_lines[0];
    EXPECT_EQ(static_cast<int>(g1.id), 1);
    EXPECT_EQ(static_cast<int>(g1.storey_id), 1);
    EXPECT_EQ(static_cast<int>(g1.layer_id), 1);
    EXPECT_DOUBLE_EQ(g1.segment.start.x_mm, 100.0);
    EXPECT_DOUBLE_EQ(g1.segment.start.y_mm, 200.0);
    EXPECT_DOUBLE_EQ(g1.segment.end.x_mm, 900.0);
    EXPECT_DOUBLE_EQ(g1.segment.end.y_mm, 200.0);
    const GuideLine& g2 = loaded.guide_lines[1];
    EXPECT_EQ(static_cast<int>(g2.storey_id), 2);  // storey ≠ layer: fängt einen Swap
    EXPECT_EQ(static_cast<int>(g2.layer_id), 1);
    EXPECT_DOUBLE_EQ(g2.segment.start.x_mm, 0.0);
    EXPECT_DOUBLE_EQ(g2.segment.start.y_mm, 0.0);
    EXPECT_DOUBLE_EQ(g2.segment.end.x_mm, 0.0);
    EXPECT_DOUBLE_EQ(g2.segment.end.y_mm, 1500.0);

    fs::remove(path);
}

// Leere Ebenen/Hilfslinien round-trippen sauber (keine DRW-Daten).
TEST(SqliteProjectRepository_LH_FA_DRW, LeereEbenenUndHilfslinien) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_drawing_empty.bcad");
    fs::remove(path);

    saveProject(repo, sampleBuilding(), path);  // Wände, keine DRW-Daten
    const Building loaded = repo.load(path);

    EXPECT_TRUE(loaded.layers.empty());
    EXPECT_TRUE(loaded.guide_lines.empty());
    fs::remove(path);
}

// LH-FA-DOR-001/WIN-001 (ADR-0011/0006, slice-013c): Türen + Fenster
// überleben den Round-Trip feldgleich (openings + doors/windows-CTI).
TEST(SqliteProjectRepository_LH_FA_DOR_WIN, RoundTripErhaeltOeffnungen) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});
    original.walls.push_back({WallId{1}, StoreyId{1}, {0.0, 0.0},
                              {5000.0, 0.0}, 240.0, 2500.0, WallType::Aussen});
    Opening door{};
    door.id = OpeningId{1};
    door.wall_id = WallId{1};
    door.kind = OpeningKind::Door;
    door.offset_mm = 1000.0;
    door.width_mm = 900.0;
    door.height_mm = 2100.0;
    door.sill_height_mm = 0.0;
    door.swing = SwingDirection::Right;
    Opening window{};
    window.id = OpeningId{2};
    window.wall_id = WallId{1};
    window.kind = OpeningKind::Window;
    window.offset_mm = 3000.0;
    window.width_mm = 1200.0;
    window.height_mm = 1300.0;
    window.sill_height_mm = 900.0;
    original.openings = {door, window};

    const fs::path path = tempPath("bcad_openings.bcad");
    fs::remove(path);
    saveProject(repo, original, path);
    const Building loaded = repo.load(path);

    ASSERT_EQ(loaded.openings.size(), original.openings.size());
    for (std::size_t i = 0; i < original.openings.size(); ++i) {
        const Opening& want = original.openings[i];
        const Opening& got = loaded.openings[i];
        EXPECT_EQ(static_cast<int>(got.id), static_cast<int>(want.id));
        EXPECT_EQ(static_cast<int>(got.wall_id), static_cast<int>(want.wall_id));
        EXPECT_EQ(static_cast<int>(got.kind), static_cast<int>(want.kind));
        EXPECT_DOUBLE_EQ(got.offset_mm, want.offset_mm);
        EXPECT_DOUBLE_EQ(got.width_mm, want.width_mm);
        EXPECT_DOUBLE_EQ(got.height_mm, want.height_mm);
        EXPECT_DOUBLE_EQ(got.sill_height_mm, want.sill_height_mm);
        if (want.kind == OpeningKind::Door) {
            EXPECT_EQ(static_cast<int>(got.swing), static_cast<int>(want.swing));
        }
    }
    fs::remove(path);
}

// LH-FA-ROF-001 (ADR-0011/0006, slice-014c): Dächer überleben den
// Round-Trip feldgleich (roofs + footprint_json). Der nicht-glatte
// `origin.x` belegt die `%.17g`-Präzision diskriminierend (MED-1). Seit
// slice-023c (LH-FA-ROF-006): `thickness_mm` (Dach-Volumenkörper) trägt einen
// nicht-glatten, vom Default (200) verschiedenen Wert — `bind_double` ist
// bit-exakt, der Wert diskriminiert daher gegen (a) ein Test, der nur auf dem
// geteilten 0.0-Default beider Seiten „besteht", und (b) eine Spalten-/
// Index-Vertauschung in SELECT/INSERT (LOW-2).
TEST(SqliteProjectRepository_LH_FA_ROF_001, RoundTripErhaeltDaecher) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});
    Roof sattel{};
    sattel.id = RoofId{1};
    sattel.storey_id = StoreyId{1};
    sattel.type = RoofType::Sattel;
    sattel.origin = {1234.56789012345, -50.0};  // nicht-glatt → prüft %.17g
    sattel.width_mm = 8000.0;
    sattel.depth_mm = 6000.0;
    sattel.base_z_mm = 2500.0;
    sattel.pitch_deg = 30.0;
    sattel.overhang_mm = 500.0;
    sattel.thickness_mm = 223.456789012345;  // nicht-glatt, ≠ Default (slice-023c)
    Roof walm{};
    walm.id = RoofId{2};
    walm.storey_id = StoreyId{1};
    walm.type = RoofType::Walm;
    walm.origin = {0.0, 0.0};
    walm.width_mm = 5000.0;
    walm.depth_mm = 5000.0;
    walm.base_z_mm = 3000.0;
    walm.pitch_deg = 45.0;
    walm.overhang_mm = 300.0;
    walm.thickness_mm = 321.0;  // ≠ Default 200 (slice-023c)
    original.roofs = {sattel, walm};

    const fs::path path = tempPath("bcad_roofs.bcad");
    fs::remove(path);
    saveProject(repo, original, path);
    const Building loaded = repo.load(path);

    ASSERT_EQ(loaded.roofs.size(), original.roofs.size());
    for (std::size_t i = 0; i < original.roofs.size(); ++i) {
        const Roof& want = original.roofs[i];
        const Roof& got = loaded.roofs[i];
        EXPECT_EQ(static_cast<int>(got.id), static_cast<int>(want.id));
        EXPECT_EQ(static_cast<int>(got.storey_id),
                  static_cast<int>(want.storey_id));
        EXPECT_EQ(static_cast<int>(got.type), static_cast<int>(want.type));
        EXPECT_DOUBLE_EQ(got.origin.x_mm, want.origin.x_mm);
        EXPECT_DOUBLE_EQ(got.origin.y_mm, want.origin.y_mm);
        EXPECT_DOUBLE_EQ(got.width_mm, want.width_mm);
        EXPECT_DOUBLE_EQ(got.depth_mm, want.depth_mm);
        EXPECT_DOUBLE_EQ(got.base_z_mm, want.base_z_mm);
        EXPECT_DOUBLE_EQ(got.pitch_deg, want.pitch_deg);
        EXPECT_DOUBLE_EQ(got.overhang_mm, want.overhang_mm);
        EXPECT_DOUBLE_EQ(got.thickness_mm, want.thickness_mm);  // slice-023c
    }
    fs::remove(path);
}

// LH-FA-SLB-001 / LH-FA-FND-001 (ADR-0011/0006, slice-015c): Platten überleben
// den Round-Trip feldgleich (slabs + verschachteltes polygon_json). Die Decke
// trägt einen Ausschnitt (LH-FA-SLB-003) und einen nicht-glatten `double` →
// belegt `%.17g` diskriminierend (MED-1, Präzedenz 014c). Das Fundament ohne
// Ausschnitt prüft den Ein-Ring-Fall (cutouts leer, MED-1).
TEST(SqliteProjectRepository_LH_FA_SLB_FND, RoundTripErhaeltPlatten) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});

    Slab decke{};
    decke.id = SlabId{1};
    decke.storey_id = StoreyId{1};
    decke.type = SlabType::Decke;
    decke.footprint.points = {{0.0, 0.0},
                              {5000.0, 0.0},
                              {5000.0, 4000.0},
                              {1234.56789012345, 4000.0}};  // nicht-glatt → %.17g
    decke.thickness_mm = 200.0;
    decke.cutouts.push_back(Footprint{{{1000.0, 1000.0},
                                       {2000.0, 1000.0},
                                       {2000.0, 2000.0},
                                       {1000.0, 2000.0}}});  // LH-FA-SLB-003

    Slab fundament{};
    fundament.id = SlabId{2};
    fundament.storey_id = StoreyId{1};
    fundament.type = SlabType::Fundament;
    fundament.footprint.points = {
        {0.0, 0.0}, {6000.0, 0.0}, {6000.0, 5000.0}, {0.0, 5000.0}};
    fundament.thickness_mm = 500.0;  // ohne Ausschnitt → Ein-Ring polygon_json

    Slab bodenplatte{};  // dritter SlabType-Zweig (Mapper-Deckung)
    bodenplatte.id = SlabId{3};
    bodenplatte.storey_id = StoreyId{1};
    bodenplatte.type = SlabType::Bodenplatte;
    bodenplatte.footprint.points = {
        {0.0, 0.0}, {7000.0, 0.0}, {7000.0, 6000.0}, {0.0, 6000.0}};
    bodenplatte.thickness_mm = 250.0;

    original.slabs = {decke, fundament, bodenplatte};

    const fs::path path = tempPath("bcad_slabs.bcad");
    fs::remove(path);
    saveProject(repo, original, path);
    const Building loaded = repo.load(path);

    ASSERT_EQ(loaded.slabs.size(), original.slabs.size());
    for (std::size_t i = 0; i < original.slabs.size(); ++i) {
        const Slab& want = original.slabs[i];
        const Slab& got = loaded.slabs[i];
        EXPECT_EQ(static_cast<int>(got.id), static_cast<int>(want.id));
        EXPECT_EQ(static_cast<int>(got.storey_id),
                  static_cast<int>(want.storey_id));
        EXPECT_EQ(static_cast<int>(got.type), static_cast<int>(want.type));
        EXPECT_DOUBLE_EQ(got.thickness_mm, want.thickness_mm);
        ASSERT_EQ(got.footprint.points.size(), want.footprint.points.size());
        for (std::size_t p = 0; p < want.footprint.points.size(); ++p) {
            EXPECT_DOUBLE_EQ(got.footprint.points[p].x_mm,
                             want.footprint.points[p].x_mm);
            EXPECT_DOUBLE_EQ(got.footprint.points[p].y_mm,
                             want.footprint.points[p].y_mm);
        }
        ASSERT_EQ(got.cutouts.size(), want.cutouts.size());
        for (std::size_t c = 0; c < want.cutouts.size(); ++c) {
            ASSERT_EQ(got.cutouts[c].points.size(), want.cutouts[c].points.size());
            for (std::size_t p = 0; p < want.cutouts[c].points.size(); ++p) {
                EXPECT_DOUBLE_EQ(got.cutouts[c].points[p].x_mm,
                                 want.cutouts[c].points[p].x_mm);
                EXPECT_DOUBLE_EQ(got.cutouts[c].points[p].y_mm,
                                 want.cutouts[c].points[p].y_mm);
            }
        }
    }
    EXPECT_TRUE(loaded.slabs[1].cutouts.empty());  // Ein-Ring-Fall (MED-1)
    EXPECT_EQ(static_cast<int>(loaded.slabs[2].type),
              static_cast<int>(SlabType::Bodenplatte));  // dritter Mapper-Zweig
    fs::remove(path);
}

// White-box-Korruption für die Negativ-Parse-Tests: roher UPDATE auf die
// gespeicherte .bcad. Im Produktcode wäre rohes sqlite3 außerhalb des
// Persistenz-Adapters ein a-check-Regel-D-Verstoß — Regel D wie auch `lint`
// (clang-tidy) greppen aber nur `src/`, nicht `tests/`, daher hier zulässig.
void corruptColumn(const fs::path& path, const char* sql) {
    sqlite3* db = nullptr;
    sqlite3_open(path.string().c_str(), &db);
    sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
    sqlite3_close(db);
}

// Liest einen einzelnen DOUBLE-Skalar direkt per SQL (Regel-D-frei in `tests/`,
// s. corruptColumn). Für die Verifikation **write-derived** Spalten, die `load`
// bewusst NICHT zurückliest (`rise_mm`, slice-042d) — der einzige Weg, den
// kern-gelieferten Wert zu prüfen.
double readDoubleScalar(const fs::path& path, const char* sql) {
    sqlite3* db = nullptr;
    sqlite3_open(path.string().c_str(), &db);
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    double value = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        value = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return value;
}

// LH-FA-SLB-001 (slice-015c, Code-Review MED-2): der polygon_json-/slab_type-
// Parser ist total nach außen — verfälschter DB-Inhalt führt zu neutralem
// E-IO (`std::runtime_error`), nicht zu Crash/UB/stillem Falschwert. Genau die
// Fehlerklasse, die in 013b/014b/015b durch grüne Gates rutschte.
TEST(SqliteProjectRepository_LH_FA_SLB_FND, MalformedSpaltenWerfenNeutral) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});
    Slab slab{};
    slab.id = SlabId{1};
    slab.storey_id = StoreyId{1};
    slab.type = SlabType::Decke;
    slab.footprint.points = {{0.0, 0.0}, {1000.0, 0.0}, {1000.0, 1000.0}};
    slab.thickness_mm = 200.0;
    original.slabs = {slab};

    const fs::path path = tempPath("bcad_slab_malformed.bcad");

    // (a) ungerade Wertzahl im Ring → parseRing wirft.
    fs::remove(path);
    saveProject(repo, original, path);
    corruptColumn(path, "UPDATE slabs SET polygon_json='[[1,2,3]]';");
    EXPECT_THROW(repo.load(path), std::runtime_error);

    // (b) unbalancierte Klammern → parseSlabPolygonJson wirft.
    saveProject(repo, original, path);
    corruptColumn(path, "UPDATE slabs SET polygon_json='[[1,2,3,4]';");
    EXPECT_THROW(repo.load(path), std::runtime_error);

    // (c) Müll-Suffix im Zahl-Token → Vollständig-Verbrauch-Check wirft (MED-1).
    saveProject(repo, original, path);
    corruptColumn(path, "UPDATE slabs SET polygon_json='[[1,2,3x,4]]';");
    EXPECT_THROW(repo.load(path), std::runtime_error);

    // (d) unbekannter slab_type (Schema ohne CHECK) → textToSlabType wirft.
    saveProject(repo, original, path);
    corruptColumn(path, "UPDATE slabs SET slab_type='xyz';");
    EXPECT_THROW(repo.load(path), std::runtime_error);

    fs::remove(path);
}

// LH-FA-ROF-006 (slice-023c, Plan-Review MED-1): Default-Pfad-Sonde. Eine
// `roofs`-Zeile, die OHNE `thickness_mm` eingefügt wird (simuliert eine
// Alt-Datei vor 023c), trägt beim Laden den SQL-`DEFAULT 200` == die
// Domänen-Konstante `kDefaultRoofThicknessMm`. Geprüft gegen die KONSTANTE
// (nicht bare 200.0), um YAML-Default ↔ Domänen-Default im Test zu pinnen.
TEST(SqliteProjectRepository_LH_FA_ROF_006, FehlendeDickeLaedtAlsDefault) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});
    // Ein „normales" Dach, damit projects/storeys-Zeilen + FK-Ziele existieren.
    Roof seed{};
    seed.id = RoofId{1};
    seed.storey_id = StoreyId{1};
    seed.type = RoofType::Pult;
    seed.origin = {0.0, 0.0};
    seed.width_mm = 4000.0;
    seed.depth_mm = 3000.0;
    seed.base_z_mm = 2500.0;
    seed.pitch_deg = 20.0;
    seed.overhang_mm = 400.0;
    seed.thickness_mm = 350.0;
    original.roofs = {seed};

    const fs::path path = tempPath("bcad_roof_thickness_default.bcad");
    fs::remove(path);
    saveProject(repo, original, path);
    // Zeile ohne thickness_mm einfügen → SQL-DEFAULT 200 feuert (FK: storey 1).
    corruptColumn(path,
                  "INSERT INTO roofs (id,project_id,storey_id,roof_type,"
                  "footprint_json) VALUES (2,1,1,'pult','[0,0,1000,1000,0]');");
    const Building loaded = repo.load(path);

    ASSERT_EQ(loaded.roofs.size(), 2U);
    const Roof& defaulted = loaded.roofs[1];  // id 2, ORDER BY id
    EXPECT_EQ(static_cast<int>(defaulted.id), 2);
    EXPECT_DOUBLE_EQ(defaulted.thickness_mm,
                     bcad::hexagon::model::kDefaultRoofThicknessMm);
    // Die explizit geschriebene Zeile behält ihren Wert (Regression).
    EXPECT_DOUBLE_EQ(loaded.roofs[0].thickness_mm, 350.0);
    fs::remove(path);
}

// LH-FA-STR-001 (ADR-0011/0006, slice-016c): Treppen überleben den Round-Trip
// feldgleich (stairs). `rise_mm` ist abgeleitet → write-derived, beim Laden
// NICHT zurückgelesen; `name` nicht im Modell. Der nicht-glatte `start.x` belegt
// die `sqlite3_bind_double`-Exaktheit diskriminierend (kein %.17g-Text-Pfad für
// stairs — INFO-3).
TEST(SqliteProjectRepository_LH_FA_STR_001, RoundTripErhaeltTreppen) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});  // untere Etage
    original.storeys.push_back({StoreyId{2}, 3000.0});  // obere Etage

    Stair stair{};
    stair.id = StairId{1};
    stair.from_storey_id = StoreyId{1};
    stair.to_storey_id = StoreyId{2};
    stair.type = StairType::Gerade;
    stair.start = {1234.56789012345, -50.0};  // nicht-glatt → bind_double exakt
    stair.width_mm = 1000.0;
    stair.step_count = 15;
    stair.tread_mm = 287.3125;
    original.stairs = {stair};

    const fs::path path = tempPath("bcad_stairs.bcad");
    fs::remove(path);
    saveProject(repo, original, path);
    const Building loaded = repo.load(path);

    ASSERT_EQ(loaded.stairs.size(), original.stairs.size());
    const Stair& want = original.stairs[0];
    const Stair& got = loaded.stairs[0];
    EXPECT_EQ(static_cast<int>(got.id), static_cast<int>(want.id));
    EXPECT_EQ(static_cast<int>(got.from_storey_id),
              static_cast<int>(want.from_storey_id));
    EXPECT_EQ(static_cast<int>(got.to_storey_id),
              static_cast<int>(want.to_storey_id));
    EXPECT_EQ(static_cast<int>(got.type), static_cast<int>(want.type));
    EXPECT_DOUBLE_EQ(got.start.x_mm, want.start.x_mm);
    EXPECT_DOUBLE_EQ(got.start.y_mm, want.start.y_mm);
    EXPECT_DOUBLE_EQ(got.width_mm, want.width_mm);
    EXPECT_EQ(got.step_count, want.step_count);
    EXPECT_DOUBLE_EQ(got.tread_mm, want.tread_mm);
    fs::remove(path);
}

// slice-042d (ADR-0020): der `rise`-Skalar wird jetzt **kern-geliefert** (der
// Adapter **bindet** ihn statt ihn via `services/geometry` zu berechnen). Weil
// `load` `rise_mm` bewusst NIE zurückliest (write-derived), pinnt dieser Test die
// Spalte **direkt per SQL** `== stairRiseMm` gegen die bekannte Geschosshöhe —
// Beleg der Byte-Identität zum früher adapter-berechneten Wert (sonst bliebe der
// Berechnungs-Umzug un-genetzt; das Netz-Loch des Plan-Reviews).
TEST(SqliteProjectRepository_LH_FA_STR_001, RiseMmKernGeliefertByteGleich) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});  // from_storey liefert Höhe
    original.storeys.push_back({StoreyId{2}, 3000.0});
    Stair stair{};
    stair.id = StairId{1};
    stair.from_storey_id = StoreyId{1};
    stair.to_storey_id = StoreyId{2};
    stair.type = StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 16;  // 2500/16 = 156.25 → nicht-glatt, exakter bind_double
    stair.tread_mm = 280.0;
    original.stairs = {stair};

    const fs::path path = tempPath("bcad_stair_rise.bcad");
    fs::remove(path);
    saveProject(repo, original, path);

    const double expected = bcad::hexagon::services::stairRiseMm(stair, 2500.0);
    const double stored =
        readDoubleScalar(path, "SELECT rise_mm FROM stairs WHERE id=1;");
    EXPECT_DOUBLE_EQ(stored, expected);
    EXPECT_DOUBLE_EQ(stored, 2500.0 / 16.0);  // Verankerung: bekannte Höhe/step
    fs::remove(path);
}

// slice-042d: danglendes `from_storey_id` → der (kern-seitige) Aufrufer wirft
// **E-IO vor** `save` (die throw-Semantik des früheren adapter-lokalen
// `fromStoreyHeight` überlebt den Umzug); **kein Teil-Save**, Zieldatei + Temp
// unberührt (beobachtbar identisch zum früheren Rollback-mitten-in-der-Transaktion).
TEST(SqliteProjectRepository_LH_FA_STR_001, DanglendesGeschossWirftEIOKeinTeilSave) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});
    Stair stair{};
    stair.id = StairId{1};
    stair.from_storey_id = StoreyId{99};  // existiert NICHT in storeys → dangling
    stair.to_storey_id = StoreyId{1};
    stair.type = StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 15;
    stair.tread_mm = 280.0;
    original.stairs = {stair};

    const fs::path path = tempPath("bcad_stair_dangling.bcad");
    fs::remove(path);

    std::string what;
    try {
        saveProject(repo, original, path);
        ADD_FAILURE() << "saveProject haette werfen muessen";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("E-IO"), std::string::npos) << what;
    EXPECT_FALSE(fs::exists(path));                            // kein Teil-Save
    EXPECT_FALSE(fs::exists(fs::path(path.string() + ".tmp")));
    fs::remove(path);
}

// LH-FA-STR-001 (slice-016c, MR-006 MED-2): unbekannter stair_type → `load`
// wirft neutral E-IO (`textToStairType` total; Schema ohne CHECK). Muster 015c.
TEST(SqliteProjectRepository_LH_FA_STR_001, MalformedStairTypeWirftNeutral) {
    const SqliteProjectRepository repo;
    Building original;
    original.storeys.push_back({StoreyId{1}, 2500.0});
    original.storeys.push_back({StoreyId{2}, 3000.0});
    Stair stair{};
    stair.id = StairId{1};
    stair.from_storey_id = StoreyId{1};
    stair.to_storey_id = StoreyId{2};
    stair.type = StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 15;
    stair.tread_mm = 280.0;
    original.stairs = {stair};

    const fs::path path = tempPath("bcad_stair_malformed.bcad");
    fs::remove(path);
    saveProject(repo, original, path);
    corruptColumn(path, "UPDATE stairs SET stair_type='wendel';");
    EXPECT_THROW(repo.load(path), std::runtime_error);
    fs::remove(path);
}

TEST(SqliteProjectRepository_LH_FA_BLD_002_003, LadenFehlendeDateiWirftNeutral) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_does_not_exist.bcad");
    fs::remove(path);
    EXPECT_THROW(repo.load(path), std::runtime_error);
}

}  // namespace
