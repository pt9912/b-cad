# Code-Review-Report: slice-014b — 2026-06-13

**Review-Art:** Code-Review (nach Implementierung) — Diff
`5507393..2995ea4`, zwei unabhängige Reviewer (Reviewer ≠ Autor).

**Gegenstand:** slice-014b (Dach Sattel/Walm/Pult, analytisches Netz im
Kern). **Linsen:** A = Geometrie/Korrektheit; B = Architektur/
Integration/Schichtung/Disziplin. Read-only, keine `make`-Läufe.

---

## Gesamt-Ergebnis: **keine HIGH (beide Linsen).**

**Linse A:** Dach-Mathematik in allen 3 Typen + beiden Achsen-Zweigen
**numerisch verifiziert korrekt** (Formeln, Firsthöhen, Walm-Einrückung,
alle Flächennormalen außen/oben, kein Copy-Paste-Vertausch im y-Zweig,
total bei Degeneration). **Linse B:** Schichtung (Regel A, Kern
framework-frei), Notification (genau `RoofChanged`, nie `RoomsChanged`,
no-op/Reject schweigen), `switch` exhaustiv, keine Suppression, Wand-Pfad
additiv-unberührt, ADR-0011-#6.3-Abweichung konsistent dokumentiert.

## Findings & Auflösung

- **MEDIUM-1 (B) — `setRoofType` ohne AK (DoD-Lücke).** Eine der 5 Ops
  + ihr no-op-Pfad unverifiziert. **Behoben:** Test
  `LH_FA_ROF_002_FormwechselSattelZuWalmFolgt` (Sattel 4 → Walm 6
  Dreiecke; No-op-Wechsel meldet nicht / kein Update).
- **LOW-1 (B) — `addRoof` speicherte NaN-Neigung/-Überstand**
  (`std::clamp(NaN)=NaN` umging die Validierung → Geister-Dach;
  inkonsistent zur `addWall`-Linie). **Behoben:** `addRoof` verwirft
  nicht-endliche `pitch_deg`/`overhang_mm`/`base_z_mm` (→ `nullopt`).
- **LOW-3 (A) — Flächen-Orientierung ungetestet** (Orakel-Lücke gegen
  invertierte Quads). **Behoben:** Test `AlleFlaechenZeigenNachOben`
  (Normale.z > 0 je Dreieck, alle 3 Typen).
- **LOW-2 (A) — `tan(90°)` finite, Totalitäts-Guard griff nicht.**
  **Behoben:** `roofMesh` verwirft Neigung ≤ 0 / ≥ 90° (leeres Netz) —
  der Totalitäts-Vertrag gilt jetzt unabhängig vom Service-Klemmen.
- **LOW-1 (A) — toter Zeltdach-Branch** (`rx1 < rx0` unerreichbar).
  **Behoben:** `<=` (defensiv gegen Float-Drift), beide Achsen-Zweige.
- **LOW-4 (A) — Viewer pullt storey-übergreifend alle Dächer.**
  Funktional korrekt (`meshEqual` macht fremde Dächer zu No-ops), nur
  ineffizient bei Mehr-Geschoss. **Notiert** (Closure; Re-Eval bei
  Performance-Budget) — kein Code-Fix in welle-2.
- **INFO-1 (B) — `appendTriangle(V3,V3,V3)` swappable-Params.** lint
  grün (Funktion ist anonym/static, Aufrufer setzen CCW bewusst). Kein
  Fix nötig.

## Verifikation nach Fix

`make gates` grün (2026-06-13): docs-check 0, arch-check A–E, lint 0 +
suppression-gate, **Tests 94/94** (zuvor 92; +Orientierung +Formwechsel),
Coverage **92,2 %**.

**Bewertung:** Solider, geometrisch korrekter Slice; das Review schloss
zwei Verifikations-Lücken (`setRoofType`-AK, Orientierung) und eine
Konsistenz-Lücke (NaN-Dach) sowie zwei Robustheits-Kanten (pitch≥90,
Zeltdach-Branch). Erneut Beleg für den Wert des Code-Reviews über grüne
Gates hinaus (Modul 11).
</content>
