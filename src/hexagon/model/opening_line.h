#pragma once

namespace bcad::hexagon::model {

// Eine Zeile der Türliste (LH-FA-EVL-005): die Maße einer platzierten Tür.
// Die „Anzahl" ist die Größe der Liste. Pure Werte, framework-frei (ADR-0001).
struct DoorLine {
    double width_mm{};
    double height_mm{};
};

// Eine Zeile der Fensterliste (LH-FA-EVL-006): die Maße eines platzierten
// Fensters inkl. Brüstung (LH-FA-WIN-004). Die „Anzahl" ist die Größe der Liste.
struct WindowLine {
    double width_mm{};
    double height_mm{};
    double sill_height_mm{};
};

}  // namespace bcad::hexagon::model
