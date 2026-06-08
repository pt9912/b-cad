// Driving-Adapter (GUI) — Skelett-Probe (slice-001).
// Beweist, dass bcad_adapters real gegen Qt 6 linkt. Der echte
// Qt-GUI-Adapter (ruft Driving Ports, ADR-0001) folgt ab slice-003.

#include <QtGlobal>

#include <string>

namespace bcad::adapters::ui {

std::string qt_version() {
    return qVersion();
}

}  // namespace bcad::adapters::ui
