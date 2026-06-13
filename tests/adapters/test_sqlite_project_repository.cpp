// Round-Trip-Tests für den SQLite-Persistenz-Adapter (BLD-002 Happy /
// BLD-003): speichern → laden → feldgleiches Domänen-Modell. Crash-Recovery
// (kill -9, LH-QA-005) + E-IO-002 (Medium voll) sind slice-008b.

#include "adapters/persistence/sqlite_project_repository.h"

#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"

namespace {

namespace fs = std::filesystem;
using bcad::adapters::persistence::SqliteProjectRepository;
using bcad::hexagon::model::Building;
using bcad::hexagon::model::Storey;
using bcad::hexagon::model::StoreyId;
using bcad::hexagon::model::Opening;
using bcad::hexagon::model::OpeningId;
using bcad::hexagon::model::OpeningKind;
using bcad::hexagon::model::Roof;
using bcad::hexagon::model::RoofId;
using bcad::hexagon::model::RoofType;
using bcad::hexagon::model::SwingDirection;
using bcad::hexagon::model::Wall;
using bcad::hexagon::model::WallId;
using bcad::hexagon::model::WallType;

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

fs::path tempPath(const char* name) { return fs::temp_directory_path() / name; }

TEST(SqliteProjectRepository_LH_FA_BLD_002_003, RoundTripErhaeltModell) {
    const SqliteProjectRepository repo;
    const Building original = sampleBuilding();
    const fs::path path = tempPath("bcad_roundtrip.bcad");
    fs::remove(path);

    repo.save(original, path);
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

    repo.save(Building{}, path);
    const Building loaded = repo.load(path);

    EXPECT_TRUE(loaded.storeys.empty());
    EXPECT_TRUE(loaded.walls.empty());
    fs::remove(path);
}

TEST(SqliteProjectRepository_LH_FA_BLD_002_003, SaveErsetztSauberOhneTempRest) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_overwrite.bcad");
    fs::remove(path);

    repo.save(sampleBuilding(), path);
    Building second;
    second.storeys.push_back({StoreyId{7}, 2750.0});
    repo.save(second, path);

    const Building loaded = repo.load(path);
    ASSERT_EQ(loaded.storeys.size(), 1U);
    EXPECT_EQ(static_cast<int>(loaded.storeys[0].id), 7);
    EXPECT_TRUE(loaded.walls.empty());
    EXPECT_FALSE(fs::exists(fs::path(path.string() + ".tmp")));
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
    repo.save(original, path);
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
// `origin.x` belegt die `%.17g`-Präzision diskriminierend (MED-1).
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
    original.roofs = {sattel, walm};

    const fs::path path = tempPath("bcad_roofs.bcad");
    fs::remove(path);
    repo.save(original, path);
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
    }
    fs::remove(path);
}

TEST(SqliteProjectRepository_LH_FA_BLD_002_003, LadenFehlendeDateiWirftNeutral) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_does_not_exist.bcad");
    fs::remove(path);
    EXPECT_THROW(repo.load(path), std::runtime_error);
}

}  // namespace
