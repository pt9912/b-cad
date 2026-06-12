#pragma once

#include <map>
#include <optional>
#include <vector>

#include "hexagon/model/building.h"
#include "hexagon/model/room.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/solid.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/ports/driving/detect_rooms_port.h"
#include "hexagon/ports/driving/edit_structure_port.h"
#include "hexagon/ports/driving/view_model_port.h"

namespace bcad::hexagon::services {

// Anwendungslogik (Hexagon-Kern, framework-frei): implementiert den
// Driving Port `EditStructurePort`, validiert/klemmt Parameter
// (LH-FA-WAL-002/003, E-VAL-001) und baut nach jeder Änderung das Solid
// über den Driven Port `GeometryKernelPort` neu (Rebuild). Bei Anlage
// existiert genau ein Default-Geschoss (LH-FA-BLD-001, Domänen-Teil).
//
// Räume (LH-FA-ROM-001, ADR-0007) werden AUTOMATISCH bei jeder
// Wand-Mutation neu erkannt („when er geschlossen wird") — der
// `DetectRoomsPort` ist reine Abfrage des zuletzt erkannten Stands
// (spez. §1 §Auslösung). Die Erkennung ist total (wirft nicht) und
// läuft erst NACH dem transaktionalen Commit der Mutation.
class StructureEditService final : public ports::driving::EditStructurePort,
                                   public ports::driving::DetectRoomsPort,
                                   public ports::driving::ViewModelPort {
public:
    explicit StructureEditService(const ports::driven::GeometryKernelPort& geometry);

    model::StoreyId addStorey(double height_mm) override;
    std::optional<model::WallId> addWall(model::StoreyId storey,
                                         model::Segment seg) override;
    ports::driving::ParamResult setWallThickness(model::WallId wall, double mm) override;
    ports::driving::ParamResult setWallHeight(model::WallId wall, double mm) override;

    // DetectRoomsPort (LH-FA-ROM-001): zuletzt erkannte Räume, reine Query.
    std::vector<model::Room> rooms(model::StoreyId storey) const override;

    // ViewModelPort (LH-FA-D3-001, ADR-0009 (b)): tessellierter Stand für
    // die Darstellung — totale Queries (Tessellations-Fehler ⇒ leer),
    // on demand über den GeometryKernelPort.
    std::vector<ports::driving::WallMesh> wallMeshes() const override;
    std::optional<model::TriangleMesh> wallMesh(model::WallId id) const override;

    // ADR-0008 (LH-FA-D3-002): Beobachter-Registrierung — mehrfach,
    // nicht-besitzend; Beobachter melden sich vor ihrer Zerstörung ab.
    void subscribe(ports::driven::ModelChangedPort& listener);
    void unsubscribe(ports::driven::ModelChangedPort& listener);

    // Verschluckte Beobachter-Fehler (ADR-0008 #6: Kapselung) — sichtbar
    // für Tests; Telemetrie-Anschluss folgt mit REQ-TEC-006.
    int swallowedListenerErrors() const { return swallowed_listener_errors_; }

    // Queries (für Konsumenten/Tests; nicht Teil des Command-Ports).
    const model::Building& building() const { return building_; }
    const model::Wall& wall(model::WallId id) const;
    const model::Solid& wallSolid(model::WallId id) const;

private:
    model::Wall& mutableWall(model::WallId id);
    void redetectRooms(model::StoreyId storey);
    void notifyListeners(const ports::driven::ModelChange& change);

    const ports::driven::GeometryKernelPort& geometry_;
    model::Building building_{};
    std::map<model::WallId, model::Solid> solids_{};
    std::map<model::StoreyId, std::vector<model::Room>> rooms_{};
    std::vector<ports::driven::ModelChangedPort*> listeners_{};
    int swallowed_listener_errors_{0};
    int next_storey_id_{1};
    int next_wall_id_{1};
};

}  // namespace bcad::hexagon::services
