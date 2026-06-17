// Import-/Export-Use-Case (Hexagon-Kern). Format-Dispatch über ein
// Exporter-Registry + Atomaritäts-Grenze; framework-frei (ADR-0001) — kennt nur
// Ports und das Domänen-Modell.

#include "hexagon/services/exchange_service.h"

#include <stdexcept>
#include <utility>

namespace bcad::hexagon::services {

ExchangeService::ExchangeService(
    const ports::driven::ModelImporterPort& ifc_importer, ExporterMap exporters)
    : ifc_importer_(ifc_importer), exporters_(std::move(exporters)) {}

model::Building ExchangeService::importModel(
    const std::filesystem::path& path,
    ports::driving::ExchangeFormat format) const {
    switch (format) {
        case ports::driving::ExchangeFormat::Ifc:
            // Erfolg: vollständiges Building. Fehler: der Importer wirft
            // E-IO-003 (event=import_rejected) — propagiert, kein Teil-Import.
            return ifc_importer_.read(path);
        case ports::driving::ExchangeFormat::Step:
        case ports::driving::ExchangeFormat::Stl:
            // STEP/STL sind export-only (kein ModelImporterPort).
            throw std::runtime_error(
                "E-IO-003: Format ist export-only (kein Import); "
                "event=import_rejected");
    }
    // Unerreichbar bei gültigem Enum; defensiv (kein erkanntes Format).
    throw std::runtime_error(
        "E-IO-003: nicht unterstütztes Austauschformat; event=import_rejected");
}

void ExchangeService::exportModel(const model::Building& building,
                                  const std::filesystem::path& path,
                                  ports::driving::ExchangeFormat format) const {
    const auto it = exporters_.find(format);
    if (it == exporters_.end() || it->second == nullptr) {
        // Format nicht verdrahtet → wie nicht beschreibbar behandelt.
        throw std::runtime_error(
            "E-IO-001: nicht unterstütztes/unverdrahtetes Export-Format; "
            "event=io_no_permission");
    }
    // Erfolg: vollständige Datei. Fehler: der Exporter wirft E-IO-001 (bzw.
    // E-IO-003) — propagiert, kein Teil-Export.
    it->second->write(building, path);
}

}  // namespace bcad::hexagon::services
