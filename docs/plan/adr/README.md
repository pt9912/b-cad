# ADR-Index — b-cad

| ID | Titel | Status | Bezug |
|---|---|---|---|
| [0001](0001-hexagonale-architektur.md) | Hexagonale Architektur (Ports & Adapters) | Accepted (2026-06-08) | OBJ-002/003/004/005, LH-FA-* |
| [0002](0002-geometrie-kern-opencascade.md) | Geometrie-Kern OpenCascade hinter `GeometryKernelPort` (Backend: Solids/Extrusion/Booleans/Wandöffnungen) | Accepted (2026-06-09) | REQ-TEC-003, LH-FA-WAL/D3-*, LH-FA-DOR-004/WIN-005 |
| [0003](0003-persistenz-sqlite.md) | Projekt-Persistenz SQLite, atomar | Accepted (2026-06-09) | REQ-TEC-007, LH-FA-BLD-*, LH-QA-005 |
| [0004](0004-toolchain-dependency-pinning.md) | Container-/Dependency-Pinning + Base-Version (24.04→26.04, node 24, Digest+Snapshot) | Accepted (2026-06-09) | REQ-TEC-009, Modul 14, spike-001 |
| [0005](0005-drittanbieter-lizenz-attribution.md) | Drittanbieter-Lizenz-Attribution & Auslieferungs-Layout (ScanCode+REUSE, kuratiertes Manifest, dist-Layout) | Proposed (2026-06-08) | LH-QA-007 (vorgeschlagen), ADR-0002/0004, slice-006 |
| [0006](0006-relationales-schema-design.md) | Relationales Schema-Design des Gebäudemodells (per-Typ-Tabellen, `openings`-Spezialisierung, JSON-Geometrie, persistierter Undo-Stack) | Accepted (2026-06-09) | OBJ-003, ADR-0001/0003, LH-QA-003 |
| [0007](0007-raumerkennung-geometrie-basis.md) | Geometrie-Basis der Raumerkennung (Innenkante, Ring-Modell, Erkennung total) | Accepted (2026-06-11) | LH-FA-ROM-001/002/003, LH-FA-EVL-003, ADR-0001/0006 |

## ADR-Folgepflichten (Status)

Benannte Folge-Aufgaben aus akzeptierten ADRs. Da `Accepted`-ADRs
**immutable** sind, wird ihr *Erfüllungsstatus* hier (mutabler Index)
geführt — nicht im ADR-Body. Das Abhaken einer Folge-Aufgabe ändert die
Entscheidung nicht und braucht daher keine Supersedes-ADR.

| ADR | Folgepflicht | Status |
|---|---|---|
| ADR-0002 | `arch-check` **Regel C** (OCC-`.hxx` nur in `src/adapters/geometry/`) | **erfüllt** durch [slice-003b](../planning/done/slice-003b-occ-extrusion.md) (2026-06-09) |
| ADR-0003 | `arch-check` **Regel D** (`sqlite3*` nur in `src/adapters/persistence/`) | **erfüllt** durch slice-008a (2026-06-09) |
| ADR-0003 | Crash-Recovery-Test (`kill -9`, LH-QA-005) | **erfüllt** durch slice-008b (2026-06-09) |
| ADR-0007 | Innenkanten-Offset + Ring-Modell implementieren; Boundary-Test prüft Netto-Fläche verschachtelter Wandzüge | **erfüllt** durch [slice-009b](../planning/done/slice-009b-raumerkennung-implementierung.md) (2026-06-11) |

## Konventionen

- ADRs sind nach `Accepted` **immutable** (Hard Rule, siehe
  [`AGENTS.md` §2.5](../../../AGENTS.md)). Schärfungen entstehen als
  neue ADR mit `Supersedes ADR-NN`.
- Eine ADR im Status `Proposed` darf während des Slice-Reviews iteriert
  werden; bei `Accepted` wird dieser Index aktualisiert (Status, Datum).
- Gates dürfen nicht ohne ADR gelockert werden (siehe `AGENTS.md` §2.6).

## Offene ADR-Themen (emergieren aus der Architektur-Outline)

Noch nicht als ADR angelegt, in der Roadmap verortet:

- GUI-Framework-Bindung Qt 6 (Driving Adapter) — welle-1/5.
- Plugin-API-/ABI-Vertrag und Sandbox-Modell (LH-FA-PLG-*) — welle-5.
- IFC-Bibliothek und -Schema-Version (LH-FA-IO-001/002) — welle-4.
- STEP-/Format-Export-Backend hinter `ModelExporterPort` (LH-FA-IO-005;
  aus ADR-0002 ausgegliedert) inkl. Adapter-Grenzen Geometrie↔IO — welle-4.
- Atomare Write-Strategie / Crash-Recovery-Detail (LH-QA-005) — welle-1.
- Observability/OTel-Anbindung (`TracingPort`) — spätere Welle.
- ~~Build-/Container-Strategie (REQ-TEC-009)~~ → als [ADR-0004](0004-toolchain-dependency-pinning.md) erfasst (Proposed, aus spike-001).
