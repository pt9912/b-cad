// Save-Use-Case (slice-047a): baut die kern-abgeleiteten Persistenz-Skalare
// (`rise` je Treppe) und speichert über den `ProjectRepositoryPort`. Kern-Test
// mit einem **Fake-Repo** (dependency-frei) — prüft die Ableitung + die
// **fail-closed**-Semantik (danglendes `from_storey` → Wurf VOR `save`).

#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/persisted_derivations.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/storey.h"
#include "hexagon/services/geometry/stair_geometry.h"
#include "hexagon/services/manage_project.h"

namespace {

namespace model = bcad::hexagon::model;
namespace fs = std::filesystem;

// Nicht-persistierendes Port-Double: hält fest, ob/wo `save` mit welchem
// abgeleiteten Bündel aufgerufen wurde.
class FakeRepository final
    : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
public:
    mutable bool save_called = false;
    mutable model::PersistedDerivations last_derived;

    void save(const model::Building& /*building*/,
              const model::PersistedDerivations& derived,
              const fs::path& /*path*/) const override {
        save_called = true;
        last_derived = derived;
    }
    model::Building load(const fs::path& /*path*/) const override { return {}; }
};

model::Stair straightStair(model::StoreyId from) {
    model::Stair stair;
    stair.id = model::StairId{1};
    stair.from_storey_id = from;
    stair.to_storey_id = model::StoreyId{2};
    stair.type = model::StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 10;
    stair.tread_mm = 280.0;
    return stair;
}

// Happy: die `rise`-Ableitung landet im gespeicherten Bündel, `save` läuft.
TEST(ManageProject, SaveBuildsRisePerStair) {
    model::Building building;
    building.storeys.push_back({model::StoreyId{1}, 2500.0});
    const model::Stair stair = straightStair(model::StoreyId{1});
    building.stairs.push_back(stair);

    const FakeRepository repo;
    bcad::hexagon::services::saveProject(repo, building, "irrelevant.bcad");

    EXPECT_TRUE(repo.save_called);
    ASSERT_EQ(repo.last_derived.stairRiseMm.count(model::StairId{1}), 1U);
    EXPECT_DOUBLE_EQ(repo.last_derived.stairRiseMm.at(model::StairId{1}),
                     bcad::hexagon::services::stairRiseMm(stair, 2500.0));
}

// Fail-closed: danglendes `from_storey` → Wurf VOR `save` (kein Teil-Speichern).
TEST(ManageProject, DanglingFromStoreyThrowsBeforeSave) {
    model::Building building;  // KEIN Geschoss 99
    building.stairs.push_back(straightStair(model::StoreyId{99}));

    const FakeRepository repo;
    EXPECT_THROW(
        bcad::hexagon::services::saveProject(repo, building, "irrelevant.bcad"),
        std::runtime_error);
    EXPECT_FALSE(repo.save_called)
        << "fail-closed: save darf bei danglendem from_storey nicht laufen";
}

}  // namespace
