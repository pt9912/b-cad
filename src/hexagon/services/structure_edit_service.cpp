#include "hexagon/services/structure_edit_service.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "hexagon/model/constants.h"
#include "hexagon/services/room_detection.h"

namespace bcad::hexagon::services {

namespace {

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

    // Kandidat-Wand bauen und ZUERST extrudieren. Schlägt die Geometrie
    // fehl (Wurf), bleibt das Modell unverändert (E-GEO-002): erst nach
    // Erfolg wird die Wand übernommen — transaktional.
    model::Wall wall{};
    wall.storey_id = storey;
    wall.start = seg.start;
    wall.end = seg.end;
    wall.thickness_mm = model::kDefaultWallThicknessMm;  // LH-FA-WAL-001
    wall.height_mm = sit->height_mm;  // Default-Höhe = Geschosshöhe
    wall.type = model::WallType::Innen;
    const model::Solid solid = geometry_.extrudeWall(wall);

    const auto id = static_cast<model::WallId>(next_wall_id_++);
    wall.id = id;
    building_.walls.push_back(wall);
    solids_[id] = solid;
    redetectRooms(storey);  // LH-FA-ROM-001: automatisch beim Schließen
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
    // Transaktional: neues Solid zuerst berechnen, dann committen.
    model::Wall trial = target;
    trial.thickness_mm = result.applied_mm;
    const model::Solid solid = geometry_.extrudeWall(trial);
    target.thickness_mm = result.applied_mm;
    solids_[id] = solid;
    redetectRooms(target.storey_id);  // Stärke verschiebt die Innenkante
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
    model::Wall trial = target;
    trial.height_mm = result.applied_mm;
    const model::Solid solid = geometry_.extrudeWall(trial);
    target.height_mm = result.applied_mm;
    solids_[id] = solid;
    redetectRooms(target.storey_id);  // Mutation-Trigger (spez. §1 §Auslösung)
    return result;
}

std::vector<model::Room> StructureEditService::rooms(model::StoreyId storey) const {
    const auto it = rooms_.find(storey);
    return (it != rooms_.end()) ? it->second : std::vector<model::Room>{};
}

void StructureEditService::redetectRooms(model::StoreyId storey) {
    rooms_[storey] = detectRooms(building_, storey);
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
