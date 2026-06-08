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
| `open/` | slice-002 (Code-Gates & Sensors-Promotion), slice-003 (Domain-Kern & Wände) |
| `next/` | — |
| `in-progress/` | — (nur `roadmap.md`) |
| `done/` | slice-001 (Build-Skelett & DevContainer) |

slice-001 ist der Übergang vom Bootstrap zum Workflow (Kurs-Modul 2
§Bootstrap-Ende vs Workflow-Beginn): das erste Code-Slice, verifiziert
grün (`make build`). slice-002/003 sind weiterhin Pläne.

## Roadmap

Siehe [`in-progress/roadmap.md`](in-progress/roadmap.md).
