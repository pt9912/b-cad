---
id: slice-037
titel: Fresh-Clone-Gate — d-check-Modul `tracked` (Link-Ziele git-getrackt)
status: done
welle: welle-5-erweiterung (Quergewerk harness-steering)
lastenheft_refs: []  # reines Prozess-/Gate-Steering, keine LH-Anforderung
adr_refs: []         # kein ADR (Verschärfung/Prozess-Gate, §2.6 n/a)
---

# Slice 037: Fresh-Clone-Gate — Modul `tracked`

**Status:** **done** (ausgeführt 2026-07-05; angelegt evidence-first, Muster
[slice-036](../done/slice-036-planning-lifecycle.md)).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
0 HIGH / 1 MED / 3 LOW → startbar** (Gate-Member als „einzig werthaltige Platzierung" bestätigt
[CI-Vakuität]; M1/L1–L4 eingearbeitet; [Report](../../../reviews/2026-07-05-slice-037-plan.md)).

**Welle:** welle-5-erweiterung — **Quergewerk `harness-steering`** (kein Wellen-Feature;
Muster [slice-035](../done/slice-035-vcs-adr-immutability.md)/[slice-036](../done/slice-036-planning-lifecycle.md)).
**DRW-Scope (032a/b/c) unberührt** — pausiert.

**Bezug:** eine **neue** Referenz-Integritäts-Regel (heute nirgends geregelt — repo-weiter Sweep
bestätigt): jedes **auflösbare, existierende** Link-/Bild-**Datei**-Ziel im Doku-Korpus ist im
**git-Index getrackt** — ein untracktes/gitignoriertes Ziel existiert lokal, **fehlt aber auf
jedem frischen Klon** (`target-missing`). Das d-check-Modul `tracked` (v0.37.0) macht das
computational (`target-untracked`; **kein** `tracked-check`-Skript im Repo — erstmals gegatet).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-05.

**Schnitt-Herkunft:** Modul-Adoptions-Steering (Roadmap §Harness-Gate-Fahrplan). `tracked` ist der
vierte Modul-Gewinner nach `commits`/`vcs`/`planning`. **Besonderheit: `tracked` braucht `.git`**
(git-Index, read-only), **aber keine Range** → **Gate-Member** (Kriterium **range-frei**, `.git` liegt im
`make gates`-Bind-Mount) — der **erste** Gate-Member, der `.git` liest (anders als `planning`). **Der einfachste Adoptions-Slice** (keine Toggle-Pflicht, nur ein
`modules:`-Eintrag). **Reine Doku-/Tooling-Entscheidung, kein Produktions-Code.**

---

## 1. Ziel

Die Fresh-Clone-Integrität der Doku-Ziele wird von (bisher un-formulierter) Konvention zu **Gate**:
`tracked` prüft gegen den **git-Index** (gestagt = getrackt, keine `.gitignore`-Interpretation),
dass jedes **existierende** Link-/Bild-Ziel getrackt ist (sonst `target-untracked`). Fehlende Ziele
bleiben Sache von `links` (kein Doppelbefund); Verzeichnis-/Symlink-Ziele prüft `tracked` nicht.
`tracked` wird ins **`modules:`-Set** aufgenommen → es läuft in **`make docs-check` = `make gates`**
(`.git` liegt im read-only Bind-Mount; kein separater CI-Sensor).

**Evidenz (vor dem Plan gemessen):**
- **Baseline** (`make doc-tracked`) → **0**: b-cad hat **kein** untracktes Link-Ziel — der Ist-Zustand
  ist konsistent.
- **Positiv** — ein Wegwerf-Test-Doc mit Link auf ein **untracktes** Ziel →
  **`target-untracked`**: Enforcement greift (Test-Dateien revertiert, Baum sauber).

## 2. Definition of Done

- [x] **`.d-check.yml` `tracked`-Block** (`exempt-targets: []` — **explizit** [Muster slice-036-L2];
      `build/**` bliebe das Ventil für lokal-generierte Ziele, heute kein `build/`-Dir → leer).
      *(Bereits als Messschritt vollzogen.)*
- [x] **`tracked` in das `modules:`-Set** aufnehmen → **`make gates`-Member** (`.git` im Bind-Mount;
      **kein Range** — das Gate-Member-Kriterium ist **„range-frei", nicht „git-frei"** [Review-M1]).
      Ist-Zustand konsistent (Baseline 0) → `make docs-check` bleibt grün. **Keine Toggle-/Wartungspflicht**
      (anders als `planning`). **CI-Vakuität = die eigentliche Gate-Member-Rechtfertigung:** auf
      `index == HEAD` (frischer CI-Checkout) ist alles getrackt → `tracked` in CI **immer 0**; sein Wert
      liegt **nur** im lokalen `make gates` auf dem schmutzigen Arbeitsbaum → CI-only wäre nutzlos-grün.
- [x] **`harness/conventions.md` neuer Eintrag [MR-018](../../../../harness/conventions.md)**
      (Verschärfung/Prozess-Gate, kein [§2.6](../../../../AGENTS.md)-Lockerungsfall; Muster
      [MR-017](../../../../harness/conventions.md)): dokumentiert (1) die `tracked`-Adoption + Config,
      (2) **Gate-Member-Kriterium = „range-frei" (nicht „git-frei")** [Review-M1]: `tracked` ist der
      **erste Gate-Member, der `.git` liest** (Index, range-frei), fail-closed ohne `.git` — unkritisch,
      da `docs-check` `.git` mountet (Ein-Zeilen-Notiz, Review-L2); **CI-Vakuität** = die Rechtfertigung
      (in CI `index==HEAD` → immer 0, Wert nur lokal); **Index-Semantik** gestagt = getrackt [Review-L1];
      (3) `exempt-targets`-Ventil, (4) Abgrenzung zu `links` (fehlend = `target-missing`) **und zum
      „frischer Klon"-Homonym** ([MR-005](../../../../harness/conventions.md) = Nachweis-Hash über Datei-
      *Inhalte*; `tracked` = Index-Mitgliedschaft der Referenz-**Ziele**) [Review-L3]; die Symlink-
      Auslassung als „aus d-check-Doku, nicht b-cad-verifiziert" kennzeichnen [Review-L4].
- [x] **Gate-Doku:** [`harness/README.md` §Sensors](../../../../harness/README.md) + [AGENTS.md §3](../../../../AGENTS.md)
      — die `make docs-check`-Modul-Liste um `tracked` (Fresh-Clone-Schutz, git-Index).
- [x] **Roadmap-Fahrplan-Nachzug:** [`roadmap.md`](../in-progress/roadmap.md) §Harness-Gate-Fahrplan
      `tracked` → **adoptiert (slice-037, Gate-Member)** + Quergewerk-Zeile §Historische Trigger-
      Verschiebungen (Muster slice-036). Verbleibt im Fahrplan nur `--trace` (Report, kein Gate).
- [x] **`CHANGELOG.md`** (grobkörnig, [MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- [x] **`make gates` grün** (jetzt **inklusive** `tracked` — Baseline 0 belegt); `make doc-tracked`
      = 0 + Positiv-Test belegt.
- [x] **Nicht Teil:** die DRW-Arbeit; `--trace` (Report-only, kein Gate) — eigener/optionaler Schritt.

## 3. Plan (vor Ausführung)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.d-check.yml` | ändern | `tracked`-Block (`exempt-targets: []`) **+** `tracked` in `modules:` (Gate-Member) |
| `harness/README.md` §Sensors + [AGENTS.md §3](../../../../AGENTS.md) | ändern | `docs-check`-Modul-Liste um `tracked` |
| `harness/conventions.md` | ändern | neuer Eintrag **[MR-018](../../../../harness/conventions.md)** (tracked-Adoption, Gate-Member) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Fahrplan `tracked`→adoptiert; Reihenfolge; Quergewerk-Eintrag |
| `CHANGELOG.md` | ändern | `[Unreleased]`-Eintrag |
| `docs/reviews/{2026-07-05-slice-037-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review durchlaufen** (0 HIGH / 1 MED / 3 LOW; M1/L1–L4 eingearbeitet) → **startbar.**
  `tracked`-Modul + `doc-tracked`-Target sind mit slice-033 (d-check v0.37.1) real; Invariante
  evidenzbasiert (Baseline 0 / Positiv `target-untracked`).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (inkl. `tracked`), Closure-Notiz → der Harness-Gate-Fahrplan ist
  **bis auf `--trace`** (Report-only, kein Gate) abgearbeitet; die vier Gate-Gewinner
  (`commits`/`vcs`/`planning`/`tracked`) sind live.

## 6. Risiken und offene Punkte

- **Gate-Member-Kriterium „range-frei" (Review-M1):** `tracked` liest den git-Index read-only, **ohne
  Range** — das ist das Gate-Kriterium, **nicht** „git-frei" (so darf [MR-017](../../../../harness/conventions.md) für `planning` nicht gelesen
  werden). `tracked` ist der **erste Gate-Member, der `.git` liest**; der `make gates`-Bind-Mount
  (`-v $(CURDIR):/repo:ro`) enthält `.git`, also gate-fähig. **fail-closed ohne `.git`** — im Container-
  Mount unkritisch, aber erster harter `.git`-abhängiger Member (Ein-Zeilen-Notiz, Review-L2). **Keine
  Toggle-Pflicht** (anders als `planning`).
- **CI-Vakuität — warum Gate-Member, nicht CI-only (Review-Schlüsselbefund):** auf `index == HEAD`
  (frischer CI-Checkout) ist jede vorhandene Datei getrackt → `tracked` in **CI immer 0**. Sein Wert liegt
  **ausschließlich** im lokalen `make gates` auf dem schmutzigen Arbeitsbaum (Pre-Handoff-Wächter): CI-only
  (wie commits/vcs) wäre nutzlos-grün. Das löst zugleich jede Commit-Reihenfolge-Sorge (die „untracked-
  existing"-Bedingung tritt auf einem frischen Commit-Checkout nicht auf; Split-Commit fängt `links`
  `target-missing`, nicht `tracked`).
- **`exempt-targets`-Ventil:** heute leer (kein `build/`-Dir, Baseline 0). Referenziert die Doku künftig
  ein **absichtlich** untracktes/generiertes Ziel (z. B. `build/**`), wird es dort eingetragen (bewusster
  Akt) — sonst `target-untracked`. Kein stiller Bypass.
- **Abgrenzung (kein Doppelbefund / kein Konzept-Clash):** fehlende Ziele bleiben `links`
  (`target-missing`); `tracked` ergänzt nur den *getrackt*-Fall. Der Working-Tree-Hash
  ([MR-005](../../../../harness/conventions.md)) ist ein **anderes** Konzept (Gates-Nachweis über
  Datei-Inhalte tracked+untracked) — keine Regel-Kollision (H1-Sweep bestätigt: neue Regel, eine Heimat).
- **Kein Scope-Creep:** `--trace` (Report) und die DRW-Arbeit bleiben unberührt. `tracked` ändert
  **kein** Produktions-Code, **kein** Schema, **keine** LH-Anforderung.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Konventionen & Harness-Doku

- **Modus:** GF; **Dichte:** hoch — `MR-NNN`, Verschärfungs-Disziplin ([AGENTS.md §2.6](../../../../AGENTS.md)).
  **Phase-Reife:** Phase 4. **Risiko:** niedrig (Baseline 0, Enforcement belegt).

### Sub-Area: Build & Toolchain

- **Modus:** GF; **Dichte:** hoch — `.d-check.yml`-`modules:`-Set + d-check.mk ([MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-014](../../../../harness/conventions.md));
  zweite Gate-Member-Aufnahme über den Fahrplan (nach `planning`). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Ausgeführt 2026-07-05** — DoD vollständig; `make gates` grün **inklusive** des neuen `tracked`-Moduls
(Ist-Zustand konsistent: keine untrackten Link-Ziele), `make doc-tracked` = 0.

**Beobachtbare Closure-Kriterien:**
- d-check-Modul `tracked` **im `.d-check.yml`-`modules:`-Set** → **zweiter Fahrplan-`make gates`-Member**
  (nach `planning`; **erster `.git`-lesender** Member, range-frei).
- Regel dokumentiert ([MR-018](../../../../harness/conventions.md)): Kriterium **range-frei** (nicht
  git-frei), **CI-Vakuität** als Rechtfertigung, Index-Semantik, `exempt-targets`-Ventil, Abgrenzung zu
  `links` + zum „frischer Klon"-Homonym.
- Gate-Doku ([AGENTS §3](../../../../AGENTS.md) + [`harness/README.md` §Sensors](../../../../harness/README.md)).

**Lerneintrag:** Kein HIGH — der sauberste der vier Adoptions-Slices. Schlüsselbefund des Reviews: die
**CI-Vakuität** — ein Modul, das nur den *schmutzigen Arbeitsbaum* prüft (in CI immer grün), gehört
**lokal ins Gate**, nicht in die CI-Liste — die Umkehrung der `commits`/`vcs`-Logik. Zweite Lehre (M1):
das **Gate-Member-Kriterium ist „range-frei", nicht „git-frei"** — die zwei immutablen [MR-017](../../../../harness/conventions.md)/[MR-018](../../../../harness/conventions.md)-
Einträge müssen dasselbe Kriterium tragen, sonst latenter Normen-Drift.

**Folge:** die **vier Gate-Gewinner** (`commits`/`vcs`/`planning`/`tracked`) sind live; der Harness-Gate-
Fahrplan ist **bis auf `--trace`** (Report-only, kein Gate) abgearbeitet.
