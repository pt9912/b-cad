// Test-Fixture (AK LH-FA-PLG-003/004, Code-Review-MED-1): der
// Shutdown-Hook wirft. Erwartung: der reguläre Entlade-Weg isoliert das
// Plugin (Fehlerpfad ohne Entladen), E-PLG-001/event=plugin_error, das
// Modell bleibt unverändert, der Host läuft weiter — sichert den
// Shutdown-Zweig der Fehler-Barriere computational ab.
#include <cstdint>
#include <stdexcept>
#include <string>

#include "plugin_api/plugin.h"
#include "plugin_api/plugin_abi.h"
#include "plugin_api/plugin_context.h"

namespace {

class UnloadThrowingPlugin : public bcad::plugin_api::Plugin {
public:
    std::string name() const override { return "bcad-unload-throwing"; }

    void onLoad(bcad::plugin_api::PluginContext& /*context*/) override {}

    void onUnload() override {
        throw std::runtime_error(
            "Test-Fehlverhalten: Shutdown-Hook wirft");
    }
};

}  // namespace

extern "C" std::uint32_t bcad_plugin_abi_version() {
    return bcad::plugin_api::kAbiVersion;
}

extern "C" bcad::plugin_api::Plugin* bcad_plugin_create() {
    return new UnloadThrowingPlugin;
}

extern "C" void bcad_plugin_destroy(bcad::plugin_api::Plugin* plugin) {
    delete plugin;
}
