---
id: slice-036
titel: Planning-Lifecycle-Gate — d-check-Modul `planning` (Roadmap↔in-progress, erster Gate-Member)
status: done
welle: welle-5-erweiterung (Quergewerk harness-steering)
lastenheft_refs: []  # reines Prozess-/Gate-Steering, keine LH-Anforderung
adr_refs: []         # kein ADR (Verschärfung/Prozess-Gate, §2.6 n/a)
---

# Slice 036: Planning-Lifecycle-Gate — Modul `planning`

**Status:** **done** (ausgeführt 2026-07-05; angelegt evidence-first, Muster
[slice-035](../done/slice-035-vcs-adr-immutability.md)).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
0 HIGH / 2 MED / 3 LOW → startbar** (Gate-Member-Entscheidung als sound bestätigt; M1/M2
eingearbeitet; [Report](../../../reviews/2026-07-05-slice-036-plan.md)).

**Welle:** welle-5-erweiterung — **Quergewerk `harness-steering`** (kein Wellen-Feature;
Muster [slice-034](../done/slice-034-commits-traceability.md)/[slice-035](../done/slice-035-vcs-adr-immutability.md)).
**DRW-Scope (032a/b/c) unberührt** — pausiert.

**Bezug:** eine **neue** Planning-Lifecycle-Invariante (heute nirgends geregelt — repo-weiter
Sweep bestätigt): der **Ruhe-Marker** im `## Aktuelle Welle`-Block der Roadmap steht **genau
dann**, wenn **kein** `slice-*` in `in-progress/` liegt. Das d-check-Modul `planning` (v0.36)
macht sie **computational** (`planning-drift`; **kein** `planning-check`-Skript im Repo — erstmals
gegatet; das Modul löst das a-check-`planning-check`-Muster ab, das *letzte* Familien-Skript).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-05.

**Schnitt-Herkunft:** Modul-Adoptions-Steering (Roadmap §Harness-Gate-Fahrplan). `planning` ist
der dritte Modul-Gewinner nach `commits`/`vcs`. **Besonderheit: `planning` ist HERMETISCH**
(nur Arbeitsbaum-Listing + Roadmap, kein git) → **erstes über den Harness-Gate-Fahrplan
adoptiertes Modul, das ins `make gates`-Aggregat wandert** (erste Modul-Aufnahme seit der
`ids`/`matrix`-Etablierung; statt CI-only wie commits/vcs). **Reine Doku-/Tooling-Entscheidung,
kein Produktions-Code.**

---

## 1. Ziel

Die Roadmap-↔-`in-progress`-Konsistenz wird von (bisher un-formulierter) Konvention zu **Gate**:
`planning` prüft **hermetisch**, dass der Ruhe-Marker im `## Aktuelle Welle`-Block **genau dann**
präsent ist, wenn `in-progress/` **kein** `slice-*` enthält (sonst `planning-drift`). Weil
hermetisch, wird `planning` in das **`modules:`-Set** aufgenommen → es läuft in **`make docs-check`
= `make gates`** (nicht als separater CI-Sensor).

**Evidenz (vor dem Plan gemessen):**
- **Baseline** (`make doc-planning`, slice-032a aktiv, **kein** Marker) → **0**: der Ist-Zustand
  ist bereits konsistent (aktives Slice ⇒ Marker abwesend).
- **Positiv** — Ruhe-Marker in `## Aktuelle Welle` eingefügt, obwohl slice-032a aktiv → **`planning-drift`**
  (`roadmap.md:12`): Enforcement greift.

## 2. Definition of Done

- [x] **`.d-check.yml` `planning`-Block** (`roadmap: docs/plan/planning/in-progress/roadmap.md`,
      `marker: "Keine offenen Slices"` [reservierter Sentinel]; `heading` **+** `slice-glob` explizit `## Aktuelle Welle`/`slice-*` [Review-L2],
      passen). *(Bereits als Messschritt vollzogen.)*
- [x] **`planning` in das `modules:`-Set** aufnehmen → **`make gates`-Member** (nicht CI-only). Der
      Ist-Zustand ist konsistent (slice-032a aktiv ⇒ kein Marker) → `make docs-check` bleibt grün.
      **Entscheidung gegen den CI-only-Sensor** (`make doc-immutable`-Muster): `planning` ist
      hermetisch, gehört also ins Gate (Muster `ids`/`matrix`); ein separater Sensor verschenkte die
      lokale Blockade. (Alternative in §6 benannt.)
- [x] **Ruhe-Marker-Konvention dokumentieren** — **neue** Regel, **eine** Heimat
      ([`planning/README.md`](../README.md) Lifecycle-Abschnitt): der `## Aktuelle Welle`-Block trägt
      den **reservierten Sentinel „Keine offenen Slices"** (exakt-case, **nie** als Prosa — [Review-L3])
      genau dann, wenn `in-progress/` keinen `slice-*`-Plan enthält; **Toggle-Pflicht** — beim **Öffnen
      des ersten** Slice entfernen, beim **Schließen des letzten** setzen. **Commit-Sequenz (Review-M1):**
      der Marker-Toggle **reitet im selben Commit wie der `git mv`** (die `roadmap.md`-Edit senkt die
      per-Datei-Rename-Similarität des Slice **nicht** → AGENTS §2.8 gewahrt) → **kein** transienter
      `planning-drift`-Snapshot. Erklär-Kommentar (Sentinel reserviert) im Roadmap-`## Aktuelle Welle`-Block.
- [x] **`harness/conventions.md` neuer Eintrag [MR-017](../../../../harness/conventions.md)**
      (Verschärfung/Prozess-Gate, kein [§2.6](../../../../AGENTS.md)-Lockerungsfall; Muster
      [MR-016](../../../../harness/conventions.md)): dokumentiert (1) die `planning`-Adoption + Config,
      (2) **Gate-Member-Status** (hermetisch → `modules:`; **erstes über den Harness-Gate-Fahrplan
      adoptiertes Gate-Modul** [Review-L1], Kontrast zu den CI-only `commits`/`vcs`), (3) die Ruhe-Marker-
      Toggle-Pflicht **+ Commit-Sequenz** (Marker reitet im `git mv`-Commit), (4) tool-native
      Ablösung des `planning-check`-Skript-Musters (a-check, letztes Familien-Skript).
- [x] **Gate-Doku:** [`harness/README.md` §Sensors](../../../../harness/README.md) + [AGENTS.md §3](../../../../AGENTS.md)
      — die `make docs-check`-Zeile nennt `planning` als neues Modul (Roadmap↔in-progress, hermetisch);
      **kein** separater `doc-planning`-Sensor-Eintrag (es ist Teil von `docs-check`/`gates`).
- [x] **[AGENTS.md §2.8](../../../../AGENTS.md)-Toggle-Referenz (Review-M2):** die Hard-Rule-Heimat der
      Lifecycle-Bewegung (`git mv`) erhält eine Ein-Zeilen-Referenz auf die Ruhe-Marker-Toggle-Pflicht
      (Marker reitet im Move-Commit des ersten/letzten Slice) — sonst läuft ein schließender Agent, der
      nur §2.8 liest, blind in den `planning-drift` (bei einem **Gate-Member** verschärft).
- [x] **Roadmap-Fahrplan-Nachzug:** [`roadmap.md`](../in-progress/roadmap.md) §Harness-Gate-Fahrplan
      `planning` → **adoptiert (slice-036, Gate-Member)** + Quergewerk-Zeile §Historische Trigger-
      Verschiebungen (Muster slice-035).
- [x] **`CHANGELOG.md`** (grobkörnig, [MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- [x] **`make gates` grün** (jetzt **inklusive** `planning` — Ist-Zustand konsistent belegt);
      `make doc-planning` = 0 + Positiv-Test belegt.
- [x] **Nicht Teil:** die DRW-Arbeit; die verbleibenden Modul-Kandidaten `tracked` + `--trace` —
      eigene Folge-Slices.

## 3. Plan (vor Ausführung)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.d-check.yml` | ändern | `planning`-Block **+** `planning` in `modules:` (Gate-Member) |
| [`planning/README.md`](../README.md) | ändern | Ruhe-Marker-Konvention + Toggle-Pflicht (neue Regel, eine Heimat) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | `## Aktuelle Welle` Erklär-Kommentar; Fahrplan `planning`→adoptiert; Quergewerk-Eintrag |
| `harness/README.md` §Sensors + [AGENTS.md §3](../../../../AGENTS.md) | ändern | `docs-check`-Modul-Liste um `planning` |
| `harness/conventions.md` | ändern | neuer Eintrag **[MR-017](../../../../harness/conventions.md)** (planning-Adoption, Gate-Member) |
| `CHANGELOG.md` | ändern | `[Unreleased]`-Eintrag |
| `docs/reviews/{2026-07-05-slice-036-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review durchlaufen** (0 HIGH / 2 MED / 3 LOW; M1/M2 eingearbeitet) → **startbar.**
  Das `planning`-Modul + `doc-planning`-Target sind mit slice-033 (d-check v0.37.1) real; die
  Invariante evidenzbasiert validiert (Baseline/Positiv).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (inkl. `planning`), Closure-Notiz → die verbleibenden
  Kandidaten (`tracked`, `--trace`) werden startbar; die drei „Regel→computational"-Gewinner
  (`commits`/`vcs`/`planning`) sind dann live.

## 6. Risiken und offene Punkte

- **Gate-Member vs. CI-only-Sensor (zentrale Entscheidung):** `planning` ist hermetisch → es
  **kann** ins `make gates`-Aggregat (via `modules:`), anders als die nicht-hermetischen `commits`/`vcs`
  (CI-only). **Gewählt: Gate-Member** (lokale Blockade vor Merge, Muster `ids`/`matrix`). **Preis:**
  eine **Toggle-Pflicht** — schließt der *letzte* `in-progress`-Slice, muss der Ruhe-Marker gesetzt
  werden (sonst `planning-drift` → `gates` rot); öffnet der *erste*, entfernt. Das erweitert den
  Slice-Lifecycle um einen Handgriff (an [AGENTS §2.8](../../../../AGENTS.md) referenziert, Review-M2).
  **Alternative** (CI-only `doc-planning`) wäre nachsichtiger, verschenkt aber die Hermetik-Stärke —
  **das Review bestätigte Gate-Member als sound** (Failure-Mode transient/selbstheilend).
- **Transiente Ruhe⇄Aktiv-Drift + §2.8-Kopplung (Review-M1):** unter der Zwei-Commit-Regel stünde der
  Baum zwischen `git mv` und Marker-Nachzug kurz in `planning-drift` (empirisch S1/S3) — per-Commit-CI
  röte den Move-Commit. **Auflösung:** der Marker-Toggle **reitet im selben Commit wie der `git mv`**
  (Rename-Detection ist per-Datei; eine `roadmap.md`-Edit senkt die Rename-Similarität des Slice nicht →
  §2.8 gewahrt) → kein transienter Snapshot; per-Commit-CI würde `planning` zudem am PR-finalen HEAD werten.
- **Wellen- vs. Slice-Granularität (Marker-Text):** der Default-Marker „Keine aktive **Welle**" passt
  nicht — b-cads Welle (z. B. welle-5) kann **aktiv** sein, während **kein Slice** offen ist (zwischen
  Slices). Der Marker signalisiert **Slice**-Ruhe, nicht Wellen-Ende → gewählt **„Keine offenen
  Slices"**. **Review-L3:** der Match ist ein case-sensitiver Substring im AW-Block → der Satz ist als
  **reservierter Sentinel** deklariert (Config-Kommentar + `planning/README.md` + Erklär-Kommentar;
  exakt-case, nie als Prosa) — eine Prosa-Kollision wäre fail-safe (rot), ist damit aber ausgeschlossen.
- **Ist-Zustand konsistent (kein Remediations-Aufwand):** slice-032a aktiv ⇒ Marker abwesend ⇒
  `planning` grün ab Aufnahme ins Gate; kein Nachzieh-Bedarf am Bestand (anders denkbar gewesen).
- **H1-Lehre (verschärft, slice-035) angewandt:** repo-weiter Sweep — die Ruhe-Marker-Regel existiert
  **nirgends** vorab (neue Konvention), also **kein** Multi-Home-Abgleich; die Regel bekommt **eine**
  Heimat (`planning/README.md`), von Roadmap-Kommentar + [MR-017](../../../../harness/conventions.md) referenziert.
- **Kein Scope-Creep:** `tracked`/`--trace` und die DRW-Arbeit bleiben unberührt. `planning` ändert
  **kein** Produktions-Code, **kein** Schema, **keine** LH-Anforderung.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Konventionen & Harness-Doku

- **Modus:** GF; **Dichte:** hoch — `MR-NNN`, Verschärfungs-Disziplin ([AGENTS.md §2.6](../../../../AGENTS.md)),
  Slice-Lifecycle (§2.8). **Phase-Reife:** Phase 4. **Risiko:** niedrig (Ist-Zustand konsistent, Enforcement belegt).

### Sub-Area: Build & Toolchain

- **Modus:** GF; **Dichte:** hoch — `.d-check.yml`-`modules:`-Set + d-check.mk ([MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-014](../../../../harness/conventions.md));
  **erste** Modul-Aufnahme ins Gate seit der `ids`/`matrix`-Etablierung. **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Ausgeführt 2026-07-05** — DoD vollständig; `make gates` grün **inklusive** des neuen `planning`-Moduls
(Ist-Zustand konsistent: slice-032a + slice-036 aktiv ⇒ Sentinel abwesend), `make doc-planning` = 0.

**Beobachtbare Closure-Kriterien:**
- d-check-Modul `planning` **im `.d-check.yml`-`modules:`-Set** → **erster über den Harness-Gate-Fahrplan
  adoptierter `make gates`-Member** (hermetisch; Kontrast zu den CI-only `commits`/`vcs`).
- Ruhe-Marker-Invariante dokumentiert (**eine** Heimat [`planning/README.md`](../README.md) §Ruhe-Marker)
  + Toggle-Pflicht an [AGENTS §2.8](../../../../AGENTS.md) referenziert (Review-M2); der Sentinel ist
  reserviert; [MR-017](../../../../harness/conventions.md) verankert.
- Commit-Sequenz gelöst (Review-M1): der Marker-Toggle reitet **im `git mv`-Commit** → kein transienter Drift.

**Lerneintrag:** Diesmal **kein HIGH** — der repo-weite H1-Sweep (slice-035-Lehre) trug, die Ruhe-Marker-
Regel war echt neu (eine Heimat). Das Review verlagerte den Fokus auf **Gate-Wechselwirkungen** (die
slice-035-M1-Klasse): ein neuer hermetischer Gate-Member koppelt an die §2.8-Zwei-Commit-Regel — Auflösung
über „Toggle reitet im Move-Commit" (Rename-Detection ist per-Datei). Lehre: **bei einem Gate-Member die
Commit-Sequenz + Lifecycle-Wechselwirkung mitdenken**, nicht nur die Einzel-Invariante.

**Folge:** die drei „Regel→computational"-Gewinner (`commits`/`vcs`/`planning`) sind live; verbleibend
`tracked` + `--trace`.
