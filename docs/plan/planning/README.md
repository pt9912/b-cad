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
| `open/` | slice-003 (Domain-Kern & Wände), slice-004 (Toolchain-Pinning + 26.04/node24) |
| `next/` | — |
| `in-progress/` | — (nur `roadmap.md`) |
| `done/` | slice-001 (Build-Skelett), slice-002 (Code-Gates), spike-001 (Toolchain-Reproduzierbarkeit) |

slice-001/002 sind abgeschlossen (DoD erfüllt, Closure-Notiz vorhanden;
die `make gates`-Verifikation ist dort als Punkt-in-Zeit belegt — der
aktuelle Lauf-Status gehört in CI, nicht in diese Doku, Modul 13).
spike-001 hat die Toolchain-Reproduzierbarkeit untersucht → ADR-0004
(Proposed) + slice-004 (Digest+Snapshot-Pinning, Migration 26.04/node24).
Offen: slice-003 (erste Fachlogik) und slice-004.

## Roadmap

Siehe [`in-progress/roadmap.md`](in-progress/roadmap.md).
