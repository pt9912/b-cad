// Akzeptanz-Tests für die Flächen-Auswertung (slice-017b, EvaluatePort) —
// OCC-frei über ein GeometryKernelPort-Double (ADR-0001 §Testbarkeit).
// Geprüft wird LH-FA-EVL-001 (Netto-Grundfläche je Raum + Summe je Geschoss)
// und LH-FA-EVL-003 (Wohnfläche gebäudeweit) gegen spez. §1 LH-FA-EVL-001.a
// und ADR-0012:
// - EVL-001 Happy: geschlossener Wandzug -> floorArea liefert die Per-Raum-
//   Netto-Grundfläche UND deren Summe in m² (analytisches Orakel, mm²→m²).
// - EVL-001 Mehrraum: floorArea aggregiert die erkannten Räume exakt
//   (elementweise = rooms().net_area_mm2 / 1e6) — keine Doppelzählung.
// - EVL-001 Loch-Ring (Plan-Review LOW-2): verschachtelter Zug -> der
//   äußere Raum trägt sein Loch; die Netto-Fläche (Ring − Loch) fließt durch
//   die Aggregation (ADR-0007-Netto-Wiederverwendung, kein Brutto).
// - EVL-001 Boundary: Geschoss ohne/unbekannt -> total 0, leer, kein Wurf.
// - EVL-003: Wohnfläche = Summe aller Raum-Netto-Flächen × kLivingAreaFactor,
//   gebäudeweit über mehrere Geschosse.
// - read-only (LOW-1): die Auswertung mutiert das Modell nicht; zwei Abfragen
//   liefern identische Ergebnisse (kein versteckter Re-Detect).
// Erwartungswerte: Innenkanten-Basis (ADR-0007) — Mittellinie minus halbe
// Wandstärke je Seite; die Auswertung ist reine Aggregation dieser Werte.

#include <gtest/gtest.h>

#include <array>
#include <vector>

#include "analytic_geometry_double.h"
#include "hexagon/model/area_report.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/room.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driving/evaluate_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace ports = bcad::hexagon::ports;
namespace services = bcad::hexagon::services;
using bcad::testing::AnalyticGeometry;

constexpr model::StoreyId kGroundStorey{1};
// Flächen in m²; Netto-Flächen sind exakte Integer-mm² (z.B. 18 240 000),
// ÷ 1e6 -> glatte m² — eine großzügige fp-Toleranz genügt.
constexpr double kAreaToleranceM2 = 1e-6;

// Fügt eine Wand auf `storey` hinzu und setzt ihre Stärke; schlägt der Add
// fehl, scheitert der Test sauber statt per UB-Dereferenzierung.
void addWallOn(services::StructureEditService& svc, model::StoreyId storey,
               model::Segment seg, double thickness_mm) {
    const std::optional<model::WallId> id = svc.addWall(storey, seg);
    EXPECT_TRUE(id.has_value()) << "addWall hat die Wand verworfen";
    if (id.has_value()) {
        svc.setWallThickness(*id, thickness_mm);
    }
}

// Achsenparalleles Rechteck (Mittellinien-Koordinaten).
struct Rect {
    double x0;
    double y0;
    double x1;
    double y1;
};

// Vier Wände eines geschlossenen Rechteck-Wandzugs auf `storey`.
void addRectOn(services::StructureEditService& svc, model::StoreyId storey,
               Rect r, double thickness_mm) {
    const std::array<model::Segment, 4> segments{{
        {{r.x0, r.y0}, {r.x1, r.y0}},
        {{r.x1, r.y0}, {r.x1, r.y1}},
        {{r.x1, r.y1}, {r.x0, r.y1}},
        {{r.x0, r.y1}, {r.x0, r.y0}},
    }};
    for (const model::Segment& seg : segments) {
        addWallOn(svc, storey, seg, thickness_mm);
    }
}

// Belegt, dass floorArea die erkannten Räume eines Geschosses 1:1 aggregiert
// (Per-Raum-Fläche elementweise = net_area_mm2/1e6, Summe = total) — der
// geometrie-agnostische Treue-Beleg der Aggregation.
void expectAggregatesRooms(const services::StructureEditService& svc,
                           model::StoreyId storey) {
    const std::vector<model::Room> rooms = svc.rooms(storey);
    const model::AreaReport report = svc.floorArea(storey);
    ASSERT_EQ(report.room_areas_m2.size(), rooms.size());
    EXPECT_EQ(report.room_count(), static_cast<int>(rooms.size()));
    double sum = 0.0;
    for (std::size_t i = 0; i < rooms.size(); ++i) {
        const double expected_m2 = rooms[i].net_area_mm2 / 1.0e6;
        EXPECT_NEAR(report.room_areas_m2[i], expected_m2, kAreaToleranceM2);
        sum += expected_m2;
    }
    EXPECT_NEAR(report.total_m2, sum, kAreaToleranceM2);
}

}  // namespace

// EVL-001 Happy: ein geschlossener Wandzug -> Per-Raum-Fläche und Summe in m².
TEST(Evaluate_LH_FA_EVL_001, GrundflaecheJeRaumUndSumme) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addRectOn(svc, kGroundStorey, Rect{0.0, 0.0, 5000.0, 4000.0}, 200.0);
    ASSERT_EQ(svc.rooms(kGroundStorey).size(), 1U);

    // Innenkanten-Basis (ADR-0007): (5000-200) x (4000-200) = 18 240 000 mm²
    // = 18,24 m². floorArea liefert genau diesen Wert je Raum UND als Summe.
    const model::AreaReport report = svc.floorArea(kGroundStorey);
    ASSERT_EQ(report.room_areas_m2.size(), 1U);
    EXPECT_EQ(report.room_count(), 1);
    EXPECT_NEAR(report.room_areas_m2.front(), 18.24, kAreaToleranceM2);
    EXPECT_NEAR(report.total_m2, 18.24, kAreaToleranceM2);
}

// EVL-001 Mehrraum: zwei getrennte Wandzüge -> Summe + Per-Raum exakt.
TEST(Evaluate_LH_FA_EVL_001, MehrereRaeumeSummeUndJeRaum) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addRectOn(svc, kGroundStorey, Rect{0.0, 0.0, 5000.0, 4000.0}, 200.0);
    addRectOn(svc, kGroundStorey, Rect{8000.0, 0.0, 11000.0, 3000.0}, 200.0);
    ASSERT_EQ(svc.rooms(kGroundStorey).size(), 2U);

    // Aggregations-Treue (elementweise = rooms()) + analytische Summe:
    // (4800x3800) + (2800x2800) = 18 240 000 + 7 840 000 = 26,08 m².
    expectAggregatesRooms(svc, kGroundStorey);
    EXPECT_NEAR(svc.floorArea(kGroundStorey).total_m2, 26.08, kAreaToleranceM2);
}

// EVL-001 Loch-Ring (Plan-Review LOW-2): verschachtelter Wandzug -> der äußere
// Raum trägt ein Loch; die Netto-Fläche (Ring − Loch) fließt durch (ADR-0007).
TEST(Evaluate_LH_FA_EVL_001, LochRingNettoFliesstDurch) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addRectOn(svc, kGroundStorey, Rect{0.0, 0.0, 10000.0, 8000.0}, 200.0);
    addRectOn(svc, kGroundStorey, Rect{4000.0, 3000.0, 7000.0, 5000.0}, 100.0);
    const std::vector<model::Room> rooms = svc.rooms(kGroundStorey);
    ASSERT_EQ(rooms.size(), 2U);
    // Genau ein Raum trägt den Loch-Ring (der äußere) — Vorbedingung des Belegs.
    int with_hole = 0;
    for (const model::Room& r : rooms) {
        with_hole += r.holes.empty() ? 0 : 1;
    }
    ASSERT_EQ(with_hole, 1);

    // Aggregations-Treue (übernimmt die loch-reduzierte Netto-Fläche je Raum)
    // + analytische Summe: äußerer Ring (9800x7800) − Loch (3100x2100)
    // + innerer Raum (2900x1900) = 69 930 000 + 5 510 000 = 75,44 m².
    // (Bei brutto/Doppelzählung läge der Wert höher — der Test pinnt netto.)
    expectAggregatesRooms(svc, kGroundStorey);
    EXPECT_NEAR(svc.floorArea(kGroundStorey).total_m2, 75.44, kAreaToleranceM2);
}

// EVL-001 Boundary: Geschoss ohne geschlossene Räume bzw. unbekanntes Geschoss
// -> leerer Report (total 0), kein Wurf (Totalität).
TEST(Evaluate_LH_FA_EVL_001, GeschossOhneRaeumeLiefertNull) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    // Offener Wandzug (drei Wände) -> kein Raum.
    addWallOn(svc, kGroundStorey, {{0.0, 0.0}, {5000.0, 0.0}}, 200.0);
    addWallOn(svc, kGroundStorey, {{5000.0, 0.0}, {5000.0, 4000.0}}, 200.0);
    ASSERT_TRUE(svc.rooms(kGroundStorey).empty());

    const model::AreaReport empty = svc.floorArea(kGroundStorey);
    EXPECT_DOUBLE_EQ(empty.total_m2, 0.0);
    EXPECT_TRUE(empty.room_areas_m2.empty());
    EXPECT_EQ(empty.room_count(), 0);

    // Unbekanntes Geschoss -> ebenfalls leer, kein Wurf.
    const model::AreaReport unknown = svc.floorArea(model::StoreyId{999});
    EXPECT_DOUBLE_EQ(unknown.total_m2, 0.0);
    EXPECT_TRUE(unknown.room_areas_m2.empty());
}

// EVL-003: Wohnfläche = Summe aller Raum-Netto-Flächen × kLivingAreaFactor,
// gebäudeweit über mehrere Geschosse.
TEST(Evaluate_LH_FA_EVL_003, WohnflaecheGebaeudeweitMalFaktor) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addRectOn(svc, kGroundStorey, Rect{0.0, 0.0, 5000.0, 4000.0}, 200.0);  // 18,24
    const model::StoreyId upper = svc.addStorey(model::kDefaultStoreyHeightMm);
    addRectOn(svc, upper, Rect{0.0, 0.0, 4000.0, 3000.0}, 200.0);          // 10,64
    ASSERT_EQ(svc.rooms(kGroundStorey).size(), 1U);
    ASSERT_EQ(svc.rooms(upper).size(), 1U);

    // (4800x3800)+(3800x2800) = 18,24 + 10,64 = 28,88 m², × Faktor 1.
    const double expected_m2 = 28.88 * model::kLivingAreaFactor;
    const model::AreaReport living = svc.livingArea();
    EXPECT_EQ(living.room_count(), 2);
    EXPECT_NEAR(living.total_m2, expected_m2, kAreaToleranceM2);
    // Gebäudeweit = Summe der Geschoss-Grundflächen × Faktor.
    EXPECT_NEAR(living.total_m2,
                (svc.floorArea(kGroundStorey).total_m2 +
                 svc.floorArea(upper).total_m2) *
                    model::kLivingAreaFactor,
                kAreaToleranceM2);
}

// read-only (Plan-Review LOW-1, ADR-0012): die Auswertung mutiert nichts und
// liefert über zwei Abfragen identische Ergebnisse (kein versteckter Re-Detect).
TEST(Evaluate_LH_FA_EVL_001, AuswertungIstReadOnlyUndStabil) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    addRectOn(svc, kGroundStorey, Rect{0.0, 0.0, 5000.0, 4000.0}, 200.0);

    // Zugriff über den Port-Typ (virtuelle Auflösung wie im Adapter).
    const ports::driving::EvaluatePort& port = svc;

    const std::size_t walls_before = svc.building().walls.size();
    const double rooms_area_before = svc.rooms(kGroundStorey).front().net_area_mm2;

    const model::AreaReport first = port.floorArea(kGroundStorey);
    const model::AreaReport second = port.floorArea(kGroundStorey);

    // Identisch über zwei Abfragen (kein versteckter Zustand/Re-Detect).
    ASSERT_EQ(first.room_areas_m2.size(), second.room_areas_m2.size());
    EXPECT_DOUBLE_EQ(first.total_m2, second.total_m2);
    for (std::size_t i = 0; i < first.room_areas_m2.size(); ++i) {
        EXPECT_DOUBLE_EQ(first.room_areas_m2[i], second.room_areas_m2[i]);
    }
    // Modell unverändert: weder Wände noch erkannte Räume berührt.
    EXPECT_EQ(svc.building().walls.size(), walls_before);
    ASSERT_EQ(svc.rooms(kGroundStorey).size(), 1U);
    EXPECT_DOUBLE_EQ(svc.rooms(kGroundStorey).front().net_area_mm2,
                     rooms_area_before);
}
