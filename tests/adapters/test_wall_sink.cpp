// slice-058: die Wand-Senke als **Fehler-Barriere** und als die Stelle, die den
// Ausgang FESTSTELLT — Orakel-Zeilen §4-5 und §4-7.
//
// Warum hier und nicht im Canvas: der Kern verwirft unterhalb der Geometrie-
// Toleranz (0,1 mm), waehrend bei Maximal-Zoom (100 px/mm) benachbarte Pixel
// 0,01 mm auseinanderliegen. Ein Canvas, der an seinen eigenen zwei Punkten
// urteilte, haette den Zug fuer gueltig gehalten, waehrend der Kern ihn
// verwirft — ein FALSCHER Hinweis (slice-058 §2.3, Plan-Review Lauf 1).
//
// Kein Qt im Spiel: die Senke ist ein reines `ui/command/`-Objekt.

#include <gtest/gtest.h>

#include <optional>
#include <vector>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/command/edit_structure_wall_sink.h"
#include "hexagon/model/point2d.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace command = bcad::adapters::ui::command;

using command::WallDrawOutcome;

// Sammelt die gemeldeten Ausgaenge — das display-freie Surrogat der Hinweis-
// Meldung (der TEXT liegt im Composition-Root, nicht hier).
struct Recorder {
    std::vector<WallDrawOutcome> outcomes;

    command::EditStructureWallSink::OutcomeReport sink() {
        return [this](WallDrawOutcome o) { outcomes.push_back(o); };
    }
};

// §4-5: Entarteter Zug ⇒ keine Wand, Modell unveraendert, Hinweis.
// Der Kern lehnt wertbasiert ab (kein Wurf) — die Senke muss daraus einen
// GEMELDETEN Ausgang machen, nicht nur ein stilles `nullopt`.
TEST(WallSink, LH_FA_WAL_001_Boundary_EntarteterZugMeldetKeineWand) {
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const auto eg = service.building().storeys.front().id;
    Recorder recorder;
    const command::EditStructureWallSink sink(service, eg, recorder.sink());

    const std::size_t walls_before = service.building().walls.size();
    const std::optional<model::WallId> created =
        sink.addWall(model::Point2D{1000.0, 1000.0}, model::Point2D{1000.0, 1000.0});

    EXPECT_FALSE(created.has_value());
    EXPECT_EQ(service.building().walls.size(), walls_before)
        << "Modell muss unveraendert bleiben";
    ASSERT_EQ(recorder.outcomes.size(), 1U) << "genau EIN Ausgang je Zug";
    EXPECT_EQ(recorder.outcomes.front(), WallDrawOutcome::RejectedNoWall);
}

// §4-5-Gegenstueck: der gelungene Zug meldet `Created` — sonst waere die Zeile
// oben auch mit einer Senke gruen, die IMMER `RejectedNoWall` meldet.
TEST(WallSink, LH_FA_WAL_001_HappyPath_ZugLegtWandAnUndMeldetErfolg) {
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const auto eg = service.building().storeys.front().id;
    Recorder recorder;
    const command::EditStructureWallSink sink(service, eg, recorder.sink());

    const std::size_t walls_before = service.building().walls.size();
    const std::optional<model::WallId> created =
        sink.addWall(model::Point2D{0.0, 0.0}, model::Point2D{4000.0, 0.0});

    ASSERT_TRUE(created.has_value());
    EXPECT_EQ(service.building().walls.size(), walls_before + 1);
    ASSERT_EQ(recorder.outcomes.size(), 1U);
    EXPECT_EQ(recorder.outcomes.front(), WallDrawOutcome::Created);
    // Die neue Wand liegt im ZIEL-Geschoss (Plan-Risiko R3: das Demo-Modell hat
    // deckungsgleiche Waende in zwei Geschossen — Anzahl allein genuegt nicht).
    EXPECT_EQ(service.building().walls.back().storey_id, eg);
}

// §4-7: Kein Wurf verlaesst den Ereignis-Pfad. `addWall` WIRFT bei unbekannter
// Geschoss-Id (`std::out_of_range`) — der Fall ist real erreichbar: das
// Zeichen-Ziel ist eine injizierte Id, die nach einem Projekt-Wechsel neu
// aufgeloest werden muss (ADR-0021 E10).
TEST(WallSink, ADR_0021_E10_VeralteteGeschossIdWirdGefangenUndGemeldet) {
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const auto stale = static_cast<model::StoreyId>(4242);  // existiert nicht
    Recorder recorder;
    const command::EditStructureWallSink sink(service, stale, recorder.sink());

    const std::size_t walls_before = service.building().walls.size();
    // Die Zusage ist die Totalitaet: KEIN Wurf verlaesst diesen Aufruf.
    std::optional<model::WallId> created;
    EXPECT_NO_THROW(created = sink.addWall(model::Point2D{0.0, 0.0},
                                           model::Point2D{4000.0, 0.0}));

    EXPECT_FALSE(created.has_value());
    EXPECT_EQ(service.building().walls.size(), walls_before)
        << "transaktional: das Modell bleibt unveraendert";
    ASSERT_EQ(recorder.outcomes.size(), 1U);
    EXPECT_EQ(recorder.outcomes.front(), WallDrawOutcome::Failed)
        << "der gefangene Wurf muss als Ausgang GEMELDET werden — ein still "
           "geschluckter Wurf ist keine Barriere, sondern ein Verschwinden";
}

// §4-7-Nachzug: `setTarget` heilt die veraltete Id — derselbe Weg, den der
// Composition-Root nach einem Projekt-Laden geht (slice-047b-Muster).
TEST(WallSink, ADR_0021_E10_SetTargetHeiltDieVeralteteId) {
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    Recorder recorder;
    command::EditStructureWallSink sink(
        service, static_cast<model::StoreyId>(4242), recorder.sink());

    sink.addWall(model::Point2D{0.0, 0.0}, model::Point2D{4000.0, 0.0});
    ASSERT_EQ(recorder.outcomes.size(), 1U);
    ASSERT_EQ(recorder.outcomes.front(), WallDrawOutcome::Failed);

    sink.setTarget(service.building().storeys.front().id);
    const std::optional<model::WallId> created =
        sink.addWall(model::Point2D{0.0, 0.0}, model::Point2D{4000.0, 0.0});

    EXPECT_TRUE(created.has_value());
    ASSERT_EQ(recorder.outcomes.size(), 2U);
    EXPECT_EQ(recorder.outcomes.back(), WallDrawOutcome::Created);
}

// Die Senke ohne Melde-Callable bleibt total — ein nicht gesetzter Empfaenger
// darf nicht zum Absturz fuehren (der Canvas-Weg baut sie im Test ohne).
TEST(WallSink, OhneMeldeCallableBleibtDerAufrufTotal) {
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    const command::EditStructureWallSink sink(
        service, static_cast<model::StoreyId>(4242), {});
    EXPECT_NO_THROW(sink.addWall(model::Point2D{0.0, 0.0},
                                 model::Point2D{4000.0, 0.0}));
}

}  // namespace
