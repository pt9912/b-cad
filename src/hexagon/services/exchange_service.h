#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/ports/driven/model_importer_port.h"
#include "hexagon/ports/driving/exchange_model_port.h"

namespace bcad::hexagon::services {

// Import-Use-Case (Hexagon-Kern, framework-frei, ADR-0001/0013): implementiert
// den Driving Port `ExchangeModelPort` und dispatcht je `ExchangeFormat` auf
// den passenden Driven `ModelImporterPort`. **Pure Domäne** — kein IFC-/
// SPF-Symbol (arch-check Regel A); das Format-Backend lebt im IO-Adapter.
//
// Atomarität (`LH-FA-IO-001` Negative): der Importer baut entweder ein
// vollständiges `Building` oder wirft `E-IO-003` (`event=import_rejected`); der
// Service gibt das Modell nur im Erfolgsfall zurück und propagiert den Fehler
// sonst unverändert — kein Teil-Import (vorheriger Projektstand intakt).
class ExchangeService final : public ports::driving::ExchangeModelPort {
public:
    explicit ExchangeService(const ports::driven::ModelImporterPort& ifc_importer);

    model::Building importModel(const std::filesystem::path& path,
                                ports::driving::ExchangeFormat format) const override;

private:
    const ports::driven::ModelImporterPort& ifc_importer_;
};

}  // namespace bcad::hexagon::services
