# ADR-Index — b-cad

| ID | Titel | Status | Bezug |
|---|---|---|---|
| [0001](0001-hexagonale-architektur.md) | Hexagonale Architektur (Ports & Adapters) | Accepted (2026-06-08) | [OBJ-002](../../../spec/lastenheft.md#3-projektziele)/003/004/005, LH-FA-* |
| [0002](0002-geometrie-kern-opencascade.md) | Geometrie-Kern OpenCascade hinter `GeometryKernelPort` (Backend: Solids/Extrusion/Booleans/Wandöffnungen) | Accepted (2026-06-09) | [REQ-TEC-003](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec), LH-FA-WAL/D3-*, [LH-FA-DOR-004](../../../spec/lastenheft.md#lh-fa-dor-004--wandöffnung-automatisch-erzeugen)/WIN-005 |
| [0003](0003-persistenz-sqlite.md) | Projekt-Persistenz SQLite, atomar | Accepted (2026-06-09) | [REQ-TEC-007](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec), LH-FA-BLD-*, [LH-QA-005](../../../spec/lastenheft.md#lh-qa-005--crash-recovery) |
| [0004](0004-toolchain-dependency-pinning.md) | Container-/Dependency-Pinning + Base-Version (24.04→26.04, node 24, Digest+Snapshot) | Accepted (2026-06-09) | [REQ-TEC-009](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec), Modul 14, spike-001 |
| [0005](0005-drittanbieter-lizenz-attribution.md) | Drittanbieter-Lizenz-Attribution & Auslieferungs-Layout (ScanCode+REUSE, kuratiertes Manifest, dist-Layout) | Proposed (2026-06-08) | [LH-QA-007](../../../spec/lastenheft.md) (vorgeschlagen), ADR-0002/0004, slice-006 |
| [0006](0006-relationales-schema-design.md) | Relationales Schema-Design des Gebäudemodells (per-Typ-Tabellen, `openings`-Spezialisierung, JSON-Geometrie, persistierter Undo-Stack) | Accepted (2026-06-09) | [OBJ-003](../../../spec/lastenheft.md#3-projektziele), ADR-0001/0003, [LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo) |
| [0007](0007-raumerkennung-geometrie-basis.md) | Geometrie-Basis der Raumerkennung (Innenkante, Ring-Modell, Erkennung total) | Accepted (2026-06-11) | [LH-FA-ROM-001](../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen)/002/003, [LH-FA-EVL-003](../../../spec/lastenheft.md#lh-fa-evl-003--wohnflächenberechnung), ADR-0001/0006 |
| [0008](0008-aenderungs-benachrichtigung.md) | Änderungs-Benachrichtigung Kern → Darstellung (Observer-Port, Push-Notify/Pull-State, Kapselung) | Accepted (2026-06-11) | [LH-FA-D3-002](../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung), [OBJ-003](../../../spec/lastenheft.md#3-projektziele), ADR-0001/0007 |
| [0009](0009-gui-framework-qt6.md) | GUI-Framework-Bindung Qt 6 (Widgets, Tessellation über `ViewModelPort` — kein OCC in der GUI, Regel E, Headless-Strategie) | Accepted (2026-06-12) | [REQ-TEC-002](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec), [ACC-002](../../../spec/lastenheft.md#7-abnahmekriterien), [LH-FA-D3-001](../../../spec/lastenheft.md#modul-3d-modellierung-d3)/002, ADR-0001/0002/0008 |
| [0010](0010-headless-gl-xvfb.md) | Headless-GL via Xvfb + Mesa/llvmpipe — präzisiert ADR-0009 (f): offscreen-QPA trägt kein GL (Implementierungs-Befund slice-011b) | Accepted (2026-06-12) | ADR-0009, ADR-0004, [LH-FA-D3-002](../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung) |
| [0011](0011-bauteil-hosting-wandoeffnung.md) | Bauteil-Hosting & Wandöffnungs-Modell (wand-gehostetes Element mit Wand-Referenz; Kern liefert Schnitt-Prismen, `GeometryKernelPort` subtrahiert; `WallGeometryChanged`-Wiederverwendung; Raumerkennung unberührt; Bauteil-Erweiterungs-Muster als welle-2-Leitplanke) | Accepted (2026-06-13) | [LH-FA-DOR-001](../../../spec/lastenheft.md#lh-fa-dor-001--tür-platzieren)..004, [LH-FA-WIN-001](../../../spec/lastenheft.md#lh-fa-win-001--fenster-platzieren)..005, ADR-0001/0002/0006/0007/0008 |
| [0012](0012-evaluations-architektur.md) | Evaluations-Architektur (neuer `EvaluatePort` read-only Query; pure Ergebnis-Werttypen; pull, kein `op`; Fläche = Shoelace-Raum-Netto / Volumen analytisch im Kern — geklemmtes Öffnungsvolumen, **kein OCC-`GProp`**; Material nur konsumierte Eingabe; welle-3-Leitplanke) | Accepted (2026-06-14) | [LH-FA-EVL-001](../../../spec/lastenheft.md#lh-fa-evl-001--flächenberechnung)..006, ADR-0001/0006/0007/0011 |
| [0013](0013-ifc-bibliothek.md) | IFC-Bibliothek und -Schema-Version (IFC-SPF-Subset-Codec im IO-Adapter, **Option D** — kein Bibliotheks-Zukauf jetzt; Export IFC4 `IfcWall`+`IfcMaterialLayerSetUsage` / Import IFC4+IFC2x3-Subset; atomar/[`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder); Re-Eval auf IfcOpenShell/web-ifc; STEP/STL/DXF/PDF/PNG = Schwester-ADRs; welle-4-Leitplanke) | **Accepted** (2026-06-16) | [LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002, [ACC-003](../../../spec/lastenheft.md#7-abnahmekriterien), [OBJ-005](../../../spec/lastenheft.md#3-projektziele), ADR-0001/0002/0004/0005 |
| [0014](0014-step-stl-export-backend.md) | STEP-/STL-Export-Backend (**OCC-DataExchange nativ** — keine neue Dependency, ADR-0004-konform; geometrie-residenter `ModelExporterPort`, von **Regel C** gedeckt; STEP B-Rep / STL Mesh; atomar/[`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder); löst die ADR-0002-Ausgliederung „STEP-/Format-Export"; DXF/PDF/PNG = Schwester-ADRs) | **Accepted** (2026-06-17) | [LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005)/006, [OBJ-005](../../../spec/lastenheft.md#3-projektziele), ADR-0001/0002/0004/0005/0013 |

## ADR-Folgepflichten (Status)

Benannte Folge-Aufgaben aus akzeptierten ADRs. Da `Accepted`-ADRs
**immutable** sind, wird ihr *Erfüllungsstatus* hier (mutabler Index)
geführt — nicht im ADR-Body. Das Abhaken einer Folge-Aufgabe ändert die
Entscheidung nicht und braucht daher keine Supersedes-ADR.

| ADR | Folgepflicht | Status |
|---|---|---|
| ADR-0002 | `arch-check` **Regel C** (OCC-`.hxx` nur in `src/adapters/geometry/`) | **erfüllt** durch [slice-003b](../planning/done-archive/slice-003b-occ-extrusion.md) (2026-06-09) |
| ADR-0003 | `arch-check` **Regel D** (`sqlite3*` nur in `src/adapters/persistence/`) | **erfüllt** durch slice-008a (2026-06-09) |
| ADR-0003 | Crash-Recovery-Test (`kill -9`, [LH-QA-005](../../../spec/lastenheft.md#lh-qa-005--crash-recovery)) | **erfüllt** durch slice-008b (2026-06-09) |
| ADR-0007 | Innenkanten-Offset + Ring-Modell implementieren; Boundary-Test prüft Netto-Fläche verschachtelter Wandzüge | **erfüllt** durch [slice-009b](../planning/done-archive/slice-009b-raumerkennung-implementierung.md) (2026-06-11) |
| ADR-0008 | Observer-Port + subscribe/unsubscribe + Meldungen im `StructureEditService` (nach `redetectRooms`), inkl. Kapselungs-Test (werfender Beobachter) | **erfüllt** durch slice-010b (2026-06-11) |
| ADR-0009 | `arch-check` **Regel E** (Qt-Includes nur `src/adapters/ui/` + `src/main.cpp`) + Gate-Doku-Nachzug (`harness/README.md` §Sensors, `AGENTS.md` §3) + `acc-002-beleg`-Target außerhalb `gates` | **erfüllt** durch slice-011b (2026-06-12) |
| ADR-0011 | Öffnungs-Domänenmodell (Tür/Fenster/Opening, pure Werte) + `EditStructurePort`-Operationen + `GeometryKernelPort`-Schnitt-Prismen (OCC-`BRepAlgoAPI_Cut`, Regel C) + `WallGeometryChanged` der Wirtswand (total/transaktional) + `ViewerScene`-Folgen; AK-Tests `LH-FA-DOR-*`/`LH-FA-WIN-*` | **Geometrie/Verhalten erfüllt** durch slice-013b (2026-06-13) |
| ADR-0011 | Persistenz-Abbildung der Öffnungen (`openings`/`doors`/`windows`, ADR-0006-Schema) — Round-Trip Speichern/Laden | **erfüllt** durch slice-013c (2026-06-13) |
| ADR-0011 (#6) | Bauteil-Erweiterungs-Muster fürs **Dach** (LH-FA-ROF-*): Domäne + analytisches Netz (`roof_geometry`) + ViewModel/Viewer/Edit-Ops + Persistenz (`roofs`/`footprint_json`) | **erfüllt** durch slice-014a/b/c (2026-06-13) |
| ADR-0011 (#6) | Bauteil-Erweiterungs-Muster für **Decken/Fundament** (LH-FA-SLB-*/FND-*): Domäne + Platten-Geometrie (`slab_geometry`, base_z via Mesh-Translation) + ViewModel/Viewer/Edit-Ops + Persistenz (`slabs`/`polygon_json` mit Cutouts) | **erfüllt** durch slice-015a/b/c (2026-06-14) |
| ADR-0011 (#6) | Bauteil-Erweiterungs-Muster für **Treppen** (LH-FA-STR-*): Domäne + analytische Treppen-Geometrie (`stair_geometry`, Stufen-Polyeder + Geländer, kein OCC) + ViewModel/Viewer/Edit-Ops + Persistenz (`stairs`, `rise_mm` write-derived) | **erfüllt** durch slice-016a/b/c (2026-06-14) |
| ADR-0012 | `architecture.md` §1.1 nachziehen — EVL-001..006 von `DetectRoomsPort` auf den neuen Driving-Port `EvaluatePort` | **erfüllt** durch slice-017a (2026-06-14) |
| ADR-0012 | `EvaluatePort` + Auswertungs-Service (Shoelace-Raum-Netto-Fläche, analytisches Netto-Volumen mit geklemmtem Öffnungsvolumen, Listen-Aggregation) + pure Ergebnis-Werttypen + AK-Tests `LH-FA-EVL-*`; **[MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)** für den EVL-Impl (geometrie-korrektheits-nah) | **erfüllt** durch slice-017b–g (welle-3-Closure 2026-06-16, [MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) für 017c) |
| ADR-0013 | `spec/spezifikation.md` §6 (Vertragstabelle »IFC Schema-Version offen«) + §7 Offene Punkte nachziehen — Schema-Version mit ADR entschieden (IFC4-Export / IFC2x3+4-Import-Subset) | **erfüllt** durch slice-019a (2026-06-16) |
| ADR-0013 | `arch-check`-**Regel** (Format-/IFC-Symbole nur in `src/adapters/io/`, analog Regel C/D/E) | **gegenstandslos für Option D** (slice-019b): C/D/E gaten **externe Bibliotheks-Header** (OCC/SQLite/Qt) — Option D hat **keine** externe IFC-Lib; die Isolation trägt **Regel A+B** (`arch-check` grün inkl. `io/`). **Umdatiert** auf den externen-Bibliotheks-Re-Eval (erst eine später adoptierte IFC-Lib braucht ein Header-Gate analog C/D/E) |
| ADR-0013 | **Import:** IFC-SPF-Subset-Codec im IO-Adapter (`SpfReader`) + `ModelImporterPort` + `ExchangeService` (Driving `ExchangeModelPort`) + AK-Tests [`LH-FA-IO-001`](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) + **Adapter-Pfad-Integrationstest** (Datei→Domain, [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch echten Adapter) | **erfüllt** durch slice-019b (2026-06-17) |
| ADR-0013 | **Export:** `ModelExporterPort` + IFC-SPF-Writer + Roundtrip-AK [`LH-FA-IO-002`](../../../spec/lastenheft.md#lh-fa-io-002) ([ACC-003](../../../spec/lastenheft.md#7-abnahmekriterien)) | **erfüllt** durch slice-019c (2026-06-17) |
| ADR-0014 | **Toolchain-Beleg:** OCC-DataExchange-Toolkits lösen im gepinnten Snapshot **ohne** neuen Paketmanager auf | **belegt** (Pre-Flight 2026-06-17): OCC **7.9.2**, `libocct-data-exchange-dev` **bereits** in `.devcontainer/Dockerfile` (kein neuer apt-Eintrag, keine ADR-0004-Berührung); `libTKDESTEP`/`libTKDESTL`/`libTKRWMesh` + `STEPControl_Writer`/`StlAPI_Writer`/`RWStl`-Header im `bcad:build`-Image vorhanden |
| ADR-0014 | **Impl:** geometrie-residenter `ModelExporterPort`-Adapter (`Building`→OCC-`TopoDS_Shape`→STEP/STL via OCC-DataExchange, atomar [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) + `ExchangeFormat`-Erweiterung/Dispatch + Composition-Root-Verdrahtung; AK-Tests [`LH-FA-IO-005`](../../../spec/lastenheft.md#lh-fa-io-005)/`006` + **Adapter-Pfad-Integrationstest** (Re-Read-Orakel, [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch echten Adapter). Zusätzliche OCC-Toolkits in `src/adapters/CMakeLists.txt` linken. Regel C deckt die Isolation (keine neue arch-check-Regel) | **offen** (Impl-Slice) |
| ADR-0014 | **AK-Schärfung + Spec-Nachzug:** [`LH-FA-IO-005`](../../../spec/lastenheft.md#lh-fa-io-005)/006 von Outline auf AK (lösungsfrei, [MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) + Spec §1-Mapping; **§7** STEP/STL-Offene-Punkte streichen, **§6** auf entschiedenen Stand (OCC-DataExchange nativ) — Präzedenz [ADR-0013](0013-ifc-bibliothek.md) → slice-019a | **offen** (Schärfungs-Slice) |

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
- ~~IFC-Bibliothek und -Schema-Version ([LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002)~~ → als
  **[ADR-0013](0013-ifc-bibliothek.md)** angelegt (**Proposed** 2026-06-16,
  Accept ausstehend) — welle-4.
- ~~STEP-/Format-Export-Backend hinter `ModelExporterPort` ([LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005);
  aus ADR-0002 ausgegliedert) inkl. Adapter-Grenzen Geometrie↔IO~~ → als
  **[ADR-0014](0014-step-stl-export-backend.md)** entschieden (**Accepted** 2026-06-17)
  — OCC-DataExchange nativ, geometrie-residente `ModelExporterPort`-Naht (Regel C);
  Folgepflichten unten; welle-4.
- ~~Atomare Write-Strategie / Crash-Recovery-Detail ([LH-QA-005](../../../spec/lastenheft.md#lh-qa-005--crash-recovery))~~ →
  entschieden in [ADR-0003](0003-persistenz-sqlite.md) (Temp+Rename);
  Folgepflicht durch slice-008b erfüllt (Welle-1-Verifikation 2026-06-11).
- Observability/OTel-Anbindung (`TracingPort`) — spätere Welle.
- ~~Build-/Container-Strategie ([REQ-TEC-009](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec))~~ → als [ADR-0004](0004-toolchain-dependency-pinning.md) erfasst (Accepted 2026-06-09, aus spike-001).
