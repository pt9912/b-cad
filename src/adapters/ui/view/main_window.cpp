#include "adapters/ui/view/main_window.h"

#include <utility>

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QString>
#include <QWidget>

namespace bcad::adapters::ui::view {

MainWindow::MainWindow(QWidget* central, FileActions actions, ToolActions tools,
                       CloseGuard close_guard)
    : close_guard_(std::move(close_guard)) {
    if (central != nullptr) {
        setCentralWidget(central);  // Qt uebernimmt das Widget-Ownership
    }

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

    // slice-058 (ADR-0021 E5): die Hinweis-Zeile. Statusleiste = nicht-modal,
    // dauerhaft sichtbar, unterbricht den Zeichenfluss nicht. Ein `QLabel`
    // statt `showMessage`, damit der Text eine LESBARE Eigenschaft ist —
    // `QStatusBar::currentMessage` waere zeitgesteuert und damit ein Orakel,
    // das manchmal misst.
    hint_label_ = new QLabel(this);
    hint_label_->setObjectName(QString::fromLatin1(kHintLabelName));
    statusBar()->addWidget(hint_label_);
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
