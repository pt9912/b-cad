#pragma once

#include <filesystem>

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
// Teil-Import/-Export). Hält die beiden Driven-Ports (Importer/Exporter)
// nicht-besitzend (Composition Root verdrahtet sie).
class ExchangeService final : public ports::driving::ExchangeModelPort {
public:
    ExchangeService(const ports::driven::ModelImporterPort& ifc_importer,
                    const ports::driven::ModelExporterPort& ifc_exporter);

    model::Building importModel(const std::filesystem::path& path,
                                ports::driving::ExchangeFormat format) const override;

    void exportModel(const model::Building& building,
                     const std::filesystem::path& path,
                     ports::driving::ExchangeFormat format) const override;

private:
    const ports::driven::ModelImporterPort& ifc_importer_;
    const ports::driven::ModelExporterPort& ifc_exporter_;
};

}  // namespace bcad::hexagon::services
