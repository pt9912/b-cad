---
id: slice-060
titel: Eckenschluss — Kollaps-Kriterium und Invarianten-Sonden (Fix zu [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-HIGH-1)
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-D3-001](../../../../spec/lastenheft.md#modul-3d-modellierung-d3)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 060: Eckenschluss — Kollaps-Kriterium und Invarianten-Sonden

**Status:** open — **Detail-Schnitt vollzogen** (2026-07-29). **Fix-Slice** zum
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
des Bauteil-Strangs ([`2026-07-29-mr-009-bauteil-strang.md`](../../../reviews/2026-07-29-mr-009-bauteil-strang.md)).
**Eigenes [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.**

**Welle:** welle-6-interaktiv-planen — **Closure-blockierend.** Das Review hat **zwei** HIGH gefunden;
dieser Slice behebt **HIGH-1** (samt aller drei MEDIUM). **HIGH-2** (ungleiche Wandhöhen) ist auf
Entscheidung des Projektinhabers **abgetrennt** — [`slice-061`](slice-061-eckenschluss-ungleiche-hoehen.md),
mit ausdrücklicher Deferral-Notiz. **Das ist eine benannte Abweichung von
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)**,
die „HIGHs blockieren die Closure" ohne Vertagungs-Klausel schreibt; sie steht im Wellen-Ergebnis, statt
zu verschwinden.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-29.

## Auslöser

[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-HIGH-1
und MEDIUM-1/2/3. **Der Kern liefert einen offenen, in sich gegenläufig orientierten Wandkörper** —
und eine nach [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)
**zulässige** Stärken-Änderung zerstört dabei eine Wand, die der Benutzer **nicht angefasst hat**.

## 1. Ziel

Ein Wand-Footprint ist **nie** selbstschneidend. Wo die Eck-Konstruktion kollabieren würde, enden die
Wände **stumpf** — total, ohne Wurf, ohne Modell-Schaden. Und die ausgelieferten Wand-Körper haben
erstmals **Invarianten-Sonden**: geschlossen, konsistent orientiert, und alle drei Volumen-Wege stimmen
überein.

## 2. Die drei Entwurfs-Fragen dieses Slice

### 2.1 Das Kriterium steht schon in der Spezifikation — für die andere Seite derselben Konstruktion

`cornerAt` (`src/hexagon/services/geometry/wall_footprint.cpp`) setzt die Seitenkante um die **halbe
Stärke des Nachbarn** zurück. `WALL_MITER_LIMIT` begrenzt, wie weit ein Eckpunkt **über den
gemeinsamen Endpunkt hinaus** ragt — **nicht**, wie weit er in die Wand **hinein** greift. Ist die Wand
kürzer als dieser Rücksprung, wandert der Eckpunkt hinter das andere Wandende, und das Polygon wird
zur Schleife.

**Die Spezifikation kennt das richtige Kriterium bereits** — sie schreibt es für den Raum-Offset aus
(`spec/spezifikation.md`, Raumerkennung): *„Kollaps-Kriterium ist der **Kantenrichtungs-Erhalt**:
kehrt sich beim Offset die Richtung einer Kante um, ist der Ring kein gültiges Raumpolygon (eine reine
Flächen-Prüfung »Netto-Fläche ≤ 0« genügt nicht …)."* **Der Wand-Footprint hat diese Lehre nie
mitbekommen** — dieselbe Konstruktion, dieselbe Fehlerklasse, nur die andere Kontur.

**Gemessen im Review, und genau diskriminierend:** eine Sonde, die die Kantenrichtungen des gemiterten
Footprints gegen die des stumpfen vergleicht, meldet „umgekehrt" bei `len` 239/121/120/119/100 und
„erhalten" bei 600/241/240 — **exakt** in den selbstschneidenden Fällen und in keinem anderen.

**Warum die Flächen-Prüfung strukturell nicht genügt** (der Grund steht ebenfalls schon in der
Spezifikation): die vorzeichenlose Shoelace-Fläche des Bowtie-Polygons ist **unauffällig** — für die
gemessene 100-mm-Wand exakt `24 000 mm²`, derselbe Wert wie beim stumpfen Rechteck.

### 2.2 Die Prüfung gehört an das POLYGON, nicht an die einzelne Ecke — und der Ort ist der Kern

**Nicht per Ecke:** die zwei Schwellen sind verschieden. Bei **einem** gemiterten Eck kollabiert es
unterhalb der halben Nachbar-Stärke, bei **zwei** (Nachbarn auf derselben Seite) erst unterhalb der
**vollen**. Eine Prüfung, die nur eine Ecke sieht, verpasst die Kombination — gemessen: `len = 239`
mit zwei Ecken ist selbstschneidend, obwohl jede Ecke für sich unter ihrer Einzel-Schwelle bleibt.
**Also: erst beide Ecken bauen, dann das fertige Polygon prüfen.**

**Der Ort ist der Kern, nicht der Adapter.** Die Spezifikation legt die **Footprint-Hoheit** in den
Kern (der Adapter „extrudiert/tesselliert nur noch das Polygon") und verlangt für die Eck-Konstruktion
ausdrücklich **Totalität** („Begrenzung und Rückfälle (total, wirft nie)"). Ein Riegel im Adapter
müsste werfen oder ein leeres Solid liefern — beides widerspricht dem. **Der vorhandene
`isDegenerate`-Wächter im Adapter bleibt unverändert**; er ist eine andere Zusicherung (Punktzahl,
Endlichkeit, Fläche) und wird durch diesen Fix nur nie mehr der letzte Halt sein.

### 2.3 Der Rückfall ist LOKAL — und er braucht eine Kandidaten-Suche, keinen einzelnen Schritt

**Zwei Fragen, die der erste Plan-Lauf beide gegen meine erste Antwort entschieden hat.**

**(a) Ein Rückfall an EINER Ecke kann die ANDERE Ecke kaputtmachen.** Gemessen (Lauf 1): eine Wand
`A` (100 mm, Stärke 240) zwischen zwei rechtwinkligen Nachbarn ist **voll gemitert gesund**
(`minDot +5000`); setzt man die eine Ecke stumpf, weil dort ein Nachbar kollabiert, wird `A`
**selbstschneidend** (`minDot −5000`).

Der Mechanismus ist analytisch: die beiden Längskanten liegen auf den Seiten-Offset-Geraden, ihre
vorzeichenbehaftete Länge ist `len + e − s` (mit den Längsversätzen `s`/`e` der Eck-Punkte). Ein
Selbstschnitt liegt **genau dann** vor, wenn `len < |s − e|`. Haben `s` und `e` **dasselbe
Vorzeichen**, gleichen sie sich aus; ein Stumpf-Setzen (`s := 0`) nimmt diesen Ausgleich weg und
`|s − e|` **wächst**.

**Daraus folgt: ein einzelner Prüf-Schritt genügt nicht.** Der Footprint wird über eine **endliche,
geordnete Kandidaten-Liste** bestimmt — `(gemitert, gemitert)` → `(gemitert, stumpf)` →
`(stumpf, gemitert)` → `(stumpf, stumpf)`; der **erste** Kandidat, dessen Polygon den
Kantenrichtungs-Erhalt besteht, gewinnt. Der letzte ist das stumpfe Referenz-Polygon und besteht
**immer**. **Das terminiert nachweislich** (vier Kandidaten, feste Reihenfolge) und ist total — die
Zusage aus §1 gilt damit für das **Ergebnis**, nicht für einen Zwischenschritt.

**(b) Der Rückfall ist LOKAL, nicht symmetrisch — entgegen der ersten Fassung dieses Plans.** Die
erste Fassung wollte: kollabiert die Ecke für eine der beiden Wände, enden **beide** dort stumpf.
Begründung damals: nur so bleibe die Ecke „nahtlos". **Die Messung widerlegt genau diese
Begründung** — zwei stumpfe Enden sind an einer Ecke **nie** nahtlos:

| Ecke (rechter Winkel, Stärken 240; 3721 Abtastpunkte) | leer | doppelt |
|---|---|---|
| gemitert (gesund) | 61 (Versatz-Artefakt) | **0** |
| **beide** stumpf (symmetrisch) | **1171** | **750** |
| **nur die kurze** Wand stumpf (lokal) | **841** | **450** |

Zwei stumpfe Enden im rechten Winkel überlappen sich in einem Quadrat von `(t/2)²` und lassen im
gegenüberliegenden Quadranten eine **gleich große Kerbe**. **Die symmetrische Variante ist in beiden
Kennzahlen schlechter als die lokale** — sie kauft die Nahtlosigkeit nicht, sie verschlechtert sie.

**Entschieden: lokal.** Jede Wand bestimmt ihren Footprint **aus ihrer eigenen Geometrie und dem
Nachbar-Wissen**, wie bisher; der Kollaps-Rückfall ist Teil dieser Bestimmung und wirkt **nur auf
die eigene Wand**. Drei Gründe, alle nachprüfbar:

1. **Messbar weniger schlecht** an beiden Kennzahlen (Tabelle oben).
2. **Kein zweistufiger Melde-Radius.** Ein symmetrischer Rückfall machte den Nachbar-Rebuild
   zweistufig (A kollabiert ⇒ B ändert sich ⇒ B's andere Nachbarn?); `rebuildAffectedNeighbors` ist
   einstufig. Die lokale Regel lässt ihn unverändert.
3. **Keine Kreuz-Abhängigkeit, kein globaler Fixpunkt.** Die Kandidaten-Suche aus (a) läuft
   **innerhalb einer Wand** und terminiert dort.

**Der Preis, ausgeschrieben:** an einer kollabierten Ecke ist die Darstellung **nicht** nahtlos —
450 doppelt und 841 leer von 3721 Abtastpunkten. **Das ist eine Abweichung von der
Happy-Path-Zusage des Lastenhefts und wird dort als Boundary-Zeile nachgezogen** (§5), nach dem
Muster, das das Lastenheft für den Miter-Limit-Rückfall bereits selbst verwendet. **Was nicht geht,
ist der Fix ohne diese Zeile:** dann stünde eine Zusage im Lastenheft, die die Software gemessen
nicht hält.

## 3. Bewusst NICHT Teil

- **HIGH-2 — ungleiche Wandhöhen.** Abgetrennt nach Entscheidung des Projektinhabers:
  [`slice-061`](slice-061-eckenschluss-ungleiche-hoehen.md). Der Fall verlangt einen
  **höhen-geschichteten** Wandkörper und bricht damit die Spec-Zusage „der Adapter extrudiert nur noch
  das Polygon" — das ist eine eigene Entscheidung mit eigener ADR-Frage, kein Nebenprodukt eines
  Fix-Slice.
- **LOW-2 — zwei deckungsgleiche Wände ohne Hinweis.** Weder Lastenheft noch Spezifikation sagen dazu
  etwas; das Modell bleibt konsistent. Eigene Anforderung, nicht dieser Schnitt.
- **INFO-2 — `std::abs` auf dem OCC-Volumen.** Heute folgenlos (das Footprint-Polygon ist immer im
  Uhrzeigersinn, gemessen über sechs Zug-Richtungen); die neue Volumen-Übereinstimmungs-Sonde (§4-7)
  deckt den Fall ab, ohne die Adapter-Signatur anzufassen.
- **Jede Änderung an UI, Persistenz, Export, Schema.** Der Fix ist eine Kern-Rechnung; die Export- und
  Persistenz-Orakel sind das **Netz**, das die Verhaltens-Invarianz belegt.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Das Kollaps-Kriterium ist rein und diskriminiert exakt** — Kantenrichtungs-Erhalt. **Die Zahlenreihe gilt nur mit ihrer Fixture:** `len` 600/241/240 ⇒ erhalten, 239/121/120/119/100 ⇒ umgekehrt **in der ZWEI-Ecken-Lage** (Nachbarn an beiden Enden, Stärke 240, rechtwinklig); in der Ein-Eck-Lage sind 241/240/239/121 **alle** erhalten. **Und der Tie-Break gehört entschieden:** bei `len = 240` ist das Skalarprodukt **exakt 0** — `room_detection.cpp` nutzt dort `<= 0`; dieselbe Konvention ist zu übernehmen und in der Zeile zu nennen, sonst kippt sie an der Schwelle | Kern-Test, **ohne** Qt/OCC (Bauform `test_wall_footprint.cpp`) | Flächen- statt Richtungs-Kriterium ⇒ rot (die Bowtie-Fläche ist unauffällig: `24 000 mm²`, derselbe Wert wie beim stumpfen Rechteck) |
| 2 | **Ein gemitertes Eck: unterhalb der Schwelle wird stumpf** — der Footprint ist danach **nicht** selbstschneidend | Kern-Test, Fixture rechter Winkel, Stärken 240 (Schwelle dort 120 mm) | Rückfall entfernt ⇒ rot (Selbstschnitt) |
| 3 | **Zwei gemiterte Ecken haben eine ANDERE Schwelle als eine** — die Kombination, die eine Pro-Ecke-Prüfung verpasst | Kern-Test, Nachbarn an **beiden** Enden | Prüfung pro Ecke statt am Polygon ⇒ rot bei `len = 239` (rechtwinklig, 240) |
| 3a | **Die Schwelle ist KEINE feste Zahl** — sie hängt an Winkel **und** Stärken: rechtwinklig/gleich dick liegt sie bei 120 bzw. 240 mm, bei θ = 120° schon bei **207,75** mm, und bei Nachbarn 240/1000 bei **619,75** mm = (t₁+t₂)/2 (Lauf-1-Messung). **Die Orakel dürfen keine Zahlen einsetzen, sondern müssen die Schwelle aus der Lage bestimmen** — oder relativ prüfen (knapp darunter ⇒ stumpf, knapp darüber ⇒ gemitert) | Kern-Test über einen kleinen Winkel-/Stärke-Sweep | Schwelle als Konstante gerechnet ⇒ rot bei θ ≠ 90° |
| 4 | **Was der Rückfall WIRKLICH zusagt** — beide Wände liefern ein **gültiges** Polygon (nicht selbstschneidend), die kurze das **stumpfe Referenz-Polygon**, und der Nachbar bleibt **unverändert** gegenüber der Lage ohne Kollaps. **Ausdrücklich NICHT zugesagt: Nahtlosigkeit** — sie ist bei zwei stumpfen Enden geometrisch unmöglich (§2.3), und ein Orakel, das sie verlangte, könnte nie grün werden | Kern-Test: Polygon-Vergleich gegen `buttFootprint` **und** gegen den unveränderten Nachbar-Footprint | Kandidaten-Suche entfernt ⇒ rot; Rückfall auch beim Nachbarn ⇒ rot (er darf sich **nicht** ändern) |
| 4a | **Die Kandidaten-Suche ist abgeschlossen** — nach dem Rückfall an **einer** Ecke ist der Footprint **wieder** zu prüfen; die im Lauf 1 gemessene Lage (A 100 mm/240 zwischen Nachbarn 300 und 200, rechtwinklig) liefert **kein** selbstschneidendes Polygon mehr | Kern-Test mit **genau dieser** Lage | Einzelner Prüf-Schritt statt Kandidaten-Liste ⇒ rot (`minDot` kippt von +5000 auf −5000) |
| 5 | **Der ausgelieferte Wand-Körper ist geschlossen und konsistent orientiert** — **zwei** Sonden, weil eine nicht genügt: (a) Kanten-Manifold (jede gerichtete Kante hat ihre Gegenkante) **und** (b) signiertes Netz-Volumen == `Solid.volume_mm3` | Adapter-Test am echten OCC-Netz | Kollaps-Prüfung entfernt ⇒ rot. **Die Divergenz-Sonde ALLEIN hätte HIGH-1 nicht gefangen** (gemessen): die Normalensumme bleibt 0, weil sich die zwei gegenläufigen Lappen auslöschen — deshalb ist (b) Pflicht, nicht Kür |
| 6 | **Eine zulässige Stärken-Änderung beschädigt keine fremde Wand** — die exakte Reproduktion des Review-Wegs: drei Wände, `setWallThickness(lange Wand, 1000)`, die **nicht angefasste** kurze Wand bleibt gesund | Dienst-Test (Kern + OCC) | Kollaps-Prüfung entfernt ⇒ rot (12 → 8 Dreiecke an der fremden Wand, gemessen) |
| 7 | **Die drei Volumen-Wege stimmen überein** — `Solid.volume_mm3`, das analytische Auswertungs-Volumen und das signierte Netz-Volumen | Adapter-Test | Kollaps-Prüfung entfernt ⇒ rot (33 % Differenz im kaputten Fall). Auf gesunder Geometrie stimmen alle drei **exakt** (Review-Messung über sechs Zug-Richtungen) |
| 8 | **Der interaktive Weg liefert eine gesunde Wand** — die im Review gemessene Geste (starker Zoom, 1,2-mm-Zug auf einem gefangenen Endpunkt) erzeugt einen **geschlossenen** Körper und den Ausgang „angelegt" | `CanvasWidget`-Test, headless | Kollaps-Prüfung entfernt ⇒ rot. **Bewusst KEINE Ablehnung:** 1,2 mm liegt über der Geometrie-Toleranz, die Wand ist zulässig — sie muss **gesund** entstehen, nicht verhindert werden |
| 9 | **Der Eckenschluss des interaktiven Pfads wird an der ECKE geprüft, nicht an Koordinaten** (MEDIUM-2) — zwei Züge, ein **Grad-2**-Knoten, Abtast-Sonde auf Dichtheit | `CanvasWidget`-Test. **Fixture-Vorbedingung:** die Zug-Enden dürfen **nicht** auf einem Endpunkt der Fixture-Wände landen — sonst entsteht ein Grad-3-Knoten, an dem die Spezifikation **stumpfe** Enden vorschreibt und gar keine Ecke zu prüfen ist (der Bestands-Test tut genau das) | Eck-Konstruktion im Kern umgangen ⇒ rot |
| 10 | **Der Bestand bleibt unverändert** — Footprint-, Raum-, Auswertungs-, Export- und Persistenz-Orakel | Bestands-Orakel | (Regressions-Netz) |

**Elf Zeilen tragen einen eigenen Sensor** (1–9 inkl. 3a und 4a), eine ist **Netz** (10).

**Was ausdrücklich KEIN eigenes Orakel bekommt:** die Adapter-seitige Degenerations-Prüfung
(`isDegenerate`, MEDIUM-3). Sie bleibt **unverändert**; dass sie den Selbstschnitt nicht sehen **kann**,
ist im Plan benannt (§2.2) und durch §4-2/§4-3 gegenstandslos gemacht — der Kern liefert kein solches
Polygon mehr.

## 5. Definition of Done

- [x] **R1 vor dem Start entschieden** (§2.3) — **gemessen im ersten Plan-Lauf**, und **gegen** die
      erste Fassung: der Rückfall ist **lokal**, nicht symmetrisch (450/841 statt 750/1171
      Abtastpunkte), und er braucht eine **Kandidaten-Suche**, weil ein einzelner Schritt die andere
      Ecke kaputtmacht (`minDot` +5000 → −5000).
- [ ] **`src/hexagon/services/geometry/wall_footprint.{h,cpp}`**: Kollaps-Kriterium
      (**Kantenrichtungs-Erhalt**, Tie-Break wie `room_detection.cpp`) am **fertigen Polygon**, plus
      die **geordnete Kandidaten-Liste** (§2.3 a). **Lokal** — der Nachbar-Footprint wird nicht
      angefasst. Total, wirft nie. Orakel §4-1..4a.
- [ ] **`spec/lastenheft.md` — eine neue Boundary-Zeile bei
      [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden).** Die erste
      Fassung dieses Plans behauptete, das Lastenheft bleibe unberührt, „denn die Zusage gilt
      unverändert" — **das ist gemessen falsch** (Lauf-1-HIGH-3): eine Wand unterhalb der Schwelle
      fällt unter den **Happy Path** (gleiche Höhe, gemeinsamer Endpunkt, im Winkel), und dessen
      Ausnahmen-Liste ist **abschließend**. Nach dem Fix ist die Ecke dort gemessen **nicht**
      geschlossen. Also bekommt sie eine Boundary-Zeile — **nach dem Muster, das das Lastenheft für
      den Miter-Limit-Rückfall selbst verwendet** („sonst enden beide Wände stumpf"), also
      **benutzer-beobachtbar und lösungsfrei**
      ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
      *„Given eine Wand, die im Verhältnis zu ihren Nachbarn sehr kurz ist, then enden die Wände an
      dieser Ecke stumpf."* **Mechanik gehört nicht hinein** — kein „Rücksprung", kein
      „Kantenrichtungs-Erhalt". Plus **Historien-Zeile und Header-Version**
      ([MR-010](../../../../harness/conventions.md) — Header-Version == oberste Historie-Zeile).
- [ ] **`spec/spezifikation.md`**: der Block „Begrenzung und Rückfälle" von
      [`LH-FA-WAL-006.a`](../../../../spec/spezifikation.md#lh-fa-wal-006a--eckenschluss-footprint-regel-teilumfang) bekommt
      den **dritten** Rückfall samt Kriterium und Kandidaten-Suche. **Keine neue Regel, sondern die
      Anwendung einer vorhandenen** — das Kriterium steht bereits in der Raumerkennungs-Sektion; der
      Text sagt das ausdrücklich, damit niemand zwei Kriterien für dieselbe Sache vermutet. **Hier**
      lebt die Mechanik, nicht im Lastenheft.
- [ ] **Tests**: `tests/hexagon/test_wall_footprint.cpp` (§4-1..4) · `tests/adapters/test_occ_geometry_adapter.cpp`
      (§4-5, §4-7 — die **ersten** Invarianten-Sonden für Wand-Netze im Repo) ·
      `tests/hexagon/test_structure_edit_service.cpp` oder ein neuer Dienst-Test (§4-6) ·
      `tests/adapters/test_canvas_widget.cpp` (§4-8, §4-9).
- [ ] **Die Invarianten-Sonde wird wiederverwendbar abgelegt** (MEDIUM-1): Kanten-Manifold **und**
      Volumen-Übereinstimmung als Test-Helfer, damit die nächste Bauteil-Familie sie nicht neu
      erfindet. Für **Dach** und **Treppe** existieren sie bereits — für **Wände** bisher nicht.
- [ ] **Orakel §4-1 bis §4-9 (inkl. 3a und 4a) je mit roter Gegenprobe** im Closure-Text,
      **einzeln** gemessen; §4-10 als **Netz** benannt.
- [ ] **`make gates` grün**; **`make io-smoke` grün**; **`make golden-check` grün** —
      **letzteres ist hier nicht Routine, sondern die Kernfrage der Verhaltens-Invarianz.**
      **Erwartung: byte-identisch — und der Beleg dafür ist im Lauf 1 gemessen**, am **richtigen**
      Modell: `golden-check` erzeugt aus `tests/adapters/golden_model.cpp` (nicht aus dem
      Demo-Aufbau in `src/main.cpp`); dessen Wände liegen bei `minDot` +57 600/+90 000, die
      Demo-Wände bei ≥ +13 225 — **keine** ist selbstschneidend, keine liegt unter der Schwelle.
      **Weicht trotzdem ein Byte ab, ist das ein Befund und keine Nebenwirkung.**
- [ ] **`make acc-002-beleg` grün**; das Bild wird **nicht** committet (Abnahme-Artefakt von
      [`slice-012`](../done-archive/slice-012-eckenschluss-wal006-teil.md)).
- [ ] **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report
      nachgezogen**: HIGH-1 und MEDIUM-1/2/3 als **behoben** vermerkt, mit Verweis auf die Orakel.
- [ ] **Die [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Abweichung
      wird VERANKERT, nicht nur erwähnt** (Lauf-1-MEDIUM-5): sie steht heute nur in zwei
      Slice-Dateien. Sie gehört in die **Roadmap** (Wellen-Block) und in das **Wellen-Ergebnis** —
      sonst liest sie sich nach der Closure wie erledigt. **Die Regel kennt keine Vertagungs-Klausel;
      die Entscheidung des Projektinhabers ist der einzige Grund, und der muss auffindbar sein.**
- [ ] **CHANGELOG** [Unreleased]-Eintrag. **Kein** Handbuch-Nachzug: der Benutzer sieht keine neue
      Fähigkeit, sondern eine, die vorher still kaputtging — **das gehört in den CHANGELOG, nicht ins
      Handbuch** (das Handbuch beschreibt, was man tun kann, nicht welche Fehler behoben sind).
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/services/geometry/wall_footprint.cpp` | ändern | Kollaps-Kriterium + symmetrischer Rückfall |
| `spec/lastenheft.md` | ändern | neue Boundary-Zeile bei [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) + Historie + Header-Version |
| `spec/spezifikation.md` | ändern | dritter Rückfall im Eckenschluss-Block ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden).a) |
| `tests/hexagon/test_wall_footprint.cpp` | ändern | §4-1..4 |
| `tests/adapters/test_occ_geometry_adapter.cpp` | ändern | §4-5, §4-7 (erste Wand-Invarianten) |
| `tests/adapters/`-Dienst-Test (OCC-fähiges Ziel) | neu/ändern | §4-6 — **nicht** `tests/hexagon/`: die Zeile prüft Dreiecke am echten OCC-Netz, und das Kern-Testziel ist **dependency-frei** (kein Qt/OCC) |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-8, §4-9 |
| `tests/`-Helfer für die Invarianten-Sonden | neu/ändern | MEDIUM-1, wiederverwendbar |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/2026-07-29-mr-009-bauteil-strang.md` | ändern | Befund-Status |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `src/adapters/ui/**` (außer Tests), `src/adapters/io/**`,
`src/adapters/persistence/**`, `src/adapters/geometry/occ_solids.cpp` (§2.2),
`src/hexagon/services/structure_edit_service.cpp` (**geprüft, nicht angenommen**: der Rückfall ist
**lokal**, der Melde-/Rebuild-Radius bleibt damit **einstufig** — das war der Grund, warum die
symmetrische Variante ihn angefasst hätte), `data-model.yaml`/`schema.sql`, alle ADRs.

## 7. Risiken

- **R1 — der Rückfall ist ENTSCHIEDEN, aber seine Abgeschlossenheit bleibt das Risiko** (§2.3). Die
  Kandidaten-Liste terminiert per Konstruktion; **dass sie in jeder Lage ein gültiges Polygon
  liefert, ist zu messen** — der letzte Kandidat (stumpf/stumpf) ist immer gültig, aber die Reihenfolge
  entscheidet, welcher gewinnt, und eine falsch geordnete Liste liefert unnötig oft stumpf.
- **R2 — der Fix ändert Geometrie im Bestand.** Der Rückfall greift genau dort, wo bisher ein
  kaputtes Polygon entstand — **aber das ist zu belegen, nicht anzunehmen**: `make golden-check`
  byte-identisch und die Auswertungs-Orakel unverändert. **Ein Byte Abweichung ist ein Befund.**
- **R3 — die Schwellen hängen an WINKEL und Stärken, nicht nur an den Stärken.** Die erste Fassung
  dieses Plans rechnete „halbe bzw. volle Nachbar-Stärke" — **gemessen falsch** (Lauf-1-MEDIUM-2):
  rechtwinklig/gleich dick stimmt es, bei θ = 120° liegt die Schwelle bei **207,75** mm statt 120,
  und bei Nachbarn 240/1000 bei **619,75** mm = (t₁+t₂)/2. **Die Orakel prüfen relativ** (knapp
  darunter ⇒ stumpf, knapp darüber ⇒ gemitert) **oder bestimmen die Schwelle aus der Lage** — feste
  Zahlen sind nur mit ihrer Fixture gültig, und das gehört in die Zeile (§4-1, §4-3a).
- **R4 — die Fixture-Falle des Bestands-Tests** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-MEDIUM-2): zwei Züge auf einen Endpunkt der
  Fixture-Wände ergeben einen **Grad-3-Knoten**, an dem gar keine Ecke entsteht. §4-9 braucht eine
  Lage, die einen **Grad-2**-Knoten erzeugt.
- **R5 — die Divergenz-Sonde allein trügt.** Die Normalensumme des kaputten Körpers ist **0**, weil
  sich die gegenläufigen Lappen auslöschen. Wer nur sie baut, hat eine grüne Sonde über einem offenen
  Körper — **die sechste Wiederholung der Instrument-Klasse in diesem Strang.**
- **R6 — die Zusage, die WEGFÄLLT, muss im Lastenheft stehen.** Der Rückfall nimmt der
  Happy-Path-Zusage einen Fall weg (Lauf-1-HIGH-3). Ohne die Boundary-Zeile stünde eine Zusage im
  abnahmebindenden Dokument, die die Software gemessen nicht hält — **das ist schlimmer als der
  Fehler, den der Slice behebt**, weil es unsichtbar ist.


## 8. Trigger

- [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report
  vom 2026-07-29, HIGH-1 und MEDIUM-1/2/3.

## 9. Closure-Trigger

- §4-1 bis §4-9 grün + **je einzeln** diskriminierend belegt; §4-10 als Netz grün; `make gates`,
  `make io-smoke`, `make golden-check` und `make acc-002-beleg` grün; Spec-Nachzug vollzogen;
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report
  auf „behoben" nachgezogen; Closure-Notiz.
- **Danach ist die Welle closure-fähig** — mit der **benannten Vertagung** von HIGH-2
  ([`slice-061`](slice-061-eckenschluss-ungleiche-hoehen.md)), die im Wellen-Ergebnis als
  **Abweichung von [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)**
  steht, nicht als erledigt.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: Geometrie-Kern (Wand-Footprint)

- **Modus:** GF; **Dichte:** mittel — eine Kern-Regel, ein Spec-Nachzug, zehn Orakel-Zeilen, davon
  drei an Nähten, die es im Repo für Wände **noch nie** gab.
- **Risiko:** **hoch** — es ist der erste Kern-Geometrie-Eingriff seit welle-2, und er ändert die
  Form ausgelieferter Körper. Das Netz dagegen ist ungewöhnlich dicht (Golden, Auswertung, Räume,
  Export, Persistenz).
- **Warum kein Split:** die vier Orakel-Gruppen (Kriterium · Rückfall · Invarianten · interaktiver
  Weg) prüfen **eine** Änderung aus vier Richtungen. Getrennt lieferte die erste Hälfte eine Regel
  ohne Beleg, dass sie im Produkt ankommt.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-07-29)

Report: [`2026-07-29-slice-060-plan.md`](../../../reviews/2026-07-29-slice-060-plan.md) —
**3 HIGH / 5 MEDIUM / 4 LOW / 2 INFO + 12 Negativbefunde, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor.

**Der Lauf hat die zentrale Entwurfsentscheidung dieses Plans widerlegt — mit Zahlen, die in die
andere Richtung zeigen als meine Begründung.**

| # | Behandlung |
|---|---|
| **HIGH-1** (der symmetrische Rückfall ist **nicht abgeschlossen**: gemessen erzeugt er an der **anderen** Ecke einer gesunden Wand einen **neuen** Selbstschnitt — `minDot` +5000 → −5000; analytisch: das Stumpf-Setzen nimmt den Ausgleich zwischen den Längsversätzen weg, `\|s−e\|` wächst) | **§2.3 (a) ist neu:** der Footprint wird über eine **geordnete Kandidaten-Liste** bestimmt (gemitert/gemitert → gemitert/stumpf → stumpf/gemitert → stumpf/stumpf), der erste gültige gewinnt, der letzte ist immer gültig. **Terminiert per Konstruktion**, und §4-4a prüft **genau die gemessene Lage**. |
| **HIGH-2** (Orakel §4-4 verlangte Nahtlosigkeit von zwei stumpfen Enden — **geometrisch unmöglich**; gemessen: symmetrisch **750 doppelt / 1171 leer**, asymmetrisch **450/841** — die **gewählte** Variante ist in **beiden** Kennzahlen schlechter als die verworfene) | **Die Entscheidung ist umgedreht: der Rückfall ist LOKAL.** Und §4-4 misst jetzt, was er **wirklich** zusagt (gültiges Polygon, stumpfes Referenz-Polygon, **unveränderter Nachbar**) statt Dichtheit. **Meine Begründung war nicht nur schwach, sie war widerlegt** — ich hatte Nahtlosigkeit als Vorteil der Symmetrie behauptet, und sie ist bei zwei stumpfen Enden nie erreichbar. |
| **HIGH-3** (der Rückfall bricht die **Happy-Path-AK** von [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), deren Ausnahmen-Liste abschließend ist — und die DoD behauptete ausdrücklich, das Lastenheft bleibe unberührt, „denn die Zusage gilt unverändert") | **Das Lastenheft bekommt eine Boundary-Zeile**, nach dem Muster des Miter-Limit-Rückfalls, **lösungsfrei** formuliert, plus Historie und Header-Version. **Die Vertrags-Behauptung war gemessen falsch** — eine Zusage, die die Software nicht hält, ist schlimmer als der Fehler, den der Slice behebt (R6). |
| **MEDIUM-1** (die Zahlenreihe von §4-1 stammt **komplett** aus der Zwei-Ecken-Fixture; in der Ein-Eck-Lage sind 241/240/239/121 **alle** „erhalten" — und bei `len = 240` ist das Skalarprodukt **exakt 0**, `room_detection.cpp` nutzt dort `<= 0`) | Zeile nennt jetzt **ihre Fixture** und **die Tie-Break-Konvention**. |
| **MEDIUM-2** (die Schwellen gelten nur rechtwinklig/gleich dick: θ = 120° ⇒ 207,75 statt 120; Nachbarn 240/1000 ⇒ 619,75 = (t₁+t₂)/2 — R3s Rechenvorschrift war falsch) | **Neue Zeile §4-3a** + R3 korrigiert: die Orakel prüfen **relativ** oder bestimmen die Schwelle aus der Lage. |
| **MEDIUM-3** (die Symmetrie machte den Melde-/Rebuild-Radius **zweistufig**, `rebuildAffectedNeighbors` ist einstufig, und `structure_edit_service.cpp` stand in keiner §6-Zeile) | **Durch die lokale Entscheidung gegenstandslos** — und §6 sagt das jetzt ausdrücklich, statt zu schweigen. Der Befund ist damit zugleich das **dritte** Argument für „lokal". |
| **MEDIUM-4** (§4-6 „Kern + OCC" war dem **dependency-freien** Kern-Testziel zugewiesen) | §6 verortet die Zeile im OCC-fähigen Adapter-Ziel. |
| **MEDIUM-5** (die [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Abweichung ist nur in zwei Slice-Dateien verankert — nicht in der Roadmap, in keiner DoD-Zeile) | **Eigene DoD-Zeile**: Roadmap **und** Wellen-Ergebnis. Sonst liest sich die Vertagung nach der Closure wie erledigt. |
| **LOW-1** (der Plan begründete die Golden-Erwartung mit dem **falschen** Modell: `golden-check` erzeugt aus `golden_model.cpp`, nicht aus dem Demo-Aufbau) | Korrigiert — **und die Erwartung ist am richtigen Modell gemessen bestätigt** (`minDot` +57 600/+90 000). |

**Positiv bestätigt — der wichtigste Negativbefund:** das **Kollaps-Kriterium trägt exakt**. Der
Reviewer hat einen Sweep über **4 416 000** Konfigurationen (1°–179°, acht Stärke-Paare 50–1000,
Längen bis 2500 mm) gegen einen unabhängigen Segment-Schnitt-Test gefahren: **null Fehl-Negative**;
die einzigen 28 Abweichungen liegen **exakt auf** der Schwelle (Doppelpunkt, kein Selbstschnitt).
Analytisch belegt: Stirnkanten können nie umkehren, Längskanten kehren **genau dann** um, wenn
`len < |s − e|` — dieselbe Bedingung wie der Stirnkanten-Schnitt. **Die Grundlage des Slice steht;
falsch war, was ich daraus gebaut habe.**

**Startbar:** **nein.** Drei HIGH sind eingearbeitet, davon zwei mit **umgedrehter** Entscheidung —
das verlangt eine unabhängige Prüfung der Einarbeitung. Auftrag an Lauf 2, eng: **ist die
Kandidaten-Liste in der richtigen Reihenfolge und liefert sie in jeder Lage ein gültiges Polygon —
und misst §4-4 jetzt eine Zusage, die die lokale Bauart auch halten kann?** Zusätzlich: **ist die
neue Lastenheft-Zeile lösungsfrei und deckt sie den Fall vollständig?**

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
