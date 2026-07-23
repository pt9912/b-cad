#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/model/persisted_derivations.h"

namespace bcad::hexagon::ports::driven {

// Driven Port (ADR-0003): persistiert das Gebäudemodell. Implementierungen
// kapseln das Dateiformat (SQLite) vollständig — dieser Vertrag kennt nur
// das Domänen-Modell und `std::filesystem::path`. Fehler werden als
// neutrale `std::runtime_error` gemeldet; kein Backend-Typ verlässt den
// Adapter (analog `GeometryKernelPort`/E-GEO-002, ADR-0001).
class ProjectRepositoryPort {
public:
    virtual ~ProjectRepositoryPort() = default;

    // Speichert das vollständige Modell **atomar** (LH-FA-BLD-002). Bei
    // Schreibfehler bleibt der vorherige Dateistand unverändert (Temp +
    // Rename); geworfen wird eine neutrale `std::runtime_error`.
    // Die **kern-abgeleiteten** write-derived Skalare (`rise` je Treppe) reicht
    // der Aufrufer als `PersistedDerivations` mit — der Adapter **serialisiert
    // nur**, er leitet nichts mehr aus `services/geometry` ab (ADR-0020).
    virtual void save(const model::Building& building,
                      const model::PersistedDerivations& derived,
                      const std::filesystem::path& path) const = 0;

    // Lädt das vollständige Modell wieder (LH-FA-BLD-003). Wirft bei
    // fehlender/korrupter Datei eine neutrale `std::runtime_error`.
    virtual model::Building load(const std::filesystem::path& path) const = 0;
};

}  // namespace bcad::hexagon::ports::driven
