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

### 2.3 Der Rückfall ist NICHT symmetrisch — und das ist die eigentliche Entwurfsfrage

Fällt Wand A auf **stumpf** zurück, ist ihr Nachbar B davon zunächst unberührt: B baut seine Ecke aus
**seiner** Geometrie und A's Stärke weiter als Gehrung. An dieser Ecke entstünde dann eine
**Überlappung oder Kerbe** — genau das, was die Spezifikation für den Eckenschluss ausschließt
(„nahtlos, keine Überlappung, keine Kerbe").

| Variante | Konsequenz |
|---|---|
| **A fällt zurück, B mitert weiter** | die Zusage „nahtlos" ist an dieser Ecke verletzt; der Fehler ist kleiner als HIGH-1, aber es ist **derselbe Zusagen-Bruch in klein** |
| **Symmetrischer Rückfall** (gewählt): kollabiert die Ecke für **eine** der beiden Wände, enden **beide** dort stumpf | eine Ecke, eine Entscheidung; die Zusage bleibt heil. `cornerAt` kennt den Nachbarn bereits vollständig und kann dieselbe Prüfung für **dessen** Footprint mitführen |

**Entschieden: symmetrisch.** Das ist keine Geschmacksfrage — eine asymmetrische Lösung tauschte einen
großen Zusagen-Bruch gegen einen kleinen, statt ihn zu beheben. **Vor dem Start zu messen** (§7 R1): ob
die symmetrische Prüfung stabil ist, wenn **beide** Wände kurz sind (beide fallen zurück — erwartet:
zwei stumpfe Enden, die sich an der Ecke berühren, kein Loch).

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
| 1 | **Das Kollaps-Kriterium ist rein und diskriminiert exakt** — Kantenrichtungs-Erhalt, geprüft an den im Review gemessenen Schwellen: `len` 600/241/240 ⇒ erhalten, 239/121/120/119/100 ⇒ umgekehrt | Kern-Test, **ohne** Qt/OCC (Bauform `test_wall_footprint.cpp`) | Flächen- statt Richtungs-Kriterium ⇒ rot (die Bowtie-Fläche ist unauffällig: `24 000 mm²`, derselbe Wert wie beim stumpfen Rechteck) |
| 2 | **Ein gemitertes Eck: unterhalb der halben Nachbar-Stärke wird stumpf** — der Footprint ist danach **nicht** selbstschneidend | Kern-Test, Fixture rechter Winkel, Stärken 240 | Rückfall entfernt ⇒ rot (Selbstschnitt) |
| 3 | **Zwei gemiterte Ecken: die Schwelle ist die VOLLE Nachbar-Stärke** — die Kombination, die eine Pro-Ecke-Prüfung verpasst | Kern-Test, Nachbarn auf **derselben** Seite | Prüfung pro Ecke statt am Polygon ⇒ rot bei `len = 239` |
| 4 | **Der Rückfall ist symmetrisch** — kollabiert die Ecke, enden **beide** Wände dort stumpf; die Ecke bleibt **nahtlos** (kein Loch, keine Überlappung) | Kern-Test + **Abtast-Sonde** über dem Eck-Kasten (Bauform des Review-Belegs: Raster über 240×240 mm, jeder Punkt in genau einem der beiden Footprints) | Rückfall nur bei der kurzen Wand ⇒ rot (Überlappung/Kerbe an der Ecke) |
| 5 | **Der ausgelieferte Wand-Körper ist geschlossen und konsistent orientiert** — **zwei** Sonden, weil eine nicht genügt: (a) Kanten-Manifold (jede gerichtete Kante hat ihre Gegenkante) **und** (b) signiertes Netz-Volumen == `Solid.volume_mm3` | Adapter-Test am echten OCC-Netz | Kollaps-Prüfung entfernt ⇒ rot. **Die Divergenz-Sonde ALLEIN hätte HIGH-1 nicht gefangen** (gemessen): die Normalensumme bleibt 0, weil sich die zwei gegenläufigen Lappen auslöschen — deshalb ist (b) Pflicht, nicht Kür |
| 6 | **Eine zulässige Stärken-Änderung beschädigt keine fremde Wand** — die exakte Reproduktion des Review-Wegs: drei Wände, `setWallThickness(lange Wand, 1000)`, die **nicht angefasste** kurze Wand bleibt gesund | Dienst-Test (Kern + OCC) | Kollaps-Prüfung entfernt ⇒ rot (12 → 8 Dreiecke an der fremden Wand, gemessen) |
| 7 | **Die drei Volumen-Wege stimmen überein** — `Solid.volume_mm3`, das analytische Auswertungs-Volumen und das signierte Netz-Volumen | Adapter-Test | Kollaps-Prüfung entfernt ⇒ rot (33 % Differenz im kaputten Fall). Auf gesunder Geometrie stimmen alle drei **exakt** (Review-Messung über sechs Zug-Richtungen) |
| 8 | **Der interaktive Weg liefert eine gesunde Wand** — die im Review gemessene Geste (starker Zoom, 1,2-mm-Zug auf einem gefangenen Endpunkt) erzeugt einen **geschlossenen** Körper und den Ausgang „angelegt" | `CanvasWidget`-Test, headless | Kollaps-Prüfung entfernt ⇒ rot. **Bewusst KEINE Ablehnung:** 1,2 mm liegt über der Geometrie-Toleranz, die Wand ist zulässig — sie muss **gesund** entstehen, nicht verhindert werden |
| 9 | **Der Eckenschluss des interaktiven Pfads wird an der ECKE geprüft, nicht an Koordinaten** (MEDIUM-2) — zwei Züge, ein **Grad-2**-Knoten, Abtast-Sonde auf Dichtheit | `CanvasWidget`-Test. **Fixture-Vorbedingung:** die Zug-Enden dürfen **nicht** auf einem Endpunkt der Fixture-Wände landen — sonst entsteht ein Grad-3-Knoten, an dem die Spezifikation **stumpfe** Enden vorschreibt und gar keine Ecke zu prüfen ist (der Bestands-Test tut genau das) | Eck-Konstruktion im Kern umgangen ⇒ rot |
| 10 | **Der Bestand bleibt unverändert** — Footprint-, Raum-, Auswertungs-, Export- und Persistenz-Orakel | Bestands-Orakel | (Regressions-Netz) |

**Neun Zeilen tragen einen eigenen Sensor** (1–9), eine ist **Netz** (10).

**Was ausdrücklich KEIN eigenes Orakel bekommt:** die Adapter-seitige Degenerations-Prüfung
(`isDegenerate`, MEDIUM-3). Sie bleibt **unverändert**; dass sie den Selbstschnitt nicht sehen **kann**,
ist im Plan benannt (§2.2) und durch §4-2/§4-3 gegenstandslos gemacht — der Kern liefert kein solches
Polygon mehr.

## 5. Definition of Done

- [ ] **R1 vor dem Start entschieden** (§2.3, §7): die symmetrische Rückfall-Regel ist **an einer
      Messung** zu bestätigen — beide Wände kurz ⇒ zwei stumpfe Enden, kein Loch.
- [ ] **`src/hexagon/services/geometry/wall_footprint.{h,cpp}`**: Kollaps-Kriterium
      (**Kantenrichtungs-Erhalt**) am **fertigen Polygon**, symmetrischer Rückfall auf stumpf. Total,
      wirft nie. Orakel §4-1..4.
- [ ] **`spec/spezifikation.md`**: der Block „Begrenzung und Rückfälle" von
      [`LH-FA-WAL-006.a`](../../../../spec/spezifikation.md#lh-fa-wal-006a--eckenschluss-footprint-regel-teilumfang) bekommt
      den **dritten** Rückfall. **Keine neue Regel, sondern die Anwendung einer vorhandenen** — das
      Kriterium steht bereits in der Raumerkennungs-Sektion; der Text sagt das ausdrücklich, damit
      niemand zwei Kriterien für dieselbe Sache vermutet. **Lösungsfrei bleibt das Lastenheft**
      ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
      es ändert sich **nicht**, denn die Zusage „geschlossene Ecke ohne Kerbe" gilt unverändert.
- [ ] **Tests**: `tests/hexagon/test_wall_footprint.cpp` (§4-1..4) · `tests/adapters/test_occ_geometry_adapter.cpp`
      (§4-5, §4-7 — die **ersten** Invarianten-Sonden für Wand-Netze im Repo) ·
      `tests/hexagon/test_structure_edit_service.cpp` oder ein neuer Dienst-Test (§4-6) ·
      `tests/adapters/test_canvas_widget.cpp` (§4-8, §4-9).
- [ ] **Die Invarianten-Sonde wird wiederverwendbar abgelegt** (MEDIUM-1): Kanten-Manifold **und**
      Volumen-Übereinstimmung als Test-Helfer, damit die nächste Bauteil-Familie sie nicht neu
      erfindet. Für **Dach** und **Treppe** existieren sie bereits — für **Wände** bisher nicht.
- [ ] **Orakel §4-1 bis §4-9 je mit roter Gegenprobe** im Closure-Text, **einzeln** gemessen; §4-10 als
      **Netz** benannt.
- [ ] **`make gates` grün**; **`make io-smoke` grün**; **`make golden-check` grün** —
      **letzteres ist hier nicht Routine, sondern die Kernfrage der Verhaltens-Invarianz:** ändert der
      Fix eine Wand-Geometrie im Demo-Modell, ändern sich die Export-Golden. **Erwartung: byte-identisch**
      (das Demo-Modell hat keine Wand unterhalb der Schwelle) — **weicht auch nur ein Byte ab, ist das
      ein Befund und keine Nebenwirkung**, und der Grund gehört in die Closure.
- [ ] **`make acc-002-beleg` grün**; das Bild wird **nicht** committet (Abnahme-Artefakt von
      [`slice-012`](../done-archive/slice-012-eckenschluss-wal006-teil.md)).
- [ ] **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report
      nachgezogen**: HIGH-1 und MEDIUM-1/2/3 als **behoben** vermerkt, mit Verweis auf die Orakel.
- [ ] **CHANGELOG** [Unreleased]-Eintrag. **Kein** Handbuch-Nachzug: der Benutzer sieht keine neue
      Fähigkeit, sondern eine, die vorher still kaputtging — **das gehört in den CHANGELOG, nicht ins
      Handbuch** (das Handbuch beschreibt, was man tun kann, nicht welche Fehler behoben sind).
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/services/geometry/wall_footprint.cpp` | ändern | Kollaps-Kriterium + symmetrischer Rückfall |
| `spec/spezifikation.md` | ändern | dritter Rückfall im Eckenschluss-Block ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden).a) |
| `tests/hexagon/test_wall_footprint.cpp` | ändern | §4-1..4 |
| `tests/adapters/test_occ_geometry_adapter.cpp` | ändern | §4-5, §4-7 (erste Wand-Invarianten) |
| `tests/hexagon/test_structure_edit_service.cpp` | ändern | §4-6 |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-8, §4-9 |
| `tests/`-Helfer für die Invarianten-Sonden | neu/ändern | MEDIUM-1, wiederverwendbar |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/2026-07-29-mr-009-bauteil-strang.md` | ändern | Befund-Status |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `src/adapters/ui/**` (außer Tests), `src/adapters/io/**`,
`src/adapters/persistence/**`, `src/adapters/geometry/occ_solids.cpp` (§2.2),
`spec/lastenheft.md`, `data-model.yaml`/`schema.sql`, alle ADRs.

## 7. Risiken

- **R1 — die Symmetrie des Rückfalls** (§2.3). Vor dem Start zu messen: beide Wände kurz ⇒ beide
  stumpf, kein Loch. Und: die Prüfung darf **nicht** rekursiv werden (A prüft B prüft A) — sie läuft
  auf den **stumpfen** Referenz-Polygonen, die ohne Nachbar-Wissen entstehen.
- **R2 — der Fix ändert Geometrie im Bestand.** Der Rückfall greift genau dort, wo bisher ein
  kaputtes Polygon entstand — **aber das ist zu belegen, nicht anzunehmen**: `make golden-check`
  byte-identisch und die Auswertungs-Orakel unverändert. **Ein Byte Abweichung ist ein Befund.**
- **R3 — die Schwellen sind stärke-abhängig, nicht absolut.** Bei 1000 mm Stärke liegt die Schwelle
  bei 500 mm (ein Eck) bzw. 1000 mm (zwei) — **eine 900-mm-Wand ist kurz genug**. Die Orakel müssen
  die Schwelle aus den Stärken rechnen, nicht Zahlen einsetzen.
- **R4 — die Fixture-Falle des Bestands-Tests** (MEDIUM-2): zwei Züge auf einen Endpunkt der
  Fixture-Wände ergeben einen **Grad-3-Knoten**, an dem gar keine Ecke entsteht. §4-9 braucht eine
  Lage, die einen **Grad-2**-Knoten erzeugt.
- **R5 — die Divergenz-Sonde allein trügt.** Die Normalensumme des kaputten Körpers ist **0**, weil
  sich die gegenläufigen Lappen auslöschen. Wer nur sie baut, hat eine grüne Sonde über einem offenen
  Körper — **die sechste Wiederholung der Instrument-Klasse in diesem Strang.**

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

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung

_(bei Ausführung auszufüllen)_

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
