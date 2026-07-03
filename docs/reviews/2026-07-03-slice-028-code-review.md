# Unabhängiges Code-Review — slice-028 (Berechnungs-Kerne → services/geometry/, Commits `b22632e` + `76cec48`)

**Datum:** 2026-07-03
**Reviewer:** unabhängig (Reviewer ≠ Autor, read-only auf committetem Stand; Parallelarbeit an `src/adapters/ui/**` explizit ausgeklammert und Nicht-Berührung verifiziert)
**Linsen:** (a) Move-Vollständigkeit/Korrektheit, (b) Plan-Treue (DoD inkl. HIGH-1/MED-1/MED-2-Auflagen), (c) Reinheits-Invariante, (d) Abnahme-Kriterium

## Verdikt

**Closure-tauglich: JA — 0 HIGH / 0 MED, 2 INFO.**

## Verifikations-Log

- **Commit i (`b22632e`):** 10/10 Renames zu 100 %, 0 Insertions/Deletions — reiner Move (HIGH-1-Auflage Zwei-Commit-Split eingehalten). ✓
- **Commit ii (`76cec48`):** 25 Dateien, jede Hunk-Änderung geprüft — ausschließlich Include-Pfade/CMake/Kommentar-Verweise/Doku, **keine** Signatur-/Logik-Änderung. ✓
- **Alt-Pfad-Sweep:** keine alten Includes in `src/`/`tests/`/`plugins/`; alte Dateistandorte physisch weg; gerootete Doku-Reste nur als gate-neutrale Brace-Formen. ✓
- **architecture.md (MED-2):** Sub-Schicht-Zeile + „Darf importieren"-Erweiterung Geometrie-/Persistenz-Adapter; **alle 11 Tabellen-Zeilen auf Folge-Widersprüche geprüft — konsistent**; die 11 Bestands-Includes sind jetzt regel-gedeckt (Heilung der vorbestehenden Inkonsistenz). ✓
- **d-check:ignore (MED-1):** an beiden benannten Stellen (023b-Archiv, 024b-Review) mit Begründungstext; keine `.d-check.yml`-Scope-Änderung. ✓
- **Reinheits-Invariante:** alle 10 Dateien unter `services/geometry/` inkludieren nur `hexagon/model/` + Std-Lib + eigenen Self-Include; **kein** Geschwister-Kern-Include (Plan-LOW-1-Behauptung verifiziert). ✓
- **Abnahme-Kriterium:** `grep '#include "hexagon/services/' src/adapters/` → 11 Treffer, **alle** `services/geometry/` — deckungsgleich mit dem Pilot-Befund. ✓

## Findings + Disposition

### INFO-1 — Gate-neutrale historische Alt-Bezeichner-Reste (Brace-Form/nicht-gerootet) in done-archive
**Disposition: keine Änderung** — planmäßig codepaths-neutral; die MED-1-Auflage betraf nur gerootete Nicht-Brace-Pfade (beide markiert).

### INFO-2 — Roadmap-Quergewerk-Zeile chronologisch vor einer älteren Retro-Zeile
**Disposition: behoben** (Zeile ans Tabellen-Ende verschoben, im 029-Commit-ii mitgeführt).
