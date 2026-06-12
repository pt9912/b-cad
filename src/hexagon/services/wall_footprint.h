#pragma once

#include <vector>

#include "hexagon/model/footprint.h"
#include "hexagon/model/wall.h"

namespace bcad::hexagon::services {

// Stumpfer Wand-Footprint: Rechteck Segment × Stärke (Enden stumpf —
// der Stand vor LH-FA-WAL-006).
model::Footprint buttFootprint(const model::Wall& wall);

// Wand-Footprint mit Eckenschluss (LH-FA-WAL-006-Teilumfang,
// spez. §1): an Endpunkten, die sich **im selben Geschoss** mit genau
// einer weiteren Wand verbinden (Grad-2-Knoten, Toleranz
// GEOMETRY_TOLERANCE_MM), werden die Seitenkanten beider Wände im
// Schnittpunkt verbunden; Begrenzung WALL_MITER_LIMIT
// (max. größere Wandstärke über den Endpunkt hinaus) mit Rückfall auf
// stumpfes Ende. Total — fällt bei jeder Degeneration (kollinear,
// Grad ≠ 2, Überschreitung) auf stumpf zurück, wirft nie.
model::Footprint wallFootprint(const model::Wall& wall,
                               const std::vector<model::Wall>& walls);

}  // namespace bcad::hexagon::services
