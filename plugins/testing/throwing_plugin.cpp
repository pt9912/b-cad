// Test-Fixture (AK LH-FA-PLG-004, Sandbox): wohlgeformtes Fehlverhalten —
// der onLoad-Hook wirft VOR jeder Mutation. Erwartung: Fehler-Barriere
// fängt die Ausnahme im Host (E-PLG-001, event=plugin_error), das Modell
// bleibt unverändert, der Host läuft weiter. Zugleich der empirische
// Symbol-Naht-Beleg (Exception-Typ trägt über die Modul-Grenze).
#include <cstdint>
#include <stdexcept>
#include <string>

#include "plugin_api/plugin.h"
#include "plugin_api/plugin_abi.h"
#include "plugin_api/plugin_context.h"

namespace {

class ThrowingPlugin : public bcad::plugin_api::Plugin {
public:
    [[nodiscard]] std::string name() const override { return "bcad-throwing"; }

    void onLoad(bcad::plugin_api::PluginContext& /*context*/) override {
        throw std::runtime_error(
            "Test-Fehlverhalten: Hook wirft vor jeder Mutation");
    }

    void onUnload() override {}
};

}  // namespace

extern "C" std::uint32_t bcad_plugin_abi_version() {
    return bcad::plugin_api::kAbiVersion;
}

extern "C" bcad::plugin_api::Plugin* bcad_plugin_create() {
    return new ThrowingPlugin;
}

extern "C" void bcad_plugin_destroy(bcad::plugin_api::Plugin* plugin) {
    delete plugin;
}
