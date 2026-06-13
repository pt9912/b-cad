#include "adapters/ui/viewer_scene.h"

#include <map>
#include <optional>
#include <utility>

namespace bcad::adapters::ui {

namespace driven = hexagon::ports::driven;
namespace model = hexagon::model;

namespace {

bool meshEqual(const model::TriangleMesh& a, const model::TriangleMesh& b) {
    return a.positions == b.positions && a.normals == b.normals &&
           a.indices == b.indices;
}

}  // namespace

ViewerScene::ViewerScene(
    const hexagon::ports::driving::ViewModelPort& view_model)
    : view_model_(view_model) {}

void ViewerScene::loadAll() {
    meshes_.clear();
    for (hexagon::ports::driving::WallMesh& wall_mesh :
         view_model_.wallMeshes()) {
        meshes_[wall_mesh.wall_id] = std::move(wall_mesh.mesh);
    }
    roof_meshes_.clear();
    for (hexagon::ports::driving::RoofMesh& roof_mesh :
         view_model_.roofMeshes()) {
        roof_meshes_[roof_mesh.roof_id] = std::move(roof_mesh.mesh);
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
        case driven::ModelChangeOp::RoofChanged: {  // LH-FA-ROF-*
            // Storey-bezogene Meldung: Dächer neu pullen und idempotent
            // ersetzen. Der Zähler steigt um die Zahl TATSÄCHLICH
            // geänderter Dach-Netze (neu/ersetzt/entfernt); eine
            // identische erneute Meldung erzeugt kein Update (MED-2).
            std::map<model::RoofId, model::TriangleMesh> next;
            for (hexagon::ports::driving::RoofMesh& roof_mesh :
                 view_model_.roofMeshes()) {
                next[roof_mesh.roof_id] = std::move(roof_mesh.mesh);
            }
            int changed = 0;
            for (const auto& [id, mesh] : next) {
                const auto it = roof_meshes_.find(id);
                if (it == roof_meshes_.end() || !meshEqual(it->second, mesh)) {
                    ++changed;
                }
            }
            for (const auto& [id, mesh] : roof_meshes_) {
                if (next.find(id) == next.end()) {
                    ++changed;  // entfernt
                }
            }
            if (changed > 0) {
                roof_meshes_ = std::move(next);
                effective_updates_ += changed;
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
