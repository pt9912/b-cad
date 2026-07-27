#pragma once

#include <optional>

#include "hexagon/model/material.h"  // MaterialId
#include "hexagon/model/point2d.h"
#include "hexagon/model/wall_type.h"

namespace bcad::hexagon::model {

// Starke Id-Typen (enum class): nicht implizit nach `double`/`int`
// konvertierbar. Damit kann eine Id nie versehentlich gegen einen
// Messwert (Stärke/Höhe in mm) vertauscht werden — die Bauteil-
// Signaturen bleiben eindeutig (clang-tidy bugprone-easily-swappable).
enum class WallId : int {};
enum class StoreyId : int {};

// Parametrische Wand (Einzel-Segment). Stärke/Höhe in Millimetern,
// validiert/geklemmt im StructureEditService (LH-FA-WAL-002/003).
struct Wall {
    WallId id{};
    StoreyId storey_id{};
    Point2D start{};
    Point2D end{};
    double thickness_mm{};
    double height_mm{};
    WallType type{WallType::Innen};
    std::optional<MaterialId> material_id{};  // eigenes Material (Override, MAT-003)

    // slice-052a: **`= default` ist Pflicht, kein Stil.** Der compiler-generierte
    // Vergleich nimmt jedes kuenftig ergaenzte Feld automatisch auf; ein
    // handgeschriebener Operator wuerde es stillschweigend uebersehen — und genau
    // das waere der stille Datenverlust, gegen den der Sitzungs-Vergleich steht.
    auto operator==(const Wall&) const -> bool = default;
};

}  // namespace bcad::hexagon::model
