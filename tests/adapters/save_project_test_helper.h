#pragma once

#include <filesystem>

#include "adapters/persistence/sqlite_project_repository.h"
#include "hexagon/model/building.h"
#include "hexagon/services/manage_project.h"

namespace bcad::adapters::persistence::test {

// Dünner Test-Aufrufer: **delegiert** an den promoteten Produktions-Save-Use-Case
// `hexagon::services::saveProject` (slice-047a — die Naht ist jetzt Produktionscode,
// nicht mehr test-lokal). `SqliteProjectRepository` ist ein `ProjectRepositoryPort`.
inline void saveProject(const SqliteProjectRepository& repo,
                        const hexagon::model::Building& building,
                        const std::filesystem::path& path) {
    hexagon::services::saveProject(repo, building, path);
}

}  // namespace bcad::adapters::persistence::test
