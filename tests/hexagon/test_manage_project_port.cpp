// slice-054: der `ManageProjectPort` — die seit dem Bootstrap in
// spec/architecture.md §1.1 deklarierte Ziel-Form der Projekt-Use-Cases.
//
// Gegenstand ist AUSSCHLIESSLICH die Port-Naht. Das Verhalten selbst
// (rise-Ableitung, fail-closed, Modell-Ersetzung, Zeichen-Ziel-Aufloesung) ist
// unveraendert und bleibt in `test_manage_project.cpp` auf den freien
// Funktionen belegt — die Datei ist mit diesem Slice byte-unveraendert
// geblieben, DAS ist der Invarianz-Beleg (Plan §3).
//
// Die Zusicherung hier: ein Aufrufer, der NUR `ManageProjectPort&` haelt —
// keinen `StructureEditService`, kein `ProjectRepositoryPort`, keinen
// `services`-Typ — kann oeffnen und speichern. Das ist exakt die Sicht eines
// `adapters/ui/command/`-Handlers, dem `.a-check.yml` die Kante nach
// `services`/`ports_driven` verbietet. Den ARCHITEKTUR-Beleg dafuer fuehrt
// slice-053 (a-check sieht erst dort eine reale ui_command-Datei am Port);
// hier wird die VERTRAGS-Seite belegt: der Port ist aus dieser Sicht bedienbar.

#include <filesystem>
#include <stdexcept>
#include <utility>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/persisted_derivations.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/project_repository_port.h"
#include "hexagon/ports/driving/manage_project_port.h"
#include "hexagon/services/geometry/stair_geometry.h"
#include "hexagon/services/manage_project.h"
#include "hexagon/services/project_session.h"
#include "hexagon/services/structure_edit_service.h"

#include "analytic_geometry_double.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driving = bcad::hexagon::ports::driving;
namespace fs = std::filesystem;

using bcad::testing::AnalyticGeometry;

// Repository-Double: liefert ein frei waehlbares Projekt und haelt fest, WAS
// gespeichert wurde (Building + abgeleitetes Buendel).
class RecordingRepository final
    : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
public:
    explicit RecordingRepository(model::Building fixture)
        : fixture_(std::move(fixture)) {}

    mutable bool save_called = false;
    mutable model::Building last_saved;
    mutable model::PersistedDerivations last_derived;
    mutable fs::path last_path;

    void save(const model::Building& building,
              const model::PersistedDerivations& derived,
              const fs::path& path) const override {
        save_called = true;
        last_saved = building;
        last_derived = derived;
        last_path = path;
    }

    model::Building load(const fs::path& /*path*/) const override {
        if (fail_load) {
            throw std::runtime_error("E-IO-001: Datei nicht lesbar");
        }
        return fixture_;
    }

    bool fail_load = false;

private:
    model::Building fixture_;
};

// Projekt mit Geschoss, Ebene und einer Treppe — die Treppe deckt die
// rise-Ableitung ab, die beim Speichern entsteht.
model::Building projectWithStair(model::StoreyId stair_from) {
    model::Building b;
    model::Storey s;
    s.id = model::StoreyId{7};
    s.height_mm = 2500.0;
    b.storeys.push_back(s);

    model::Layer l;
    l.id = model::LayerId{9};
    l.name = "Achsen";
    b.layers.push_back(l);

    model::Stair stair;
    stair.id = model::StairId{3};
    stair.from_storey_id = stair_from;
    stair.to_storey_id = model::StoreyId{8};
    stair.type = model::StairType::Gerade;
    stair.start = model::Point2D{0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 10;
    stair.tread_mm = 280.0;
    b.stairs.push_back(stair);
    return b;
}

// DIE Nagelprobe des Slice: ein Aufrufer in Adapter-Sicht. Er sieht nur den
// Port — kein `services::`, kein `ports::driven::`. Kompiliert diese Funktion
// nicht mehr, ist der Port-Zuschnitt kaputt.
driving::DrawingTargetResolution openThenSaveThroughPortOnly(
    driving::ManageProjectPort& port, const fs::path& open_path,
    const fs::path& save_path, const driving::DrawingTargetSinks& sinks) {
    const driving::DrawingTargetResolution resolution =
        port.openProject(open_path, sinks);
    port.saveProject(save_path);
    return resolution;
}

// §3-1: allein aus der Port-Sicht oeffnen UND speichern.
TEST(ManageProjectPort, PortSightAloneOpensAndSaves) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);  // ohne Sitzung

    const auto resolution =
        openThenSaveThroughPortOnly(port, "quelle.bcad", "ziel.bcad", {});

    EXPECT_EQ(resolution, driving::DrawingTargetResolution::Resolved);
    // Geoeffnet: der Sitzungs-Stand traegt das geladene Projekt.
    ASSERT_EQ(svc.building().storeys.size(), 1U);
    EXPECT_EQ(svc.building().storeys.front().id, model::StoreyId{7});
    // Gespeichert: unter dem gereichten Pfad, mit abgeleiteter rise.
    EXPECT_TRUE(repo.save_called);
    EXPECT_EQ(repo.last_path, fs::path{"ziel.bcad"});
    ASSERT_EQ(repo.last_derived.stairRiseMm.count(model::StairId{3}), 1U);
    EXPECT_DOUBLE_EQ(
        repo.last_derived.stairRiseMm.at(model::StairId{3}),
        services::stairRiseMm(svc.building().stairs.front(), 2500.0));
}

// §3-3: der Port speichert den Stand des SITZUNGS-Service — nicht ein von
// aussen gereichtes Modell. Genau deshalb braucht ein ui_command-Handler
// keinen `StructureEditService`. Diskriminierend: wuerde die Naht ein leeres
// oder fremdes Building speichern, faellt die Wand-Zusicherung.
TEST(ManageProjectPort, PortSaveWritesSessionStateNotAPassedModel) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);  // ohne Sitzung

    port.openProject("quelle.bcad", {});  // ohne Sichten: sichtbar leer (slice-053 §1.1)
    // Nach dem Oeffnen mutiert die Sitzung — der Port muss DIESEN Stand
    // schreiben, nicht den geladenen.
    const auto wall = svc.addWall(model::StoreyId{7},
                                  model::Segment{{0.0, 0.0}, {4000.0, 0.0}});
    ASSERT_TRUE(wall.has_value());

    port.saveProject("ziel.bcad");

    ASSERT_TRUE(repo.save_called);
    ASSERT_EQ(repo.last_saved.walls.size(), 1U)
        << "der Port speichert den Sitzungs-Stand, nicht den geladenen";
    EXPECT_EQ(repo.last_saved.walls.front().id, *wall);
}

// §3-3, zweite Haelfte: fail-closed bleibt fail-closed — auch ueber den Port.
TEST(ManageProjectPort, PortSaveIsFailClosedOnDanglingFromStorey) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    // Treppe zeigt auf ein Geschoss, das es nicht gibt.
    const RecordingRepository repo{projectWithStair(model::StoreyId{99})};
    services::ManageProjectService port(svc, repo, nullptr);  // ohne Sitzung

    port.openProject("quelle.bcad", {});  // ohne Sichten: sichtbar leer (slice-053 §1.1)

    EXPECT_THROW(port.saveProject("ziel.bcad"), std::runtime_error);
    EXPECT_FALSE(repo.save_called)
        << "fail-closed: kein Teil-Speichern ueber den Port";
}

// §3-2: die Zeichen-Ziel-Aufloesung gilt ueber den Port unveraendert weiter,
// und die Senken bleiben METHODEN-Parameter (Plan §2.1, R2) — der Adapter
// reicht sie durch, der Port haelt sie nicht.
TEST(ManageProjectPort, PortOpenResolvesDrawingTargetThroughMethodSinks) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);  // ohne Sitzung

    auto seen_storey = model::StoreyId{1};
    auto target_storey = model::StoreyId{1};
    auto target_layer = model::LayerId{1};
    const driving::DrawingTargetSinks sinks{
        [&seen_storey](model::StoreyId s) { seen_storey = s; },
        [&target_storey, &target_layer](model::StoreyId s, model::LayerId l) {
            target_storey = s;
            target_layer = l;
        },
    };

    const auto resolution = port.openProject("quelle.bcad", sinks);

    EXPECT_EQ(resolution, driving::DrawingTargetResolution::Resolved);
    EXPECT_EQ(seen_storey, model::StoreyId{7});
    EXPECT_EQ(target_storey, model::StoreyId{7});
    EXPECT_EQ(target_layer, model::LayerId{9});
}

// §3-4: Fehler kommen neutral durch — kein Framework-Typ im Vertrag, und der
// Sitzungs-Stand bleibt unberuehrt (erst laden, dann ersetzen).
TEST(ManageProjectPort, PortOpenPropagatesNeutralErrorAndKeepsState) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);  // ohne Sitzung

    const auto storey_before = svc.addStorey(3000.0);
    const auto storeys_before = svc.building().storeys.size();
    repo.fail_load = true;

    EXPECT_THROW((void)port.openProject("fehlt.bcad", {}), std::runtime_error);

    ASSERT_EQ(svc.building().storeys.size(), storeys_before)
        << "gescheitertes Oeffnen laesst den bisherigen Stand unveraendert";
    EXPECT_EQ(svc.building().storeys.back().id, storey_before)
        << "der zuletzt angelegte Stand steht unveraendert da";
}

// Der Port ist ueber die Basis-Referenz benutzbar (polymorph) — ohne diese
// Zusicherung waere `ManageProjectService` nur eine Klasse mit passenden
// Methodennamen.
TEST(ManageProjectPort, ServiceIsUsableThroughThePortReference) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService concrete(svc, repo, nullptr);

    driving::ManageProjectPort& port = concrete;
    EXPECT_EQ(port.openProject("quelle.bcad", {}),
              driving::DrawingTargetResolution::Resolved);
}

// --- slice-052b: "Neues Projekt" (LH-FA-BLD-001) -------------------------

// §6-3: das neue Projekt hat GENAU EIN Geschoss mit der Hoehe aus der
// Spezifikation. Der erwartete Wert steht hier als EIGENES LITERAL — laese der
// Test dieselbe Konstante wie die Produktion, wanderten beide gemeinsam und die
// Gegenprobe "Konstante geaendert" bliebe gruen (Plan-Review Lauf 2, MEDIUM-1).
// 2500.0 ist der Wert aus spec/spezifikation.md §3.
TEST(ManageProjectPort, NeuesProjektHatEinGeschossMitSpezifikationsHoehe) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);

    port.newProject({});

    ASSERT_EQ(svc.building().storeys.size(), 1U);
    EXPECT_DOUBLE_EQ(svc.building().storeys.front().height_mm, 2500.0)
        << "Default-Geschosshoehe der Spezifikation (§3)";
}

// §6-8: die L3-Entscheidung ist beobachtbar umgesetzt — das neue Projekt hat
// eine Zeichen-Ebene, sonst waere die einzige heute erreichbare
// Benutzer-Mutation (Hilfslinie) unmoeglich.
TEST(ManageProjectPort, NeuesProjektHatEineZeichenEbene) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);

    const auto resolution = port.newProject({});

    ASSERT_EQ(svc.building().layers.size(), 1U)
        << "ohne Ebene waere das neue Projekt eine Sackgasse (L3)";
    EXPECT_EQ(resolution, driving::DrawingTargetResolution::Resolved)
        << "Geschoss UND Ebene sind da -> das Zeichen-Ziel ist vollstaendig";
}

// Und es ist wirklich LEER bis auf diese beiden — kein Rest des Vorgaengers.
TEST(ManageProjectPort, NeuesProjektTraegtNichtsVomVorgaenger) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);

    port.openProject("quelle.bcad", {});
    ASSERT_FALSE(svc.building().stairs.empty()) << "Vorgaenger hat eine Treppe";

    port.newProject({});

    EXPECT_TRUE(svc.building().stairs.empty());
    EXPECT_TRUE(svc.building().walls.empty());
    EXPECT_TRUE(svc.building().guide_lines.empty());
}

// §6-4: nach "Neu" zeigt das Zeichen-Ziel auf das NEUE Geschoss/die neue Ebene
// — dieselbe B4-Klasse wie beim Oeffnen (slice-047-Verify).
TEST(ManageProjectPort, NeuesProjektLoestDasZeichenZielNeuAuf) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ManageProjectService port(svc, repo, nullptr);

    port.openProject("quelle.bcad", {});  // Ziel steht auf Storey 7 / Layer 9

    auto seen_storey = model::StoreyId{7};
    auto target_storey = model::StoreyId{7};
    auto target_layer = model::LayerId{9};
    const driving::DrawingTargetSinks sinks{
        [&seen_storey](model::StoreyId s) { seen_storey = s; },
        [&target_storey, &target_layer](model::StoreyId s, model::LayerId l) {
            target_storey = s;
            target_layer = l;
        },
    };

    port.newProject(sinks);

    const auto neues_geschoss = svc.building().storeys.front().id;
    const auto neue_ebene = svc.building().layers.front().id;
    EXPECT_EQ(seen_storey, neues_geschoss);
    EXPECT_EQ(target_storey, neues_geschoss);
    EXPECT_EQ(target_layer, neue_ebene)
        << "sonst zeigten die Sichten auf einen Stand, den es nicht mehr gibt";
}

// §6-1 + §6-2 — DER Datenverlust-Pfad (L1): nach "Neu" ist KEINE Datei mehr
// gemerkt, und die Sitzung ist sauber. Ohne den Reset schriebe ein
// anschliessendes "Speichern" dialoglos in das ZUVOR GEOEFFNETE Projekt.
TEST(ManageProjectPort, NeuesProjektVergisstDieGemerkteDateiUndIstSauber) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);
    const RecordingRepository repo{projectWithStair(model::StoreyId{7})};
    services::ProjectSessionService session(svc.building());
    services::ManageProjectService port(svc, repo, &session);

    port.openProject("alt.bcad", {});
    ASSERT_EQ(session.saveTarget().kind, driving::SaveTargetKind::KnownPath)
        << "nach dem Oeffnen ist die Datei gemerkt";

    port.newProject({});

    EXPECT_EQ(session.saveTarget().kind, driving::SaveTargetKind::AskUser)
        << "nach Neu muss das Ziel wieder erfragt werden (L1)";
    EXPECT_FALSE(session.path().has_value());
    EXPECT_FALSE(session.isDirty(svc.building()))
        << "die Baseline ist das neue Projekt";
}

}  // namespace
