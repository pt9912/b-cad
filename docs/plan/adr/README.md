# ADR-Index — b-cad

| ID | Titel | Status | Bezug |
|---|---|---|---|
| [0001](0001-hexagonale-architektur.md) | Hexagonale Architektur (Ports & Adapters) | Accepted (2026-06-08) | OBJ-002/003/004/005, LH-FA-* |
| [0002](0002-geometrie-kern-opencascade.md) | Geometrie-Kern OpenCascade hinter `GeometryKernelPort` | Proposed (2026-06-08) | REQ-TEC-003, LH-FA-WAL/D3-* |
| [0003](0003-persistenz-sqlite.md) | Projekt-Persistenz SQLite, atomar | Proposed (2026-06-08) | REQ-TEC-007, LH-FA-BLD-*, LH-QA-005 |
| [0004](0004-toolchain-dependency-pinning.md) | Container-/Dependency-Pinning + Base-Version (24.04→26.04, node 24, Digest+Snapshot) | Proposed (2026-06-08) | REQ-TEC-009, Modul 14, spike-001 |

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
- Atomare Write-Strategie / Crash-Recovery-Detail (LH-QA-005) — welle-1.
- Observability/OTel-Anbindung (`TracingPort`) — spätere Welle.
- ~~Build-/Container-Strategie (REQ-TEC-009)~~ → als [ADR-0004](0004-toolchain-dependency-pinning.md) erfasst (Proposed, aus spike-001).
