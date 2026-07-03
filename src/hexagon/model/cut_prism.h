#pragma once

#include "hexagon/model/footprint.h"

namespace bcad::hexagon::model {

// Schnitt-Prisma für eine boolesche Subtraktion (LH-FA-DOR-004/WIN-005,
// ADR-0011 (b)): ein vertikales Prisma — Grundriss-Polygon `polygon`,
// extrudiert über den Höhen-Bereich `[z_min_mm, z_max_mm]`. Reiner
// Geometrie-Wert: der KERN berechnet die Öffnungs-Schnittkörper (aus
// Position/Breite/Brüstung/Höhe + Wandstärke, `services/geometry/opening_geometry.h`),
// der `GeometryKernelPort` subtrahiert sie vom extrudierten Wand-Solid.
// Der Geometrie-Adapter kennt keine Öffnungs-Semantik (ADR-0001/0002).
struct CutPrism {
    Footprint polygon;
    double z_min_mm{};
    double z_max_mm{};
};

}  // namespace bcad::hexagon::model
