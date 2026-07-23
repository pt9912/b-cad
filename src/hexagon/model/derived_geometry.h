#pragma once

#include <vector>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/plan_view.h"
#include "hexagon/model/step_box.h"
#include "hexagon/model/triangle_mesh.h"

namespace bcad::hexagon::model {

// Kern-berechnete, abgeleitete Bauteil-Geometrie, die der Kern den
// **Export-Adaptern** über den `ModelExporterPort` reicht — driven Adapter
// serialisieren nur, sie leiten **keine** Domänen-Geometrie mehr ab (ADR-0020).
// Ein **pures Werttyp-Aggregat** (nur `model/`-Typen + Standardbibliothek →
// lib-frei, Kern-Link-Barriere; **kein** `TopoDS_Shape` — der Adapter baut das
// B-Rep aus diesen pre-OCC-Primitiven selbst). **Format-selektiv befüllt:** der
// `ExchangeService` füllt je Format nur die relevanten Teile; ein Format ohne
// abgeleitete Geometrie (IFC/DXF/PDF/PNG) bekommt ein leeres Bündel.
//
// slice-042a legt **nur die Naht** (Typ + Port-Vertrag): das Bündel wird noch
// nicht befüllt und von keinem Adapter konsumiert; die format-selektive
// Berechnung im `ExchangeService` + der STEP/STL-Konsum folgen mit slice-042c.

// Wand: Grundriss + Höhe + Öffnungs-Schnitt-Prismen (STEP: `makeNetSolid`;
// STL: `tessellateFootprint` + Boolean über den `GeometryKernelPort`).
struct DerivedWall {
    Footprint footprint;
    double height_mm{};
    std::vector<CutPrism> cutPrisms;
};

// Decke/Fundament: Grundriss + Dicke + Ausschnitte, plus die Aufstandshöhe
// `baseZ` (die extrudierte Platte wird nach dem Boolean per `translateMeshZ`
// bzw. OCC-Lift auf `baseZ` gehoben).
struct DerivedSlab {
    Footprint footprint;
    double thickness_mm{};
    std::vector<CutPrism> cutPrisms;
    double baseZ_mm{};
};

// Dach: wasserdichtes Netz (STEP: `meshToSolid`-Vernähung; STL: direkt).
struct DerivedRoof {
    TriangleMesh mesh;
};

// Treppe: solide Stufen-Boxen (STEP: `makeBoxSolid`) **und** das render-fertige
// Netz inkl. Geländer (STL) — jeder Exporter liest sein Feld. (Der abgeleitete
// `rise`-Persistenz-Skalar liegt **nicht** hier: die Persistenz hat eine eigene
// `model::PersistedDerivations`-Naht — slice-042d, ADR-0020.)
struct DerivedStair {
    std::vector<StepBox> boxes;
    TriangleMesh mesh;
};

struct DerivedGeometry {
    std::vector<DerivedWall> walls;
    std::vector<DerivedSlab> slabs;
    std::vector<DerivedRoof> roofs;
    std::vector<DerivedStair> stairs;

    // 2D-Grundriss-Projektion für die 2D-Export-Formate (PDF/PNG); kern-berechnet
    // (`services::projectPlan`), format-selektiv befüllt — leer für Formate, die
    // sie nicht brauchen (STEP/STL/IFC; DXF iteriert direkt). ADR-0019/ADR-0020.
    PlanView plan;
};

}  // namespace bcad::hexagon::model
