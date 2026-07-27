// slice-053: der `ui/command/`-Handler am `ManageProjectPort` — Orakel-Zeilen
// 5 und 6.
//
// Zeile 5: der Handler ruft fuer "Oeffnen"/"Speichern" den Port (slice-054).
// Diese Datei ist zugleich der ARCHITEKTUR-Beleg, den slice-054 bewusst offen
// liess: mit ihr existiert erstmals eine reale `ui_command`-Datei, die den
// Driving Port ruft — `make a-check` sieht die Kante ui_command -> ports_driving
// jetzt an einem Artefakt, nicht nur in der Konfiguration.
//
// Zeile 6 ist die wichtigere: der Handler REICHT DIE SENKEN DURCH. Ohne sie
// zeigt das Zeichen-Ziel nach einem Projekt-Laden weiter auf Ids des ALTEN
// Modells (slice-047-Verify-B4). Der Port hat dafuer keinen Default mehr, also
// faengt der Compiler das Vergessen — aber NICHT das Durchreichen leerer
// Senken. Genau das prueft `HandlerForwardsTheDrawingTargetSinks`.

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <utility>

#include <gtest/gtest.h>

#include "adapters/ui/command/project_menu_handler.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driving/manage_project_port.h"
#include "hexagon/ports/driving/project_session_port.h"
#include "hexagon/model/building.h"

namespace {

namespace driving = bcad::hexagon::ports::driving;
namespace model = bcad::hexagon::model;
namespace fs = std::filesystem;

using bcad::adapters::ui::command::ProjectMenuHandler;

// slice-052a: Sitzungs-Doppel. Haelt fest, WAS der Handler gefragt hat — der
// Handler darf keine dieser Entscheidungen selbst bilden.
class RecordingSession final : public driving::ProjectSessionPort {
public:
    mutable int verdict_calls = 0;
    mutable int save_target_calls = 0;
    mutable int evaluate_calls = 0;
    int mark_calls = 0;
    driving::DiscardVerdict verdict{driving::DiscardVerdict::Proceed};
    driving::SaveTarget target{driving::SaveTargetKind::AskUser, {}};
    driving::DiscardOutcome outcome{driving::DiscardOutcome::Abort};

    bool isDirty(const model::Building&) const override { return dirty; }
    driving::DiscardVerdict verdictForDiscard(
        const model::Building&) const override {
        ++verdict_calls;
        return verdict;
    }
    driving::SaveTarget saveTarget() const override {
        ++save_target_calls;
        return target;
    }
    driving::DiscardOutcome evaluate(driving::DiscardAnswer) const override {
        ++evaluate_calls;
        return outcome;
    }
    void markPersisted(const fs::path&, const model::Building&) override {
        ++mark_calls;
    }
    std::optional<fs::path> path() const override { return std::nullopt; }

    bool dirty = false;
};

// Der "aktuelle Stand" als port-freie Naht — im Produkt der
// StructureEditService, den ein ui_command-Adapter nicht sehen darf.
const model::Building& leeresGebaeude() {
    static const model::Building leer;
    return leer;
}

ProjectMenuHandler baueHandler(driving::ManageProjectPort& port,
                               driving::ProjectSessionPort& session,
                               driving::DrawingTargetSinks sinks = {}) {
    return ProjectMenuHandler(port, session, []() -> const model::Building& {
        return leeresGebaeude();
    }, std::move(sinks));
}

// Port-Doppel: haelt fest, WOMIT gerufen wurde — insbesondere, ob die Senken
// gesetzt ankamen, und meldet ueber sie die Ids eines "geladenen" Stands.
class RecordingPort final : public driving::ManageProjectPort {
public:
    fs::path opened_path;
    fs::path saved_path;
    int open_calls = 0;
    int save_calls = 0;
    bool sinks_had_active_storey = false;
    bool sinks_had_draw_target = false;
    bool fail_open = false;

    driving::DrawingTargetResolution openProject(
        const fs::path& path, const driving::DrawingTargetSinks& sinks) override {
        ++open_calls;
        opened_path = path;
        if (fail_open) {
            throw std::runtime_error("E-IO-001: Datei nicht lesbar");
        }
        sinks_had_active_storey = static_cast<bool>(sinks.set_active_storey);
        sinks_had_draw_target = static_cast<bool>(sinks.set_draw_target);
        // Wie der echte Use-Case: die Ids des GELADENEN Stands melden.
        if (sinks.set_active_storey) {
            sinks.set_active_storey(model::StoreyId{7});
        }
        if (sinks.set_draw_target) {
            sinks.set_draw_target(model::StoreyId{7}, model::LayerId{9});
        }
        return driving::DrawingTargetResolution::Resolved;
    }

    void saveProject(const fs::path& path) override {
        ++save_calls;
        saved_path = path;
        if (fail_save) {
            throw std::runtime_error("E-IO-002: nicht schreibbar");
        }
    }

    bool fail_save = false;
};

// §3-5: "Oeffnen" geht ueber den Port, mit dem gereichten Pfad.
TEST(ProjectMenuHandler, OpenCallsThePortWithTheGivenPath) {
    RecordingPort port;
    RecordingSession session;
    ProjectMenuHandler handler = baueHandler(port, session);

    const auto resolution = handler.open("projekt.bcad");

    EXPECT_EQ(port.open_calls, 1);
    EXPECT_EQ(port.opened_path, fs::path{"projekt.bcad"});
    EXPECT_EQ(resolution, driving::DrawingTargetResolution::Resolved);
    EXPECT_EQ(port.save_calls, 0) << "Oeffnen speichert nicht";
}

// §3-5: "Speichern unter" ebenso — und OHNE `Building`, weil der Port es aus
// dem Sitzungs-Service bezieht (slice-054). Genau deshalb kommt dieser
// Adapter ohne `StructureEditService` aus.
TEST(ProjectMenuHandler, SaveAsCallsThePortWithTheGivenPath) {
    RecordingPort port;
    RecordingSession session;
    ProjectMenuHandler handler = baueHandler(port, session);

    handler.saveAs("ziel.bcad");

    EXPECT_EQ(port.save_calls, 1);
    EXPECT_EQ(port.saved_path, fs::path{"ziel.bcad"});
    EXPECT_EQ(port.open_calls, 0);
}

// §3-6 — DIE Zeile dieses Slice: die gehaltenen Senken kommen beim Port an und
// werden mit den Ids des geladenen Stands gerufen.
//
// Diskriminierend: reicht der Handler `{}` statt `sinks_` durch, sind beide
// Callables leer, keine Id wird gemeldet — und alle vier EXPECTs fallen. Das
// blosse WEGLASSEN faengt seit slice-053 §1.1 bereits der Compiler (der Port
// hat keinen Default mehr); diese Zeile faengt den Fall, den er nicht sieht.
TEST(ProjectMenuHandler, HandlerForwardsTheDrawingTargetSinks) {
    RecordingPort port;
    auto seen_storey = model::StoreyId{1};
    auto target_storey = model::StoreyId{1};
    auto target_layer = model::LayerId{1};

    RecordingSession session;
    ProjectMenuHandler handler = baueHandler(
        port, session,
        driving::DrawingTargetSinks{
            [&seen_storey](model::StoreyId s) { seen_storey = s; },
            [&target_storey, &target_layer](model::StoreyId s,
                                            model::LayerId l) {
                target_storey = s;
                target_layer = l;
            },
        });

    handler.open("projekt.bcad");

    EXPECT_TRUE(port.sinks_had_active_storey)
        << "der Handler reicht die Geschoss-Senke durch, nicht {}";
    EXPECT_TRUE(port.sinks_had_draw_target)
        << "der Handler reicht die Zeichen-Ziel-Senke durch, nicht {}";
    EXPECT_EQ(seen_storey, model::StoreyId{7})
        << "die Sicht bekommt die Id des GELADENEN Stands (slice-047-B4)";
    EXPECT_EQ(target_storey, model::StoreyId{7});
    EXPECT_EQ(target_layer, model::LayerId{9});
}

// Die Senken werden bei JEDEM Oeffnen gereicht, nicht nur beim ersten — ein
// Handler, der sie nach dem ersten Aufruf "verbraucht" (z. B. per std::move),
// waere sonst unauffaellig.
TEST(ProjectMenuHandler, SinksAreForwardedOnEveryOpen) {
    RecordingPort port;
    int calls = 0;
    RecordingSession session;
    ProjectMenuHandler handler = baueHandler(
        port, session,
        driving::DrawingTargetSinks{
            [&calls](model::StoreyId) { ++calls; },
            [](model::StoreyId, model::LayerId) {},
        });

    handler.open("a.bcad");
    handler.open("b.bcad");

    EXPECT_EQ(calls, 2);
    EXPECT_TRUE(port.sinks_had_active_storey);
}

// Fehler kommen neutral durch — der Aufrufer (Composition-Root) zeigt sie an;
// der Handler kennt kein Qt und schluckt nichts.
TEST(ProjectMenuHandler, ErrorsPropagateNeutrally) {
    RecordingPort port;
    RecordingSession session;
    ProjectMenuHandler handler = baueHandler(port, session);

    port.fail_open = true;
    EXPECT_THROW((void)handler.open("fehlt.bcad"), std::runtime_error);

    port.fail_save = true;
    EXPECT_THROW(handler.saveAs("ziel.bcad"), std::runtime_error);
}

// --- slice-052a, Orakel-Zeile 15 -----------------------------------------
//
// Der Handler HOLT Verdikt, Ziel-Wahl und Antwort-Auswertung aus dem Kern — er
// bildet sie nicht. Genau das war der HIGH des vierten Plan-Reviews: ohne Port
// duerfte nur main.cpp diese Abfragen rufen, und dort sieht sie kein Sensor.

TEST(ProjectMenuHandler, VerdictKommtAusDemSitzungsPort) {
    RecordingPort port;
    RecordingSession session;
    session.verdict = driving::DiscardVerdict::AskFirst;
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_EQ(handler.verdictForDiscard(), driving::DiscardVerdict::AskFirst);
    EXPECT_EQ(session.verdict_calls, 1) << "der Handler fragt den Port";

    session.verdict = driving::DiscardVerdict::Proceed;
    EXPECT_EQ(handler.verdictForDiscard(), driving::DiscardVerdict::Proceed)
        << "der Handler haelt kein eigenes Verdikt";
}

TEST(ProjectMenuHandler, AntwortAuswertungKommtAusDemSitzungsPort) {
    RecordingPort port;
    RecordingSession session;
    session.outcome = driving::DiscardOutcome::Abort;
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_EQ(handler.evaluate(driving::DiscardAnswer::Cancel),
              driving::DiscardOutcome::Abort);
    EXPECT_EQ(session.evaluate_calls, 1);

    session.outcome = driving::DiscardOutcome::Proceed;
    EXPECT_EQ(handler.evaluate(driving::DiscardAnswer::Discard),
              driving::DiscardOutcome::Proceed)
        << "die Auswertung faellt im Kern, nicht im Adapter";
}

// §6-9/§6-10 an der Handler-Naht: "Speichern" schreibt in die GEMERKTE Datei.
TEST(ProjectMenuHandler, SaveSchreibtInDieGemerkteDatei) {
    RecordingPort port;
    RecordingSession session;
    session.target = {driving::SaveTargetKind::KnownPath, fs::path{"haus.bcad"}};
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_TRUE(handler.save());
    EXPECT_EQ(session.save_target_calls, 1) << "die Ziel-WAHL faellt im Kern";
    EXPECT_EQ(port.save_calls, 1);
    EXPECT_EQ(port.saved_path, fs::path{"haus.bcad"});
}

// Ohne bekannten Pfad speichert der Handler NICHT ins Blaue — er meldet es,
// und der Composition-Root fragt das Ziel ab.
TEST(ProjectMenuHandler, SaveOhneBekanntenPfadSchreibtNichts) {
    RecordingPort port;
    RecordingSession session;
    session.target = {driving::SaveTargetKind::AskUser, {}};
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_FALSE(handler.save());
    EXPECT_EQ(port.save_calls, 0)
        << "ohne Ziel darf kein Schreibvorgang laufen";
}

// --- slice-052a, §6-12 / §6-12a: die KOMPOSITION -------------------------
//
// `mayDiscard` ist der Ort, an dem aus Verdikt + Antwort eine Handlung wird.
// Er liegt bewusst im Handler und nicht in `main.cpp` — dort waere er
// sensorlos. `main` reicht nur die zwei Dialoge herein.

TEST(ProjectMenuHandler, SauberesProjektFragtGarNicht) {
    RecordingPort port;
    RecordingSession session;
    session.verdict = driving::DiscardVerdict::Proceed;
    ProjectMenuHandler handler = baueHandler(port, session);

    int gefragt = 0;
    EXPECT_TRUE(handler.mayDiscard(
        [&gefragt]() {
            ++gefragt;
            return driving::DiscardAnswer::Cancel;
        },
        []() { return std::optional<fs::path>{}; }));
    EXPECT_EQ(gefragt, 0) << "ohne ungesicherten Stand keine Rueckfrage";
}

// DIE schaerfste Zusage: "abbrechen" unterlaesst die ausloesende Aktion.
TEST(ProjectMenuHandler, AbbrechenUnterlaesstDieAktion) {
    RecordingPort port;
    RecordingSession session;
    session.verdict = driving::DiscardVerdict::AskFirst;
    session.outcome = driving::DiscardOutcome::Abort;
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_FALSE(handler.mayDiscard(
        []() { return driving::DiscardAnswer::Cancel; },
        []() { return std::optional<fs::path>{}; }));
    EXPECT_EQ(port.save_calls, 0) << "abbrechen speichert auch nicht";
}

TEST(ProjectMenuHandler, VerwerfenFuehrtAusOhneZuSpeichern) {
    RecordingPort port;
    RecordingSession session;
    session.verdict = driving::DiscardVerdict::AskFirst;
    session.outcome = driving::DiscardOutcome::Proceed;
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_TRUE(handler.mayDiscard(
        []() { return driving::DiscardAnswer::Discard; },
        []() { return std::optional<fs::path>{}; }));
    EXPECT_EQ(port.save_calls, 0);
}

TEST(ProjectMenuHandler, SpeichernImVerwerfFlussSchreibtDannFuehrtAus) {
    RecordingPort port;
    RecordingSession session;
    session.verdict = driving::DiscardVerdict::AskFirst;
    session.outcome = driving::DiscardOutcome::SaveThenProceed;
    session.target = {driving::SaveTargetKind::KnownPath, fs::path{"haus.bcad"}};
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_TRUE(handler.mayDiscard(
        []() { return driving::DiscardAnswer::Save; },
        []() { return std::optional<fs::path>{}; }));
    EXPECT_EQ(port.save_calls, 1);
    EXPECT_EQ(port.saved_path, fs::path{"haus.bcad"});
}

// §6-12a: scheitert das Speichern im Verwerf-Fluss, UNTERBLEIBT die
// ausloesende Aktion. Der Fehler wird nicht geschluckt-und-trotzdem-gemacht.
TEST(ProjectMenuHandler, GescheitertesSpeichernUnterlaesstDieAktion) {
    RecordingPort port;
    port.fail_save = true;
    RecordingSession session;
    session.verdict = driving::DiscardVerdict::AskFirst;
    session.outcome = driving::DiscardOutcome::SaveThenProceed;
    session.target = {driving::SaveTargetKind::KnownPath, fs::path{"haus.bcad"}};
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_FALSE(handler.mayDiscard(
        []() { return driving::DiscardAnswer::Save; },
        []() { return std::optional<fs::path>{}; }))
        << "Schreibfehler ⇒ die ausloesende Aktion unterbleibt (Orakel 12a)";
}

// Und ebenso, wenn die ZIEL-Abfrage abgebrochen wird.
TEST(ProjectMenuHandler, AbgebrocheneZielAbfrageUnterlaesstDieAktion) {
    RecordingPort port;
    RecordingSession session;
    session.verdict = driving::DiscardVerdict::AskFirst;
    session.outcome = driving::DiscardOutcome::SaveThenProceed;
    session.target = {driving::SaveTargetKind::AskUser, {}};
    ProjectMenuHandler handler = baueHandler(port, session);

    EXPECT_FALSE(handler.mayDiscard(
        []() { return driving::DiscardAnswer::Save; },
        []() { return std::optional<fs::path>{}; }));
    EXPECT_EQ(port.save_calls, 0);
}

}  // namespace
