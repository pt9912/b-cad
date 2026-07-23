// Import-/Export-Use-Case (Hexagon-Kern). Format-Dispatch über ein
// Exporter-Registry + Atomaritäts-Grenze; framework-frei (ADR-0001) — kennt nur
// Ports und das Domänen-Modell.

#include "hexagon/services/exchange_service.h"

#include <stdexcept>
#include <utility>

#include "hexagon/model/derived_geometry.h"

namespace bcad::hexagon::services {

ExchangeService::ExchangeService(ImporterMap importers, ExporterMap exporters)
    : importers_(std::move(importers)), exporters_(std::move(exporters)) {}

model::Building ExchangeService::importModel(
    const std::filesystem::path& path,
    ports::driving::ExchangeFormat format) const {
    const auto it = importers_.find(format);
    if (it == importers_.end() || it->second == nullptr) {
        // Format nicht für Import verdrahtet (z. B. STEP/STL export-only) →
        // wie nicht erkannt behandelt. **Beide** Token (E-IO-003 +
        // event=import_rejected), damit der export-only-Vertrag erhalten bleibt
        // (slice-021b MED-1).
        throw std::runtime_error(
            "E-IO-003: Format ist nicht für Import verdrahtet (export-only); "
            "event=import_rejected");
    }
    // Erfolg: vollständiges Building. Fehler: der Importer wirft E-IO-003
    // (event=import_rejected) — propagiert, kein Teil-Import.
    return it->second->read(path);
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
    // slice-042a (ADR-0020): der Kern reicht das abgeleitete-Geometrie-Bündel
    // über den Port — driven Adapter serialisieren nur, sie leiten keine
    // Geometrie ab. In dieser Stufe **leer** (accept-and-ignore); die
    // format-selektive Berechnung folgt mit der STEP/STL-Body-Migration (042c).
    const model::DerivedGeometry derived{};
    // Erfolg: vollständige Datei. Fehler: der Exporter wirft E-IO-001 (bzw.
    // E-IO-003) — propagiert, kein Teil-Export.
    it->second->write(building, derived, path);
}

}  // namespace bcad::hexagon::services
