// Plugin-Host-Implementierung (ADR-0017). Einziger Ort im Repo, der den
// System-Lader anspricht (arch-check Regel P1).
#include "adapters/plugin/plugin_host.h"

#include <dlfcn.h>

#include <cstdint>
#include <utility>

namespace bcad::adapters::plugin {

namespace {

// Sichtbare Meldungen in der §4-Konvention (spec/spezifikation.md):
// E-PLG-001, ein Code — zwei Log-Events (plugin_rejected | plugin_error).
std::string rejectedMessage(const std::string& path,
                            const std::string& reason) {
    return "E-PLG-001 Plugin abgelehnt (" + path + "): " + reason +
           "; event=plugin_rejected";
}

std::string errorMessage(const std::string& name,
                         const std::string& reason) {
    return "E-PLG-001 Plugin '" + name +
           "' isoliert, Modell unverändert: " + reason +
           "; event=plugin_error";
}

std::string dlErrorText() {
    const char* why = ::dlerror();
    return why != nullptr ? std::string(why) : std::string("unbekannt");
}

// Barriere um die Plugin-eigene Destroy-Funktion: liefert false statt zu
// werfen — der Fehlschlag wird vom Aufrufer in die Meldung aufgenommen
// (kein stilles Verschlucken), ändert aber die Isolierung nicht.
bool destroyWithBarrier(bcad::plugin_api::DestroyFn destroy,
                        bcad::plugin_api::Plugin* instance) {
    try {
        destroy(instance);
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

// Interner Zustand eines geladenen Plugins. Isolierte Plugins verbleiben
// NICHT in der Verwaltung: ihre Instanz ist (barriere-gesichert)
// freigegeben, ihr Library-Handle bleibt bewusst offen (Fehlerpfad ohne
// Entladen — kontrolliertes Belassen, ADR-0017).
struct PluginHost::LoadedPlugin {
    void* handle{nullptr};
    plugin_api::Plugin* instance{nullptr};
    plugin_api::DestroyFn destroy{nullptr};
    std::unique_ptr<plugin_api::PluginContext> context;
    std::string name;
    bool active{false};
};

PluginHost::PluginHost(hexagon::ports::driving::EditStructurePort& edit,
                       hexagon::ports::driving::EvaluatePort& evaluate)
    : edit_(edit), evaluate_(evaluate) {}

PluginHost::~PluginHost() { unloadAll(); }

PluginResult PluginHost::load(const std::string& path) {
    // Geladen: RTLD_NOW löst alle Referenzen sofort auf (fail-closed beim
    // Laden statt später beim Aufruf); RTLD_LOCAL hält Plugin-Symbole
    // privat — die Kern-Symbole liefert die exportierte Symboltabelle
    // des ladenden Executables (Symbol-Naht: ENABLE_EXPORTS).
    void* handle = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        return {false, {}, rejectedMessage(path, dlErrorText())};
    }

    // Handshake VOR jedem C++-Kontakt (exakte Versions-Gleichheit).
    // Ablehnungen ab hier belassen das Handle bewusst offen (Fehlerpfad
    // ohne Entladen — die statische Initialisierung des Plugins ist mit
    // dem Laden bereits gelaufen).
    auto version_fn = reinterpret_cast<plugin_api::AbiVersionFn>(
        ::dlsym(handle, plugin_api::kAbiVersionSymbol));
    auto create_fn = reinterpret_cast<plugin_api::CreateFn>(
        ::dlsym(handle, plugin_api::kCreateSymbol));
    auto destroy_fn = reinterpret_cast<plugin_api::DestroyFn>(
        ::dlsym(handle, plugin_api::kDestroySymbol));
    if (version_fn == nullptr || create_fn == nullptr ||
        destroy_fn == nullptr) {
        return {false, {},
                rejectedMessage(path, "Eintrittspunkt fehlt (kein Plugin "
                                      "im Sinne des Vertrags)")};
    }

    std::uint32_t found = 0;
    try {
        found = version_fn();
    } catch (...) {
        return {false, {},
                rejectedMessage(path, "Versions-Abfrage fehlgeschlagen")};
    }
    if (found != plugin_api::kAbiVersion) {
        return {false, {},
                rejectedMessage(
                    path, "Vertragsstand passt nicht: erwartet " +
                              std::to_string(plugin_api::kAbiVersion) +
                              ", vorgefunden " + std::to_string(found))};
    }

    // Initialisiert -> Aktiv: erst nach bestandenem Handshake entsteht
    // die C++-Instanz; jeder Übergang ist exception-gesichert.
    auto entry = std::make_unique<LoadedPlugin>();
    entry->handle = handle;
    entry->destroy = destroy_fn;
    entry->context =
        std::make_unique<plugin_api::PluginContext>(edit_, evaluate_);
    std::string name = path;  // bis der Plugin-Name bekannt ist
    try {
        entry->instance = create_fn();
        if (entry->instance == nullptr) {
            // Post-Handshake-Fehler (Init-Schritt) => plugin_error, nicht
            // plugin_rejected (Code-Review-MED-2; §4-Phasen-Zuordnung).
            entry->context->invalidate();
            return {false, name,
                    errorMessage(name, "Factory lieferte kein Plugin")};
        }
        name = entry->instance->name();
        entry->name = name;
        entry->instance->onLoad(*entry->context);
    } catch (const std::exception& e) {
        return {false, name,
                errorMessage(name, isolateAfterHookFailure(*entry) +
                                       e.what())};
    } catch (...) {
        return {false, name,
                errorMessage(name, isolateAfterHookFailure(*entry) +
                                       "Ausnahme unbekannten Typs im Hook")};
    }

    entry->active = true;
    PluginResult result{true, entry->name,
                        "Plugin '" + entry->name +
                            "' geladen (Vertragsstand " +
                            std::to_string(plugin_api::kAbiVersion) +
                            "): " + path};
    plugins_.push_back(std::move(entry));
    return result;
}

PluginResult PluginHost::unload(const std::string& plugin_name) {
    for (auto it = plugins_.begin(); it != plugins_.end(); ++it) {
        if (!(*it)->active || (*it)->name != plugin_name) {
            continue;
        }
        std::string error;
        const bool clean = shutdownAndClose(**it, &error);
        plugins_.erase(it);
        if (clean) {
            return {true, plugin_name,
                    "Plugin '" + plugin_name + "' entladen"};
        }
        return {false, plugin_name, error};
    }
    return {false, plugin_name,
            "Plugin '" + plugin_name + "' ist nicht geladen"};
}

void PluginHost::unloadAll() {
    // Zuletzt geladen -> zuerst entladen; Fehler isolieren nur das
    // jeweilige Plugin (kein Wurf aus dem Host, dtor-sicher).
    while (!plugins_.empty()) {
        std::string error;
        if (plugins_.back()->active) {
            shutdownAndClose(*plugins_.back(), &error);
        }
        plugins_.pop_back();
    }
}

std::size_t PluginHost::activeCount() const {
    std::size_t count = 0;
    for (const auto& entry : plugins_) {
        if (entry->active) {
            ++count;
        }
    }
    return count;
}

std::string PluginHost::isolateAfterHookFailure(LoadedPlugin& entry) {
    entry.context->invalidate();
    std::string prefix;
    if (entry.instance != nullptr) {
        if (!destroyWithBarrier(entry.destroy, entry.instance)) {
            prefix = "Destroy-Hook warf ebenfalls; ";
        }
        entry.instance = nullptr;
    }
    return prefix;
}

bool PluginHost::shutdownAndClose(LoadedPlugin& entry,
                                  std::string* error_message) {
    bool clean = true;
    try {
        entry.instance->onUnload();  // Beendet
    } catch (const std::exception& e) {
        clean = false;
        *error_message =
            errorMessage(entry.name,
                         std::string("Shutdown-Hook warf: ") + e.what());
    } catch (...) {
        clean = false;
        *error_message =
            errorMessage(entry.name, "Shutdown-Hook warf (unbekannter Typ)");
    }
    entry.context->invalidate();  // ab hier kein Port-Zugang mehr
    if (!destroyWithBarrier(entry.destroy, entry.instance)) {
        clean = false;
        if (error_message->empty()) {
            *error_message = errorMessage(entry.name, "Destroy warf");
        }
    }
    entry.instance = nullptr;
    entry.active = false;
    if (clean) {
        ::dlclose(entry.handle);  // Entladen — nur der reguläre Weg
        entry.handle = nullptr;
    }
    // Fehlerpfad: Handle bleibt bewusst offen (ADR-0017, Entscheidung des
    // Impl-Slice: Isolieren ohne Entladen).
    return clean;
}

}  // namespace bcad::adapters::plugin
