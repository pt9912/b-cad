#pragma once

#include <map>

#include "hexagon/model/stair.h"  // StairId

namespace bcad::hexagon::model {

// Kern-abgeleitete Persistenz-Skalare (ADR-0020, slice-042d): der Kern
// berechnet die **write-derived** Werte, die der Persistenz-Adapter nur noch
// **serialisiert** — er leitet nichts mehr selbst aus `services/geometry` ab
// (analog dem `DerivedGeometry`-Bündel bei STEP/STL, aber **eigene** Naht: die
// Persistenz braucht keine Export-Geometrie → Kopplung vermeiden). Ein pures
// Werttyp-Aggregat (nur `model/` + Standardbibliothek). `map<StairId,…>` =
// order-unabhängig/explizit (kein fragiler positionaler Vektor).
struct PersistedDerivations {
    // Abgeleitete Steigung je Treppe (`services::stairRiseMm`), gebunden an
    // `stairs.rise_mm`. Fehlt der Skalar zu einer `stair.id`, bindet der
    // Adapter **fail-closed** (`.at()` wirft → Rollback, kein stilles
    // `rise_mm=0`).
    std::map<StairId, double> stairRiseMm;
};

}  // namespace bcad::hexagon::model
