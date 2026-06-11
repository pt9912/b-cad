// Akzeptanz-Tests für die Raum-Autoerkennung (slice-009b), OCC-frei über
// ein GeometryKernelPort-Double (ADR-0001 §Testbarkeit). Geprüft wird
// LH-FA-ROM-001 gegen die in slice-009a geschärfte Spezifikation
// (spez. §1 LH-FA-ROM-001.a) und ADR-0007:
// - Happy: geschlossener Wandzug -> genau ein Raum, automatisch beim
//   Schließen (kein expliziter Abruf), Innenkanten-Polygon/Netto-Fläche
//   analytisch korrekt.
// - Mutation: Stärke-Änderung löst Re-Detektion aus (spez. §1 §Auslösung).
// - Boundary: verschachtelte Wandzüge -> innerer und äußerer Raum
//   getrennt, äußerer mit Loch-Ring, keine Flächen-Doppelzählung.
// - Negative: offener Wandzug -> kein Raum, kein Fehler.
// - Degeneriert: kollabierender Innenkanten-Offset -> kein Raum, kein
//   Fehler, Modell unverändert (Erkennung ist total, kein E-GEO-002).
// Test-Namen tragen die LH-ID. Erwartungswerte: Innenkanten-Basis
// (ADR-0007) — Rechteck-Mittellinie minus halbe Wandstärke je Seite.

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <optional>
#include <vector>

#include "hexagon/model/constants.h"
#include "hexagon/model/room.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/solid.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;

// Deterministisches Geometrie-Double ohne OpenCascade (wie in
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

constexpr model::StoreyId kGroundStorey{1};
constexpr double kAreaToleranceMm2 = 1e-6;

// Achsenparalleles Rechteck (Mittellinien-Koordinaten).
struct Rect {
    double x0;
    double y0;
    double x1;
    double y1;
};

// Fügt die vier Wände eines Rechtecks hinzu (geschlossener Wandzug) und
// setzt jede auf `thickness_mm`. Liefert die Wand-Ids in Zeichen-
// Reihenfolge (unten, rechts, oben, links).
std::vector<model::WallId> addRect(services::StructureEditService& svc, Rect r,
                                   double thickness_mm) {
    const std::array<model::Segment, 4> segments{{
        {{r.x0, r.y0}, {r.x1, r.y0}},   // unten
        {{r.x1, r.y0}, {r.x1, r.y1}},   // rechts
        {{r.x1, r.y1}, {r.x0, r.y1}},   // oben
        {{r.x0, r.y1}, {r.x0, r.y0}},   // links
    }};
    std::vector<model::WallId> ids;
    for (const model::Segment& seg : segments) {
        const std::optional<model::WallId> id = svc.addWall(kGroundStorey, seg);
        if (!id.has_value()) {
            ADD_FAILURE() << "addRect: Wand wurde verworfen";
            continue;
        }
        svc.setWallThickness(*id, thickness_mm);
        ids.push_back(*id);
    }
    return ids;
}

}  // namespace

TEST(RoomDetection_LH_FA_ROM_001, GeschlossenerWandzugErzeugtGenauEinenRaumAutomatisch) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // Drei Wände: Zug noch offen -> kein Raum.
    svc.setWallThickness(*svc.addWall(kGroundStorey, {{0.0, 0.0}, {5000.0, 0.0}}), 200.0);
    svc.setWallThickness(*svc.addWall(kGroundStorey, {{5000.0, 0.0}, {5000.0, 4000.0}}), 200.0);
    svc.setWallThickness(*svc.addWall(kGroundStorey, {{5000.0, 4000.0}, {0.0, 4000.0}}), 200.0);
    EXPECT_TRUE(svc.rooms(kGroundStorey).empty());

    // Vierte Wand schließt den Zug -> der Raum entsteht automatisch beim
    // Schließen (LH-FA-ROM-001 Happy); rooms() ist reine Abfrage.
    svc.setWallThickness(*svc.addWall(kGroundStorey, {{0.0, 4000.0}, {0.0, 0.0}}), 200.0);
    const std::vector<model::Room> rooms = svc.rooms(kGroundStorey);
    ASSERT_EQ(rooms.size(), 1U);

    // Innenkanten-Basis (ADR-0007): 5000x4000 Mittellinie, t=200 ->
    // lichte Fläche (5000-200) x (4000-200).
    EXPECT_NEAR(rooms.front().net_area_mm2, 4800.0 * 3800.0, kAreaToleranceMm2);
    EXPECT_EQ(rooms.front().outer.size(), 4U);
    EXPECT_TRUE(rooms.front().holes.empty());
    EXPECT_EQ(rooms.front().storey_id, kGroundStorey);
}

TEST(RoomDetection_LH_FA_ROM_001, WandMutationAktualisiertRaumAutomatisch) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const std::vector<model::WallId> ids =
        addRect(svc, Rect{0.0, 0.0, 5000.0, 4000.0}, 200.0);
    ASSERT_EQ(svc.rooms(kGroundStorey).size(), 1U);

    // Stärke der unteren Wand 200 -> 400: deren Innenkante rückt um
    // weitere 100 mm ein -> lichte Höhe 4000 - 200 - 100 = 3700
    // (Re-Detektion bei Modell-Mutation, spez. §1 §Auslösung).
    svc.setWallThickness(ids.front(), 400.0);
    const std::vector<model::Room> rooms = svc.rooms(kGroundStorey);
    ASSERT_EQ(rooms.size(), 1U);
    EXPECT_NEAR(rooms.front().net_area_mm2, 4800.0 * 3700.0, kAreaToleranceMm2);
}

TEST(RoomDetection_LH_FA_ROM_001, VerschachtelteWandzuegeOhneFlaechenDoppelzaehlung) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // Äußerer Zug 10000x8000 (t=200), innerer Zug 3000x2000 (t=100),
    // vollständig enthalten und nicht verbunden (LH-FA-ROM-001 Boundary).
    addRect(svc, Rect{0.0, 0.0, 10000.0, 8000.0}, 200.0);
    addRect(svc, Rect{4000.0, 3000.0, 7000.0, 5000.0}, 100.0);

    const std::vector<model::Room> rooms = svc.rooms(kGroundStorey);
    ASSERT_EQ(rooms.size(), 2U);

    const model::Room& outer = rooms.front().holes.empty() ? rooms.back() : rooms.front();
    const model::Room& inner = rooms.front().holes.empty() ? rooms.front() : rooms.back();

    // Innerer Raum: Innenkante des inneren Zugs -> (3000-100) x (2000-100).
    EXPECT_TRUE(inner.holes.empty());
    EXPECT_NEAR(inner.net_area_mm2, 2900.0 * 1900.0, kAreaToleranceMm2);

    // Äußerer Raum: eigener Innenkanten-Ring (10000-200) x (8000-200)
    // minus Loch-Ring = AUSSENKONTUR des inneren Zugs (3000+100) x
    // (2000+100) — Netto-Fläche ohne Doppelzählung (ADR-0007 Ring-Modell).
    ASSERT_EQ(outer.holes.size(), 1U);
    EXPECT_NEAR(outer.net_area_mm2,
                (9800.0 * 7800.0) - (3100.0 * 2100.0), kAreaToleranceMm2);
}

TEST(RoomDetection_LH_FA_ROM_001, OffenerWandzugErzeugtKeinenRaumKeinFehler) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // Drei Seiten eines Rechtecks — Zug bleibt offen (LH-FA-ROM-001
    // Negative): kein Raum, kein Fehler.
    svc.setWallThickness(*svc.addWall(kGroundStorey, {{0.0, 0.0}, {5000.0, 0.0}}), 200.0);
    svc.setWallThickness(*svc.addWall(kGroundStorey, {{5000.0, 0.0}, {5000.0, 4000.0}}), 200.0);
    svc.setWallThickness(*svc.addWall(kGroundStorey, {{5000.0, 4000.0}, {0.0, 4000.0}}), 200.0);

    EXPECT_TRUE(svc.rooms(kGroundStorey).empty());
    // Unbekanntes Geschoss: leere Antwort, kein Fehler.
    EXPECT_TRUE(svc.rooms(static_cast<model::StoreyId>(99)).empty());
}

TEST(RoomDetection_LH_FA_ROM_001, DegenerierterZyklusErzeugtKeinenRaumKeinFehler) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // 150x150-Mittellinien-Zyklus mit 200er-Wänden: der Innenkanten-
    // Offset (je 100 mm) kollabiert — die Erkennung ist total
    // (spez. §1 §Degenerierte Zyklen): kein Raum, kein E-GEO-002,
    // die Wände selbst bleiben unverändert im Modell.
    const std::vector<model::WallId> ids =
        addRect(svc, Rect{0.0, 0.0, 150.0, 150.0}, 200.0);

    EXPECT_TRUE(svc.rooms(kGroundStorey).empty());
    EXPECT_EQ(ids.size(), 4U);
    EXPECT_EQ(svc.building().walls.size(), 4U);
}
