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
umgesetzt und verifiziert. **Reale Gates** (überwiegend Dockerfile-Target-
Stages, keine Bind-Mounts): `make docs-check`, `make gate-consistency`,
`make a-check`, `make arch-check`, `make lint`, `make test`,
`make coverage-gate`, `make build` — aggregiert in `make gates`.
**Ausnahmen `make a-check`** (slice-030 / [MR-013](conventions.md#mr-013--arch-check-via-a-check))
und **`make docs-check`** (slice-033 / [MR-014](conventions.md)): externe,
digest-gepinnte Images (netzloser read-only Bind-Mount statt COPY-Stage,
Definition im jeweiligen `.mk`) — die zwei Bind-Mount-Member im Aggregat.
Details und Verträge: §Sensors. Geplant (noch nicht behauptet): `coverage-gate-critical`,
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
| [`../docs/plan/adr/`](../docs/plan/adr/) | [ADR-0001](../docs/plan/adr/0001-hexagonale-architektur.md) (Hexagonal), 0002 (OCC), 0003 (SQLite) |
| [`../docs/plan/planning/`](../docs/plan/planning/) | Slices und Roadmap |
| [`../AGENTS.md`](../AGENTS.md) | Hard Rules, Workflow |
| [`conventions.md`](conventions.md) | Strukturregeln, `MR-*`-Adaptionen, Modus-Deklaration |
| [Agents-Regelwerk des Kurses](https://raw.githubusercontent.com/pt9912/ai-harness-course/main/kurs/de/agents-regelwerk.md) | adoptiertes Betriebsregelwerk in Agenten-Kurzform; derivativ, Stand siehe [`conventions.md`](conventions.md) §Baseline |

## Sensors (Feedback-Gates)

**Real existierend.** Die meisten Gates sind **Dockerfile-Target-Stages**
(`docker build --target …`, Quelle per `COPY` eingebacken) — **keine
Bind-Mounts**, maximal reproduzierbar (Modul 14, Vorbild cmake-xray).
**Ausnahme:** `make a-check` (`a-check.mk`) und `make docs-check`
(`d-check.mk`) laufen als externe, digest-gepinnte Images per read-only
Bind-Mount (`--network none`):

| Target | Vertrag | Bindung |
|---|---|---|
| `make docs-check` | interne Links, Anker, Inline-Code-Pfade + **Referenz-Richtung Spec→ADR + ADR↛Slice** (`matrix`/`ids`) + Span-/Host-Pfad-Hygiene; kein Pfad aus dem Repo, keine Abwärts-Referenz Spec → ADR, **kein Slice-Token im ADR-Körper** (no-downward, ab d-check v0.37.1) — via d-check (`d-check.mk`, digest-gepinnt v0.37.1, `.d-check.yml`; netzlos read-only Bind-Mount; Module links/anchors/codepaths/spans/hostpaths/matrix/ids) | [`MR-007`](conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check), [`MR-011`](conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths), [`MR-014`](conventions.md) |
| `make gate-consistency` | jeder als real dokumentierte `make`-Befehl (AGENTS §3 / §Sensors) existiert im Makefile — fängt halluzinierte Gates | Modul 13 |
| `make a-check` | **primäres Architektur-Gate** via externem digest-gepinntem Image **a-check** (`.a-check.yml`; `docker run --network none -v $(CURDIR):/src:ro`, read-only Bind-Mount): Kern importiert kein Qt/OCC/SQLite/`adapters/` (Regel A); kein Adapter importiert einen anderen (Regel B, MeshSource-Naht via `adapter_sink`); OCC-`.hxx`/`sqlite3`/Qt/`dlfcn.h`-Include gekapselt (Regel C/D/E); **Schicht-Kanten** (erlaubter Import-Graph) + **driving/driven-Richtung** | [MR-013](conventions.md#mr-013--arch-check-via-a-check), [ADR-0001](../docs/plan/adr/0001-hexagonale-architektur.md), [ADR-0002](../docs/plan/adr/0002-geometrie-kern-opencascade.md), [ADR-0003](../docs/plan/adr/0003-persistenz-sqlite.md), [ADR-0009](../docs/plan/adr/0009-gui-framework-qt6.md), [ADR-0017](../docs/plan/adr/0017-plugin-api-abi.md) |
| `make arch-check` | Plugin-System-**P-Rest**, was der kanten-basierte a-check strukturell nicht sieht: `dlopen`/`dlsym`/`dlclose`-**Funktionsaufruf** nur in `adapters/plugin/` (Regel P1-Aufruf; der `dlfcn.h`-**Include** liegt bei a-check) + feine Quote-Import-Allowlist `plugins/` + `src/plugin_api/` auf `plugin_api`/`hexagon/model`/`hexagon/ports/driving` (Regel P2, Quote-Form + Angle-Verbot) | [ADR-0017](../docs/plan/adr/0017-plugin-api-abi.md), [MR-013](conventions.md#mr-013--arch-check-via-a-check) |
| `make lint` | clang-tidy (0 Befunde in `src/` + `plugins/`) + Suppression-Gate | [ADR-0001](../docs/plan/adr/0001-hexagonale-architektur.md) §Fitness (AGENTS.md §2.4) |
| `make test` | GoogleTest-Suite: prüft Kern-Logik + echte Adapter-Linkage (Qt/OCC/SQLite); Viewer-AK display-frei (Szenen-Surrogat) + GL-Smoke headless via Xvfb/llvmpipe | [ADR-0009](../docs/plan/adr/0009-gui-framework-qt6.md) (f), [ADR-0010](../docs/plan/adr/0010-headless-gl-xvfb.md) |
| `make coverage-gate` | Line-Coverage ≥ Schwelle (bootstrap-aware, Composition Root ausgenommen) | Schwelle 70 %, Ramp → M2 (siehe AGENTS.md §3) |
| `make build` | Target-Kette kompiliert; erzwingt CMake-Target-Trennung (Kern ohne Adapter-Deps) | [ADR-0001](../docs/plan/adr/0001-hexagonale-architektur.md) |
| `make gates` | Aggregat: docs-check · gate-consistency · a-check · arch-check · lint · test · coverage-gate (+ `record-gates`-Nachweis) | — |
| `make schema-check` | [ADR-0006](../docs/plan/adr/0006-relationales-schema-design.md)-Drift: committete `schema.sql` == d-migrate(`data-model.yaml`). **Nicht** in `make gates` (d-migrate aus dem hermetischen Gate-Pfad gehalten) — gehört in die **CI-Befehlsliste** | [ADR-0006](../docs/plan/adr/0006-relationales-schema-design.md) |
| `make acc-002-beleg` | **kein Gate:** rendert das [ACC-001](../spec/lastenheft.md#7-abnahmekriterien)-Kern-Demo headless (Xvfb) und schreibt das [ACC-002](../spec/lastenheft.md#7-abnahmekriterien)-Beleg-Bild — manueller Abnahme-Schritt des Projektinhabers, bewusst nicht in `gates` | [ADR-0009](../docs/plan/adr/0009-gui-framework-qt6.md) (f), [ADR-0010](../docs/plan/adr/0010-headless-gl-xvfb.md) |
| `make run` | **kein Gate:** startet die App im Container am lokalen Display (X11/XWayland; GPU-Durchreichung via `/dev/dri`, sonst llvmpipe-Fallback; vorher ggf. `xhost +local:`) | [ADR-0009](../docs/plan/adr/0009-gui-framework-qt6.md), AGENTS §2.9 |
| `make io-smoke` | **kein Gate** (nicht in `gates` → CI-Befehlsliste, Muster `make schema-check`): startet das gebaute Binary headless (xvfb, [ADR-0010](../docs/plan/adr/0010-headless-gl-xvfb.md)) je Austauschformat — **IFC/DXF** Export+Re-Import, **STEP/STL** Export — jeder Aufruf exit 0 + nicht-leere Datei (fail-closed je-Aufruf-Guard). Belegt die sonst coverage-ausgenommene `main.cpp`-CLI-/Composition-Root-Glue der IO-Pfade | LH-Bindung [LH-FA-IO-001](../spec/lastenheft.md#lh-fa-io-001--ifc-import) … [LH-FA-IO-006](../spec/lastenheft.md#lh-fa-io-006) (conventions.md) |
| `make doc-commits` | Commit-Traceability: jede Commit-Message einer Range trägt eine `slice-*`/`ADR-*`/`MR-*`/`LH-*`-Kennung (d-check-Modul `commits`, git-Range, `d-check.mk`; `exempt-pattern` Merge/Revert) — macht die §Traceability-Regel / [`AGENTS.md` §4](../AGENTS.md) computational; **kein Gate** (nicht in `gates` → CI-Befehlsliste, Muster `make schema-check`) | [MR-015](conventions.md) |

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

- `make coverage-gate-critical` — Critical-Path-Coverage (Persistenz/Recovery, [LH-QA-005](../spec/lastenheft.md#lh-qa-005--crash-recovery)), höhere Schwelle. Bindung: [LH-QA-005](../spec/lastenheft.md#lh-qa-005--crash-recovery).
- `make ci` / `make fullbuild` — weitere Aggregat-/Closure-Gates (inkl. Image-Hash, Modul 14).

**Aktueller Lauf-Status:** wird hier **nicht** geführt — Lauf-Wahrheit
pro Commit gehört in CI, nicht in diese Datei (Rang 9; Kurs-Modul 13).
**Rote Gates:** würden als `CO-<NNN>` in
[`../docs/plan/carveouts/`](../docs/plan/carveouts/) dokumentiert (Modul 7).

## Traceability rules

- PRs/Commits **müssen** mindestens eine `LH-*`-, `ADR-*`-, `MR-*`- oder
  `slice-*`-ID nennen (Steering-/Prozess-Slices sind anforderungsfrei, `slice-*`
  ist ihr Anker). **Maschinell erzwungen** via `make doc-commits`
  ([MR-015](conventions.md), d-check-Modul `commits`, CI-Range).
- Neue Anforderungen brauchen Beleg: Test (mit ID im Namen), Gate, Demo
  oder ADR.
- Neue ADRs müssen [`../docs/plan/adr/README.md`](../docs/plan/adr/README.md) aktualisieren.
- Slice-Lifecycle-Bewegung (`open → next → in-progress → done`) ist
  reiner `git mv` (siehe [`../AGENTS.md` §2.8](../AGENTS.md)).

## Safety and scope boundaries

- **Datenverlust am Gebäudemodell ist der schärfste Fehlerfall.**
  Persistenz schreibt atomar (Temp + Rename); Crash-Recovery erhält den
  letzten konsistenten Stand ([LH-QA-005](../spec/lastenheft.md#lh-qa-005--crash-recovery), [LH-FA-BLD-002](../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)).
- **Kein OCC/Qt/SQLite im Kern.** Geometrie-, GUI- und DB-Technologie
  leben ausschließlich in Adaptern ([ADR-0001](../docs/plan/adr/0001-hexagonale-architektur.md)).
- **Plugins laufen in einer Sandbox** und dürfen das Modell nicht
  umgehen oder korruptieren ([LH-FA-PLG-004](../spec/lastenheft.md#modul-plugin-system-plg)).
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
