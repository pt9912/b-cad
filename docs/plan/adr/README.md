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
| [0008](0008-aenderungs-benachrichtigung.md) | Änderungs-Benachrichtigung Kern → Darstellung (Observer-Port, Push-Notify/Pull-State, Kapselung) | Accepted (2026-06-11) | LH-FA-D3-002, OBJ-003, ADR-0001/0007 |
| [0009](0009-gui-framework-qt6.md) | GUI-Framework-Bindung Qt 6 (Widgets, Tessellation über `ViewModelPort` — kein OCC in der GUI, Regel E, Headless-Strategie) | Accepted (2026-06-12) | REQ-TEC-002, ACC-002, LH-FA-D3-001/002, ADR-0001/0002/0008 |
| [0010](0010-headless-gl-xvfb.md) | Headless-GL via Xvfb + Mesa/llvmpipe — präzisiert ADR-0009 (f): offscreen-QPA trägt kein GL (Implementierungs-Befund slice-011b) | Accepted (2026-06-12) | ADR-0009, ADR-0004, LH-FA-D3-002 |
| [0011](0011-bauteil-hosting-wandoeffnung.md) | Bauteil-Hosting & Wandöffnungs-Modell (wand-gehostetes Element mit Wand-Referenz; Kern liefert Schnitt-Prismen, `GeometryKernelPort` subtrahiert; `WallGeometryChanged`-Wiederverwendung; Raumerkennung unberührt; Bauteil-Erweiterungs-Muster als welle-2-Leitplanke) | Accepted (2026-06-13) | LH-FA-DOR-001..004, LH-FA-WIN-001..005, ADR-0001/0002/0006/0007/0008 |

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
| ADR-0008 | Observer-Port + subscribe/unsubscribe + Meldungen im `StructureEditService` (nach `redetectRooms`), inkl. Kapselungs-Test (werfender Beobachter) | **erfüllt** durch slice-010b (2026-06-11) |
| ADR-0009 | `arch-check` **Regel E** (Qt-Includes nur `src/adapters/ui/` + `src/main.cpp`) + Gate-Doku-Nachzug (`harness/README.md` §Sensors, `AGENTS.md` §3) + `acc-002-beleg`-Target außerhalb `gates` | **erfüllt** durch slice-011b (2026-06-12) |
| ADR-0011 | Öffnungs-Domänenmodell (Tür/Fenster/Opening, pure Werte) + `EditStructurePort`-Operationen + `GeometryKernelPort`-Schnitt-Prismen (OCC-`BRepAlgoAPI_Cut`, Regel C) + `WallGeometryChanged` der Wirtswand (total/transaktional) + `ViewerScene`-Folgen; AK-Tests `LH-FA-DOR-*`/`LH-FA-WIN-*` | **Geometrie/Verhalten erfüllt** durch slice-013b (2026-06-13) |
| ADR-0011 | Persistenz-Abbildung der Öffnungen (`openings`/`doors`/`windows`, ADR-0006-Schema) — Round-Trip Speichern/Laden | **erfüllt** durch slice-013c (2026-06-13) |
| ADR-0011 (#6) | Bauteil-Erweiterungs-Muster fürs **Dach** (LH-FA-ROF-*): Domäne + analytisches Netz (`roof_geometry`) + ViewModel/Viewer/Edit-Ops + Persistenz (`roofs`/`footprint_json`) | **erfüllt** durch slice-014a/b/c (2026-06-13) |
| ADR-0011 (#6) | Bauteil-Erweiterungs-Muster für **Decken/Fundament** (LH-FA-SLB-*/FND-*): Domäne + Platten-Geometrie (`slab_geometry`, base_z via Mesh-Translation) + ViewModel/Viewer/Edit-Ops + Persistenz (`slabs`/`polygon_json` mit Cutouts) | **erfüllt** durch slice-015a/b/c (2026-06-14) |

## Konventionen

- ADRs sind nach `Accepted` **immutable** (Hard Rule, siehe
  [`AGENTS.md` §2.5](../../../AGENTS.md)). Schärfungen entstehen als
  neue ADR mit `Supersedes ADR-NN`.
- Eine ADR im Status `Proposed` darf während des Slice-Reviews iteriert
  werden; bei `Accepted` wird dieser Index aktualisiert (Status, Datum).
- Gates dürfen nicht ohne ADR gelockert werden (siehe `AGENTS.md` §2.6).

## Offene ADR-Themen (emergieren aus der Architektur-Outline)

Noch nicht als ADR angelegt, in der Roadmap verortet:

- ~~GUI-Framework-Bindung Qt 6 (Driving Adapter)~~ → entschieden in
  [ADR-0009](0009-gui-framework-qt6.md) (Accepted 2026-06-12,
  slice-011a); Folgepflichten → slice-011b.
- Plugin-API-/ABI-Vertrag und Sandbox-Modell (LH-FA-PLG-*) — welle-5.
- IFC-Bibliothek und -Schema-Version (LH-FA-IO-001/002) — welle-4.
- STEP-/Format-Export-Backend hinter `ModelExporterPort` (LH-FA-IO-005;
  aus ADR-0002 ausgegliedert) inkl. Adapter-Grenzen Geometrie↔IO — welle-4.
- ~~Atomare Write-Strategie / Crash-Recovery-Detail (LH-QA-005)~~ →
  entschieden in [ADR-0003](0003-persistenz-sqlite.md) (Temp+Rename);
  Folgepflicht durch slice-008b erfüllt (Welle-1-Verifikation 2026-06-11).
- Observability/OTel-Anbindung (`TracingPort`) — spätere Welle.
- ~~Build-/Container-Strategie (REQ-TEC-009)~~ → als [ADR-0004](0004-toolchain-dependency-pinning.md) erfasst (Accepted 2026-06-09, aus spike-001).
