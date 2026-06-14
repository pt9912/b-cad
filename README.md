# b-cad

> **Projekt-Status:** Welle `welle-3-auswertung` aktiv. **Meilenstein M2
> erreicht** — Kern-MVP (Wände, Raumerkennung, OCC-Extrusion, SQLite-Persistenz
> inkl. Crash-Recovery, Änderungs-Benachrichtigung; welle-1, M1), sichtbarer
> Qt-6-3D-Viewer (welle-1v, ACC-002) und **alle parametrischen Bauteile** —
> Türen/Fenster, Dach, Decken/Fundament, Treppen (welle-2) — sind implementiert.
> Aktuell: **Auswertungen** (Flächen/Volumen/Wohnfläche, Material) über einen
> read-only `EvaluatePort`. Einstieg: [`harness/README.md`](harness/README.md).

## Was ist b-cad?

**b-cad** ist eine Desktop-Anwendung zur Erstellung, Bearbeitung,
Analyse und Visualisierung von **Wohngebäuden** — Einfamilien- und
Mehrfamilienhäuser, Anbauten, Garagen, Nebengebäude. Gebäude werden
**parametrisch** modelliert; 2D- und 3D-Darstellung leiten sich aus
**einem durchgängigen Datenmodell** ab. Zielgruppe: private Bauherren
*und* professionelle Planer.

## Warum b-cad?

Wohngebäude-Planung bedient heute zwei getrennte Welten: geführte
Hausplaner für Laien und vollwertige CAD-Systeme für Profis. b-cad
richtet sich an **beide Rollen** mit *einem* Werkzeug
([`spec/lastenheft.md` §2/§3](spec/lastenheft.md)):

- **Private Bauherren** modellieren ihr Gebäude geführt und ohne
  CAD-Kenntnisse (OBJ-001) — Räume werden z. B. beim Schließen eines
  Wandzugs automatisch erkannt (LH-FA-ROM-001).
- **Architekten und Planer** führen vollständige Planungen durch und
  tauschen über offene Formate aus — IFC, DXF, STEP, STL (OBJ-005).
- **Erweiterbarkeit** über ein Plugin-System (OBJ-004) statt
  Funktions-Monolith.

## Kerngedanke

**Ein Modell, viele Sichten.** Jedes Bauteil ist parametrisch
(OBJ-002); Grundriss, Schnitt und 3D-Darstellung sind *abgeleitete
Sichten* auf dasselbe Gebäudemodell (OBJ-003) — es gibt keine zweite,
manuell synchron zu haltende Geometrie. Ändert sich ein Parameter
(Wandstärke, Geschosshöhe), folgen alle Sichten in Echtzeit
(LH-FA-D3-002).

Die Architektur verkörpert das: ein **framework-freier Domain-Kern**
(hexagonal, [ADR-0001](docs/plan/adr/0001-hexagonale-architektur.md))
hält das Gebäudemodell; Geometrie-Kern, GUI und Persistenz sind
austauschbare Adapter hinter Ports
([ADR-0002](docs/plan/adr/0002-geometrie-kern-opencascade.md),
[ADR-0003](docs/plan/adr/0003-persistenz-sqlite.md)).

## Was macht es vertrauenswürdig?

**Das Gebäudemodell ist das Wertobjekt — Datenverlust ist der
Ernstfall.** Die Hard Rules des Repos sind genau dort am schärfsten
([`harness/conventions.md` §Repo-Klasse](harness/conventions.md#repo-klasse)):

- **Atomares Speichern** via Temp+Rename — ein abgebrochener
  Schreibvorgang hinterlässt nie eine halbe Projektdatei
  (LH-FA-BLD-002, ADR-0003).
- **Crash-Recovery** ist getestet, nicht behauptet: ein
  `kill -9`-Test (fork+SIGKILL) gehört zur Test-Suite (LH-QA-005).
- **Definierte Fehler-Codes** (`E-IO-001`/`E-IO-002`, …) statt
  stiller Fehlschläge ([`spec/spezifikation.md`](spec/spezifikation.md)).

Auch der **Entstehungsprozess** ist abgesichert: Spec führt, Code folgt
— jede Anforderung trägt Akzeptanzkriterien (Happy/Boundary/Negative),
und jede Änderung passiert reale Gates (`make gates`: Doku-Konsistenz,
Architektur-Regeln, Lint, Tests, Coverage) in einer **gepinnten,
reproduzierbaren Toolchain**
([ADR-0004](docs/plan/adr/0004-toolchain-dependency-pinning.md)).

## Harness-Engineering

Dieses Repo ist nach dem **AI-Harness-Kurs** (Harness Engineering für
Coding Agents) aufgesetzt: Spec führt, Code folgt (Greenfield). Wer hier
— als Mensch oder AI-Agent — etwas ändert, **startet bei**
[`harness/README.md`](harness/README.md) und beachtet die Source
Precedence und Hard Rules in [`AGENTS.md`](AGENTS.md).

## Technischer Stack (Ziel)

| Bereich | Wahl | Entscheidung |
|---|---|---|
| Sprache | C++20 | REQ-TEC-001 |
| Architektur | hexagonal (Ports & Adapters) | [ADR-0001](docs/plan/adr/0001-hexagonale-architektur.md) |
| Geometrie-Kern | OpenCascade (hinter Port) | [ADR-0002](docs/plan/adr/0002-geometrie-kern-opencascade.md) |
| GUI | Qt 6 (Driving Adapter) | REQ-TEC-002 |
| Persistenz | SQLite (atomar, hinter Port) | [ADR-0003](docs/plan/adr/0003-persistenz-sqlite.md), [ADR-0006](docs/plan/adr/0006-relationales-schema-design.md) |
| Build | CMake | REQ-TEC-004, [ADR-0004](docs/plan/adr/0004-toolchain-dependency-pinning.md) |
| Tests | GoogleTest | REQ-TEC-005 |
| Observability | OpenTelemetry | REQ-TEC-006 |
| Plugins | Shared Libraries | REQ-TEC-008 |
| Container | Docker DevContainer | REQ-TEC-009 |

## Verzeichnisstruktur

```
b-cad/
├── README.md                 (diese Datei)
├── AGENTS.md                 Hard Rules + Source Precedence
├── LICENSE                   MIT
├── CHANGELOG.md              Keep a Changelog (MR-004)
├── Makefile                  Gate-Targets (make gates / make help; Liste: harness/README §Sensors)
├── CMakeLists.txt            hexagonale Target-Trennung (ADR-0001)
├── .devcontainer/            Qt6+OpenCascade+SQLite-Build (make build)
├── harness/
│   ├── README.md             Harness-Einstieg: Guides, Sensors, Safety
│   └── conventions.md        repo-lokale Strukturregeln (MR-*, Modus pro Sub-Area)
├── spec/
│   ├── lastenheft.md         LH-FA-*/LH-QA-*-Anforderungen, Akzeptanzkriterien
│   ├── spezifikation.md      Wertebereiche, Fehler-Codes, OTel-Spans
│   └── architecture.md       hexagonale Zerlegung, Ports, CMake-Targets
├── src/
│   ├── hexagon/              Kern (model/ ports/ services/) — framework-frei
│   ├── adapters/             Qt/OCC/SQLite (ui/ geometry/ persistence/ io/ plugin/)
│   └── main.cpp              Composition Root
├── tests/                    GoogleTest (hexagon/ adapters/ e2e/)
├── tools/                    Gate-Skripte (arch-check, gate-consistency, suppression-gate) + Dockerfile; docs-check via d-check (MR-007)
└── docs/
    ├── glossar.md
    ├── user/releasing.md
    └── plan/
        ├── adr/              ADR-Index + ADR-0001..0012
        ├── planning/         Slice-Lifecycle (open/next/in-progress/done) + Roadmap
        └── carveouts/        dokumentierte Gate-Ausnahmen (derzeit keine)
```

> Stand: **welle-3-auswertung aktiv** (M3 offen). Umgesetzt: Domain-Kern,
> Wände, Raumerkennung, OCC-Extrusion, SQLite-Persistenz inkl. Crash-Recovery
> und Änderungs-Benachrichtigung (welle-1, M1); Qt-6-3D-Viewer (welle-1v,
> ACC-002); **alle parametrischen Bauteile** — Türen/Fenster, Dach,
> Decken/Fundament, Treppen, je inkl. Persistenz (welle-2, M2, ADR-0011);
> erste Auswertungen — `EvaluatePort` + Flächen/Wohnfläche (slice-017a/b,
> ADR-0012). Offen: Volumen + Material-/Bauteil-Listen, slice-006 Attribution.
> Struktur:
> [`spec/architecture.md` §2.1](spec/architecture.md#21-verzeichnis--und-build-struktur).
> Aktueller Stand der Slices: [`docs/plan/planning/README.md`](docs/plan/planning/README.md).

## Quick start (für Agenten und Menschen)

1. [`harness/README.md`](harness/README.md) lesen.
2. [`spec/lastenheft.md`](spec/lastenheft.md) für das *was*,
   [`docs/plan/adr/README.md`](docs/plan/adr/README.md) für das *warum so*.
3. Aktuelle Welle: [`docs/plan/planning/in-progress/roadmap.md`](docs/plan/planning/in-progress/roadmap.md).
4. Offene Slices: [`docs/plan/planning/open/`](docs/plan/planning/open/).

## Lizenz

MIT — siehe [`LICENSE`](LICENSE). Die Doku-Validierung läuft über
[d-check](https://github.com/pt9912/d-check) (MIT, digest-gepinntes
Container-Image; Ablösung des vendorten Kurs-Validators:
[`harness/conventions.md` MR-007](harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)).
