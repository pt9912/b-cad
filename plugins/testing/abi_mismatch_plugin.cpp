// Test-Fixture (AK LH-FA-PLG-002, Plugin-API): meldet einen fremden
// Vertragsstand. Erwartung: Ablehnung VOR jeder Wirkung — der Host darf
// die Factory NIE rufen. Fail-loud-Sonde: die Factory bricht den Prozess
// ab; riefe ein fehlerhafter Host sie doch, stirbt der Test laut statt
// still zu bestehen.
#include <cstdint>
#include <cstdlib>

#include "plugin_api/plugin.h"
#include "plugin_api/plugin_abi.h"

extern "C" std::uint32_t bcad_plugin_abi_version() {
    return bcad::plugin_api::kAbiVersion + 999;
}

extern "C" bcad::plugin_api::Plugin* bcad_plugin_create() {
    std::abort();  // Ablehnung ohne Initialisierung verletzt -> laut
}

extern "C" void bcad_plugin_destroy(bcad::plugin_api::Plugin* /*plugin*/) {
    std::abort();
}
