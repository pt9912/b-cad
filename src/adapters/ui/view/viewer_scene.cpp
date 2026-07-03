#include "adapters/ui/view/viewer_scene.h"

#include <map>
#include <optional>
#include <utility>

namespace bcad::adapters::ui::view {

namespace driven = hexagon::ports::driven;
namespace model = hexagon::model;

namespace {

bool meshEqual(const model::TriangleMesh& a, const model::TriangleMesh& b) {
    return a.positions == b.positions && a.normals == b.normals &&
           a.indices == b.indices;
}

// Lädt eine Element-Netz-Map neu (Pull über die MeshSource-Naht) und
// ersetzt idempotent: liefert die Zahl TATSÄCHLICH geänderter Netze
// (neu/ersetzt/entfernt); bei 0 bleibt die Map unverändert (kein
// Flackern bei identischer Mehrfach-Meldung, MED-2). Gemeinsam für
// Dächer (`RoofChanged`), Platten (`SlabChanged`) und Treppen.
template <typename Map>
int reloadKeyed(Map& current, Map fresh) {
    int changed = 0;
    for (const auto& [id, mesh] : fresh) {
        const auto it = current.find(id);
        if (it == current.end() || !meshEqual(it->second, mesh)) {
            ++changed;
        }
    }
    for (const auto& [id, mesh] : current) {
        if (fresh.find(id) == fresh.end()) {
            ++changed;  // entfernt
        }
    }
    if (changed > 0) {
        current = std::move(fresh);
    }
    return changed;
}

}  // namespace

ViewerScene::ViewerScene(const MeshSource& mesh_source)
    : mesh_source_(mesh_source) {}

void ViewerScene::loadAll() {
    meshes_ = mesh_source_.allWallMeshes();
    roof_meshes_ = mesh_source_.roofMeshes();
    slab_meshes_ = mesh_source_.slabMeshes();
    stair_meshes_ = mesh_source_.stairMeshes();
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
                mesh_source_.wallMesh(change.wall_id);
            if (mesh.has_value()) {
                meshes_[change.wall_id] = std::move(*mesh);
                ++effective_updates_;
            }
            break;
        }
        case driven::ModelChangeOp::RoofChanged:  // LH-FA-ROF-* (storey-bezogen)
            effective_updates_ +=
                reloadKeyed(roof_meshes_, mesh_source_.roofMeshes());
            break;
        case driven::ModelChangeOp::SlabChanged:  // LH-FA-SLB-*/FND-*
            effective_updates_ +=
                reloadKeyed(slab_meshes_, mesh_source_.slabMeshes());
            break;
        case driven::ModelChangeOp::StairChanged:  // LH-FA-STR-* (projektweit)
            effective_updates_ +=
                reloadKeyed(stair_meshes_, mesh_source_.stairMeshes());
            break;
        case driven::ModelChangeOp::StoreyAdded:
        case driven::ModelChangeOp::RoomsChanged:
            // Kein 3D-Szenen-Inhalt: Geschosse sind Container, Räume
            // bedient die 2D-Sicht — kein wirksames Update, kein
            // redundanter Rebuild (Idempotenz-AK).
            break;
    }
}

}  // namespace bcad::adapters::ui::view
