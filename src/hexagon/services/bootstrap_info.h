#pragma once

#include <string>

// Anwendungskern (Hexagon) — framework-frei. Keine Qt-, OpenCascade-
// oder SQLite-Abhängigkeit (ADR-0001). Dieses Minimal-Modul existiert,
// damit das Kern-Target im Build-Skelett (slice-001) eine echte
// Übersetzungseinheit hat; Fachlogik folgt ab slice-003.

namespace bcad::hexagon::services {

// Liefert einen Banner "b-cad <version>" — rein, ohne I/O.
std::string application_banner();

}  // namespace bcad::hexagon::services
