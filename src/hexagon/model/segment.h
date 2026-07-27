#pragma once

#include "hexagon/model/point2d.h"

namespace bcad::hexagon::model {

// Ein Wand-Segment als Strecke zwischen zwei Punkten (Grundriss).
// Eigener Typ statt zweier `Point2D`-Parameter — hält Use-Case-Signaturen
// eindeutig (kein vertauschbares Parameter-Paar) und benennt die Absicht.
struct Segment {
    Point2D start{};
    Point2D end{};

    // slice-052a: **`= default` ist Pflicht, kein Stil.** Der compiler-generierte
    // Vergleich nimmt jedes kuenftig ergaenzte Feld automatisch auf; ein
    // handgeschriebener Operator wuerde es stillschweigend uebersehen — und genau
    // das waere der stille Datenverlust, gegen den der Sitzungs-Vergleich steht.
    auto operator==(const Segment&) const -> bool = default;
};

}  // namespace bcad::hexagon::model
