---
id: slice-024a
titel: Dächer STEP-B-Rep — wasserdichtes Netz → vernähtes Solid (meshToSolid)
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 024a: Dächer STEP-B-Rep

**Status:** done (2026-06-19). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review (gemeinsam mit 024b; HIGH-1 vor
Start behoben) **+** unabhängiges [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Geometrie-Code-Review **0 HIGH**
([Report](../../../reviews/2026-06-19-slice-024a-code-review.md); 2 MED/2 LOW eingearbeitet — der Reviewer
hat alle 3 Dachtypen + Zeltdach-Apex empirisch als wasserdicht/volumen-exakt belegt). DoD vollständig,
`make gates` grün (206/206). Closure-Notiz §8.

**Welle:** welle-4-austausch. **Split-Hälfte** der STEP-B-Rep-Schließung (Projektinhaber-Entscheidung
2026-06-19, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Empfehlung): **024a = Dächer** (Vernähung), **024b = Treppen** (analytische Box-Solids).
Schließt **die Dach-Hälfte** der in [slice-020b](../done-archive/slice-020b-step-stl-export.md) benannten
STEP-Lücke: STEP exportiert bisher nur die OCC-Solid-Bauteile (Wände + Decken); Dächer waren STL-only.
Seit [slice-023b](../done-archive/slice-023b-dach-volumen-impl.md) sind Dächer **geschlossene, wasserdichte
Volumenkörper** (und seit [slice-023c](../done-archive/slice-023c-dach-thickness-persistenz.md) ihre Dicke
persistent) — die Voraussetzung, sie als B-Rep zu vernähen.

**Bezug:** [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) (STEP-Export — B-Rep aller 3D-Bauteile),
`spec/spezifikation.md` §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005) (020a/020b-Lücke „Dächer/Treppen
STL-only" — 024a schärft die **Dach**-Hälfte; die Treppen-Lücke bleibt benannt bis 024b),
[LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006) (Dach-Volumenkörper, 023b). **Parametrisiert auf
[ADR-0014](../../adr/0014-step-stl-export-backend.md)** (OCC-DataExchange nativ, geometrie-residenter `ModelExporterPort`),
[ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (OCC nur im Geometrie-Adapter, Regel C),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt, neutraler Wurf). **Kein neuer ADR** (§6 — selbe
Backend-Entscheidung wie 020b; die Vernähung war dort bereits als Folge-Inkrement benannt).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-19.

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start), Reviewer ≠ Autor):** Das Review lief auf dem
**Unified-Plan** (Dächer + Treppen) — [Report](../../../reviews/2026-06-19-slice-024-plan.md), **1 HIGH / 2 MED /
3 LOW / 3 INFO**, Quellen-Konsistenz durchgehend grün. **Auf 024a entfallen: HIGH-1** (OCC-freier
Wasserdichtheits-Sensor — der `BRepCheck_Analyzer`/`BRepGProp`-Ansatz hatte keinen realen Ort in der bewusst
OCC-freien Test-Schicht → **behoben:** `meshToSolid` **fail-closed im Adapter** + OCC-freies
`MANIFOLD_SOLID_BREP`/`CLOSED_SHELL`-Text-Orakel; Volumen-Exaktheit bleibt 023b-Netz-Ebene) und **MED-1**
(`TKShHealing` für Sewing gepinnt). **LOW-1** (Split) ist mit dieser Datei umgesetzt. **HIGH behoben → Start
frei.**

---

## 1. Ziel

Der **STEP-Export** ([LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)) deckt jetzt auch **Dächer** als
B-Rep-Solids: der seit 023b **wasserdichte** `roofMesh` wird zu **einem** OCC-B-Rep-Solid **vernäht**
(`BRepBuilderAPI_Sewing` → Shell → `MakeSolid`), **fail-closed** geprüft. Alles **geometrie-resident** (OCC
nur in `src/adapters/geometry/`, Regel C), **atomar** ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)), Kern bleibt
format-frei. **STL unverändert** (deckte Dächer schon als Netz). **Treppen bleiben STL-only bis 024b**
(benannte Rest-Lücke).

## 2. Definition of Done

- [x] **DoD-1 — Dach-B-Rep via Vernähung, fail-closed (`occ_solids`-Helfer; HIGH-1).** Ein geteilter,
      geometrie-residenter Helfer `meshToSolid(const model::TriangleMesh&) → TopoDS_Shape` (in `occ_solids`,
      Regel C) vernäht ein **wasserdichtes** Dreiecksnetz: je Dreieck eine `TopoDS_Face` (Polygon-Wire) →
      `BRepBuilderAPI_Sewing` (Toleranz ≥ µm-Raster der 023b-Kanten, ≤ `kGeometryToleranceMm`=0,1 mm) →
      `TopoDS_Shell` → `BRepBuilderAPI_MakeSolid`. **Fail-closed (HIGH-1):** der Helfer prüft das Ergebnis mit
      **`BRepCheck_Analyzer.IsValid()`** (geschlossene, gültige Shell); **ungültig/offen → übersprungen**
      (kein nicht-geschlossenes Solid erreicht je die Datei — auch ein künftig nicht-wasserdichtes Netz kann
      kein Schrott-Solid emittieren). Der Validitäts-Check lebt **im Adapter** (OCC erlaubt in `geometry/`),
      nicht im Test.
- [x] **DoD-2 — `buildSolidCompound` ergänzt die Dach-Solids.** Im `StepExportAdapter` fügt
      `buildSolidCompound` je Dach `meshToSolid(roofMesh(r))` dem Compound hinzu (neben den bestehenden
      Wand-/Decken-Solids). **Totalität:** leeres/degeneriertes Netz (023b: Dicke ≤ 0 / degenerierter
      Grundriss → leer) → **übersprungen** (kein Wurf), wie heute Wände/Decken. **`TKShHealing`** in
      `src/adapters/CMakeLists.txt` linken (MED-1; `MakeSolid`/`BRepCheck` in bereits gelinkten
      TKTopAlgo) — **keine** neue `find_package`-Dependency (gleiche Distribution, [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) unberührt).
- [x] **DoD-3 — Spec §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005): Dach-Hälfte auf B-Rep geschärft
      (kein Lastenheft).** Das committete §1 trägt die 020b-Lücke „Dächer/Treppen STL-only". 024a schärft die
      **Dach**-Hälfte (technisch, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei): Spec, **nicht** Lastenheft): **Dächer** = vernähtes
      B-Rep-Solid aus dem wasserdichten 023b-Netz. Die **Treppen**-Lücke bleibt **explizit benannt** (STL-only
      bis 024b — kein stiller Teilumfang). `spezifikation-historie.md` nachgezogen. **Lastenheft unberührt**.
- [x] **DoD-4 — AK-Tests + OCC-freie Wasserdichtheits-Invariante ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Orakel; HIGH-1/LOW-3).**
      Die STEP-Fixture in `tests/adapters/test_step_stl_export.cpp` bekommt **mindestens ein Dach** — **OCC-frei**
      (Text-Re-Read, wie die 020b-Tests, Regel C). **Sensor (nicht „nicht-leer", 016b/020b-Lehre):** das
      ISO-10303-21-Text-Orakel assertiert, dass das Dach als **`MANIFOLD_SOLID_BREP` mit `CLOSED_SHELL`**
      geschrieben ist (geschlossene Shell = topologische Wasserdichtheit; eine offene Vernähung schriebe
      `OPEN_SHELL`/`SHELL_BASED_SURFACE_MODEL` **oder** würde durch den Adapter-fail-closed [DoD-1] gar nicht
      geschrieben). Die exakte **`bx·ty·d`-Volumengleichheit** bleibt auf **Netz-Ebene** bewiesen (023b,
      `test_roof_geometry`). **Boundary/Regression (LOW-3):** `EmptyBuildingYieldsValidStepEnvelope` bleibt grün;
      ein Modell **mit Wänden, ohne Dach** assertiert **keinen** Solid-Zuwachs (`BuildingWithWallsYieldsBRepSolids`-
      Zahl unverändert). **STL-Regression** unverändert grün.
- [x] **DoD-5 — Schicht + Gates + Reviews + Closure.** `arch-check` **Regel C** deckt die OCC-Nutzung (kein
      neuer Regelbedarf, [ADR-0014](../../adr/0014-step-stl-export-backend.md)); OCC-Typen lecken **nicht** über `ModelExporterPort`
      (neutraler Wurf). `make gates` grün (Invarianten-Tests, lint 0, coverage, docs-check 0). **Unabhängiges
      [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review 0 HIGH** (Wasserdichtheit/Gültigkeit des vernähten Dach-Solids; OCC-Typ-
      Kapselung). `CHANGELOG.md` + `roadmap.md`; Closure-Notiz; Plan nach `done-archive/` (reiner `git mv`,
      [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)). **Dach-Hälfte der STEP-Lücke geschlossen** → 024b (Treppen) wird startbar.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/geometry/occ_solids.{h,cpp}` | ändern | `meshToSolid(TriangleMesh)` (Vernähung wasserdichtes Netz → Solid, **fail-closed** via `BRepCheck_Analyzer`) |
| `src/adapters/geometry/step_export_adapter.cpp` | ändern | `buildSolidCompound`: Dach-Solids (`meshToSolid(roofMesh(r))`) ergänzen |
| `src/adapters/CMakeLists.txt` | ändern | `TKShHealing` für `BRepBuilderAPI_Sewing` linken (MED-1) — keine neue Dependency |
| `spec/spezifikation.md` (+ `spezifikation-historie.md`) | ändern | §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005): Dächer B-Rep; Treppen-Lücke bleibt benannt ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) |
| `tests/adapters/test_step_stl_export.cpp` | ändern | Fixture um Dach; OCC-frei `MANIFOLD_SOLID_BREP`/`CLOSED_SHELL` + Kein-Zuwachs-Regression |
| `CHANGELOG.md` · `roadmap.md` | ändern (Closure) | 024a done; Dach-Hälfte der STEP-Lücke geschlossen |
| `docs/reviews/{…-slice-024a-code-review}.md` | neu (Closure) | [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report |

**Kein** Kern-Code (Export geometrie-resident; `ExchangeService`/`ModelExporterPort`/`ExchangeFormat::Step`
existieren seit 020b) → `arch-check` Regel A unberührt. **Kein** neuer ADR.

## 4. Trigger

- **Startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: STEP-Backend ([ADR-0014](../../adr/0014-step-stl-export-backend.md), 020b) + wasserdichtes
  Dach-Netz (023b) + persistente Dicke (023c) liegen vor; `buildSolidCompound`/`occ_solids` existieren.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH**, Closure-Notiz → 024b (Treppen) startbar
  (`meshToSolid`/`buildSolidCompound`-Infrastruktur steht).

## 6. Risiken und offene Punkte

- **Dach-Vernähung-Toleranz + fail-closed Sensor (HIGH-1):** `roofMesh` ist **flat-shaded ohne geteilte
  Vertices** (µm-kanonische Kanten, 023b). `BRepBuilderAPI_Sewing` muss mit Toleranz ≥ µm-Raster arbeiten →
  **eine** Shell → Solid. Zu klein → offene Shell (kein Solid); zu groß verschmilzt echte Kanten. **Sensor:**
  `meshToSolid` prüft `BRepCheck_Analyzer.IsValid()` und **überspringt** ungültig (nie ein Schrott-Solid in der
  Datei); der Test assertiert OCC-frei das geschriebene `CLOSED_SHELL`/`MANIFOLD_SOLID_BREP`. Die Volumen-
  Exaktheit (`bx·ty·d`) bleibt auf Netz-Ebene bewiesen (023b) — **nicht** via OCC-`BRepGProp` im Test (das
  passierte auch für offene Shells, HIGH-1).
- **OCC-Typ-Leck (Regel A/C):** der Adapter nutzt OCC direkt (erlaubt in `geometry/`), darf **keinen** OCC-Typ
  über `ModelExporterPort` lecken (neutraler Wurf, Muster `OccGeometryAdapter` / 020b).
- **Kein neuer ADR (begründet):** selbe Backend-Entscheidung wie [ADR-0014](../../adr/0014-step-stl-export-backend.md); die Vernähung war
  im 020b-Plan als Folge-Inkrement unter [ADR-0014](../../adr/0014-step-stl-export-backend.md)/[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) benannt. Vom [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) bestätigt (INFO-2).
- **Benannte Rest-Lücke:** Treppen bleiben bis 024b STL-only — in §1 explizit benannt (kein stiller
  Teilumfang).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Geometrie-Kern-Adapter (`src/adapters/geometry/`)

- **Modus:** GF; **Konventionen-Dichte:** hoch — OCC gekapselt (Regel C), neutraler Wurf, geteilter
  `occ_solids`-Builder, Atomarik aus 020b. **Phase-Reife:** Phase 4. **Evidenz-/Diskrepanz-Risiko:**
  **mittel** (Vernähung = neue Geometrie) → [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) + Gültigkeits-Invariante. **Reconciliation:**
  schließt die Dach-Hälfte der 020b-STEP-Lücke.

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; **Dichte:** hoch (§1 technisch, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-treu — Lastenheft unberührt).
  **Phase-Reife:** Phase 4. **Risiko:** niedrig.

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; **Dichte:** hoch (OCC-freies Text-Re-Read-Orakel + topologische Wasserdichtheits-Assertion
  statt „nicht leer", 016b/020b-Lehre). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-19):**

- **Vernähung (DoD-1/2):** neuer geometrie-residenter Helfer `meshToSolid` (`occ_solids`) vernäht das
  wasserdichte `roofMesh` (023b) via `BRepBuilderAPI_Sewing` (Toleranz `kGeometryToleranceMm`) →
  **genau eine** geschlossene `TopoDS_Shell` → `BRepBuilderAPI_MakeSolid`; **fail-closed** via
  `BRepCheck_Analyzer.IsValid()` (offen/ungültig → leeres Shape, übersprungen). `buildSolidCompound`
  ergänzt je Dach das Solid (Per-Bauteil-`try/catch` wie Wände/Decken, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) MED-2). `TKShHealing`
  gelinkt (gleiche OCC-Distribution, **keine** neue Dependency, [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) unberührt).
- **Spec (DoD-3):** §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005) geschärft (Dächer B-Rep via Vernähung;
  STL/STEP-Divergenz eines fail-closed-Dachs benannt, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) LOW-1); **Treppen-Lücke bleibt explizit
  benannt** bis 024b. `spezifikation-historie.md` nachgezogen. **Lastenheft unberührt** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).
- **Test (DoD-4):** `RoofYieldsClosedShellBRepSolid` **OCC-frei** — über **{Sattel, Walm, Pult,
  Walm-Zeltdach-Apex}** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) LOW-2) je `MANIFOLD_SOLID_BREP` + **genau eine zusätzliche**
  `CLOSED_SHELL` ggü. dem Wand-only-Modell (topologische Wasserdichtheit + Kein-Zuwachs-Regression).
  Volumen-Exaktheit `bx·ty·d` bleibt auf Netz-Ebene bewiesen (023b).
- **Gates (DoD-5):** `make gates` grün (**206/206**, arch-check Regel C — OCC im Geometrie-Adapter,
  lint 0, docs-check 0/141); kein OCC-Typ-Leck über `ModelExporterPort`.
- **Reviews:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) HIGH-1 vor Start behoben + **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH**
  ([Report](../../../reviews/2026-06-19-slice-024a-code-review.md)): der Reviewer hat über echte OCC-Proben
  alle 3 Typen + Zeltdach-Apex als **wasserdicht + volumen-exakt** (`bx·ty·d`) belegt; 2 MED (Vertrags-
  Wortlaut Orientierung geerbt; `try/catch`-Parität) + 2 LOW (§1-Divergenz; Test-Breite) eingearbeitet.

**Lerneintrag:**

- **Mesh→Shape-Vernähung für wasserdichte Netze ist tragfähig:** ein topologisch wasserdichtes
  Flat-Shading-Soup (ohne geteilte Vertices) vernäht `BRepBuilderAPI_Sewing` über geometrische
  Nähe zu **einer** geschlossenen Shell (Toleranz ≥ µm-Raster, ≤ `kGeometryToleranceMm`); `MakeSolid`
  + `BRepCheck_Analyzer` als fail-closed Wächter. Die Roadmap-Verkürzung „Mesh→Shape-Vernähung" gilt
  damit für **Dächer** — **nicht** für Treppen (nicht-manifolde Box-Union → 024b analytisch).
- **OCC-freier topologischer Sensor:** `CLOSED_SHELL`-**Anzahl** (statt „BREP vorhanden") prüft
  Wasserdichtheit ohne OCC im Test (Regel C) — eine offene Vernähung erzeugt keine zusätzliche
  `CLOSED_SHELL` (fail-closed) → der Zähler fällt, der Test schlägt fehl.
- **`BRepCheck.IsValid()` ≠ Orientierungs-Prüfung ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) MED-1):** es fängt Offenheit, **nicht**
  Inversion — die Außennormalen-Korrektheit ist vom Eingabe-Netz **geerbt** (023b). Der geteilte
  Helfer-Vertrag wurde entsprechend ehrlich formuliert (wichtig für die 024b-Wiederverwendung).
- **Review-Agent-Hygiene:** der [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Agent setzte beim empirischen Proben-Lauf die **unkommittierten**
  Test-Änderungen zurück — Lehre: vor einem Code-Review-Agenten mit Schreibzugriff committen. Fixture/
  Test wurden neu (in der parametrierten Form) eingespielt und verifiziert.

**Restrisiko / Nachfolge:** **024b** (Treppen STEP-B-Rep — analytische Box-Solids, Geländer ausgelassen;
nutzt `meshToSolid` **nicht**, sondern `makeBoxSolid`). Danach sind **alle** 3D-Bauteile B-Rep → welle-4
offen nur noch für **PDF/PNG** ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) + Welle-4-Verifikation → M4.
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-MED-1 (Volumen-Sign-Guard in `meshToSolid`) als benannter, nicht erreichbarer Mini-Cleanup.
