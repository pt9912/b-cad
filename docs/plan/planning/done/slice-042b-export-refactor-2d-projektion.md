---
id: slice-042b
titel: Export-Refactor 2D-Projektion — plan_geometry in den Kern + PlanViewPort (entsperrt Canvas)
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-003](../../../../spec/lastenheft.md#3-projektziele), [OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0001](../../adr/0001-hexagonale-architektur.md)]
---

# Slice 042b: Export-Refactor 2D-Projektion — Kern-Hebung + `PlanViewPort`

**Status:** done (2026-07-23 — Impl geliefert, `make gates` grün [249 Tests, Coverage 90,7 %], 2D-Export-Orakel unverändert grün = Invarianz-Beweis). **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)** 0 HIGH / 0 MED / 2 LOW ([Plan-Report](../../../reviews/2026-07-23-slice-042b-plan.md)) + **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) Code-Review** (042a+042b) 0 HIGH ([Report](../../../reviews/2026-07-23-slice-042a-042b-code-review.md)).

**Welle:** welle-5-erweiterung. **Zweiter** der fünf [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices
(Familie 042a…e). **Vorgänger:** slice-042a (Kern-Naht, done). **Deckt zugleich [ADR-0019](../../adr/0019-drw-2d-canvas.md)s
Lese-Naht-Refactor** (dieselbe Hebung) — **entsperrt den DRW-Canvas** (der Canvas zieht später `PlanViewPort`).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (2D-Projektion
kern-geliefert im `DerivedGeometry`-Bündel; **kein** Adapter leitet 2D-Geometrie ab; `visibleLayerIds` bleibt
io-lokal/geteilt) + [ADR-0019](../../adr/0019-drw-2d-canvas.md) (2D-Lese-Naht = Kern-Projektion + `PlanViewPort`,
2D-Analog zum `ViewModelPort`; eine Quelle für Bildschirm **und** Export), [ADR-0001](../../adr/0001-hexagonale-architektur.md)
(Kern-Reinheit — die gehobene Projektion zieht keine Library).

---

## 1. Ziel

Die reine 2D-Grundriss-Projektion (heute io-resident: `projectPlan(Building) → PlanView` in
`adapters/io/plan_geometry`) in den framework-freien **Kern** heben und über einen neuen Driving-Read-Port
`PlanViewPort` lesbar machen — das 2D-Analog zum `ViewModelPort`. Die 2D-Exporter **PDF/PNG** beziehen die
`PlanView` fortan vom Kern **im `DerivedGeometry`-Bündel** (der `ExchangeService` berechnet sie format-selektiv),
statt sie selbst zu projizieren; **DXF** bleibt unberührt (iteriert direkt + `visibleLayerIds`). **Eine Quelle**
(Kern) für Bildschirm (`PlanViewPort`, Canvas später) und Export (Bündel). **Rein mechanischer, verhaltens-
invarianter Umzug** — die 2D-Export-Decode-Orakel bleiben byte-/struktur-identisch grün.

**Kanten-Kernpunkt:** würde PDF/PNG die gehobene Projektion **direkt** rufen, entstünde die verbotene
`io → services_geo`-Kante (`.a-check.yml` führt für `io` nur `→ model` / `→ ports_driven`). Der **Bündel-Weg**
(`io → ports_driven` über `ModelExporterPort`) vermeidet sie; die Projektion überquert die Naht nur als
`model::PlanView` im Bündel, das der `ExchangeService` (`services`, darf `services_geo`) befüllt.

## 2. Definition of Done

- [x] **`PlanView`-Werttypen → `model/`.** `PlanSegment`/`StoreyPlan`/`PlanView` (heute `bcad::adapters::io`,
      `plan_geometry.h`) → neuer header-only `src/hexagon/model/{plan_view}.h` (Namespace `bcad::hexagon::model`,
      reine Structs — Muster der übrigen `model/`-Werttypen). Nötig, damit `DerivedGeometry` (ein `model/`-Typ)
      die `PlanView` referenzieren darf.
- [x] **`projectPlan` → Kern.** Die Projektions-Funktion (`plan_geometry.cpp` `projectPlan` + Helfer `accumulate`)
      → neuer `src/hexagon/services/geometry/{plan_projection}.{h,cpp}` (`bcad::hexagon::services`,
      `PlanView projectPlan(const model::Building&)`; Muster `wall_footprint`). Registrieren in
      `src/hexagon/CMakeLists.txt`. **Reiner Umzug** (Logik unverändert). a-check: `services_geo → model` deckt es
      — **keine** neue Kante.
- [x] **`visibleLayerIds` → `model/` (geteilt, kein Duplikat).** Der reine Ebenen-Sichtbarkeits-**Filter** (kein
      Geometrie-Wert) → header-only `src/hexagon/model/{layer_visibility}.h` (`visibleLayerIds(const Building&) →
      set<int>`), damit **beide** ihn ziehen dürfen: Kern-`projectPlan` (`services_geo → model`) **und** io-**DXF**
      (`io → model`). **Empfehlung** gegen die Alternativen (io-lokal + Kern-Duplikat / in `services_geo` [dann
      dürfte DXF ihn nicht mehr rufen → `io → services_geo` verboten]); das Review entscheidet. **DXF bleibt
      sonst unberührt.**
- [x] **`adapters/io/plan_geometry` reduziert.** Nur noch `visibleLayerIds`-Nutzung (jetzt aus `model/`);
      `projectPlan`/`PlanView`-Typen daraus entfernt. Falls danach leer → Datei entfernen + DXF-Include auf
      `model/{layer_visibility}.h` umziehen.
- [x] **`PlanViewPort` (Driving-Read-Port).** Neuer `src/hexagon/ports/driving/{plan_view_port}.h` (abstrakte
      const-Query, nur `model/`-Includes; Muster `ViewModelPort`/`EvaluatePort`) — liefert die `PlanView` (bzw.
      je Geschoss). Vom `StructureEditService` implementiert (`building_` vorhanden; ruft `services::projectPlan`).
      **Composition-Root (`main.cpp`) noch NICHT an einen Konsumenten verdrahtet** — der Canvas (eigener
      [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Impl-Slice) zieht ihn später über eine `ui/command/`-Source
      (Muster `ViewModelMeshSource`). Ein kleiner Kern-Test belegt Port → `projectPlan`.
- [x] **`DerivedGeometry` um die `PlanView`.** `src/hexagon/model/derived_geometry.h` bekommt ein `PlanView`-Feld
      (format-selektiv befüllt; leer für Nicht-2D-Formate).
- [x] **`ExchangeService` berechnet die `PlanView` (Pdf/Png).** In `exportModel` (nach der 042a-Stelle) für
      `ExchangeFormat::Pdf`/`Png` `derived.plan = projectPlan(building)` setzen (per-Format-Selektion; STEP/STL
      bleiben leer → 042c; IFC/DXF leer). `services → services_geo` deckt den Aufruf. **Anders als der 042a-MED-1-
      Fall wird die Berechnung hier KONSUMIERT und von den PDF/PNG-Decode-Orakeln VERIFIZIERT** — kein
      un-genetzter Prüf-Verlust; `projectPlan` ist total (kein fail-closed-Wurf-Risiko wie STEP/STL).
- [x] **PDF/PNG konsumieren das Bündel.** `pdf_export_adapter.cpp`/`png_export_adapter.cpp`: statt
      `projectPlan(building)` das `derived.plan` lesen (Parameter `derived` aktivieren). **Byte-identische Ausgabe.**
- [x] **Verhaltens-Invarianz maschinell.** `make gates` grün; die 2D-Export-Decode-Orakel (`test_plan_geometry`
      [→ ggf. `test_plan_projection` umbenannt/Namespace], `test_pdf_export` `" l\n"`-Zählung, `test_png_export`
      Tinte, `test_dxf_export` roher Reader) bleiben **unverändert grün**. Tests der gehobenen Typen/Funktion auf
      die neuen Namespaces/Pfade nachziehen; PDF/PNG-Tests, die `write` **direkt** rufen, übergeben ein **befülltes**
      `DerivedGeometry` (oder gehen über den `ExchangeService`) — sonst zeichnen sie leer. `make schema-check`
      unberührt.
- [x] **Doku.** `spec/spezifikation.md` §1 (2D-Export-Datenfluss-Umfeld) um die Kern-Projektion
      + `PlanViewPort`-Lese-Naht (token-frei, [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)
      vor Gate greppen); `spezifikation-historie` + Header. **architecture** §1.1 hatte `PlanViewPort` schon in
      slice-041a eingetragen — mit diesem Slice **real**; **kein architecture-Edit nötig** (die §1.1-Prosa ist
      wahr, sobald der Port existiert; die §2-io-Zeile führt `services/geometry` schon nicht, bleibt **unberührt**
      → zieht 042e). architecture steht daher **nicht** in der §3-Tabelle ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-INFO-4). [ADR-Index](../../adr/README.md)
      2D-Projektions-Zeile „erfüllt"; **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
      Closure-Notiz. **[MR-020](../../../../harness/conventions.md) Closure-Disziplin:** vor Closure existiert
      slice-042c (Skelett) → erfüllt.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{plan_view}.h` | neu | `PlanView`/`StoreyPlan`/`PlanSegment` nach `model/` (Bündel-referenzierbar) |
| `src/hexagon/model/{layer_visibility}.h` | neu | `visibleLayerIds` geteilt (Kern + io-DXF), kein Duplikat/keine `io→services_geo` |
| `src/hexagon/services/geometry/{plan_projection}.{h,cpp}` | neu | `projectPlan` in den Kern (reiner Umzug) |
| `src/hexagon/ports/driving/{plan_view_port}.h` | neu | 2D-Lese-Naht (Muster `ViewModelPort`) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | `PlanViewPort` implementieren (ruft `projectPlan(building_)`) |
| `src/hexagon/model/derived_geometry.h` | ändern | `PlanView`-Feld ergänzen |
| `src/hexagon/services/exchange_service.cpp` | ändern | `PlanView` für Pdf/Png format-selektiv befüllen |
| `src/adapters/io/plan_geometry.{h,cpp}` | ändern/entfernen | `projectPlan`/`PlanView` raus; nur `visibleLayerIds`-Nutzung (aus `model/`) |
| `src/adapters/io/{pdf,png}_export_adapter.cpp` | ändern | `derived.plan` lesen statt `projectPlan` rufen |
| `src/adapters/io/dxf_export_adapter.cpp` | ändern (minimal) | `visibleLayerIds`-Include auf `model/` |
| `src/hexagon/CMakeLists.txt` | ändern | `plan_projection.cpp` registrieren |
| `src/adapters/CMakeLists.txt` | ändern | `io/plan_geometry.cpp` aus der io-Quell-Liste, **falls** die Datei leer/entfernt wird (sonst `make build` bricht) — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-1 |
| `tests/CMakeLists.txt` | ändern | den Projektions-Test von `adapters/` nach `hexagon/` retargeten (LOW-1) |
| `tests/adapters/test_plan_geometry.cpp` (→ ggf. `tests/hexagon/{test_plan_projection}.cpp`) | ändern/verschieben | gehobene Typen/Funktion (Kern-Test) |
| `tests/adapters/{pdf,png,dxf}_export`-Tests | ändern | befülltes `DerivedGeometry` / Namespace; Orakel-**Inhalt** unverändert |
| `spec/spezifikation.md` + `-historie.md`, [ADR-Index](../../adr/README.md), `CHANGELOG.md` | ändern | Doku-Nachzug (token-frei) |

**Bewusst NICHT Teil:** das **Canvas-Widget** selbst + die `ui/command/`-PlanView-Source (eigener
[ADR-0019](../../adr/0019-drw-2d-canvas.md)-Canvas-Impl-Slice, **nach** dieser Familie); STEP/STL (042c);
Persistenz (042d); `.a-check.yml`/architecture-§2-Abschluss (042e).

## 4. Trigger

- **slice-042a done** (`DerivedGeometry` + Port-Vertrag vorhanden). [ADR-0019](../../adr/0019-drw-2d-canvas.md)/[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) Accepted.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  Geometrie-berührend → Code-Review vor Welle-Closure ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (**2D-Export-Orakel unverändert grün** = Invarianz-Beweis + `PlanViewPort`-
  Test), Closure-Notiz → **slice-042c (STEP/STL)** startbar; der DRW-Canvas ist danach architektonisch entsperrt.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — `visibleLayerIds`-Heimat (Design-Prüfstein fürs Review):** Empfehlung `model/` (geteilt, kein
  Duplikat, keine `io → services_geo`-Kante). Alternativen: (a) io-lokal + trivialer Kern-Reimpl (Duplikat einer
  3-Zeilen-Schleife), (b) nach `services_geo` (dann darf DXF ihn **nicht** mehr rufen → `io → services_geo`
  verboten → schiede aus). Das Review bestätigt die Heimat.
- **Rest-Risiko #2 — Test-Migration PDF/PNG ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-2):**
  die Direkt-`write`-Tests übergeben heute `model::DerivedGeometry{}` (leer) — nach der Umstellung zeichnen
  PDF/PNG daraus **leer** (harter Fail): u. a. PDF `ScaleFidelityKnownEdge`, `PreservesAxisOrientationNoYFlip`,
  `LH_FA_DRW_005_SichtbareHilfslinieErscheint`/`…UnsichtbareEbeneKeineHilfslinie`, PNG
  `LH_FA_DRW_005_SichtbareHilfslinieMehrTinte`; **Intent-Verlust ohne Bruch:** PNG `DegenerateBBoxNoDivByZero`
  (prüfte sonst die Fit-Guard gegen ein leeres Bündel). **Mitigation (byte-identitäts-erhaltend):** die Tests
  über den `ExchangeService` führen (der befüllt real) **oder** `derived.plan = services::projectPlan(b)` für
  **dasselbe** Building setzen — **nicht** eine hand-autorisierte `PlanView` (die verifizierte den Adapter sonst
  gegen eine Test-View statt gegen die Kern-Rechnung; die Invarianz-Aussage verwässert still). Die Orakel-
  **Erwartungen** (Linien-/Tinte-Zählung) bleiben unverändert.
- **Rest-Risiko #3 — `ExchangeService`-Berechnung (ANDERS als 042a-MED-1):** hier wird die `PlanView` **konsumiert**
  (PDF/PNG) **und** von den Decode-Orakeln **verifiziert** → die Berechnung bringt Prüfgewinn, keine un-genetzte
  Wurf-Fläche; `projectPlan` ist **total** (kein fail-closed-Muster wie STEP/STL). Kein Deferral nötig.
- **`test_plan_geometry` Namespace/Ort:** die Tests nutzen `bcad::adapters::io`-`projectPlan`/`PlanView` — nach der
  Hebung Namespace + ggf. Verzeichnis (→ `tests/hexagon/`) nachziehen; Orakel-Logik unverändert.
- **Kern-Reinheit:** `model::PlanView`/`plan_projection`/`layer_visibility` ziehen **keine** Library
  (`bcad_hexagon` dep-frei, [ADR-0001](../../adr/0001-hexagonale-architektur.md)); Prüfung `make build` + `make a-check`.
- **Spec-Straten token-frei** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)):
  §1/§Historie ohne ADR-/Slice-Token im Körper — vor dem Gate greppen (Lerneintrag 032/041a/042a).
- **Scope-Größe:** breit (Typ-/Funktions-Hebung + neuer Port + Bündel-Feld + 2 Exporter + `visibleLayerIds`-Umzug
  + Tests), aber kohärent und überwiegend mechanisch; die Invarianz-Orakel sind das Netz. Das Review bewertet, ob
  ein Split (z. B. Hebung getrennt von der PDF/PNG-Bündel-Migration) nötig ist.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Kern + Adapter-Code `src/`

- **Modus:** GF; **Dichte:** hoch (Typ-/Funktions-Umzug mit Verwender-Nachzug, neuer Read-Port, Bündel-Feld,
  `ExchangeService`-Berechnung, 2 Exporter-Bodies). **Phase-Reife:** welle-5 Refactor. **Risiko:** mittel — breiter,
  aber verhaltens-invarianter Umzug; die 2D-Export-Decode-Orakel sind das Netz.

### Sub-Area: Spec/Doku `spec/` + `docs/plan/`

- **Modus:** GF; **Dichte:** mittel (2D-Datenfluss + `PlanViewPort` token-frei; ADR-Index + CHANGELOG). **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure 2026-07-23.** `make gates` grün (249/249 Tests, Coverage 90,7 %); die 2D-Export-Decode-Orakel
(DXF/PDF/PNG Erscheint/Fehlt) **unverändert grün** = Invarianz-Beweis. `make schema-check` unberührt.

- **Gehobene Typen/Funktion (verhaltens-invariant, Byte-Identität im [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  Zeile-für-Zeile verifiziert):** `PlanView`/`StoreyPlan`/`PlanSegment` → `model/plan_view.h`; `projectPlan` →
  `services/geometry/plan_projection.{h,cpp}`; `visibleLayerIds` → `model/layer_visibility.h`. Das io-`plan_geometry`
  **entfernt**.
- **`visibleLayerIds`-Heimat = `model/`** (Rest-Risiko-#1-Entscheidung): geteilt von Kern-`projectPlan`
  (`services_geo → model`) **und** io-DXF (`io → model`) — **kein Duplikat, keine `io → services_geo`-Kante**.
- **Naht:** neuer `PlanViewPort` (`ports/driving`), vom `StructureEditService` implementiert
  (`planView() → projectPlan(building_)`; 6. Driving-Port) — **DRW-Canvas architektonisch entsperrt** (Konsument =
  späterer Canvas-Impl-Slice; `main.cpp` noch nicht verdrahtet). `DerivedGeometry.plan`-Feld; der `ExchangeService`
  befüllt die `PlanView` **format-selektiv** für PDF/PNG; PDF/PNG lesen `derived.plan` (Writer serialisieren nur,
  `building` folgenlos ungenutzt). **DXF unberührt.**
- **`ExchangeService`-Berechnung ohne 042a-MED-1-Wiederholung:** `projectPlan` ist **total** (kein Wurf) und die
  `PlanView` wird **konsumiert + von den PDF/PNG-Decode-Orakeln verifiziert** → Prüfgewinn, keine un-genetzte
  Wurf-Fläche.
- **Immutabilitäts-Konflikt (Lerneintrag):** die Accepted/immutable **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)** (+ done-slice-032c) referenzieren
  das entfernte io-`plan_geometry` historisch. **Kein** Edit am immutablen ADR-Core (§2.5) — stattdessen
  `codepaths.ignore-refs`-**Tombstone** in `.d-check.yml` (Präzedenz `tools/Dockerfile`; keine Schwellen-Lockerung,
  [§2.6](../../../../AGENTS.md) n/a). Der [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Reviewer
  bestätigte: keine funktionale Code-Referenz wird verdeckt.
- **Doku:** Spec §1 PDF/PNG-„Encoding & Schicht" um die 2D-Projektions-Bereitstellung + die DRW-Canvas-2D-Lese-Naht
  „inzwischen realisiert" (token-frei) + `spezifikation-historie`; CHANGELOG; ADR-Index 2D-Projektions-
  Zeile „erfüllt". architecture §1.1 trug `PlanViewPort` bereits (slice-041a) → jetzt real, **kein Edit** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-INFO-4);
  §2-Tabelle unberührt (zieht 042e).
- **Reviews:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  0 HIGH / 2 LOW (CMake-Gegenstücke + Test aus echter `projectPlan`-Quelle) + [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  (042a+042b) 0 HIGH / 1 LOW (Kommentar-Drift, behoben) / 1 INFO (Negativ-Test → 042c).
- **Folge:** **slice-042c** (STEP/STL-Body auf das Bündel + `ExchangeService`-Berechnung [von 042a, MED-1] +
  `geometry → services_geo`-Kante raus; Skelett in `open/`, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  beim Start) — [MR-020](../../../../harness/conventions.md) Closure-Disziplin erfüllt (042c existiert).
