---
id: slice-024b
titel: Treppen STEP-B-Rep — analytische OCC-Box-Solids (Stufen), Geländer ausgelassen
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-STR-001](../../../../spec/lastenheft.md#lh-fa-str-001--treppe-erzeugen)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 024b: Treppen STEP-B-Rep

**Status:** done (2026-06-19). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review (gemeinsam mit 024a; kein offener HIGH
auf 024b) **+** unabhängiges [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Geometrie-Code-Review **0 HIGH**
([Report](../../../reviews/2026-06-19-slice-024b-code-review.md); Refaktor byte-identisch verifiziert,
2 LOW/1 INFO eingearbeitet). DoD vollständig, `make gates` grün (208/208). Closure-Notiz §8.

**Welle:** welle-4-austausch. **Split-Hälfte** der STEP-B-Rep-Schließung (Projektinhaber-Entscheidung
2026-06-19): **024a = Dächer** (Vernähung), **024b = Treppen** (analytische Box-Solids). Schließt **die
Treppen-Hälfte** der in [slice-020b](../done-archive/slice-020b-step-stl-export.md) benannten STEP-Lücke —
**danach sind alle 3D-Bauteile B-Rep** (Wände/Decken seit 020b, Dächer seit 024a, Treppen hier).

**Bezug:** [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) (STEP-Export — B-Rep aller 3D-Bauteile),
`spec/spezifikation.md` §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005) (024a schärfte die Dach-Hälfte;
024b schließt die **Treppen**-Hälfte), [LH-FA-STR-001](../../../../spec/lastenheft.md#lh-fa-str-001--treppe-erzeugen)
(gerade einläufige Treppe). **Parametrisiert auf [ADR-0014](../../adr/0014-step-stl-export-backend.md)** (OCC-DataExchange
nativ, geometrie-residenter `ModelExporterPort`), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (OCC nur im
Geometrie-Adapter, Regel C), [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt, neutraler Wurf).
**Kein neuer ADR** (§6 — selbe Backend-Entscheidung wie 020b/024a; die Treppen-Analytik ist Implementierungs-
Taktik im selben Backend).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-19.

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start), Reviewer ≠ Autor):** Das Review lief auf dem
**Unified-Plan** (Dächer + Treppen) — [Report](../../../reviews/2026-06-19-slice-024-plan.md), **1 HIGH / 2 MED /
3 LOW / 3 INFO**, Quellen-Konsistenz durchgehend grün (jede Code-Behauptung gegen den realen Adapter
verifiziert; insbesondere: `stairMesh` ist eine **nicht-manifolde Box-Union**, bestätigt). **Auf 024b
entfallen: MED-2** (Geländer ist render-only mit Heuristik-Dicke → **aus dem STEP-B-Rep ausgelassen**, bleibt
in STL; in §1 benannt — revidiert den Unified-Default „mitexportieren") und **LOW-2** (`stairStepBoxes`-Kern-
Query als **eine** Box-Wahrheit für `stairMesh` + Adapter). **HIGH-1 entfällt auf 024a** (Dach-Vernähung) —
024b hat **keinen** offenen HIGH. **Start frei.**

---

## 1. Ziel

Der **STEP-Export** ([LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)) deckt jetzt auch **Treppen** als
B-Rep-Solids: die **Stufen** werden als **analytische OCC-Box-Solids** aus den `Stair`-Parametern
rekonstruiert und dem STEP-Compound hinzugefügt — **nicht** aus dem flachen `stairMesh` vernäht (die
Stufen-Boxen sind eine **nicht-manifolde Union**, s. §6). Das **Geländer** (render-only) wird **ausgelassen**
(bleibt im STL). Alles **geometrie-resident** (OCC nur in `src/adapters/geometry/`, Regel C), **atomar**
([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)), Kern bleibt format-frei. **Danach sind alle 3D-Bauteile B-Rep** — die
020b-STEP-Lücke ist vollständig geschlossen.

## 2. Definition of Done

- [x] **DoD-1 — Kern-Query `stairStepBoxes` (eine Box-Wahrheit; LOW-2).** Eine **pure Kern-Query**
      `stairStepBoxes(stair, from_storey_height) → std::vector<StepBox>` in `stair_geometry` (Box-Extents je
      Stufe: `x∈[x0+i·tread, x0+(i+1)·tread] · y∈[y0,y1] · z∈[0,(i+1)·rise]`, abgeleitete Steigung via
      `stairRiseMm`). `stairMesh` wird **darauf refaktoriert** (Stufen-Boxen aus der Query; Geländer-Boxen
      weiterhin separat) → **eine** Box-Wahrheit für Netz **und** STEP-Adapter, kein Formel-Duplikat.
      **Totalität:** `step_count < 1` / degeneriert → **leere** Box-Liste (kein Wurf), wie heute das Netz.
      Bestehende `stairMesh`-/`test_stair_geometry`-Tests bleiben grün (Refactoring-Regression).
- [x] **DoD-2 — Treppen-B-Rep via analytische Box-Solids im Adapter.** `buildSolidCompound` (im
      `StepExportAdapter`) baut je `StepBox` ein OCC-Box-Solid (`BRepPrimAPI_MakeBox` o. Ä.; geteilter
      `makeBoxSolid`-Helfer in `occ_solids`, Regel C) und fügt es **einzeln** dem Compound hinzu — **kein**
      `BRepAlgoAPI_Fuse` (berührende Solids im Compound sind valide, Muster Wände/Decken). **Begründung
      (nicht vernähen):** `stairMesh` ist eine **nicht-manifolde Union** (x-benachbarte Stufen-Boxen mit
      partiell koinzidenten Flächen) → `BRepBuilderAPI_Sewing` über das ganze Netz ergäbe keine gültige
      Einzel-Solid. **Totalität:** leere Box-Liste → keine Stufen-Solids (kein Wurf).
- [x] **DoD-3 — Geländer aus dem STEP-B-Rep ausgelassen (explizit, MED-2).** Das Geländer ist **render-only**
      (nicht persistiert, Heuristik-Dicke `kRailThicknessMm`) — es bleibt im **STL** (Display/Druck), wird
      aber **nicht** ins STEP-**B-Rep** geschrieben (CAD-Austausch-Strukturmodell = nur Stufen).
      `stairStepBoxes` liefert nur die Stufen; die Auslassung ist in §1 **explizit benannt** (kein stiller
      Teilumfang).
- [x] **DoD-4 — Spec §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005): Treppen-Hälfte geschärft, Lücke
      geschlossen (kein Lastenheft).** §1 schärft die **Treppen**-Hälfte (technisch, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei): Spec,
      **nicht** Lastenheft): **Treppen-Stufen** = analytische OCC-Box-Solids; **Geländer ausgelassen**
      (render-only, STL). Damit ist die 020b-STL-only-Lücke **vollständig geschlossen** (alle 3D-Bauteile
      B-Rep). `spezifikation-historie.md` nachgezogen. **Lastenheft unberührt**
      ([LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)-AK wird *erfüllt*).
- [x] **DoD-5 — AK-Tests + OCC-freie Invarianten ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Orakel; LOW-3).** Die STEP-Fixture in
      `tests/adapters/test_step_stl_export.cpp` bekommt **mindestens eine Treppe** — **OCC-frei** (Text-
      Re-Read, Regel C). **Sensor (nicht „nicht-leer"):** das ISO-10303-21-Text-Orakel assertiert die
      **erwartete Anzahl** `MANIFOLD_SOLID_BREP`/`CLOSED_SHELL` = `step_count` (**ohne** Geländer — DoD-3).
      **Boundary/Regression (LOW-3):** `EmptyBuildingYieldsValidStepEnvelope` bleibt grün; ein Modell **mit
      Wänden, ohne Treppe** assertiert **keinen** Solid-Zuwachs. **STL-Regression** unverändert grün (Treppe
      + Geländer waren dort schon enthalten).
- [x] **DoD-6 — Schicht + Gates + Reviews + Closure.** `arch-check` **Regel C** deckt die OCC-Nutzung (kein
      neuer Regelbedarf, [ADR-0014](../../adr/0014-step-stl-export-backend.md)); OCC-Typen lecken **nicht** über `ModelExporterPort`.
      `make gates` grün. **Unabhängiges [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review 0 HIGH** (Korrektheit/
      Vollständigkeit der Stufen-Box-Solids; Geländer-Auslassung; OCC-Typ-Kapselung). `CHANGELOG.md` +
      `roadmap.md`; Closure-Notiz; Plan nach `done-archive/` (reiner `git mv`,
      [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)). **STEP-B-Rep-Lücke vollständig geschlossen** → welle-4 offen nur noch für
      **PDF/PNG** ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) + Welle-4-Verifikation → M4.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/services/stair_geometry.{h,cpp}` | ändern | pure Query `stairStepBoxes(stair, height) → vector<StepBox>` (eine Box-Wahrheit, LOW-2); `stairMesh` darauf refaktoriert |
| `src/adapters/geometry/occ_solids.{h,cpp}` | ändern | `makeBoxSolid(StepBox/Quader) → TopoDS_Shape` (analytisches Box-Solid) |
| `src/adapters/geometry/step_export_adapter.cpp` | ändern | `buildSolidCompound`: Treppen-Stufen-Box-Solids ergänzen (Geländer ausgelassen) |
| `spec/spezifikation.md` (+ `spezifikation-historie.md`) | ändern | §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005): Treppen-Stufen B-Rep, Geländer-Auslassung; Lücke geschlossen ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) |
| `tests/adapters/test_step_stl_export.cpp` | ändern | Fixture um Treppe; OCC-frei `MANIFOLD_SOLID_BREP`/`CLOSED_SHELL`-Anzahl + Kein-Zuwachs-Regression |
| `tests/hexagon/test_stair_geometry.cpp` | ggf. ändern | `stairStepBoxes`-Query direkt prüfen (Box-Extents je Stufe) |
| `CHANGELOG.md` · `roadmap.md` | ändern (Closure) | 024b done; STEP-B-Rep-Lücke vollständig geschlossen |
| `docs/reviews/{…-slice-024b-code-review}.md` | neu (Closure) | [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report |

**Kern-Berührung:** nur die **pure Query** `stairStepBoxes` in `stair_geometry` (additiv, analytisch, **kein**
OCC/Framework — Muster `roof_geometry`/`stair_geometry`) → `arch-check` Regel A/C unberührt. **Kein** neuer ADR.

## 4. Trigger

- **Startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review **und** 024a done (`buildSolidCompound`/`occ_solids`/
  `meshToSolid`-Infrastruktur + STEP-Fixture-Erweiterung stehen).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH**, Closure-Notiz → die STEP-B-Rep-Lücke ist
  **vollständig** geschlossen (alle 3D-Bauteile B-Rep); welle-4 offen nur noch für **PDF/PNG**
  ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) + Welle-4-Verifikation → M4.

## 6. Risiken und offene Punkte

- **Treppen-Netz ist nicht-manifold (PRIMÄR-Begründung, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)):** `stairMesh` ist eine **Union
  individuell-wasserdichter Boxen** (je Stufe ein Quader `z∈[0,(i+1)·rise]`, x-benachbart → **partiell
  koinzidente** Flächen an den gemeinsamen x-Ebenen; + Geländer-Boxen). `BRepBuilderAPI_Sewing` über das
  **ganze** Netz erzeugt interne/koinzidente Flächen → **keine** gültige Einzel-Solid. **Entscheidung:**
  Treppen-Stufen **analytisch als Box-Solids** (DoD-2), **nicht** vernäht — robuster, ohne Toleranz-
  Fragilität. Das korrigiert die Roadmap-Verkürzung „Mesh→Shape-Vernähung" (gilt für Dächer, nicht Treppen).
- **Box-Wahrheit-Duplikat (LOW-2, mitigiert):** ohne geteilte Query würde der Adapter die Box-Formeln aus
  `stair_geometry` duplizieren → stille Drift bei Tread-/Width-/Start-Änderung. **Mitigation:** `stairStepBoxes`
  als **eine** Quelle, von `stairMesh` **und** Adapter konsumiert (DoD-1).
- **Geländer-Auslassung (MED-2):** render-only Heuristik gehört nicht ins CAD-Austausch-B-Rep → ausgelassen,
  bleibt in STL; in §1 **explizit benannt** (kein stiller Teilumfang in beide Richtungen).
- **STEP-Compound aus berührenden Solids:** zulässig (keine Manifold-Anforderung über Compound-Member;
  CAD-Assemblies tun das). **Kein** `Fuse` (teuer, kann scheitern) — Stufen-Boxen bleiben getrennte
  Compound-Solids (Muster Wände/Decken).
- **OCC-Typ-Leck (Regel A/C):** der Adapter nutzt OCC direkt (erlaubt in `geometry/`), darf **keinen** OCC-Typ
  über `ModelExporterPort` lecken (neutraler Wurf).
- **Kein neuer ADR (begründet):** selbe Backend-Entscheidung wie [ADR-0014](../../adr/0014-step-stl-export-backend.md); die Treppen-Analytik
  ist Implementierungs-Taktik (kein neuer Layer/Dependency/Port). Vom [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) bestätigt (INFO-2).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (Hexagon-Kern)

- **Modus:** GF; **Konventionen-Dichte:** hoch — `stairStepBoxes` ist pure analytische Query (kein OCC/
  Framework, Muster `roof_geometry`/`stair_geometry`). **Phase-Reife:** Phase 4. **Risiko:** niedrig (additive
  Query; Refactoring von `stairMesh` durch bestehende Tests gedeckt).

### Sub-Area: Geometrie-Kern-Adapter (`src/adapters/geometry/`)

- **Modus:** GF; **Konventionen-Dichte:** hoch — OCC gekapselt (Regel C), neutraler Wurf, geteilter
  `occ_solids`-Builder. **Phase-Reife:** Phase 4. **Evidenz-/Diskrepanz-Risiko:** **mittel** (analytische
  Box-Solids = neue Geometrie) → [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) + Gültigkeits-Invariante.

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; **Dichte:** hoch (OCC-freies Text-Re-Read-Orakel + Solid-Anzahl == `step_count` statt
  „nicht leer"; `stairStepBoxes`-Direkttest). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-19):**

- **Box-Wahrheit (DoD-1):** neue pure Kern-Query `stairStepBoxes(stair, height) → vector<StepBox>` in
  `stair_geometry` (Stufen-Extents je Stufe); `stairMesh` darauf **refaktoriert** (Stufen aus der Query,
  Geländer separat daraus abgeleitet) — **byte-identisch** zum committeten Netz ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) via `git show HEAD`
  verifiziert; alle bestehenden `test_stair_geometry`-Orakel grün). Direkter Box-Truth-Test
  (`StepBoxesTragenDieStufenWahrheit`: Anzahl == `step_count`, Extents exakt, degeneriert → leer).
- **Box-Solids (DoD-2):** neuer geometrie-residenter `makeBoxSolid` (`occ_solids`, `BRepPrimAPI_MakeBox`,
  Degenerations-Guard als defense-in-depth); `buildSolidCompound` baut je `StepBox` ein Solid und fügt es
  **einzeln** dem Compound hinzu (kein `Fuse`, Per-Bauteil-`try/catch`). **Nicht** aus dem flachen
  `stairMesh` vernäht (nicht-manifolde Box-Union, §6).
- **Geländer ausgelassen (DoD-3):** `stairStepBoxes` liefert nur die Stufen; das render-only Geländer
  bleibt dem STL vorbehalten — in §1 explizit benannt.
- **Spec (DoD-4):** §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005) geschärft (Treppen-Stufen B-Rep,
  Geländer-Auslassung) → **alle 3D-Bauteile B-Rep**, die 020b-STL-only-Lücke ist **vollständig geschlossen**.
  `spezifikation-historie.md` nachgezogen. **Lastenheft unberührt** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).
- **Test (DoD-5):** `StairStepsYieldClosedShellBRepSolids` **OCC-frei** — die Treppe trägt **genau
  `step_count`** zusätzliche `CLOSED_SHELL` bei (12 Stufen → +12); die **exakte** Anzahl belegt zugleich,
  dass das **Geländer ausgelassen** ist (mit Geländer wären es `3·step_count`). + Kein-Zuwachs-Regression.
- **Gates (DoD-6):** `make gates` grün (**208/208**, arch-check Regel C, lint 0, docs-check 0); kein
  OCC-Typ-Leck über `ModelExporterPort`.
- **Reviews:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) (gemeinsam, kein offener HIGH auf 024b) + **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  0 HIGH** ([Report](../../../reviews/2026-06-19-slice-024b-code-review.md)): Refaktor byte-identisch belegt,
  Orakel solide (exakte Anzahl); 2 LOW (stale Header-Doku; `makeBoxSolid`-Guard defense-in-depth) + 1 INFO
  (Kommentar-Präzision „berührend statt überlappend") eingearbeitet.

**Lerneintrag:**

- **Analytische Rekonstruktion statt Vernähung — die richtige Wahl für nicht-manifolde Netze:** das
  flache `stairMesh` ist eine Union x-benachbarter, an gemeinsamen x-Ebenen **berührender** Stufen-Boxen
  (partiell koinzidente Flächen) → `BRepBuilderAPI_Sewing` über das Ganze ergäbe **keine** gültige
  Einzel-Solid. Die Stufen **analytisch** als getrennte `BRepPrimAPI_MakeBox`-Solids im Compound (kein
  `Fuse`) ist robust und schema-konform (STEP-Compound darf berührende Solids tragen). **Komplement zu
  024a:** Mesh→Shape-Vernähung gilt für *wasserdichte* Netze (Dächer), analytische Rekonstruktion für
  *nicht-manifolde* (Treppen).
- **Eine Box-Wahrheit (`stairStepBoxes`):** dieselbe pure Kern-Query speist Netz (Display/STL) **und**
  STEP-Adapter — kein Formel-Duplikat, keine stille Drift ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-2). Der Refaktor ist byte-identisch
  (per `git show HEAD` + bestehende Orakel belegt).
- **Exakte `CLOSED_SHELL`-Anzahl als Auslassungs-Beweis:** `== step_count` (nicht `≥`) belegt **dual**,
  dass alle Stufen exportieren **und** das Geländer fehlt — ein Auslassungs-Vertrag wird so positiv
  getestet, nicht nur behauptet.

**Restrisiko / Nachfolge:** Die STEP-B-Rep-Lücke ist **vollständig geschlossen** (alle 3D-Bauteile B-Rep).
welle-4 offen nur noch für **PDF/PNG** ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) + die
**Welle-4-Verifikation** → `done/welle-4-results.md` → Meilenstein M4. `makeBoxSolid`-Guard-Coverage
([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-LOW-2) als benannter, upstream-gefilterter (unerreichbarer) Mini-Punkt.
