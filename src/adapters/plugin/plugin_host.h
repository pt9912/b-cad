// Plugin-Host (ADR-0017): Driving Adapter — lädt Shared-Library-Plugins
// (REQ-TEC-008) über den System-Lader und vermittelt ihnen den
// Plugin-Kontext (Port-Subset v1: EditStructurePort + EvaluatePort).
//
// dlfcn-Monopol (arch-check Regel P1): dlopen/dlsym/dlclose leben
// AUSSCHLIESSLICH in diesem Adapter.
//
// Lifecycle (spec/spezifikation.md §1, LH-FA-PLG-003): Entdeckt ->
// Geladen -> Handshake -> Initialisiert -> Aktiv -> Beendet -> Entladen.
// Jeder Host->Plugin-Übergang ist exception-gesichert (Fehler-Barriere,
// Muster ADR-0008 "werfender Beobachter"); jeder Fehlerpfad isoliert das
// Plugin bei unverändertem Modell (E-PLG-001) — im Fehlerpfad OHNE
// dlclose (kontrolliertes Belassen, ADR-0017: Entlade-Restrisiken), nur
// der reguläre Weg entlädt vollständig (Shutdown-Hook ->
// Kontext-Invalidierung -> dlclose).
#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "hexagon/ports/driving/edit_structure_port.h"
#include "hexagon/ports/driving/evaluate_port.h"
#include "plugin_api/plugin.h"
#include "plugin_api/plugin_abi.h"
#include "plugin_api/plugin_context.h"

namespace bcad::adapters::plugin {

// Ergebnis eines Lade-/Entlade-Vorgangs. `message` trägt die sichtbare
// Annahme- bzw. Ablehnungs-/Fehler-Meldung (E-PLG-001-Konvention:
// "E-PLG-001 ...; event=plugin_rejected|plugin_error").
struct PluginResult {
    bool ok{false};
    std::string plugin_name;  // gesetzt, sobald der Name bekannt ist
    std::string message;
};

class PluginHost {
public:
    PluginHost(hexagon::ports::driving::EditStructurePort& edit,
               hexagon::ports::driving::EvaluatePort& evaluate);
    ~PluginHost();  // entlädt alle noch aktiven Plugins (regulärer Weg)

    PluginHost(const PluginHost&) = delete;
    PluginHost& operator=(const PluginHost&) = delete;
    PluginHost(PluginHost&&) = delete;
    PluginHost& operator=(PluginHost&&) = delete;

    // Lädt eine Plugin-Datei fail-closed: nicht ladbar / fehlendes
    // Symbol / ABI-Mismatch => Ablehnung OHNE Factory-/Init-Aufruf
    // (event=plugin_rejected; die Mismatch-Meldung nennt erwarteten und
    // vorgefundenen Vertragsstand). Wirft der onLoad-Hook, wird das
    // Plugin isoliert (event=plugin_error), das Modell bleibt
    // unverändert, der Host läuft weiter.
    PluginResult load(const std::string& path);

    // Entlädt ein aktives Plugin kontrolliert: onUnload-Hook ->
    // Kontext-Invalidierung -> dlclose. Wirft der Hook, wird isoliert
    // (ohne dlclose, event=plugin_error).
    PluginResult unload(const std::string& plugin_name);

    // Regulärer Abschluss aller aktiven Plugins (Reihenfolge: zuletzt
    // geladen zuerst entladen).
    void unloadAll();

    // Anzahl der AKTIVEN Plugins (isolierte zählen nicht).
    std::size_t activeCount() const;

private:
    struct LoadedPlugin;

    // Barriere-gesicherte Abwicklung eines aktiven Plugins; true, wenn
    // regulär entladen (sonst isoliert).
    bool shutdownAndClose(LoadedPlugin& entry, std::string* error_message);

    // Isolierung nach einem werfenden Hook (Fehlerpfad ohne Entladen):
    // Kontext entziehen, Instanz barriere-gesichert freigeben, Handle
    // bewusst offen lassen. Liefert den Meldungs-Präfix (nennt einen
    // zusätzlichen Destroy-Fehlschlag, statt ihn zu verschlucken).
    std::string isolateAfterHookFailure(LoadedPlugin& entry);

    hexagon::ports::driving::EditStructurePort& edit_;
    hexagon::ports::driving::EvaluatePort& evaluate_;
    std::vector<std::unique_ptr<LoadedPlugin>> plugins_;
};

}  // namespace bcad::adapters::plugin
