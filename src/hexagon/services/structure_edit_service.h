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
#include "hexagon/ports/driving/evaluate_port.h"
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
                                   public ports::driving::ViewModelPort,
                                   public ports::driving::EvaluatePort {
public:
    explicit StructureEditService(const ports::driven::GeometryKernelPort& geometry);

    model::StoreyId addStorey(double height_mm) override;
    std::optional<model::WallId> addWall(model::StoreyId storey,
                                         model::Segment seg) override;
    ports::driving::ParamResult setWallThickness(model::WallId wall, double mm) override;
    ports::driving::ParamResult setWallHeight(model::WallId wall, double mm) override;

    // Wandöffnungen (LH-FA-DOR-*/WIN-*, ADR-0011): platzieren/verschieben/
    // Parameter/entfernen. Öffnungs-Mutationen bauen das Solid der
    // Wirtswand neu (transaktional) und melden `WallGeometryChanged` der
    // Wirtswand — KEINE Raum-Re-Detektion (Öffnung ändert weder Wandachse
    // noch Stärke; Raumerkennung/Footprint unberührt, ADR-0011 (5)).
    std::optional<model::OpeningId> addDoor(model::WallId wall,
                                            double offset_mm) override;
    std::optional<model::OpeningId> addWindow(model::WallId wall,
                                              double offset_mm) override;
    ports::driving::ParamResult setOpeningWidth(model::OpeningId opening,
                                                double mm) override;
    ports::driving::ParamResult setOpeningHeight(model::OpeningId opening,
                                                 double mm) override;
    ports::driving::ParamResult setWindowSill(model::OpeningId opening,
                                              double mm) override;
    bool moveOpening(model::OpeningId opening, double offset_mm) override;
    void setDoorSwing(model::OpeningId opening,
                      model::SwingDirection swing) override;
    bool removeOpening(model::OpeningId opening) override;

    // Dächer (LH-FA-ROF-*, ADR-0011 #6): Dach ist ein eigenständiges
    // Element; jede Mutation meldet `RoofChanged` (storey-bezogen),
    // KEINE RoomsChanged. Das Dach-Netz ist eine reine Query
    // (`roof_geometry`, total) — kein gespeichertes Solid, keine
    // Transaktion über externe Geometrie nötig.
    std::optional<model::RoofId> addRoof(const model::Roof& prototype) override;
    ports::driving::ParamResult setRoofPitch(model::RoofId roof,
                                             double deg) override;
    ports::driving::ParamResult setRoofOverhang(model::RoofId roof,
                                                double mm) override;
    void setRoofType(model::RoofId roof, model::RoofType type) override;
    bool removeRoof(model::RoofId roof) override;

    // Platten (LH-FA-SLB-*/FND-*, ADR-0011 #6): eigenständiges Element;
    // jede Mutation meldet `SlabChanged` (storey-bezogen), KEINE
    // RoomsChanged. Das Platten-Netz ist eine reine Query
    // (`slab_geometry` + Port + base_z-Translation); kein gespeichertes
    // Solid, das Volumen wird on demand gemessen.
    std::optional<model::SlabId> addSlab(const model::Slab& prototype) override;
    ports::driving::ParamResult setSlabThickness(model::SlabId slab,
                                                 double mm) override;
    bool addSlabCutout(model::SlabId slab,
                       const model::Footprint& cutout) override;
    bool removeSlab(model::SlabId slab) override;

    // Treppen (LH-FA-STR-*, ADR-0011 #6): gerade einläufige Treppe,
    // eigenständiges Element; jede Mutation meldet `StairChanged` (an die
    // untere Etage `from_storey`), KEINE RoomsChanged. Das Treppen-Netz ist
    // eine reine Query (`stair_geometry`, analytisch/total) — kein OCC, kein
    // gespeichertes Solid.
    std::optional<model::StairId> addStair(const model::Stair& prototype) override;
    ports::driving::ParamResult setStairWidth(model::StairId stair,
                                              double mm) override;
    ports::driving::ParamResult setStairStepCount(model::StairId stair,
                                                  int count) override;
    bool removeStair(model::StairId stair) override;

    // DetectRoomsPort (LH-FA-ROM-001): zuletzt erkannte Räume, reine Query.
    std::vector<model::Room> rooms(model::StoreyId storey) const override;

    // EvaluatePort (LH-FA-EVL-001/003, ADR-0012): read-only Flächen-Auswertung.
    // Reine Aggregation der Raum-Netto-Flächen (Room.net_area_mm2, ADR-0007),
    // mm² → m²; const ⇒ keine Re-Detektion, kein op, kein GeometryKernelPort.
    model::AreaReport floorArea(model::StoreyId storey) const override;
    model::AreaReport livingArea() const override;

    // ViewModelPort (LH-FA-D3-001, ADR-0009 (b)): tessellierter Stand für
    // die Darstellung — totale Queries (Tessellations-Fehler ⇒ leer),
    // on demand über den GeometryKernelPort.
    std::vector<ports::driving::WallMesh> wallMeshes() const override;
    std::optional<model::TriangleMesh> wallMesh(model::WallId id) const override;

    // ViewModelPort (LH-FA-ROF-*): Dach-Netze (analytisch, `roof_geometry`).
    std::vector<ports::driving::RoofMesh> roofMeshes() const override;

    // ViewModelPort (LH-FA-SLB-*/FND-*): Platten-Netze (Port-Extrusion +
    // base_z-Translation, `slab_geometry`).
    std::vector<ports::driving::SlabMesh> slabMeshes() const override;

    // ViewModelPort (LH-FA-STR-*): Treppen-Netze (analytisch, `stair_geometry`;
    // rise aus der `from_storey`-Höhe abgeleitet).
    std::vector<ports::driving::StairMesh> stairMeshes() const override;

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
    const std::vector<model::Opening>& openings() const {
        return building_.openings;
    }
    const model::Opening& opening(model::OpeningId id) const;
    const std::vector<model::Roof>& roofs() const { return building_.roofs; }
    const model::Roof& roof(model::RoofId id) const;
    const std::vector<model::Slab>& slabs() const { return building_.slabs; }
    const model::Slab& slab(model::SlabId id) const;
    const std::vector<model::Stair>& stairs() const { return building_.stairs; }
    const model::Stair& stair(model::StairId id) const;

private:
    // Neu zu bauende Nachbar-Solids (LH-FA-WAL-006: Eckenschluss macht
    // Nachbar-Footprints von der mutierten Wand abhängig).
    struct NeighborRebuild {
        model::WallId id{};
        model::StoreyId storey_id{};
        model::Solid solid{};
    };

    model::Wall& mutableWall(model::WallId id);
    void redetectRooms(model::StoreyId storey);
    void notifyListeners(const ports::driven::ModelChange& change);
    // Baut das Solid einer Wand inkl. ihrer Öffnungs-Schnittkörper
    // (Footprint-Extrusion minus Cutouts). Wirft bei E-GEO-002.
    model::Solid buildWallSolid(const model::Wall& w,
                                const std::vector<model::Wall>& walls,
                                const std::vector<model::Opening>& openings) const;
    // Übernimmt `trial_openings` und baut das Solid der Wirtswand
    // `wall_id` transaktional neu: gelingt der Bau, werden Öffnungen +
    // Solid committet und `WallGeometryChanged` der Wirtswand gemeldet;
    // schlägt er fehl (E-GEO-002), bleibt das Modell unverändert und es
    // ergeht keine Meldung. Gibt den Erfolg zurück.
    bool commitOpenings(model::WallId wall_id,
                        std::vector<model::Opening> trial_openings);
    model::Opening& mutableOpening(model::OpeningId id);
    std::optional<model::OpeningId> addOpening(model::Opening prototype,
                                               double offset_mm);
    model::Roof& mutableRoof(model::RoofId id);
    void notifyRoofChanged(model::StoreyId storey);
    model::Slab& mutableSlab(model::SlabId id);
    void notifySlabChanged(model::StoreyId storey);
    model::Stair& mutableStair(model::StairId id);
    void notifyStairChanged(model::StoreyId storey);
    double storeyHeight(model::StoreyId id) const;
    // Berechnet VOR dem Commit die Solids der Nachbarn, deren Footprint
    // sich durch `trial` ändert (wirft bei Geometrie-Fehler — dann
    // bleibt das Modell unverändert, transaktionale Garantie).
    std::vector<NeighborRebuild> rebuildAffectedNeighbors(
        const model::Wall& changed,
        const std::vector<model::Wall>& trial) const;
    void commitNeighborRebuilds(const std::vector<NeighborRebuild>& rebuilds);
    // Meldet die Nachbar-Folgeänderungen (Reihenfolge spez. §1: nach der
    // auslösenden Op, vor RoomsChanged).
    void notifyNeighborRebuilds(const std::vector<NeighborRebuild>& rebuilds);

    const ports::driven::GeometryKernelPort& geometry_;
    model::Building building_{};
    std::map<model::WallId, model::Solid> solids_{};
    std::map<model::StoreyId, std::vector<model::Room>> rooms_{};
    std::vector<ports::driven::ModelChangedPort*> listeners_{};
    int swallowed_listener_errors_{0};
    int next_storey_id_{1};
    int next_wall_id_{1};
    int next_opening_id_{1};
    int next_roof_id_{1};
    int next_slab_id_{1};
    int next_stair_id_{1};
};

}  // namespace bcad::hexagon::services
