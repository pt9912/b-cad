// driving-Seite der ui (slice-029): implementiert die ui-interne
// MeshSource-Naht über den ViewModelPort — das EINZIGE
// driving-Port-Include des ui-Adapters (Richtungs-Reinheit: ui/command/
// = driving, ui/view/ = driven). Konvertiert die Port-Structs
// (WallMesh/RoofMesh/…) in die port-freien MeshSource-Rückgabetypen.
#pragma once

#include <map>
#include <optional>

#include "adapters/ui/view/mesh_source.h"
#include "hexagon/ports/driving/view_model_port.h"

namespace bcad::adapters::ui::command {

class ViewModelMeshSource final : public view::MeshSource {
public:
    explicit ViewModelMeshSource(
        const hexagon::ports::driving::ViewModelPort& view_model);

    std::map<hexagon::model::WallId, hexagon::model::TriangleMesh>
    allWallMeshes() const override;
    std::optional<hexagon::model::TriangleMesh> wallMesh(
        hexagon::model::WallId id) const override;
    std::map<hexagon::model::RoofId, hexagon::model::TriangleMesh>
    roofMeshes() const override;
    std::map<hexagon::model::SlabId, hexagon::model::TriangleMesh>
    slabMeshes() const override;
    std::map<hexagon::model::StairId, hexagon::model::TriangleMesh>
    stairMeshes() const override;

private:
    const hexagon::ports::driving::ViewModelPort& view_model_;
};

}  // namespace bcad::adapters::ui::command
