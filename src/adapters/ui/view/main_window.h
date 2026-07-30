#pragma once

#include <functional>
#include <optional>

#include <QMainWindow>

#include "hexagon/model/wall_params.h"

class QCloseEvent;
class QLabel;
class QLineEdit;
class QString;
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
        Action new_project;  // slice-052b (LH-FA-BLD-001)
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

    // Was die Werkzeug-Aktionen auslösen (slice-058, ADR-0021 E1). Das Fenster
    // **kennt den Canvas nicht** — es meldet nur, welches Werkzeug gewählt
    // wurde; der Composition-Root ruft darauf `CanvasWidget::setToolMode`.
    // Dieselbe port-freie Bauform wie `FileActions`.
    struct ToolActions {
        std::function<void()> select_guide_line;
        std::function<void()> select_wall;
        std::function<void()> select_pick;  // slice-059a: Auswahl-Werkzeug
    };

    // Was der Eigenschaften-Bereich auslöst (slice-059b, ADR-0021 E4/E5):
    // eine Übernahme mit dem **Text** des Feldes. Der Text — nicht die Zahl:
    // die Umwandlung ist eine Ausgangs-Entscheidung und gehört in die
    // `ui/command/`-Senke, nicht in zwei Autoritäten (slice-059b §2.2).
    using ParamCommit = std::function<void(const QString& text)>;

    struct ParamActions {
        ParamCommit commit_thickness;
        ParamCommit commit_height;
    };

    // Ausgang einer Parameter-Änderung, wie ihn das Fenster **anzeigt**.
    // **Bewusst Werte statt eines fertigen Textes** (slice-059b §2.3): der
    // abnahmebindende Konjunkt ist „der tatsächlich übernommene Wert wird dem
    // Nutzer **genannt**". Käme der Hinweis als fertige Zeichenkette aus dem
    // Composition-Root, wäre die Stelle, an der der Wert in den Text gelangt,
    // per Konstruktion orakel-los — belegt wäre dann nur, dass **ein**
    // gereichter Text erscheint. Der **Wortlaut** bleibt Sache dieses Fensters
    // und ist **keine** Zusage; geprüft wird, dass der Wert darin vorkommt.
    enum class ParamOutcome { Accepted, Clamped, Rejected, NotANumber, Failed };

    // `central` wird dem Fenster übergeben (Qt-Ownership via
    // `setCentralWidget`) — genau wie bisher im Composition-Root. Das Fenster
    // **baut** die Sichten nicht: 3D-/2D-Widgets sind nicht Gegenstand dieses
    // Slice, und der Root behält die Zeiger, die er selbst erzeugt hat.
    MainWindow(QWidget* central, FileActions actions, ToolActions tools,
               ParamActions params, CloseGuard close_guard);

    // Zeigt einen Hinweis an (slice-058, ADR-0021 E5): **nicht-modal** — eine
    // Klemmung oder Ablehnung beim Zeichnen darf den Fluss nicht unterbrechen.
    // Der **Text** kommt von außen (Composition-Root, benannte Grenze); dieses
    // Fenster entscheidet nur, WO er erscheint.
    //
    // Im Bestand gab es **keine** Anzeige — ohne sie wäre „jeder Fehl-Ausgang
    // gibt einen Hinweis" eine Zusage ohne Adressat.
    void showHint(const QString& text);

    // Objektnamen der Menü-Aktionen — Tests lösen sie darüber aus
    // (`findChild<QAction*>`), statt sich auf Menü-Reihenfolge oder
    // Beschriftungen zu verlassen.
    static constexpr auto kNewActionName = "action_new";
    static constexpr auto kOpenActionName = "action_open";
    static constexpr auto kSaveActionName = "action_save";
    static constexpr auto kSaveAsActionName = "action_save_as";
    static constexpr auto kToolGuideLineActionName = "action_tool_guide_line";
    static constexpr auto kToolWallActionName = "action_tool_wall";
    static constexpr auto kToolSelectActionName = "action_tool_select";
    static constexpr auto kHintLabelName = "hint_label";
    static constexpr auto kThicknessFieldName = "field_wall_thickness";
    static constexpr auto kHeightFieldName = "field_wall_height";
    static constexpr auto kSelectionLabelName = "label_selection";

    // Der Eigenschaften-Bereich zeigt die Parameter der gewählten Wand — oder
    // **nichts** (`nullopt`): dann sagt er „keine Auswahl" und trägt **keine**
    // Werte einer zuvor gewählten Wand mehr
    // ([LH-FA-WAL-002](../../../../spec/lastenheft.md) Boundary (Auswahl),
    // abnahmebindend).
    void showWallParams(std::optional<hexagon::model::WallParams> params);

    // Der Rückweg (slice-059b §2.3): der übernommene Wert geht **ins Feld**
    // zurück und **in den Hinweis**. Er liegt hier und nicht im
    // Composition-Root, weil die Gegenprobe „Rückweg entfernt" sonst im Test
    // nicht herstellbar wäre.
    void showParamOutcome(ParamOutcome outcome, double applied_mm,
                          bool thickness);

protected:
    // Ruft den `CloseGuard`; sagt er „nein", wird das Ereignis abgelehnt und
    // das Fenster bleibt sichtbar.
    void closeEvent(QCloseEvent* event) override;

private:
    CloseGuard close_guard_;
    QLineEdit* thickness_field_{nullptr};
    QLineEdit* height_field_{nullptr};
    QLabel* selection_label_{nullptr};
    // Die Hinweis-Zeile. Qt-Eltern-Ownership (Statusleiste); der Zeiger dient
    // nur dem Setzen des Textes.
    QLabel* hint_label_{nullptr};
};

}  // namespace bcad::adapters::ui::view
