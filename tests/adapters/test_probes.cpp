// Adapter-Linkage-Test (slice-001, Review-Befund). Dieses Test-Target
// linkt gegen bcad_adapters; jede Probe ruft eine reale Laufzeit-
// funktion ihrer Bibliothek (qVersion / TKernel-String /
// sqlite3_libversion). Ein fehlerhaftes Linken gegen Qt6/OpenCascade/
// SQLite lässt schon den Link-Schritt scheitern, sonst die Assertion —
// damit beweist `make build` echte Linkage, nicht nur Kompilierung.

#include <gtest/gtest.h>

#include "adapters/geometry/occ_probe.h"
#include "adapters/persistence/sqlite_probe.h"
#include "adapters/ui/qt_probe.h"

TEST(AdapterProbes, QtGelinktUndVersionNichtLeer) {
    EXPECT_FALSE(bcad::adapters::ui::qt_version().empty());
}

TEST(AdapterProbes, OpenCascadeGelinktUndVersionNichtLeer) {
    EXPECT_FALSE(bcad::adapters::geometry::occ_version().empty());
}

TEST(AdapterProbes, SqliteGelinktUndVersionNichtLeer) {
    EXPECT_FALSE(bcad::adapters::persistence::sqlite_version().empty());
}
