#pragma once

// Wertebereiche und Defaults des Domänen-Kerns.
// Quelle der Wahrheit: spec/spezifikation.md §3 (Defaults und Konstanten).
// Diese Header-Konstanten spiegeln die dort gepflegten Werte; bei
// Abweichung gewinnt die Spezifikation (Source Precedence).

namespace bcad::hexagon::model {

inline constexpr double kWallThicknessMinMm = 50.0;       // LH-FA-WAL-002
inline constexpr double kWallThicknessMaxMm = 1000.0;     // LH-FA-WAL-002
inline constexpr double kWallHeightMinMm = 500.0;         // LH-FA-WAL-003
inline constexpr double kWallHeightMaxMm = 10000.0;       // LH-FA-WAL-003
inline constexpr double kDefaultWallThicknessMm = 240.0;  // LH-FA-WAL-001
inline constexpr double kDefaultStoreyHeightMm = 2500.0;  // LH-FA-BLD-001/FLR-004
inline constexpr double kGeometryToleranceMm = 0.1;       // LH-FA-WAL-006/ROM-001

}  // namespace bcad::hexagon::model
