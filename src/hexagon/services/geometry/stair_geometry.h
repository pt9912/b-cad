#pragma once

#include <vector>

#include "hexagon/model/stair.h"
#include "hexagon/model/triangle_mesh.h"

namespace bcad::hexagon::services {

// Analytisches Treppen-Netz (gerade einläufige Treppe, spez. §1
// `LH-FA-STR-001.a`): `step_count` solide Stufen-Quader + Geländer, direkt als
// Dreiecke angegeben — ein Polyeder, **kein OCC/Port** (ADR-0001/0009, Muster
// `roof_geometry`; das Netz fließt framework-frei über den `ViewModelPort`).
// Die Steigung ist abgeleitet: `rise = from_storey_height_mm / step_count`;
// `base_z = 0` (welle-2-Ein-Geschoss-Annahme wie `slab_geometry`). Stufe `i`
// spannt `x∈[start.x+i·tread, start.x+(i+1)·tread]`, `y∈[start.y,
// start.y+width]`, `z∈[0,(i+1)·rise]` (+x-Aufstieg). Das Geländer folgt der
// Stufenfolge auf beiden Lauf-Seiten bis `kStairRailingHeightMm` über die
// jeweilige Stufenoberkante. **Total:** ungültige Parameter (`step_count < 1`,
// `width`/`tread`/`from_storey_height_mm < Toleranz, nicht-endlich) → **leeres**
// Netz (kein Wurf).
model::TriangleMesh stairMesh(const model::Stair& stair,
                              double from_storey_height_mm);

// Achsen-paralleler Stufen-Quader (mm) — Extents einer soliden Stufe **ohne**
// Geländer. Pure Geometrie-Daten (kein OCC/Framework).
struct StepBox {
    double x0_mm;
    double x1_mm;
    double y0_mm;
    double y1_mm;
    double z0_mm;
    double z1_mm;
};

// Die soliden Stufen-Quader (ohne Geländer) als **eine Box-Wahrheit** (slice-024b):
// Stufe `i` = `[start.x+i·tread, start.x+(i+1)·tread] × [start.y, start.y+width] ×
// [0, (i+1)·rise]`. Konsumiert von **`stairMesh`** (Darstellung + STL) **und** dem
// **STEP-Export** (B-Rep-Box-Solids) — kein Formel-Duplikat. **Total:** ungültige
// Parameter (wie `stairMesh`) → **leere** Liste (kein Wurf). Das Geländer ist
// **nicht** enthalten (render-only; STEP exportiert nur die Stufen, slice-024b).
std::vector<StepBox> stairStepBoxes(const model::Stair& stair,
                                    double from_storey_height_mm);

// Abgeleitete Steigung je Stufe (`from_storey_height_mm / step_count`); 0 bei
// `step_count ≤ 0` (Division-Schutz). Pure Query.
double stairRiseMm(const model::Stair& stair, double from_storey_height_mm);

// Lauflänge (`step_count · tread_mm`). Pure Query.
double stairRunLengthMm(const model::Stair& stair);

}  // namespace bcad::hexagon::services
