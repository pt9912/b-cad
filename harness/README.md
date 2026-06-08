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

**Bootstrap-Stand:** b-cad hat den **Greenfield-Bootstrap** (Kurs-Modul
2) durchlaufen und ist **bereit für erste Code-Slices**. Es existiert
Doku (Spec, ADRs, Roadmap) und **genau ein reales Gate** (`docs-check`,
Doku-Validator), aber **noch kein Anwendungscode**. Code-Gates stehen
daher im „Nicht behauptet"-Block (§Sensors) und werden mit dem ersten
Code-Slice gepromotet.

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

(Drei-Schichten-Spec-Precedence siehe [`conventions.md` MR-001](conventions.md#mr-001-source-precedence-mit-eigener-spezifikations-schicht).)

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

## Sensors (Feedback-Gates)

**Real existierend** (im Makefile, läuft im Container):

| Target | Vertrag | Bindung |
|---|---|---|
| `make docs-check` | interne Markdown-Links, Anker und ID-Pfade konsistent; kein Pfad führt aus dem Repo | [`MR-003`](conventions.md#mr-003-docs-check-als-vendored-doku-sensor) |
| `make gates` | Aggregat — **derzeit nur** `docs-check` | — |

**Nicht behauptet (geplant, entstehen mit dem ersten Code-Slice —
Promotion-Trigger).** Sobald ein Target real im Makefile steht, wandert
es mit Vertrag und Bindung in die obige Tabelle:

- `make build` — Container-Build, erzwingt CMake-Target-Trennung (Kern
  ohne Adapter-/Qt-/OCC-/SQLite-Deps). Bindung: ADR-0001.
- `make arch-check` — hexagonale Layering-Constraints. Bindung: ADR-0001/0002/0003.
- `make lint` — clang-tidy + Suppression-Gate.
- `make test` — GoogleTest inkl. Crash-Recovery (LH-QA-005) und
  Determinismus. Bindung (LH-Klasse, siehe
  [`conventions.md` §Zusatzklassen](conventions.md#zusatzklassen-deklaration-für-sensors-bindung)): LH-QA-005, LH-FA-WAL-002.
- `make coverage-gate` / `make coverage-gate-critical` — Coverage, bootstrap-aware (Critical: Persistenz/Recovery).
- `make ci` / `make fullbuild` — weitere Aggregat-Gates.

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
5. Engsten nützlichen Sensor laufen lassen (sobald Gates existieren).
6. Repo-weiten Gate-Lauf vor Handoff (`make gates`, sobald vorhanden).
7. Doku/Indizes aktualisieren, falls ein öffentlicher Vertrag berührt.
8. Ausgeführte Sensors und verbleibende Risiken berichten — keine
   Erfolgsmeldung ohne Gate-Ausführung.
