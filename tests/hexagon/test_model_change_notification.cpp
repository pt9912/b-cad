// Akzeptanz-Tests für die Echtzeit-Änderungs-Benachrichtigung
// (slice-010b), OCC-frei über Port-Doubles (ADR-0001 §Testbarkeit).
// Geprüft wird LH-FA-D3-002 gegen das in slice-010a geschärfte
// Lastenheft (0.1.1) und ADR-0008 (Observer-Port, Push-Notify/
// Pull-State, Meldung nach Re-Detektion, Kapselung):
// - Happy: Parameteränderung -> Meldung ohne expliziten Abruf; der per
//   Pull geholte Stand ist aktualisiert.
// - Ordnung: Meldung kommt NACH der Raum-Re-Detektion — ein Pull im
//   Callback sieht den konsistenten Raum-Stand.
// - Boundary: geklemmte Änderung meldet; gepullter Stand ist der
//   geklemmte (tatsächlich übernommene) Wert.
// - Negative: abgelehnte (E-VAL-001 Rejected) und verworfene
//   (Null-Länge) Mutationen melden nicht.
// - Kapselung: ein werfender Beobachter kippt die committete Mutation
//   nicht und blockiert Folge-Beobachter nicht (ADR-0008 #6).
// Test-Namen tragen die LH-ID.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "hexagon/model/constants.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/solid.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;

// Deterministisches Geometrie-Double ohne OpenCascade (Muster aus
// test_structure_edit_service.cpp): Volumen = Länge · Stärke · Höhe.
class AnalyticGeometry final : public driven::GeometryKernelPort {
public:
    model::Solid extrudeWall(const model::Wall& w) const override {
        const double dx = w.end.x_mm - w.start.x_mm;
        const double dy = w.end.y_mm - w.start.y_mm;
        const double length = std::sqrt((dx * dx) + (dy * dy));
        return model::Solid{length * w.thickness_mm * w.height_mm};
    }
};

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

constexpr model::StoreyId kGroundStorey{1};

}  // namespace

TEST(ModelChange_LH_FA_D3_002, ParameteraenderungMeldetOhneExplizitenAbruf) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall =
        *svc.addWall(kGroundStorey, {{0.0, 0.0}, {1000.0, 0.0}});

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
    const model::WallId wall =
        *svc.addWall(kGroundStorey, {{0.0, 0.0}, {1000.0, 0.0}});

    RecordingListener listener;
    svc.subscribe(listener);
    svc.setWallThickness(wall, 1001.0);  // Boundary: wird auf 1000 geklemmt

    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::WallThicknessChanged), 1U);
    EXPECT_DOUBLE_EQ(svc.wall(wall).thickness_mm, model::kWallThicknessMaxMm);
}

TEST(ModelChange_LH_FA_D3_002, AbgelehnteUndVerworfeneMutationMeldenNicht) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const model::WallId wall =
        *svc.addWall(kGroundStorey, {{0.0, 0.0}, {1000.0, 0.0}});

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
    const model::WallId wall =
        *svc.addWall(kGroundStorey, {{0.0, 0.0}, {1000.0, 0.0}});

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
