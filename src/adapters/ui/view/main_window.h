#pragma once

#include <functional>

#include <QMainWindow>

class QCloseEvent;
class QWidget;

namespace bcad::adapters::ui::view {

// Hauptfenster als **testbarer Adapter** (slice-053). Bis hierher lebte das
// Fenster als lokale `QMainWindow`-Variable im coverage-ausgenommenen
// `src/main.cpp` — in **kein** Testbinary gelinkt. Damit hatte weder der
// Menü-Aufbau noch das Schließ-Ereignis je einen Sensor; slice-052a scheiterte
// genau daran (Plan-Review Lauf 3, HIGH-1: die zugesagte Orakel-Zeile für die
// Ungesichert-Rückfrage war unter dem eigenen Datei-Plan nicht herstellbar).
//
// **Port-frei** (Muster ADR-0019 Option A, Präzedenz `CanvasWidget`): das
// Fenster kennt **keinen** Driving Port und **keinen** `command/`-Header. Was
// eine Aktion *tut*, kommt als injizierte `std::function` vom
// Composition-Root — der sie aus einem `ui/command/`-Handler bezieht. Deshalb
// bleibt `ui_view → ports_driven`/`model` unverletzt (`.a-check.yml`), obwohl
// das Fenster ein Kommando auslöst.
//
// **Was hier NICHT liegt** (slice-053 §3, benannte Grenze): die modalen
// Dialoge, die Meldungstexte, der Fenstertitel, die `.bcad`-Suffix-Ergänzung
// und der Fenster-Aufbau selbst. Sie bleiben im Composition-Root. Dieses
// Fenster trägt **keine Entscheidung** — es macht die vorhandenen beobachtbar.
class MainWindow final : public QMainWindow {
public:
    // Was das Datei-Menü auslöst. Beide Callables sind optional: eine nicht
    // gesetzte Aktion wird nicht verdrahtet (der Menüpunkt bleibt trotzdem
    // sichtbar — kein Verhaltensunterschied für den Benutzer, aber Tests
    // können das Fenster ohne Handler bauen).
    //
    // Das Fenster reicht sich beim Auslösen **selbst** als Eltern-Widget
    // durch: die modalen Dialoge liegen im Composition-Root (§3, benannte
    // Grenze), brauchen aber ein Eltern-Fenster — und nur das Fenster kennt
    // sich zum Auslöse-Zeitpunkt. Ein `nullptr` ist zulässig (Tests).
    using Action = std::function<void(QWidget* dialog_parent)>;

    struct FileActions {
        Action open;
        Action save;     // slice-052a: auf die bekannte Datei (LH-FA-BLD-002)
        Action save_as;
    };

    // Antwort auf „darf geschlossen werden?" — `true` schließt, `false` hält
    // das Fenster offen. **Nicht gesetzt heißt: schließen** (heutiges
    // Verhalten, dieser Slice ändert nichts daran). Die Naht existiert, damit
    // slice-052a die Ungesichert-Rückfrage daran hängen kann — und weil sie
    // hier liegt, ist sie prüfbar.
    using CloseGuard = std::function<bool()>;

    // `central` wird dem Fenster übergeben (Qt-Ownership via
    // `setCentralWidget`) — genau wie bisher im Composition-Root. Das Fenster
    // **baut** die Sichten nicht: 3D-/2D-Widgets sind nicht Gegenstand dieses
    // Slice, und der Root behält die Zeiger, die er selbst erzeugt hat.
    MainWindow(QWidget* central, FileActions actions, CloseGuard close_guard);

    // Objektnamen der Menü-Aktionen — Tests lösen sie darüber aus
    // (`findChild<QAction*>`), statt sich auf Menü-Reihenfolge oder
    // Beschriftungen zu verlassen.
    static constexpr auto kOpenActionName = "action_open";
    static constexpr auto kSaveActionName = "action_save";
    static constexpr auto kSaveAsActionName = "action_save_as";

protected:
    // Ruft den `CloseGuard`; sagt er „nein", wird das Ereignis abgelehnt und
    // das Fenster bleibt sichtbar.
    void closeEvent(QCloseEvent* event) override;

private:
    CloseGuard close_guard_;
};

}  // namespace bcad::adapters::ui::view
