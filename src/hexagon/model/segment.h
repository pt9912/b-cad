#pragma once

#include "hexagon/model/point2d.h"

namespace bcad::hexagon::model {

// Ein Wand-Segment als Strecke zwischen zwei Punkten (Grundriss).
// Eigener Typ statt zweier `Point2D`-Parameter — hält Use-Case-Signaturen
// eindeutig (kein vertauschbares Parameter-Paar) und benennt die Absicht.
struct Segment {
    Point2D start{};
    Point2D end{};
};

}  // namespace bcad::hexagon::model
