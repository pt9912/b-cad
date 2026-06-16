#pragma once

#include <vector>

#include "hexagon/model/material.h"

namespace bcad::hexagon::model {

// Eine Zeile der Materialliste (LH-FA-EVL-004): ein Material + die Anzahl der
// ihm zugewiesenen Bauteile + die summierte Menge = Netto-Volumen in m³.
// welle-3: Menge = Σ Netto-Volumen über Wand + Decke/Fundament (EVL-002/
// `volume_geometry`); das Dach ist material-tragend, sein Volumen aber
// welle-3 zurückgestellt → **nicht** enthalten (benannte Lücke, spez. §1).
// Das ganze `Material` (Kennwerte) wird getragen → trägt die spätere
// Kosten-Auswertung. Pure Werte, framework-frei (ADR-0001).
struct MaterialLine {
    Material material{};
    int component_count{0};
    double quantity_m3{0.0};
};

// Ergebnis der Materiallisten-Auswertung (LH-FA-EVL-004): je Material eine
// Zeile (nach `MaterialId` sortiert → deterministisch) + die Gesamtmenge.
// Bauteile **ohne** Material erscheinen nicht (Boundary, kein Fehler).
// Read-only abgeleitet, nie persistiert; die Auswertung mutiert nichts.
struct MaterialReport {
    std::vector<MaterialLine> lines;
    double total_m3{0.0};
};

}  // namespace bcad::hexagon::model
