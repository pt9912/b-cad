# Code-Review — slice-032b (DRW-Impl: Layer/GuideLine + EditDrawingPort + guide_lines-Persistenz)

**Datum:** 2026-07-22 · **Reviewer:** unabhängiger Agent (≠ Autor), read-only ·
**Latte:** Persistenz (stille-Verfälschungs-/Round-Trip-Klasse, Muster
[slice-017e](../plan/planning/done-archive/slice-017e-material-persistenz.md)) ·
**[MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a**
(Hilfslinie = 2 Punkte, keine Solid-Geometrie).

**Verdikt: 0 HIGH / 1 MED / 2 LOW / 2 INFO → closure-fähig.** Der Persistenz-Round-Trip ist
**korrekt**: jede Bind-Position und jeder Spalten-Index wurde gegen die INSERT-/SELECT-Spaltenliste
geprüft — kein Off-by-one, kein Swap, ID-erhaltend, boolean- und NULL-sicher. FK-Insert-Reihenfolge
solide, alle Ablehnungspfade lassen das Modell unverändert, ADR-0018 vollständig erfüllt. **Kein
Datenkorruptions-Defekt.**

## Verifiziert korrekt (die HIGH-Klasse)

- **Bind/Spalten-Mapping exakt.** `insertLayers` `VALUES (?,1,?,?,?,?)` (id/name/visible/locked/color_hex)
  ↔ `loadLayers` `SELECT id,name,visible,locked,color_hex` (col0..4). `insertGuideLines`
  `VALUES (?,1,?,?,?,?,?,?)` (id/storey_id/layer_id/start_x/start_y/end_x/end_y) ↔ `loadGuideLines`
  identische Spaltenordnung; `Point2D`-Aggregat-Init bildet start/end korrekt. IDs via
  `bind_int(static_cast<int>(id))` erhalten; `visible`/`locked` als INTEGER 0/1 (`!=0` gelesen);
  `color_hex` via `bindOptionalText`/`columnOptionalText` → NULL → `nullopt` (nicht `""`).
- **FK-Insert-Reihenfolge korrekt.** `save()`: storeys → layers → … → guide_lines, alles in
  `BEGIN/COMMIT` mit `PRAGMA foreign_keys=ON` vor BEGIN. guide_lines nach beiden Eltern (storeys, layers).
- **restrict beidseitig konsistent.** App: `removeLayer` lehnt via `layerReferenced` ab (false, Modell
  unverändert); DB: `schema.sql` rendert `layer_id … ON DELETE RESTRICT`.
- **Validierungs-Totalität.** `addLayer` (blank + projekt-doppelt, App-Guard == byte-exakter
  `uq_layers_project_name` → kein Save-Zeit-Constraint-Wurf), `renameLayer` (unbekannt/blank/Kollision
  mit Self-Exclude), `addGuideLine` (Entartung/unbekannte Ebene/unbekanntes Geschoss — alles **vor**
  jedem push). Kein mutate-then-fail-Pfad.
- **ADR-0018 vollständig konform.** `EditDrawingPort` echter eigener Port (keine `EditStructurePort`-
  Erweiterung), am geteilten `StructureEditService`/einer `Building`-Instanz (EvaluatePort-Muster), **kein**
  `op`, direkter typisierter `layer_id`-Bezug (nicht `entity_layers`), framework-freie Werttypen.

## Findings + Einarbeitung

**MED-1 (behoben) — Round-Trip-Orakel fing keinen `storey_id`↔`layer_id`-Swap.** Beide Hilfslinien-
Zeilen in `drawingBuilding()` hatten `storey_id == layer_id` (1,1 und 2,2) — ein Swap der Bind-Positionen
(pos2↔pos3) oder Spalten-Indizes (col1↔col2) hätte beide FKs weiter erfüllt und gleiche Werte geliefert:
der Test wäre **grün geblieben, während jede Hilfslinie stumm auf der falschen Ebene lädt.** Der Code ist
korrekt; das Orakel war blind. **Fix eingearbeitet:** g2 auf `StoreyId{2}, LayerId{1}` gesetzt (storey ≠
layer) + beide Felder asserted → ein Swap fällt jetzt durch.

**LOW-2 (behoben) — g2-Koordinaten nur partiell asserted.** Nur `g2.end.y` geprüft; `start.x/y` + `end.x`
ergänzt (Zeile self-sufficient).

**LOW-1 (kein Change, bewusst) — Entartungs-Check ≠ Wand-Präzedenz.** `addGuideLine` nutzt `nearPoint`
(Per-Achse-Box) statt `segmentLength < tol` (euklidisch wie `addWall`). `nearPoint` ist die **strengere**
„keine Ausdehnung"-Lesart und deckt sich mit der Port-Doku — verteidigbar, kein Defekt.

**INFO-1 — decimal(12,3) → REAL (W200).** Harmlos: das Modell hält `double` mm, REAL = IEEE-754-double,
double→REAL→double exakt (Wand-Koordinaten-Präzedenz).

**INFO-2 — `next_layer_id_`/`next_guide_line_id_` nach load nicht restauriert.** Kein load→mutate-Pfad in
diesem Slice (`load()` liefert ein bares `Building`, keinen hydrierten Service) — konsistent mit
`next_material_id_` u. a., out of scope.

**Nachweis der Fixes:** `make gates` grün nach Einarbeitung (Round-Trip-Test mit storey ≠ layer + voller
Koordinaten-Assertion).
