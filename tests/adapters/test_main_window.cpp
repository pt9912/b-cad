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
#include <QWidget>

#include <gtest/gtest.h>

#include "adapters/ui/view/main_window.h"

namespace {

using bcad::adapters::ui::view::MainWindow;

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
    MainWindow window(central, {}, {});

    EXPECT_EQ(window.centralWidget(), central)
        << "das Fenster uebernimmt das gereichte Widget (Qt-Ownership)";
    // Die beiden Menue-Aktionen existieren, auch ohne verdrahtete Handler.
    EXPECT_NE(actionNamed(window, MainWindow::kOpenActionName), nullptr);
    EXPECT_NE(actionNamed(window, MainWindow::kSaveAsActionName), nullptr);
}

// §3-4: eine Menue-Aktion ist ausloesbar und ruft ihre injizierte Funktion.
TEST(MainWindow, TriggeringMenuActionsCallsTheInjectedHandlers) {
    const QtFixture qt;
    int opened = 0;
    int saved = 0;
    QWidget* open_parent = nullptr;
    MainWindow window(nullptr,
                      {[&opened, &open_parent](QWidget* parent) {
                           ++opened;
                           open_parent = parent;
                       },
                       [&saved](QWidget*) { ++saved; }},
                      {});

    actionNamed(window, MainWindow::kOpenActionName)->trigger();
    EXPECT_EQ(opened, 1);
    EXPECT_EQ(saved, 0) << "die Aktionen sind nicht vertauscht";

    actionNamed(window, MainWindow::kSaveAsActionName)->trigger();
    EXPECT_EQ(opened, 1);
    EXPECT_EQ(saved, 1);

    // Das Fenster reicht sich SELBST als Dialog-Eltern durch — sonst haetten
    // die modalen Dialoge im Composition-Root kein Eltern-Fenster.
    EXPECT_EQ(open_parent, &window);
}

// §3-2: `close()` fuehrt die Schliess-Behandlung aus — der injizierte Haken
// wird gerufen. Ohne Traeger-Klasse gab es dafuer keinen Ort.
TEST(MainWindow, CloseRunsTheCloseGuard) {
    const QtFixture qt;
    int asked = 0;
    MainWindow window(nullptr, {}, [&asked]() {
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
    MainWindow window(nullptr, {}, []() { return false; });
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
    MainWindow window(nullptr, {}, {});
    window.show();

    EXPECT_TRUE(window.close());
    EXPECT_FALSE(window.isVisible());
}

}  // namespace
