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

TEST(SqliteProjectRepository, RoundTripErhaeltModell) {
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

TEST(SqliteProjectRepository, LeeresProjektRoundTrip) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_empty.bcad");
    fs::remove(path);

    repo.save(Building{}, path);
    const Building loaded = repo.load(path);

    EXPECT_TRUE(loaded.storeys.empty());
    EXPECT_TRUE(loaded.walls.empty());
    fs::remove(path);
}

TEST(SqliteProjectRepository, SaveErsetztSauberOhneTempRest) {
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

TEST(SqliteProjectRepository, LadenFehlendeDateiWirftNeutral) {
    const SqliteProjectRepository repo;
    const fs::path path = tempPath("bcad_does_not_exist.bcad");
    fs::remove(path);
    EXPECT_THROW(repo.load(path), std::runtime_error);
}

}  // namespace
