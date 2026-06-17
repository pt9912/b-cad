#pragma once

#include <filesystem>
#include <map>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_exporter_port.h"
#include "hexagon/ports/driven/model_importer_port.h"
#include "hexagon/ports/driving/exchange_model_port.h"

namespace bcad::hexagon::services {

// Import-Use-Case (Hexagon-Kern, framework-frei, ADR-0001/0013): implementiert
// den Driving Port `ExchangeModelPort` und dispatcht je `ExchangeFormat` auf
// den passenden Driven `ModelImporterPort`. **Pure Domäne** — kein IFC-/
// SPF-Symbol (arch-check Regel A); das Format-Backend lebt im IO-Adapter.
//
// Atomarität (`LH-FA-IO-001`/`002` Negative): Import baut ein vollständiges
// `Building` oder wirft `E-IO-003`; Export schreibt die vollständige Datei oder
// wirft `E-IO-001` — der Service propagiert den Fehler unverändert (kein
// Teil-Import/-Export). Hält den Importer + ein **Exporter-Registry**
// (Format→Port) nicht-besitzend; der Composition Root verdrahtet je Format die
// passende `ModelExporterPort`-Implementierung (IFC io-resident, STEP/STL
// geometrie-resident — ADR-0014). STEP/STL sind **export-only**.
class ExchangeService final : public ports::driving::ExchangeModelPort {
public:
    // Format → Exporter-Implementierung (nicht-besitzende Zeiger; der
    // Composition Root hält die Adapter). Fehlt ein Format hier, ist Export für
    // dieses Format nicht verdrahtet (→ E-IO-001).
    using ExporterMap =
        std::map<ports::driving::ExchangeFormat,
                 const ports::driven::ModelExporterPort*>;

    ExchangeService(const ports::driven::ModelImporterPort& ifc_importer,
                    ExporterMap exporters);

    model::Building importModel(const std::filesystem::path& path,
                                ports::driving::ExchangeFormat format) const override;

    void exportModel(const model::Building& building,
                     const std::filesystem::path& path,
                     ports::driving::ExchangeFormat format) const override;

private:
    const ports::driven::ModelImporterPort& ifc_importer_;
    ExporterMap exporters_;
};

}  // namespace bcad::hexagon::services
