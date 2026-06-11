# Harness — b-cad

## Purpose

Dieser Harness verbindet Spezifikationen, ADRs, Planning-Dokumente und
(künftige) Gates von **b-cad**. Er ist **kein Ersatz** für `spec/` oder
`docs/`, sondern ein **Einstiegspunkt** für Menschen und
AI-Code-Agenten.

Wenn diese Datei einer kanonischen Quelle widerspricht, gewinnt die
kanonische Quelle und diese Datei wird angepasst.

Strukturregeln (Verzeichniskonvention, ID-Schema, Modus-Deklaration pro
Sub-Area, Zusatzklassen für Sensors-Bindung) sowie Adaptionen ggü. der
Baseline leben in [`conventions.md`](conventions.md). Diese Datei
dupliziert sie nicht.

**Stand:** Greenfield-Bootstrap (Kurs-Modul 2) abgeschlossen; **slice-001**
(Build-Skelett & DevContainer) und **slice-002** (Code-Gates) sind
umgesetzt und verifiziert. **Reale Gates** (jeweils Dockerfile-Target-
Stage, keine Bind-Mounts): `make docs-check`, `make gate-consistency`,
`make arch-check`, `make lint`, `make test`, `make coverage-gate`,
`make build` — aggregiert in `make gates`. Details und Verträge:
§Sensors. Geplant (noch nicht behauptet): `coverage-gate-critical`,
`ci`, `fullbuild`.

## Source precedence

| Rang | Datei | Charakter |
|---|---|---|
| 1 | [`../spec/lastenheft.md`](../spec/lastenheft.md) | vertraglich abnahmebindend |
| 2 | [`../spec/spezifikation.md`](../spec/spezifikation.md) | technisch fortschreibbar |
| 3 | [`../spec/architecture.md`](../spec/architecture.md) | Komponenten/Schichten, meilensteinfrei |
| 4 | [`../docs/plan/adr/`](../docs/plan/adr/) | Architekturentscheidungen |
| 5 | [`../docs/plan/planning/in-progress/roadmap.md`](../docs/plan/planning/in-progress/roadmap.md) | aktuelle Welle |
| 6 | [`../docs/user/`](../docs/user/) | Operations, Releasing |
| 7 | [`../README.md`](../README.md) | Projekt-Überblick |
| 8 | [`../AGENTS.md`](../AGENTS.md) | Agent-Briefing |
| 9 | diese Datei | Harness-Einstieg |

(Drei-Schichten-Spec-Precedence siehe [`conventions.md` MR-001](conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht).)

## Guides (Feedforward-Quellen)

| Quelle | Inhalt |
|---|---|
| [`../spec/lastenheft.md`](../spec/lastenheft.md) | `LH-FA-*`/`LH-QA-*` mit Akzeptanzkriterien |
| [`../spec/spezifikation.md`](../spec/spezifikation.md) | Wertebereiche, Fehler-Codes, OTel-Spans |
| [`../spec/architecture.md`](../spec/architecture.md) | hexagonale Schichten, Ports, Constraints |
| [`../docs/plan/adr/`](../docs/plan/adr/) | ADR-0001 (Hexagonal), 0002 (OCC), 0003 (SQLite) |
| [`../docs/plan/planning/`](../docs/plan/planning/) | Slices und Roadmap |
| [`../AGENTS.md`](../AGENTS.md) | Hard Rules, Workflow |
| [`conventions.md`](conventions.md) | Strukturregeln, `MR-*`-Adaptionen, Modus-Deklaration |
| [Agents-Digest des Kurses](https://raw.githubusercontent.com/pt9912/ai-harness-course/main/kurs/de/agents-digest.md) | adoptiertes Betriebsregelwerk in Agenten-Kurzform; derivativ, Stand siehe [`conventions.md`](conventions.md) §Baseline |

## Sensors (Feedback-Gates)

**Real existierend.** Jeder Gate ist eine **Dockerfile-Target-Stage**
(`docker build --target …`, Quelle per `COPY` eingebacken) — **keine
Bind-Mounts**, maximal reproduzierbar (Modul 14, Vorbild cmake-xray):

| Target | Vertrag | Bindung |
|---|---|---|
| `make docs-check` | interne Markdown-Links, Anker und ID-Pfade konsistent; kein Pfad führt aus dem Repo | [`MR-003`](conventions.md#mr-003--docs-check-als-vendored-doku-sensor) |
| `make gate-consistency` | jeder als real dokumentierte `make`-Befehl (AGENTS §3 / §Sensors) existiert im Makefile — fängt halluzinierte Gates | Modul 13 |
| `make arch-check` | hexagonale Schichtung: Kern importiert kein Qt/OCC/SQLite/`adapters/`; kein Adapter importiert einen anderen; OCC-`.hxx` nur in `adapters/geometry/` (Regel C); `sqlite3*` nur in `adapters/persistence/` (Regel D) | ADR-0001, ADR-0002, ADR-0003 |
| `make lint` | clang-tidy (0 Befunde in `src/`) + Suppression-Gate | ADR-0001 §Fitness (AGENTS.md §2.4) |
| `make test` | GoogleTest-Suite: prüft Kern-Logik + echte Adapter-Linkage (Qt/OCC/SQLite) | — |
| `make coverage-gate` | Line-Coverage ≥ Schwelle (bootstrap-aware, Composition Root ausgenommen) | Schwelle 70 %, Ramp → M2 (siehe AGENTS.md §3) |
| `make build` | Target-Kette kompiliert; erzwingt CMake-Target-Trennung (Kern ohne Adapter-Deps) | ADR-0001 |
| `make gates` | Aggregat: docs-check · gate-consistency · arch-check · lint · test · coverage-gate (+ `record-gates`-Nachweis) | — |
| `make schema-check` | ADR-0006-Drift: committete `schema.sql` == d-migrate(`data-model.yaml`). **Nicht** in `make gates` (d-migrate aus dem hermetischen Gate-Pfad gehalten) — gehört in die **CI-Befehlsliste** | ADR-0006 |

**Warum `build` nicht in `gates`:** `test`, `lint` und `coverage-gate`
sind Dockerfile-Stages `FROM build` und **kompilieren die Target-Kette
bereits** (inkl. der Target-Trennung). `make build` ist daher ein
eigenständiger Einzel-Sensor (reines Kompilieren ohne Tests), nicht
separat in `gates` — ein Aufnehmen wäre redundanter Rebuild.

**Gate-Nachweis (Stop-Hook):** `make gates` schreibt nach Erfolg
`.harness/state/gates-passed.diffsha` — einen **Hash des gesamten
Arbeitsbaum-Zustands** (tracked-modified, staged, **untracked**,
gelöscht, umbenannt) via [`tools/harness/working-tree-hash.sh`](../tools/harness/working-tree-hash.sh).
`record-gates` und der Stop-Hook (`.claude/hooks/stop-require-gates.sh`)
teilen dieselbe Hash-Funktion (keine Logik-Dopplung). Der Stop-Hook gibt
nur frei, wenn der Arbeitsbaum sauber ist (`git status --porcelain` leer)
**oder** der Hash zum aktuellen Zustand passt. `.harness/state/` ist
Laufzeit-State (gitignored). *Hinweis:* lokale Arbeitsbremse, kein
Ersatz für `make gates` in CI (letzte Instanz).

**Nicht behauptet (geplant).** Sobald real, wandern sie mit Vertrag und
Bindung in die obige Tabelle:

- `make coverage-gate-critical` — Critical-Path-Coverage (Persistenz/Recovery, LH-QA-005), höhere Schwelle. Bindung: LH-QA-005.
- `make ci` / `make fullbuild` — weitere Aggregat-/Closure-Gates (inkl. Image-Hash, Modul 14).

**Aktueller Lauf-Status:** wird hier **nicht** geführt — Lauf-Wahrheit
pro Commit gehört in CI, nicht in diese Datei (Rang 9; Kurs-Modul 13).
**Rote Gates:** würden als `CO-<NNN>` in
[`../docs/plan/carveouts/`](../docs/plan/carveouts/) dokumentiert (Modul 7).

## Traceability rules

- PRs/Commits **müssen** mindestens eine `LH-*`- oder `ADR-*`-ID nennen.
- Neue Anforderungen brauchen Beleg: Test (mit ID im Namen), Gate, Demo
  oder ADR.
- Neue ADRs müssen [`../docs/plan/adr/README.md`](../docs/plan/adr/README.md) aktualisieren.
- Slice-Lifecycle-Bewegung (`open → next → in-progress → done`) ist
  reiner `git mv` (siehe [`../AGENTS.md` §2.8](../AGENTS.md)).

## Safety and scope boundaries

- **Datenverlust am Gebäudemodell ist der schärfste Fehlerfall.**
  Persistenz schreibt atomar (Temp + Rename); Crash-Recovery erhält den
  letzten konsistenten Stand (LH-QA-005, LH-FA-BLD-002).
- **Kein OCC/Qt/SQLite im Kern.** Geometrie-, GUI- und DB-Technologie
  leben ausschließlich in Adaptern (ADR-0001).
- **Plugins laufen in einer Sandbox** und dürfen das Modell nicht
  umgehen oder korruptieren (LH-FA-PLG-004).
- b-cad ist **keine Statik-/Tragwerksberechnung** — „Tragwand" ist eine
  Klassifikation, keine Bemessung (Lastenheft §6).

## Minimal agent workflow

1. Diese Datei lesen.
2. Relevante kanonische Quelle lesen (Source Precedence beachten).
3. Betroffene IDs identifizieren.
4. Kleinste Änderung planen.
5. Engsten nützlichen Sensor laufen lassen.
6. Repo-weiten Gate-Lauf vor Handoff (`make gates`).
7. Doku/Indizes aktualisieren, falls ein öffentlicher Vertrag berührt.
8. Ausgeführte Sensors und verbleibende Risiken berichten — keine
   Erfolgsmeldung ohne Gate-Ausführung.
