// Akzeptanz-Tests für die Raum-Autoerkennung (slice-009b; erweitert um
// die Code-Review-Findings M1/M2 der Welle-1-Prüfung), OCC-frei über
// ein GeometryKernelPort-Double (ADR-0001 §Testbarkeit). Geprüft wird
// LH-FA-ROM-001 gegen spez. §1 LH-FA-ROM-001.a und ADR-0007:
// - Happy: geschlossener Wandzug -> genau ein Raum, automatisch beim
//   Schließen, Innenkanten-Polygon/Netto-Fläche analytisch korrekt.
// - Mutation: Stärke-Änderung löst Re-Detektion aus.
// - Boundary: verschachtelte Wandzüge -> innen/außen getrennt, äußerer
//   mit Loch-Ring, keine Flächen-Doppelzählung.
// - Geteilte Wand (Review M1): zwei Räume mit gemeinsamer Wand
//   (Grad-3-Knoten) werden beide erkannt (minimale Zyklen via
//   Flächen-Traversierung).
// - Kollineare Kanten ungleicher Stärke (Review M2): dokumentierte
//   Welle-1-Näherung, Erwartungswert gepinnt.
// - Negative: offener Wandzug -> kein Raum, kein Fehler.
// - Degeneriert: kollabierender Innenkanten-Offset -> kein Raum, kein
//   Fehler, Modell unverändert (Erkennung total, kein E-GEO-002).
// Test-Namen tragen die LH-ID. Erwartungswerte: Innenkanten-Basis
// (ADR-0007) — Mittellinie minus halbe Wandstärke je Seite.

#include <gtest/gtest.h>

#include <array>
#include <optional>
#include <vector>

#include "analytic_geometry_double.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/room.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
using bcad::testing::AnalyticGeometry;

constexpr model::StoreyId kGroundStorey{1};
constexpr double kAreaToleranceMm2 = 1e-6;

// Fügt eine Wand hinzu und setzt ihre Stärke; schlägt der Add fehl,
// scheitert der Test sauber statt per UB-Dereferenzierung (Review L4).
model::WallId addWallChecked(services::StructureEditService& svc,
                             model::Segment seg, double thickness_mm) {
    const std::optional<model::WallId> id = svc.addWall(kGroundStorey, seg);
    EXPECT_TRUE(id.has_value()) << "addWall hat die Wand verworfen";
    if (!id.has_value()) {
        return model::WallId{};
    }
    svc.setWallThickness(*id, thickness_mm);
    return *id;
}

// Achsenparalleles Rechteck (Mittellinien-Koordinaten).
struct Rect {
    double x0;
    double y0;
    double x1;
    double y1;
};

// Vier Wände eines Rechtecks (geschlossener Wandzug), je `thickness_mm`.
// Liefert die Wand-Ids in Zeichen-Reihenfolge (unten, rechts, oben, links).
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
        ids.push_back(addWallChecked(svc, seg, thickness_mm));
    }
    return ids;
}

}  // namespace

TEST(RoomDetection_LH_FA_ROM_001, GeschlossenerWandzugErzeugtGenauEinenRaumAutomatisch) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // Drei Wände: Zug noch offen -> kein Raum.
    addWallChecked(svc, {{0.0, 0.0}, {5000.0, 0.0}}, 200.0);
    addWallChecked(svc, {{5000.0, 0.0}, {5000.0, 4000.0}}, 200.0);
    addWallChecked(svc, {{5000.0, 4000.0}, {0.0, 4000.0}}, 200.0);
    EXPECT_TRUE(svc.rooms(kGroundStorey).empty());

    // Vierte Wand schließt den Zug -> der Raum entsteht automatisch beim
    // Schließen (LH-FA-ROM-001 Happy); rooms() ist reine Abfrage.
    addWallChecked(svc, {{0.0, 4000.0}, {0.0, 0.0}}, 200.0);
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

TEST(RoomDetection_LH_FA_ROM_001, GeteilteWandErzeugtBeideRaeume) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // Zwei Räume mit GEMEINSAMER Mittelwand — die Eckknoten (4000,0)
    // und (4000,4000) haben Grad 3 (Review-Finding M1: minimale Zyklen
    // via Flächen-Traversierung, nicht nur Grad-2-Komponenten).
    addWallChecked(svc, {{0.0, 0.0}, {4000.0, 0.0}}, 200.0);        // unten links
    addWallChecked(svc, {{4000.0, 0.0}, {8000.0, 0.0}}, 200.0);     // unten rechts
    addWallChecked(svc, {{8000.0, 0.0}, {8000.0, 4000.0}}, 200.0);  // rechts
    addWallChecked(svc, {{8000.0, 4000.0}, {4000.0, 4000.0}}, 200.0);  // oben rechts
    addWallChecked(svc, {{4000.0, 4000.0}, {0.0, 4000.0}}, 200.0);  // oben links
    addWallChecked(svc, {{0.0, 4000.0}, {0.0, 0.0}}, 200.0);        // links
    addWallChecked(svc, {{4000.0, 0.0}, {4000.0, 4000.0}}, 200.0);  // Mittelwand

    const std::vector<model::Room> rooms = svc.rooms(kGroundStorey);
    ASSERT_EQ(rooms.size(), 2U);

    // Beide Räume: Breite 4000 - 100 (Außenwand) - 100 (Mittelwand),
    // Höhe 4000 - 200 -> je 3800 x 3800; keine Löcher (nebeneinander,
    // nicht verschachtelt).
    for (const model::Room& room : rooms) {
        EXPECT_TRUE(room.holes.empty());
        EXPECT_NEAR(room.net_area_mm2, 3800.0 * 3800.0, kAreaToleranceMm2);
    }
}

TEST(RoomDetection_LH_FA_ROM_001, KollineareKantenUngleicherStaerkeGepinnteNaeherung) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // Untere Seite in zwei kollineare Wände ungleicher Stärke gesplittet
    // (Review-Finding M2): die Ecke am Übergang springt per
    // dokumentierter Welle-1-Näherung auf den Offset-Punkt der
    // Folgekante (lineare Überblendung statt exakter Stufe, spez. §1).
    addWallChecked(svc, {{0.0, 0.0}, {2500.0, 0.0}}, 200.0);
    addWallChecked(svc, {{2500.0, 0.0}, {5000.0, 0.0}}, 400.0);
    addWallChecked(svc, {{5000.0, 0.0}, {5000.0, 4000.0}}, 200.0);
    addWallChecked(svc, {{5000.0, 4000.0}, {0.0, 4000.0}}, 200.0);
    addWallChecked(svc, {{0.0, 4000.0}, {0.0, 0.0}}, 200.0);

    const std::vector<model::Room> rooms = svc.rooms(kGroundStorey);
    ASSERT_EQ(rooms.size(), 1U);

    // Gepinnter Erwartungswert der Näherung (Polygon (100,100),
    // (2500,200), (4900,200), (4900,3900), (100,3900)): 17.880.000 mm²
    // — exakte Stufenkontur wäre 18.000.000 mm² (Differenz = halbes
    // Übergangs-Dreieck, dokumentiert; exakt erst mit LH-FA-WAL-006).
    EXPECT_NEAR(rooms.front().net_area_mm2, 17880000.0, kAreaToleranceMm2);
}

TEST(RoomDetection_LH_FA_ROM_001, OffenerWandzugErzeugtKeinenRaumKeinFehler) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    // Drei Seiten eines Rechtecks — Zug bleibt offen (LH-FA-ROM-001
    // Negative): kein Raum, kein Fehler.
    addWallChecked(svc, {{0.0, 0.0}, {5000.0, 0.0}}, 200.0);
    addWallChecked(svc, {{5000.0, 0.0}, {5000.0, 4000.0}}, 200.0);
    addWallChecked(svc, {{5000.0, 4000.0}, {0.0, 4000.0}}, 200.0);

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
