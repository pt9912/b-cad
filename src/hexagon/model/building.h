#pragma once

#include <vector>

#include "hexagon/model/opening.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace bcad::hexagon::model {

// Gebäudemodell (OBJ-003): durchgängiges parametrisches Datenmodell,
// Quelle für 2D und 3D. In 003a: Geschosse + Wände im Speicher;
// seit slice-013b: wand-gehostete Öffnungen (Türen/Fenster, ADR-0011);
// seit slice-014b: Dächer (LH-FA-ROF-*, ADR-0011-Leitplanke).
struct Building {
    std::vector<Storey> storeys;
    std::vector<Wall> walls;
    std::vector<Opening> openings;
    std::vector<Roof> roofs;
};

}  // namespace bcad::hexagon::model
