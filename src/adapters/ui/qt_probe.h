#pragma once

#include <string>

namespace bcad::adapters::ui {

// Skelett-Probe (slice-001): liefert die zur Laufzeit gegen Qt 6
// gelinkte Version. Beweist echte Linkage (qVersion() ist ein reales
// Symbol aus Qt6Core, kein Compile-Time-Makro).
std::string qt_version();

}  // namespace bcad::adapters::ui
