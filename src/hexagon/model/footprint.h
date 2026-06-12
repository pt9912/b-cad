#pragma once

#include <vector>

#include "hexagon/model/point2d.h"

namespace bcad::hexagon::model {

// Grundriss-Polygon eines Bauteils (LH-FA-WAL-006-Teilumfang,
// slice-012): die Footprint-Hoheit liegt im Kern — nur dort ist das
// Nachbar-Wissen des Gebäude-Graphen vorhanden. Der Geometrie-Adapter
// extrudiert/tesselliert dieses Polygon, statt es aus der Einzelwand
// zu raten. Pure Werte, einfache (nicht selbstschneidende) Polygone;
// Punktfolge umlaufend.
struct Footprint {
    std::vector<Point2D> points;
};

}  // namespace bcad::hexagon::model
