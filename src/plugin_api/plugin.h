// Plugin-Interface (ADR-0017, Entscheidung 2): pure-virtual C++-Facade
// hinter dem bestandenen extern-"C"-Handshake. Hooks laufen synchron im
// Hauptthread; Port-Aufrufe sind NUR aus dem Hook-Kontext zulässig
// (Plugin-eigene Threads rufen keine Ports — Vertragspflicht,
// spec/spezifikation.md §1).
#pragma once

#include <string>

namespace bcad::plugin_api {

class PluginContext;

class Plugin {
public:
    virtual ~Plugin() = default;

    // Metadaten: stabiler, menschenlesbarer Plugin-Name (für Meldungen
    // und den Entlade-Aufruf).
    [[nodiscard]] virtual std::string name() const = 0;

    // Initialisiert -> Aktiv: der Kontext bleibt bis onUnload gültig.
    // Wirft der Hook, wird das Plugin isoliert (E-PLG-001), das Modell
    // bleibt unverändert.
    virtual void onLoad(PluginContext& context) = 0;

    // Beendet: nach der Rückkehr darf das Plugin KEINE Port-/Kontext-
    // Referenz mehr halten (der Host invalidiert den Kontext danach).
    virtual void onUnload() = 0;
};

}  // namespace bcad::plugin_api
