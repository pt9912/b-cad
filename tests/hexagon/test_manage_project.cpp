// Save-/Open-Use-Case (slice-047a/b): `saveProject` baut die kern-abgeleiteten
// Persistenz-Skalare; `openProject`/`replaceBuilding` ersetzen das Modell SAMT
// abgeleiteter Zustaende (Solids, Raeume, Id-Zaehler) und melden EINEN
// Full-Refresh. Kern-Test, OCC-frei ueber das GeometryKernelPort-Double.
// Fake-Repo (dependency-frei) für die Save-Hälfte: prüft die rise-Ableitung und
// die fail-closed-Semantik (danglendes `from_storey` → Wurf VOR `save`).

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
#include "hexagon/model/area_report.h"
#include "hexagon/model/material.h"
#include "hexagon/model/opening.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/slab.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"
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

// Ergaenzt das geladene Projekt um die vier Element-Arten, deren Zaehler-Reset
// bis zum Verify-Lauf ungedeckt war (Finding B2, Gegenprobe CP-1: die vier
// Resets entfernt -> 280/280 blieben gruen). Wieder mit PAARWEISE VERSCHIEDENEN
// Maxima, damit ein vertauschter `maxIdValue`-Aufruf auffaellt.
void addHighIdComponents(model::Building& b) {
    // Zweites Geschoss: eine Treppe verlangt eine gueltige Zwei-Geschoss-Spanne
    // (`from != to`, beide bekannt).
    model::Storey og;
    og.id = model::StoreyId{8};
    og.height_mm = 2500.0;
    b.storeys.push_back(og);

    model::Opening o;  // Tuer in der geladenen Wand 42
    o.id = model::OpeningId{31};
    o.wall_id = model::WallId{42};
    o.kind = model::OpeningKind::Door;
    o.offset_mm = 500.0;
    o.width_mm = 900.0;
    o.height_mm = 2000.0;
    b.openings.push_back(o);

    model::Roof r;
    r.id = model::RoofId{23};
    r.storey_id = model::StoreyId{7};
    r.origin = model::Point2D{0.0, 0.0};
    r.width_mm = 4000.0;
    r.depth_mm = 3000.0;
    r.pitch_deg = 30.0;
    r.thickness_mm = 200.0;
    b.roofs.push_back(r);

    model::Slab sl;
    sl.id = model::SlabId{64};
    sl.storey_id = model::StoreyId{7};
    sl.type = model::SlabType::Decke;
    sl.footprint = model::Footprint{{model::Point2D{0.0, 0.0},
                                     model::Point2D{4000.0, 0.0},
                                     model::Point2D{4000.0, 3000.0},
                                     model::Point2D{0.0, 3000.0}}};
    sl.thickness_mm = 200.0;
    b.slabs.push_back(sl);

    model::Stair st;
    st.id = model::StairId{88};
    st.from_storey_id = model::StoreyId{7};
    st.to_storey_id = model::StoreyId{8};
    st.start = model::Point2D{1000.0, 1000.0};
    st.width_mm = 1000.0;
    st.step_count = 10;
    st.tread_mm = 280.0;
    b.stairs.push_back(st);
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

// --- Verify-Finding B4: die Neu-Aufloesung des Zeichen-Ziels ---------------
//
// Sie lag als Lambda im coverage-ausgenommenen `main` und wurde von KEINEM
// Sensor ausgefuehrt (Gegenprobe CP-5: Aufruf entfernt -> 280/280 gruen). Jetzt
// gehoert sie zum Use-Case und wird hier direkt geprueft: die Senken bekommen
// die Ids des GELADENEN Stands, nicht die eingefrorenen des alten Modells.

// Repository-Double, das ein frei waehlbares Projekt liefert.
class FixtureRepository final
    : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
public:
    explicit FixtureRepository(model::Building fixture)
        : fixture_(std::move(fixture)) {}
    void save(const model::Building&, const model::PersistedDerivations&,
              const fs::path&) const override {}
    model::Building load(const fs::path&) const override { return fixture_; }

private:
    model::Building fixture_;
};

TEST(ManageProject_047b, OpenProjectRetargetsDrawingToLoadedIds) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const FixtureRepository repo{loadedProject()};  // Storey 7, Layer 9

    // Ausgangsstand der Sichten: die eingefrorenen Ids des Demo-Modells.
    auto seen_storey = model::StoreyId{1};
    auto target_storey = model::StoreyId{1};
    auto target_layer = model::LayerId{1};
    const services::DrawingTargetSinks sinks{
        [&seen_storey](model::StoreyId s) { seen_storey = s; },
        [&target_storey, &target_layer](model::StoreyId s, model::LayerId l) {
            target_storey = s;
            target_layer = l;
        },
    };

    const auto resolution = services::openProject(svc, repo, "egal.bcad", sinks);

    EXPECT_EQ(resolution, services::DrawingTargetResolution::Resolved);
    EXPECT_EQ(seen_storey, model::StoreyId{7}) << "Canvas-Geschoss";
    EXPECT_EQ(target_storey, model::StoreyId{7}) << "Hilfslinien-Geschoss";
    EXPECT_EQ(target_layer, model::LayerId{9}) << "Hilfslinien-Ebene";
}

// Teil-Aufloesung: Projekt ohne Zeichen-Ebene. Das Geschoss wird gesetzt (der
// Grundriss zeichnet), das Hilfslinien-Ziel NICHT — und der Aufrufer erfaehrt
// es, statt dass still eine Ebene angelegt wird (Code-Review MEDIUM-8: Oeffnen
// ist lesend).
TEST(ManageProject_047b, OpenProjectReportsMissingLayerWithoutCreatingOne) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    model::Building without_layer = loadedProject();
    without_layer.layers.clear();
    const FixtureRepository repo{without_layer};

    bool storey_set = false;
    bool target_set = false;
    const services::DrawingTargetSinks sinks{
        [&storey_set](model::StoreyId) { storey_set = true; },
        [&target_set](model::StoreyId, model::LayerId) { target_set = true; },
    };

    const auto resolution = services::openProject(svc, repo, "egal.bcad", sinks);

    EXPECT_EQ(resolution, services::DrawingTargetResolution::NoLayer);
    EXPECT_TRUE(storey_set) << "das Geschoss ist aufloesbar und wird gesetzt";
    EXPECT_FALSE(target_set) << "ohne Ebene bleibt das Hilfslinien-Ziel offen";
    EXPECT_TRUE(svc.building().layers.empty())
        << "Oeffnen darf keine Ebene anlegen";
}

// Leeres Projekt: nichts aufloesbar, keine Senke gerufen, kein Wurf.
TEST(ManageProject_047b, OpenProjectReportsMissingStorey) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const FixtureRepository repo{model::Building{}};

    bool any_sink = false;
    const services::DrawingTargetSinks sinks{
        [&any_sink](model::StoreyId) { any_sink = true; },
        [&any_sink](model::StoreyId, model::LayerId) { any_sink = true; },
    };

    EXPECT_EQ(services::openProject(svc, repo, "egal.bcad", sinks),
              services::DrawingTargetResolution::NoStorey);
    EXPECT_FALSE(any_sink);
}

// Ohne Senken (CLI/Tests ohne Sichten) laeuft derselbe Pfad wurf-frei durch.
TEST(ManageProject_047b, OpenProjectWithoutSinksIsSafe) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const FixtureRepository repo{loadedProject()};

    EXPECT_EQ(services::openProject(svc, repo, "egal.bcad"),
              services::DrawingTargetResolution::Resolved);
    EXPECT_EQ(svc.building().storeys.front().id, model::StoreyId{7});
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


// --- Code-Review 2026-07-25: die drei belegten Test-Luecken (MEDIUM-1/2/3) ---

// Geometrie-Double, das beim N-ten Extrusions-Aufruf wirft (E-GEO-002-Surrogat).
class ThrowingOnNthExtrude final
    : public bcad::hexagon::ports::driven::GeometryKernelPort {
public:
    explicit ThrowingOnNthExtrude(int throw_on_call) : throw_on_(throw_on_call) {}

    model::Solid extrudeFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const override {
        if (++calls_ == throw_on_) {
            throw std::runtime_error("E-GEO-002: Extrusion fehlgeschlagen");
        }
        return inner_.extrudeFootprint(footprint, height_mm, cutouts);
    }
    model::TriangleMesh tessellateFootprint(
        const model::Footprint& footprint, double height_mm,
        const std::vector<model::CutPrism>& cutouts) const override {
        return inner_.tessellateFootprint(footprint, height_mm, cutouts);
    }

private:
    AnalyticGeometry inner_{};
    int throw_on_;
    mutable int calls_ = 0;
};

// Geschlossenes Rechteck aus vier Waenden — traegt einen erkennbaren Raum.
model::Building roomedProject() {
    model::Building b;
    model::Storey s;
    s.id = model::StoreyId{3};
    s.height_mm = 2500.0;
    b.storeys.push_back(s);

    const double x0 = 0.0, y0 = 0.0, x1 = 4000.0, y1 = 3000.0;
    const model::Point2D corners[4] = {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
    for (int i = 0; i < 4; ++i) {
        model::Wall w;
        w.id = static_cast<model::WallId>(10 + i);
        w.storey_id = s.id;
        w.start = corners[i];
        w.end = corners[(i + 1) % 4];
        w.thickness_mm = 240.0;
        w.height_mm = 2500.0;
        b.walls.push_back(w);
    }
    return b;
}

// MEDIUM-1: die redetectRooms-Schleife wird wirklich durchlaufen — nach dem
// Laden eines geschlossenen Grundrisses meldet floorArea eine Flaeche. Ohne
// die Schleife bliebe rooms_ leer und die Flaeche 0.
TEST(ManageProject_047b, ReplaceBuildingRedetectsRoomsOfLoadedProject) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    svc.replaceBuilding(roomedProject());

    const model::AreaReport report = svc.floorArea(model::StoreyId{3});
    EXPECT_FALSE(report.room_areas_m2.empty())
        << "geladener geschlossener Grundriss muss einen Raum ergeben";
    EXPECT_GT(report.total_m2, 0.0);
}

// MEDIUM-2: der Zaehler-Reset gilt fuer JEDEN Zaehler, nicht nur Wand/Ebene.
// Jede Element-Art bekommt ein ANDERES geladenes Maximum — ein vertauschter
// maxIdValue-Aufruf (Copy-Paste) faellt damit auf.
TEST(ManageProject_047b, ReplaceBuildingResetsEveryIdCounter) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    model::Building b = loadedProject();  // Storey 7, Wall 42, Layer 9
    model::Material m;
    m.id = model::MaterialId{55};
    m.name = "Beton";
    b.materials.push_back(m);

    model::GuideLine g;
    g.id = model::GuideLineId{77};
    g.storey_id = model::StoreyId{7};
    g.layer_id = model::LayerId{9};
    g.segment = model::Segment{{0.0, 0.0}, {100.0, 0.0}};
    b.guide_lines.push_back(g);

    svc.replaceBuilding(std::move(b));

    EXPECT_GT(static_cast<int>(svc.addStorey(2500.0)), 7)
        << "next_storey_id_";
    const auto w = svc.addWall(model::StoreyId{7},
                               model::Segment{{0.0, 500.0}, {1000.0, 500.0}});
    ASSERT_TRUE(w.has_value());
    EXPECT_GT(static_cast<int>(*w), 42) << "next_wall_id_";

    model::Layer l;
    l.name = "Frisch";
    const auto lid = svc.addLayer(l);
    ASSERT_TRUE(lid.has_value());
    EXPECT_GT(static_cast<int>(*lid), 9) << "next_layer_id_";

    model::Material fresh_m;
    fresh_m.name = "Holz";
    const auto mid = svc.addMaterial(fresh_m);
    ASSERT_TRUE(mid.has_value());
    EXPECT_GT(static_cast<int>(*mid), 55) << "next_material_id_";

    model::GuideLine fresh_g;
    fresh_g.storey_id = model::StoreyId{7};
    fresh_g.layer_id = *lid;
    fresh_g.segment = model::Segment{{0.0, 900.0}, {900.0, 900.0}};
    const auto gid = svc.addGuideLine(fresh_g);
    ASSERT_TRUE(gid.has_value());
    EXPECT_GT(static_cast<int>(*gid), 77) << "next_guide_line_id_";
}

// Verify-Finding B2: dieselbe Zusage fuer die vier RESTLICHEN Zaehler. Der
// Code-Review hatte die Luecke als MEDIUM-2 gemeldet ("7 der 9 ungetestet"),
// die Einarbeitung schloss fuenf — die Gegenprobe des Verifiers (CP-1: Resets
// fuer opening/roof/slab/stair entfernt) blieb 280/280 gruen. Ein Verlust
// dieser vier Resets bedeutet eine Id-KOLLISION bei der ersten Oeffnung/dem
// ersten Dach/der ersten Platte/der ersten Treppe nach dem Laden.
TEST(ManageProject_047b, ReplaceBuildingResetsComponentIdCounters) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    model::Building b = loadedProject();  // Storey 7, Wall 42, Layer 9
    addHighIdComponents(b);               // Opening 31, Roof 23, Slab 64, Stair 88

    svc.replaceBuilding(std::move(b));

    // Zweite Tuer in derselben geladenen Wand — weit genug von der geladenen
    // (offset 500 + Breite 900) entfernt, damit sie nicht ueberlappt.
    const auto oid = svc.addDoor(model::WallId{42}, 2000.0);
    ASSERT_TRUE(oid.has_value());
    EXPECT_GT(static_cast<int>(*oid), 31) << "next_opening_id_";

    model::Roof fresh_r;
    fresh_r.storey_id = model::StoreyId{7};
    fresh_r.origin = model::Point2D{0.0, 0.0};
    fresh_r.width_mm = 3000.0;
    fresh_r.depth_mm = 2000.0;
    fresh_r.pitch_deg = 25.0;
    fresh_r.thickness_mm = 180.0;
    const auto rid = svc.addRoof(fresh_r);
    ASSERT_TRUE(rid.has_value());
    EXPECT_GT(static_cast<int>(*rid), 23) << "next_roof_id_";

    model::Slab fresh_sl;
    fresh_sl.storey_id = model::StoreyId{7};
    fresh_sl.type = model::SlabType::Fundament;
    fresh_sl.footprint = model::Footprint{{model::Point2D{0.0, 0.0},
                                           model::Point2D{2000.0, 0.0},
                                           model::Point2D{2000.0, 2000.0},
                                           model::Point2D{0.0, 2000.0}}};
    fresh_sl.thickness_mm = 300.0;
    const auto slid = svc.addSlab(fresh_sl);
    ASSERT_TRUE(slid.has_value());
    EXPECT_GT(static_cast<int>(*slid), 64) << "next_slab_id_";

    model::Stair fresh_st;
    fresh_st.from_storey_id = model::StoreyId{7};
    fresh_st.to_storey_id = model::StoreyId{8};
    fresh_st.start = model::Point2D{2000.0, 2000.0};
    fresh_st.width_mm = 1000.0;
    fresh_st.step_count = 12;
    fresh_st.tread_mm = 280.0;
    const auto stid = svc.addStair(fresh_st);
    ASSERT_TRUE(stid.has_value());
    EXPECT_GT(static_cast<int>(*stid), 88) << "next_stair_id_";
}

// MEDIUM-3: die zugesagte Transaktionalitaet. Wirft der Solid-Bau einer
// GELADENEN Wand, bleibt der bisherige Stand vollstaendig unberuehrt — kein
// halb ersetztes Modell, keine Meldung.
TEST(ManageProject_047b, ReplaceBuildingIsTransactionalOnGeometryFailure) {
    const ThrowingOnNthExtrude geometry(3);  // erste zwei Bauten ok, dritter wirft
    services::StructureEditService svc(geometry);
    const auto own_storey = svc.building().storeys.front().id;
    const auto own_wall = svc.addWall(
        own_storey, model::Segment{{0.0, 0.0}, {2000.0, 0.0}});
    ASSERT_TRUE(own_wall.has_value());

    RecordingListener listener;
    svc.subscribe(listener);
    const auto storeys_before = svc.building().storeys.size();
    const auto walls_before = svc.building().walls.size();

    // roomedProject hat vier Waende -> der dritte Extrusions-Aufruf wirft.
    EXPECT_THROW(svc.replaceBuilding(roomedProject()), std::runtime_error);

    EXPECT_EQ(svc.building().storeys.size(), storeys_before)
        << "gescheiterter Solid-Bau darf das Modell nicht anfassen";
    EXPECT_EQ(svc.building().walls.size(), walls_before);
    EXPECT_EQ(svc.building().walls.front().id, *own_wall);
    EXPECT_EQ(listener.replaced, 0) << "keine Meldung bei gescheitertem Tausch";
    svc.unsubscribe(listener);
}


// MEDIUM-10: Ganzdatei-Ablehnung. Wirft der Solid-Bau EINER geladenen Wand,
// scheitert das Oeffnen der GESAMTEN Datei — der Fehler kommt neutral durch
// und der bisherige Stand bleibt. Bewusst anders als der Query-Zweig
// (`wallMesh` faengt E-GEO-002 und liefert nullopt): eine Darstellungs-Abfrage
// darf total sein, ein Modell-Tausch nicht halb gelingen.
TEST(ManageProject_047b, OpenProjectRejectsWholeFileOnGeometryFailure) {
    class LoadingRoomedRepository final
        : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
    public:
        void save(const model::Building&, const model::PersistedDerivations&,
                  const fs::path&) const override {}
        model::Building load(const fs::path&) const override {
            return roomedProject();
        }
    };

    const ThrowingOnNthExtrude geometry(3);
    services::StructureEditService svc(geometry);
    const auto own_storey = svc.building().storeys.front().id;
    ASSERT_TRUE(svc.addWall(own_storey,
                            model::Segment{{0.0, 0.0}, {2000.0, 0.0}}).has_value());
    const auto walls_before = svc.building().walls.size();

    const LoadingRoomedRepository repo;
    EXPECT_THROW(services::openProject(svc, repo, "geometrie-kaputt.bcad"),
                 std::runtime_error);

    EXPECT_EQ(svc.building().walls.size(), walls_before)
        << "Ganzdatei-Ablehnung: kein Teil-Zustand";
}

}  // namespace
