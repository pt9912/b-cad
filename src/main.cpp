// Composition Root von b-cad (ADR-0001).
//
// Im Build-Skelett (slice-001) verdrahtet main noch keine Adapter — es
// belegt nur, dass die Target-Kette (bcad_hexagon + bcad_adapters)
// linkt. Ab slice-003 werden hier konkrete Adapter (Qt-GUI,
// OpenCascade-Geometrie, SQLite-Persistenz) in den Kern injiziert.

#include <iostream>

#include "adapters/geometry/occ_probe.h"
#include "adapters/persistence/sqlite_probe.h"
#include "adapters/ui/qt_probe.h"
#include "hexagon/services/bootstrap_info.h"

int main() {
    // Composition Root (darf Adapter berühren, ADR-0001). Im Skelett
    // (slice-001) ist die Aufgabe ein Toolchain-Banner: die Referenz auf
    // die Adapter-Proben erzwingt echtes Linken der Executable gegen
    // Qt6/OpenCascade/SQLite. Ab slice-003 werden hier echte Adapter in
    // den Kern injiziert.
    std::cout << bcad::hexagon::services::application_banner() << '\n';
    std::cout << "  Qt           " << bcad::adapters::ui::qt_version() << '\n';
    std::cout << "  OpenCascade  " << bcad::adapters::geometry::occ_version() << '\n';
    std::cout << "  SQLite       " << bcad::adapters::persistence::sqlite_version() << '\n';
    return 0;
}
