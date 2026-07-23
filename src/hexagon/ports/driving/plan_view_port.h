#pragma once

#include "hexagon/model/plan_view.h"

namespace bcad::hexagon::ports::driving {

// Driving Port (ADR-0001, ADR-0019): read-only 2D-Lese-Naht — liefert die
// 2D-Grundriss-Projektion (Wand-Achsen + sichtbare Hilfslinien je Geschoss +
// gemeinsame Bounding-Box) aus dem committeten Modell. Das **2D-Analog zum
// `ViewModelPort`** (der die 3D-Tessellation liefert): reine Query (pull on
// demand, kein …Changed-`op`, keine Mutation), framework-freier Werttyp
// (`model::PlanView`). Eine Quelle für den interaktiven 2D-Canvas (zieht diesen
// Port) UND den 2D-Export (bekommt dieselbe Projektion im `DerivedGeometry`-
// Bündel) — kern-berechnet, kein Adapter leitet 2D-Geometrie ab (ADR-0020).
class PlanViewPort {
public:
    virtual ~PlanViewPort() = default;

    // Der projizierte 2D-Grundriss des aktuellen Modells (total; leeres Modell →
    // `has_geometry == false`).
    virtual model::PlanView planView() const = 0;
};

}  // namespace bcad::hexagon::ports::driving
