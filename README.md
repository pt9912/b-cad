# b-cad

**b-cad** ist eine Desktop-Anwendung zur Erstellung, Bearbeitung,
Analyse und Visualisierung von **Wohngebäuden** — Einfamilien- und
Mehrfamilienhäuser, Anbauten, Garagen, Nebengebäude. Gebäude werden
**parametrisch** modelliert; 2D- und 3D-Darstellung leiten sich aus
**einem durchgängigen Datenmodell** ab. Zielgruppe: private Bauherren
*und* professionelle Planer.

> **Projekt-Status:** Greenfield-Bootstrap abgeschlossen — Spec, ADRs
> und Roadmap stehen, **Code folgt** (erste Welle `welle-1-mvp`). Siehe
> [`harness/README.md`](harness/README.md).

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
| Persistenz | SQLite (atomar, hinter Port) | [ADR-0003](docs/plan/adr/0003-persistenz-sqlite.md) |
| Build | CMake | REQ-TEC-004 |
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
├── Makefile                  Gates (real: docs-check, build; Code-Gates folgen)
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
│   ├── adapters/             Qt/OCC/SQLite (Skelett-Proben ab slice-001)
│   └── main.cpp              Composition Root
├── tests/                    GoogleTest (Kern-Smoke-Test ab slice-001)
├── tools/                    docs-check (Doku-Link-Validator) + Dockerfile
└── docs/
    ├── glossar.md
    ├── user/releasing.md
    └── plan/
        ├── adr/              ADR-Index + ADR-0001..0003
        ├── planning/         Slice-Lifecycle (open/next/in-progress/done) + Roadmap
        └── carveouts/        dokumentierte Gate-Ausnahmen (derzeit keine)
```

> Stand: **slice-001** (Build-Skelett & DevContainer) umgesetzt; `make build`
> grün. Fachlogik (Domain, Wände, Extrusion) folgt ab slice-003 — siehe
> [`spec/architecture.md` §2.1](spec/architecture.md#21-verzeichnis--und-build-struktur).

## Quick start (für Agenten und Menschen)

1. [`harness/README.md`](harness/README.md) lesen.
2. [`spec/lastenheft.md`](spec/lastenheft.md) für das *was*,
   [`docs/plan/adr/README.md`](docs/plan/adr/README.md) für das *warum so*.
3. Aktuelle Welle: [`docs/plan/planning/in-progress/roadmap.md`](docs/plan/planning/in-progress/roadmap.md).
4. Offene Slices: [`docs/plan/planning/open/`](docs/plan/planning/open/).

## Lizenz

MIT — siehe [`LICENSE`](LICENSE). `tools/docs-check.js` ist aus dem
AI-Harness-Kurs übernommen (ebenfalls MIT, [`harness/conventions.md` MR-003](harness/conventions.md#mr-003-docs-check-als-vendored-doku-sensor)).
