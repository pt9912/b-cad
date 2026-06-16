---
id: slice-017g
titel: Auswertung Kosten — MAT-006 (Kostenkennwerte je Material in die Summe)
status: done
welle: welle-3-auswertung
lastenheft_refs: [[LH-FA-MAT-006](../../../../spec/lastenheft.md#lh-fa-mat-006--kostenkennwerte)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0012](../../adr/0012-evaluations-architektur.md)]
---

# Slice 017g: Auswertung Kosten — MAT-006

**Status:** done (2026-06-16, `make gates` grün, 145 Tests). Schließt die
**Material-Kennwert-Nutzung** (welle-3-Substanz MAT+EVL komplett). Unabhängiges
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review** (0 HIGH, 2 LOW Politur eingearbeitet —
[Report](../../../reviews/2026-06-16-slice-017g-plan.md)). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a**, kein
Code-Review (read-only). Closure-Notiz §8.

**Welle:** welle-3-auswertung (siebter Slice; schließt die **Material-Kennwert-
Nutzung** — letzter Auswertungs-Baustein vor der Welle-Verifikation).

**Bezug:** [LH-FA-MAT-006](../../../../spec/lastenheft.md#lh-fa-mat-006--kostenkennwerte) (Kostenkennwerte je m²/m³ fließen je zugewiesenem
Bauteil in die Summe). Baut **direkt** auf der Materialliste (017f
`materialList`): die trägt schon je Material die `quantity_m3` **und** das
`Material` (mit `cost_per_m3`). [ADR-0012](../../adr/0012-evaluations-architektur.md) (Auswertung = read-only-Ableitung),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (reine Kern-Query). **Kein neuer ADR, keine neuen Werttypen** (nur die
017f-`MaterialLine`/`MaterialReport` erweitert).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-16.

**Schnitt-Herkunft:** Kosten-Aufschlag auf die Materialliste. **Design (aus der
017f-Volumen-Entscheidung):** Kosten je Material = `quantity_m3 × cost_per_m3`;
Σ = Projekt-Material-Kosten. **`cost_per_m2` = benannte Lücke** (Flächen-Kosten
bräuchten eine Flächen-Menge — EVL-004 führt **Volumen**; Re-Eval mit einer
flächen-basierten Materialliste).

**Bewusst NICHT Teil (benannte Lücken / Folge):**

- **`cost_per_m2`** — Flächen-Kosten; EVL-004 führt Volumen → keine Flächen-Menge.
  Benannte Lücke (spez. §1); Re-Eval mit flächen-basierter Aggregation.
- **Dach/`stairs`/`openings`-Kosten** — erben die EVL-004-Lücken (Dach kein
  Volumen; jene tragen kein `material_id`).

---

## 1. Ziel

Die Material-Kennwert-Nutzung schließen: je Material-Zeile der Materialliste die
**Kosten** = `quantity_m3 × cost_per_m3` (sofern `cost_per_m3` gesetzt), und die
**Projekt-Material-Kosten-Summe**. Read-only, pull, kein `op` — als Erweiterung
der bestehenden `materialList()`-Aggregation (017f); **kein** neuer Port, **kein**
neuer Werttyp.

## 2. Definition of Done

- [x] **`MaterialLine` + `MaterialReport` erweitert** (`material_line.h`):
      `MaterialLine` gewinnt `std::optional<double> cost` (= `quantity_m3 ×
      *material.cost_per_m3`, falls `cost_per_m3` gesetzt; sonst `std::nullopt` —
      „kein Kostenkennwert" ≠ Kosten 0). `MaterialReport` gewinnt
      `double total_cost{0.0}` (Σ der Zeilen-Kosten; Materialien ohne
      `cost_per_m3` tragen 0 bei). Pure Werte, framework-frei.
- [x] **`materialList()` berechnet die Kosten** (`StructureEditService`,
      bestehende Methode erweitert): nach dem Aufsummieren der `quantity_m3` je
      Material je Zeile `line.cost = cost_per_m3.has_value() ? quantity_m3 ×
      *cost_per_m3 : nullopt`; `report.total_cost += line.cost.value_or(0.0)`.
      **Keine Mutation, kein neuer Pfad** (gleiche Iteration). **Kein neuer
      `EvaluatePort`-Eintrag** (die Kosten reisen in der bestehenden
      `MaterialReport`).
- [x] **AK-Test mit [`LH-FA-MAT-006`](../../../../spec/lastenheft.md#lh-fa-mat-006--kostenkennwerte) im Namen** (Kern, OCC-frei, analytisches
      Orakel): Material **mit** `cost_per_m3` an Wand/Decke → Zeilen-`cost` =
      bekannte `Menge × cost_per_m3`, `total_cost` = Σ; Material **ohne**
      `cost_per_m3` → Zeilen-`cost` == `std::nullopt` (**nicht** 0) und trägt
      **0** zur Summe bei (Boundary, kein Fehler). **Regression:** bestehende
      017f-Materiallisten-AK unverändert grün (additive Felder).
- [x] **`make arch-check` grün** (pure Domäne); **`make gates`** grün;
      Closure-Notiz; CHANGELOG ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)). **Spec:** spez. §1 Listen-Block um
      „Kosten = Menge × `cost_per_m3` je Material + Summe; `cost_per_m2`-Lücke"
      ergänzen + §8-Historie. **Lastenheft unberührt** (MAT-006-AK schon auf
      Niveau seit 017a; kein [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/material_line.h` | ändern | `MaterialLine.cost` + `MaterialReport.total_cost` |
| `src/hexagon/services/structure_edit_service.cpp` | ändern | Kosten in `materialList()` berechnen |
| `tests/hexagon/test_evaluate_lists.cpp` | ändern | MAT-006-AK (Kosten + Kein-Kennwert-Boundary) |
| `spec/spezifikation.md` | ändern | §1: Kosten = Menge × `cost_per_m3`, `cost_per_m2`-Lücke |
| `spec/spezifikation-historie.md` | ändern | §8-Historie-Zeile |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `docs/reviews/2026-06-16-slice-017g-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- slice-017f done ✓ (`materialList` mit `quantity_m3` + `Material`/`cost_per_m3`).
- Projektinhaber 2026-06-16: Kosten als Folge-Schritt; `cost_per_m2`-Lücke.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (HIGHs blockieren) — **gelaufen ✓**, 0 HIGH.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **Material-Kennwert-Nutzung
  geschlossen**; danach `wall_type`-Fallback (optional) + **Welle-3-Verifikation**
  + `done/welle-3-results.md` → **Meilenstein M3**.

## 6. Risiken und offene Punkte

- **`cost_per_m2`-Lücke (Design):** EVL-004 führt Volumen → Flächen-Kosten nicht
  berechenbar. Benannt (spez. §1 + Closure); Re-Eval mit flächen-basierter
  Aggregation. Kein über-versprochener Kosten-Umfang.
- **Kein-Kennwert ≠ 0 (Korrektheit):** Material ohne `cost_per_m3` → `cost`
  `nullopt`, **nicht** `0.0` — sonst sähe „kein Kostenkennwert" wie „kostenlos"
  aus. Test pinnt es (analog zur 017e-NULL-Korrektheit, hier in-memory).
- **Additive Erweiterung:** `MaterialLine`/`MaterialReport` gewinnen Felder
  (Default 0/`nullopt`) → bestehende 017f-AK bleiben textlich grün.
- **Einheiten-Konsistenz:** `quantity_m3` (m³) × `cost_per_m3` (Kosten/m³) →
  Kosten; die Materialliste-Menge ist welle-3 Volumen (017f), daher `cost_per_m3`
  (nicht `cost_per_m2`).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte niedrig (zwei additive Felder + eine Multiplikation in der
  bestehenden Aggregation); Risiko niedrig (read-only, keine Geometrie, kein
  neuer Pfad).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte niedrig (ein MAT-006-AK + Kein-Kennwert-Boundary); Risiko
  niedrig.

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; Dichte niedrig (§1 Kosten-Satz + `cost_per_m2`-Lücke); Risiko
  niedrig.

## 8. Closure-Notiz

**Ergebnis:** `make gates` grün (**145 Tests**, +1 MAT-006-AK; Coverage 92,7 %).
Die **Material-Kennwert-Nutzung ist geschlossen**: die Materialliste
(`materialList`) trägt je Material `cost = quantity_m3 × cost_per_m3`
(`MaterialLine.cost`, `std::optional`) + die Projekt-Summe
(`MaterialReport.total_cost`). Additive Erweiterung der 017f-Typen — **kein neuer
Port/Werttyp**, gleiche Aggregations-Iteration. Material **ohne** `cost_per_m3` →
`cost` `nullopt` (kein Kennwert ≠ kostenlos), trägt 0 zur Summe.

**DoD:** alle Haken erfüllt — `MaterialLine.cost` + `MaterialReport.total_cost`,
Kosten in `materialList()`, AK [`LH-FA-MAT-006`](../../../../spec/lastenheft.md#lh-fa-mat-006--kostenkennwerte) (Kosten + Kein-Kennwert-Boundary),
`make gates` grün, spez. §1 nachgezogen (Kosten + `cost_per_m2`-Lücke), CHANGELOG.
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)** 0 HIGH (2 LOW Politur eingearbeitet: direkter `<optional>`-Include,
§1-Einwebung). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a**, kein Code-Review (read-only, keine Persistenz).

**Lerneintrag (modul-05):** der **NULL-Sicher-Reflex aus 017e trägt in-memory
weiter** — „kein Kostenkennwert" als `nullopt` (nicht 0,0) hält „nicht bepreist"
von „kostenlos" getrennt; im AK gepinnt. Bestätigt: die NULL-vs-0-Disziplin gilt
überall, wo ein abgeleiteter Wert von einem optionalen Eingabe-Kennwert abhängt,
nicht nur in der Persistenz.

**Restrisiko / Nachfolge:** Damit ist der **welle-3-Substanz-Scope (MAT + EVL)
komplett** — MAT-001/002/003 (017d) · MAT-005 U-Wert (via `effectiveMaterial`
abrufbar, 017d) · MAT-006 Kosten (017g) · EVL-001/003 (017b) · EVL-002 (017c) ·
EVL-004/005/006 (017f). Offen vor **M3**:
- **`wall_type`-Fallback** (Wall-Type-Entität, 017d-Lücke) · `cost_per_m2`/
  Dach-Volumen/Eck-/Aussparungs-Näherungen (benannte Re-Eval-Trigger) ·
  MAT-004 Texturen (Sicht).
- **Welle-3-Verifikation** (unabhängig, analog welle-1/-2) + Carveout-Audit +
  `done/welle-3-results.md` → **Meilenstein M3 „Auswertbar"**.
