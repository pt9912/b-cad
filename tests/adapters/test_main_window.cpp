// slice-053: das Hauptfenster als Adapter-Klasse — Orakel-Zeilen 1–4.
//
// Bis hierher lebte das Fenster als lokale Variable im coverage-ausgenommenen
// `src/main.cpp`, das in KEIN Testbinary gelinkt ist. Menue-Ausloesung und
// Schliess-Ereignis hatten damit ueberhaupt keinen Sensor — slice-052a
// scheiterte genau daran (Plan-Review Lauf 3, HIGH-1). Diese Datei ist die
// ERSTE Deckung dieses Codes; einen Vorher-Sensor gibt es nicht (Plan §3).
//
// Qt-Ereignisse headless: Xvfb wie bei test_canvas_widget/test_viewer_widget
// (ADR-0010). Das Veto wird ueber `close()` geprueft, NICHT ueber ein per
// `sendEvent` zugestelltes QCloseEvent — `ignore()` wirkt an `close()`
// (Plan-Review Lauf 1, MEDIUM-2).

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

#include <gtest/gtest.h>

#include <filesystem>
#include <optional>

#include "adapters/ui/command/project_menu_handler.h"
#include "adapters/ui/command/wall_param_sink.h"
#include "adapters/ui/view/main_window.h"
#include "hexagon/model/building.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall_params.h"
#include "hexagon/ports/driving/manage_project_port.h"
#include "hexagon/services/project_session.h"

namespace {

namespace model = bcad::hexagon::model;

using bcad::adapters::ui::view::MainWindow;

// Minimal-Doppel: der Schliess-Weg braucht keinen echten Projekt-Port.
class StubManageProject final
    : public bcad::hexagon::ports::driving::ManageProjectPort {
public:
    bcad::hexagon::ports::driving::DrawingTargetResolution openProject(
        const std::filesystem::path&,
        const bcad::hexagon::ports::driving::DrawingTargetSinks&) override {
        return bcad::hexagon::ports::driving::DrawingTargetResolution::Resolved;
    }
    bcad::hexagon::ports::driving::DrawingTargetResolution newProject(
        const bcad::hexagon::ports::driving::DrawingTargetSinks&) override {
        return bcad::hexagon::ports::driving::DrawingTargetResolution::Resolved;
    }
    void saveProject(const std::filesystem::path&) override {}
};

// Qt erlaubt nur EINE QApplication pro Prozess; `gtest_discover_tests` startet
// jeden Test als eigenen ctest-Prozess, also baut jeder Test seine eigene
// (Muster test_canvas_widget.cpp).
class QtFixture {
public:
    QtFixture() : app_(argc_, static_cast<char**>(argv_)) {}

private:
    int argc_ = 1;
    char arg0_[19] = "bcad_adapter_tests";
    char* argv_[2] = {static_cast<char*>(arg0_), nullptr};
    QApplication app_;
};

QAction* actionNamed(MainWindow& window, const char* name) {
    return window.findChild<QAction*>(QString::fromLatin1(name));
}

// §3-1: headless konstruierbar — Voraussetzung aller weiteren Zeilen.
TEST(MainWindow, IsConstructibleHeadlessWithCentralWidget) {
    const QtFixture qt;
    auto* central = new QLabel(QStringLiteral("zentral"));
    MainWindow window(central, {}, {}, {}, {});

    // slice-059b: das Fenster setzt das gereichte Widget nicht mehr DIREKT als
    // Zentral-Widget — es baut eine Spalte aus Sicht und Eigenschaften-Bereich
    // (ADR-0021 E4: ein fester, nicht-modaler Bereich). Die Zusage war nie
    // "es IST das Zentral-Widget", sondern "das Fenster uebernimmt es": es
    // gehoert zum Fenster und wird angezeigt. Genau das wird jetzt geprueft —
    // liesse das Fenster es fallen, waere `window()` nicht dieses Fenster.
    EXPECT_EQ(central->window(), &window)
        << "das Fenster uebernimmt das gereichte Widget (Qt-Ownership)";
    EXPECT_NE(window.centralWidget(), nullptr);
    // Die beiden Menue-Aktionen existieren, auch ohne verdrahtete Handler.
    EXPECT_NE(actionNamed(window, MainWindow::kOpenActionName), nullptr);
    EXPECT_NE(actionNamed(window, MainWindow::kNewActionName), nullptr);
    EXPECT_NE(actionNamed(window, MainWindow::kSaveActionName), nullptr);
    EXPECT_NE(actionNamed(window, MainWindow::kSaveAsActionName), nullptr);
}

// §3-4: eine Menue-Aktion ist ausloesbar und ruft ihre injizierte Funktion.
TEST(MainWindow, TriggeringMenuActionsCallsTheInjectedHandlers) {
    const QtFixture qt;
    int opened = 0;
    int saved = 0;
    int saved_as = 0;
    int created = 0;
    QWidget* open_parent = nullptr;
    MainWindow window(nullptr,
                      {[&created](QWidget*) { ++created; },
                       [&opened, &open_parent](QWidget* parent) {
                           ++opened;
                           open_parent = parent;
                       },
                       [&saved](QWidget*) { ++saved; },
                       [&saved_as](QWidget*) { ++saved_as; }},
                      {}, {}, {});

    // slice-052b: "Neu" ist eine EIGENE Aktion — eine Vertauschung mit
    // "Oeffnen" waere ein Datenverlust ohne Datei-Dialog.
    actionNamed(window, MainWindow::kNewActionName)->trigger();
    EXPECT_EQ(created, 1);
    EXPECT_EQ(opened, 0);

    actionNamed(window, MainWindow::kOpenActionName)->trigger();
    EXPECT_EQ(opened, 1);
    EXPECT_EQ(saved, 0) << "die Aktionen sind nicht vertauscht";

    // slice-052a: "Speichern" (bekannte Datei) und "Speichern unter..." sind
    // ZWEI Aktionen — eine Vertauschung waere ein stiller Datenverlust.
    actionNamed(window, MainWindow::kSaveActionName)->trigger();
    EXPECT_EQ(saved, 1);
    EXPECT_EQ(saved_as, 0);

    actionNamed(window, MainWindow::kSaveAsActionName)->trigger();
    EXPECT_EQ(opened, 1);
    EXPECT_EQ(saved, 1);
    EXPECT_EQ(saved_as, 1);

    // Das Fenster reicht sich SELBST als Dialog-Eltern durch — sonst haetten
    // die modalen Dialoge im Composition-Root kein Eltern-Fenster.
    EXPECT_EQ(open_parent, &window);
}

// §3-2: `close()` fuehrt die Schliess-Behandlung aus — der injizierte Haken
// wird gerufen. Ohne Traeger-Klasse gab es dafuer keinen Ort.
TEST(MainWindow, CloseRunsTheCloseGuard) {
    const QtFixture qt;
    int asked = 0;
    MainWindow window(nullptr, {}, {}, {}, [&asked]() {
        ++asked;
        return true;
    });
    window.show();

    EXPECT_TRUE(window.close());
    EXPECT_EQ(asked, 1) << "close() muss den Waechter fragen";
}

// §3-3: sagt der Haken "nicht schliessen", bleibt das Fenster SICHTBAR — die
// Naht, an der slice-052a sein "abbrechen" aufhaengt.
TEST(MainWindow, CloseGuardVetoKeepsTheWindowVisible) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, []() { return false; });
    window.show();
    ASSERT_TRUE(window.isVisible());

    EXPECT_FALSE(window.close()) << "das Veto verhindert das Schliessen";
    EXPECT_TRUE(window.isVisible())
        << "abgelehntes Schliessen laesst das Fenster stehen";
}

// Ohne Waechter schliesst das Fenster wie bisher — dieser Slice fuehrt KEINE
// Rueckfrage ein (Verhaltens-Invarianz, Plan §2).
TEST(MainWindow, WithoutGuardTheWindowClosesAsBefore) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, {});
    window.show();

    EXPECT_TRUE(window.close());
    EXPECT_FALSE(window.isVisible());
}

// --- slice-052a, §6-13: Schliessen mit ungesichertem Stand ----------------
//
// Die Naht ist der `CloseGuard` (slice-053). Hier wird sie mit dem ECHTEN
// Handler und dem ECHTEN Sitzungs-Service besetzt — geprueft wird die Kette
// Fenster -> Handler -> Sitzung, nicht ein nachgebautes Verhalten. Ungeprueft
// bleibt allein der modale Dialog (benannte Grenze).

TEST(MainWindow, SchliessenMitUngesichertemStandUndAbbrechenHaeltDasFensterOffen) {
    const QtFixture qt;
    model::Building baseline;
    baseline.storeys.push_back({model::StoreyId{1}, 2500.0});
    bcad::hexagon::services::ProjectSessionService session(baseline);

    model::Building aktuell = baseline;
    aktuell.storeys.push_back({model::StoreyId{2}, 2700.0});  // ungesichert

    StubManageProject project;
    bcad::adapters::ui::command::ProjectMenuHandler handler(
        project, session, [&aktuell]() -> const model::Building& { return aktuell; },
        {});

    MainWindow window(nullptr, {}, {}, {}, [&handler]() {
        return handler.mayDiscard(
            []() { return bcad::hexagon::ports::driving::DiscardAnswer::Cancel; },
            []() { return std::optional<std::filesystem::path>{}; });
    });
    window.show();
    ASSERT_TRUE(window.isVisible());

    EXPECT_FALSE(window.close());
    EXPECT_TRUE(window.isVisible())
        << "abbrechen bei ungesichertem Stand haelt das Fenster offen";
}

// Gegenstueck: ist nichts ungesichert, schliesst das Fenster ohne Rueckfrage.
TEST(MainWindow, SchliessenOhneUngesichertenStandFragtNicht) {
    const QtFixture qt;
    model::Building baseline;
    baseline.storeys.push_back({model::StoreyId{1}, 2500.0});
    bcad::hexagon::services::ProjectSessionService session(baseline);

    StubManageProject project;
    bcad::adapters::ui::command::ProjectMenuHandler handler(
        project, session,
        [&baseline]() -> const model::Building& { return baseline; }, {});

    int gefragt = 0;
    MainWindow window(nullptr, {}, {}, {}, [&handler, &gefragt]() {
        return handler.mayDiscard(
            [&gefragt]() {
                ++gefragt;
                return bcad::hexagon::ports::driving::DiscardAnswer::Cancel;
            },
            []() { return std::optional<std::filesystem::path>{}; });
    });
    window.show();

    EXPECT_TRUE(window.close());
    EXPECT_EQ(gefragt, 0);
}

// --- slice-058, §4-9 und §4-5a: Werkzeug-Modus und Hinweis-Anzeige ---------
//
// Reichweite, ausgeschrieben (Plan §4-9): geprueft wird der FENSTER-VERTRAG —
// die Aktion existiert, sie ruft ihr injiziertes Callable, und die aktive Aktion
// ist markiert. Die PRODUKTIVE Verdrahtung (Aktion → `CanvasWidget::setToolMode`)
// liegt im Composition-Root und ist per Konstruktion orakel-los: `src/main.cpp`
// ist in kein Testbinary gelinkt. Das ist die bekannte Klasse, keine neue Luecke.

// §4-9: der Modus ist bedienbar UND sichtbar.
TEST(MainWindow, ADR_0021_E1_WerkzeugAktionenSindAusloesbarUndMarkiert) {
    const QtFixture qt;
    int guide_line_calls = 0;
    int wall_calls = 0;
    MainWindow window(nullptr, {},
                      {[&guide_line_calls]() { ++guide_line_calls; },
                       [&wall_calls]() { ++wall_calls; }},
                      {}, {});

    QAction* guide_line = actionNamed(window, MainWindow::kToolGuideLineActionName);
    QAction* wall = actionNamed(window, MainWindow::kToolWallActionName);
    ASSERT_NE(guide_line, nullptr) << "die Werkzeug-Aktion muss auffindbar sein";
    ASSERT_NE(wall, nullptr);

    // Default ist HILFSLINIE (ADR-0021 E1) — die bestehende Bedienung laeuft
    // unveraendert weiter, und die Markierung sagt es.
    EXPECT_TRUE(guide_line->isChecked());
    EXPECT_FALSE(wall->isChecked());

    // Ausloesen ruft das injizierte Callable …
    wall->trigger();
    EXPECT_EQ(wall_calls, 1);
    EXPECT_EQ(guide_line_calls, 0);
    // … und verschiebt die Markierung. Exklusiv: genau EIN Werkzeug ist aktiv.
    EXPECT_TRUE(wall->isChecked());
    EXPECT_FALSE(guide_line->isChecked())
        << "zwei gleichzeitig markierte Werkzeuge waeren eine Luege ueber den "
           "Zustand des Canvas";

    guide_line->trigger();
    EXPECT_EQ(guide_line_calls, 1);
    EXPECT_TRUE(guide_line->isChecked());
    EXPECT_FALSE(wall->isChecked());
}

// §4-5a: die Hinweis-Anzeige existiert und traegt den GEMELDETEN Text.
//
// Bewusst OHNE `isVisible()`-Konjunkt (Plan-Review Lauf 3): den entscheidet
// `show()` und die Konstruktions-Reihenfolge, nicht die Implementierung — auch
// ein nie eingelayoutetes Waisen-Widget ist nach `show()` sichtbar. Der
// Text-Konjunkt traegt die Zeile allein.
//
// Und bewusst OHNE Tinten-Sonde: am Fenster zaehlt sie 120 000 von 120 000
// Pixeln als Tinte, weil der Hintergrund nicht weiss ist (Lauf 2, gemessen).
// Sie traegt nur am Canvas — dort loest §4-8a die ADR-0021-E14-Folgepflicht ein.
TEST(MainWindow, ADR_0021_E5_HinweisAnzeigeTraegtDenGemeldetenText) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, {});

    auto* hint = window.findChild<QLabel*>(
        QString::fromLatin1(MainWindow::kHintLabelName));
    ASSERT_NE(hint, nullptr)
        << "ohne Anzeige waere 'jeder Fehl-Ausgang gibt einen Hinweis' eine "
           "Zusage ohne Adressaten";
    EXPECT_TRUE(hint->text().isEmpty()) << "frisch: kein Hinweis";

    window.showHint(QStringLiteral("Keine Wand angelegt."));
    EXPECT_EQ(hint->text(), QStringLiteral("Keine Wand angelegt."));

    // Ein zweiter Hinweis ERSETZT den ersten — eine Zeile, kein Protokoll.
    window.showHint(QStringLiteral("Wand nicht angelegt."));
    EXPECT_EQ(hint->text(), QStringLiteral("Wand nicht angelegt."));

    // Und der Erfolgsfall raeumt sie ab (ADR-0021 E5: die Wand ist der Beleg).
    window.showHint(QString());
    EXPECT_TRUE(hint->text().isEmpty());
}

// --- slice-059a, §4-9a: die dritte Werkzeug-Aktion ------------------------
//
// Reichweite wie bei slice-058 §4-9: geprueft wird der FENSTER-VERTRAG, nicht
// die produktive Verdrahtung Aktion -> CanvasWidget::setToolMode — die liegt im
// Composition-Root und ist per Konstruktion orakel-los.
TEST(MainWindow, ADR_0021_E3_AuswahlWerkzeugIstAusloesbarUndMarkiert) {
    const QtFixture qt;
    int guide_line_calls = 0;
    int wall_calls = 0;
    int select_calls = 0;
    MainWindow window(nullptr, {},
                      {[&guide_line_calls]() { ++guide_line_calls; },
                       [&wall_calls]() { ++wall_calls; },
                       [&select_calls]() { ++select_calls; }},
                      {}, {});

    QAction* guide_line = actionNamed(window, MainWindow::kToolGuideLineActionName);
    QAction* wall = actionNamed(window, MainWindow::kToolWallActionName);
    QAction* select = actionNamed(window, MainWindow::kToolSelectActionName);
    ASSERT_NE(select, nullptr) << "die Auswahl-Aktion muss auffindbar sein";
    EXPECT_FALSE(select->isChecked()) << "Default bleibt Hilfslinie";

    select->trigger();
    EXPECT_EQ(select_calls, 1);
    EXPECT_EQ(wall_calls, 0) << "die Aktionen sind nicht vertauscht";
    EXPECT_EQ(guide_line_calls, 0);

    // Die Markierung springt um, und die Gruppe bleibt EXKLUSIV: genau ein
    // Werkzeug ist aktiv. Zwei markierte Werkzeuge waeren eine Luege ueber den
    // Zustand des Canvas.
    EXPECT_TRUE(select->isChecked());
    EXPECT_FALSE(guide_line->isChecked());
    EXPECT_FALSE(wall->isChecked());

    wall->trigger();
    EXPECT_TRUE(wall->isChecked());
    EXPECT_FALSE(select->isChecked());
}

// --- slice-059b, §4-1/§4-2/§4-5 (Fenster)/§4-6/§4-13 ----------------------
//
// Der Eigenschaften-Bereich. Die Naht nimmt WERTE, keinen fertigen Text
// (Plan §2.3) — nur so ist der abnahmebindende Konjunkt "der tatsaechlich
// uebernommene Wert wird dem Nutzer GENANNT" ueberhaupt messbar.

QLineEdit* fieldNamed(MainWindow& window, const char* name) {
    return window.findChild<QLineEdit*>(QString::fromLatin1(name));
}

// Der Feld-Inhalt als Zahl — zurueckgelesen mit DERSELBEN Umwandlung wie die
// Senke (Plan §2.1). Mit einer anderen waere ein Feld-Inhalt "50,0" im Test
// gruen und im Produkt eine Ablehnung.
std::optional<double> fieldValue(MainWindow& window, const char* name) {
    QLineEdit* field = fieldNamed(window, name);
    if (field == nullptr) {
        return std::nullopt;
    }
    return bcad::adapters::ui::command::WallParamSink::parse(
        field->text().toStdString());
}

// §4-1: der Bereich zeigt die Parameter der gewaehlten Wand.
TEST(MainWindow, ADR_0021_E4_EigenschaftenBereichZeigtDieParameter) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, {});

    window.showWallParams(model::WallParams{240.0, 2500.0});
    EXPECT_EQ(fieldValue(window, MainWindow::kThicknessFieldName), 240.0);
    EXPECT_EQ(fieldValue(window, MainWindow::kHeightFieldName), 2500.0);
}

// §4-2: keine Auswahl ⇒ nichts zu aendern, und KEINE Werte einer zuvor
// gewaehlten Wand. Abnahmebindend (LH-FA-WAL-002 Boundary (Auswahl)).
TEST(MainWindow, LH_FA_WAL_002_Boundary_OhneAuswahlStehenKeineAltenWerteDa) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, {});
    window.showWallParams(model::WallParams{240.0, 2500.0});
    ASSERT_EQ(fieldValue(window, MainWindow::kThicknessFieldName), 240.0);

    window.showWallParams(std::nullopt);

    auto* label = window.findChild<QLabel*>(
        QString::fromLatin1(MainWindow::kSelectionLabelName));
    ASSERT_NE(label, nullptr);
    EXPECT_FALSE(label->text().isEmpty()) << "der Bereich sagt, dass nichts gewaehlt ist";
    EXPECT_TRUE(fieldNamed(window, MainWindow::kThicknessFieldName)->text().isEmpty())
        << "die Werte der zuvor gewaehlten Wand duerfen NICHT stehen bleiben";
    EXPECT_TRUE(fieldNamed(window, MainWindow::kHeightFieldName)->text().isEmpty());
    EXPECT_FALSE(fieldNamed(window, MainWindow::kThicknessFieldName)->isEnabled())
        << "ohne Auswahl gibt es nichts zu aendern";
}

// §4-5 (Fenster-Haelfte): die Klemmung wird mit dem UEBERNOMMENEN WERT genannt.
// Abnahmebindend ist die Nennung des Werts, nicht der Wortlaut.
TEST(MainWindow, LH_FA_WAL_002_Boundary_KlemmungWirdMitDemWertGenannt) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, {});
    window.showWallParams(model::WallParams{240.0, 2500.0});

    window.showParamOutcome(MainWindow::ParamOutcome::Clamped, 50.0, true);

    auto* hint = window.findChild<QLabel*>(
        QString::fromLatin1(MainWindow::kHintLabelName));
    ASSERT_NE(hint, nullptr);
    EXPECT_TRUE(hint->text().contains(QStringLiteral("50")))
        << "der uebernommene Wert muss im Hinweis vorkommen — sonst erfaehrt "
           "der Nutzer die Klemmung, aber nicht das Ergebnis";
}

// §4-6: nach der Klemmung zeigt das FELD den uebernommenen Wert, nicht die
// Eingabe. Eigener Weg, eigene Zeile (der Rueckweg liegt im Fenster).
TEST(MainWindow, ADR_0021_E5_NachDerKlemmungZeigtDasFeldDenUebernommenenWert) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, {});
    window.showWallParams(model::WallParams{240.0, 2500.0});

    // Der Nutzer tippt 49; der Kern klemmt auf 50.
    fieldNamed(window, MainWindow::kThicknessFieldName)
        ->setText(QStringLiteral("49"));
    window.showParamOutcome(MainWindow::ParamOutcome::Clamped, 50.0, true);

    EXPECT_EQ(fieldValue(window, MainWindow::kThicknessFieldName), 50.0)
        << "das Feld muss den uebernommenen Wert zeigen, nicht die Eingabe";
    // Und die Hoehe bleibt unberuehrt.
    EXPECT_EQ(fieldValue(window, MainWindow::kHeightFieldName), 2500.0);
}

// §4-13: die Anzeige-Form ist von der Senken-Umwandlung lesbar — der Rundlauf
// schliesst. Ohne diese Zeile haette das Produkt einen Fehler, den kein anderes
// Orakel sieht: Enter auf einem nie geaenderten Feld ergaebe eine Ablehnung.
TEST(MainWindow, DieAnzeigeFormIstVonDerSenkenUmwandlungLesbar) {
    const QtFixture qt;
    MainWindow window(nullptr, {}, {}, {}, {});

    for (const model::WallParams params :
         {model::WallParams{240.0, 2500.0}, model::WallParams{50.0, 500.0},
          model::WallParams{1000.0, 10000.0}}) {
        window.showWallParams(params);
        for (const char* name : {MainWindow::kThicknessFieldName,
                                 MainWindow::kHeightFieldName}) {
            const QString shown = fieldNamed(window, name)->text();
            EXPECT_TRUE(bcad::adapters::ui::command::WallParamSink::parse(
                            shown.toStdString())
                            .has_value())
                << "die Anzeige-Form muss die Senke UNVERAENDERT passieren: "
                << shown.toStdString();
        }
    }
}

}  // namespace