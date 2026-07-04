---
id: slice-031
titel: lint-Härtung II — Folge-Kandidaten misc-const-correctness + modernize-use-nodiscard scharfschalten (evidence-first)
status: in-progress
welle: harness-steering
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md), [ADR-0017](../../adr/0017-plugin-api-abi.md)]
---

# Slice 031: lint-Härtung II — const-correctness + use-nodiscard

**Status:** in-progress — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
(zwei unabhängige Agenten) **0 HIGH / 2 MED / 2 LOW / INFO**; Start **nicht
blockiert**, Findings eingearbeitet: MED-1 (Re-Exklusion ist global, keine
per-Fundstelle-Ausnahme — Klausel §2/§6 präzisiert) + MED-2 (Dry-Run-Kommandoform
an die **gebackene** `bcad:build`-Quelle gepinnt, Sollwerte 13/10 — Bind-Mount
ohne `build/` liefert still 37 Phantom-Befunde) + LOW-1 (§6-Charakterisierung:
2 `std::ifstream`, 3 Kern, 3 main.cpp — nicht „überwiegend OCC"). Aktuelle
Zählung **const 13 / nodiscard 10**. [Report](../../../reviews/2026-07-04-slice-031-plan.md).

**Welle:** harness-steering (Quergewerk, direkte Fortsetzung von slice-027 —
**welle-5-Scope unberührt**; Roadmap erhält den üblichen Quergewerk-Eintrag).

**Bezug:** `make lint`-Gate (0-Befunde-Latte). slice-027 hat die zwei Checks als
**zurückgestellte wertvolle Folge-Kandidaten** benannt (Umfangs-Deckel/
kumulativer Escape); Projektinhaber-Anstoß (2026-07-04) sie jetzt scharfzuschalten.
Baseline-Evidenz: die slice-027-Befund-Matrix
([dryrun](../../../reviews/2026-07-04-slice-027-dryrun.md)) — `misc-const-correctness`
**14**, `modernize-use-nodiscard` **10** (Stand pre-slice-027; aktuelle Zählung im
Beleg dieses Slice).

**Kein ADR (begründet):** Aktivierung zusätzlicher Checks ist eine
**Verschärfung** — [AGENTS.md §2.6](../../../../AGENTS.md) verlangt ADRs nur für
Gate-**Lockerungen**. Slice-Deckung genügt (`.clang-tidy` verlangt „ADR- **oder**
Slice-ID"). Präzedenz: slice-002 (Initial-Satz), slice-027 (7-Familien-Erweiterung).

**Autor:** Dietmar Burkard (Anstoß) / AI-Session (Ausführung). **Datum:** 2026-07-04.

**Nicht Teil:** der dritte slice-027-Folge-Kandidat
`readability-inconsistent-declaration-parameter-name` (19, rein kosmetisch) —
bewusst **nicht** in diesem Schnitt (Projektinhaber-Wahl: nur die zwei
wert-tragenden Checks). Keine weiteren Familien-Änderungen; keine
Schnittstellen-/Verhaltens-Umbauten.

## 1. Ziel

Die zwei zurückgestellten Checks werden scharfgeschaltet: aus der
`.clang-tidy`-Exclude-Liste (Checks + WarningsAsErrors) entfernt, ihre Befunde
**behandelt** (kein NOLINT), Kommentar-Nachzug. Die 0-Befunde-Latte bleibt.

## 2. Definition of Done

- [ ] **Dry-Run-Beleg (aktuelle Zählung):** `misc-const-correctness` +
      `modernize-use-nodiscard` über `src/`+`plugins/` im gepinnten `bcad:build`
      ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)) — Fundstellen-
      Liste + Anzahl als Beleg `docs/reviews/2026-07-04-slice-031-dryrun.md`
      (inkl. `docker run`-Kommando).
- [ ] **`.clang-tidy`:** `-misc-const-correctness` und `-modernize-use-nodiscard`
      aus **Checks und WarningsAsErrors** entfernt; der Kommentarkopf zieht die
      zwei aus „zurückgestellte Folge-Kandidaten" heraus (jetzt aktiviert+gefixt).
- [ ] **Befunde behandelt (verhaltensneutral, kein NOLINT):**
      **const-correctness** = `const`-Qualifikation lokaler Variablen (Vorbehalt:
      stateful **OCC-Builder/Streams** — jede Änderung muss kompilieren
      [`--warnings-as-errors`] **und** die Test-Suite bestehen; ein false-positive
      wird per **Alternativ-Fix** aufgelöst — oder, falls nötig, per begründetem
      Wieder-Deaktivieren des **ganzen** Checks in `.clang-tidy` [Check-Exklusion
      ist **global**, es gibt keine per-Fundstelle-Ausnahme; §2.4 gewahrt, **kein**
      NOLINT]; bei den 13 const-kompilierbaren Fällen **nicht** erwartet).
      **use-nodiscard** = `[[nodiscard]]` an const-Accessoren; bei Plugin-/
      Interface-Methoden **konsistent** (Basis-Deklaration im Plugin-API mit,
      damit Overrides nicht auseinanderlaufen).
- [ ] **Gate-Verträge unverändert wahr:** AGENTS §3 / README §Sensors nennen
      keine Check-Namen (nur „clang-tidy, 0 Befunde") — **kein** Doku-Nachzug;
      Schwelle/Scope unverändert (Verschärfung, kein §2.6-Fall, kein ADR).
- [ ] **Roadmap:** Quergewerk-Eintrag „Historische Trigger-Verschiebungen".
- [ ] `make gates` grün; **kein** Code-Verhalten geändert (`make test` 228/228
      unverändert); CHANGELOG; Closure-Notiz mit Aktivierungs-/Fix-Bilanz;
      Lifecycle-Move nach `done-archive/` (reiner `git mv`, [§2.8](../../../../AGENTS.md)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/reviews/2026-07-04-slice-031-dryrun.md` | neu | Beleg: aktuelle Fundstellen der zwei Checks |
| `.clang-tidy` | ändern | zwei Excludes entfernen (Checks+WAE); Kommentarkopf nachziehen |
| `src/**` · `plugins/**` (punktuell) | ändern | `const`-Qualifikation + `[[nodiscard]]` (kein Verhaltens-Umbau, kein NOLINT) |
| `src/plugin_api/**` | ggf. ändern | Basis-Accessoren `[[nodiscard]]` für Override-Konsistenz |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Quergewerk-Eintrag |
| `CHANGELOG.md` | ändern (Closure) | Unreleased-Eintrag slice-031 |
| `docs/reviews/2026-07-04-slice-031-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Startbar nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review;
  keine fachliche Vorbedingung (Quergewerk, Fortsetzung slice-027).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → welle-5-Arbeit läuft
  unverändert weiter.

## 6. Risiken und offene Punkte

- **const-correctness false-positives (Kern-Risiko):** die **13** Fundstellen
  (aktuelle Zählung, Beleg) verteilen sich: **OCC-Builder ×5**
  (`BRepBuilderAPI_MakeFace`/`BRepMesh_IncrementalMesh`/`BRep_Builder`),
  **`std::ifstream` ×2**, **Hexagon-Kern** `structure_edit_service` ×3
  (`model::Opening&` → `const&`, nur gelesen), **`main.cpp`-Composition ×3**
  (`QApplication`/`OccGeometryAdapter`/`ViewModelMeshSource`). 6 der 13 liegen
  also **nicht** bei OCC. clang-tidy flaggt nur, wenn alle Verwendungen
  const-kompatibel sind — der `const`-Zusatz ist daher kompilierbar; das Restrisiko
  (OCC-`mutable`-Lazy-Build, Stream-Zustand) wird **durch die Test-Suite**
  abgefangen (228/228). Ein false-positive wird per Alternativ-Fix oder begründetem
  Voll-Check-Re-Exclude aufgelöst (**Check-Exklusion ist global**), nie per NOLINT
  ([AGENTS.md §2.4](../../../../AGENTS.md)).
- **nodiscard-Override-Konsistenz:** `[[nodiscard]]` wird nicht vererbt; wird eine
  Plugin-/Interface-Accessor-Methode markiert, erhält die **Basis** im
  `src/plugin_api/` denselben Marker (sonst driftet die API). Reine Test-Fixture-
  Plugins in `plugins/` werden mitmarkiert (AK-Fixtures, kein öffentlicher Vertrag).
- **Verhaltens-Risiko:** beide Fix-Klassen sind Qualifikations-/Attribut-
  Ergänzungen ohne Laufzeit-Semantik; jeder Lauf durch die volle Suite.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF (`.clang-tidy`, gepinnte Toolchain). **Phase-Reife:** etabliert.
  **Evidenz-Risiko:** niedrig-mittel (const-correctness-false-positive-Vorbehalt,
  durch Tests abgefangen). **Reconciliation:** keiner.

### Sub-Areas der Fixes: Geometrie-Adapter · IO-Adapter · Plugin-Host/-API

- **Modus:** je GF (deklariert). Verhaltensneutrale Rand-Qualifikation unter der
  §2-Grenze („kein Verhaltens-Umbau, sonst Check auslassen").

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; Quergewerk-Disziplin ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  davor, Roadmap-Eintrag). **Risiko:** niedrig.
