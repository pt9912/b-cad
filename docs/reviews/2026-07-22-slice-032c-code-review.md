# Code-Review — slice-032c (DRW-Export: Hilfslinien im 2D-Grundriss, Ebenen-Sichtbarkeits-Filter)

**Datum:** 2026-07-22 · **Reviewer:** unabhängiger Agent (≠ Autor), read-only ·
**Latte:** Export (self-rolled-Writer-/Decode-Orakel-Klasse, Muster
[slice-025b](../plan/planning/done/slice-025b-pdf-export-impl.md)/[025c](../plan/planning/done/slice-025c-png-export-impl.md)) ·
**[MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a**
(Hilfslinie = 2-Punkt-`LINE`/-Segment, keine Solid-Geometrie).

**Verdikt: 0 HIGH / 1 MED / 1 LOW / 3 INFO → closure-fähig.** Die Kern-Korrektheit hält: alle drei
Formate wenden **denselben** `visibleLayerIds`-Filter an (identische Semantik, inkl. unbekannter
`layer_id` → nicht gezeichnet, leere `layers` → nichts gezeichnet), Koordinaten-/Geschoss-Zuordnung ist
richtig, der `accumulate`-Refactor ist verhaltensgleich zur alten Wand-BBox-Logik, `writeLayerTable`
(Geschoss-`LAYER`) ist unberührt. Kein Export-Defekt.

## Verifiziert korrekt

- **Filter-Konsistenz (primäre HIGH-Klasse) sauber.** DXF (`writeEntities`) und `projectPlan` (PDF/PNG)
  berechnen beide `visibleLayerIds` und testen `visible.contains(static_cast<int>(guide.layer_id))` —
  identisch. Unbekannte `layer_id` / leere `layers` → in **beiden** übersprungen. `Layer::visible`
  default `true`. Je Format ein „Fehlt"-Test.
- **Koordinaten/Geschoss:** DXF-`LINE` auf `layerName(guide.storey_id)` (`STOREY_n`, kein neuer Layer),
  10/20/11/21 aus `segment.start/end`, z=0, nach dem Wand-Loop. `projectPlan` filtert `storey_id` +
  Sichtbarkeit korrekt.
- **`writeLayerTable` unberührt** — Hilfslinien reiten den bestehenden Geschoss-`LAYER` (ADR-0018 §3).
- **ACC-002-Neutralität bestätigt** (INFO-3): `ViewModelPort`/`view_model_mesh_source` tragen **keine**
  `guide_line`/`layer`-Referenz — der headless-3D-Render der Demo bleibt unverändert.
- **Intra-io-Include ok** (INFO-2): `dxf_export_adapter.cpp` inkludiert `plan_geometry.h` (beide in
  `adapters/io/`) — intra-Modul, keine neue Regel-B-Kante; a-check/arch-check unverändert.

## Findings + Einarbeitung

**MED-1 (behoben) — BBox-Erweiterung für Hilfslinien außerhalb der Wand-Ausdehnung war implementiert,
aber von keinem Orakel gepinnt.** Der `accumulate(view, seg)` im Hilfslinien-Loop (`plan_geometry.cpp`)
verhindert das Abschneiden in PDF/PNG; **kein Test** hätte eine Regression gefangen, die ihn entfernt
(mein PNG-Test hatte die Hilfslinie *innerhalb* der Box → BBox identisch; PDF-`" l\n"`-Count ist
BBox-agnostisch). Der Plan §6 hatte genau diesen Test versprochen. **Fix:** neuer direkter
`plan_geometry`-Unit-Test (`test_plan_geometry.cpp`) mit einer Hilfslinie **außerhalb** der Wand-
Ausdehnung → asserted `view.max_x/max_y` = Hilfslinien-Endpunkte (BBox erweitert), sichtbar/unsichtbar.

**LOW-1 (behoben) — PDF/PNG-Orakel pinnten die Hilfslinien-Koordinaten nicht (nur Präsenz); der
`projectPlan`-Pfad war swap-blind.** Die 032b-„distinkte-Koordinaten-fängt-Swap"-Lehre war nur auf den
DXF-Pfad angewandt. **Fix:** derselbe `plan_geometry`-Unit-Test asserted die projizierten Hilfslinien-
Koordinaten exakt (x1/y1/x2/y2, distinkt) → ein start/end- oder x/y-Swap im `PlanSegment`-Bau fällt durch.

**INFO-1 — Orphan-Geschoss-Drift** (DXF zeichnet eine Hilfslinie auf `STOREY_n` unabhängig davon, ob
Geschoss `n` existiert; `projectPlan` filtert auf existierende Geschosse) — spiegelt das bestehende
Wand-Verhalten und ist durch den `guide_lines.storey_id`-FK (`cascade`) unerreichbar. Bestehend, kein
032c-Defekt.

**Nachweis der Fixes:** `make gates` grün nach Einarbeitung — **245 Tests** (+3 `PlanGeometry`),
Coverage 90,7 %.
