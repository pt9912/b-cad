#pragma once

#include "hexagon/model/wall.h"  // StoreyId

namespace bcad::hexagon::model {

// Geschoss (LH-FA-FLR-*). Höhe in Millimetern.
struct Storey {
    StoreyId id{};
    double height_mm{};
};

}  // namespace bcad::hexagon::model
