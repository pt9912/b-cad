# Code-Review slice-016b — Treppen-Geometrie (gerade einläufig)

**Datum:** 2026-06-14. **Reviewer:** unabhängiger Agent (Reviewer ≠ Autor),
Modul 11 — geometrielastiges Review vor Welle-2-Closure. **Gegenstand:** die
committete slice-016b-Implementierung (`2eca848` Phase i + `f10bd45` Phase ii).

## Verdikt: HIGH-Findings vorhanden: nein — Welle-Closure hält

Die geometrielastige Kern-Implementierung (`stair_geometry.cpp`) ist **korrekt**.
Die Fehlerklasse, die in 013b/014b/015b je 1 HIGH trotz grüner Gates ergab
(invertierte Normalen, Off-by-one, ungenaue Höhe, Spalt zwischen Körpern),
wurde gezielt durchgespielt — **alle bestanden**. Drei MEDIUM (1 Code-
Robustheit, 2 Test-Orakel-Lücken) eingearbeitet.

## Belegte Edge-Case-Durchläufe (Korrektheit verifiziert)

- **`appendBox`-Normalen-Winding:** alle 6 Flächen extern nachgerechnet
  (Kreuzprodukte) — `+z/−z/−y/+y/+x/−x` zeigen **nach außen**, Fan konsistent.
  Keine invertierte Fläche (014b-Klasse nicht vorhanden).
- **rise-Exaktheit:** `rise = height/steps`, `steps·rise == height` exakt
  (für h∈{2500,3000}, steps∈{2,7,13,15,30}) → oberste Stufe erreicht die obere
  Ebene flächenbündig.
- **Stufen-Bündigkeit:** Ende Stufe i = `x_start+(i+1)·tread` = Anfang Stufe i+1
  (identischer Ausdruck) → exakt bündig, kein Spalt/keine Überlappung.
- **Geländer:** beidseitig, je Stufe `z∈[top, top+rail_h]`, Index `0..steps−1` →
  über jeder Stufe inkl. letzter (**kein Off-by-one**); `rail_t = min(50,
  width/4)` → die zwei Seiten überlappen nie (auch bei kleiner width).
- **Totalität:** `stairMesh`-Guard fängt step_count<1, nicht-endlich,
  width/tread/height<Toleranz → leeres Netz; Division durch steps abgesichert.

## MEDIUM (eingearbeitet)

- **M1 — `stairRunLengthMm`/`stairRiseMm` ohne Totalitäts-Guard:** anders als
  `stairMesh` führten die puren Queries keinen Endlichkeits-/Bereichs-Guard
  (kein aktiver Defekt, da nur `addStair` produziert/klemmt — aber inkonsistenter
  Kontrakt). **Eingearbeitet:** `stairRunLengthMm` guardet jetzt
  `step_count<1 || !isfinite(tread) → 0` (gleicher Kontrakt wie `stairMesh`;
  `stairRiseMm` hatte den step_count≤0-Guard bereits).
- **M2 — Test prüfte Normalen-Orientierung NICHT:** eine invertierte Box-Fläche
  (014b-Defekt) wäre durchgerutscht (Bounding-Box/Vertices identisch). **Ein-
  gearbeitet:** Test `NetzGeschlossenUndKonsistentOrientiert` — Summe der
  flächengewichteten Außennormalen (∑ ½·(b−a)×(c−a)) ≈ 0 (Divergenzsatz; jeder
  geschlossene Quader summiert zu 0, eine invertierte Fläche läge bei
  ~Flächeninhalt ≫ 1).
- **M3 — Test prüfte Stufen-Bündigkeit NICHT:** ein Spalt/eine fehlende Stufe
  wäre bei kompensierter Dreiecks-Anzahl durchgerutscht. **Eingearbeitet:** Test
  `StufenBuendigKeineLuecke` — die distinkten x-Stufenkanten sind genau
  `step_count + 1`.

## LOW / INFO

- **L1 — Über-Meldung (bewusst belassen):** `setStairWidth`/`setStairStepCount`
  melden `StairChanged` auch bei effektiv unverändertem (geklemmtem) Wert.
  Funktional unkritisch — `ViewerScene` ist via `reloadKeyed` idempotent (im
  Szenen-Test belegt). Konsistenz-Schwäche, kein Bug; nicht geändert.
- **I — Integration korrekt:** `addStair`-Validierung (from≠to, beide Geschosse
  via `any_of`, Höhe>Toleranz, Klemmung **vor** push; geklemmter step_count
  speist die rise-Ableitung → Mesh↔Wert konsistent); `StairChanged` an
  `from_storey`, `stairMeshes` projektweit; `base_z=0` (Ein-Geschoss-Annahme,
  spec-konform); Geländer „beidseitig" ⊆ spec „Lauf-Seite(n)".

## Ergebnis nach Einarbeitung

`make gates` grün: **Tests 116/116** (zuvor 114, +M2/M3-Härtung), Geometrie-
Korrektheit jetzt **durch die Tests** abgesichert (nicht nur durch den Code).
Keine offenen HIGH/MEDIUM. **Welle-2-Closure freigegeben** (aus Korrektheits-/
Review-Sicht).
