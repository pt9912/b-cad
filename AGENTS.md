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
Projekt darf beobachtbar sein (LH-QA-005, LH-FA-BLD-002). Plugins dürfen
das Modell nicht umgehen (Sandbox, LH-FA-PLG-004).

**Falsch:** Projektdatei direkt in-place überschreiben / `fopen(path, "w")`.
**Richtig:** in `path.tmp` schreiben, `fsync`, dann atomar nach `path` renamen.

### 2.3 Docker-only (DevContainer)

Build und Gates laufen über den reproduzierbaren Docker-DevContainer
(REQ-TEC-009) via `make`. Host braucht nur Docker und GNU `make` — kein
lokales Qt-/OCC-SDK.

**Begründung:** Toolchain-Reproduzierbarkeit + Supply-Chain-Defense.
*(DevContainer + `make build` existieren seit slice-001; weitere
Code-Gates folgen in slice-002 — siehe §3.)*

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

`spec/architecture.md` referenziert ADRs, aber **keine** Wellen, Slices,
Commit-Hashes oder Closure-Daten. Die zeitliche Schicht lebt in
`docs/plan/planning/`.

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

## 3. Quality Gates

> **Bootstrap-Hinweis (ehrlich).** b-cad ist im Greenfield-Bootstrap.
> Es existiert **genau ein reales Gate** (`docs-check`, Doku-Validator)
> und **noch kein Code**. Code-Gates sind **geplant, nicht behauptet**;
> sie werden mit dem ersten Code-Slice real (Promotion-Trigger,
> Kurs-Modul 2). Ein Agent darf **keinen** geplanten Befehl als
> existierend ausgeben, bevor er im Makefile steht — und `make gates`
> darf nur real existierende Sub-Targets aggregieren.

**Real (existieren im Makefile, mit ID-Kommentar):**

| Target | Zweck | Bindung |
|---|---|---|
| `make docs-check` | Doku-Konsistenz: interne Markdown-Links/Anker/ID-Pfade (im Container) | MR-003 |
| `make build` | DevContainer-Build der Target-Kette + `ctest`; erzwingt CMake-Target-Trennung (Kern ohne Adapter-/Qt-/OCC-/SQLite-Deps) | ADR-0001 |
| `make gates` | alle inneren Gates — **derzeit nur** `docs-check` | — |

**Geplant (noch NICHT behauptet — entstehen mit slice-002):**

| Target (geplant) | Zweck | Bindung |
|---|---|---|
| `make lint` | clang-tidy + Suppression-Gate | — |
| `make arch-check` | hexagonale Layering-Constraints | ADR-0001, ADR-0002, ADR-0003 |
| `make test` | GoogleTest inkl. Crash-Recovery/Determinismus | LH-QA-005 |
| `make coverage-gate` | Gesamt-Coverage (bootstrap-aware, mit dokumentierter Hochschalt-Schwelle) | — |
| `make coverage-gate-critical` | Critical-Path-Coverage: Persistenz/Crash-Recovery (Datenverlust = schärfster Fehlerfall) | LH-QA-005 |
| `make ci` | gates + Extras | — |
| `make fullbuild` | volle Closure inkl. Runtime-Image + Image-Hash | — |

## 4. Dokumentations-Regeln

- Requirement- und ADR-IDs müssen in PRs/Commits referenziert sein
  (`LH-FA-*`, `LH-QA-*`, `ADR-*`).
- Neue ADRs müssen [`docs/plan/adr/README.md`](docs/plan/adr/README.md) aktualisieren.
- Roadmap/Status-Geschichte lebt in `docs/plan/planning/`, nicht in `spec/architecture.md`.
- Slice-Lifecycle-Bewegung ist reiner `git mv` (siehe §2.8).

## 5. Minimal Agent Workflow

1. [`harness/README.md`](harness/README.md) lesen.
2. Relevante kanonische Quelle lesen (Source Precedence beachten).
3. Betroffene Requirement-/ADR-IDs identifizieren.
4. Kleinste sinnvolle Änderung planen.
5. Engsten nützlichen Sensor laufen lassen (sobald welche existieren).
6. Repo-weiten Gate-Lauf vor Handoff (`make gates`, sobald vorhanden).
7. Doku/Indizes aktualisieren, falls ein öffentlicher Vertrag berührt.
8. Ausgeführte Sensors und verbleibende Risiken berichten — **keine
   Erfolgsmeldung ohne Gate-Ausführung, kein behauptetes Gate**.
