#pragma once

#include <filesystem>

#include "hexagon/model/building.h"
#include "hexagon/model/persisted_derivations.h"
#include "hexagon/ports/driven/project_repository_port.h"

namespace bcad::adapters::persistence {

// Driven Adapter (ADR-0003): erfüllt `ProjectRepositoryPort` über SQLite.
// Die SQLite-Typen bleiben vollständig in der `.cpp` gekapselt — dieser
// Header bindet KEIN `sqlite3.h` ein (a-check Regel D). Schreiben ist
// atomar (Temp-Datei + `fsync` + `rename`, LH-FA-BLD-002 Boundary); der
// Crash-Recovery-Nachweis (`kill -9`, LH-QA-005) folgt in slice-008b.
class SqliteProjectRepository final
    : public hexagon::ports::driven::ProjectRepositoryPort {
public:
    void save(const hexagon::model::Building& building,
              const hexagon::model::PersistedDerivations& derived,
              const std::filesystem::path& path) const override;

    hexagon::model::Building load(
        const std::filesystem::path& path) const override;
};

}  // namespace bcad::adapters::persistence
