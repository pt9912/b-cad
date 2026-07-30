// slice-059b: die Parameter-Senke — Umwandlung, Klemmung, Ablehnung, Barriere.
// Orakel-Zeilen §4-5 (Senken-Haelfte), §4-7, §4-8, §4-9 (Senken-Haelfte), §4-10
// und die Umwandlungs-Haelfte von §4-13.
//
// Warum die Ausgangs-Entscheidung HIER faellt und nicht im Fenster oder im
// Composition-Root (Plan §2.2): eine nicht-numerische Eingabe erreicht den Kern
// NIE — im Root waere die Entscheidung orakel-los, im Fenster entstuende eine
// zweite Ausgangs-Autoritaet neben dieser Senke.
//
// Kein Qt im Spiel: die Senke nimmt `std::string_view`.

#include <gtest/gtest.h>

#include <optional>
#include <vector>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/command/wall_param_sink.h"
#include "hexagon/model/segment.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace command = bcad::adapters::ui::command;

using command::ParamEditOutcome;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

struct Fixture {
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service{geometry};
    std::vector<command::ParamEditResult> reported;
    model::WallId wall{};

    Fixture() {
        const auto storey = service.building().storeys.front().id;
        wall = *service.addWall(storey, seg(0, 0, 4000, 0));
    }

    command::WallParamSink sink() {
        return command::WallParamSink(
            service, [this](command::ParamEditResult r) { reported.push_back(r); });
    }

    double thickness() const {
        return service.building().walls.front().thickness_mm;
    }
    double height() const { return service.building().walls.front().height_mm; }
};

// §4-8: die Umwandlung selbst — sie ist Bauvorschrift, nicht Freiheit.
// Volle Konsumption: ein Rest macht die Eingabe zu KEINER Zahl.
TEST(WallParamSink, DieUmwandlungVerlangtVolleKonsumption) {
    using Sink = command::WallParamSink;
    EXPECT_TRUE(Sink::parse("240").has_value());
    EXPECT_TRUE(Sink::parse("240.5").has_value());
    EXPECT_TRUE(Sink::parse("49").has_value());
    // Nicht-endliche Werte reicht die Umwandlung DURCH — sie werden im KERN
    // abgelehnt, und diese Zweiteilung ist gewollt (Plan §2.2).
    EXPECT_TRUE(Sink::parse("inf").has_value());
    // Kein Zahlwert bzw. Rest uebrig:
    EXPECT_FALSE(Sink::parse("abc").has_value());
    EXPECT_FALSE(Sink::parse("").has_value());
    EXPECT_FALSE(Sink::parse("50 mm").has_value()) << "Rest uebrig";
    EXPECT_FALSE(Sink::parse("50,0").has_value()) << "Komma ist kein Dezimalpunkt";
    EXPECT_FALSE(Sink::parse("1.000").has_value() &&
                 *Sink::parse("1.000") == 1000.0)
        << "ein Punkt ist ein Dezimalpunkt, kein Tausender-Trenner";
}

// §4-8: nicht-numerische Eingabe ⇒ Ablehnung IN DER SENKE, kein Port-Aufruf.
TEST(WallParamSink, LH_FA_WAL_002_Negative_NichtNumerischWirdInDerSenkeAbgelehnt) {
    Fixture fx;
    const auto sink = fx.sink();
    const double before = fx.thickness();

    for (const char* input : {"abc", "", "50 mm", "50,0"}) {
        fx.reported.clear();
        const auto result = sink.setThickness(fx.wall, input);
        EXPECT_EQ(result.outcome, ParamEditOutcome::NotANumber) << input;
        EXPECT_DOUBLE_EQ(fx.thickness(), before)
            << "Modell unveraendert fuer: " << input;
        ASSERT_EQ(fx.reported.size(), 1U);
        EXPECT_EQ(fx.reported.front().outcome, ParamEditOutcome::NotANumber);
    }
    // Die Gegenprobe des Plans, hier als positive Zusage: ohne die
    // Umwandlungs-Pruefung wuerde `0` uebernommen und auf 50 GEKLEMMT — eine
    // stille Falsch-Uebernahme statt einer Ablehnung.
    EXPECT_DOUBLE_EQ(fx.thickness(), before);
}

// §4-7: nicht-endliche Eingabe erreicht den Kern und wird DORT abgelehnt.
TEST(WallParamSink, LH_FA_WAL_002_Negative_NichtEndlichWirdVomKernAbgelehnt) {
    Fixture fx;
    const auto sink = fx.sink();
    const double before = fx.thickness();

    const auto result = sink.setThickness(fx.wall, "inf");
    EXPECT_EQ(result.outcome, ParamEditOutcome::Rejected)
        << "der KERN lehnt ab (E-VAL-001), nicht die Senke";
    EXPECT_DOUBLE_EQ(fx.thickness(), before);
    ASSERT_EQ(fx.reported.size(), 1U);
    EXPECT_EQ(fx.reported.front().outcome, ParamEditOutcome::Rejected);
}

// §4-5 (Senken-Haelfte): Klemmung mit dem UEBERNOMMENEN Wert.
TEST(WallParamSink, LH_FA_WAL_002_Boundary_KlemmungMeldetDenUebernommenenWert) {
    Fixture fx;
    const auto sink = fx.sink();

    const auto low = sink.setThickness(fx.wall, "49");
    EXPECT_EQ(low.outcome, ParamEditOutcome::Clamped);
    EXPECT_DOUBLE_EQ(low.applied_mm, 50.0)
        << "der uebernommene Wert ist der abnahmebindende Teil der Meldung";
    EXPECT_DOUBLE_EQ(fx.thickness(), 50.0);

    const auto high = sink.setThickness(fx.wall, "1001");
    EXPECT_EQ(high.outcome, ParamEditOutcome::Clamped);
    EXPECT_DOUBLE_EQ(high.applied_mm, 1000.0);
    EXPECT_DOUBLE_EQ(fx.thickness(), 1000.0);

    // Innerhalb des Bereichs: uebernommen wie eingegeben.
    const auto ok = sink.setThickness(fx.wall, "240");
    EXPECT_EQ(ok.outcome, ParamEditOutcome::Accepted);
    EXPECT_DOUBLE_EQ(ok.applied_mm, 240.0);
    EXPECT_DOUBLE_EQ(fx.thickness(), 240.0);
}

// §4-9: die HOEHE gilt gleichlautend — eigener Test, nicht "analog".
// Eine Verdrahtungs-Verwechslung waere sonst still: beide Wege liefern
// denselben Ergebnis-Typ.
TEST(WallParamSink, LH_FA_WAL_003_HoeheGiltGleichlautendUndTrifftDenEigenenWert) {
    Fixture fx;
    const auto sink = fx.sink();
    const double thickness_before = fx.thickness();

    const auto clamped = sink.setHeight(fx.wall, "499");
    EXPECT_EQ(clamped.outcome, ParamEditOutcome::Clamped);
    EXPECT_DOUBLE_EQ(clamped.applied_mm, 500.0)
        << "die Hoehe klemmt auf 500, nicht auf 50 — waere der Hoehen-Weg auf "
           "den Staerke-Mutator verdrahtet, staende hier 50";
    EXPECT_DOUBLE_EQ(fx.height(), 500.0);
    EXPECT_DOUBLE_EQ(fx.thickness(), thickness_before)
        << "die Staerke darf sich dabei NICHT bewegen";

    const auto ok = sink.setHeight(fx.wall, "2600");
    EXPECT_EQ(ok.outcome, ParamEditOutcome::Accepted);
    EXPECT_DOUBLE_EQ(fx.height(), 2600.0);
    EXPECT_DOUBLE_EQ(fx.thickness(), thickness_before);
}

// §4-10: kein Wurf verlaesst den Ereignis-Pfad. `setWallThickness` WIRFT bei
// unbekannter Wand-Id — der Fall ist real erreichbar (eine Auswahl, die einen
// Projekt-Wechsel ueberlebt haette; ADR-0021 E10/E17).
TEST(WallParamSink, ADR_0021_E10_VeralteteWandIdWirdGefangenUndGemeldet) {
    Fixture fx;
    const auto sink = fx.sink();
    const auto stale = static_cast<model::WallId>(4242);
    const double before = fx.thickness();

    command::ParamEditResult result{};
    EXPECT_NO_THROW(result = sink.setThickness(stale, "240"));
    EXPECT_EQ(result.outcome, ParamEditOutcome::Failed);
    EXPECT_DOUBLE_EQ(fx.thickness(), before);
    ASSERT_EQ(fx.reported.size(), 1U);
    EXPECT_EQ(fx.reported.front().outcome, ParamEditOutcome::Failed)
        << "der gefangene Wurf muss als Ausgang GEMELDET werden — ein still "
           "geschluckter Wurf ist keine Barriere, sondern ein Verschwinden";

    EXPECT_NO_THROW(sink.setHeight(stale, "2600"));
}

// §4-13 (Umwandlungs-Haelfte): was die Anzeige zeigt, nimmt die Senke wieder an.
// Die Anzeige-Form selbst prueft der Fenster-Test; hier steht die Zusage der
// Senke: eine Zahl mit einer Nachkommastelle ist vollstaendig lesbar.
TEST(WallParamSink, DieAnzeigeFormBleibtLesbar) {
    Fixture fx;
    const auto sink = fx.sink();
    // Genau die Form, die das Fenster erzeugt (QString::number(v, 'f', 1)).
    for (const char* shown : {"240.0", "50.0", "1000.0", "2500.0"}) {
        const auto result = sink.setThickness(fx.wall, shown);
        EXPECT_NE(result.outcome, ParamEditOutcome::NotANumber)
            << "die Anzeige-Form muss ohne Aenderung wieder annehmbar sein: "
            << shown;
    }
}

// Ohne Melde-Callable bleibt der Aufruf total.
TEST(WallParamSink, OhneMeldeCallableBleibtDerAufrufTotal) {
    Fixture fx;
    const command::WallParamSink sink(fx.service, {});
    EXPECT_NO_THROW(sink.setThickness(static_cast<model::WallId>(4242), "240"));
    EXPECT_NO_THROW(sink.setThickness(fx.wall, "abc"));
}

}  // namespace
