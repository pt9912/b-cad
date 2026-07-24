#pragma once

#include "hexagon/model/building.h"
#include "hexagon/model/export_provenance.h"

namespace bcad::golden {

// Festes Byte-Golden-Referenzmodell (slice-044a): Wände (Aussen/Innen) + Decke +
// Sattel-Dach + gerade Treppe + eine sichtbare Ebene mit Hilfslinie — so trägt
// JEDES der sechs Austauschformate nicht-triviale Geometrie:
//   - DXF/PDF/PNG: den 2D-Grundriss (Wand-Achsen + sichtbare Hilfslinie);
//   - IFC: die Wand-Extrusionen (+ Material-Layer) — der SPF-Subset, kein Grundriss;
//   - STEP/STL: die B-Rep-/Netz-Solids aller Bauteile (Wände/Decke/Dach/Treppe).
//
// EINE geteilte TU (kein Copy-Drift, MR-006-044-LOW-3): sowohl der
// Golden-Vergleichs-Test (`test_golden_export.cpp`) als auch der `golden_gen`-
// Generator (`golden_gen.cpp`) linken diese Quelle und speisen so denselben
// Modell-Zustand in `golden-regen` (schreibt die committeten Golden) und in den
// Byte-Vergleich. Bewusst NICHT `buildAcc001KernDemo` (main-resident,
// änderungsanfällig, ohne Decke/Dach/Treppe — MR-006-044-MED-2).
hexagon::model::Building goldenModel();

// Feste Export-Provenance fürs Golden (slice-046): deterministisch — **kein**
// Wall-Clock, **keine** `BCAD_VERSION`-Kopplung (fester Versionsstring) → das
// Byte-Golden bleibt stabil, zeigt aber die **injizierte** Herkunft (Unterscheidbarkeit
// sichtbar im Golden). Test + `golden_gen` linken diese eine Quelle (kein Drift).
hexagon::model::ExportProvenance goldenProvenance();

}  // namespace bcad::golden
