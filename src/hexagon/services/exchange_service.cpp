// Import-Use-Case (Hexagon-Kern). Format-Dispatch + Atomaritäts-Grenze;
// framework-frei (ADR-0001) — kennt nur Ports und das Domänen-Modell.

#include "hexagon/services/exchange_service.h"

#include <stdexcept>

namespace bcad::hexagon::services {

ExchangeService::ExchangeService(
    const ports::driven::ModelImporterPort& ifc_importer)
    : ifc_importer_(ifc_importer) {}

model::Building ExchangeService::importModel(
    const std::filesystem::path& path,
    ports::driving::ExchangeFormat format) const {
    switch (format) {
        case ports::driving::ExchangeFormat::Ifc:
            // Erfolg: vollständiges Building. Fehler: der Importer wirft
            // E-IO-003 (event=import_rejected) — propagiert, kein Teil-Import.
            return ifc_importer_.read(path);
    }
    // Unerreichbar bei gültigem Enum; defensiv (kein erkanntes Format).
    throw std::runtime_error(
        "E-IO-003: nicht unterstütztes Austauschformat; event=import_rejected");
}

}  // namespace bcad::hexagon::services
