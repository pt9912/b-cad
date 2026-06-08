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
| `open/` | slice-003 (Domain-Kern & Wände), spike-001 (Toolchain-Reproduzierbarkeit) |
| `next/` | — |
| `in-progress/` | — (nur `roadmap.md`) |
| `done/` | slice-001 (Build-Skelett & DevContainer), slice-002 (Code-Gates & Sensors-Promotion) |

slice-001/002 sind abgeschlossen und verifiziert grün (`make gates`):
hexagonales Skelett + fünf reale Gates ohne Bind-Mounts. slice-003
(erste Fachlogik) und spike-001 (Base-Version/Pinning → ADR-0004) sind
offen.

## Roadmap

Siehe [`in-progress/roadmap.md`](in-progress/roadmap.md).
