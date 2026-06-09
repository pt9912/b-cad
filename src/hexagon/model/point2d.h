#pragma once

// Domain-Modell (Hexagon-Kern) — framework-frei (ADR-0001). Pure Werte,
// keine I/O, kein Qt/OCC/SQLite.

namespace bcad::hexagon::model {

// Punkt in der Grundriss-Ebene eines Geschosses, in Millimetern.
struct Point2D {
    double x_mm{};
    double y_mm{};
};

}  // namespace bcad::hexagon::model
