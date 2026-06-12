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
- **Geschlossene Außenecken** (LH-FA-WAL-006-Teilumfang, slice-012):
  endpunkt-verbundene Wände verschneiden sich — keine Eck-Kerben
  (2D-Verifikation:
  [`acc-002-befund-2d-ecken-geschlossen.png`](acc-002-befund-2d-ecken-geschlossen.png);
  Befund-Bild Runde 1 daneben).
- Trennwand teilt den Grundriss — beide Raumvolumina einsehbar
  (perspektivische Orbit-Ansicht von schräg oben, Lambert-Shading);
  ihr T-Stoß bleibt vereinbarungsgemäß stumpf (kein gemeinsamer
  Endpunkt — WAL-006-Vollumfang, spätere Welle).
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
- **Code-Stand:** Commit `8fe8dad` (slice-011b-Viewer + slice-012-
  Eckenschluss); `make gates` grün auf diesem Stand (63/63 Tests,
  Coverage 94,2 %).
- **Datum:** 2026-06-12 (Runde 2; Runde-1-Render vom Stand `ba61087`
  ersetzt).

## Abnahme (Projektinhaber)

- [ ] *Ausstehend (Runde 2):* „Das Gebäude wird automatisch als
  3D-Modell dargestellt" (ACC-002) ist am Beleg-Bild
  benutzer-beobachtbar erfüllt; der Runde-1-Befund (offene
  Außenecken) ist behoben. — Dietmar Burkard, Datum

**Abnahme-Befund 2026-06-12 (Runde 1):** Abnahme zurückgestellt —
die Wände schließen an den Außenecken nicht (fehlendes
½×½-Stärke-Quadrat; Befund-Grundriss:
[`acc-002-befund-2d-ecken.png`](acc-002-befund-2d-ecken.png)).
Ursache ist fehlender LH-FA-WAL-006-Umfang, kein Viewer-Fehler →
Eckenschluss vorgezogen als
[slice-012](../in-progress/slice-012-eckenschluss-wal006-teil.md); der
Beleg wird danach regeneriert und erneut vorgelegt.
