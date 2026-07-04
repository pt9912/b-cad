# AGENTS.md — Briefing für AI-Coding-Agenten (b-cad)

Onboarding für jede AI-Session, die in **b-cad** Code oder Dokumentation
ändert. Diese Datei trägt die **harten Regeln** und **Pointer auf die
kanonischen Quellen**, nicht deren Inhalt.

**Bei Konflikt zwischen dieser Datei und einer kanonischen Quelle gilt
die kanonische Quelle.**

Strukturregeln (ID-Schema, Verzeichniskonvention, Adaptionen ggü.
Baseline, Modus-Deklaration pro Sub-Area, Zusatzklassen für
Sensors-Bindung) leben in
[`harness/conventions.md`](harness/conventions.md).

Das **Betriebsregelwerk der adoptierten Baseline in Agenten-Kurzform**
ist das
[Agents-Regelwerk des Kurses](https://raw.githubusercontent.com/pt9912/ai-harness-course/main/kurs/de/agents-regelwerk.md)
— einmal pro Session lesen, bevor der Workflow (§5) startet. Derivativ:
bei Konflikt gelten die kanonischen Quellen; adoptierter Stand steht in
[`harness/conventions.md`](harness/conventions.md) §Baseline.

## 1. Kanonische Quellen (Source Precedence)

1. [`spec/lastenheft.md`](spec/lastenheft.md) — vertraglich abnahmebindend.
2. [`spec/spezifikation.md`](spec/spezifikation.md) — technisch verbindlich, fortschreibbar.
3. [`spec/architecture.md`](spec/architecture.md) — Komponenten/Schichten, meilensteinfrei.
4. [`docs/plan/adr/README.md`](docs/plan/adr/README.md) — ADR-Index.
5. [`docs/plan/planning/in-progress/roadmap.md`](docs/plan/planning/in-progress/roadmap.md) — aktuelle Welle.
6. [`docs/user/`](docs/user/) — Operations, Releasing.
7. [`README.md`](README.md) — Projekt-Überblick.
8. AGENTS.md (diese Datei).
9. [`harness/README.md`](harness/README.md) — Harness-Einstieg.

## 2. Harte Regeln

### 2.1 Hexagonale Abhängigkeitsrichtung ist unverletzlich

Der Kern (`src/hexagon/`) importiert **niemals** aus `src/adapters/` und
**niemals** Qt, OpenCascade oder SQLite. Abhängigkeiten zeigen nach
innen (Adapter → Kern, nie umgekehrt). Durchgesetzt über getrennte
CMake-Targets (`bcad_hexagon` ohne externe Deps) — siehe
[ADR-0001](docs/plan/adr/0001-hexagonale-architektur.md).

**Falsch:** `#include <TopoDS_Shape.hxx>` in `src/hexagon/services/…`
**Richtig:** OCC nur in `src/adapters/geometry/`, Kern nutzt `GeometryKernelPort`.

### 2.2 Datenverlust-Schutz: Persistenz ist atomar

Jeder Projekt-Schreibvorgang schreibt in eine Temp-Datei und ersetzt den
bestehenden Stand erst nach Erfolg (Rename). Kein halb geschriebenes
Projekt darf beobachtbar sein ([LH-QA-005](spec/lastenheft.md#lh-qa-005--crash-recovery), [LH-FA-BLD-002](spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)). Plugins dürfen
das Modell nicht umgehen (Sandbox, [LH-FA-PLG-004](spec/lastenheft.md#modul-plugin-system-plg)).

**Falsch:** Projektdatei direkt in-place überschreiben / `fopen(path, "w")`.
**Richtig:** in `path.tmp` schreiben, `fsync`, dann atomar nach `path` renamen.

### 2.3 Docker-only (DevContainer)

Build und Gates laufen über den reproduzierbaren Docker-DevContainer
([REQ-TEC-009](spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec)) via `make`. Host braucht nur Docker und GNU `make` — kein
lokales Qt-/OCC-SDK.

**Begründung:** Toolchain-Reproduzierbarkeit + Supply-Chain-Defense.
*(DevContainer + alle Code-Gates existieren seit slice-001/002 —
siehe §3.)*

### 2.4 Suppression-Verbot

Inline-Suppression (`// NOLINT`, `// NOLINTNEXTLINE`,
`#pragma GCC diagnostic ignored`) bricht das künftige Suppression-Gate.
Ausnahmen leben zentral in `.clang-tidy` mit Begründung (ADR- oder
Slice-ID).

**Falsch:** `int x; // NOLINT(...)` direkt im Code.
**Richtig:** Regel in `.clang-tidy` (`Checks:`/`CheckOptions`) mit Kommentar `# ADR-NNNN / slice-NNN`.

### 2.5 ADRs sind nach `Accepted` immutable

Korrekturen entstehen als neue ADR mit `Supersedes ADR-NN`. Den
ADR-Index aktualisieren.

### 2.6 Gates dürfen nicht ohne ADR gelockert werden

Jede Schwellen-Senkung (Coverage, clang-tidy-Strenge, Architekturregel)
ist ein ADR plus Carveout, kein PR-Kommentar.

### 2.7 Architektur ist sprach- und meilensteinfrei

`spec/architecture.md` ist ein **derivatives Sicht-Stratum**: es trägt
**keine** Wellen, Slices, Commit-Hashes oder Closure-Daten (die zeitliche
Schicht lebt in `docs/plan/planning/`) und — seit slice-018a — **keine
Abwärts-Referenz auf ADRs im Körper**. Maßgeblich ist das Regelwerk
(§Referenz-Richtung): „Spec → ADR existiert im bindenden Text nicht — auch
nicht als Quellen-Spalte"; die frühere Lesart „referenziert ADRs" war eine
undeklarierte Inkonsistenz (AGENTS rangiert unter dem Regelwerk, §1). Die
normative Begründung lebt in den ADRs selbst (Aufwärts-Verweis ADR → Spec);
ADR-Provenance einer Architektur-Aussage steht nur in der Provenance-Rand-
Tabelle `## Geschichte` (matrix-`exclude-sections`) und im Vollindex
[`docs/plan/adr/README.md`](docs/plan/adr/README.md). Computational
durchgesetzt vom `docs-check`-Gate (Module `matrix` + `ids`,
[`MR-011`](harness/conventions.md)).

### 2.8 git mv + Inhaltsänderung = zwei Commits

Reine Move-Commits zuerst (Lifecycle-Bewegung von Slices/Carveouts),
dann inhaltliche Änderungen — sonst fällt Git-Rename-Detection unter die
50 %-Schwelle.

### 2.9 Werkzeugbindung (Tool-Allowlist)

Der Agent baut, testet und prüft **nur über `make`** (das Docker nutzt,
§2.3) — kein direktes `cmake`/`clang`/`clang-tidy`/`ctest` auf dem Host,
kein `apt`/`vcpkg`/`conan`-Install außerhalb des Containers, kein
Netzwerkzugriff während eines Slice außer den im Container gepinnten
Abhängigkeiten.

| Erlaubt | Nicht erlaubt (ohne ADR/Slice-Deckung) |
|---|---|
| `make <target>`, `git`, Lesen/Schreiben im Repo | direktes Host-Build (`cmake --build`, `clang++`), Host-Paketinstallation, Netzzugriff im Slice |
| `make docs-check` / `make gates` (real) | Gates behaupten, die kein Make-Target sind (§3) |

**Begründung:** Reproduzierbarkeit (gleiches Image lokal und in CI,
Modul 14) und Supply-Chain-Defense. Was nicht im Container gepinnt ist,
existiert für den Lauf nicht.

**Durchsetzung (computational feedforward, Modul 9):** der PreToolUse-Hook
`.claude/hooks/pretooluse-command-guard.sh` blockt direkte Host-Aufrufe
der verbotenen Tool-Namen an Wortgrenzen (prüft nur `tool_input.command`).

## 3. Quality Gates

> **Honesty-Hinweis.** Jeder hier als *real* gelistete Gate existiert
> als `make`-Target (Dockerfile-Stage, **kein** Bind-Mount). Ein Agent
> darf **keinen** geplanten Befehl als existierend ausgeben, bevor er im
> Makefile steht — und `make gates` aggregiert nur real existierende
> Sub-Targets (Kurs-Modul 13).

**Real (existieren im Makefile als Dockerfile-Target-Stage):**

| Target | Zweck | Bindung |
|---|---|---|
| `make docs-check` | Doku-Konsistenz: interne Links/Anker/Inline-Code-Pfade + **Referenz-Richtung Spec→ADR** (d-check-Module links/anchors/codepaths/spans/hostpaths/matrix/ids) | [MR-007](harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check), [MR-011](harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths) |
| `make gate-consistency` | jeder als real dokumentierte `make`-Befehl existiert im Makefile (fängt halluzinierte Gates) | Modul 13 |
| `make a-check` | Architektur (**primär**): hexagonale Schichtung via externem digest-gepinntem Image **a-check** (`.a-check.yml`; netzlos `--network none`, read-only Bind-Mount) — Kern-Reinheit (Regel A) · laterale Adapter (B, MeshSource-Naht via `adapter_sink`) · Tech-Kapselung OCC-`.hxx`/`sqlite3`/Qt/`dlfcn.h`-Include (C/D/E) · **Schicht-Kanten** · **driving/driven-Richtung** | [MR-013](harness/conventions.md#mr-013--arch-check-via-a-check), [ADR-0001](docs/plan/adr/0001-hexagonale-architektur.md), [ADR-0002](docs/plan/adr/0002-geometrie-kern-opencascade.md), [ADR-0003](docs/plan/adr/0003-persistenz-sqlite.md), [ADR-0009](docs/plan/adr/0009-gui-framework-qt6.md), [ADR-0017](docs/plan/adr/0017-plugin-api-abi.md) |
| `make arch-check` | Plugin-System-**P-Rest** (was a-check strukturell nicht sieht): `dlopen`/`dlsym`/`dlclose`-**Aufruf** nur in `adapters/plugin/` (Regel P1-Aufruf; der `dlfcn.h`-Include liegt bei a-check) + feine Import-Allowlist `plugins/`+`src/plugin_api/` (nur `plugin_api/`/`hexagon/model/`/`hexagon/ports/driving/`, Quote-Form + Angle-Verbot, Regel P2) | [ADR-0017](docs/plan/adr/0017-plugin-api-abi.md), [MR-013](harness/conventions.md#mr-013--arch-check-via-a-check) |
| `make lint` | clang-tidy (0 Befunde in `src/` + `plugins/`) + Suppression-Gate | [ADR-0001](docs/plan/adr/0001-hexagonale-architektur.md), AGENTS §2.4 |
| `make test` | GoogleTest: Kern-Logik + echte Adapter-Linkage (Qt/OCC/SQLite); Viewer headless via Xvfb | [ADR-0009](docs/plan/adr/0009-gui-framework-qt6.md)/0010 |
| `make coverage-gate` | bootstrap-aware Line-Coverage ≥ `COVERAGE_THRESHOLD` (Composition Root ausgenommen) | Schwelle 70 %, Ramp → M2 |
| `make build` | Target-Kette kompilieren; CMake-Target-Trennung (Kern ohne Adapter-Deps) | [ADR-0001](docs/plan/adr/0001-hexagonale-architektur.md) |
| `make gates` | docs-check · gate-consistency · a-check · arch-check · lint · test · coverage-gate | — |
| `make schema-check` | [ADR-0006](docs/plan/adr/0006-relationales-schema-design.md)-Drift: `schema.sql` == d-migrate(`data-model.yaml`); **nicht** in `gates` (d-migrate aus dem Gate-Pfad) → CI-Befehlsliste | [ADR-0006](docs/plan/adr/0006-relationales-schema-design.md) |
| `make acc-002-beleg` | [ACC-002](spec/lastenheft.md#7-abnahmekriterien)-Beleg-Bild headless rendern — **kein Gate**, manueller Abnahme-Schritt, nicht in `gates` | [ADR-0009](docs/plan/adr/0009-gui-framework-qt6.md) (f)/0010 |
| `make run` | App im Container am lokalen Display starten — **kein Gate** (GPU via `/dev/dri`, sonst llvmpipe) | [ADR-0009](docs/plan/adr/0009-gui-framework-qt6.md) |
| `make io-smoke` | IO-Binary headless je Format (IFC/DXF Export+Re-Import, STEP/STL Export; exit 0 + nicht-leere Datei, fail-closed) — belegt die coverage-ausgenommene `main.cpp`-CLI-/Composition-Root-Glue; **kein Gate**, nicht in `gates` → CI-Befehlsliste (Muster `schema-check`) | LH-Bindung [LH-FA-IO-001](spec/lastenheft.md#lh-fa-io-001--ifc-import) … [LH-FA-IO-006](spec/lastenheft.md#lh-fa-io-006) |

**Geplant (noch NICHT behauptet):**

| Target (geplant) | Zweck | Bindung |
|---|---|---|
| `make coverage-gate-critical` | Critical-Path-Coverage: Persistenz/Crash-Recovery (Datenverlust = schärfster Fehlerfall) | [LH-QA-005](spec/lastenheft.md#lh-qa-005--crash-recovery) |
| `make ci` | gates + Extras | — |
| `make fullbuild` | volle Closure inkl. Runtime-Image + Image-Hash | — |

## 4. Dokumentations-Regeln

- Requirement- und ADR-IDs müssen in PRs/Commits referenziert sein
  (`LH-FA-*`, `LH-QA-*`, `ADR-*`). **Vergeben** werden IDs beim
  Spec-/ADR-Schreiben nach dem in
  [`harness/conventions.md` (MR-002)](harness/conventions.md) deklarierten
  Schema — nie ad hoc im PR. Agenten **referenzieren** IDs nur, sie
  **erfinden** keine.
- Neue ADRs müssen [`docs/plan/adr/README.md`](docs/plan/adr/README.md) aktualisieren.
- Roadmap/Status-Geschichte lebt in `docs/plan/planning/`, nicht in `spec/architecture.md`.
- Slice-Lifecycle-Bewegung ist reiner `git mv` (siehe §2.8).
- **Lastenheft-Schärfung bleibt lösungsfrei**
  ([`harness/conventions.md` MR-008](harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)): schärft ein Slice eine Anforderung
  von Outline auf AK-Niveau, sind die AK benutzer-beobachtbar
  (Given/When/Then; Wertebereiche = das *Was*); Lösungsmechanik
  (Algorithmen, Ports, Formeln, Fehler-Code-/`op`-Vokabular) gehört in
  `spec/spezifikation.md` bzw. ADRs — **nie ins Lastenheft**.
- **Lastenheft-Schärfung zieht den Header-Version nach**
  ([`harness/conventions.md` MR-010](harness/conventions.md)): der
  `**Version:**`-Header von `spec/lastenheft.md` == oberste (jüngste)
  §9-Historie-Zeile. Die [MR-006](harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse prüft den Nachzug je Schärfungs-Slice.

## 5. Minimal Agent Workflow

1. [`harness/README.md`](harness/README.md) lesen.
2. Relevante kanonische Quelle lesen (Source Precedence beachten).
3. Betroffene Requirement-/ADR-IDs identifizieren.
4. Kleinste sinnvolle Änderung planen.
5. Vor Implementierungs-Start eines Slice: **unabhängiges
   Plan-Review** des Slice-Plans, Findings einarbeiten — HIGHs
   blockieren den Start
   ([`harness/conventions.md` MR-006](harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)).
   Geometrieschwere Implementierungs-Slices erhalten **zusätzlich** ein
   unabhängiges **Code-Review vor der Welle-Closure** (Geometrie-Korrektheit
   gegen die Spec; [MR-009](harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) — HIGHs blockieren die
   Closure.
6. Engsten nützlichen Sensor laufen lassen.
7. Repo-weiten Gate-Lauf vor Handoff (`make gates`).
8. Doku/Indizes aktualisieren, falls ein öffentlicher Vertrag berührt.
9. Ausgeführte Sensors und verbleibende Risiken berichten — **keine
   Erfolgsmeldung ohne Gate-Ausführung, kein behauptetes Gate**.
