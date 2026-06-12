#include "hexagon/services/structure_edit_service.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "hexagon/model/constants.h"
#include "hexagon/services/room_detection.h"
#include "hexagon/services/wall_footprint.h"

namespace bcad::hexagon::services {

namespace {

bool nearPoint(model::Point2D a, model::Point2D b) {
    return std::abs(a.x_mm - b.x_mm) < model::kGeometryToleranceMm &&
           std::abs(a.y_mm - b.y_mm) < model::kGeometryToleranceMm;
}

bool sharesEndpoint(const model::Wall& a, const model::Wall& b) {
    return nearPoint(a.start, b.start) || nearPoint(a.start, b.end) ||
           nearPoint(a.end, b.start) || nearPoint(a.end, b.end);
}

bool footprintsEqual(const model::Footprint& a, const model::Footprint& b) {
    if (a.points.size() != b.points.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.points.size(); ++i) {
        if (!nearPoint(a.points[i], b.points[i])) {
            return false;
        }
    }
    return true;
}

// Geschlossenes Intervall [min, max] für die Parameter-Klemmung. Eigener
// Typ, damit `evaluateParam` kein vertauschbares Doppel `(double, double)`
// als Nachbar-Parameter trägt.
struct Range {
    double min;
    double max;
};

// Bewertet eine Parameter-Eingabe gegen [min, max] OHNE Seiteneffekt.
// `current` ist der bestehende Wert (Rückgabe bei Ablehnung). Reihenfolge
// (value, range, current) hält keine zwei gleichtypigen Nachbar-Parameter.
ports::driving::ParamResult evaluateParam(double value, Range range, double current) {
    // Nicht-endliche Eingaben (NaN/Inf) sind ungültig — nicht klemmbar.
    // Modell bleibt unverändert (Rejected), damit kein NaN-Wert/-Solid
    // gespeichert wird (spez. §1 LH-FA-WAL-002.a: „Modell-Zustand bleibt
    // gültig").
    if (!std::isfinite(value)) {
        return ports::driving::ParamResult{current, ports::driving::ParamStatus::Rejected};
    }
    const double applied = std::clamp(value, range.min, range.max);
    const auto status = (applied != value) ? ports::driving::ParamStatus::Clamped
                                           : ports::driving::ParamStatus::Accepted;
    return ports::driving::ParamResult{applied, status};
}

double segmentLength(const model::Segment& seg) {
    const double dx = seg.end.x_mm - seg.start.x_mm;
    const double dy = seg.end.y_mm - seg.start.y_mm;
    return std::sqrt((dx * dx) + (dy * dy));
}

}  // namespace

StructureEditService::StructureEditService(
    const ports::driven::GeometryKernelPort& geometry)
    : geometry_(geometry) {
    // LH-FA-BLD-001 (Domänen-Teil): leeres Gebäude mit genau einem
    // Default-Geschoss (EG, Default-Höhe aus spezifikation §3).
    building_.storeys.push_back(model::Storey{
        static_cast<model::StoreyId>(next_storey_id_++), model::kDefaultStoreyHeightMm});
}

model::StoreyId StructureEditService::addStorey(double height_mm) {
    const auto id = static_cast<model::StoreyId>(next_storey_id_++);
    building_.storeys.push_back(model::Storey{id, height_mm});
    notifyListeners({.op = ports::driven::ModelChangeOp::StoreyAdded,
                     .storey_id = id});  // ADR-0008 §Umfang
    return id;
}

std::optional<model::WallId> StructureEditService::addWall(
    model::StoreyId storey, model::Segment seg) {
    // LH-FA-WAL-001 Boundary: nur endliche Segmente oberhalb der Toleranz
    // (verwirft Null-Länge UND nicht-endliche Koordinaten).
    const double length = segmentLength(seg);
    if (!std::isfinite(length) || length < model::kGeometryToleranceMm) {
        return std::nullopt;
    }

    const auto sit = std::find_if(
        building_.storeys.begin(), building_.storeys.end(),
        [storey](const model::Storey& s) { return s.id == storey; });
    if (sit == building_.storeys.end()) {
        throw std::out_of_range("addWall: unbekannte Geschoss-Id");
    }

    // Kandidat-Wand bauen und ZUERST extrudieren (inkl. der Nachbarn,
    // deren Eck-Geometrie sich ändert — LH-FA-WAL-006). Schlägt eine
    // Geometrie-Operation fehl (Wurf), bleibt das Modell unverändert
    // (E-GEO-002): erst nach Erfolg wird committet — transaktional.
    model::Wall wall{};
    wall.id = static_cast<model::WallId>(next_wall_id_);  // Commit zählt hoch
    wall.storey_id = storey;
    wall.start = seg.start;
    wall.end = seg.end;
    wall.thickness_mm = model::kDefaultWallThicknessMm;  // LH-FA-WAL-001
    wall.height_mm = sit->height_mm;  // Default-Höhe = Geschosshöhe
    wall.type = model::WallType::Innen;

    std::vector<model::Wall> trial = building_.walls;
    trial.push_back(wall);
    const model::Solid solid =
        geometry_.extrudeFootprint(wallFootprint(wall, trial), wall.height_mm);
    const std::vector<NeighborRebuild> neighbor_rebuilds =
        rebuildAffectedNeighbors(wall, trial);

    ++next_wall_id_;
    const model::WallId id = wall.id;
    building_.walls.push_back(wall);
    solids_[id] = solid;
    commitNeighborRebuilds(neighbor_rebuilds);
    redetectRooms(storey);  // LH-FA-ROM-001: automatisch beim Schließen
    // ADR-0008 #4: Meldung NACH allen Post-Commit-Schritten; Reihenfolge
    // spez. §1: auslösende Op → Nachbarn einzeln → RoomsChanged.
    notifyListeners({.op = ports::driven::ModelChangeOp::WallAdded,
                     .storey_id = storey,
                     .wall_id = id});
    notifyNeighborRebuilds(neighbor_rebuilds);
    notifyListeners({.op = ports::driven::ModelChangeOp::RoomsChanged,
                     .storey_id = storey});
    return id;
}

ports::driving::ParamResult StructureEditService::setWallThickness(
    model::WallId id, double mm) {
    model::Wall& target = mutableWall(id);
    const ports::driving::ParamResult result = evaluateParam(
        mm, Range{model::kWallThicknessMinMm, model::kWallThicknessMaxMm},
        target.thickness_mm);
    if (result.status == ports::driving::ParamStatus::Rejected) {
        return result;  // Modell unverändert
    }
    // Transaktional: neue Solids (Wand + betroffene Eck-Nachbarn,
    // LH-FA-WAL-006) zuerst berechnen, dann committen.
    std::vector<model::Wall> trial_walls = building_.walls;
    auto tit = std::find_if(trial_walls.begin(), trial_walls.end(),
                            [id](const model::Wall& w) { return w.id == id; });
    tit->thickness_mm = result.applied_mm;
    const model::Solid solid =
        geometry_.extrudeFootprint(wallFootprint(*tit, trial_walls), tit->height_mm);
    const std::vector<NeighborRebuild> neighbor_rebuilds =
        rebuildAffectedNeighbors(*tit, trial_walls);

    target.thickness_mm = result.applied_mm;
    solids_[id] = solid;
    commitNeighborRebuilds(neighbor_rebuilds);
    redetectRooms(target.storey_id);  // Stärke verschiebt die Innenkante
    notifyListeners({.op = ports::driven::ModelChangeOp::WallThicknessChanged,
                     .storey_id = target.storey_id,
                     .wall_id = id});
    notifyNeighborRebuilds(neighbor_rebuilds);
    notifyListeners({.op = ports::driven::ModelChangeOp::RoomsChanged,
                     .storey_id = target.storey_id});
    return result;
}

ports::driving::ParamResult StructureEditService::setWallHeight(
    model::WallId id, double mm) {
    model::Wall& target = mutableWall(id);
    const ports::driving::ParamResult result = evaluateParam(
        mm, Range{model::kWallHeightMinMm, model::kWallHeightMaxMm},
        target.height_mm);
    if (result.status == ports::driving::ParamStatus::Rejected) {
        return result;  // Modell unverändert
    }
    // Transaktional: neues Solid zuerst berechnen, dann committen.
    // Höhe ändert den Footprint NICHT — keine Nachbar-Folgeänderung
    // (LH-FA-WAL-006/spez. §1).
    model::Wall trial = target;
    trial.height_mm = result.applied_mm;
    const model::Solid solid = geometry_.extrudeFootprint(
        wallFootprint(trial, building_.walls), trial.height_mm);
    target.height_mm = result.applied_mm;
    solids_[id] = solid;
    redetectRooms(target.storey_id);  // Mutation-Trigger (spez. §1 §Auslösung)
    notifyListeners({.op = ports::driven::ModelChangeOp::WallHeightChanged,
                     .storey_id = target.storey_id,
                     .wall_id = id});
    notifyListeners({.op = ports::driven::ModelChangeOp::RoomsChanged,
                     .storey_id = target.storey_id});
    return result;
}

std::vector<model::Room> StructureEditService::rooms(model::StoreyId storey) const {
    const auto it = rooms_.find(storey);
    return (it != rooms_.end()) ? it->second : std::vector<model::Room>{};
}

std::vector<ports::driving::WallMesh> StructureEditService::wallMeshes() const {
    std::vector<ports::driving::WallMesh> meshes;
    meshes.reserve(building_.walls.size());
    for (const model::Wall& w : building_.walls) {
        if (std::optional<model::TriangleMesh> mesh = wallMesh(w.id)) {
            meshes.push_back(ports::driving::WallMesh{w.id, std::move(*mesh)});
        }
    }
    return meshes;
}

std::optional<model::TriangleMesh> StructureEditService::wallMesh(
    model::WallId id) const {
    const auto it = std::find_if(
        building_.walls.begin(), building_.walls.end(),
        [id](const model::Wall& w) { return w.id == id; });
    if (it == building_.walls.end()) {
        return std::nullopt;  // unbekannte Id — Query ist total
    }
    try {
        return geometry_.tessellateFootprint(
            wallFootprint(*it, building_.walls), it->height_mm);
    } catch (const std::exception&) {
        // E-GEO-002 in der Query-Hälfte: totale Antwort statt Wurf —
        // committete Wände sind validiert, ein Tessellations-Fehler darf
        // die Darstellungs-Abfrage nicht kippen (ADR-0009 (b)).
        return std::nullopt;
    }
}

void StructureEditService::redetectRooms(model::StoreyId storey) {
    rooms_[storey] = detectRooms(building_, storey);
}

std::vector<StructureEditService::NeighborRebuild>
StructureEditService::rebuildAffectedNeighbors(
    const model::Wall& changed, const std::vector<model::Wall>& trial) const {
    std::vector<NeighborRebuild> rebuilds;
    for (const model::Wall& other : trial) {
        if (other.id == changed.id || other.storey_id != changed.storey_id ||
            !sharesEndpoint(other, changed)) {
            continue;
        }
        // Nur Nachbarn, deren Footprint sich WIRKLICH ändert (auch der
        // Übergang Miter↔Stumpf beim Grad-Wechsel 1↔2↔3) — keine
        // Über-Meldung (spez. §1 D3-002.a).
        const model::Footprint before = wallFootprint(other, building_.walls);
        const model::Footprint after = wallFootprint(other, trial);
        if (footprintsEqual(before, after)) {
            continue;
        }
        rebuilds.push_back(NeighborRebuild{
            other.id, other.storey_id,
            geometry_.extrudeFootprint(after, other.height_mm)});
    }
    return rebuilds;
}

void StructureEditService::commitNeighborRebuilds(
    const std::vector<NeighborRebuild>& rebuilds) {
    for (const NeighborRebuild& rebuild : rebuilds) {
        solids_[rebuild.id] = rebuild.solid;
    }
}

void StructureEditService::notifyNeighborRebuilds(
    const std::vector<NeighborRebuild>& rebuilds) {
    for (const NeighborRebuild& rebuild : rebuilds) {
        notifyListeners(
            {.op = ports::driven::ModelChangeOp::WallGeometryChanged,
             .storey_id = rebuild.storey_id,
             .wall_id = rebuild.id});
    }
}

void StructureEditService::subscribe(ports::driven::ModelChangedPort& listener) {
    if (std::find(listeners_.begin(), listeners_.end(), &listener) ==
        listeners_.end()) {
        listeners_.push_back(&listener);
    }
}

void StructureEditService::unsubscribe(ports::driven::ModelChangedPort& listener) {
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), &listener),
        listeners_.end());
}

void StructureEditService::notifyListeners(
    const ports::driven::ModelChange& change) {
    // Kopie der Liste: ein Beobachter darf sich im Callback abmelden,
    // ohne die Iteration zu invalidieren.
    const std::vector<ports::driven::ModelChangedPort*> snapshot = listeners_;
    for (ports::driven::ModelChangedPort* listener : snapshot) {
        try {
            listener->onModelChanged(change);
        } catch (...) {
            // ADR-0008 #6 (Kapselung): ein werfender Beobachter kippt die
            // committete Mutation nicht und blockiert Folge-Beobachter
            // nicht. Der Fehler bleibt über den Zähler beobachtbar;
            // Telemetrie-Anschluss folgt mit REQ-TEC-006.
            ++swallowed_listener_errors_;
        }
    }
}

const model::Wall& StructureEditService::wall(model::WallId id) const {
    const auto it = std::find_if(
        building_.walls.begin(), building_.walls.end(),
        [id](const model::Wall& w) { return w.id == id; });
    if (it == building_.walls.end()) {
        throw std::out_of_range("wall: unbekannte Wand-Id");
    }
    return *it;
}

const model::Solid& StructureEditService::wallSolid(model::WallId id) const {
    return solids_.at(id);
}

model::Wall& StructureEditService::mutableWall(model::WallId id) {
    const auto it = std::find_if(
        building_.walls.begin(), building_.walls.end(),
        [id](const model::Wall& w) { return w.id == id; });
    if (it == building_.walls.end()) {
        throw std::out_of_range("mutableWall: unbekannte Wand-Id");
    }
    return *it;
}

}  // namespace bcad::hexagon::services
