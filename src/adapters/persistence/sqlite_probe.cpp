// Driven-Adapter (Persistenz) — Skelett-Probe (slice-001).
// Beweist, dass bcad_adapters real gegen SQLite linkt. Der echte
// ProjectRepositoryPort-Adapter (atomar, ADR-0003) folgt ab slice-003.

#include <sqlite3.h>

#include <string>

namespace bcad::adapters::persistence {

std::string sqlite_version() {
    return sqlite3_libversion();
}

}  // namespace bcad::adapters::persistence
