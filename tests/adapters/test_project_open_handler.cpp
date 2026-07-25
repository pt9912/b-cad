// slice-047b (LH-FA-BLD-002/003): der GUI-Weg „Datei → Speichern/Öffnen"
// über den **Handler**, nicht über den modalen Dialog (Plan-Review MED-1 —
// `QFileDialog` lebt im coverage-ausgenommenen main und wird bewusst nicht
// getestet). Geprüft wird der real beobachtbare Teil: ein gespeichertes
// Projekt wird über `openProject` geladen, der Service ersetzt das Modell,
// und die **abonnierten** Sichten spiegeln den geladenen Stand — der
// per-op-Viewer nur, weil `ModelReplaced` ihn voll neu aufbauen lässt
// (Plan-Review HIGH-1).
//
// Echter Adapter-Pfad (welle-2-Lehre slice-015b): echte SQLite-Datei, echter
// OCC-Geometrie-Adapter — kein Stub als einziges Orakel.

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <string>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/persistence/sqlite_project_repository.h"
#include "adapters/ui/command/view_model_mesh_source.h"
#include "adapters/ui/view/viewer_scene.h"
#include "hexagon/model/segment.h"
#include "hexagon/services/manage_project.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace fs = std::filesystem;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

fs::path tempProjectPath(const std::string& name) {
    return fs::temp_directory_path() / name;
}

// Speichern → Öffnen über die Handler; die abonnierte Szene spiegelt den
// geladenen Stand (Full-Refresh statt stale Netze).
TEST(ProjectOpenHandler_LH_FA_BLD_003, SavedProjectOpensAndViewsFollow) {
    const bcad::adapters::persistence::SqliteProjectRepository repository;
    const fs::path path = tempProjectPath("bcad_047b_roundtrip.bcad");
    fs::remove(path);

    // --- Quell-Projekt bauen und speichern (Handler-Weg) ---
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    {
        services::StructureEditService source(geometry);
        const auto eg = source.building().storeys.front().id;
        ASSERT_TRUE(source.addWall(eg, seg(0, 0, 4000, 0)).has_value());
        ASSERT_TRUE(source.addWall(eg, seg(4000, 0, 4000, 3000)).has_value());
        services::saveProject(repository, source.building(), path);
    }
    ASSERT_TRUE(fs::exists(path)) << "Speichern muss eine Datei erzeugen";

    // --- Ziel-Service mit ABWEICHENDEM Stand + abonnierter Szene ---
    // Bewusst MEHR Wände als das geladene Projekt (3 statt 2): nur so ist die
    // Zusicherung diskriminierend. Bliebe der Full-Refresh aus, hielte die
    // Szene weiterhin 3 Netze — die Id-Mengen überlappen sich (beide Services
    // vergeben ab 1), ein Vergleich einzelner Ids würde also nichts beweisen.
    services::StructureEditService target(geometry);
    const auto other_storey = target.building().storeys.front().id;
    ASSERT_TRUE(target.addWall(other_storey, seg(0, 0, 1000, 0)).has_value());
    ASSERT_TRUE(target.addWall(other_storey, seg(1000, 0, 1000, 800)).has_value());
    const auto stale_wall = target.addWall(other_storey, seg(1000, 800, 0, 800));
    ASSERT_TRUE(stale_wall.has_value());

    bcad::adapters::ui::command::ViewModelMeshSource mesh_source(target);
    bcad::adapters::ui::view::ViewerScene scene(mesh_source);
    target.subscribe(scene);
    scene.loadAll();
    ASSERT_EQ(scene.wallMeshes().size(), 3U) << "Ausgangs-Szene: drei Wände";

    // --- Öffnen über den Handler ---
    services::openProject(target, repository, path);

    // Modell entspricht dem gespeicherten Stand.
    ASSERT_EQ(target.building().walls.size(), 2U);

    // Die Szene spiegelt GENAU ihn — OHNE ModelReplaced-Fall stünden hier noch
    // drei Netze (der per-op-Pfad pullt nur gemeldete Elemente).
    EXPECT_EQ(scene.wallMeshes().size(), 2U)
        << "ModelReplaced muss den Viewer voll neu aufbauen";
    EXPECT_EQ(scene.wallMeshes().count(*stale_wall), 0U)
        << "das Netz der dritten (nur im alten Stand vorhandenen) Wand muss weg sein";
    for (const auto& w : target.building().walls) {
        EXPECT_EQ(scene.wallMeshes().count(w.id), 1U)
            << "jede geladene Wand muss ein Netz haben";
    }

    target.unsubscribe(scene);
    fs::remove(path);
}

// Fehlerpfad des Öffnen-Handlers: nicht existente Datei → neutraler Wurf,
// Modell + Szene unverändert (der GUI-Aufrufer zeigt das als Dialog).
TEST(ProjectOpenHandler_LH_FA_BLD_003, MissingFileThrowsAndLeavesStateIntact) {
    const bcad::adapters::persistence::SqliteProjectRepository repository;
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService target(geometry);
    const auto eg = target.building().storeys.front().id;
    ASSERT_TRUE(target.addWall(eg, seg(0, 0, 2000, 0)).has_value());

    bcad::adapters::ui::command::ViewModelMeshSource mesh_source(target);
    bcad::adapters::ui::view::ViewerScene scene(mesh_source);
    target.subscribe(scene);
    scene.loadAll();

    const auto walls_before = target.building().walls.size();
    const auto meshes_before = scene.wallMeshes().size();

    EXPECT_THROW(
        services::openProject(target, repository,
                              tempProjectPath("bcad_047b_gibt_es_nicht.bcad")),
        std::runtime_error);

    EXPECT_EQ(target.building().walls.size(), walls_before);
    EXPECT_EQ(scene.wallMeshes().size(), meshes_before);
    target.unsubscribe(scene);
}

}  // namespace
