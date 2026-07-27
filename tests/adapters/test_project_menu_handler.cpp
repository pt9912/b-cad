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
#include <stdexcept>

#include <gtest/gtest.h>

#include "adapters/ui/command/project_menu_handler.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driving/manage_project_port.h"

namespace {

namespace driving = bcad::hexagon::ports::driving;
namespace model = bcad::hexagon::model;
namespace fs = std::filesystem;

using bcad::adapters::ui::command::ProjectMenuHandler;

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
    ProjectMenuHandler handler(port, {});

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
    ProjectMenuHandler handler(port, {});

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

    ProjectMenuHandler handler(
        port, driving::DrawingTargetSinks{
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
    ProjectMenuHandler handler(
        port, driving::DrawingTargetSinks{
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
    ProjectMenuHandler handler(port, {});

    port.fail_open = true;
    EXPECT_THROW((void)handler.open("fehlt.bcad"), std::runtime_error);

    port.fail_save = true;
    EXPECT_THROW(handler.saveAs("ziel.bcad"), std::runtime_error);
}

}  // namespace
