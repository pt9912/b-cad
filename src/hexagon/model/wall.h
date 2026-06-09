#pragma once

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
};

}  // namespace bcad::hexagon::model
