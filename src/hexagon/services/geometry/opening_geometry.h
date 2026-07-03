#pragma once

#include <optional>
#include <vector>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/opening.h"
#include "hexagon/model/wall.h"

namespace bcad::hexagon::services {

// Schnitt-Prisma EINER Öffnung in ihrer Wirtswand (ADR-0011 (b),
// spez. §1 LH-FA-DOR-004.a/WIN-005.a): ein Rechteck quer zur Wandachse
// bei `[offset, offset+width]` über die **volle Wandstärke**, extrudiert
// über `[sill, min(sill+height, Wandhöhe)]`. Die Oberkante ist auf die
// Wandhöhe geklemmt (kein Durchbruch über die Wand hinaus, LH-FA-WIN-004
// Boundary). **Total:** liefert `nullopt`, wenn der Höhen-Bereich
// kollabiert (Brüstung ≥ Wandhöhe, nicht-endliche Werte) — dann entsteht
// kein Schnitt. Pure Domäne (kein Adapter/Framework).
std::optional<model::CutPrism> openingCutPrism(const model::Opening& opening,
                                               const model::Wall& wall);

// Alle Schnitt-Prismen der Öffnungen, deren Wirtswand `wall` ist
// (`opening.wall_id == wall.id`). Degenerierte Öffnungen werden
// übersprungen (total).
std::vector<model::CutPrism> wallCutPrisms(
    const model::Wall& wall, const std::vector<model::Opening>& openings);

}  // namespace bcad::hexagon::services
