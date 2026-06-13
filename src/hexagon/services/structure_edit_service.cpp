#include "hexagon/services/structure_edit_service.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "hexagon/model/constants.h"
#include "hexagon/services/opening_geometry.h"
#include "hexagon/services/roof_geometry.h"
#include "hexagon/services/room_detection.h"
#include "hexagon/services/slab_geometry.h"
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

double wallLength(const model::Wall& wall) {
    return segmentLength(model::Segment{wall.start, wall.end});
}

// Kind-spezifische Wertebereiche für Öffnungs-Parameter (spez. §3).
Range openingWidthRange(model::OpeningKind kind) {
    return (kind == model::OpeningKind::Door)
               ? Range{model::kDoorWidthMinMm, model::kDoorWidthMaxMm}
               : Range{model::kWindowWidthMinMm, model::kWindowWidthMaxMm};
}

Range openingHeightRange(model::OpeningKind kind) {
    return (kind == model::OpeningKind::Door)
               ? Range{model::kDoorHeightMinMm, model::kDoorHeightMaxMm}
               : Range{model::kWindowHeightMinMm, model::kWindowHeightMaxMm};
}

// Dicke-/Tiefe-Bereich je Platten-Typ (spez. §3).
Range slabThicknessRange(model::SlabType type) {
    return (type == model::SlabType::Decke)
               ? Range{model::kSlabThicknessMinMm, model::kSlabThicknessMaxMm}
               : Range{model::kFoundationDepthMinMm, model::kFoundationDepthMaxMm};
}

// Vorzeichenlose Polygon-Fläche (Shoelace) — Degenerations-Prüfung des
// Platten-Grundrisses.
double polygonArea(const model::Footprint& footprint) {
    double twice = 0.0;
    const std::size_t n = footprint.points.size();
    for (std::size_t i = 0; i < n; ++i) {
        const model::Point2D& a = footprint.points[i];
        const model::Point2D& b = footprint.points[(i + 1) % n];
        twice += (a.x_mm * b.y_mm) - (b.x_mm * a.y_mm);
    }
    return std::abs(twice) * 0.5;
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
    const model::Solid solid = buildWallSolid(wall, trial, building_.openings);
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
    const model::Solid solid = buildWallSolid(*tit, trial_walls, building_.openings);
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
    const model::Solid solid =
        buildWallSolid(trial, building_.walls, building_.openings);
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
            wallFootprint(*it, building_.walls), it->height_mm,
            wallCutPrisms(*it, building_.openings));
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
            buildWallSolid(other, trial, building_.openings)});
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

// --- Wandöffnungen (LH-FA-DOR-*/WIN-*, ADR-0011) ---

model::Solid StructureEditService::buildWallSolid(
    const model::Wall& w, const std::vector<model::Wall>& walls,
    const std::vector<model::Opening>& openings) const {
    return geometry_.extrudeFootprint(wallFootprint(w, walls), w.height_mm,
                                      wallCutPrisms(w, openings));
}

bool StructureEditService::commitOpenings(
    model::WallId wall_id, std::vector<model::Opening> trial_openings) {
    const model::Wall& host = wall(wall_id);
    model::Solid solid{};
    try {
        // Transaktional: neues Wirtswand-Solid VOR dem Commit bauen.
        solid = buildWallSolid(host, building_.walls, trial_openings);
    } catch (const std::exception&) {
        return false;  // E-GEO-002: Modell bleibt unverändert, keine Meldung
    }
    building_.openings = std::move(trial_openings);
    solids_[wall_id] = solid;
    // ADR-0011 (4): Öffnung ist ein Hohlraum der Wirtswand → deren
    // `WallGeometryChanged`; KEINE Raum-Re-Detektion (ADR-0011 (5)).
    notifyListeners({.op = ports::driven::ModelChangeOp::WallGeometryChanged,
                     .storey_id = host.storey_id,
                     .wall_id = wall_id});
    return true;
}

std::optional<model::OpeningId> StructureEditService::addOpening(
    model::Opening prototype, double offset_mm) {
    const auto wit = std::find_if(
        building_.walls.begin(), building_.walls.end(),
        [&](const model::Wall& w) { return w.id == prototype.wall_id; });
    if (wit == building_.walls.end()) {
        return std::nullopt;  // keine Wirtswand (Negative)
    }
    const double length = wallLength(*wit);
    const Range wr = openingWidthRange(prototype.kind);
    if (!std::isfinite(length) || length < wr.min) {
        return std::nullopt;  // Wand zu kurz für die schmalste Öffnung
    }
    // Breite: Default in den Bereich geklemmt, dann auf die Wandlänge
    // verkürzt; Position so geklemmt, dass die Öffnung in der Wand liegt.
    const double width = std::min(std::clamp(prototype.width_mm, wr.min, wr.max),
                                  length);
    prototype.width_mm = width;
    prototype.offset_mm = std::clamp(offset_mm, 0.0, length - width);
    prototype.id = static_cast<model::OpeningId>(next_opening_id_);

    std::vector<model::Opening> trial = building_.openings;
    trial.push_back(prototype);
    if (!commitOpenings(prototype.wall_id, std::move(trial))) {
        return std::nullopt;  // E-GEO-002
    }
    ++next_opening_id_;
    return prototype.id;
}

std::optional<model::OpeningId> StructureEditService::addDoor(
    model::WallId wall_id, double offset_mm) {
    model::Opening proto{};
    proto.wall_id = wall_id;
    proto.kind = model::OpeningKind::Door;
    proto.width_mm = model::kDefaultDoorWidthMm;
    proto.height_mm = model::kDefaultDoorHeightMm;
    proto.sill_height_mm = 0.0;  // Türen: keine Brüstung
    proto.swing = model::SwingDirection::Left;
    return addOpening(proto, offset_mm);
}

std::optional<model::OpeningId> StructureEditService::addWindow(
    model::WallId wall_id, double offset_mm) {
    model::Opening proto{};
    proto.wall_id = wall_id;
    proto.kind = model::OpeningKind::Window;
    proto.width_mm = model::kDefaultWindowWidthMm;
    proto.height_mm = model::kDefaultWindowHeightMm;
    proto.sill_height_mm = model::kDefaultWindowSillMm;  // LH-FA-WIN-004
    return addOpening(proto, offset_mm);
}

ports::driving::ParamResult StructureEditService::setOpeningWidth(
    model::OpeningId id, double mm) {
    model::Opening& target = mutableOpening(id);
    const double length = wallLength(wall(target.wall_id));
    const ports::driving::ParamResult result =
        evaluateParam(mm, openingWidthRange(target.kind), target.width_mm);
    if (result.status == ports::driving::ParamStatus::Rejected ||
        result.applied_mm > length) {
        // ungültig oder breiter als die Wand → Modell unverändert
        return ports::driving::ParamResult{target.width_mm,
                                           ports::driving::ParamStatus::Rejected};
    }
    std::vector<model::Opening> trial = building_.openings;
    auto& t = *std::find_if(trial.begin(), trial.end(),
                            [id](const model::Opening& o) { return o.id == id; });
    t.width_mm = result.applied_mm;
    t.offset_mm = std::clamp(t.offset_mm, 0.0, length - result.applied_mm);
    if (!commitOpenings(target.wall_id, std::move(trial))) {
        return ports::driving::ParamResult{target.width_mm,
                                           ports::driving::ParamStatus::Rejected};
    }
    return result;
}

ports::driving::ParamResult StructureEditService::setOpeningHeight(
    model::OpeningId id, double mm) {
    model::Opening& target = mutableOpening(id);
    const ports::driving::ParamResult result =
        evaluateParam(mm, openingHeightRange(target.kind), target.height_mm);
    if (result.status == ports::driving::ParamStatus::Rejected) {
        return result;
    }
    std::vector<model::Opening> trial = building_.openings;
    std::find_if(trial.begin(), trial.end(),
                 [id](const model::Opening& o) { return o.id == id; })
        ->height_mm = result.applied_mm;
    if (!commitOpenings(target.wall_id, std::move(trial))) {
        return ports::driving::ParamResult{target.height_mm,
                                           ports::driving::ParamStatus::Rejected};
    }
    return result;
}

ports::driving::ParamResult StructureEditService::setWindowSill(
    model::OpeningId id, double mm) {
    model::Opening& target = mutableOpening(id);
    if (target.kind != model::OpeningKind::Window) {
        // Türen haben keine Brüstung (LH-FA-WIN-004 nur Fenster).
        return ports::driving::ParamResult{target.sill_height_mm,
                                           ports::driving::ParamStatus::Rejected};
    }
    const ports::driving::ParamResult result = evaluateParam(
        mm, Range{model::kWindowSillMinMm, model::kWindowSillMaxMm},
        target.sill_height_mm);
    if (result.status == ports::driving::ParamStatus::Rejected) {
        return result;
    }
    std::vector<model::Opening> trial = building_.openings;
    std::find_if(trial.begin(), trial.end(),
                 [id](const model::Opening& o) { return o.id == id; })
        ->sill_height_mm = result.applied_mm;
    if (!commitOpenings(target.wall_id, std::move(trial))) {
        return ports::driving::ParamResult{target.sill_height_mm,
                                           ports::driving::ParamStatus::Rejected};
    }
    return result;
}

bool StructureEditService::moveOpening(model::OpeningId id, double offset_mm) {
    const auto it = std::find_if(
        building_.openings.begin(), building_.openings.end(),
        [id](const model::Opening& o) { return o.id == id; });
    if (it == building_.openings.end()) {
        return false;
    }
    const double length = wallLength(wall(it->wall_id));
    std::vector<model::Opening> trial = building_.openings;
    auto& t = *std::find_if(trial.begin(), trial.end(),
                            [id](const model::Opening& o) { return o.id == id; });
    t.offset_mm = std::clamp(offset_mm, 0.0, std::max(0.0, length - t.width_mm));
    return commitOpenings(it->wall_id, std::move(trial));
}

void StructureEditService::setDoorSwing(model::OpeningId id,
                                        model::SwingDirection swing) {
    const auto it = std::find_if(
        building_.openings.begin(), building_.openings.end(),
        [id](const model::Opening& o) { return o.id == id; });
    // Reine Eigenschaft (LH-FA-DOR-003) ohne Geometrie-Folge in welle-2
    // (kein Türblatt-Solid, ADR-0011 Re-Eval) → keine Meldung.
    if (it != building_.openings.end() && it->kind == model::OpeningKind::Door) {
        it->swing = swing;
    }
}

bool StructureEditService::removeOpening(model::OpeningId id) {
    const auto it = std::find_if(
        building_.openings.begin(), building_.openings.end(),
        [id](const model::Opening& o) { return o.id == id; });
    if (it == building_.openings.end()) {
        return false;
    }
    const model::WallId host = it->wall_id;
    std::vector<model::Opening> trial = building_.openings;
    trial.erase(std::remove_if(trial.begin(), trial.end(),
                               [id](const model::Opening& o) { return o.id == id; }),
                trial.end());
    // Wirtswand ohne die Öffnung neu bauen (schließt sich wieder) +
    // WallGeometryChanged. commitOpenings ist hier nicht-werfend
    // (Footprint der Wand ohne Cutout ist gültig).
    return commitOpenings(host, std::move(trial));
}

const model::Opening& StructureEditService::opening(model::OpeningId id) const {
    const auto it = std::find_if(
        building_.openings.begin(), building_.openings.end(),
        [id](const model::Opening& o) { return o.id == id; });
    if (it == building_.openings.end()) {
        throw std::out_of_range("opening: unbekannte Öffnungs-Id");
    }
    return *it;
}

model::Opening& StructureEditService::mutableOpening(model::OpeningId id) {
    const auto it = std::find_if(
        building_.openings.begin(), building_.openings.end(),
        [id](const model::Opening& o) { return o.id == id; });
    if (it == building_.openings.end()) {
        throw std::out_of_range("mutableOpening: unbekannte Öffnungs-Id");
    }
    return *it;
}

// --- Dächer (LH-FA-ROF-*, ADR-0011 #6) ---

std::vector<ports::driving::RoofMesh> StructureEditService::roofMeshes() const {
    std::vector<ports::driving::RoofMesh> meshes;
    meshes.reserve(building_.roofs.size());
    for (const model::Roof& r : building_.roofs) {
        model::TriangleMesh mesh = roofMesh(r);  // analytisch, total
        if (!mesh.empty()) {
            meshes.push_back(ports::driving::RoofMesh{r.id, std::move(mesh)});
        }
    }
    return meshes;
}

std::optional<model::RoofId> StructureEditService::addRoof(
    const model::Roof& prototype) {
    // Degenerierter Grundriss oder nicht-endliche Parameter → abgelehnt
    // (LH-FA-ROF Negative; kein NaN-Wert im Modell — Konsistenz zur
    // addWall-Linie und zum evaluateParam-Kontrakt).
    if (!std::isfinite(prototype.width_mm) ||
        !std::isfinite(prototype.depth_mm) ||
        !std::isfinite(prototype.pitch_deg) ||
        !std::isfinite(prototype.overhang_mm) ||
        !std::isfinite(prototype.base_z_mm) ||
        prototype.width_mm < model::kGeometryToleranceMm ||
        prototype.depth_mm < model::kGeometryToleranceMm) {
        return std::nullopt;
    }
    model::Roof roof = prototype;
    roof.id = static_cast<model::RoofId>(next_roof_id_);
    roof.pitch_deg = std::clamp(prototype.pitch_deg, model::kRoofPitchMinDeg,
                                model::kRoofPitchMaxDeg);
    roof.overhang_mm = std::clamp(prototype.overhang_mm,
                                  model::kRoofOverhangMinMm,
                                  model::kRoofOverhangMaxMm);
    building_.roofs.push_back(roof);
    ++next_roof_id_;
    notifyRoofChanged(roof.storey_id);
    return roof.id;
}

ports::driving::ParamResult StructureEditService::setRoofPitch(
    model::RoofId id, double deg) {
    model::Roof& target = mutableRoof(id);
    const ports::driving::ParamResult result = evaluateParam(
        deg, Range{model::kRoofPitchMinDeg, model::kRoofPitchMaxDeg},
        target.pitch_deg);
    if (result.status != ports::driving::ParamStatus::Rejected) {
        target.pitch_deg = result.applied_mm;
        notifyRoofChanged(target.storey_id);
    }
    return result;
}

ports::driving::ParamResult StructureEditService::setRoofOverhang(
    model::RoofId id, double mm) {
    model::Roof& target = mutableRoof(id);
    const ports::driving::ParamResult result = evaluateParam(
        mm, Range{model::kRoofOverhangMinMm, model::kRoofOverhangMaxMm},
        target.overhang_mm);
    if (result.status != ports::driving::ParamStatus::Rejected) {
        target.overhang_mm = result.applied_mm;
        notifyRoofChanged(target.storey_id);
    }
    return result;
}

void StructureEditService::setRoofType(model::RoofId id, model::RoofType type) {
    model::Roof& target = mutableRoof(id);
    if (target.type != type) {
        target.type = type;
        notifyRoofChanged(target.storey_id);
    }
}

bool StructureEditService::removeRoof(model::RoofId id) {
    const auto it = std::find_if(building_.roofs.begin(), building_.roofs.end(),
                                 [id](const model::Roof& r) { return r.id == id; });
    if (it == building_.roofs.end()) {
        return false;
    }
    const model::StoreyId storey = it->storey_id;
    building_.roofs.erase(it);
    notifyRoofChanged(storey);
    return true;
}

void StructureEditService::notifyRoofChanged(model::StoreyId storey) {
    notifyListeners({.op = ports::driven::ModelChangeOp::RoofChanged,
                     .storey_id = storey});
}

const model::Roof& StructureEditService::roof(model::RoofId id) const {
    const auto it = std::find_if(building_.roofs.begin(), building_.roofs.end(),
                                 [id](const model::Roof& r) { return r.id == id; });
    if (it == building_.roofs.end()) {
        throw std::out_of_range("roof: unbekannte Dach-Id");
    }
    return *it;
}

model::Roof& StructureEditService::mutableRoof(model::RoofId id) {
    const auto it = std::find_if(building_.roofs.begin(), building_.roofs.end(),
                                 [id](const model::Roof& r) { return r.id == id; });
    if (it == building_.roofs.end()) {
        throw std::out_of_range("mutableRoof: unbekannte Dach-Id");
    }
    return *it;
}

// --- Platten: Decken/Fundament (LH-FA-SLB-*/FND-*, ADR-0011 #6) ---

double StructureEditService::storeyHeight(model::StoreyId id) const {
    const auto it = std::find_if(
        building_.storeys.begin(), building_.storeys.end(),
        [id](const model::Storey& s) { return s.id == id; });
    return (it != building_.storeys.end()) ? it->height_mm : 0.0;
}

std::vector<ports::driving::SlabMesh> StructureEditService::slabMeshes() const {
    std::vector<ports::driving::SlabMesh> meshes;
    meshes.reserve(building_.slabs.size());
    for (const model::Slab& s : building_.slabs) {
        try {
            // Footprint-Extrusion bei z∈[0,Dicke] (mit Ausschnitten), DANN
            // auf die Aufstandshöhe verschieben (MED-2: Cutouts relativ,
            // Translation nach dem Boolean).
            model::TriangleMesh mesh = geometry_.tessellateFootprint(
                s.footprint, s.thickness_mm, slabCutPrisms(s));
            mesh = translateMeshZ(std::move(mesh),
                                  slabBaseZ(s, storeyHeight(s.storey_id)));
            if (!mesh.empty()) {
                meshes.push_back(ports::driving::SlabMesh{s.id, std::move(mesh)});
            }
        } catch (const std::exception&) {
            continue;  // E-GEO-002 in der Query-Hälfte: total, Platte überspringen
        }
    }
    return meshes;
}

std::optional<model::SlabId> StructureEditService::addSlab(
    const model::Slab& prototype) {
    // Degenerierter Grundriss / nicht-endliche Parameter → abgelehnt.
    if (prototype.footprint.points.size() < 3 ||
        !std::isfinite(prototype.thickness_mm) ||
        polygonArea(prototype.footprint) <
            (model::kGeometryToleranceMm * model::kGeometryToleranceMm)) {
        return std::nullopt;
    }
    // Bekanntes Geschoss (für die Decken-Aufstandshöhe).
    const auto sit = std::find_if(
        building_.storeys.begin(), building_.storeys.end(),
        [&](const model::Storey& s) { return s.id == prototype.storey_id; });
    if (sit == building_.storeys.end()) {
        return std::nullopt;
    }
    model::Slab slab = prototype;
    slab.id = static_cast<model::SlabId>(next_slab_id_);
    const Range range = slabThicknessRange(slab.type);
    slab.thickness_mm = std::clamp(prototype.thickness_mm, range.min, range.max);
    building_.slabs.push_back(slab);
    ++next_slab_id_;
    notifySlabChanged(slab.storey_id);
    return slab.id;
}

ports::driving::ParamResult StructureEditService::setSlabThickness(
    model::SlabId id, double mm) {
    model::Slab& target = mutableSlab(id);
    const ports::driving::ParamResult result =
        evaluateParam(mm, slabThicknessRange(target.type), target.thickness_mm);
    if (result.status != ports::driving::ParamStatus::Rejected) {
        target.thickness_mm = result.applied_mm;
        notifySlabChanged(target.storey_id);
    }
    return result;
}

bool StructureEditService::addSlabCutout(model::SlabId id,
                                         const model::Footprint& cutout) {
    const auto it = std::find_if(building_.slabs.begin(), building_.slabs.end(),
                                 [id](const model::Slab& s) { return s.id == id; });
    if (it == building_.slabs.end()) {
        return false;
    }
    it->cutouts.push_back(cutout);  // auf den Umriss begrenzt der Boolean
    notifySlabChanged(it->storey_id);
    return true;
}

bool StructureEditService::removeSlab(model::SlabId id) {
    const auto it = std::find_if(building_.slabs.begin(), building_.slabs.end(),
                                 [id](const model::Slab& s) { return s.id == id; });
    if (it == building_.slabs.end()) {
        return false;
    }
    const model::StoreyId storey = it->storey_id;
    building_.slabs.erase(it);
    notifySlabChanged(storey);
    return true;
}

void StructureEditService::notifySlabChanged(model::StoreyId storey) {
    notifyListeners({.op = ports::driven::ModelChangeOp::SlabChanged,
                     .storey_id = storey});
}

const model::Slab& StructureEditService::slab(model::SlabId id) const {
    const auto it = std::find_if(building_.slabs.begin(), building_.slabs.end(),
                                 [id](const model::Slab& s) { return s.id == id; });
    if (it == building_.slabs.end()) {
        throw std::out_of_range("slab: unbekannte Platten-Id");
    }
    return *it;
}

model::Slab& StructureEditService::mutableSlab(model::SlabId id) {
    const auto it = std::find_if(building_.slabs.begin(), building_.slabs.end(),
                                 [id](const model::Slab& s) { return s.id == id; });
    if (it == building_.slabs.end()) {
        throw std::out_of_range("mutableSlab: unbekannte Platten-Id");
    }
    return *it;
}

}  // namespace bcad::hexagon::services
