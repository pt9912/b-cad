---
id: slice-034
titel: Commit-Traceability-Gate — d-check-Modul `commits` (AGENTS §4 computational)
status: done
welle: welle-5-erweiterung (Quergewerk harness-steering)
lastenheft_refs: []  # reines Prozess-/Gate-Steering, keine LH-Anforderung
adr_refs: []         # kein ADR (Verschärfung/Prozess-Gate, §2.6 n/a)
---

# Slice 034: Commit-Traceability-Gate — Modul `commits`

**Status:** **done** (ausgeführt 2026-07-05; angelegt evidence-first — Fallout vor dem
Plan gemessen, Muster [slice-027](../done/slice-027-lint-haertung.md)/[slice-033](../done/slice-033-dcheck-matrix-adr-slice.md)).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
1 HIGH behoben** (H1 — zweite §4-Fundstelle in `harness/README.md`; + 2 MED / 2 LOW
eingearbeitet; [Report](../../../reviews/2026-07-05-slice-034-plan.md)).

**Welle:** welle-5-erweiterung — **Quergewerk `harness-steering`** (kein
Wellen-Feature; Muster [slice-030](../done/slice-030-a-check-gate.md)/[slice-033](../done/slice-033-dcheck-matrix-adr-slice.md)).
**DRW-Scope (032a/b/c) unberührt** — pausiert.

**Bezug:** die Traceability-Regel steht an **zwei** normativen Stellen —
[AGENTS.md §4](../../../../AGENTS.md) **und** [`harness/README.md` §Traceability rules](../../../../harness/README.md)
(„PRs/Commits müssen mindestens eine LH-*/ADR-*-ID nennen") — heute reine Konvention. Das d-check-Modul `commits`
(ab v0.35, [MR-014](../../../../harness/conventions.md)/d-check.mk-Weg schon da) macht sie
**computational** (tool-native Ablösung des `trace-check`-Skript-Musters, wie a-check).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-05.

**Schnitt-Herkunft:** Modul-Adoptions-Steering (aus der d-check-Modul-Bewertung nach
slice-033 emergiert). `commits` ist einer der drei „Regel→computational"-Gewinner
(neben `vcs` §2.5 und `planning` Lifecycle); zuerst gewählt (höchster Wert, klarer
Scope, a-check-erprobt). **Reine Doku-/Tooling-Entscheidung, kein Produktions-Code.**

---

## 1. Ziel

Jede Commit-Message trägt eine **Traceability-Kennung** (`slice-*`, `ADR-*`, `MR-*`,
`LH-*`) — die [AGENTS.md §4](../../../../AGENTS.md)-Regel wird von Konvention zu
**Gate**. Umsetzung über `make doc-commits` (d-check-Modul `commits`, git-Range),
als **CI-only-Sensor** (Muster `make schema-check`/`make io-smoke`) — **nicht** in
`make gates` (git-abhängig, nicht-hermetisch).

**Evidenz (vor dem Plan gemessen, Muster slice-033):**
- **Recent range (`HEAD~30..HEAD`): `make doc-commits` = 0 `commit-untraceable`** — die
  `id-patterns` (`slice-\d{3}` · `ADR-\d{4}` · `MR-\d{3}` · `LH-(FA-…|QA)-\d+`) treffen
  b-cads gelebte Disziplin **exakt** (nahezu jeder Commit trägt `slice-NNN`).
- **Ältere Range (`HEAD~80..HEAD~30`): 1 Befund** — `502fb86` („docs: zweiten
  README-Status-Block streichen") ist der einzige kennungslose Commit. **Vor-Adoptions,
  außerhalb künftiger PR-Ranges** (CI prüft `origin/main..HEAD`) → keine laufende Last.

## 2. Definition of Done

- [x] **`.d-check.yml` `commits`-Block** (`id-patterns: [slice-\d{3}, ADR-\d{4}, MR-\d{3},
      LH-(FA-[A-Z][A-Z0-9]*|QA)-\d+]` — `slice-\d{3}` deckungsgleich zu `matrix`/[MR-002](../../../../harness/conventions.md#mr-002--id-schema-für-b-cad) [Review-L1];
      `exempt-pattern: '^(Merge |Revert )'`). **NICHT** in
      die `modules`-Liste (git-abhängig, opt-in via `--enable`/`doc-commits`). *(Bereits als
      Messschritt vollzogen.)*
- [x] **CI-only-Sensor `make doc-commits`** (d-check.mk, git-Range) in die **CI-Befehlsliste**
      aufgenommen (Muster `make schema-check`/`make io-smoke`): CI ruft `make doc-commits
      RANGE=<base>..<head>` (Push-/PR-Range, typ. `origin/main..HEAD`); Exit 1 bei
      kennungslosem Commit bricht ab. **Kein `make gates`-Member** (nicht-hermetisch).
      *(Kein `.github/workflows/` vorhanden — die CI-Befehlsliste ist heute dokumentarisch;
      der Sensor wird dort verankert, real-verdrahtet sobald eine CI-Datei existiert.)*
- [x] **Optional: lokaler `commit-msg`-Hook** (dokumentiert, opt-in): `d-check --enable
      commits --commit-msg <datei>` fängt die kennungslose Message **vor** dem Commit
      (bessere DX). Kein Pflicht-Teil des Gates (die CI-Range ist die Durchsetzung).
- [x] **`harness/conventions.md` neuer Eintrag [MR-015](../../../../harness/conventions.md)** (Verschärfung/Prozess-Gate, kein
      [§2.6](../../../../AGENTS.md)-Lockerungsfall; Muster [MR-014](../../../../harness/conventions.md)):
      dokumentiert (1) die `commits`-Adoption + `id-patterns`, (2) den CI-only-Sensor-Status
      (Muster `schema-check`), (3) dass die `id-patterns` **bewusst breiter** sind als der
      bisherige Regel-Wortlaut (die gelebte `slice-`/`MR-`-Praxis); **(3a) dass Steering-/
      Prozess-Slices legitim anforderungsfrei sind** (`lastenheft_refs`/`adr_refs` leer, wie
      slice-034 selbst) und `slice-*` ihr Traceability-Anker ist — der ADR-Verzicht deckt damit
      auch die *Verbreiterung* des Kennungs-Sets, nicht nur die Verschärfung (Review-M1); (4)
      die tool-native Ablösung des `trace-check`-Musters (a-check-Präzedenz).
- [x] **Wortlaut-Nachzug an BEIDEN Regel-Fundstellen (Review-H1):** die Traceability-Regel steht
      in [AGENTS.md §4](../../../../AGENTS.md) **und** in [`harness/README.md` §Traceability rules](../../../../harness/README.md) —
      beide nennen heute nur `LH-*`/`ADR-*`. **Beide** werden **symmetrisch** um **`slice-*` und
      `MR-*`** ergänzt + die Gate-Bindung (`make doc-commits`) benannt, sodass der Regel-Text (an
      **beiden** Stellen) und die Gate-`id-patterns` deckungsgleich sind. Andernfalls bliebe genau
      der **stille Regel↔Gate-Drift** (drei Formulierungen, zwei Kennungs-Sets), den dieser Slice
      tilgen soll.
- [x] **Gate-Doku:** neue Zeile in [`harness/README.md` §Sensors](../../../../harness/README.md)
      + [AGENTS.md §3](../../../../AGENTS.md) — `make doc-commits` als **CI-only-Sensor**
      (kein Gate, Muster `schema-check`/`io-smoke`), Bindung [AGENTS.md §4](../../../../AGENTS.md).
- [x] **`CHANGELOG.md`** (grobkörnig, [MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- [x] **Roadmap-Quergewerk-Eintrag (Review-M2):** eine Zeile in [`roadmap.md`](../in-progress/roadmap.md)
      §Historische Trigger-Verschiebungen (Muster slice-033-Zeile) — jedes harness-steering-
      Quergewerk trägt eine; das `planning`-Modul prüft Roadmap↔in-progress-Konsistenz.
- [x] **`make gates` grün** (unberührt — `commits` ist **nicht** in `gates`; nur additive
      `.d-check.yml`-`commits`-Config, die `docs-check` nicht anfasst — `make docs-check` grün
      **mit** dem Block bereits belegt, Config-Parse-Regression ausgeschlossen [Review-L2]).
      `make doc-commits RANGE=origin/main..HEAD` (die künftige CI-Range) = 0 Befunde belegt.
- [x] **Nicht Teil:** die DRW-Arbeit; die weiteren Modul-Kandidaten `vcs` (§2.5) und
      `planning` (Lifecycle) — eigene Folge-Slices.

## 3. Plan (vor Ausführung)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.d-check.yml` | ändern | `commits`-Block (`id-patterns` + `exempt-pattern`); **nicht** in `modules` |
| CI-Befehlsliste (`harness/README.md` §Sensors, [AGENTS.md §3](../../../../AGENTS.md)) | ändern | `make doc-commits` als CI-only-Sensor (Muster `schema-check`/`io-smoke`) |
| `AGENTS.md` §4 **+** `harness/README.md` §Traceability rules | ändern | **beide** Fundstellen symmetrisch um `slice-*`/`MR-*` + Gate-Bindung ergänzen (Review-H1; Deckung mit `id-patterns`) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Quergewerk-Eintrag §Historische Trigger-Verschiebungen (Review-M2) |
| `harness/conventions.md` | ändern | neuer Eintrag **[MR-015](../../../../harness/conventions.md)** (commits-Adoption) |
| `CHANGELOG.md` | ändern | `[Unreleased]`-Eintrag (grobkörnig) |
| `docs/reviews/{2026-07-05-slice-034-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |
| lokaler `commit-msg`-Hook | ggf. neu (opt-in) | dokumentierter DX-Hook, kein Gate-Pflichtteil |

## 4. Trigger

- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review durchlaufen** (1 HIGH behoben + 2 MED/2 LOW eingearbeitet) → **startbar.**
  Das `commits`-Modul + `doc-commits`-Target sind mit slice-033 (d-check v0.37.1 /
  d-check.mk) real; `id-patterns` evidenzbasiert (0 recent Fallout gemessen).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (unberührt), `make doc-commits RANGE=origin/main..HEAD`
  = 0, Closure-Notiz → die weiteren Modul-Kandidaten (`vcs` §2.5, `planning` Lifecycle)
  werden startbar.

## 6. Risiken und offene Punkte

- **§4-Wortlaut vs. `id-patterns` (zentral):** §4 nennt nur `LH-/ADR-`; die Praxis (und
  damit die `id-patterns`) sind breiter (`slice-` dominiert, `MR-`). **Ohne** §4-Nachzug
  driftete Regel-Text von Gate-Verhalten — der Nachzug (§4 um `slice-`/`MR-` ergänzen) hält
  beide deckungsgleich. Alternative (id-patterns auf §4-Literal verengen) würde **fast alle**
  b-cad-Commits flaggen (die tragen `slice-NNN`, nicht `LH-/ADR-`) → verworfen.
- **Nicht-hermetisch → CI-only (kein `gates`-Member):** `commits` braucht eine git-Range
  (`--range`), die der hermetische `make gates`-Pfad nicht liefert. Verankerung daher in der
  **CI-Befehlsliste** (Muster `schema-check`/`io-smoke`, die aus demselben Grund CI-only
  sind). Da **kein `.github/workflows/`** existiert, ist die Verdrahtung heute dokumentarisch;
  real-scharf mit der ersten CI-Datei (benannt).
- **Vor-Adoptions-Commits (`502fb86`):** ein historischer Commit ohne Kennung — außerhalb
  künftiger PR-Ranges (`origin/main..HEAD`), daher keine laufende Last. Bei einem Voll-
  Historie-Lauf würde er flaggen; die CI prüft aber Push-/PR-Ranges, nicht die Voll-Historie.
- **Range-Strategie (benannt):** `origin/main..HEAD` (PR-Commits) ist der Standardfall; für
  Push-auf-main die Push-Range. Die exakte CI-Range-Ableitung fixiert die CI-Datei; der
  Sensor selbst ist range-agnostisch.
- **Kein Scope-Creep:** `vcs`/`planning`/`tracked` und die DRW-Arbeit bleiben unberührt.
  `commits` ändert **kein** Produktions-Code, **kein** Schema, **keine** LH-Anforderung.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Konventionen & Harness-Doku

- **Modus:** GF; **Dichte:** hoch — Heimat der `MR-NNN`, Verschärfungs-Disziplin
  ([AGENTS.md §2.6](../../../../AGENTS.md): kein ADR für ein *strengeres* Gate),
  Traceability-Regel §4. **Phase-Reife:** Phase 4. **Risiko:** niedrig (0 recent Fallout).

### Sub-Area: Build & Toolchain

- **Modus:** GF; **Dichte:** hoch — d-check.mk/`.d-check.yml`-Konvention ([MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-014](../../../../harness/conventions.md)),
  CI-only-Sensor-Muster (`schema-check`/`io-smoke`). **Phase-Reife:** Phase 4. **Risiko:**
  niedrig.

## 8. Closure-Notiz

**Ausgeführt 2026-07-05** — DoD vollständig; `make gates` grün, `make docs-check` = 0 **mit**
`commits`-Block (Config-Parse-Regression ausgeschlossen), `make doc-commits RANGE=origin/main..HEAD`
= 0.

**Beobachtbare Closure-Kriterien:**
- d-check-Modul `commits` adoptiert (`.d-check.yml`-Block; `id-patterns` `slice-\d{3}`/`ADR`/`MR`/`LH`,
  `exempt-pattern` Merge/Revert) — **nicht** im `modules:`-Set → `make gates`/`make docs-check`
  hermetik-erhaltend unberührt.
- `make doc-commits` als **CI-only-Sensor** dokumentiert ([`harness/README.md` §Sensors](../../../../harness/README.md)
  + [AGENTS.md §3](../../../../AGENTS.md), Muster `schema-check`/`io-smoke`).
- **Beide** §4-Regel-Fundstellen ([AGENTS.md §4](../../../../AGENTS.md) **+**
  [`harness/README.md` §Traceability rules](../../../../harness/README.md)) symmetrisch um
  `slice-*`/`MR-*` + Gate-Bindung ergänzt; [MR-015](../../../../harness/conventions.md) verankert.

**Lerneintrag:** Der [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review fing den zentralen Fehler (H1) — die §4-Regel steht an **zwei**
normativen Stellen; nur eine zu amendieren hätte genau den *stillen Regel↔Gate-Drift* erzeugt, den
der Slice tilgt. Lehre: **bei einer Regel-Verschärfung zuerst *alle* Fundstellen der Regel im
Doku-Korpus lokalisieren** (grep über AGENTS/README/conventions), bevor die Amendment-Fläche
fixiert wird. Zweite Lehre: **Steering-/Prozess-Slices sind legitim anforderungsfrei** — `slice-*`
ist ihr Traceability-Anker (in [MR-015](../../../../harness/conventions.md) explizit, nicht nur „id-patterns breiter").

**Folge:** die weiteren Modul-Kandidaten `vcs` (§2.5) und `planning` (Lifecycle) werden startbar
(eigene evidence-first-Steering-Slices).
