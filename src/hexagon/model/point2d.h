#pragma once

// Domain-Modell (Hexagon-Kern) — framework-frei (ADR-0001). Pure Werte,
// keine I/O, kein Qt/OCC/SQLite.

namespace bcad::hexagon::model {

// Punkt in der Grundriss-Ebene eines Geschosses, in Millimetern.
struct Point2D {
    double x_mm{};
    double y_mm{};

    // slice-052a: **`= default` ist Pflicht, kein Stil.** Der compiler-generierte
    // Vergleich nimmt jedes kuenftig ergaenzte Feld automatisch auf; ein
    // handgeschriebener Operator wuerde es stillschweigend uebersehen — und genau
    // das waere der stille Datenverlust, gegen den der Sitzungs-Vergleich steht.
    auto operator==(const Point2D&) const -> bool = default;
};

}  // namespace bcad::hexagon::model
