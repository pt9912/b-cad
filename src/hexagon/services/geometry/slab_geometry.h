#pragma once

#include <vector>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/slab.h"

namespace bcad::hexagon::services {

// Pure Platten-Geometrie-Helfer (spez. §1 `LH-FA-SLB-001.a`, ADR-0011 #6).
// Das Platten-Solid/-Netz selbst entsteht über den `GeometryKernelPort`
// (Footprint-Extrusion + Schnitt-Prismen — dasselbe Vokabular wie
// Wand/Öffnung); diese Helfer liefern die **reinen Werte** dafür und die
// Aufstands-Verschiebung. Kein Port, kein Framework.

// Aufstandshöhe der Platte (Unterkante des extrudierten Solids) je Typ.
// **Ein-Geschoss-Annahme der Welle** (`Storey` trägt keine Elevation):
// Decke = Geschoss-Oberkante (`storey_height_mm`, Unterkante 0);
// Bodenplatte/Fundament = −Dicke → Oberkante auf Höhe 0 (Tiefe nach
// unten). Mehr-Geschoss-Stapelung ist späterer Ausbau.
double slabBaseZ(const model::Slab& slab, double storey_height_mm);

// Schnitt-Prismen der Aussparungen (LH-FA-SLB-003). z **relativ** zum
// Extrusions-Solid `[−ε, Dicke+ε]` (NICHT base_z!) — der Schnitt erfolgt
// am Footprint-Solid bei z∈[0,Dicke], die base_z-Verschiebung kommt erst
// danach auf das fertige Netz (`translateMeshZ`). Überstand ε
// volumen-neutral (außerhalb der Platte).
std::vector<model::CutPrism> slabCutPrisms(const model::Slab& slab);

// Liegt der Ausschnitt vollständig im Umriss der Platte (spez. §1
// LH-FA-SLB-001.a: „auf den Platten-Umriss begrenzt")? True nur für ein
// nicht-degeneriertes (≥3 Stützpunkte, endliche Koordinaten, Fläche >
// `GEOMETRY_TOLERANCE_MM²`) Polygon, dessen Stützpunkte alle strikt im
// `slab.footprint` liegen. Rand-/außenliegende oder degenerierte
// Ausschnitte werden an der API abgelehnt — so bleibt der OCC-Boolean
// koplanar-frei (innenliegendes Loch braucht keinen lateralen Überstand,
// anders als die die Wand durchspannende Öffnung). Vorbedingung:
// einfacher Footprint.
bool cutoutInsideSlab(const model::Slab& slab, const model::Footprint& cutout);

// `translateMeshZ` (die base_z-Aufstands-Verschiebung, §oben) lebt seit
// ADR-0020 als reine Wert-Util in `model/mesh_ops.h` — adapter-erreichbar ohne
// `services/geometry`-Import (die Slab-Mesh entsteht adapter-seitig).

}  // namespace bcad::hexagon::services
