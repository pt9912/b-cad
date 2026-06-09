#pragma once

#include <vector>

#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace bcad::hexagon::model {

// Gebäudemodell (OBJ-003): durchgängiges parametrisches Datenmodell,
// Quelle für 2D und 3D. In 003a: Geschosse + Wände im Speicher.
struct Building {
    std::vector<Storey> storeys;
    std::vector<Wall> walls;
};

}  // namespace bcad::hexagon::model
