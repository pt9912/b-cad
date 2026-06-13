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

// Lädt eine storey-bezogene Element-Netz-Map neu (Pull) und ersetzt
// idempotent: liefert die Zahl TATSÄCHLICH geänderter Netze
// (neu/ersetzt/entfernt); bei 0 bleibt die Map unverändert (kein
// Flackern bei identischer Mehrfach-Meldung, MED-2). Gemeinsam für
// Dächer (`RoofChanged`) und Platten (`SlabChanged`).
template <typename Map, typename MeshList, typename IdOf>
int reloadKeyed(Map& current, MeshList fresh, IdOf id_of) {
    Map next;
    for (auto& item : fresh) {
        next[id_of(item)] = std::move(item.mesh);
    }
    int changed = 0;
    for (const auto& [id, mesh] : next) {
        const auto it = current.find(id);
        if (it == current.end() || !meshEqual(it->second, mesh)) {
            ++changed;
        }
    }
    for (const auto& [id, mesh] : current) {
        if (next.find(id) == next.end()) {
            ++changed;  // entfernt
        }
    }
    if (changed > 0) {
        current = std::move(next);
    }
    return changed;
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
    slab_meshes_.clear();
    for (hexagon::ports::driving::SlabMesh& slab_mesh :
         view_model_.slabMeshes()) {
        slab_meshes_[slab_mesh.slab_id] = std::move(slab_mesh.mesh);
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
        case driven::ModelChangeOp::RoofChanged:  // LH-FA-ROF-* (storey-bezogen)
            effective_updates_ += reloadKeyed(
                roof_meshes_, view_model_.roofMeshes(),
                [](const hexagon::ports::driving::RoofMesh& m) { return m.roof_id; });
            break;
        case driven::ModelChangeOp::SlabChanged:  // LH-FA-SLB-*/FND-*
            effective_updates_ += reloadKeyed(
                slab_meshes_, view_model_.slabMeshes(),
                [](const hexagon::ports::driving::SlabMesh& m) { return m.slab_id; });
            break;
        case driven::ModelChangeOp::StoreyAdded:
        case driven::ModelChangeOp::RoomsChanged:
            // Kein 3D-Szenen-Inhalt: Geschosse sind Container, Räume
            // bedient die 2D-Sicht — kein wirksames Update, kein
            // redundanter Rebuild (Idempotenz-AK).
            break;
    }
}

}  // namespace bcad::adapters::ui
