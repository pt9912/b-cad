---
id: slice-030
titel: Architektur-Gate-Umstellung — a-check (externes digest-gepinntes Image) übernimmt A–E + Schicht-Kanten + driving/driven; tools/arch-check.sh auf den P-Rest geschrumpft
status: done
welle: harness-steering
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0017](../../adr/0017-plugin-api-abi.md)]
---

# Slice 030: Architektur-Gate via a-check (arch-check.sh → P-Rest)

**Status:** **done** (2026-07-04) — `make gates` grün: docs-check 171/0 · gate-consistency ok ·
**a-check `gesamt: 0 Befund(e)`** (netzlos, read-only Aggregat-Member) · arch-check P-Rest ok ·
lint + suppression-gate ok · **test 228/228 (100 %, 0 failed)** · coverage **90,4 %** (Schwelle 70) ·
record-gates geschrieben. Beide Gegenproben belegt: a `QWidget`→`core-impurity`/Exit 1;
b `dlopen(`-Aufruf außerhalb Host → geschrumpftes `arch-check.sh` feuert P1/Exit 1 (beide revertiert).
— [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
**0 HIGH / 3 MED / 1 LOW / INFO**; Start **nicht blockiert**, alle Findings vor
Start eingearbeitet: MED-1 (Doku-Konsistenz-Inventar auf die ~22 A–E-Zuschreibungen
in `src/`+`tests/` erweitert — P1/P2-Zuschreibungen bleiben bei arch-check) + MED-2
(Beleg-#3 präzisiert: die Plan-Datei-Prosa vermeidet die kollidierenden
a-check-Repo-Fremd-IDs, damit docs-check grün bleibt) + MED-3 (Makefile-Kopf-
Invariante um die a-check-Bind-Mount-Ausnahme korrigiert) + LOW-1
(`grep -h` beim Zwei-Datei-Scan). Governance [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)-statt-ADR **bestätigt**;
P-Rest-Deckung empirisch belegt (a-check fängt Qt/OCC/verbotenen-Import im
`plugins/`-Baum; `dlopen(`-Aufruf bleibt a-check-blind → P-Rest zwingend).
[Report](../../../reviews/2026-07-04-slice-030-plan.md).

**Welle:** harness-steering (Quergewerk, Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)
docs-check→d-check / slice-018a–c / slice-022 — **welle-5-Scope unberührt**, kein
Wellen-Feature; Roadmap erhält den üblichen Quergewerk-Eintrag in „Historische
Trigger-Verschiebungen"). Die Roadmap hat diesen Folge-Slice bereits
vorgezeichnet (Eintrag 2026-07-03: „*geplante Gate-Umstellung `tools/arch-check.sh`
→ a-check, eigener Folge-Slice Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)*").

**Bezug:** `make arch-check`-Gate (Vertrag heute: hexagonale Schichtung Regel
A–E + P1/P2, [ADR-0001](../../adr/0001-hexagonale-architektur.md)/[ADR-0002](../../adr/0002-geometrie-kern-opencascade.md)/[ADR-0003](../../adr/0003-persistenz-sqlite.md)/[ADR-0009](../../adr/0009-gui-framework-qt6.md)/[ADR-0017](../../adr/0017-plugin-api-abi.md)).
**Projektinhaber-Anstoß (2026-07-04):** a-check `v0.9.0` ist released
(Root-Sub-Einheit/Blatt-Klassifikation für den `lateral-adapter` — die frühere
Falsch-Positiv-Klasse `x.cpp → x.h` ist getilgt); das digest-gepinnte Image wurde
gegen b-cad `8464d38` mit der Vollrichtungs-Config verifiziert (0 Befunde,
Exit 0; Gegenprobe `QWidget`→`core-impurity`, Exit 1). b-cad wird **erster
Pilot-Konsument** (a-check-Meilenstein M3). Struktur-Vorbedingungen (slice-028
`services/geometry/`, slice-029 `ui/command|view`-Split) sind geliefert.

**Autor:** Dietmar Burkard (Anstoß) / AI-Session (Ausführung). **Datum:** 2026-07-04.

## Governance-Verankerung: MR-013 (Muster MR-007), keine ADR

Die Umstellung wird als **neue Konventions-Adaption
[MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)**
in [`harness/conventions.md`](../../../../harness/conventions.md) verankert —
**exakt analog [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)**
(docs-check via d-check), das denselben Formwechsel (hand-gerollter/vendorter
Gate → externes digest-gepinntes Image + deklarative `.<tool>.yml`-Config)
verankerte. **Kein ADR**, weil:

1. **Keine Gate-Lockerung** ([AGENTS.md §2.6](../../../../AGENTS.md) verlangt
   ADRs nur für Lockerungen): die durchgesetzten Architekturregeln bleiben
   erhalten **und werden strenger** — a-check trägt Kern-Reinheit (A), laterale
   Adapter (B), Tech-Kapselung Qt/OCC/SQLite/`dlfcn.h` (C/D/E + P1-Include) UND
   **neu** den vollen **Schicht-Kanten-Graph** + die **driving/driven-Richtung**,
   die `arch-check.sh` nie prüfte. Verschärfung, kein §2.6-Fall.
2. **Roadmap-Vorgabe:** der Roadmap-Eintrag 2026-07-03 nennt ausdrücklich
   „*eigener Folge-Slice Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)*"
   (kanonische Quelle Rang 5).
3. **Keine Architektur-Entscheidung ändert sich** — nur der
   **Durchsetzungs-Mechanismus** (das genau ist der Gegenstand einer
   MR-Adaption). Die Architektur-ADRs 0001/0002/0003/0009/0017 bleiben
   unverändert gültig; ihre Bindung wandert in den Sensors-Tabellen von
   `arch-check` auf `a-check` (+ P-Rest).

## Arbeitsteilung nach der Umstellung

| Gegenstand | vor slice-030 | nach slice-030 |
|---|---|---|
| Kern-Reinheit (Regel A: kein Qt/OCC/SQLite/`adapters/`-Import im Kern) | `arch-check.sh` | **a-check** (Schicht-Kanten + tech) |
| Laterale Adapter (Regel B: kein Adapter→anderer Adapter) | `arch-check.sh` | **a-check** (+ `adapter_sink` MeshSource-Naht) |
| OCC-`.hxx` nur in `adapters/geometry/` (Regel C) | `arch-check.sh` | **a-check** (`tech: .hxx`) |
| `sqlite3*` nur in `adapters/persistence/` (Regel D) | `arch-check.sh` | **a-check** (`tech: sqlite3`) |
| Qt-Header nur in `adapters/ui/` + `main.cpp` (Regel E) | `arch-check.sh` | **a-check** (`tech: Q[A-Za-z]` regex) |
| Schicht-Kanten (erlaubter Import-Graph) | — (nie geprüft) | **a-check** (`edges`) |
| driving/driven-Richtung | — (nie geprüft) | **a-check** (`direction` + `ui_command→ui_view`) |
| `dlfcn.h`-**Include** nur im Plugin-Host (P1-Include-Teil) | `arch-check.sh` | **a-check** (`tech: dlfcn.h`, `composition_root: forbid`) |
| **`dlopen`/`dlsym`/`dlclose`-AUFRUF** außerhalb des Plugin-Hosts (P1-Call) | `arch-check.sh` | **`arch-check.sh` (P-Rest)** — a-check prüft nur Include-/Import-**Kanten**, kein Funktionsaufruf |
| **Feine P2-Allowlist** (Plugins/`plugin_api` bauen nur gegen `plugin_api/`+`hexagon/model/`+`hexagon/ports/driving/`; Quote-Form + Angle-Verbot) | `arch-check.sh` | **`arch-check.sh` (P-Rest)** |

**Der P-Rest bleibt** genau da, wo a-check strukturell blind ist: a-check
prüft ausschließlich **Include-/Import-Kanten** — der `dlopen(`-**Funktionsaufruf**
ist keine Include-Kante, und die **feine Quote-vs-Angle-Allowlist** ist die
projektspezifische P2-Härtung (slice-026b, Review-MED-3), die unterhalb der
Kanten-Granularität liegt. Die Tech-Kapselung Qt/OCC/SQLite **im `plugins/`-Baum**
(vormals `arch-check.sh` P2b) fällt an a-check (`tech`-Regeln greifen
repo-weit) und wird aus `arch-check.sh` **entfernt**.

## Bind-Mount-Abweichung (dokumentiert, begründet)

Die anderen `gates`-Member sind **Dockerfile-Target-Stages** (Quelle per `COPY`
eingebacken, kein Bind-Mount — maximal reproduzierbar, Modul 14). `a-check` läuft
per `docker run --network none -v "$(CURDIR)":/src:ro` — ein **read-only
Bind-Mount** statt COPY-Stage. Bewusste, dokumentierte Abweichung, weil:

- Das **Image ist digest-gepinnt** ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Prinzip
  auf externe Tools, Muster [ADR-0006](../../adr/0006-relationales-schema-design.md) `d-migrate`
  `@sha256`) — die Reproduzierbarkeit sitzt auf **Image-Ebene**, nicht in der
  COPY-Stage.
- `--network none` (netzlos) hält die Hermetik zur Laufzeit; `:ro` verhindert
  jede Repo-Mutation durch das Tool.
- Der Pin-Bezug im generierten `a-check.mk`-Kommentar (`AC-QA-03` sowie
  a-check-repo-**eigene** `ADR`-Nummern) verweist auf **a-check-Repo-IDs**, nicht
  auf b-cad-ADRs (empirisch belegt: `make docs-check` bleibt 0 Befunde — b-cads
  d-check wendet die ID-Linkpflicht nicht auf `.mk`-Fremd-IDs an; **kein**
  `.d-check.yml`-Exempt nötig). *(Diese Plan-Prosa nennt die kollidierenden
  Fremd-IDs bewusst ohne 4-stellige Nummer, damit die `ids`-Linkpflicht sie
  nicht fälschlich auf b-cads gleichnamige ADRs zu verlinken verlangt —
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2.)*

Dies ist dieselbe Klasse wie `make docs-check` (d-check) und `make schema-check`
(d-migrate): externe digest-gepinnte Images. Neu ist nur, dass `a-check` als
`gates`-Member **im Aggregat** läuft (docs-check ist COPY-Stage; a-check ist
Bind-Mount-Run — beide digest-gepinnt).

## Pre-Flight-Belege (vor Plan-Review erhoben, gepinntes v0.9.0-Image)

1. `docker run --rm --network none -v "$PWD":/src:ro <a-check@sha256:0378211f…> /src`
   gegen `8464d38`+`.a-check.yml` → **`gesamt: 0 Befund(e)`, Exit 0** (reproduziert
   die Projektinhaber-Verifikation unabhängig).
2. Gegenprobe a: `#include <QWidget>` in `src/hexagon/model/wall.h` →
   **`core-impurity: Kern importiert QWidget`, Exit 1**; sauber revertiert.
3. `make docs-check` mit `.a-check.yml`+`a-check.mk` im Baum → **169 Dateien,
   0 Befunde** (Fremd-ID-Stolperfalle materialisiert sich nicht).

## 1. Ziel

`make gates` fährt **a-check** (Image, netzlos, read-only; primäres
Architektur-Gate: A–E + Schicht-Kanten + driving/driven) **plus** den auf den
**P-Rest** geschrumpften `tools/arch-check.sh` (`dlopen`/`dlsym`/`dlclose`-Aufruf
+ feine P2-Allowlist). Die 0-Befunde-Latte bleibt; nichts wird gelockert.

## 2. Definition of Done

- [ ] `.a-check.yml` **byte-genau** (Task-Vorlage) in der Repo-Wurzel;
      Reihenfolge der `tech`-Einträge (`.hxx` **vor** `Q[A-Za-z]`-Regex),
      `adapter_sink`, `ui` ohne `direction`, `dlfcn.h`-`composition_root: forbid`,
      Scan-Wurzel `.` **nicht** verändert.
- [ ] `a-check.mk` **byte-genau** (digest-gepinnt v0.9.0) in der Repo-Wurzel;
      im `Makefile` per `include a-check.mk` eingebunden; `a-check` ins
      `gates`-Aggregat (`docs-check gate-consistency a-check arch-check lint
      test coverage-gate`).
- [ ] `tools/arch-check.sh` auf den **P-Rest** geschrumpft: es bleiben NUR
      (a) das `dlopen`/`dlsym`/`dlclose`-**Aufrufmuster** (ohne den `dlfcn.h`-Include
      — den deckt a-check) außerhalb `src/adapters/plugin/`, und (b) die feine
      **P2-Allowlist** (Quote-Include-Allowlist + Angle-Include-Verbot für
      `plugins/`+`src/plugin_api/`). Regel A/B/C/D/E + P2b (Qt/OCC/SQLite im
      `plugins/`-Baum) **entfernt** (a-check übernimmt). Header-Kommentar auf
      den Rest-Scope + a-check-Zuständigkeit nachgezogen.
- [ ] `tools/gate-consistency.sh` scannt zusätzlich `a-check.mk` (sonst meldet
      es `make a-check` fälschlich als halluziniert — der `a-check:`-Target lebt
      im Include, nicht im `Makefile`). Negativtest-Wirksamkeit erhalten.
- [ ] **Quell-/Test-Kommentar-Nachzug ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1):** die ~22 Live-Kommentare,
      die eine **A–E**-Regel `arch-check` zuschreiben (`src/adapters/io/*`,
      `src/adapters/geometry/*`, `src/adapters/persistence/*`,
      `src/hexagon/services/exchange_service.h`, `src/adapters/CMakeLists.txt`,
      `tests/**`), werden auf **a-check** umgelabelt; die **P1/P2**-Zuschreibungen
      (`src/adapters/plugin/plugin_host.*`, `src/plugin_api/CMakeLists.txt`,
      `src/adapters/CMakeLists.txt` dlfcn-Monopol-Zeilen) bleiben **arch-check**
      (P-Rest). `done/welle-*-results.md` sind historische Snapshots — unangetastet.
      Verhaltensneutral (nur Kommentare); `make test` unverändert grün.
- [ ] **Makefile-Kopf-Invariante ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-3):** der Kopfkommentar
      („jeder Gate ist eine COPY-Stage, kein Bind-Mount") erhält die
      a-check-Bind-Mount-Ausnahme (erster Bind-Mount-Member im Aggregat).
- [ ] **Gate-Verträge nachgezogen (Honesty):** [AGENTS.md §3](../../../../AGENTS.md)
      + [`harness/README.md`](../../../../harness/README.md) §Sensors (+ §Stand)
      erhalten eine **`a-check`-Zeile** (real; A–E + Kanten + Richtung; Bindung
      [ADR-0001](../../adr/0001-hexagonale-architektur.md)/[ADR-0002](../../adr/0002-geometrie-kern-opencascade.md)/[ADR-0003](../../adr/0003-persistenz-sqlite.md)/[ADR-0009](../../adr/0009-gui-framework-qt6.md)/[ADR-0017](../../adr/0017-plugin-api-abi.md)
      + [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check));
      die **`arch-check`-Zeile** wird auf den P-Rest umgeschrieben (Bindung
      [ADR-0017](../../adr/0017-plugin-api-abi.md)); die `gates`-Aggregatzeile
      nennt a-check.
- [ ] **[MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)**
      in `harness/conventions.md` (Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)): Geltungsbereich, Adaption,
      Begründung (kein §2.6-Fall), Bind-Mount-Abweichung, P-Rest-Abgrenzung,
      Auflösungs-Trigger.
- [ ] **Roadmap:** Quergewerk-Eintrag „Historische Trigger-Verschiebungen"
      (Umstellung vollzogen; welle-5-Sequenz unberührt).
- [ ] **Verifikation (real, Ausgabe berichtet):** (1) `make a-check` → 0 Befunde,
      Exit 0; (2) Gegenprobe a `QWidget` in `wall.h` → `core-impurity`, Exit 1,
      revert; (3) Gegenprobe b `dlopen(`-Aufruf außerhalb Plugin-Host → der
      geschrumpfte `arch-check.sh` feuert, revert; (4) `make gates` grün.
- [ ] CHANGELOG-Unreleased-Eintrag; Closure-Notiz; Lifecycle-Move nach
      `done-archive/` (reiner `git mv`, [§2.8](../../../../AGENTS.md)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.a-check.yml` | neu (byte-genau) | a-check-Config, Vollrichtung; **nicht aufräumen** (tech-Reihenfolge/adapter_sink semantisch) |
| `a-check.mk` | neu (byte-genau) | digest-gepinntes Gate-Snippet v0.9.0 |
| `Makefile` | ändern | `include a-check.mk`; `a-check` ins `gates`-Aggregat; `.PHONY`/help-Kohärenz |
| `tools/arch-check.sh` | schrumpfen | nur P-Rest (dlopen-Aufruf + P2-Allowlist); A/B/C/D/E/P2b entfernt; Header nachgezogen |
| `tools/gate-consistency.sh` | ändern | Target-Scan um `a-check.mk` erweitern (Include-Awareness) |
| `AGENTS.md` §3 | ändern | a-check-Zeile + arch-check-Zeile (P-Rest) + gates-Aggregat |
| `harness/README.md` §Stand/§Sensors | ändern | dito |
| `harness/conventions.md` | ändern | **[MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)** (Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Quergewerk-Eintrag |
| `CHANGELOG.md` | ändern (Closure) | Unreleased-Eintrag slice-030 |
| `docs/reviews/2026-07-04-slice-030-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |
| `src/**` + `tests/**` (Kommentare) | punktuell | ~22 A–E-Zuschreibungen „arch-check Regel …" → a-check ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1); P1/P2-Zuschreibungen bleiben arch-check; verhaltensneutral |

## 4. Trigger

- Startbar nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review
  (HIGHs blockieren). Fachliche Vorbedingung erfüllt: a-check v0.9.0 released,
  slice-028/029 (Struktur) geliefert, Pre-Flight 0 Befunde.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, beide Gegenproben belegt, Closure-Notiz →
  a-check-M3 „erster Pilot-Konsument" drüben schließbar; welle-5-Arbeit läuft
  unverändert weiter (slice-027 lint-Härtung bleibt geparkt startbar).

## 6. Risiken und offene Punkte

- **P-Rest-Vollständigkeit:** a-check deckt die `dlfcn.h`-Include-Kante und die
  Plugin-Import-**Kanten** (via `edges` `plugins → plugin_api/model/ports_driving`);
  der P-Rest hält nur, was a-check strukturell **nicht** sieht: den
  `dlopen(`-Funktionsaufruf und die feine Quote-vs-Angle-Allowlist.
  Gegenprobe b belegt, dass der Aufruf-Sensor nach dem Schrumpfen weiter feuert.
- **Bind-Mount statt COPY-Stage:** siehe §Bind-Mount-Abweichung — digest-Pin +
  `--network none` + `:ro` erhalten Reproduzierbarkeit/Hermetik; als [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)
  dokumentiert.
- **gate-consistency-Include-Awareness:** ohne den `a-check.mk`-Scan meldete das
  Konsistenz-Gate `make a-check` als halluziniert (Target lebt im Include).
  Der Scan-Zusatz ist minimal und lässt die Real-vs-geplant-Heuristik unberührt.
- **Selbst-Pin-Lag (Task-Hinweis):** `a-check --print-mk` aus dem v0.9.0-Image
  druckt bauartbedingt noch den v0.8.0-Pin; maßgeblich ist das byte-genau
  eingebettete `a-check.mk` (v0.9.0-Digest) — nicht neu generieren.
- **Immutable ADRs:** [ADR-0001](../../adr/0001-hexagonale-architektur.md) §Fitness
  nennt `make arch-check` (als „geplant" markiert, vor-Gate-Zeit). ADR-Body ist
  nach Accepted immutable; die **aktuelle** Bindung lebt in den Sensors-Tabellen
  + [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check), nicht im ADR-Body — kein Body-Edit (Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check), das ADRs unberührt ließ).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF (deklariert: `Makefile`, `tools/`, `.devcontainer/`;
  Konventions-Dichte hoch: Docker-only, gepinnte Toolchain, 0-Befunde-Gate).
  **Phase-Reife:** etabliert. **Evidenz-Risiko:** niedrig (a-check v0.9.0
  gegen `8464d38` verifiziert + Pre-Flight reproduziert). **Reconciliation:**
  Gate-Mechanismus-Wechsel via [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check) (Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)).

### Sub-Area: Konventionen & Harness-Doku

- **Modus:** GF; die [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)-Adaption + Sensors-Vertrags-Nachzug sind die
  Kern-Doku-Lieferung. **Risiko:** niedrig (Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check) etabliert).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; Quergewerk-Disziplin (Muster 018/022: Roadmap-Eintrag, kein
  Wellen-Feature, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  davor). **Risiko:** niedrig.
