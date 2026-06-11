// Akzeptanz-Tests für die Echtzeit-Änderungs-Benachrichtigung
// (slice-010b; erweitert um die Code-Review-Findings L2/L3 der
// Welle-1-Prüfung), OCC-frei über Port-Doubles (ADR-0001
// §Testbarkeit). Geprüft wird LH-FA-D3-002 gegen das geschärfte
// Lastenheft (0.1.1) und ADR-0008 (Observer-Port, Push-Notify/
// Pull-State, Meldung nach Re-Detektion, Kapselung):
// - Happy: Parameteränderung -> Meldung ohne expliziten Abruf; der per
//   Pull geholte Stand ist aktualisiert (Stärke UND Höhe, je eigener op).
// - Ordnung: Meldung kommt NACH der Raum-Re-Detektion.
// - Boundary: geklemmte Änderung meldet; gepullter Stand = Grenzwert.
// - Negative: abgelehnte/verworfene Mutationen melden nicht.
// - Kapselung: werfender Beobachter kippt nichts, blockiert nicht.
// - Registrierung: subscribe idempotent; unsubscribe (auch im
//   Callback) beendet die Zustellung.
// Test-Namen tragen die LH-ID.

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "analytic_geometry_double.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;
using bcad::testing::AnalyticGeometry;

// Zählender/aufzeichnender Beobachter (Push-Notify-Empfänger).
class RecordingListener final : public driven::ModelChangedPort {
public:
    void onModelChanged(const driven::ModelChange& change) override {
        received.push_back(change);
    }
    std::size_t countOf(driven::ModelChangeOp op) const {
        return static_cast<std::size_t>(
            std::count_if(received.begin(), received.end(),
                          [op](const driven::ModelChange& c) { return c.op == op; }));
    }
    std::vector<driven::ModelChange> received;
};

// Pull-State-Beobachter: fragt im Callback den Raum-Stand ab — prüft
// die ADR-0008-#4-Reihenfolge (Meldung NACH der Re-Detektion).
class RoomPullingListener final : public driven::ModelChangedPort {
public:
    RoomPullingListener(const services::StructureEditService& svc,
                        model::StoreyId storey)
        : svc_(svc), storey_(storey) {}
    void onModelChanged(const driven::ModelChange& change) override {
        if (change.op == driven::ModelChangeOp::RoomsChanged) {
            rooms_seen_at_callback.push_back(svc_.rooms(storey_).size());
        }
    }
    std::vector<std::size_t> rooms_seen_at_callback;

private:
    const services::StructureEditService& svc_;
    model::StoreyId storey_;
};

// Werfender Beobachter — prüft die Kapselung (ADR-0008 #6).
class ThrowingListener final : public driven::ModelChangedPort {
public:
    void onModelChanged(const driven::ModelChange& /*change*/) override {
        throw std::runtime_error("Beobachter wirft");
    }
};

// Meldet sich beim ersten Callback selbst ab — prüft, dass die
// Snapshot-Iteration ein unsubscribe im Callback verkraftet
// (Closure-Behauptung slice-010b, Review L3).
class SelfUnsubscribingListener final : public driven::ModelChangedPort {
public:
    explicit SelfUnsubscribingListener(services::StructureEditService& svc)
        : svc_(svc) {}
    void onModelChanged(const driven::ModelChange& /*change*/) override {
        ++callbacks;
        svc_.unsubscribe(*this);
    }
    int callbacks{0};

private:
    services::StructureEditService& svc_;
};

constexpr model::StoreyId kGroundStorey{1};

model::WallId addWallChecked(services::StructureEditService& svc,
                             model::Segment seg) {
    const auto id = svc.addWall(kGroundStorey, seg);
    EXPECT_TRUE(id.has_value()) << "addWall hat die Wand verworfen";
    return id.has_value() ? *id : model::WallId{};
}

}  // namespace

TEST(ModelChange_LH_FA_D3_002, ParameteraenderungMeldetOhneExplizitenAbruf) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWallChecked(svc, {{0.0, 0.0}, {1000.0, 0.0}});

    RecordingListener listener;
    svc.subscribe(listener);
    svc.setWallThickness(wall, 300.0);

    // Happy: die Meldung kam im Mutationsaufruf an — kein expliziter
    // Abruf-/Refresh-Schritt; der gepullte Stand ist aktualisiert.
    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::WallThicknessChanged), 1U);
    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::RoomsChanged), 1U);
    EXPECT_DOUBLE_EQ(svc.wallSolid(wall).volume_mm3,
                     1000.0 * 300.0 * model::kDefaultStoreyHeightMm);
}

TEST(ModelChange_LH_FA_D3_002, HoehenAenderungMeldetMitEigenemOp) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWallChecked(svc, {{0.0, 0.0}, {1000.0, 0.0}});

    RecordingListener listener;
    svc.subscribe(listener);
    svc.setWallHeight(wall, 3000.0);  // Review L2: eigener Notify-Pfad

    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::WallHeightChanged), 1U);
    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::WallThicknessChanged), 0U);
    EXPECT_DOUBLE_EQ(svc.wall(wall).height_mm, 3000.0);
}

TEST(ModelChange_LH_FA_D3_002, MeldungKommtNachRaumReDetektion) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    RoomPullingListener listener(svc, kGroundStorey);
    svc.subscribe(listener);

    // Rechteck schließen: beim vierten addWall muss der Pull im
    // Callback bereits genau einen Raum sehen (ADR-0008 #4).
    svc.addWall(kGroundStorey, {{0.0, 0.0}, {5000.0, 0.0}});
    svc.addWall(kGroundStorey, {{5000.0, 0.0}, {5000.0, 4000.0}});
    svc.addWall(kGroundStorey, {{5000.0, 4000.0}, {0.0, 4000.0}});
    svc.addWall(kGroundStorey, {{0.0, 4000.0}, {0.0, 0.0}});

    ASSERT_EQ(listener.rooms_seen_at_callback.size(), 4U);
    EXPECT_EQ(listener.rooms_seen_at_callback[2], 0U);  // Zug noch offen
    EXPECT_EQ(listener.rooms_seen_at_callback[3], 1U);  // beim Schließen
}

TEST(ModelChange_LH_FA_D3_002, GeklemmteAenderungMeldetGeklemmtenStand) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWallChecked(svc, {{0.0, 0.0}, {1000.0, 0.0}});

    RecordingListener listener;
    svc.subscribe(listener);
    svc.setWallThickness(wall, 1001.0);  // Boundary: wird auf 1000 geklemmt

    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::WallThicknessChanged), 1U);
    EXPECT_DOUBLE_EQ(svc.wall(wall).thickness_mm, model::kWallThicknessMaxMm);
}

TEST(ModelChange_LH_FA_D3_002, AbgelehnteUndVerworfeneMutationMeldenNicht) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWallChecked(svc, {{0.0, 0.0}, {1000.0, 0.0}});

    RecordingListener listener;
    svc.subscribe(listener);

    // Negative 1: ungültige Eingabe (E-VAL-001 Rejected) — keine Meldung.
    svc.setWallThickness(wall, std::numeric_limits<double>::quiet_NaN());
    // Negative 2: verworfene Null-Längen-Wand (WAL-001 Boundary) — keine
    // Meldung.
    EXPECT_FALSE(svc.addWall(kGroundStorey, {{0.0, 0.0}, {0.0, 0.0}}).has_value());

    EXPECT_TRUE(listener.received.empty());
}

TEST(ModelChange_LH_FA_D3_002, WerfenderBeobachterKipptNichtsUndBlockiertNicht) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall = addWallChecked(svc, {{0.0, 0.0}, {1000.0, 0.0}});

    ThrowingListener thrower;
    RecordingListener recorder;
    svc.subscribe(thrower);   // wirft zuerst …
    svc.subscribe(recorder);  // … der nächste Beobachter wird trotzdem bedient

    svc.setWallThickness(wall, 300.0);

    // Kapselung (ADR-0008 #6): Mutation committed, Folge-Beobachter
    // bedient, Fehler beobachtbar gezählt — nicht still verloren.
    EXPECT_DOUBLE_EQ(svc.wall(wall).thickness_mm, 300.0);
    EXPECT_EQ(recorder.countOf(driven::ModelChangeOp::WallThicknessChanged), 1U);
    EXPECT_GE(svc.swallowedListenerErrors(), 1);
}

TEST(ModelChange_LH_FA_D3_002, DoppelteRegistrierungMeldetGenauEinmal) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    RecordingListener listener;
    svc.subscribe(listener);
    svc.subscribe(listener);  // Review L3: subscribe ist idempotent

    svc.addStorey(2600.0);
    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::StoreyAdded), 1U);
}

TEST(ModelChange_LH_FA_D3_002, UnsubscribeImCallbackIstSicher) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    SelfUnsubscribingListener self_unsubscriber(svc);
    RecordingListener recorder;
    svc.subscribe(self_unsubscriber);
    svc.subscribe(recorder);

    // Erste Mutation: beide werden bedient (Snapshot-Iteration), der
    // erste meldet sich dabei ab — Review L3 / Closure-Behauptung 010b.
    svc.addStorey(2600.0);
    EXPECT_EQ(self_unsubscriber.callbacks, 1);
    EXPECT_EQ(recorder.countOf(driven::ModelChangeOp::StoreyAdded), 1U);

    // Zweite Mutation: nur noch der verbliebene Beobachter.
    svc.addStorey(2600.0);
    EXPECT_EQ(self_unsubscriber.callbacks, 1);
    EXPECT_EQ(recorder.countOf(driven::ModelChangeOp::StoreyAdded), 2U);
}

TEST(ModelChange_LH_FA_D3_002, GeschossAnlageMeldetUndUnsubscribeBeendet) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    RecordingListener listener;
    svc.subscribe(listener);

    svc.addStorey(2600.0);  // ADR-0008 §Umfang: auch Geschoss-Anlage
    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::StoreyAdded), 1U);

    svc.unsubscribe(listener);
    svc.addStorey(2600.0);
    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::StoreyAdded), 1U);
}
