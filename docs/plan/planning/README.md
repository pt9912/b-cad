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
| `done/` | DoD erfüllt, gemerged, Closure-Notiz vorhanden. |

## Aktueller Stand (nach Greenfield-Bootstrap)

| Verzeichnis | Slices |
|---|---|
| `open/` | slice-006 (Drittanbieter-Attribution) |
| `next/` | slice-013a (Bauteil-Hosting & Wandöffnung — ADR + AK-Schärfung DOR/WIN; erster Slice von welle-2-bauteile; MR-006-Review gelaufen 2026-06-13, implementierungsbereit) |
| `in-progress/` | — (nur `roadmap.md`) |
| `done/` | **welle-1-mvp:** slice-001 (Build-Skelett), slice-002 (Code-Gates), spike-001 (Toolchain-Reproduzierbarkeit), slice-005 (Gate-Consistency-Sensor), slice-003a (Domain-Kern & Wände, OCC-frei), slice-003b (OCC-Extrusion + arch-check Regel C), slice-004 (Toolchain-Pinning 26.04/node24 + Snapshot), slice-007 (Datenmodell-Definition + ADR-0006), slice-008a (Persistenz: SQLite-Adapter + arch-check Regel D), slice-008b (Persistenz-Härtung: Crash-Recovery + E-IO-Codes), slice-009a/009b (Raumerkennung: ADR-0007 + Implementierung), slice-010a/010b (Echtzeit-Benachrichtigung: ADR-0008 + Implementierung). **welle-1v-viewer:** slice-011a/011b (Qt-6-Viewer: ADR-0009/0010 + Implementierung), slice-012 (Eckenschluss WAL-006-Teilumfang) |

slice-001/002 sind abgeschlossen (DoD erfüllt, Closure-Notiz vorhanden;
die `make gates`-Verifikation ist dort als Punkt-in-Zeit belegt — der
aktuelle Lauf-Status gehört in CI, nicht in diese Doku, Modul 13).
spike-001 hat die Toolchain-Reproduzierbarkeit untersucht → ADR-0004
(Proposed) + slice-004 (Digest+Snapshot-Pinning, Migration 26.04/node24).
slice-003a (Domain-Kern & Wände, OCC-frei) ist abgeschlossen; der
ursprüngliche slice-003 wurde in 003a (Kern) + 003b (OCC-Extrusion)
geschnitten (Roadmap §Historische Trigger-Verschiebungen). slice-003b
(erster OCC-Code + arch-check Regel C), slice-004 (gepinnte 26.04/node24-
Toolchain + apt-Snapshot), slice-007 (Datenmodell-Definition,
d-migrate-validiert + ADR-0006) und slice-008a (SQLite-Persistenz hinter
`ProjectRepositoryPort`, atomar + arch-check Regel D) und slice-008b
(Persistenz-Härtung: Crash-Recovery `kill -9` + `E-IO-001`/`E-IO-002`) sind
abgeschlossen; offen: slice-006.

## Roadmap

Siehe [`in-progress/roadmap.md`](in-progress/roadmap.md).
