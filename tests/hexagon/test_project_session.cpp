// slice-052a: der Sitzungs-Zustand (`ProjectSessionService` hinter dem
// `ProjectSessionPort`) — Orakel-Zeilen 1, 2, 4–9, 11, 12, 12a und 14.
//
// **Warum der Zustand aus dem VERGLEICH kommt und nicht aus Meldungen:** der
// erste Plan-Entwurf wollte ihn aus `ModelChangedPort`-Meldungen ableiten. Das
// erste Plan-Review hat belegt, dass das fuer KEINE heute erreichbare
// GUI-Mutation funktioniert — Hilfslinien, Ebenen und Materialien melden
// nichts ("kein op" ist Entscheidung 2 der Accepted-ADR-0018). Die Warnung
// waere nie ausgeloest worden, waehrend alle Orakel gruen stehen. Zeile 1
// unten ist der Regressions-Schutz dagegen: sie prueft genau die Mutation, die
// KEINEN op meldet.

#include <filesystem>
#include <stdexcept>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/guide_line.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/segment.h"

#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"
#include "hexagon/model/persisted_derivations.h"
#include "hexagon/ports/driven/project_repository_port.h"
#include "hexagon/ports/driving/project_session_port.h"
#include "hexagon/services/manage_project.h"
#include "hexagon/services/project_session.h"
#include "hexagon/services/structure_edit_service.h"

#include "analytic_geometry_double.h"

namespace {

namespace model = bcad::hexagon::model;
namespace driving = bcad::hexagon::ports::driving;
namespace fs = std::filesystem;

using bcad::hexagon::services::ProjectSessionService;

model::Building startStand() {
    model::Building b;
    b.storeys.push_back({model::StoreyId{1}, 2500.0});
    model::Wall w;
    w.id = model::WallId{1};
    w.storey_id = model::StoreyId{1};
    w.start = {0.0, 0.0};
    w.end = {4000.0, 0.0};
    w.thickness_mm = 240.0;
    w.height_mm = 2500.0;
    b.walls.push_back(w);
    model::Layer l;
    l.id = model::LayerId{1};
    l.name = "Achsen";
    b.layers.push_back(l);
    return b;
}

model::GuideLine hilfslinie(model::GuideLineId id) {
    model::GuideLine g;
    g.id = id;
    g.storey_id = model::StoreyId{1};
    g.layer_id = model::LayerId{1};
    g.segment = {{0.0, 500.0}, {4000.0, 500.0}};
    return g;
}

// §6-1 — DIE Zeile: eine Hilfslinie meldet KEINEN op. Ein Meldungs-Beobachter
// wuerde sie nie sehen; der Vergleich sieht sie.
TEST(ProjectSession, HilfslinieMachtDieSitzungUngesichert) {
    const model::Building start = startStand();
    const ProjectSessionService session(start);
    ASSERT_FALSE(session.isDirty(start)) << "frischer Stand ist sauber";

    model::Building mutiert = start;
    mutiert.guide_lines.push_back(hilfslinie(model::GuideLineId{1}));

    EXPECT_TRUE(session.isDirty(mutiert))
        << "die op-lose DRW-Mutation muss erfasst sein (Regression Lauf-1-HIGH-1)";
}

// §6-2: eine op-tragende Mutation (Wand) ebenso.
TEST(ProjectSession, WandaenderungMachtDieSitzungUngesichert) {
    const model::Building start = startStand();
    const ProjectSessionService session(start);

    model::Building mutiert = start;
    mutiert.walls.front().thickness_mm = 115.0;

    EXPECT_TRUE(session.isDirty(mutiert));
}

// §6-4/§6-5: nach erfolgreichem Speichern bzw. Oeffnen ist die Sitzung sauber.
TEST(ProjectSession, NachMarkPersistedIstDieSitzungSauber) {
    const model::Building start = startStand();
    ProjectSessionService session(start);

    model::Building mutiert = start;
    mutiert.guide_lines.push_back(hilfslinie(model::GuideLineId{1}));
    ASSERT_TRUE(session.isDirty(mutiert));

    session.markPersisted("projekt.bcad", mutiert);

    EXPECT_FALSE(session.isDirty(mutiert))
        << "der gespeicherte Stand ist der neue Vergleichs-Massstab";
}

// §6-6: **gescheitertes** Speichern laesst die Sitzung ungesichert. Der Beleg
// liegt in der Reihenfolge: `markPersisted` wird erst NACH dem Schreiben
// gerufen (`ManageProjectService`), also bleibt der Zustand hier unberuehrt.
TEST(ProjectSession, OhneMarkPersistedBleibtDieSitzungUngesichert) {
    const model::Building start = startStand();
    const ProjectSessionService session(start);

    model::Building mutiert = start;
    mutiert.walls.front().height_mm = 3000.0;

    // Kein markPersisted — genau der Zustand nach einem Schreibfehler.
    EXPECT_TRUE(session.isDirty(mutiert));
    EXPECT_EQ(session.verdictForDiscard(mutiert), driving::DiscardVerdict::AskFirst);
}

// §6-7: zurueck-geaendert auf den Dateistand ⇒ wieder sauber. Ein Einweg-Flag
// koennte das nicht — deshalb der Vergleich.
TEST(ProjectSession, ZurueckGeaendertIstWiederSauber) {
    const model::Building start = startStand();
    const ProjectSessionService session(start);

    model::Building mutiert = start;
    mutiert.walls.front().thickness_mm = 115.0;
    ASSERT_TRUE(session.isDirty(mutiert));

    mutiert.walls.front().thickness_mm = 240.0;  // zurueck

    EXPECT_FALSE(session.isDirty(mutiert))
        << "ein Einweg-Flag waere hier weiter schmutzig";
}

// §6-8 + §6-14: die Baseline ist der Stand BEI SITZUNGS-BEGINN, nicht leer.
// Zeile 14 ist hier belegt — am Service, nicht am Fenster: `MainWindow` weiss
// von Sitzung und Baseline nichts (Lauf-4-HIGH-2).
TEST(ProjectSession, BaselineIstDerStartStandNichtLeer) {
    const model::Building start = startStand();
    const ProjectSessionService session(start);

    EXPECT_EQ(session.verdictForDiscard(start), driving::DiscardVerdict::Proceed)
        << "eine frische Sitzung ueber einem aufgebauten Modell ist NICHT ungesichert";

    // Gegenprobe in Form einer zweiten Sitzung: mit leerer Baseline waere
    // derselbe Stand sofort ungesichert.
    const ProjectSessionService leer_gestartet{model::Building{}};
    EXPECT_EQ(leer_gestartet.verdictForDiscard(start),
              driving::DiscardVerdict::AskFirst)
        << "so saehe es aus, wenn die Baseline-Uebergabe fehlte";
}

// §6-9: Ziel-Wahl beim Speichern.
TEST(ProjectSession, SaveTargetFragtOhnePfadUndKenntIhnDanach) {
    ProjectSessionService session(startStand());

    EXPECT_EQ(session.saveTarget().kind, driving::SaveTargetKind::AskUser)
        << "ohne bekannten Pfad muss gefragt werden";
    EXPECT_FALSE(session.path().has_value());

    session.markPersisted("haus.bcad", startStand());

    const driving::SaveTarget target = session.saveTarget();
    EXPECT_EQ(target.kind, driving::SaveTargetKind::KnownPath);
    EXPECT_EQ(target.path, fs::path{"haus.bcad"});
    ASSERT_TRUE(session.path().has_value());
    EXPECT_EQ(*session.path(), fs::path{"haus.bcad"});
}

// §6-11: das Verdikt selbst.
TEST(ProjectSession, VerdiktProceedOderAskFirst) {
    const model::Building start = startStand();
    const ProjectSessionService session(start);

    EXPECT_EQ(session.verdictForDiscard(start), driving::DiscardVerdict::Proceed);

    model::Building mutiert = start;
    mutiert.guide_lines.push_back(hilfslinie(model::GuideLineId{1}));
    EXPECT_EQ(session.verdictForDiscard(mutiert), driving::DiscardVerdict::AskFirst);
}

// §6-12 — die schaerfste Zusage des Slice: "abbrechen" UNTERLAESST.
TEST(ProjectSession, AntwortAuswertung) {
    const ProjectSessionService session(startStand());

    EXPECT_EQ(session.evaluate(driving::DiscardAnswer::Cancel),
              driving::DiscardOutcome::Abort)
        << "abbrechen darf die ausloesende Aktion NICHT ausfuehren";
    EXPECT_EQ(session.evaluate(driving::DiscardAnswer::Discard),
              driving::DiscardOutcome::Proceed);
    EXPECT_EQ(session.evaluate(driving::DiscardAnswer::Save),
              driving::DiscardOutcome::SaveThenProceed);
}

// §6-12a: scheitert das "speichern" im Verwerf-Fluss, bleibt der Stand
// ungesichert — die Sitzung darf ihn nicht als gesichert ausweisen. Der
// Aufrufer unterlaesst daraufhin die ausloesende Aktion (main.cpp:
// `saveViaHandlerOrAsk` liefert `false`).
TEST(ProjectSession, GescheitertesSpeichernImVerwerfFlussBleibtUngesichert) {
    const model::Building start = startStand();
    const ProjectSessionService session(start);

    model::Building mutiert = start;
    mutiert.guide_lines.push_back(hilfslinie(model::GuideLineId{1}));

    ASSERT_EQ(session.evaluate(driving::DiscardAnswer::Save),
              driving::DiscardOutcome::SaveThenProceed);
    // Das Speichern scheitert -> kein markPersisted -> weiterhin ungesichert,
    // und ein erneuter Anlauf fragt wieder.
    EXPECT_TRUE(session.isDirty(mutiert));
    EXPECT_EQ(session.verdictForDiscard(mutiert), driving::DiscardVerdict::AskFirst);
}

// --- §6-6: die REIHENFOLGE, an der die Zusage haengt ----------------------
//
// "Nach GESCHEITERTEM Speichern weiter ungesichert" ist keine Eigenschaft des
// Sitzungs-Service allein — sie haengt daran, dass `ManageProjectService`
// `markPersisted` ERST NACH dem Schreiben ruft. Die erste Fassung dieser Datei
// pruefte nur den Service isoliert; die Gegenprobe "Ruecksetzen vor statt nach
// dem Schreiben" liess daraufhin ALLE Tests gruen. Genau die Luecke schliesst
// dieser Test — er fuehrt den Fehlschlag durch den echten Use-Case.

class WerfendesRepository final
    : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
public:
    mutable bool save_versucht = false;

    void save(const model::Building&, const model::PersistedDerivations&,
              const fs::path&) const override {
        save_versucht = true;
        throw std::runtime_error("E-IO-002: Zielmedium voll");
    }
    model::Building load(const fs::path&) const override { return {}; }
};

TEST(ProjectSession, GescheitertesSpeichernLaesstDieSitzungUngesichert) {
    const bcad::testing::AnalyticGeometry geometry;
    bcad::hexagon::services::StructureEditService svc(geometry);
    bcad::hexagon::services::ProjectSessionService session(svc.building());

    const WerfendesRepository repo;
    bcad::hexagon::services::ManageProjectService project(svc, repo, &session);

    // Eine Mutation -> ungesichert.
    const auto storey = svc.building().storeys.front().id;
    ASSERT_TRUE(svc.addWall(storey, model::Segment{{0.0, 0.0}, {4000.0, 0.0}})
                    .has_value());
    ASSERT_TRUE(session.isDirty(svc.building()));

    EXPECT_THROW(project.saveProject("voll.bcad"), std::runtime_error);

    EXPECT_TRUE(repo.save_versucht);
    EXPECT_TRUE(session.isDirty(svc.building()))
        << "ein GESCHEITERTES Speichern darf den Stand nicht als gesichert "
           "ausweisen — markPersisted gehoert NACH das Schreiben";
    EXPECT_FALSE(session.path().has_value())
        << "und der Pfad darf nicht gemerkt werden";
}

// Das Gegenstueck: gelingt es, ist die Sitzung sauber UND der Pfad gemerkt.
TEST(ProjectSession, GelungenesSpeichernSetztStandUndPfad) {
    class StillesRepository final
        : public bcad::hexagon::ports::driven::ProjectRepositoryPort {
    public:
        void save(const model::Building&, const model::PersistedDerivations&,
                  const fs::path&) const override {}
        model::Building load(const fs::path&) const override { return {}; }
    };

    const bcad::testing::AnalyticGeometry geometry;
    bcad::hexagon::services::StructureEditService svc(geometry);
    bcad::hexagon::services::ProjectSessionService session(svc.building());

    const StillesRepository repo;
    bcad::hexagon::services::ManageProjectService project(svc, repo, &session);

    const auto storey = svc.building().storeys.front().id;
    ASSERT_TRUE(svc.addWall(storey, model::Segment{{0.0, 0.0}, {4000.0, 0.0}})
                    .has_value());
    ASSERT_TRUE(session.isDirty(svc.building()));

    project.saveProject("haus.bcad");

    EXPECT_FALSE(session.isDirty(svc.building()));
    ASSERT_TRUE(session.path().has_value());
    EXPECT_EQ(*session.path(), fs::path{"haus.bcad"});
}

// Der Port ist ueber die Basis-Referenz bedienbar — sonst waere
// `ProjectSessionService` nur eine Klasse mit passenden Methodennamen.
TEST(ProjectSession, ServiceIstUeberDenPortBedienbar) {
    const model::Building start = startStand();
    ProjectSessionService konkret(start);
    driving::ProjectSessionPort& port = konkret;

    EXPECT_FALSE(port.isDirty(start));
    port.markPersisted("x.bcad", start);
    EXPECT_EQ(port.saveTarget().kind, driving::SaveTargetKind::KnownPath);
}

}  // namespace
