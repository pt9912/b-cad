#include "adapters/ui/command/view_model_mesh_source.h"

#include <utility>
#include <vector>

namespace bcad::adapters::ui::command {

namespace {

// Port-Struct-Liste ({id, mesh}) -> port-freie Modelltyp-Map (die
// MeshSource-Rückgabeform der Naht, slice-029 Review-MED-1).
template <typename Id, typename Mesh, typename IdOf>
std::map<Id, hexagon::model::TriangleMesh> toMap(std::vector<Mesh> items,
                                                 IdOf id_of) {
    std::map<Id, hexagon::model::TriangleMesh> out;
    for (Mesh& item : items) {
        out[id_of(item)] = std::move(item.mesh);
    }
    return out;
}

}  // namespace

ViewModelMeshSource::ViewModelMeshSource(
    const hexagon::ports::driving::ViewModelPort& view_model)
    : view_model_(view_model) {}

std::map<hexagon::model::WallId, hexagon::model::TriangleMesh>
ViewModelMeshSource::allWallMeshes() const {
    return toMap<hexagon::model::WallId>(
        view_model_.wallMeshes(),
        [](const auto& m) { return m.wall_id; });
}

std::optional<hexagon::model::TriangleMesh> ViewModelMeshSource::wallMesh(
    hexagon::model::WallId id) const {
    return view_model_.wallMesh(id);
}

std::map<hexagon::model::RoofId, hexagon::model::TriangleMesh>
ViewModelMeshSource::roofMeshes() const {
    return toMap<hexagon::model::RoofId>(
        view_model_.roofMeshes(),
        [](const auto& m) { return m.roof_id; });
}

std::map<hexagon::model::SlabId, hexagon::model::TriangleMesh>
ViewModelMeshSource::slabMeshes() const {
    return toMap<hexagon::model::SlabId>(
        view_model_.slabMeshes(),
        [](const auto& m) { return m.slab_id; });
}

std::map<hexagon::model::StairId, hexagon::model::TriangleMesh>
ViewModelMeshSource::stairMeshes() const {
    return toMap<hexagon::model::StairId>(
        view_model_.stairMeshes(),
        [](const auto& m) { return m.stair_id; });
}

}  // namespace bcad::adapters::ui::command
