#pragma once

#include "hexagon/model/area_report.h"
#include "hexagon/model/wall.h"  // model::StoreyId

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
};

}  // namespace bcad::hexagon::ports::driving
