---
id: slice-017f
titel: Auswertung Listen — EVL-004/005/006 (Material-/Tür-/Fensterlisten, Aggregation)
status: in-progress
welle: welle-3-auswertung
lastenheft_refs: [[LH-FA-EVL-004](../../../../spec/lastenheft.md#lh-fa-evl-004--materiallisten), [LH-FA-EVL-005](../../../../spec/lastenheft.md#lh-fa-evl-005--türlisten), [LH-FA-EVL-006](../../../../spec/lastenheft.md#lh-fa-evl-006--fensterlisten)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0012](../../adr/0012-evaluations-architektur.md)]
---

# Slice 017f: Auswertung Listen — EVL-004/005/006

**Status:** in-progress. **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen** (unabhängig, ohne
Autoren-Kontext): **0 HIGH**, 1 LOW (§1-Edit zweiteilig: Menge=Volumen + `roofs`
in die Lücke) eingearbeitet ([Report](../../../reviews/2026-06-16-slice-017f-plan.md))
→ **startbar**. **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** (keine neue Geometrie — reine Aggregation, wie
017b; das Volumen-Reuse ist bereits in 017c code-reviewt).

**Welle:** welle-3-auswertung (sechster Slice; **schließt das EVL-Modul** —
letzte Auswertungs-Listen).

**Bezug:** [LH-FA-EVL-004](../../../../spec/lastenheft.md#lh-fa-evl-004--materiallisten) (Materiallisten) / EVL-005 (Türlisten) / EVL-006
(Fensterlisten), spez. §1 [`LH-FA-EVL-001.a`](../../../../spec/lastenheft.md#lh-fa-evl-001--flächenberechnung) (Listen = Aggregation über das
Modell). Baut auf dem **durablen Material-Strang** (017d `effectiveMaterial` +
017e Persistenz) und auf **EVL-002** (017c `volume_geometry`). [ADR-0012](../../adr/0012-evaluations-architektur.md)
(Auswertung = read-only-Ableitung über `EvaluatePort`, pull, kein `op`), [ADR-0001](../../adr/0001-hexagonale-architektur.md)
(reine Kern-Query). **Kein neuer ADR.**

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-16.

**Schnitt-Herkunft:** Listen-Hälfte des Auswertungs-Strangs. **Scope
(Projektinhaber 2026-06-16):** **alle drei Listen** in einem Slice (Tür/Fenster
sind triviale Aggregationen — kein eigener 005-Slice). **EVL-004-Menge =
Volumen (m³)** je effektivem Material über **Wand + Decke/Fundament**.

**Bewusst NICHT Teil (benannte Lücken / Folge):**

- **Dach in EVL-004:** ein Dach ist material-tragend, sein **Volumen ist welle-3
  zurückgestellt** (017c, dicke-loses Modell) → **nicht** in die `quantity_m3`-
  Aggregation eingerechnet (benannte Lücke, konsistent mit der EVL-002-Dach-
  Entscheidung). Re-Eval mit Dach-Dicken-/Material-Volumen-Semantik.
- **Material an `stairs`/`openings`** (kein `material_id` im Schema) · **`windows.
  frame_material`** (Freitext, kein `materials`-FK) → fließen **nicht** in EVL-004
  (spez. §1, benannte Lücke).
- **Kosten-Auswertung** (MAT-006: Menge × `cost_per_m3`) — die Materialliste
  trägt das `Material` (mit Kennwerten) je Zeile, der Kosten-**Aufschlag** ist ein
  Folge-Schritt (nicht dieser Slice).

---

## 1. Ziel

Das EVL-Modul schließen: drei **read-only-Aggregationen** über das committete
Modell via `EvaluatePort` (pull, kein `op`, kein `GeometryKernelPort`):

- **EVL-004 Materialliste:** je **effektivem Material** (017d `effectiveMaterial`,
  Override) die **Menge** = Σ **Netto-Volumen** (m³, 017c `volume_geometry`) der
  zugeordneten **Wand + Decke/Fundament** + die Bauteil-Anzahl. Bauteile **ohne**
  Material werden nicht gruppiert (Boundary, kein Fehler). **Dach ausgenommen**
  (Volumen-Lücke).
- **EVL-005 Türliste:** die `openings` mit `kind == Door` mit ihren Maßen
  (Breite/Höhe) + Anzahl.
- **EVL-006 Fensterliste:** die `openings` mit `kind == Window` mit ihren Maßen
  (Breite/Höhe/Brüstung) + Anzahl.

Deterministisch (Material-Gruppen nach `MaterialId` sortiert; Öffnungen in
Modell-Reihenfolge). Totalität: leeres Modell → leere Listen (kein Wurf).

## 2. Definition of Done

- [x] **Ergebnis-Werttypen (neu, `src/hexagon/model/`, pure Werte):**
      `MaterialLine { model::Material material; int component_count; double
      quantity_m3; }` + `MaterialReport { std::vector<MaterialLine> lines;
      double total_m3; }` (EVL-004); `DoorLine { double width_mm; double
      height_mm; }` + `WindowLine { double width_mm; double height_mm; double
      sill_height_mm; }` (EVL-005/006). Framework-frei (kein OCC/Qt/SQLite).
      `MaterialLine` trägt das **ganze `Material`** (Kennwerte) → trägt die
      spätere Kosten-Auswertung.
- [x] **`EvaluatePort` (additiv, read-only):**
      `MaterialReport materialList() const` (EVL-004);
      `std::vector<DoorLine> doorList() const` (EVL-005);
      `std::vector<WindowLine> windowList() const` (EVL-006). Reine Abfrage; kein
      `op`, kein `GeometryKernelPort`, kein `Solid.volume_mm3`.
- [x] **Service implementiert die drei Listen** (`StructureEditService`, `const`):
      **EVL-004:** über `building_.walls` + `.slabs` gruppieren — je Bauteil
      `effectiveMaterial(id)`; **kein Material → übersprungen** (Boundary); sonst
      Gruppe nach `MaterialId` (`std::map` → deterministisch): `component_count++`,
      `quantity_m3 += wallNetVolumeMm3(...)/1e9` bzw. `slabNetVolumeMm3(...)/1e9`
      (Reuse `volume_geometry`, mm³→m³), `material = *eff`. **Dach NICHT
      iteriert** (Volumen-Lücke). `total_m3` = Σ Zeilen. **EVL-005/006:** über
      `building_.openings` nach `kind` filtern → `DoorLine`/`WindowLine` mit den
      Maßen. **Keine Mutation, kein Re-Detect.**
- [x] **AK-Tests mit [`LH-FA-EVL-004/005/006`](../../../../spec/lastenheft.md#lh-fa-evl-004--materiallisten) im Namen** (Kern, OCC-frei über
      `AnalyticGeometry`-Double, analytische Orakel):
      **EVL-004 Happy** (zwei Materialien an Wänden/Decke → je Material
      `quantity_m3` = Σ bekannter Netto-Volumina, `component_count` korrekt);
      **EVL-004 Boundary** (Bauteil ohne Material → nicht in der Liste, kein
      Fehler); **EVL-004 Dach-Lücke** (Dach mit Material → erscheint **nicht** in
      der Materialliste / trägt nicht zu `quantity_m3` bei); **EVL-005** (zwei
      Türen → `doorList` Größe 2, Maße korrekt); **EVL-006** (Fenster → `windowList`
      mit Breite/Höhe/Brüstung); **Boundary leer** (leeres Modell → alle Listen
      leer); **read-only/total** (Listen mutieren nicht; zwei Abfragen identisch).
      **Regression:** bestehende EVL-/Material-Tests unverändert grün.
- [x] **`make arch-check` grün** (neue Typen/Service-Pfad pure Domäne, kein OCC/
      Qt/SQLite, kein `GeometryKernelPort`); **`make gates`** grün; Closure-Notiz
      mit Lerneintrag; CHANGELOG ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)). **Spec (LOW — zwei Teile):** spez. §1
      Listen-Block präzisieren — (1) EVL-004-Menge „Fläche/Volumen" →
      **Netto-Volumen (m³) über Wand+Decke**; (2) **`roofs` aus der
      material-tragenden Aggregations-Menge in die benannte-Lücke-Aufzählung
      verschieben** (Dach material-tragend, aber Volumen welle-3 zurückgestellt) —
      sonst bleibt §1 widersprüchlich (Dach gruppiert, aber Dach-Volumen = Lücke);
      + §8-Historie. **Lastenheft unberührt** (EVL-AK schon auf Niveau seit 017a;
      kein [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/material_line.h` | neu | `MaterialLine` + `MaterialReport` (EVL-004) |
| `src/hexagon/model/opening_line.h` | neu | `DoorLine` + `WindowLine` (EVL-005/006) |
| `src/hexagon/ports/driving/evaluate_port.h` | ändern | `materialList`/`doorList`/`windowList` (additiv) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | drei Listen-Aggregationen (Reuse `effectiveMaterial` + `volume_geometry`) |
| `tests/hexagon/test_evaluate_lists.cpp` | neu | EVL-004/005/006-AK |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |
| `spec/spezifikation.md` | ändern | §1 Listen-Block: EVL-004-Menge=Volumen, Dach-Lücke |
| `spec/spezifikation-historie.md` | ändern | §8-Historie-Zeile |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `docs/reviews/2026-06-16-slice-017f-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- slice-017d/017e done ✓ (`effectiveMaterial` + Material-Persistenz).
- slice-017c done ✓ (`volume_geometry` — Reuse für die EVL-004-Menge).
- Projektinhaber 2026-06-16: alle drei Listen; EVL-004-Menge=Volumen, Dach-Lücke.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (HIGHs blockieren) — **gelaufen ✓**,
  0 HIGH (1 LOW §1-Edit eingearbeitet) → startbar.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **EVL-Modul geschlossen**;
  danach **`wall_type`-Fallback** (optional) + **Welle-3-Verifikation** +
  `done/welle-3-results.md` → **Meilenstein M3**.

## 6. Risiken und offene Punkte

- **EVL-004-Menge = Volumen, Dach-Lücke (Projektinhaber):** aggregiert wird
  Netto-Volumen über Wand+Decke; Dach (material-tragend, Volumen welle-3
  zurückgestellt) **nicht** — benannte Lücke, ehrlich in spez. §1 + Closure.
  Konsistent mit der EVL-002-Dach-Entscheidung (017c).
- **Volumen-Reuse, keine neue Geometrie:** EVL-004 nutzt `wallNetVolumeMm3`/
  `slabNetVolumeMm3` (017c, code-reviewt) — **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a**. Die Eck-/Aussparungs-
  Näherungen aus EVL-002 tragen sich fort (bereits benannt), keine neue Klasse.
- **Gruppierungs-Determinismus:** `std::map<MaterialId, …>` → stabile Reihenfolge
  (nach Id); Test pinnt die Zuordnung elementweise, nicht nur die Summe.
- **EVL-005/006-Form:** flache Liste je Öffnung mit Maßen; „Anzahl" = Listengröße
  (kein Gruppieren nach Maß — float-Gruppierung wäre fragil; AK „mit Anzahl und
  Maßen" ist erfüllt). Review prüft die Form.
- **`windows.frame_material`/`stairs`/`openings`-Material:** nicht in EVL-004
  (spez. §1, benannte Lücke) — kein über-versprochener Material-Umfang.
- **Kosten (MAT-006):** nicht Teil; `MaterialLine` trägt das `Material` (Kennwerte)
  → Kosten = Menge × `cost_per_m3` ist ein Folge-Schritt.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (neue Ergebnis-Werttypen + Port-Methoden, read-only-
  Aggregation, Determinismus, Totalität, Reuse `effectiveMaterial`/`volume_geometry`);
  Risiko niedrig (Aggregation vorhandener Werte, keine neue Geometrie).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (AK mit `LH-FA-EVL-`-ID, analytische Orakel je
  Material-Gruppe + Öffnungs-Maße); Risiko niedrig.

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; Dichte niedrig (§1 Listen-Mengen-/Dach-Lücken-Präzisierung);
  Risiko niedrig (Doku).

## 8. Closure-Notiz

**Ergebnis:** `make gates` grün (**144 Tests**, +5 EVL-Listen-AK; Coverage 92,7 %).
Das **EVL-Modul ist geschlossen**: `EvaluatePort` trägt jetzt alle Auswertungen —
Flächen (017b), Volumen (017c), Material-/Tür-/Fensterlisten (017f). Die drei
Listen sind read-only-Aggregationen (`materialList`/`doorList`/`windowList`):
pull, kein `op`, kein `GeometryKernelPort`. **EVL-004** gruppiert je effektivem
Material (017d `effectiveMaterial`) die Menge = Σ Netto-Volumen (017c
`volume_geometry`) über Wand+Decke — **Dach ausgenommen** (Volumen welle-3
zurückgestellt, benannte Lücke); deterministisch (`std::map<MaterialId>`).
**EVL-005/006** projizieren die `openings` nach `kind` auf ihre Maße.

**DoD:** alle Haken erfüllt — `MaterialReport`/`MaterialLine`/`DoorLine`/
`WindowLine`, drei Listen-Methoden + Service-Aggregation (Reuse `effectiveMaterial`
+ `volume_geometry`), AK [`LH-FA-EVL-004/005/006`](../../../../spec/lastenheft.md#lh-fa-evl-004--materiallisten) (Gruppierung/Boundary/Dach-Lücke/
Tür/Fenster/leer/read-only), `make arch-check` + `make gates` grün, spez. §1
nachgezogen (Menge=Volumen + `roofs`-Lücke), CHANGELOG.

**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review (unabhängig):** 0 HIGH, 1 LOW eingearbeitet — der §1-Edit war
zweiteilig: Menge → Netto-Volumen (m³) **und** `roofs` aus der Aggregations-Menge
in die benannte Lücke verschieben (sonst wäre §1 widersprüchlich: Dach gruppiert,
aber Dach-Volumen = Lücke). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** (keine neue Geometrie — Reuse der
017c-code-reviewten Volumen-Helfer).

**Lerneintrag (modul-05, Form „geschärfte Praxis"):** Schicht-übergreifender
**Reuse** trägt — EVL-004 ist die dritte Auswertung, die `effectiveMaterial`
(017d) + `volume_geometry` (017c) wiederverwendet; weil die Geometrie bereits
(017c-[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) verifiziert war, blieb [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) hier n/a und der Slice war trotz vier
neuer Werttypen + drei Port-Methoden in **einer** Sitzung review-bar. Der
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Befund war **Spec-Konsistenz** (das §1 listete `roofs` als material-tragend,
während EVL-002 das Dach-Volumen zurückstellt) — eine stille Inkonsistenz zwischen
zwei welle-3-Entscheidungen, die das unabhängige Plan-Review fing.

**Restrisiko / Nachfolge:** **EVL-Modul geschlossen.** Offen in welle-3:
- **Kosten-Auswertung** (MAT-006: Menge × `cost_per_m3`) — `MaterialLine` trägt
  das `Material` mit Kennwerten, der Kosten-Aufschlag ist ein kleiner Folge-Schritt.
- **`wall_type`-Fallback** (Wall-Type-Entität, 017d-Lücke) + **Dach-Volumen** /
  Eck-/Aussparungs-Näherungen (EVL-002-Re-Eval-Trigger).
- **Welle-3-Verifikation** + `done/welle-3-results.md` → **Meilenstein M3
  „Auswertbar"**.
