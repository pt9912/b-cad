# ADR-0007: Geometrie-Basis der Raumerkennung (Innenkante, Ring-Modell)

**Status:** Accepted

**Datum:** 2026-06-11

**Autor:** Dietmar Burkard

**Bezug:** [LH-FA-ROM-001](../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen) (Raum-Autoerkennung), [LH-FA-ROM-002](../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen)/003 und
[LH-FA-EVL-003](../../../spec/lastenheft.md#lh-fa-evl-003--wohnflächenberechnung) (Flächen-/Volumen-Konsumenten, Trade-off-Kontext),
ADR-0001 (Kern führt), ADR-0006 (`rooms.polygon_json`)

---

## Kontext

Die Raum-Autoerkennung ([LH-FA-ROM-001](../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen)) leitet aus geschlossenen
Wandzügen Räume mit Polygon und Fläche ab. Die Spezifikations-Outline
(`spezifikation.md` §1) ließ zwei Lösungsfragen offen — §7 führte die
erste als expliziten offenen Punkt:

1. **Polygon-Basis:** Liegt das Raumpolygon auf der Wand-**Mittellinie**
   (die Wand-Segmente des Domänen-Modells sind Achsen) oder auf der
   **Innenkante** (lichte Maße)?
2. **Verschachtelungs-Repräsentation:** Wie wird der äußere Raum
   verschachtelter Wandzüge dargestellt, sodass „keine Doppelzählung
   der Fläche" ([LH-FA-ROM-001](../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen) Boundary) darstellbar und testbar ist?
   Ein einfaches Polygon pro Zyklus kann ein Loch nicht ausdrücken
   (Plan-Review slice-009, Finding H1).

Das Lastenheft-Glossar definiert *Raum* als „aus geschlossenem Wandzug
abgeleitetes Flächen-/Volumen-Element" — die Fläche ist Teil des
Begriffs, nicht Beiwerk.

## Entscheidung

1. **Polygon-Basis: Innenkante.** Das Raumpolygon liegt auf den
   Innenkanten der umschließenden Wände: jede Zyklus-Kante wird um die
   **halbe Wandstärke des jeweiligen Segments** zum Zyklus-Inneren
   versetzt; benachbarte Offset-Geraden werden geschnitten. Der
   Offset-Schritt ist Teil der Erkennung, nicht der Auswertung.
2. **Verschachtelung: Ring-Modell.** Ein Raum ist ein Polygon mit
   **äußerem Ring + 0..n Loch-Ringen**. Der innere Zyklus erzeugt einen
   eigenen Raum (Innenkante nach innen); im umschließenden Raum wird
   die **Außenkontur** des inneren Zyklus als Loch-Ring geführt. Die
   Raum-**Fläche ist die Netto-Fläche** (äußerer Ring minus
   Loch-Ringe).
3. **Totalität:** Die Erkennung ist eine ableitende Berechnung und
   **wirft kein [`E-GEO-002`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)** — degenerierte Zyklen (Offset kollabiert,
   selbstschneidend, Netto-Fläche ≤ 0) erzeugen *keinen Raum*, wie
   offene Wandzüge. [`E-GEO-002`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) bleibt mutierenden
   Geometrie-Operationen vorbehalten (Präzisierung in
   `spezifikation.md` §1).

## Verglichene Alternativen

### Option A — Innenkante + Ring-Modell (gewählt)

- Pro: Raumpolygon = begehbare/lichte Fläche, deckungsgleich mit dem
  Glossar-Begriff und dem, was ROM-002 (Raumfläche) und EVL-003
  (Wohnfläche, lichte Maße) später brauchen — kein Umbau in welle-3;
  korrekte 2D-Darstellung des Raums im Grundriss; Boundary-Kriterium
  („keine Doppelzählung") ist als Netto-Fläche direkt testbar.
- Contra: Offset-Schritt nötig (pro Kante Wandstärke/2, Schnitt
  benachbarter Offset-Geraden); Degenerationsfälle (sehr kleine
  Zyklen) müssen definiert sein — adressiert durch Entscheidung #3.

### Option B — Mittellinie

- Pro: trivial (Segmente sind bereits Achsen), kein Offset, keine
  Degenerationsfälle aus dem Offset.
- Contra: Raumfläche systematisch zu groß (überdeckt die halbe
  Wandstärke ringsum); ROM-002/EVL-003 bräuchten den
  Innenkanten-Offset später doch — die Entscheidung wäre nur vertagt
  und das Raumpolygon im Grundriss läge sichtbar falsch (in der
  Wandmitte).

### Option C — Einfaches Polygon + entkoppelte Netto-Fläche (statt Ringen)

- Pro: einfachere Datenstruktur.
- Contra: die Loch-Geometrie geht verloren — 2D-Darstellung und
  spätere Auswertungen (Bodenbeläge, EVL) können das Loch nicht
  rekonstruieren; Fläche und Polygon widersprechen sich (Polygonfläche
  ≠ gespeicherte Fläche), eine latente Inkonsistenz.

## Konsequenzen

- Positiv: ROM-002/003 und EVL-003 erben eine korrekte, lichte
  Flächen-Basis; das Boundary-Akzeptanzkriterium ist mit analytischen
  Netto-Flächen-Erwartungswerten testbar.
- Folgepflicht (slice-009b): Innenkanten-Offset und Ring-Modell
  implementieren; Boundary-Test prüft die Netto-Fläche verschachtelter
  Wandzüge gegen den analytischen Wert.
- `rooms.polygon_json` (ADR-0006) muss die **Ring-Struktur** tragen
  (äußerer Ring + Loch-Ringe), sobald Räume persistiert werden.
  ADR-0006 bleibt unverändert gültig — die JSON-*Form* ist bewusst
  formvariabel (ADR-0006 #3); die konkrete Ring-Kodierung legt der
  Raum-Persistenz-Slice fest.
- Welle-1-Reichweite: Zyklen entstehen nur über endpunkt-verbundene
  Segmente (`GEOMETRY_TOLERANCE_MM`); Schnittpunkt-Knoten erst mit
  Wandverschneidung ([LH-FA-WAL-006](../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| AK-Tests (slice-009b) | Happy/Boundary/Negative gegen analytische Innenkanten-/Netto-Flächen-Werte; Boundary prüft „keine Doppelzählung" | `make test` |
| Schichtung | Erkennung bleibt pure Domäne (kein OCC/Qt/SQLite im Kern) | `make arch-check` (ADR-0001) |

## Re-Evaluierungs-Trigger

- Wandverschneidung ([LH-FA-WAL-006](../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)) führt Schnittpunkt-Knoten ein →
  Offset-/Zyklen-Logik gegen T-Stöße neu prüfen.
- Performance-Zielkomplexität (offener Punkt §7, M3) verlangt
  inkrementelle Erkennung → Vollerkennungs-Annahme neu bewerten.
- EVL-003-Slice (welle-3) stellt fest, dass die Wohnflächen-Norm eine
  andere Basis verlangt als die lichte Innenkante → Trade-off neu
  aufrollen (Supersedes-ADR, nie stille Anpassung).

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-11 | Proposed (aus Plan-Review slice-009, Findings H1/M1) | slice-009a |
| 2026-06-11 | Accepted — Spec-§1-Schärfung umgesetzt, §7-Punkt geschlossen | slice-009a |
