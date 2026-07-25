// Save-/Open-Use-Case (slice-047a/b): `saveProject` baut die kern-abgeleiteten
// Persistenz-Skalare; `openProject`/`replaceBuilding` ersetzen das Modell SAMT
// abgeleiteter Zustaende (Solids, Raeume, Id-Zaehler) und melden EINEN
// Full-Refresh. Kern-Test, OCC-frei ueber das GeometryKernelPort-Double.
//
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

#include "analytic_geometry_double.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/services/structure_edit_service.h"

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


// --- 047b: Modell-Ersetzung (replaceBuilding / openProject) ---------------

namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;
using bcad::testing::AnalyticGeometry;

// Zaehlt die Meldungen je `op` — belegt den EINEN Full-Refresh.
class RecordingListener final : public driven::ModelChangedPort {
public:
    int replaced = 0;
    int total = 0;
    void onModelChanged(const driven::ModelChange& change) override {
        ++total;
        if (change.op == driven::ModelChangeOp::ModelReplaced) {
            ++replaced;
        }
    }
};

// Ein geladenes Projekt mit hohen, bereits vergebenen Ids.
model::Building loadedProject() {
    model::Building b;
    model::Storey s;
    s.id = model::StoreyId{7};
    s.height_mm = 2500.0;
    b.storeys.push_back(s);

    model::Wall w;
    w.id = model::WallId{42};
    w.storey_id = s.id;
    w.start = model::Point2D{0.0, 0.0};
    w.end = model::Point2D{4000.0, 0.0};
    w.thickness_mm = 240.0;
    w.height_mm = 2500.0;
    b.walls.push_back(w);

    model::Layer l;
    l.id = model::LayerId{9};
    l.name = "Achsen";
    b.layers.push_back(l);
    return b;
}

// HIGH-2: nach dem Ersetzen sind die abgeleiteten Zustaende neu gebaut —
// das Solid der geladenen Wand existiert, und die naechste Mutation mintet
// eine FRISCHE Id (keine Kollision mit einer persistierten).
TEST(ManageProject_047b, ReplaceBuildingRebuildsDerivedStateAndResetsCounters) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    RecordingListener listener;
    svc.subscribe(listener);

    svc.replaceBuilding(loadedProject());

    // Modell uebernommen.
    ASSERT_EQ(svc.building().walls.size(), 1U);
    EXPECT_EQ(svc.building().walls.front().id, model::WallId{42});
    // Abgeleitet: das Solid der geladenen Wand ist gebaut (wirft sonst).
    EXPECT_NO_THROW((void)svc.wallSolid(model::WallId{42}));

    // Zaehler-Reset: die erste Mutation kollidiert NICHT mit Id 42.
    const auto fresh_wall = svc.addWall(
        model::StoreyId{7}, model::Segment{{0.0, 0.0}, {0.0, 3000.0}});
    ASSERT_TRUE(fresh_wall.has_value());
    EXPECT_GT(static_cast<int>(*fresh_wall), 42)
        << "next_wall_id_ muss ueber das geladene Maximum zurueckgesetzt sein";

    model::Layer proto;
    proto.name = "Neu";
    const auto fresh_layer = svc.addLayer(proto);
    ASSERT_TRUE(fresh_layer.has_value());
    EXPECT_GT(static_cast<int>(*fresh_layer), 9)
        << "next_layer_id_ muss ueber das geladene Maximum zurueckgesetzt sein";

    svc.unsubscribe(listener);
}

// HIGH-1: genau EINE ModelReplaced-Meldung — der Full-Refresh, nicht je
// Element eine Meldung.
TEST(ManageProject_047b, ReplaceBuildingNotifiesExactlyOneFullRefresh) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    RecordingListener listener;
    svc.subscribe(listener);

    svc.replaceBuilding(loadedProject());

    EXPECT_EQ(listener.replaced, 1);
    EXPECT_EQ(listener.total, 1)
        << "keine Element-Meldungen fuer die geladenen Bestandteile";
    svc.unsubscribe(listener);
}

// MED-2: die openProject-Naht laeuft ueber den Port und ersetzt das Modell.
TEST(ManageProject_047b, OpenProjectReplacesModelViaPort) {
    class LoadingRepository final
        : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
    public:
        void save(const model::Building&, const model::PersistedDerivations&,
                  const fs::path&) const override {}
        model::Building load(const fs::path&) const override {
            return loadedProject();
        }
    };

    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const LoadingRepository repo;

    services::openProject(svc, repo, "egal.bcad");

    ASSERT_EQ(svc.building().storeys.size(), 1U);
    EXPECT_EQ(svc.building().storeys.front().id, model::StoreyId{7});
    EXPECT_EQ(svc.building().walls.size(), 1U);
}

// Fehlerfall: wirft das Repository, bleibt der bisherige Stand unveraendert.
TEST(ManageProject_047b, OpenProjectKeepsModelWhenLoadThrows) {
    class ThrowingRepository final
        : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
    public:
        void save(const model::Building&, const model::PersistedDerivations&,
                  const fs::path&) const override {}
        model::Building load(const fs::path&) const override {
            throw std::runtime_error("E-IO-001: Datei fehlt");
        }
    };

    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto storey = svc.addStorey(2500.0);
    const auto wall = svc.addWall(
        storey, model::Segment{{0.0, 0.0}, {3000.0, 0.0}});
    ASSERT_TRUE(wall.has_value());

    // Stand VOR dem Fehlversuch festhalten (der Service legt bei Anlage
    // bereits ein Default-Geschoss an — die Invariante ist "unveraendert",
    // nicht eine feste Anzahl).
    const auto storeys_before = svc.building().storeys.size();
    const auto walls_before = svc.building().walls.size();

    const ThrowingRepository repo;
    EXPECT_THROW(services::openProject(svc, repo, "fehlt.bcad"),
                 std::runtime_error);

    EXPECT_EQ(svc.building().storeys.size(), storeys_before)
        << "gescheitertes Laden darf den Stand nicht anfassen";
    EXPECT_EQ(svc.building().walls.size(), walls_before);
    EXPECT_EQ(svc.building().walls.front().id, *wall)
        << "die bestehende Wand muss dieselbe bleiben";
}

}  // namespace
