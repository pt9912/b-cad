# ACC-002-Beleg — sichtbare 3D-Darstellung (welle-1v-viewer)

**Artefakt:** [`acc-002-beleg.png`](acc-002-beleg.png) (1280×800,
deterministischer Offscreen-Render).
**Manueller Abnahme-Schritt — kein Gate** (ADR-0009 (f), slice-011b
DoD-4).

## Verwendetes Projekt

ACC-001-Kern-Demo aus der Composition Root
(`src/main.cpp` §`buildAcc001KernDemo`): EG 8 m × 6 m Außenwände +
Trennwand bei x = 3 m (→ **zwei automatisch erkannte Räume**,
LH-FA-ROM-001), OG (2700 mm) mit vier Außenwänden; abschließend eine
**committete Parameteränderung** (Trennwand-Stärke → 115 mm) — der
dargestellte Stand folgte ihr über den ADR-0008-Vertrag ohne
Benutzer-Refresh (sichtbare Hälfte LH-FA-D3-002).

## Sichtbare Soll-Merkmale

- Extrudierte Wände als 3D-Solids (LH-FA-D3-001): geschlossener
  8×6-m-Wandzug, Wandstärken körperlich sichtbar.
- Trennwand teilt den Grundriss — beide Raumvolumina einsehbar
  (perspektivische Orbit-Ansicht von schräg oben, Lambert-Shading).
- Die Trennwand ist sichtbar dünner (115 mm) als die Außenwände
  (240 mm) — der Render zeigt den Stand **nach** der Mutation.

**Ehrlichkeits-Anmerkung:** Das Domänenmodell trägt noch keinen
Geschoss-Höhenversatz (Geschoss-Stapelung kommt mit Decken/Dach,
welle-2) — die OG-Wände extrudieren ab z = 0 und liegen deckungsgleich
im EG-Umriss (9 Wand-Netze, sichtbar als ein Baukörper mit leicht
höherem OG-Rand: 2700 vs. 2500 mm).

## Erzeugung

- **Kommando:** `make acc-002-beleg` (rendert headless via Xvfb +
  Mesa/llvmpipe, ADR-0010; Target ist **nicht** in `make gates`).
- **Code-Stand:** Commit `ba61087` (slice-011b-Implementierung);
  `make gates` grün auf diesem Stand (57/57 Tests, Coverage 93,7 %).
- **Datum:** 2026-06-12.

## Abnahme (Projektinhaber)

- [ ] *Ausstehend:* „Das Gebäude wird automatisch als 3D-Modell
  dargestellt" (ACC-002) ist am Beleg-Bild benutzer-beobachtbar
  erfüllt. — Dietmar Burkard, Datum

**Abnahme-Befund 2026-06-12 (Runde 1):** Abnahme zurückgestellt —
die Wände schließen an den Außenecken nicht (fehlendes
½×½-Stärke-Quadrat; Befund-Grundriss:
[`acc-002-befund-2d-ecken.png`](acc-002-befund-2d-ecken.png)).
Ursache ist fehlender LH-FA-WAL-006-Umfang, kein Viewer-Fehler →
Eckenschluss vorgezogen als
[slice-012](../in-progress/slice-012-eckenschluss-wal006-teil.md); der
Beleg wird danach regeneriert und erneut vorgelegt.
