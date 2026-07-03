// Plugin-ABI-Vertrag (ADR-0017, Entscheidung 2): versionierter
// extern-"C"-Handshake als Eintrittstür jedes Plugins. Der Host prüft
// die ABI-Version VOR dem ersten C++-Aufruf auf EXAKTE Gleichheit —
// Mismatch, fehlendes Symbol oder nicht ladbare Datei => Ablehnung ohne
// Initialisierung (E-PLG-001, fail-closed).
//
// Die ABI-Version steigt bei JEDER inkompatiblen Änderung des
// Plugin-Vertrags (Plugin/PluginContext-Interfaces, Port-Subset) und bei
// einem Toolchain-Hub (ADR-0004-Beschluss).
#pragma once

#include <cstdint>

namespace bcad::plugin_api {

class Plugin;

// Vertragsstand v1: EditStructurePort + EvaluatePort im Kontext.
inline constexpr std::uint32_t kAbiVersion = 1;

// Symbolnamen der drei extern-"C"-Eintrittspunkte, die jedes Plugin
// exportiert (der Host löst sie über den System-Lader auf):
inline constexpr const char* kAbiVersionSymbol = "bcad_plugin_abi_version";
inline constexpr const char* kCreateSymbol = "bcad_plugin_create";
inline constexpr const char* kDestroySymbol = "bcad_plugin_destroy";

// Signaturen hinter den Symbolen.
using AbiVersionFn = std::uint32_t (*)();
using CreateFn = Plugin* (*)();
using DestroyFn = void (*)(Plugin*);

}  // namespace bcad::plugin_api
