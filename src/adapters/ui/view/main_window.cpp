#include "adapters/ui/view/main_window.h"

#include <utility>

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QWidget>

namespace bcad::adapters::ui::view {

MainWindow::MainWindow(QWidget* central, FileActions actions,
                       CloseGuard close_guard)
    : close_guard_(std::move(close_guard)) {
    if (central != nullptr) {
        setCentralWidget(central);  // Qt uebernimmt das Widget-Ownership
    }

    QMenu* file_menu = menuBar()->addMenu(QStringLiteral("&Datei"));

    QAction* open = file_menu->addAction(QStringLiteral("&Oeffnen..."));
    open->setObjectName(QString::fromLatin1(kOpenActionName));
    if (actions.open) {
        QObject::connect(open, &QAction::triggered, this,
                         [this, handler = std::move(actions.open)]() {
                             handler(this);
                         });
    }

    QAction* save_as = file_menu->addAction(QStringLiteral("&Speichern unter..."));
    save_as->setObjectName(QString::fromLatin1(kSaveAsActionName));
    if (actions.save_as) {
        QObject::connect(save_as, &QAction::triggered, this,
                         [this, handler = std::move(actions.save_as)]() {
                             handler(this);
                         });
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
