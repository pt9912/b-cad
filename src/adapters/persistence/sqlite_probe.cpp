// Driven-Adapter (Persistenz) — Skelett-Probe (slice-001).
// Beweist, dass bcad_adapters real gegen SQLite linkt. Der echte
// ProjectRepositoryPort-Adapter (atomar, ADR-0003) folgt ab slice-003.

#include "adapters/persistence/sqlite_probe.h"

#include <sqlite3.h>

namespace bcad::adapters::persistence {

std::string sqlite_version() {
    return sqlite3_libversion();  // reales Symbol aus libsqlite3 → erzwingt Linkage
}

}  // namespace bcad::adapters::persistence
