# Planning — b-cad

Slice-Lifecycle: `open/` → `next/` → `in-progress/` → `done/`.

Reine `git mv`-Commits beim Wechsel zwischen Verzeichnissen — siehe Hard
Rule "git mv + Inhaltsänderung = zwei Commits" in
[`../../../AGENTS.md`](../../../AGENTS.md) §2.8.

## Lifecycle-Bedeutungen

| Verzeichnis | Bedeutung |
|---|---|
| `open/` | Geplant, noch nicht priorisiert. Keine Garantie auf Umsetzung. |
| `next/` | Als Nächstes priorisiert. |
| `in-progress/` | Branch / PR existiert. |
| `done/` | DoD erfüllt, gemerged, Closure-Notiz vorhanden. Hält die **lebenden** Abschluss-Artefakte (`*-results.md`, acc-002). |
| `done-archive/` | Abgeschlossene Slice-/Spike-**Pläne**, **eingefroren**. Existiert **allein als Gate-Mechanik**, nicht als eigenständige Lifecycle-Stufe: `.d-check.yml` nimmt den Pfad per `ids.scope.ignore` (`docs/plan/planning/done-archive`) aus dem Live-`ids`-Scan. **Grund:** die Slice-Pläne aus der Zeit **vor der d-check-Einführung** wurden nie für dessen `ids`-Linkpflicht geschrieben — blieben sie im Live-Korpus, müssten ihre `ids`-Links bei jeder Gate-Verschärfung (z. B. der `adr→slice`-Regel, [MR-014](../../../harness/conventions.md)) nachträglich angepasst werden. Der Freeze in `done-archive/` erspart das (slice-018a/b / [MR-011](../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)); abgeschlossene Pläne wandern generell per reinem `git mv` aus `in-progress/`/`done/` hierher. |

## Aktueller Stand (nach Greenfield-Bootstrap)

| Verzeichnis | Slices |
|---|---|
| `open/` | slice-006 (Drittanbieter-Attribution) |
| `next/` | — |
| `in-progress/` | `roadmap.md` (keine offenen Slices — slice-027/030/031 am 2026-07-04 geschlossen) |
| `done/` | **Lebende Referenz:** `welle-1-results.md` · `welle-1v-results.md` · `welle-2-results.md` · `welle-3-results.md` · `welle-4-results.md` + acc-002-Artefakte (Beleg/Befund). |
| `done-archive/` | **(eingefroren, slice-018a)** **welle-1-mvp:** slice-001 (Build-Skelett), slice-002 (Code-Gates), spike-001 (Toolchain-Reproduzierbarkeit), slice-005 (Gate-Consistency-Sensor), slice-003a (Domain-Kern & Wände, OCC-frei), slice-003b (OCC-Extrusion + arch-check Regel C), slice-004 (Toolchain-Pinning 26.04/node24 + Snapshot), slice-007 (Datenmodell-Definition + [ADR-0006](../adr/0006-relationales-schema-design.md)), slice-008a (Persistenz: SQLite-Adapter + arch-check Regel D), slice-008b (Persistenz-Härtung: Crash-Recovery + E-IO-Codes), slice-009a/009b (Raumerkennung: [ADR-0007](../adr/0007-raumerkennung-geometrie-basis.md) + Implementierung), slice-010a/010b (Echtzeit-Benachrichtigung: [ADR-0008](../adr/0008-aenderungs-benachrichtigung.md) + Implementierung). **welle-1v-viewer:** slice-011a/011b (Qt-6-Viewer: [ADR-0009](../adr/0009-gui-framework-qt6.md)/0010 + Implementierung), slice-012 (Eckenschluss WAL-006-Teilumfang). **welle-2-bauteile:** slice-013a (Bauteil-Hosting & Wandöffnung: [ADR-0011](../adr/0011-bauteil-hosting-wandoeffnung.md) + AK-Schärfung DOR/WIN), slice-013b (Türen + Fenster implementiert: Öffnung als boolesche Subtraktion), slice-013c (Öffnungs-Persistenz: openings/doors/windows-Round-Trip; [ADR-0011](../adr/0011-bauteil-hosting-wandoeffnung.md) vollständig), slice-014a (Dach: LH-FA-ROF AK-Schärfung + Spec-Geometrie, Teilumfang Rechteck), slice-014b (Dach implementiert: Sattel/Walm/Pult analytisch, Viewer folgt), slice-014c (Dach-Persistenz: roofs/footprint_json-Round-Trip), slice-015a (Decken+Fundament: LH-FA-SLB/FND AK-Schärfung + Spec-Platten-Geometrie), slice-015b (Decken+Fundament implementiert: Platten als Extrusion + Mesh-Translation auf base_z, Cutout-Boolean, SlabChanged-Viewer), slice-015c (Decken/Fundament-Persistenz: slabs-Round-Trip mit Cutouts), slice-016a (Treppen: AK-Schärfung + Spec-Geometrie, gerade einläufig), slice-016b (Treppen implementiert: analytisches Stufen-Polyeder + Geländer), slice-016c (Treppen-Persistenz: stairs-Round-Trip). **welle-3-auswertung:** slice-017a (Auswertung & Material: [ADR-0012](../adr/0012-evaluations-architektur.md) „Evaluations-Architektur" + EVL/MAT-Schärfung & Spec), slice-017b (EvaluatePort + Flächen EVL-001/003), slice-017c–g (Volumen EVL-002 analytisch + Material-System MAT-001/002/003/005/006 + Listen EVL-004/005/006 + Kosten MAT-006). **harness-steering:** slice-018a (Doku-Referenz-Gate: done-archive-Mechanik + matrix/ids, [MR-011](../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)), slice-018b (Voll-Korpus-Link-Hygiene: alle 7 ID-Familien, Linker idlink), slice-018c (Per-ID-Anker-Präzision: Inline-HTML-Anker, d-check v0.9.0). **welle-4-austausch:** slice-019a/b/c (IFC Import+Export: [ADR-0013](../adr/0013-ifc-bibliothek.md) selbst getragener SPF-Subset-Codec, io-resident, [ACC-003](../../../spec/lastenheft.md#7-abnahmekriterien)), slice-020a/b (STEP/STL-Export: [ADR-0014](../adr/0014-step-stl-export-backend.md) OCC-DataExchange nativ, geometrie-resident), slice-021a/b (DXF Import+Export: [ADR-0015](../adr/0015-dxf-backend.md) selbst getragener Subset-Codec, 2D-Grundriss), slice-022 (io-smoke: CI-only Binary-Smoke aller IO-Formate), slice-023a/b/c (Dach-Volumen: [LH-FA-ROF-006](../../../spec/lastenheft.md#lh-fa-rof-006) Dachdicke/Volumenkörper — AK+Geometrie [wasserdichter Schräg-Slab] + Persistenz `roofs.thickness_mm`), slice-024a/b (STEP-B-Rep Dächer [Vernähung] + Treppen [analytische Box-Solids] — alle 3D-Bauteile B-Rep), slice-025a (PDF/PNG: [ADR-0016](../adr/0016-pdf-png-backend.md) AK-Schärfung + Spec-Mapping, export-only/Achsen-Maßstabsplan), slice-025b (PDF-Export: self-rolled Vektor-Maßstabsplan 1:100, [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)), slice-025c (PNG-Export: self-rolled Raster-Grundriss — alle sechs welle-4-Format-Backends geliefert, M4). **welle-5-erweiterung:** slice-026a (Plugin-System: [ADR-0017](../adr/0017-plugin-api-abi.md) AK-Schärfung [LH-FA-PLG-001](../../../spec/lastenheft.md#lh-fa-plg-001)..004 + Spec-Mapping §1/§4/§5/§6, Ehrlichkeits-Klausel/ein Code-zwei Events), slice-026b (Plugin-System implementiert: Plugin-Host `src/adapters/plugin/` + Plugin-API `src/plugin_api/` + `plugins/`-Baum + arch-check-Regel P; Symbol-Naht `ENABLE_EXPORTS`, reale-`.so`-AK-Tests — [OBJ-004](../../../spec/lastenheft.md#3-projektziele)/M5-Pfad frei), slice-028 (a-check-Vorbereitung: reine Berechnungs-Kerne → `src/hexagon/services/geometry/`, Adapter-Kante deklariert, architecture-Tabelle geheilt), slice-029 (a-check-Vorbereitung: ui richtungs-rein — `ui/view/` driven + `ui/command/` driving mit port-freier `MeshSource`-Naht durch das [ADR-0008](../adr/0008-aenderungs-benachrichtigung.md)-Muster), slice-030 (Architektur-Gate-Umstellung arch-check → a-check: [MR-013](../../../harness/conventions.md#mr-013--arch-check-via-a-check) externes digest-gepinntes Image als primäres Gate [Kern-Reinheit/laterale Adapter/Tech-Kapselung/Schicht-Kanten/Richtung], `tools/arch-check.sh` → P-Rest [`dlopen`-Aufruf + Plugin-Import-Allowlist]; erster a-check-Pilot-Konsument), slice-027 (lint-Härtung: evidence-first sieben clang-tidy-Familien scharf, 8 Checks + 10 Fixes, 24 begründete Auslassungen), slice-031 (lint-Härtung II: `misc-const-correctness` + `modernize-use-nodiscard` scharf, 23 Fixes) |

slice-001/002 sind abgeschlossen (DoD erfüllt, Closure-Notiz vorhanden;
die `make gates`-Verifikation ist dort als Punkt-in-Zeit belegt — der
aktuelle Lauf-Status gehört in CI, nicht in diese Doku, Modul 13).
spike-001 hat die Toolchain-Reproduzierbarkeit untersucht → [ADR-0004](../adr/0004-toolchain-dependency-pinning.md)
(Proposed) + slice-004 (Digest+Snapshot-Pinning, Migration 26.04/node24).
slice-003a (Domain-Kern & Wände, OCC-frei) ist abgeschlossen; der
ursprüngliche slice-003 wurde in 003a (Kern) + 003b (OCC-Extrusion)
geschnitten (Roadmap §Historische Trigger-Verschiebungen). slice-003b
(erster OCC-Code + arch-check Regel C), slice-004 (gepinnte 26.04/node24-
Toolchain + apt-Snapshot), slice-007 (Datenmodell-Definition,
d-migrate-validiert + [ADR-0006](../adr/0006-relationales-schema-design.md)) und slice-008a (SQLite-Persistenz hinter
`ProjectRepositoryPort`, atomar + arch-check Regel D) und slice-008b
(Persistenz-Härtung: Crash-Recovery `kill -9` + [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/[`E-IO-002`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) sind
abgeschlossen; offen: slice-006.

## Roadmap

Siehe [`in-progress/roadmap.md`](in-progress/roadmap.md).
