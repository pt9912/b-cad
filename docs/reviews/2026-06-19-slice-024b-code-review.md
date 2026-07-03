# MR-009 Geometrie-Code-Review — slice-024b (Treppen STEP-B-Rep)

**Datum:** 2026-06-19. **Reviewer:** unabhängig (≠ Autor, ohne Autoren-Kontext),
adversarial, Geometrie-Korrektheits-Fokus, **read-only** (Arbeitsbaum unverändert —
Lehre aus 024a). **Gegenstand:** die slice-024b-Implementierung (`stairStepBoxes`-Query
+ `stairMesh`-Refaktor + `makeBoxSolid` + Treppen-Schleife in `buildSolidCompound`),
per [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure).

## Verifikation

- **Refaktor-Äquivalenz (`stairMesh` alt vs. neu) — IDENTISCH:** gegen
  `git show HEAD:src/hexagon/services/stair_geometry.cpp` difft: die Stufen-Box-Extents <!-- d-check:ignore (historisch: Pfad vor slice-028-Move nach services/geometry/) -->
  (`x_start+i·tread`, `x_start+(i+1)·tread`, `y0`, `y0+width`, `0`, `(i+1)·rise`) und die
  Geländer-Boxen (`rail_t=min(50,width/4)`, `rail_h=kStairRailingHeightMm`, beidseitig,
  z `[top,top+rail_h]`) sind **byte-gleich**; Emissions-Reihenfolge (Stufe, −y-Geländer,
  +y-Geländer) unverändert. Der **Totalitäts-Guard** von `stairStepBoxes` ist der **exakte**
  alte `stairMesh`-Guard (neun Bedingungen, gleiche Schwellen). Kein Vertex-Drift; die vier
  bestehenden Mesh-Orakel (geschlossen/orientiert, `step_count+1` bündige x-Kanten, Geländer-
  Zwei-Sonde, Lauflänge) blieben grün (Tests 76–83).
- **`makeBoxSolid`:** Guard verwirft jede Kante `< kGeometryToleranceMm` → `MakeBox` wird
  nie mit gleichen/umgekehrten Ecken aufgerufen; Box-Solids sind inhärent geschlossen/
  außen-orientiert (kein `BRepCheck` nötig, anders als der Sewing-Pfad). Parameter-Reihenfolge
  `(x0,y0,z0,x1,y1,z1)` über Header/Definition/Aufrufstelle konsistent.
- **Höhe/Schicht/Kapselung:** Treppen-Höhe via `storeyHeight(building, s.from_storey_id)`
  (deckungsgleich mit Display/STL/Volumen/Persistenz). Per-Bauteil-`try/catch` spiegelt
  Wände/Decken/Dächer (Totalität). **Kein OCC-Typ-Leck** über `ModelExporterPort`
  (`step_export_adapter.h` exponiert nur `Building`+`path`; OCC nur in `.cpp`/`occ_solids.h`,
  Regel C; `Standard_Failure` neutralisiert). arch-check grün.
- **Orakel-Solidität:** „**genau** `wallShells + step_count` `CLOSED_SHELL`" beweist dual,
  dass (a) **alle** Stufen exportieren **und** (b) das **Geländer ausgelassen** ist (mit
  Geländer wären es `3·step_count`). Die x-benachbarten Stufen-Boxen sind **getrennte**
  Compound-Member (kein Fuse) → `STEPControl_Writer`/`AsIs` schreibt je ein
  `MANIFOLD_SOLID_BREP`/`CLOSED_SHELL` ohne Merge berührender Solids (Gate-Beleg: Test 184).

## Findings

### LOW-1 — Stale Klassen-Doku in `step_export_adapter.h`
Der Header las „**Benannte Lücke (slice-020b):** Dächer **und** Treppen … werden hier (noch)
**nicht** geschrieben" — nach 024a (Dächer) + 024b (Treppen) **falsch** (Doku-Drift, keine
Geometrie-Wirkung; §1 war korrekt nachgezogen).
→ **Eingearbeitet:** Header beschreibt jetzt alle drei B-Rep-Pfade (Wände/Decken `makeNetSolid`,
Dächer `meshToSolid`-Vernähung, Treppen-Stufen `makeBoxSolid`) + Geländer-Auslassung.

### LOW-2 — `makeBoxSolid`-Degenerations-Guard ungetestet (Test-Adäquanz)
Coverage markiert den Guard-Zweig uncovered: kein Test treibt einen sub-Toleranz-Quader in
`makeBoxSolid` (`stairStepBoxes` filtert degeneriert upstream → der Guard ist für den Treppen-
Pfad effektiv unerreichbar). Der Helfer gibt ein OCC-`TopoDS_Shape` zurück → in der bewusst
OCC-freien Test-Schicht (Regel C) nicht direkt prüfbar.
→ **Eingearbeitet (Reviewer-Alternative):** Guard im Header als **defense-in-depth** benannt
(Aufrufer filtern upstream); kein OCC-Test in der Test-Schicht erzwungen. Coverage-Gate grün
(90,3 % ≥ 70 %).

### INFO-1 — Kommentar-Ungenauigkeit „überlappende Stufen-Quader"
`step_export_adapter.cpp`: die Boxen **berühren** sich an gemeinsamen x-Ebenen (partiell
koinzidente Flächen), sie **überlappen** nicht volumetrisch. Die Nicht-Manifold-Begründung
bleibt korrekt.
→ **Eingearbeitet:** Kommentar präzisiert (berührend/partiell koinzident, getrennte Compound-Member).

## Verdikt

Keine HIGH/MEDIUM-Geometrie-Defekte: Orientierung (Box-Solids inhärent außen-orientiert; Mesh-
Wicklung unverändert + re-bewiesen), Wasserdichtheit (jede Box ein geschlossenes Manifold-Solid;
Geländer korrekt ausgeschlossen), Maß-Exaktheit (Extents bit-identisch zum committeten Mesh,
`EXPECT_DOUBLE_EQ`-Box-Orakel + µm-Bündigkeits-Orakel), Totalität (Guard exakt wie zuvor) — alle
solide. Gates grün: 208/208, arch-check ok, Coverage 90,3 %.

**VERDICT: 0 HIGH, 0 MEDIUM, 2 LOW, 1 INFO — CLOSURE CLEARED**

## Einarbeitung (Autor, 2026-06-19)
LOW-1 (Header-Doku auf alle 3 B-Rep-Pfade + Geländer-Auslassung aktualisiert), LOW-2
(`makeBoxSolid`-Guard als defense-in-depth/upstream-gefiltert benannt), INFO-1 (Kommentar
„berührend/partiell koinzident" präzisiert) eingearbeitet. **0 HIGH → Closure frei.**
