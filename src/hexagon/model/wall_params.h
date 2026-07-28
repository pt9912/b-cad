#pragma once

namespace bcad::hexagon::model {

// Die **änderbaren** Parameter einer Wand (ADR-0021 E13/E15, slice-057) — der
// Umfang ist bewusst eng: genau das, was die Oberfläche ändern darf. Wandtyp und
// Material sind ausdrücklich **nicht** dabei (E13); sie hätten eigene AK.
//
// Der Werttyp ist die Antwort der 2D-Lese-Naht auf eine **benannte** Wand. Er
// steht bewusst NICHT im `PlanSegment`: das reist im `DerivedGeometry`-Bündel zu
// den Export-Adaptern und würde um Felder wachsen, die nur eine Bedienfläche
// braucht (ADR-0020-Re-Eval „das Bündel wird zu breit").
struct WallParams {
    double thickness_mm{};
    double height_mm{};

    friend bool operator==(const WallParams&, const WallParams&) = default;
};

}  // namespace bcad::hexagon::model
