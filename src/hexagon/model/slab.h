#pragma once

#include <optional>
#include <vector>

#include "hexagon/model/footprint.h"
#include "hexagon/model/material.h"  // MaterialId
#include "hexagon/model/wall.h"  // StoreyId

namespace bcad::hexagon::model {

// Starker Id-Typ (enum class): Muster WallId/RoofId.
enum class SlabId : int {};

// Platten-Art (LH-FA-SLB-* Decke / LH-FA-FND-* Fundament+Bodenplatte;
// Schema `slabs.slab_type`, ADR-0006).
enum class SlabType { Decke, Fundament, Bodenplatte };

// Horizontale Platte (Decke/Fundament/Bodenplatte, LH-FA-SLB/FND-*,
// spez. §1 `LH-FA-SLB-001.a`): ein Grundriss-Polygon, um die Dicke
// vertikal extrudiert, mit optionalen Aussparungen (LH-FA-SLB-003). Die
// Aufstandshöhe `base_z` ist **nicht** gespeichert, sondern aus
// `type`/Geschoss abgeleitet (`services/geometry/slab_geometry.h`). Pure Werte,
// framework-frei (ADR-0001).
struct Slab {
    SlabId id{};
    StoreyId storey_id{};
    SlabType type{SlabType::Decke};
    Footprint footprint{};               // Grundriss-Polygon
    double thickness_mm{};               // Decke: Dicke; Fundament: Tiefe
    std::vector<Footprint> cutouts{};    // Aussparungen (LH-FA-SLB-003)
    std::optional<MaterialId> material_id{};  // eigenes Material (Override, MAT-003)

    // slice-052a: **`= default` ist Pflicht, kein Stil.** Der compiler-generierte
    // Vergleich nimmt jedes kuenftig ergaenzte Feld automatisch auf; ein
    // handgeschriebener Operator wuerde es stillschweigend uebersehen — und genau
    // das waere der stille Datenverlust, gegen den der Sitzungs-Vergleich steht.
    auto operator==(const Slab&) const -> bool = default;
};

}  // namespace bcad::hexagon::model
