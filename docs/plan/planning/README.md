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
| `done-archive/` | Abgeschlossene Slice-/Spike-**Pläne**, **eingefroren** (Referenz-Integrität, slice-018a / [`MR-011`](../../../harness/conventions.md)). Per reinem `git mv` aus `done/` archiviert, damit slice-018b den Voll-Korpus-`ids` auf den Live-Korpus begrenzen kann. |

## Aktueller Stand (nach Greenfield-Bootstrap)

| Verzeichnis | Slices |
|---|---|
| `open/` | slice-006 (Drittanbieter-Attribution) |
| `next/` | — |
| `in-progress/` | — (nur `roadmap.md`) |
| `done/` | **Lebende Referenz:** `welle-1-results.md` · `welle-1v-results.md` · `welle-2-results.md` · `welle-3-results.md` + acc-002-Artefakte (Beleg/Befund). |
| `done-archive/` | **(eingefroren, slice-018a)** **welle-1-mvp:** slice-001 (Build-Skelett), slice-002 (Code-Gates), spike-001 (Toolchain-Reproduzierbarkeit), slice-005 (Gate-Consistency-Sensor), slice-003a (Domain-Kern & Wände, OCC-frei), slice-003b (OCC-Extrusion + arch-check Regel C), slice-004 (Toolchain-Pinning 26.04/node24 + Snapshot), slice-007 (Datenmodell-Definition + [ADR-0006](../adr/0006-relationales-schema-design.md)), slice-008a (Persistenz: SQLite-Adapter + arch-check Regel D), slice-008b (Persistenz-Härtung: Crash-Recovery + E-IO-Codes), slice-009a/009b (Raumerkennung: [ADR-0007](../adr/0007-raumerkennung-geometrie-basis.md) + Implementierung), slice-010a/010b (Echtzeit-Benachrichtigung: [ADR-0008](../adr/0008-aenderungs-benachrichtigung.md) + Implementierung). **welle-1v-viewer:** slice-011a/011b (Qt-6-Viewer: [ADR-0009](../adr/0009-gui-framework-qt6.md)/0010 + Implementierung), slice-012 (Eckenschluss WAL-006-Teilumfang). **welle-2-bauteile:** slice-013a (Bauteil-Hosting & Wandöffnung: [ADR-0011](../adr/0011-bauteil-hosting-wandoeffnung.md) + AK-Schärfung DOR/WIN), slice-013b (Türen + Fenster implementiert: Öffnung als boolesche Subtraktion), slice-013c (Öffnungs-Persistenz: openings/doors/windows-Round-Trip; [ADR-0011](../adr/0011-bauteil-hosting-wandoeffnung.md) vollständig), slice-014a (Dach: LH-FA-ROF AK-Schärfung + Spec-Geometrie, Teilumfang Rechteck), slice-014b (Dach implementiert: Sattel/Walm/Pult analytisch, Viewer folgt), slice-014c (Dach-Persistenz: roofs/footprint_json-Round-Trip), slice-015a (Decken+Fundament: LH-FA-SLB/FND AK-Schärfung + Spec-Platten-Geometrie), slice-015b (Decken+Fundament implementiert: Platten als Extrusion + Mesh-Translation auf base_z, Cutout-Boolean, SlabChanged-Viewer), slice-015c (Decken/Fundament-Persistenz: slabs-Round-Trip mit Cutouts), slice-016a (Treppen: AK-Schärfung + Spec-Geometrie, gerade einläufig), slice-016b (Treppen implementiert: analytisches Stufen-Polyeder + Geländer), slice-016c (Treppen-Persistenz: stairs-Round-Trip). **welle-3-auswertung:** slice-017a (Auswertung & Material: [ADR-0012](../adr/0012-evaluations-architektur.md) „Evaluations-Architektur" + EVL/MAT-Schärfung & Spec), slice-017b (EvaluatePort + Flächen EVL-001/003), slice-017c–g (Volumen EVL-002 analytisch + Material-System MAT-001/002/003/005/006 + Listen EVL-004/005/006 + Kosten MAT-006). **harness-steering:** slice-018a (Doku-Referenz-Gate: done-archive-Mechanik + matrix/ids, [MR-011](../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)), slice-018b (Voll-Korpus-Link-Hygiene: alle 7 ID-Familien, Linker idlink), slice-018c (Per-ID-Anker-Präzision: Inline-HTML-Anker, d-check v0.9.0). **welle-4-austausch:** slice-019a/b/c (IFC Import+Export: [ADR-0013](../adr/0013-ifc-bibliothek.md) selbst getragener SPF-Subset-Codec, io-resident, [ACC-003](../../../spec/lastenheft.md#7-abnahmekriterien)), slice-020a/b (STEP/STL-Export: [ADR-0014](../adr/0014-step-stl-export-backend.md) OCC-DataExchange nativ, geometrie-resident), slice-021a/b (DXF Import+Export: [ADR-0015](../adr/0015-dxf-backend.md) selbst getragener Subset-Codec, 2D-Grundriss), slice-022 (io-smoke: CI-only Binary-Smoke aller IO-Formate), slice-023a/b/c (Dach-Volumen: [LH-FA-ROF-006](../../../spec/lastenheft.md#lh-fa-rof-006) Dachdicke/Volumenkörper — AK+Geometrie [wasserdichter Schräg-Slab] + Persistenz `roofs.thickness_mm`), slice-024a/b (STEP-B-Rep Dächer [Vernähung] + Treppen [analytische Box-Solids] — alle 3D-Bauteile B-Rep) |

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
