#pragma once

#include <vector>

#include "hexagon/model/footprint.h"
#include "hexagon/model/opening.h"
#include "hexagon/model/slab.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/wall.h"

namespace bcad::hexagon::services {

// Analytische Netto-MATERIAL-Volumina im Kern (LH-FA-EVL-002, spez. §1
// LH-FA-EVL-001.a): reine Funktionen über das Domänen-Modell, OHNE
// GeometryKernelPort/OCC. Damit bleibt die Auswertung eine reine Kern-Query —
// es wird NICHT solids_[].volume_mm3 (die OCC-`GProp`-Adapter-Messung) gelesen,
// sondern die Geometrie selbst analytisch ausgewertet. Alle Funktionen sind
// TOTAL (degenerierte/unsinnige Eingaben -> 0, kein Wurf) und liefern nie ein
// negatives Volumen.

// Vorzeichenlose Polygon-Fläche (Shoelace) in mm². Konsolidierte Quelle: löst
// die zuvor service-lokale Kopie ab (die zweite Kopie `absPolygonArea` in
// slab_geometry.cpp bleibt eine benannte Drift-Notiz für einen späteren Merge).
double polygonArea(const model::Footprint& footprint);

// Wand-Netto-Volumen in mm³: Shoelace-Fläche des Wand-Footprints (inkl.
// Eckenschluss, `wall_footprint.h`) · height_mm minus je gehosteter Öffnung das
// REAL entfernte, geklemmte Volumen `width · wall.thickness · clamped_height`
// (`clamped_height = min(sill+height, Wandhöhe) − max(0, sill)`, spez. §1) —
// NICHT das überstands-behaftete Roh-Schnittprisma. Geklemmt auf >= 0.
double wallNetVolumeMm3(const model::Wall& wall,
                        const std::vector<model::Wall>& walls,
                        const std::vector<model::Opening>& openings);

// Platten-Netto-Volumen in mm³: (Footprint-Fläche − Σ gültige Ausschnitt-
// Flächen) · thickness_mm. Gültigkeit via `cutoutInsideSlab` (innenliegend,
// nicht-degeneriert; rand-/außenliegende Ausschnitte zählen nicht). Geklemmt
// auf >= 0 (pathologische Über-Ausschnitte).
double slabNetVolumeMm3(const model::Slab& slab);

// Treppen-Netto-Volumen in mm³: Σ Stufenkörper = `tread · width · h ·
// (step_count+1)/2` (geschlossene Form von Σ_{i=1..n} tread·width·i·rise,
// `rise = h/step_count`). GELÄNDER-FREI — das Geländer ist generierte
// Render-Geometrie (Höhe STR-004, Streifenstärke unspez.) und trägt kein
// Material. Unsinnige Maße / `from_storey_height_mm` <= 0 -> 0 (Totalität).
double stairNetVolumeMm3(const model::Stair& stair,
                         double from_storey_height_mm);

}  // namespace bcad::hexagon::services
