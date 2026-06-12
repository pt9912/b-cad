// AK-Tests Eckenschluss (LH-FA-WAL-006-Teilumfang, slice-012):
// endpunkt-verbundene Wände (Grad-2-Knoten, gleiches Geschoss)
// schließen ihre Ecke; Begrenzung/Abgrenzungen fallen auf stumpf
// zurück. Footprint-Ebene (wall_footprint) + Service-Ebene
// (Mehr-Element-Meldung WallGeometryChanged, ADR-0008 §3).

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

#include "analytic_geometry_double.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/services/structure_edit_service.h"
#include "hexagon/services/wall_footprint.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;
using bcad::testing::AnalyticGeometry;
using services::buttFootprint;
using services::wallFootprint;

model::Wall makeWall(int id, int storey, model::Point2D a, model::Point2D b,
                     double thickness_mm) {
    model::Wall wall{};
    wall.id = static_cast<model::WallId>(id);
    wall.storey_id = static_cast<model::StoreyId>(storey);
    wall.start = a;
    wall.end = b;
    wall.thickness_mm = thickness_mm;
    wall.height_mm = 2500.0;
    return wall;
}

bool nearPoint(model::Point2D a, model::Point2D b) {
    return std::abs(a.x_mm - b.x_mm) < model::kGeometryToleranceMm &&
           std::abs(a.y_mm - b.y_mm) < model::kGeometryToleranceMm;
}

bool sameFootprint(const model::Footprint& a, const model::Footprint& b) {
    if (a.points.size() != b.points.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.points.size(); ++i) {
        if (!nearPoint(a.points[i], b.points[i])) {
            return false;
        }
    }
    return true;
}

// Ray-Casting (Halbstrahl +x) — Punkt-im-Polygon für die Kerben-Probe.
bool insidePolygon(const model::Footprint& fp, model::Point2D q) {
    bool inside = false;
    const std::size_t n = fp.points.size();
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const auto& a = fp.points[i];
        const auto& b = fp.points[j];
        const bool crosses = (a.y_mm > q.y_mm) != (b.y_mm > q.y_mm);
        if (crosses &&
            q.x_mm < ((b.x_mm - a.x_mm) * (q.y_mm - a.y_mm) /
                          (b.y_mm - a.y_mm) +
                      a.x_mm)) {
            inside = !inside;
        }
    }
    return inside;
}

// Protokollierender Beobachter für die Meldungs-Reihenfolge.
struct RecordingListener final : driven::ModelChangedPort {
    std::vector<driven::ModelChange> changes;
    void onModelChanged(const driven::ModelChange& change) override {
        changes.push_back(change);
    }
    int countOf(driven::ModelChangeOp op) const {
        int count = 0;
        for (const auto& c : changes) {
            count += (c.op == op) ? 1 : 0;
        }
        return count;
    }
};

}  // namespace

// Happy: rechter Winkel → Ecke körperlich geschlossen. Der äußere
// Eckpunkt (gemeinsamer Endpunkt ± halbe Stärken) liegt auf der
// Footprint-Kontur, ein zuvor offener Kerben-Punkt liegt im Polygon.
TEST(WallFootprint_LH_FA_WAL_006, Happy_RechterWinkelSchliesstEcke) {
    const auto w1 = makeWall(1, 1, {0, 0}, {1000, 0}, 240.0);
    const auto w2 = makeWall(2, 1, {1000, 0}, {1000, 1000}, 240.0);
    const std::vector<model::Wall> walls{w1, w2};

    const model::Footprint fp1 = wallFootprint(w1, walls);
    const model::Footprint fp2 = wallFootprint(w2, walls);

    // Kerben-Punkt (vorher offen: x>1000, y<0 innerhalb der Stärken).
    EXPECT_TRUE(insidePolygon(fp1, {1090, -90}) ||
                insidePolygon(fp2, {1090, -90}));
    EXPECT_TRUE(insidePolygon(fp1, {1050, -110}) ||
                insidePolygon(fp2, {1050, -110}));
    // Beide Wände treffen sich in denselben Eck-Punkten (nahtlos).
    bool shares_outer = false;
    for (const auto& p : fp1.points) {
        shares_outer = shares_outer || nearPoint(p, {1120, -120});
    }
    EXPECT_TRUE(shares_outer) << "äußerer Eckpunkt nicht auf fp1-Kontur";
    // Symmetrischer Eckschnitt erhält die Fläche (Trapez): Volumen wie
    // stumpf (Einzelwand-Orakel bleibt gültig).
    EXPECT_NEAR(bcad::testing::shoelaceArea(fp1), 1000.0 * 240.0, 1e-6);
}

// Boundary: kollineare Fortsetzung gleicher Stärke → glatter Übergang
// (stumpfe Enden stoßen nahtlos); ungleicher Stärke → stumpfer Stoß.
TEST(WallFootprint_LH_FA_WAL_006, Boundary_KollinearFaelltAufStumpfZurueck) {
    const auto w1 = makeWall(1, 1, {0, 0}, {1000, 0}, 240.0);
    const auto w2_gleich = makeWall(2, 1, {1000, 0}, {2000, 0}, 240.0);
    const auto w2_anders = makeWall(2, 1, {1000, 0}, {2000, 0}, 115.0);

    EXPECT_TRUE(sameFootprint(
        wallFootprint(w1, {w1, w2_gleich}), buttFootprint(w1)));
    EXPECT_TRUE(sameFootprint(
        wallFootprint(w1, {w1, w2_anders}), buttFootprint(w1)));
}

// Boundary: sehr spitzer Winkel → Begrenzung (WALL_MITER_LIMIT =
// max. größere Wandstärke über den Endpunkt hinaus) greift, Rückfall
// stumpf — kein Eck-Sporn.
TEST(WallFootprint_LH_FA_WAL_006, Boundary_SpitzwinkelBegrenztAufStumpf) {
    const auto w1 = makeWall(1, 1, {0, 0}, {1000, 0}, 240.0);
    const auto w2 = makeWall(2, 1, {1000, 0}, {0, 30}, 240.0);  // ~178°

    EXPECT_TRUE(sameFootprint(wallFootprint(w1, {w1, w2}), buttFootprint(w1)));
    EXPECT_TRUE(sameFootprint(wallFootprint(w2, {w1, w2}), buttFootprint(w2)));
}

// Negative: Grad ≥ 3 am Punkt → unverändert stumpf (Vollumfang
// „Schnittpunkte als Knoten" bleibt offen); anderes Geschoss zählt
// nicht als Nachbar.
TEST(WallFootprint_LH_FA_WAL_006, Negative_Grad3UndFremdesGeschossStumpf) {
    const auto w1 = makeWall(1, 1, {0, 0}, {1000, 0}, 240.0);
    const auto w2 = makeWall(2, 1, {1000, 0}, {1000, 1000}, 240.0);
    const auto w3 = makeWall(3, 1, {1000, 0}, {2000, 0}, 240.0);
    EXPECT_TRUE(
        sameFootprint(wallFootprint(w1, {w1, w2, w3}), buttFootprint(w1)));

    const auto og = makeWall(4, 2, {1000, 0}, {1000, 1000}, 240.0);
    EXPECT_TRUE(sameFootprint(wallFootprint(w1, {w1, og}), buttFootprint(w1)));
}

// Folge-Meldung (Mehr-Element-Update, ADR-0008 §3): Stärke-Änderung
// meldet den Eck-Nachbarn (WallGeometryChanged) — nach der auslösenden
// Op, vor RoomsChanged; Höhen-Änderung meldet keinen Nachbarn.
TEST(WallFootprint_LH_FA_WAL_006, FolgeMeldungNachbarBeiStaerkeNichtBeiHoehe) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto eg = svc.building().storeys.front().id;
    const auto a = svc.addWall(eg, {{0, 0}, {1000, 0}}).value();
    const auto b = svc.addWall(eg, {{1000, 0}, {1000, 1000}}).value();

    RecordingListener listener;
    svc.subscribe(listener);
    svc.setWallThickness(a, 300.0);
    ASSERT_EQ(listener.countOf(driven::ModelChangeOp::WallGeometryChanged), 1);
    // Reihenfolge: auslösende Op → Nachbar → RoomsChanged (spez. §1).
    ASSERT_EQ(listener.changes.size(), 3U);
    EXPECT_EQ(listener.changes[0].op,
              driven::ModelChangeOp::WallThicknessChanged);
    EXPECT_EQ(listener.changes[1].op,
              driven::ModelChangeOp::WallGeometryChanged);
    EXPECT_EQ(listener.changes[1].wall_id, b);
    EXPECT_EQ(listener.changes[2].op, driven::ModelChangeOp::RoomsChanged);

    listener.changes.clear();
    svc.setWallHeight(a, 3000.0);
    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::WallGeometryChanged), 0);
    svc.unsubscribe(listener);
}

// Boundary-Übergang Grad 2→3 (W3-Q8): die dritte Wand an einer
// vermiterten Ecke baut die Bestandswände auf stumpf zurück — und
// meldet sie.
TEST(WallFootprint_LH_FA_WAL_006, Boundary_DritteWandBautEckeStumpfZurueck) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const auto eg = svc.building().storeys.front().id;
    const auto a = svc.addWall(eg, {{0, 0}, {1000, 0}}).value();
    const auto b = svc.addWall(eg, {{1000, 0}, {1000, 1000}}).value();

    const model::Footprint mitered =
        wallFootprint(svc.wall(a), svc.building().walls);
    EXPECT_FALSE(sameFootprint(mitered, buttFootprint(svc.wall(a))));

    RecordingListener listener;
    svc.subscribe(listener);
    svc.addWall(eg, {{1000, 0}, {2000, 0}});  // Grad 2 → 3

    EXPECT_EQ(listener.countOf(driven::ModelChangeOp::WallGeometryChanged), 2);
    EXPECT_TRUE(sameFootprint(wallFootprint(svc.wall(a), svc.building().walls),
                              buttFootprint(svc.wall(a))));
    EXPECT_TRUE(sameFootprint(wallFootprint(svc.wall(b), svc.building().walls),
                              buttFootprint(svc.wall(b))));
    svc.unsubscribe(listener);
}

