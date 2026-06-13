#pragma once

#include "hexagon/model/point2d.h"
#include "hexagon/model/wall.h"  // StoreyId

namespace bcad::hexagon::model {

// Starker Id-Typ (enum class): nicht implizit nach int/double konvertierbar
// (Muster WallId/OpeningId; clang-tidy bugprone-easily-swappable).
enum class RoofId : int {};

// Dachform (LH-FA-ROF-001..003; Schema `roofs.roof_type` sattel/walm/pult,
// ADR-0006).
enum class RoofType { Sattel, Walm, Pult };

// Dach über einem **rechteckigen** Grundriss (welle-2-Teilumfang,
// LH-FA-ROF-001..005, spez. §1 LH-FA-ROF-001.a). Achsen-ausgerichtetes
// Rechteck `[origin.x, origin.x+width] × [origin.y, origin.y+depth]`,
// Traufe auf `base_z_mm`; Überstand `overhang_mm` vergrößert es ringsum
// zum Traufrechteck. Firsthöhe ist abgeleitet (`pitch_deg`, nicht
// gespeichert). Pure Werte, framework-frei (ADR-0001).
struct Roof {
    RoofId id{};
    StoreyId storey_id{};
    RoofType type{RoofType::Sattel};
    Point2D origin{};        // Eck des rechteckigen Grundrisses
    double width_mm{};       // b (x-Ausdehnung des Grundrisses)
    double depth_mm{};       // t (y-Ausdehnung des Grundrisses)
    double base_z_mm{};      // Traufhöhe / Aufstandshöhe
    double pitch_deg{};      // Dachneigung (LH-FA-ROF-004)
    double overhang_mm{};    // Dachüberstand (LH-FA-ROF-005)
};

}  // namespace bcad::hexagon::model
