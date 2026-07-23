#pragma once

#include "hexagon/model/building.h"
#include "hexagon/model/plan_view.h"

namespace bcad::hexagon::services {

// Projiziert `building` auf die 2D-Grundriss-Sicht (deterministisch aus der
// Modell-Reihenfolge). Rein/framework-frei — keine I/O, kein Format, keine
// Library (Kern-Rechnung, ADR-0001). Enthält je Geschoss die Wand-Achsen + die
// Hilfslinien auf sichtbarer Ebene (LH-FA-DRW-005, ADR-0018) als schlichte
// 2D-Segmente; die gemeinsame BBox schließt beide ein. **Total** (kein Wurf).
//
// ADR-0019/ADR-0020: aus dem io-Adapter in den Kern gehoben — eine Quelle für den
// interaktiven 2D-Canvas (über den `PlanViewPort`) UND den 2D-Export (über das
// `DerivedGeometry`-Bündel, das der `ExchangeService` befüllt); kein Adapter
// leitet die 2D-Geometrie mehr selbst ab.
model::PlanView projectPlan(const model::Building& building);

}  // namespace bcad::hexagon::services
