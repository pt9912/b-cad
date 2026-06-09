#pragma once

#include <map>
#include <optional>

#include "hexagon/model/building.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/solid.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"
#include "hexagon/ports/driving/edit_structure_port.h"

namespace bcad::hexagon::services {

// Anwendungslogik (Hexagon-Kern, framework-frei): implementiert den
// Driving Port `EditStructurePort`, validiert/klemmt Parameter
// (LH-FA-WAL-002/003, E-VAL-001) und baut nach jeder Änderung das Solid
// über den Driven Port `GeometryKernelPort` neu (Rebuild). Bei Anlage
// existiert genau ein Default-Geschoss (LH-FA-BLD-001, Domänen-Teil).
class StructureEditService final : public ports::driving::EditStructurePort {
public:
    explicit StructureEditService(const ports::driven::GeometryKernelPort& geometry);

    model::StoreyId addStorey(double height_mm) override;
    std::optional<model::WallId> addWall(model::StoreyId storey,
                                         model::Segment seg) override;
    ports::driving::ParamResult setWallThickness(model::WallId wall, double mm) override;
    ports::driving::ParamResult setWallHeight(model::WallId wall, double mm) override;

    // Queries (für Konsumenten/Tests; nicht Teil des Command-Ports).
    const model::Building& building() const { return building_; }
    const model::Wall& wall(model::WallId id) const;
    const model::Solid& wallSolid(model::WallId id) const;

private:
    model::Wall& mutableWall(model::WallId id);

    const ports::driven::GeometryKernelPort& geometry_;
    model::Building building_{};
    std::map<model::WallId, model::Solid> solids_{};
    int next_storey_id_{1};
    int next_wall_id_{1};
};

}  // namespace bcad::hexagon::services
