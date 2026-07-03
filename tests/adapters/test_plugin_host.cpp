// AK-Tests Plugin-Host (slice-026b, ADR-0017; LH-FA-PLG-001..004):
// REALE .so-Fixtures durch den ECHTEN Host — kein Handshake-/Lifecycle-
// Stub als einziges Orakel (welle-2-Lehre slice-015b). Die Fixtures baut
// plugins/CMakeLists.txt als MODULE-Targets; den Ablageort liefert
// BCAD_TEST_PLUGIN_DIR (tests/CMakeLists.txt).
#include <gtest/gtest.h>

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/plugin/plugin_host.h"
#include "hexagon/services/structure_edit_service.h"
#include "plugin_api/plugin_context.h"

namespace {

namespace services = bcad::hexagon::services;
using bcad::adapters::plugin::PluginHost;

std::string pluginPath(const char* file) {
    return std::string(BCAD_TEST_PLUGIN_DIR) + "/" + file;
}

struct ModelProbe {
    std::size_t storeys{};
    std::size_t walls{};
};

ModelProbe probeModel(const services::StructureEditService& service) {
    return {service.building().storeys.size(),
            service.building().walls.size()};
}

class PluginHostTest : public ::testing::Test {
protected:
    bcad::adapters::geometry::OccGeometryAdapter geometry_;
    services::StructureEditService service_{geometry_};
    PluginHost host_{service_, service_};
};

// LH-FA-PLG-001 Happy + LH-FA-PLG-003: Laden ohne Neustart -> Wirkung
// verfügbar; Entladen -> keine weitere Wirkung, Anwendung läuft weiter;
// zweiter Zyklus übt den regulären Entlade-Pfad erneut.
TEST_F(PluginHostTest, LoadEditUnloadHappyPath) {
    const ModelProbe before = probeModel(service_);

    const auto load = host_.load(pluginPath("bcad_example_plugin.so"));
    ASSERT_TRUE(load.ok) << load.message;
    EXPECT_EQ(load.plugin_name, "bcad-example");
    EXPECT_NE(load.message.find("Vertragsstand"), std::string::npos);
    EXPECT_EQ(host_.activeCount(), 1U);

    // Wirkung über den vermittelten Driving-Port: +1 Geschoss, +1 Wand …
    const ModelProbe after_load = probeModel(service_);
    EXPECT_EQ(after_load.storeys, before.storeys + 1);
    EXPECT_EQ(after_load.walls, before.walls + 1);
    // … und die Plugin-Eingabe 20 mm wurde GEKLEMMT wie jede manuelle
    // Eingabe (LH-FA-PLG-004 „kein Nebeneingang"; Untergrenze 50 mm).
    EXPECT_DOUBLE_EQ(service_.building().walls.back().thickness_mm, 50.0);

    const auto unload = host_.unload("bcad-example");
    ASSERT_TRUE(unload.ok) << unload.message;
    EXPECT_EQ(host_.activeCount(), 0U);
    // Nach dem Entladen keine weitere Wirkung (Modell bleibt unverändert).
    const ModelProbe after_unload = probeModel(service_);
    EXPECT_EQ(after_unload.storeys, after_load.storeys);
    EXPECT_EQ(after_unload.walls, after_load.walls);

    // Zweiter Load/Unload-Zyklus (LH-FA-PLG-001: wiederholt ohne
    // Neustart; übt den dlclose-Happy-Pfad des ersten Zyklus).
    const auto again = host_.load(pluginPath("bcad_example_plugin.so"));
    ASSERT_TRUE(again.ok) << again.message;
    EXPECT_TRUE(host_.unload("bcad-example").ok);
}

// LH-FA-PLG-002 Negative: fremder Vertragsstand -> Ablehnung VOR jeder
// Wirkung; die Meldung nennt erwarteten und vorgefundenen Stand. Die
// Fixture-Factory bricht den Prozess ab — riefe der Host sie fälschlich,
// stürbe dieser Test laut statt still zu bestehen.
TEST_F(PluginHostTest, AbiMismatchRejectedWithoutInit) {
    const ModelProbe before = probeModel(service_);

    const auto result =
        host_.load(pluginPath("bcad_abi_mismatch_plugin.so"));
    ASSERT_FALSE(result.ok);
    EXPECT_NE(result.message.find("E-PLG-001"), std::string::npos);
    EXPECT_NE(result.message.find("event=plugin_rejected"),
              std::string::npos);
    EXPECT_NE(result.message.find("erwartet 1"), std::string::npos);
    EXPECT_NE(result.message.find("vorgefunden 1000"), std::string::npos);
    EXPECT_EQ(host_.activeCount(), 0U);

    const ModelProbe after = probeModel(service_);
    EXPECT_EQ(after.storeys, before.storeys);
    EXPECT_EQ(after.walls, before.walls);
}

// LH-FA-PLG-004: wohlgeformtes Fehlverhalten -> isoliert (E-PLG-001,
// event=plugin_error), Modell unverändert, Host lebt weiter. Zugleich
// der Symbol-Naht-Beleg (Entscheidung 1): die im Plugin geworfene
// std::runtime_error wird IM HOST gefangen und ihr Text transportiert
// (typeinfo-Unifikation über die Modul-Grenze).
TEST_F(PluginHostTest, ThrowingPluginIsolatedModelUnchanged) {
    const ModelProbe before = probeModel(service_);

    const auto result = host_.load(pluginPath("bcad_throwing_plugin.so"));
    ASSERT_FALSE(result.ok);
    EXPECT_NE(result.message.find("E-PLG-001"), std::string::npos);
    EXPECT_NE(result.message.find("event=plugin_error"), std::string::npos);
    EXPECT_NE(result.message.find("Test-Fehlverhalten"), std::string::npos);
    EXPECT_EQ(host_.activeCount(), 0U);

    const ModelProbe after = probeModel(service_);
    EXPECT_EQ(after.storeys, before.storeys);
    EXPECT_EQ(after.walls, before.walls);

    // Host lebt weiter: Folge-Operation funktioniert.
    const auto follow_up = host_.load(pluginPath("bcad_example_plugin.so"));
    ASSERT_TRUE(follow_up.ok) << follow_up.message;
    EXPECT_TRUE(host_.unload("bcad-example").ok);
}

// LH-FA-PLG-001 Boundary: keine ladbare Plugin-Datei -> sichtbare
// Ablehnung ohne Absturz, Modell unverändert.
TEST_F(PluginHostTest, GarbageFileRejectedWithoutCrash) {
    const std::string path = ::testing::TempDir() + "kein_plugin.so";
    {
        std::ofstream out(path, std::ios::binary);
        out << "Dies ist keine Shared Library";
    }
    const ModelProbe before = probeModel(service_);

    const auto result = host_.load(path);
    ASSERT_FALSE(result.ok);
    EXPECT_NE(result.message.find("event=plugin_rejected"),
              std::string::npos);
    EXPECT_EQ(probeModel(service_).walls, before.walls);
}

// LH-FA-PLG-002: valide Shared Library OHNE die vertraglichen
// Eintrittspunkte -> Ablehnung („kein Plugin im Sinne des Vertrags").
TEST_F(PluginHostTest, MissingEntryPointsRejected) {
    const auto result = host_.load(pluginPath("bcad_no_symbols_plugin.so"));
    ASSERT_FALSE(result.ok);
    EXPECT_NE(result.message.find("Eintrittspunkt fehlt"),
              std::string::npos);
    EXPECT_NE(result.message.find("event=plugin_rejected"),
              std::string::npos);
}

// LH-FA-PLG-003/004 (Code-Review-MED-1): Fehler im SHUTDOWN-Schritt —
// der reguläre Entlade-Weg isoliert (Fehlerpfad ohne Entladen),
// E-PLG-001/event=plugin_error, Modell unverändert, Host lebt weiter.
// Sichert den Shutdown-Zweig der Fehler-Barriere ab (ohne ihn liefe die
// Plugin-Exception im PluginHost-Destruktor in std::terminate).
TEST_F(PluginHostTest, UnloadHookThrowIsolatesPlugin) {
    const auto load =
        host_.load(pluginPath("bcad_unload_throwing_plugin.so"));
    ASSERT_TRUE(load.ok) << load.message;
    const ModelProbe before = probeModel(service_);

    const auto unload = host_.unload("bcad-unload-throwing");
    ASSERT_FALSE(unload.ok);
    EXPECT_NE(unload.message.find("E-PLG-001"), std::string::npos);
    EXPECT_NE(unload.message.find("event=plugin_error"), std::string::npos);
    EXPECT_NE(unload.message.find("Shutdown-Hook warf"), std::string::npos);
    EXPECT_EQ(host_.activeCount(), 0U);
    EXPECT_EQ(probeModel(service_).walls, before.walls);
    EXPECT_EQ(probeModel(service_).storeys, before.storeys);

    // Host lebt weiter: Folge-Operation funktioniert.
    const auto follow_up = host_.load(pluginPath("bcad_example_plugin.so"));
    ASSERT_TRUE(follow_up.ok) << follow_up.message;
    EXPECT_TRUE(host_.unload("bcad-example").ok);
}

// Entladen eines unbekannten Namens: klare Meldung, kein Wurf.
TEST_F(PluginHostTest, UnloadUnknownNameFails) {
    const auto result = host_.unload("gibt-es-nicht");
    EXPECT_FALSE(result.ok);
    EXPECT_NE(result.message.find("nicht geladen"), std::string::npos);
}

// LH-FA-PLG-003: nach der Invalidierung wirft jeder Kontext-Zugriff
// beobachtbar (kein UB-Pfad) — direkte Sonde am Kontext-Objekt.
TEST(PluginContextTest, InvalidatedContextThrowsOnAccess) {
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService service(geometry);
    bcad::plugin_api::PluginContext context(service, service);

    EXPECT_TRUE(context.valid());
    EXPECT_NO_THROW(static_cast<void>(context.evaluate().livingArea()));

    context.invalidate();
    EXPECT_FALSE(context.valid());
    EXPECT_THROW(static_cast<void>(context.edit()), std::logic_error);
    EXPECT_THROW(static_cast<void>(context.evaluate()), std::logic_error);
}

}  // namespace
