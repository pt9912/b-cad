#include "adapters/ui/view/main_window.h"

#include <utility>

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace bcad::adapters::ui::view {
namespace {

// Anzeige-Form der Millimeter-Werte. **Sie ist an eine Schliessbedingung
// gebunden** (slice-059b §2.2): was hier herauskommt, muss die Umwandlung der
// `ui/command/`-Parameter-Senke **ohne Ablehnung** wieder lesen — sonst bekaeme
// ein Benutzer, der ein zurueckgeschriebenes Feld nur bestaetigt, eine
// Ablehnung fuer einen Wert, den er nie geaendert hat.
//
// Deshalb: schlichte Dezimalzahl, **kein** Tausender-Trenner, **keine** Einheit
// im Feld (die steht in der Beschriftung), **kein** Komma. `QString::number`
// ist locale-unabhaengig — `QLocale` waere es nicht.
//
// Das Fenster kann die Senke nicht einbinden (`ui_view -> ui_command` ist keine
// deklarierte Kante), die zwei Seiten sind also **nur ueber ein Orakel**
// gekoppelt: §4-13 schickt den Feld-Inhalt unveraendert zurueck durch die Senke.
QString formatMm(double value) { return QString::number(value, 'f', 1); }

}  // namespace

MainWindow::MainWindow(QWidget* central, FileActions actions, ToolActions tools,
                       ParamActions params, CloseGuard close_guard)
    : close_guard_(std::move(close_guard)) {
    // slice-059b (ADR-0021 E4): der nicht-modale Eigenschaften-Bereich. Ein
    // FESTER Bereich unter der Sicht — keine Andock-Verwaltung (LH-FA-UI-001
    // bleibt Outline), kein modaler Dialog (E5: eine Klemmung beim Tippen darf
    // den Zeichenfluss nicht unterbrechen).
    auto* container = new QWidget(this);
    auto* column = new QVBoxLayout(container);
    column->setContentsMargins(0, 0, 0, 0);
    if (central != nullptr) {
        column->addWidget(central, 1);  // Qt uebernimmt das Widget-Ownership
    }

    auto* properties = new QWidget(container);
    auto* form = new QFormLayout(properties);
    selection_label_ = new QLabel(properties);
    selection_label_->setObjectName(QString::fromLatin1(kSelectionLabelName));
    form->addRow(selection_label_);

    // **Textfelder, keine Zahlen-Drehfelder** (slice-059b §2.1): ein Drehfeld
    // mit dem Modell-Bereich klemmte die Eingabe SELBST — dann erreichte eine
    // 49 den Kern nie, der Hinweis erschiene nie, und zwei abnahmebindende
    // Akzeptanzkriterien (Klemmung sichtbar, Ablehnung sichtbar) waeren
    // **unerreichbar statt rot**. Der Kern bleibt die einzige Klemm-Autoritaet.
    thickness_field_ = new QLineEdit(properties);
    thickness_field_->setObjectName(QString::fromLatin1(kThicknessFieldName));
    form->addRow(QStringLiteral("Staerke (mm)"), thickness_field_);
    height_field_ = new QLineEdit(properties);
    height_field_->setObjectName(QString::fromLatin1(kHeightFieldName));
    form->addRow(QStringLiteral("Hoehe (mm)"), height_field_);
    column->addWidget(properties);
    setCentralWidget(container);

    // Uebernahme bei ABSCHLUSS der Eingabe, nicht je Tastendruck: sonst
    // mutierte das Tippen von "240" das Modell dreimal (2 -> geklemmt 50,
    // 24 -> 50, 240) und erzeugte zwei falsche Klemm-Hinweise fuer EINE Eingabe.
    if (params.commit_thickness) {
        QObject::connect(thickness_field_, &QLineEdit::editingFinished, this,
                         [this, handler = std::move(params.commit_thickness)]() {
                             handler(thickness_field_->text());
                         });
    }
    if (params.commit_height) {
        QObject::connect(height_field_, &QLineEdit::editingFinished, this,
                         [this, handler = std::move(params.commit_height)]() {
                             handler(height_field_->text());
                         });
    }
    showWallParams(std::nullopt);  // Startzustand: keine Auswahl

    QMenu* file_menu = menuBar()->addMenu(QStringLiteral("&Datei"));

    QAction* new_project = file_menu->addAction(QStringLiteral("&Neu"));
    new_project->setObjectName(QString::fromLatin1(kNewActionName));
    if (actions.new_project) {
        QObject::connect(new_project, &QAction::triggered, this,
                         [this, handler = std::move(actions.new_project)]() {
                             handler(this);
                         });
    }

    QAction* open = file_menu->addAction(QStringLiteral("&Oeffnen..."));
    open->setObjectName(QString::fromLatin1(kOpenActionName));
    if (actions.open) {
        QObject::connect(open, &QAction::triggered, this,
                         [this, handler = std::move(actions.open)]() {
                             handler(this);
                         });
    }

    QAction* save = file_menu->addAction(QStringLiteral("&Speichern"));
    save->setObjectName(QString::fromLatin1(kSaveActionName));
    if (actions.save) {
        QObject::connect(save, &QAction::triggered, this,
                         [this, handler = std::move(actions.save)]() {
                             handler(this);
                         });
    }

    QAction* save_as = file_menu->addAction(QStringLiteral("Speichern &unter..."));
    save_as->setObjectName(QString::fromLatin1(kSaveAsActionName));
    if (actions.save_as) {
        QObject::connect(save_as, &QAction::triggered, this,
                         [this, handler = std::move(actions.save_as)]() {
                             handler(this);
                         });
    }

    // slice-058 (ADR-0021 E1): der Werkzeug-Modus. Eine EXKLUSIVE Aktions-Gruppe
    // aus pruefbaren Aktionen — die aktive ist `isChecked()`, also ist die
    // Markierung eine Eigenschaft und keine Malerei. Default ist **Hilfslinie**,
    // damit die bestehende Bedienung unveraendert weiterlaeuft.
    QMenu* tool_menu = menuBar()->addMenu(QStringLiteral("&Werkzeug"));
    auto* tool_group = new QActionGroup(this);
    tool_group->setExclusive(true);

    QAction* guide_line_tool = tool_menu->addAction(QStringLiteral("&Hilfslinie"));
    guide_line_tool->setObjectName(QString::fromLatin1(kToolGuideLineActionName));
    guide_line_tool->setCheckable(true);
    guide_line_tool->setChecked(true);  // E1: Default
    tool_group->addAction(guide_line_tool);
    if (tools.select_guide_line) {
        QObject::connect(guide_line_tool, &QAction::triggered, this,
                         [handler = std::move(tools.select_guide_line)]() {
                             handler();
                         });
    }

    QAction* wall_tool = tool_menu->addAction(QStringLiteral("&Wand"));
    wall_tool->setObjectName(QString::fromLatin1(kToolWallActionName));
    wall_tool->setCheckable(true);
    tool_group->addAction(wall_tool);
    if (tools.select_wall) {
        QObject::connect(wall_tool, &QAction::triggered, this,
                         [handler = std::move(tools.select_wall)]() {
                             handler();
                         });
    }

    QAction* select_tool = tool_menu->addAction(QStringLiteral("&Auswahl"));
    select_tool->setObjectName(QString::fromLatin1(kToolSelectActionName));
    select_tool->setCheckable(true);
    tool_group->addAction(select_tool);
    if (tools.select_pick) {
        QObject::connect(select_tool, &QAction::triggered, this,
                         [handler = std::move(tools.select_pick)]() {
                             handler();
                         });
    }

    // slice-058 (ADR-0021 E5): die Hinweis-Zeile. Statusleiste = nicht-modal,
    // dauerhaft sichtbar, unterbricht den Zeichenfluss nicht. Ein `QLabel`
    // statt `showMessage`, damit der Text eine LESBARE Eigenschaft ist —
    // `QStatusBar::currentMessage` waere zeitgesteuert und damit ein Orakel,
    // das manchmal misst.
    hint_label_ = new QLabel(this);
    hint_label_->setObjectName(QString::fromLatin1(kHintLabelName));
    statusBar()->addWidget(hint_label_);
}

void MainWindow::showWallParams(
    std::optional<hexagon::model::WallParams> params) {
    const bool has_selection = params.has_value();
    if (selection_label_ != nullptr) {
        selection_label_->setText(has_selection
                                      ? QStringLiteral("Gewaehlte Wand")
                                      : QStringLiteral("Keine Auswahl"));
    }
    // Ohne Auswahl gibt es NICHTS zu aendern — und vor allem stehen dann
    // **keine Werte einer zuvor gewaehlten Wand** mehr da (LH-FA-WAL-002
    // Boundary (Auswahl), abnahmebindend).
    for (QLineEdit* field : {thickness_field_, height_field_}) {
        if (field != nullptr) {
            field->setEnabled(has_selection);
            if (!has_selection) {
                field->clear();
            }
        }
    }
    if (!has_selection) {
        return;
    }
    if (thickness_field_ != nullptr) {
        thickness_field_->setText(formatMm(params->thickness_mm));
    }
    if (height_field_ != nullptr) {
        height_field_->setText(formatMm(params->height_mm));
    }
}

void MainWindow::showParamOutcome(ParamOutcome outcome, double applied_mm,
                                  bool thickness) {
    QLineEdit* field = thickness ? thickness_field_ : height_field_;
    switch (outcome) {
        case ParamOutcome::Accepted:
            if (field != nullptr) {
                field->setText(formatMm(applied_mm));
            }
            showHint(QString());
            return;
        case ParamOutcome::Clamped:
            // Der **uebernommene Wert** geht ins Feld UND in den Hinweis. Dass
            // er genannt wird, ist der abnahmebindende Teil; wie der Satz
            // lautet, ist es nicht.
            if (field != nullptr) {
                field->setText(formatMm(applied_mm));
            }
            showHint(QStringLiteral("Wert geklemmt — uebernommen: %1 mm.")
                         .arg(formatMm(applied_mm)));
            return;
        case ParamOutcome::Rejected:
            showHint(QStringLiteral("Eingabe abgelehnt — Modell unveraendert."));
            return;
        case ParamOutcome::NotANumber:
            showHint(QStringLiteral("Keine gueltige Zahl — Modell unveraendert."));
            return;
        case ParamOutcome::Failed:
            showHint(QStringLiteral("Aenderung nicht moeglich — Modell "
                                    "unveraendert."));
            return;
    }
}

void MainWindow::showHint(const QString& text) {
    if (hint_label_ != nullptr) {
        hint_label_->setText(text);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Ohne Waechter schliesst das Fenster wie bisher — dieser Slice fuehrt
    // KEINE Rueckfrage ein (slice-053 §2), er schafft nur den Ort dafuer.
    if (close_guard_ && !close_guard_()) {
        event->ignore();  // slice-052a haengt hier sein "abbrechen" auf
        return;
    }
    QMainWindow::closeEvent(event);
}

}  // namespace bcad::adapters::ui::view
