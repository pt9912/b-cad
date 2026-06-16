#pragma once

#include <optional>
#include <vector>

#include "hexagon/model/area_report.h"
#include "hexagon/model/material.h"  // Material, MaterialId
#include "hexagon/model/roof.h"      // RoofId
#include "hexagon/model/slab.h"      // SlabId
#include "hexagon/model/volume_report.h"
#include "hexagon/model/wall.h"  // model::StoreyId, WallId

namespace bcad::hexagon::ports::driving {

// Driving Port (ADR-0001, ADR-0012): read-only Auswertung von Flächen aus
// dem committeten Modell. Reine Query (Muster DetectRoomsPort) — pull on
// demand, KEIN …Changed-op, keine Mutation, und KEIN GeometryKernelPort-
// Aufruf: die Flächen sind reine Aggregation der bereits in der Raum-
// erkennung berechneten Netto-Flächen (Room.net_area_mm2, ADR-0007).
class EvaluatePort {
public:
    virtual ~EvaluatePort() = default;

    // EVL-001: Netto-Grundfläche je Raum eines Geschosses und deren Summe
    // (m²). Unbekanntes oder raumloses Geschoss -> leerer Report (total 0).
    virtual model::AreaReport floorArea(model::StoreyId storey) const = 0;

    // EVL-003: Wohnfläche (gebäudeweit) = Summe der Raum-Netto-Grundflächen
    // × kLivingAreaFactor (m²). Modell ohne Räume -> leerer Report.
    virtual model::AreaReport livingArea() const = 0;

    // EVL-002: gebäudeweites Netto-MATERIAL-Volumen (m³) + Bauteiltyp-Subtotale
    // (Wand/Decke-Fundament/Treppe). Analytisch im Kern (Footprint·Höhe −
    // geklemmte Öffnungen; (Fläche − Ausschnitte)·Dicke; Stufenkörper) — KEIN
    // GeometryKernelPort, KEIN Lesen von Solid.volume_mm3. Dach ist welle-3
    // ausgenommen (dicke-loses Modell). Leeres Modell -> alle Felder 0.
    virtual model::VolumeReport volume() const = 0;

    // EVL-Material-Vorbau (LH-FA-MAT-002/003, read-only): Quelle für die
    // späteren Material-/Kostenlisten (EVL-004/006).

    // Liste der projekt-eigenen Materialien (MAT-002).
    virtual const std::vector<model::Material>& materials() const = 0;

    // Effektives Material eines Bauteils (MAT-003 Override-Auflösung): das
    // eigene `material_id` zu seinem `Material` aufgelöst. welle-3-Teilumfang:
    // KEIN `wall_type`-Template-Fallback (Wand-Typ ist Enum, keine material-
    // tragende Typ-Entität). Unbekanntes Bauteil / kein Material -> std::nullopt
    // (Totalität, kein Wurf).
    virtual std::optional<model::Material> effectiveMaterial(
        model::WallId wall) const = 0;
    virtual std::optional<model::Material> effectiveMaterial(
        model::RoofId roof) const = 0;
    virtual std::optional<model::Material> effectiveMaterial(
        model::SlabId slab) const = 0;
};

}  // namespace bcad::hexagon::ports::driving
