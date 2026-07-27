#pragma once

#include "hexagon/model/wall.h"  // StoreyId

namespace bcad::hexagon::model {

// Geschoss (LH-FA-FLR-*). Höhe in Millimetern.
struct Storey {
    StoreyId id{};
    double height_mm{};

    // slice-052a: **`= default` ist Pflicht, kein Stil.** Der compiler-generierte
    // Vergleich nimmt jedes kuenftig ergaenzte Feld automatisch auf; ein
    // handgeschriebener Operator wuerde es stillschweigend uebersehen — und genau
    // das waere der stille Datenverlust, gegen den der Sitzungs-Vergleich steht.
    auto operator==(const Storey&) const -> bool = default;
};

}  // namespace bcad::hexagon::model
