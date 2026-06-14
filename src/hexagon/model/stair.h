#pragma once

#include "hexagon/model/point2d.h"
#include "hexagon/model/wall.h"  // StoreyId

namespace bcad::hexagon::model {

// Starker Id-Typ (enum class): nicht implizit nach int konvertierbar
// (Muster WallId/RoofId/SlabId; clang-tidy bugprone-easily-swappable).
enum class StairId : int {};

// Treppen-Art. **welle-2-Teilumfang: nur `Gerade`** (gerade einläufige Treppe,
// spez. §1 `LH-FA-STR-001.a`); Podest-/U-/L-/Wendeltreppen sind späterer Ausbau
// (Schema `stairs.stair_type`, ADR-0006 — vorwärts-kompatibel angelegt).
enum class StairType { Gerade };

// Gerade einläufige Treppe (LH-FA-STR-001..004, spez. §1 `LH-FA-STR-001.a`):
// verbindet zwei Geschosse (`from_storey` unten → `to_storey` oben) und steigt
// vom `start` in **+x-Richtung** als `step_count` Stufen auf. Die Steigung
// `rise` ist **abgeleitet** (Geschosshöhe / `step_count`) und **nicht**
// gespeichert; das Geländer ist immer Teil der Treppe (kein eigenes Feld,
// spez. §1). Pure Werte, framework-frei (ADR-0001).
struct Stair {
    StairId id{};
    StoreyId from_storey_id{};          // untere Etage (liefert Höhe + base_z)
    StoreyId to_storey_id{};            // obere Etage (Ziel des Aufstiegs)
    StairType type{StairType::Gerade};
    Point2D start{};                    // Startpunkt des Laufs (Fußpunkt)
    double width_mm{};                  // Laufbreite (LH-FA-STR-003)
    int step_count{};                   // Stufenanzahl (LH-FA-STR-002)
    double tread_mm{};                  // Auftritt / Stufentiefe
};

}  // namespace bcad::hexagon::model
