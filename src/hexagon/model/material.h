#pragma once

#include <optional>
#include <string>

namespace bcad::hexagon::model {

// Starker Id-Typ (enum class): nicht implizit nach int konvertierbar
// (Muster WallId/RoofId/SlabId; clang-tidy bugprone-easily-swappable).
enum class MaterialId : int {};

// Material als pure Domänen-Eigenschaft (LH-FA-MAT-*, spez. §2.1): wird
// Bauteilen zugewiesen und von der Auswertung (EVL) als Eingabe konsumiert —
// KEINE Geometrie. Kennwerte (U-Wert/Kosten) sind optional. `color_hex`/
// `texture_path` werden getragen, in welle-3 aber nicht aktiv genutzt
// (Darstellung/Texturen = MAT-004, Sicht). Framework-frei (ADR-0001).
struct Material {
    MaterialId id{};
    std::string name;                        // Pflicht (MAT-001: ohne Name → abgelehnt)
    std::string category;
    std::optional<double> u_value;           // MAT-005 (Wärmedurchgangskoeffizient)
    std::optional<double> cost_per_m2;       // MAT-006
    std::optional<double> cost_per_m3;       // MAT-006
    std::optional<std::string> color_hex;
    std::optional<std::string> texture_path;  // MAT-004 (Sicht, welle-3 ungenutzt)
};

}  // namespace bcad::hexagon::model
