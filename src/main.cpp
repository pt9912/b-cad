// Composition Root von b-cad (ADR-0001).
//
// Im Build-Skelett (slice-001) verdrahtet main noch keine Adapter — es
// belegt nur, dass die Target-Kette (bcad_hexagon + bcad_adapters)
// linkt. Ab slice-003 werden hier konkrete Adapter (Qt-GUI,
// OpenCascade-Geometrie, SQLite-Persistenz) in den Kern injiziert.

#include <iostream>

#include "hexagon/services/bootstrap_info.h"

int main() {
    std::cout << bcad::hexagon::services::application_banner() << '\n';
    return 0;
}
