# Unabhängiges Code-Review — slice-029 (ui-Richtungs-Trennung, Commits `dd01fc7` + `c0a12ae`)

**Datum:** 2026-07-03
**Reviewer:** unabhängig (Reviewer ≠ Autor, read-only auf committetem Stand)
**Linsen:** (a) Abnahme-Kriterium je Adapter-Verzeichnis, (b) Naht-Korrektheit vs. Plan (MED-1-Auflage port-freie Typen), (c) Verhaltens-Neutralität (alt vs. neu, semantisch), (d) Vollständigkeit (CMake/main/Tests/Namespaces/Lebenszeiten), (e) Doku-Plan-Treue

## Verdikt

**Closure-tauglich: JA — 0 HIGH / 0 MED / 1 LOW / 2 INFO.**

## Verifikations-Log

- **Abnahme-Kriterium erfüllt (grep-belegt, jedes Adapter-Verzeichnis):** `io`/`geometry`/`persistence` nur driven, `plugin` nur driving, ui-Wurzel (`qt_probe`) port-frei, `ui/view/` nur driven, `ui/command/` nur driving — **kein Verzeichnis mischt**. ✓
- **Naht:** `mesh_source.h` port-frei (nur model + Std-Lib), Aggregate als `std::map<Id,TriangleMesh>` (MED-1-Auflage), Methodenmenge exakt der Plan-§1-Bedarf; `toMap`-Konversion id-korrekt (keine roof/slab/stair-Verwechslung), move-sicher; Dependency-Inversion richtungs-korrekt (`command/` → `view/mesh_source.h`, nie umgekehrt). ✓
- **Verhaltens-Neutralität (alt `dd01fc7^` vs. neu, semantisch verglichen):** `loadAll` äquivalent (Zuweisung == clear+rebuild); `reloadKeyed`-Vereinfachung äquivalent (Zähl-Logik neu/ersetzt/entfernt, Ersetzung nur bei changed>0, identische Mehrfach-Meldung → 0 Updates); Wall-Case identisch — **der ADR-0008-Vertrag „genau EIN wirksames Update je Wand-Meldung" bleibt exakt**; Widget-Callback→Repaint unverändert. ✓
- **Vollständigkeit:** keine Alt-Includes/Alt-Namespaces; CMake vollständig; `main.cpp`-Lebenszeit-Ordnung korrekt (service → mesh_source → window/viewer); Test-Fixture-Deklarationsreihenfolge korrekt (mesh_source_ vor scene_). ✓
- **Doku:** conventions-GUI-Zeile korrigiert (LOW-2-Auflage); architecture-ui-Zeile Allow/Deny in sich und gegen alle Zeilen widerspruchsfrei + Baum; stale „Driving Adapter"-Kommentar im Widget-Kopf ersetzt; Commit i = 100 % Renames (MED-2-Auflage). ✓

## Findings + Disposition

### LOW-1 — Plan nannte fünf Archiv-Marker, gate-korrekt umgesetzt sind zwei
**Fund:** Die DoD listete `slice-{012,013b,014b,015b,016b}`; markiert wurden 012/013b (literale Pfade). Das Trio 014b/015b/016b trägt **Brace-Formen**, die das codepaths-Gate als Glob behandelt — gate-exempt, Marker unnötig; die Zwei-Datei-Umsetzung ist **freeze-schonender** als der überspezifizierte Plantext.
**Disposition: in der Closure-Notiz begründet** (5→2-Reduktion, damit kein DoD-Audit „unvollständig" liest).

### INFO-1 — Stale „MED-2"-Referenz im übernommenen reloadKeyed-Kommentar (Altbestand)
**Disposition: belassen** (nicht slice-029-verursacht; Kandidat für Gelegenheits-Pflege).

### INFO-2 — Roadmap-Reorder verbessert Chronologie
**Disposition: kein Befund.**
