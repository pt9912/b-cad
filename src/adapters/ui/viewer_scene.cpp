#include "adapters/ui/viewer_scene.h"

#include <optional>
#include <utility>

namespace bcad::adapters::ui {

namespace driven = hexagon::ports::driven;

ViewerScene::ViewerScene(
    const hexagon::ports::driving::ViewModelPort& view_model)
    : view_model_(view_model) {}

void ViewerScene::loadAll() {
    meshes_.clear();
    for (hexagon::ports::driving::WallMesh& wall_mesh :
         view_model_.wallMeshes()) {
        meshes_[wall_mesh.wall_id] = std::move(wall_mesh.mesh);
    }
}

void ViewerScene::onModelChanged(const driven::ModelChange& change) {
    switch (change.op) {
        case driven::ModelChangeOp::WallAdded:
        case driven::ModelChangeOp::WallThicknessChanged:
        case driven::ModelChangeOp::WallHeightChanged:
        case driven::ModelChangeOp::WallGeometryChanged: {  // LH-FA-WAL-006
            // Pull-State (ADR-0008): aktualisierten Stand der gemeldeten
            // Wand holen. Leere Antwort (unbekannte Id/Tessellations-
            // Fehler) lässt die Szene unverändert — der Callback wirft
            // nicht (ADR-0008 #6).
            std::optional<hexagon::model::TriangleMesh> mesh =
                view_model_.wallMesh(change.wall_id);
            if (mesh.has_value()) {
                meshes_[change.wall_id] = std::move(*mesh);
                ++effective_updates_;
            }
            break;
        }
        case driven::ModelChangeOp::StoreyAdded:
        case driven::ModelChangeOp::RoomsChanged:
            // Kein 3D-Szenen-Inhalt: Geschosse sind Container, Räume
            // bedient die 2D-Sicht — kein wirksames Update, kein
            // redundanter Rebuild (Idempotenz-AK).
            break;
    }
}

}  // namespace bcad::adapters::ui
