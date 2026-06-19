#pragma once

#include <vector>

#include <TopoDS_Shape.hxx>

#include "hexagon/model/cut_prism.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/triangle_mesh.h"

// Geteilte OCC-Solid-Konstruktion (geometrie-intern, Regel C — diese Datei
// trägt OCC-`.hxx` und darf NUR von `src/adapters/geometry/` inkludiert werden).
// Gemeinsamer Builder für Volumen/Tessellation (`OccGeometryAdapter`) und den
// STEP-Export (B-Rep-Solids, slice-020b). Reine Geometrie — kein Domänen-Wissen.
namespace bcad::adapters::geometry {

// Grundriss-Polygon auf Höhe (+Z) extrudiert, minus die Öffnungs-Schnittkörper
// `cutouts` (boolesch, OCC). Wirft `std::runtime_error` (E-GEO-002) bei
// degeneriertem Polygon oder fehlgeschlagener Subtraktion — kein OCC-Typ-Leck
// über die Adapter-Grenze hinaus.
TopoDS_Shape makeNetSolid(const hexagon::model::Footprint& footprint,
                          double height_mm,
                          const std::vector<hexagon::model::CutPrism>& cutouts);

// Ein **wasserdichtes, außen-orientiertes** Dreiecksnetz (Flat-Shading, je Dreieck
// eigene Vertices) zu einem B-Rep-Solid vernähen (slice-024a, STEP-Export der
// Dächer): je Dreieck eine Face → `BRepBuilderAPI_Sewing` → geschlossene Shell →
// Solid. **fail-closed:** liefert ein **leeres** `TopoDS_Shape`, wenn das Ergebnis
// keine *eine* gültige, **geschlossene** Solid ist (`BRepCheck_Analyzer`) — eine
// offene/nicht-manifolde Vernähung verlässt diese Funktion nie. **Grenze (MR-009
// MED-1):** `BRepCheck_Analyzer.IsValid()` prüft Geschlossenheit, **nicht**
// Orientierung — die Außennormalen-Korrektheit ist vom Eingabe-Netz **geerbt**
// (Aufrufer-Vertrag: außen-orientiert, für Dächer die 023b-Invariante signiertes
// Volumen > 0), hier nicht erzwungen. Leeres Netz → leeres Shape. Kein Wurf
// (Totalität; der Aufrufer überspringt ein leeres Shape).
TopoDS_Shape meshToSolid(const hexagon::model::TriangleMesh& mesh);

// Achsen-parallelen Quader `[x0,x1]×[y0,y1]×[z0,z1]` (mm) als B-Rep-Solid
// (slice-024b, STEP-Export der Treppen-Stufen): `BRepPrimAPI_MakeBox` liefert ein
// geschlossenes, außen-orientiertes Solid. Der Degenerations-Guard (eine
// Kantenlänge < `kGeometryToleranceMm` → **leeres** `TopoDS_Shape`, übersprungen,
// kein Wurf) ist **defense-in-depth**: er stellt sicher, dass `MakeBox` nie mit
// gleichen/umgekehrten Ecken aufgerufen wird; die Aufrufer filtern degeneriertes
// Eingang ohnehin upstream (`stairStepBoxes` liefert nur positive Quader).
TopoDS_Shape makeBoxSolid(double x0_mm, double y0_mm, double z0_mm, double x1_mm,
                          double y1_mm, double z1_mm);

}  // namespace bcad::adapters::geometry
