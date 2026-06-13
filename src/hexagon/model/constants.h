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

// Wandöffnungen — Türen (LH-FA-DOR-003) / Fenster (LH-FA-WIN-003/004).
inline constexpr double kDoorWidthMinMm = 600.0;       // LH-FA-DOR-003
inline constexpr double kDoorWidthMaxMm = 2000.0;      // LH-FA-DOR-003
inline constexpr double kDoorHeightMinMm = 1800.0;     // LH-FA-DOR-003
inline constexpr double kDoorHeightMaxMm = 2500.0;     // LH-FA-DOR-003
inline constexpr double kWindowWidthMinMm = 300.0;     // LH-FA-WIN-003
inline constexpr double kWindowWidthMaxMm = 3000.0;    // LH-FA-WIN-003
inline constexpr double kWindowHeightMinMm = 300.0;    // LH-FA-WIN-003
inline constexpr double kWindowHeightMaxMm = 2500.0;   // LH-FA-WIN-003
inline constexpr double kWindowSillMinMm = 0.0;        // LH-FA-WIN-004
inline constexpr double kWindowSillMaxMm = 2000.0;     // LH-FA-WIN-004

// Überstand des Öffnungs-Schnittkörpers über die Wandgrenzen (je Seite
// lateral + oben/unten an Boundary-Höhen) für einen sauberen Boolean
// ohne koplanare Flächen (spez. §1 LH-FA-DOR-004.a „≥ Toleranz je
// Seite"). Liegt außerhalb der Wand → volumen-neutral.
inline constexpr double kOpeningCutOvershootMm = 1.0;  // LH-FA-DOR-004/WIN-005

// Dach (LH-FA-ROF-004 Neigung / LH-FA-ROF-005 Überstand); spez. §3.
inline constexpr double kRoofPitchMinDeg = 5.0;          // LH-FA-ROF-004
inline constexpr double kRoofPitchMaxDeg = 60.0;         // LH-FA-ROF-004
inline constexpr double kRoofOverhangMinMm = 0.0;        // LH-FA-ROF-005
inline constexpr double kRoofOverhangMaxMm = 1500.0;     // LH-FA-ROF-005
inline constexpr double kDefaultRoofPitchDeg = 30.0;     // LH-FA-ROF-001 (= roofs-Schema)
inline constexpr double kDefaultRoofOverhangMm = 500.0;  // LH-FA-ROF-005 (= roofs-Schema)

// Platten — Decken (LH-FA-SLB-002) / Fundament-Tiefe (LH-FA-FND-002); spez. §3.
inline constexpr double kSlabThicknessMinMm = 100.0;       // LH-FA-SLB-002
inline constexpr double kSlabThicknessMaxMm = 500.0;       // LH-FA-SLB-002
inline constexpr double kDefaultSlabThicknessMm = 200.0;   // LH-FA-SLB-001
inline constexpr double kFoundationDepthMinMm = 200.0;     // LH-FA-FND-002
inline constexpr double kFoundationDepthMaxMm = 2000.0;    // LH-FA-FND-002
inline constexpr double kDefaultFoundationDepthMm = 500.0; // LH-FA-FND-001

// Default-Maße bei Anlage (Muster kDefaultWallThicknessMm) — spez. §3.
inline constexpr double kDefaultDoorWidthMm = 900.0;    // LH-FA-DOR-001
inline constexpr double kDefaultDoorHeightMm = 2100.0;  // LH-FA-DOR-001
inline constexpr double kDefaultWindowWidthMm = 1200.0;   // LH-FA-WIN-001
inline constexpr double kDefaultWindowHeightMm = 1300.0;  // LH-FA-WIN-001
inline constexpr double kDefaultWindowSillMm = 900.0;     // LH-FA-WIN-001/004

}  // namespace bcad::hexagon::model
