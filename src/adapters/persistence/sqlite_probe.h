#pragma once

#include <string>

namespace bcad::adapters::persistence {

// Skelett-Probe (slice-001): liefert die zur Laufzeit gegen SQLite
// gelinkte Version. Beweist echte Linkage (sqlite3_libversion() ist ein
// reales Symbol aus libsqlite3).
std::string sqlite_version();

}  // namespace bcad::adapters::persistence
