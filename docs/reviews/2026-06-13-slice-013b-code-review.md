# Code-Review-Report: slice-013b — 2026-06-13

**Review-Art:** Code-Review (nach Implementierung) — Diff `65d8aa3..9d2bcbf`,
zwei unabhängige Reviewer (Reviewer ≠ Autor, kein Autoren-Kontext).

**Gegenstand:** slice-013b (Türen + Fenster, automatische Wandöffnung
via boolesche Subtraktion).

**Linsen:** A = Geometrie- & Korrektheits-Tiefe; B = Architektur,
Transaktion, Schichtung, Code-Disziplin. **Read-only**, keine
`make`-Läufe (AGENTS §2.9).

---

## Gesamt-Ergebnis

**Linse B: PASS, keine HIGH** — transaktionale Garantie, Schichtung
(OCC im Adapter gekapselt), Notification (`WallGeometryChanged`, kein
`RoomsChanged`), Port-Migration, §2.2 (kein lebender Save-Pfad) und
DoD-Deckung (alle 8 Ops AK-getestet) unabhängig verifiziert.

**Linse A: 1 HIGH + 2 MED + 2 LOW** — der Kern ist geometrisch sauber
(Quer-Normale, Umlaufrichtung, Klemmung, Höhen-Klemmung, Transaktion
verifiziert), bis auf einen substanziellen Punkt (H1).

## Findings & Auflösung

### H1 (HIGH) — lateraler Cutter-Überstand fehlte → **behoben**

`opening_geometry.cpp` erzeugte das Schnitt-Prisma nur Z-überstehend,
**lateral aber bündig** (`half = thickness/2`) — koplanar mit den
Wandseitenflächen, entgegen `spezifikation.md` §1 („mit geringem
Überstand ≥ `GEOMETRY_TOLERANCE_MM` je Seite"). Verlass allein auf den
OCC-fuzzy-Wert ist genau der Robustheitsfall, den die Spec vermeiden
will; die Tests trafen nur achsenparallele, mittige Öffnungen.
**Fix:** der **Kern** erzeugt jetzt den vollen Überstand (lateral
`half + kOpeningCutOvershootMm` + Z-Rand an Standfläche/Krone); der
Adapter ist entsprechend dumm (kein Überstand mehr im Adapter —
Verantwortung nicht mehr gespalten). Code ist damit **spec-konform**
(Richtung MR-001). Neuer Robustheits-AK: `DiagonaleWandMitOeffnung`
(OCC, koplanare Flächen auf der Diagonale gerade vermieden).

### M1 (MEDIUM) — Volumen-Orakel XY-blind → **behoben**

`analytic_geometry_double.h` `cutVolume` nahm die nackte
Cutter-Polygon-Fläche — hätte den H1-Fix (lateral größerer Cutter)
fälschlich als Über-Subtraktion rot gemacht. **Fix:** das Orakel
klippt jetzt `Footprint ∩ Cutter` (Sutherland-Hodgman, konvexer Clip)
× Höhen-Überlappung mit `[0, Wandhöhe]` — misst das real entfernte
Areal, ehrlich gegenüber dem Überstand. H1+M1 zusammen behoben.

### M2 (MEDIUM) — Totalität von `openingCutPrism` unbelegt → **behoben**

Neuer direkter Kern-Test `OpeningCutPrismTotalitaet`: Brüstung ≥
Wandhöhe / Breite ~ 0 / nicht-endlicher Offset / Null-Längen-Wand →
`nullopt` (kein Wurf); gültige Eingabe → Prisma.

### L1 (LOW) — Zwischen-Shape-Null bei sequenziellem Cut → **behoben**

`makeNetSolid` prüft jetzt `!op.IsDone() || op.Shape().IsNull()` →
E-GEO-002.

### Linse B MEDIUM (lint-Risiko `corner`-Lambda) — **kein Befund**

`bugprone-easily-swappable-parameters` feuert auf `FunctionDecl`, nicht
auf Lambda-`operator()`; `make lint` ist grün (Gate-Lauf belegt). Keine
Aktion. Inline-Suppression-Verbot (§2.4) eingehalten.

### LOW (übrige) — kein Handlungsbedarf

`trial`-Kopie ist die transaktionale Grundlage (korrekt); `[[nodiscard]]`
konsistent zum bestehenden Port-Stil weggelassen.

## Verifikation nach Fix

`make gates` grün (2026-06-13): docs-check 0, gate-consistency,
arch-check A–E, lint 0 + suppression-gate, **Tests 82/82** (zuvor 80;
+Diagonal-OCC +Totalität), **Coverage 93,6 %**.

**Bewertung:** Der HIGH war berechtigt (Spec-Abweichung + reales
Robustheitsrisiko, das die Tests nicht trafen) — ein Beleg für den Wert
des Code-Reviews über die grünen Gates hinaus (Modul 11: Tests prüfen,
was sie testen). Alle Findings eingearbeitet.
</content>
