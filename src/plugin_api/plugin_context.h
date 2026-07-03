// Plugin-Kontext (ADR-0017, Entscheidung 3/5 — Port-Vermittlung als
// Sandbox-Kern): reicht Plugins AUSSCHLIESSLICH Driving-Port-Referenzen.
// Vertragsstand v1: EditStructurePort (editierend) + EvaluatePort
// (lesend). Kein Driven-Port, kein Beobachter-Zugang, kein Durchgriff
// auf Adapter oder Modell-Interna.
//
// Invalidierbar: der Host invalidiert den Kontext beim Entladen bzw.
// Isolieren — jeder spätere Port-Zugriff wirft (beobachtbare
// Vertragsverletzung statt undefiniertes Verhalten).
#pragma once

#include <stdexcept>

#include "hexagon/ports/driving/edit_structure_port.h"
#include "hexagon/ports/driving/evaluate_port.h"

namespace bcad::plugin_api {

class PluginContext {
public:
    PluginContext(hexagon::ports::driving::EditStructurePort& edit,
                  hexagon::ports::driving::EvaluatePort& evaluate)
        : edit_(&edit), evaluate_(&evaluate) {}

    // Editierender Zugang — dieselbe Validierung/Klemmung (E-VAL-001)
    // wie jede manuelle Eingabe (kein Nebeneingang).
    hexagon::ports::driving::EditStructurePort& edit() const {
        ensureValid();
        return *edit_;
    }

    // Lesender Zugang (pull-only; Plugins sind nur in ihren Hooks aktiv).
    hexagon::ports::driving::EvaluatePort& evaluate() const {
        ensureValid();
        return *evaluate_;
    }

    // Host-seitig: entzieht dem Plugin jeden weiteren Port-Zugang
    // (Entladen/Isolieren). Nicht für Plugins gedacht.
    void invalidate() {
        edit_ = nullptr;
        evaluate_ = nullptr;
    }

    bool valid() const { return edit_ != nullptr; }

private:
    void ensureValid() const {
        if (edit_ == nullptr) {
            throw std::logic_error(
                "PluginContext invalidiert: Port-Zugriff nach dem "
                "Entladen/Isolieren ist eine Vertragsverletzung");
        }
    }

    hexagon::ports::driving::EditStructurePort* edit_;
    hexagon::ports::driving::EvaluatePort* evaluate_;
};

}  // namespace bcad::plugin_api
