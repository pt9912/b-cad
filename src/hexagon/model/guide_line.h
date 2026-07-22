#pragma once

#include "hexagon/model/layer.h"    // LayerId
#include "hexagon/model/segment.h"  // Segment (Point2D start/end, mm)
#include "hexagon/model/wall.h"     // StoreyId

namespace bcad::hexagon::model {

// Starker Id-Typ (enum class, Muster WallId/MaterialId).
enum class GuideLineId : int {};

// Hilfslinie (LH-FA-DRW-005): gerade 2D-Zeichenhilfe (Anfangs-/Endpunkt in mm)
// auf einem Geschoss, einer Ebene zugeordnet. Der Ebenen-Bezug ist ein
// DIREKTER typisierter FK (die Hilfslinie trägt ihre Ebene), NICHT die
// polymorphe entity_layers-Zuordnung (ADR-0018 §Entscheidung 3). Überlebt
// Speichern/Laden und erscheint im 2D-Grundriss-Export (Sichtbarkeit über die
// Ebene). Eine Hilfslinie ohne Ausdehnung (Anfang = Ende) wird abgelehnt
// (spez. §1, E-VAL-001-Rejection). Pure Domänen-Werte, framework-frei (ADR-0001).
struct GuideLine {
    GuideLineId id{};
    StoreyId storey_id{};
    LayerId layer_id{};
    Segment segment{};  // Anfangs-/Endpunkt (Point2D, mm)
};

}  // namespace bcad::hexagon::model
