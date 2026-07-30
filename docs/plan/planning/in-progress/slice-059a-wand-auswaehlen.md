---
id: slice-059a
titel: Wand auswählen — Treffer-Prüfung, Lebensdauer, sichtbare Hervorhebung ([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E3/E16/E17)
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 059a: Wand auswählen

**Status:** open — **STARTBAR** (2026-07-29). Zwei
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe
mit zwei verschiedenen Reviewern; der zweite meldet **0 HIGH**, damit greift die vorab festgelegte
Abbruchregel. Einarbeitungen in §11 (Lauf 1, samt Teilung) und §11a (Lauf 2). Die Parameter-Hälfte ist [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md).

**Welle:** welle-6-interaktiv-planen. **Der Abschluss-Trigger hängt an
[`slice-059b`](../open/slice-059b-wand-parameter-aendern.md)**, nicht hier — dieser Slice liefert dessen
Voraussetzung und ist **für sich** abnehmbar: der Benutzer wählt eine Wand und **sieht**, welche.

**Setzt voraus:** [`slice-057`](../done/slice-057-lese-naht-bauteil-identitaet.md) (Bauteil-Identität
im Segment) und [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) (Werkzeug-Modus,
Aktions-Gruppe) — beide werden **benutzt**, nicht neu gebaut.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-29.

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md), **Entscheidungen 3, 14, 16, 17**. Der Canvas kann
zwei Gesten (Hilfslinie, Wand) und **keine** Auswahl — es gibt kein Bauteil, auf das eine Bedienung
zeigen könnte.

## 1. Ziel

Im **Auswahl-Modus** wählt ein Zug **genau eine** Wand des **dargestellten** Geschosses; sie ist auf
der Fläche **erkennbar** hervorgehoben, und die Auswahl ist als Widget-Eigenschaft lesbar. **Die
Auswahl fällt**, sobald ihr Bezug fallen könnte.

## 2. Die drei Entwurfs-Fragen dieses Slice

### 2.1 Die Treffer-Prüfung ist der Spiegel des Fangs — mit drei Unterschieden, die zählen

Bauform wie [`snap.h`](../../../../src/adapters/ui/view/snap.h): eine **reine, display-freie**
Funktion in `view/` aus `PlanView` + `ViewTransform` + Cursor-Pixel. Getroffen wird im
**Bildschirmraum** mit eigener Pixel-Toleranz (zoom-unabhängige Trefferfläche); **kein Default** für
den Schwellwert (Lehre slice-053: ein Default-Argument versteckt die Kalibrierung).

| | Fang (`snapTarget`) | Treffer-Prüfung (dieser Slice) |
|---|---|---|
| Geometrie | Abstand zu **Endpunkten** | Abstand zum **Segment** (eine Wand ist eine Strecke, kein Punkt) |
| Geschosse | **alle** (Koordinaten-Aussage) | **nur das dargestellte** (Zeige-Aussage) |
| Segment-Arten | Wand-Achsen **und** Hilfslinien | **nur Wand-Achsen** (`PlanSegmentKind::WallAxis`) |

**Warum die Geschoss-Beschränkung nicht optional ist:** das Demo-Modell legt in EG und OG **dieselben
vier Außenwände** an. Ohne Beschränkung träfe ein Klick auf eine sichtbare OG-Außenwand
**deterministisch** die unsichtbare des EG — angezeigt **und** (in
[`slice-059b`](../open/slice-059b-wand-parameter-aendern.md)) geändert würde die falsche. **Ein
reproduzierbar falscher Treffer ist schlimmer als ein zufälliger**, weil er zuverlässig am Benutzer
vorbeigeht.

**Rückgabe ist die starke Id** (`std::optional<model::WallId>`), nicht der Zahlwert: `ui_view → model`
ist eine deklarierte Kante, und eine `int`-Rückgabe wäre an der ersten Verwechslung mit einer
`GuideLineId` still falsch.

**Entschieden: der Treffer wird beim LOSLASSEN bestimmt**, nicht beim Drücken — dieselbe Stelle, an
der die zwei Bestands-Gesten ihr Ergebnis festlegen (`mouseReleaseEvent`). Ein Auswählen beim
Drücken wäre die dritte Variante an derselben Naht und bei einem Zug (Press an A, Release an B)
mehrdeutig.

### 2.2 Die Lebensdauer der Auswahl braucht DREI Regeln — und der Auslöser für die erste existiert schon

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E17 verlangt das Verwerfen bei (a) Modell-Ersetzung,
(b) Geschoss-Wechsel, (c) Verschwinden aus der Sicht. **Am Artefakt nachgesehen, nicht erraten:**

- **(a) hat einen exakten, vorhandenen Auslöser.** `replaceBuilding` meldet **genau eine**
  Full-Refresh-Meldung mit `op == ModelReplaced` — bei **jedem** Öffnen und **jedem** Anlegen, auch
  bei einem Projekt ohne Geschosse. Die Auswahl fällt **auf diese Meldung**, ohne Heuristik.
- **(b)** fällt in `setActiveStorey`. **Ehrlich benannt:** eine **Bedienung**, die das Geschoss
  wechselt, gibt es in dieser Ausbaustufe **nicht** — `setActiveStorey` hat genau **eine** produktive
  Aufrufstelle, den Öffnen-/Anlegen-Pfad. Geprüft wird deshalb der **Widget-Vertrag** (§4-7); der
  benutzer-seitige Auslöser ist heute derselbe wie (a). Das ist dieselbe Erreichbarkeits-Ehrlichkeit,
  die [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E8 für [`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
  verlangt — **nicht** eine Zusage über eine Bedienung, die es nicht gibt.
- **(c)** ist ein **Netz**: nach jeder anderen Meldung wird geprüft, ob die gewählte Wand noch in der
  Sicht des aktiven Geschosses vorkommt; sonst fällt sie.

**Warum keine der drei die anderen ersetzt** — der Kern dieser Frage:

- **Nur (c) prüfen wäre die Fehlerklasse, vor der E17 warnt.** Der Id-Zähler wird beim Laden auf das
  Maximum des geladenen Projekts gesetzt, und ein geladenes Projekt trägt **dichte eigene Ids** —
  eine überlebende Auswahl bezeichnet mit hoher Wahrscheinlichkeit eine **andere, existierende**
  Wand. Die Prüfung „kommt sie noch vor?" sagte dann **ja**. Die Fehler-Barriere schwiege ebenfalls:
  `setWallThickness` wirft **nicht** bei einer gültigen fremden Id (im Plan-Review **gemessen**: kein
  Wurf, `applied = 300`, Modell-Ist 300). **Die Änderung träfe still das Falsche.**
- **Nur (a) + (b) wäre ohne Netz**, sobald ein Entfernen-Pfad entsteht (heute gibt es keinen — E12).
- **(b) steckt nicht in (a):** ein Geschoss-Wechsel ersetzt kein Modell — auch wenn ihn heute nur der
  Öffnen-Pfad auslöst.

**(c) ist heute strukturell nicht erreichbar** — es gibt kein `removeWall`, und jede
Modell-Ersetzung fällt schon unter (a). Das wird **so benannt** und bekommt **kein** Orakel, das
Erreichbarkeit vortäuscht.

### 2.3 Die Tinten-Sonde trägt hier NUR mit zwei Vorbedingungen — und einer Bauvorschrift

Die Hervorhebung ist ein Zeichen-Vorgang auf der Fläche, also der Fall, für den
[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E14 die zweite Nachweis-Ebene verlangt. **Die Sonde
allein trägt aber nicht** — im Plan-Review gemessen, **ohne** eine Zeile Hervorhebung:

| Vorgang | `snapPreview()` | Tinte |
|---|---|---|
| Ausgangslage | leer | **633** |
| Klick auf die **Mitte** der Achse | leer | **633** |
| Klick **3 px neben** einem Endpunkt | gesetzt | **689** |

**+56 Pixel aus dem Fang-Marker allein.** In
[`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) §4-8a war dieses Risiko geprüft und
strukturell entkräftet („jede `op`-Meldung löscht den Marker"). **Hier gilt das Argument nicht mehr —
und zwar aus derselben Tatsache, mit der man es hier erfüllt glauben möchte:** Auswählen ist **keine**
Mutation, also gibt es **kein** `op`, also läuft `invalidateSnapPreview` **nicht**, also bleibt der
Marker stehen.

**Daraus folgen zwei Vorbedingungen und eine Bauvorschrift:**

1. **Die Messpunkte liegen außerhalb der Fang-Nähe** jedes Endpunkts — geprüft als **`snapPreview()`
   leer an beiden** Messpunkten, nicht angenommen. (Ebenfalls zulässig: dieselbe Geste läuft in
   beiden Armen, dann kürzt der Marker sich heraus.)
2. **Die Abbildung ist unverändert** (Zoom **und** Modell-Ecke, Muster
   [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) §4-8a).
3. **Die Hervorhebung muss Fläche HINZUFÜGEN** — breiterer Stift oder zusätzlicher Marker. **Eine
   reine Umfärbung derselben Achse ist keine zulässige Umsetzung dieser Zeile:** die Sonde zählt
   Nicht-Weiß, nicht Farbe; eine gleich breite Linie in anderer Farbe erzeugte **null** zusätzliche
   Pixel, und die Zeile wäre rot bei korrekter Implementierung. Eine farbselektive Variante hülfe
   nicht — der Fang-Marker ist bereits rot.

**Das ist die vierte Wiederholung derselben Klasse** in diesem Strang: ein Instrument, das anderswo
getragen hat, trägt hier nur unter Bedingungen, die man **messen** muss, bevor man die Zusage
schreibt.

## 3. Bewusst NICHT Teil

- **Parameter anzeigen und ändern** — das ist [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md).
- **Mehrfach-Auswahl, Bereichs-Geste.** E3: höchstens **eine** Wand; benannter Re-Eval.
- **Gemeinsame Auswahl 2D↔3D, Selektion im Viewport.** E3: zieht den
  [ADR-0009](../../adr/0009-gui-framework-qt6.md)-Re-Eval (AIS/V3d, **Supersedes**-ADR).
- **Andere Bauteile auswählen.** Im Grundriss kommen nur Wand-Achsen und Hilfslinien vor.
- **Eine Bedienung zum Geschoss-Wechsel** (§2.2 b).
- **Jede Änderung an Kern, Persistenz, Export, Schema.** Auswahl und Modus sind UI-Zustand.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Die Treffer-Prüfung ist rein und display-frei** — Abstand zum **Segment**, Grenze fängt (Distanz == Schwellwert trifft noch), leere Sicht ⇒ kein Treffer | neue `view/`-Funktion, **ohne** `QWidget`/`QApplication` (Bauform `test_snap.cpp`) | Endpunkt- statt Segment-Abstand ⇒ rot (ein Klick auf die **Mitte** einer 4 m langen Achse trifft dann nicht mehr) |
| 2 | **Nur Wand-Achsen sind wählbar** — ein Klick auf eine Hilfslinie wählt **nichts** | dieselbe Funktion, Fixture mit Hilfslinie **und** Wand | `WallAxis`-Filter entfernt ⇒ die Hilfslinie wird gewählt ⇒ rot |
| 3 | **Nur das dargestellte Geschoss** — bei **deckungsgleichen** Achsen in zwei Geschossen trifft der Klick die Wand des **aktiven**, an der **Id** geprüft | dieselbe Funktion. **DREI Fixture-Vorbedingungen, alle tragend:** (a) das **aktive** Geschoss ist das **zweite** in der Iterationsreihenfolge — bei gleicher Distanz gewinnt sonst der zuerst besuchte, und die Gegenprobe lieferte **dieselbe** Id; (b) die zwei deckungsgleichen Achsen tragen **verschiedene** Wand-Ids (beim Bau der `PlanView` von Hand entsteht Deckungsgleichheit am bequemsten durch **Kopieren** des Segment-Literals — samt `origin`-Id, und dann ist die Zeile gehaltlos); (c) die Prüfung entscheidet bei gleicher Distanz für den **zuerst** Besuchten (§4-4a) — mit `<=` statt `<` wäre die Gegenprobe **genau dann** grün, wenn zusätzlich die E3-Regel verletzt ist | Geschoss-Filter entfernt ⇒ die Wand des **anderen** Geschosses ⇒ rot. **Die von [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E16 benannte symptomlose Fehlerklasse** |
| 4 | **Höchstens eine, die nächstgelegene** — bei zwei Achsen in Reichweite gewinnt die nähere; außerhalb der Reichweite ⇒ **kein** Treffer | dieselbe Funktion. **Fixture-Vorbedingung:** die **nähere** Achse muss in der Iteration **später** kommen — sonst wäre auch die Reihenfolge-Auswahl zufällig richtig | Reihenfolge- statt Distanz-Auswahl ⇒ rot |
| 4a | **Bei gleicher Distanz gewinnt der ZUERST Besuchte** — die von [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E3 entschiedene und in der Spezifikation stehende Tie-Break-Regel, Bauform `snap.cpp` (strikt `<`) | dieselbe Funktion, zwei Achsen in **exakt** gleicher Distanz | `<=` statt `<` ⇒ der zuletzt Besuchte gewinnt ⇒ rot. **Ohne diese Zeile hätte eine entschiedene Regel keinen Sensor** — und §4-3 hinge an ihr, ohne sie zu prüfen |
| 5 | **Der Zug im Auswahl-Modus wählt** (entschieden beim **Loslassen**, §2.1) — und **nur** dort: im Wand-/Hilfslinien-Modus wählt derselbe Zug nichts, im Auswahl-Modus entsteht nichts | `CanvasWidget`, headless, über `selection()` **und** den Modell-Zustand | Modus ignoriert ⇒ rot (**beide** Richtungen einzeln gemessen). **Der Loslassen-Konjunkt braucht eine EIGENE Gegenprobe:** ein Zug, dessen Enden **verschiedene** Ausgänge haben (Press außerhalb der Reichweite → Release auf der Achse und umgekehrt) — bei einem Klick (Press == Release) ist „beim Drücken entschieden" **nicht** von „beim Loslassen entschieden" unterscheidbar |
| 6 | **Die Auswahl fällt bei Modell-Ersetzung** — **auch wenn** der neue Stand eine Wand mit **derselben Id** trägt | `CanvasWidget` + Dienst: `replaceBuilding` mit einem Stand, der die gewählte Wand-Id **in einem Geschoss trägt, dessen Id das Widget aktiv hält**. **Beide Konjunkte sind nötig, gemessen:** gleiche Wand-Id + **fremde** Geschoss-Id ⇒ 0 Vorkommen in der Sicht, „Neu"-Gestalt (Geschoss ohne Wände) ⇒ 0 — in beiden Fällen verwirft das **Netz** (c) die Auswahl, und die Gegenprobe bliebe grün. **Der naheliegende Fixture-Griff „den Anlegen-Pfad nachbilden" ist genau der nicht-diskriminierende** | `ModelReplaced`-Behandlung entfernt ⇒ die Auswahl überlebt ⇒ rot. **Die Fixture ist der Punkt:** mit einem Stand **ohne** diese Id wäre die Zeile auch ohne die Behandlung grün (das Netz griffe), und die eigentliche Fehlerklasse bliebe unbelegt |
| 7 | **Die Auswahl fällt bei `setActiveStorey`** — geprüft ist der **Widget-Vertrag**; der benutzer-seitige Auslöser ist heute der Öffnen-/Anlegen-Pfad (§2.2 b) | `CanvasWidget` | Behandlung entfernt ⇒ rot. **Bauvorschrift, ohne die die Gegenprobe mehrdeutig ist:** `setActiveStorey` verwirft **unbedingt**, nicht über die Sichtbarkeits-Prüfung — sonst räumte das Netz (c) die Auswahl ohnehin ab (die Wand liegt in genau **einem** Geschoss), und die Zeile bliebe grün, ohne zu belegen, was sie behauptet. **Anders als bei §4-6 ist das per Fixture NICHT heilbar** |
| 8 | **Die Auswahl ist auf der Fläche erkennbar** | **Tinten-Sonde am Canvas** (offscreen). **Zwei Vorbedingungen, beide zu PRÜFEN** (§2.3): (a) `snapPreview()` ist an **beiden** Messpunkten **leer** — sonst misst die Zeile den Fang-Marker (gemessen +56 px); (b) die Abbildung ist unverändert (Zoom **und** Modell-Ecke) | Hervorhebung entfernt ⇒ Tinte unverändert ⇒ rot. **Bauvorschrift, ohne die die Zeile nicht gilt:** die Hervorhebung fügt **Fläche** hinzu (breiterer Stift/Marker) — eine reine **Umfärbung** ist von dieser Sonde per Konstruktion unsichtbar und damit **keine** zulässige Umsetzung |
| 9 | **Die Auswahl ist display-frei lesbar und wird gemeldet** — `selection()` liefert genau die getroffene Id (`nullopt` sonst); ein injiziertes `SelectionChanged`-Callable meldet **jeden** Wechsel, **auch das Fallen** | `CanvasWidget` + Zähl-Callable | Meldung nur bei Treffer, nicht beim Fallen ⇒ rot. **Die Naht ist die Voraussetzung von [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md)** — ohne sie hätte der Eigenschaften-Bereich kein Subjekt |
| 9a | **Der Auswahl-Modus ist bedienbar und sichtbar** — die dritte Aktion ist auffindbar, sie ruft ihr injiziertes Callable, und die **Markierung springt auf sie um** (die Gruppe bleibt exklusiv) | `MainWindow` (Aktions-Surrogat, Bauform [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) §4-9) | Auslösung entfernt ⇒ rot; Exklusivität entfernt ⇒ rot. **Reichweite:** geprüft wird der **Fenster-Vertrag**, nicht die produktive Verdrahtung — die liegt im Composition-Root und ist per Konstruktion orakel-los. **Ohne diese Zeile hätte die einzige Fenster-Zusage dieses Slice keine Diskriminierung** |
| 10 | **Der Zeichen-Pfad bleibt unverändert** — Hilfslinie, Wand, Fang, Anzeige, Abbruch, Hinweise | Bestands-Orakel | (Regressions-Netz) |

**Elf Zeilen tragen einen eigenen Sensor** (1–9 inkl. 4a und 9a), eine ist **Netz** (10).

**Was ausdrücklich KEIN Orakel bekommt:** (c) aus §2.2 — „die gewählte Wand verschwindet aus der
Sicht" ist ohne Entfernen-Pfad **strukturell nicht erreichbar**. Die Prüfung wird gebaut (Netz für
den Tag, an dem ein Entfernen entsteht) und **so benannt**.

## 5. Definition of Done

- [ ] **`src/adapters/ui/view/`-Treffer-Prüfung** (neu, Bauform `snap.{h,cpp}`): rein, display-frei,
      Segment-Abstand, eigener px-Schwellwert **ohne Default**, Filter auf `WallAxis` **und** aktives
      Geschoss. Orakel §4-1..4.
- [ ] **`src/adapters/ui/view/canvas_widget.{h,cpp}`**: dritter Werkzeug-Modus (**Auswahl**), Treffer
      beim **Loslassen**, `selection()` als display-freie Naht, injiziertes
      `SelectionChanged`-Callable (port-frei, Muster [ADR-0019](../../adr/0019-drw-2d-canvas.md)
      Option A), **flächen-hinzufügende** Hervorhebung im Paint-Pfad (§2.3), **drei**
      Lebensdauer-Regeln (§2.2). Orakel §4-5..9.
- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: dritte Aktion in der Werkzeug-Gruppe
      (**Auswahl**), Objektname + drittes Callable in `ToolActions`; Default bleibt **Hilfslinie**,
      die Gruppe bleibt **exklusiv**. Orakel §4-9a.
- [ ] **`src/main.cpp`**: Verdrahtung der Auswahl-Aktion. Das `SelectionChanged`-Callable bleibt in
      diesem Slice **ohne Empfänger** außer dem Test — das ist die Naht, die
      [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) besetzt, und es steht hier, statt später
      als Lücke entdeckt zu werden (R4).
- [ ] **Tests**: neuer `tests/adapters/`-Test der Treffer-Prüfung (§4-1..4, **ohne** Qt-Fixture) ·
      `test_canvas_widget.cpp` (§4-5..9) · `test_main_window.cpp` (§4-9a).
- [ ] **Orakel §4-1 bis §4-9a (inkl. 4a und 9a) je mit roter Gegenprobe** im Closure-Text,
      **einzeln** gemessen — **elf Zeilen mit eigenem Sensor**; §4-10 als **Netz** benannt.
      **Sechs Zeilen tragen ausgeschriebene Vorbedingungen bzw. Bauvorschriften**: §4-3 (drei
      Fixture-Bedingungen), §4-4 (Iterationsreihenfolge), §4-5 (der Loslassen-Konjunkt braucht einen
      Zug mit **verschiedenen** Ausgängen), §4-6 (Wand-Id **im aktiven** Geschoss), §4-7
      (`setActiveStorey` verwirft unbedingt) und §4-8 (Fang-Marker-Freiheit **und**
      flächen-hinzufügende Hervorhebung) — ohne sie messen sie nichts.
- [ ] **`make a-check` grün** — **mit** ausgeschriebener Aussage, ob eine neue Kante entstanden ist.
      **Kein** Kern-/Persistenz-/Export-/Schema-Diff, am `git diff --stat` belegt.
- [ ] **Benutzerhandbuch** — **gesucht, nicht aufgezählt** (in slice-058 waren es zehn Stellen statt
      der geplanten vier): §1, §2.2/§2.3 (Werkzeug **Auswahl**), die 4.1-Tabelle, ein
      4.2-Unterabschnitt, ggf. die FAQ. **Der Satz „Auswahl gibt es nicht" stammt aus slice-058 und
      ist zu ERSETZEN, nicht zu ergänzen.** Plus Version + Änderungshistorie.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**;
      **`make acc-002-beleg` grün** — nötig, weil der Beleg das Demo-Modell rendert und dieser Slice
      den Composition-Root ändert. **Das erzeugte Bild wird NICHT committet** (es gehört zur
      Abnahme-Runde von [`slice-012`](../done-archive/slice-012-eckenschluss-wal006-teil.md); Lehre
      slice-058).

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/view/`-Treffer-Prüfung `.{h,cpp}` | neu | §4-1..4 (rein, display-frei) |
| `src/adapters/ui/view/canvas_widget.{h,cpp}` | ändern | Auswahl-Modus, Auswahl-Zustand, Hervorhebung, Lebensdauer, Melde-Naht |
| `src/adapters/ui/view/main_window.{h,cpp}` | ändern | dritte Werkzeug-Aktion |
| `src/main.cpp` | ändern | Verdrahtung der Aktion |
| `tests/adapters/`-Test der Treffer-Prüfung `.{cpp}` | neu | §4-1..4 |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-5..9 |
| `tests/adapters/test_main_window.cpp` | ändern | §4-9a (dritte Aktion + Markierung) |
| `tests/CMakeLists.txt` | ändern | **eine** neue Testdatei |
| `docs/user/benutzerhandbuch.md` | ändern | **gesucht** + Version |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt** (`spec/**`: die Prüf-DoD-Zeile „am Artefakt nachsehen, ob etwas fehlt" ist
**vollständig** nach [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) gewandert — für den Umfang
**dieser** Hälfte hat der zweite Plan-Lauf selbst nachgesehen und **Vollständigkeit festgestellt**;
die Spezifikation trägt Treffer-Prüfung und Lebensdauer, geliefert von
[`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md)): `src/hexagon/**`, `src/adapters/io/**`,
`src/adapters/persistence/**`, `spec/**`, `data-model.yaml`/`schema.sql`,
`docs/plan/adr/0021-wand-im-2d-canvas.md`.

## 7. Risiken

- **R1 — der symptomlose Fehler dieses Slice.** Der Geschoss-Skopus (§2.1) fällt nicht auf: die
  falsche Wand würde gewählt, ohne Wurf und ohne Ablehnung — und in
  [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) **geändert**. Sein Orakel (§4-3) ist nur so
  gut wie seine Fixture: **das aktive Geschoss muss das zweite sein**.
- **R2 — die Tinten-Sonde misst den falschen Zeichen-Vorgang** (§2.3). Zwei Vorbedingungen, beide zu
  prüfen; und die Hervorhebung ist an eine **Bauvorschrift** gebunden (Fläche hinzufügen). Wer sie
  als Umfärbung baut, bekommt eine rote Zeile bei korrekter Implementierung.
- **R3 — die Fixture-Fallen der Vorgänger, kumuliert.** Fit **synchron** erzwingen (slice-057),
  Beobachter **ausdrücklich** anmelden (slice-058 R4), Positionen nach **jeder** Mutation neu aus der
  Transformation rechnen (slice-058 R3a). Auswählen mutiert **nicht** — das erleichtert die Fixture,
  ist aber zu **prüfen**, nicht anzunehmen.
- **R4 — die Melde-Naht ohne Empfänger.** `SelectionChanged` wird in diesem Slice nur vom Test
  konsumiert. Eine Naht ohne Produktions-Konsumenten ist die Klasse, vor der
  [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E14 warnt („ein Surrogat ohne
  Produktions-Konsumenten bliebe grün, während im Produkt nichts zu sehen ist") — hier ist sie
  **gewollt und terminiert**: [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) besetzt sie, und
  §4-8 belegt unabhängig davon, dass im Produkt etwas zu sehen ist.

## 8. Trigger

- [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen, Folgepflicht
  „Auswahl-/Änderungs-Slice" — **erste Hälfte**; die Zeile im [ADR-Index](../../adr/README.md) wird
  erst mit [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) auf erfüllt gesetzt.

## 9. Closure-Trigger

- §4-1 bis §4-9 grün + **je einzeln** diskriminierend belegt; §4-10 als Netz grün; `make gates`,
  `make io-smoke` und `make acc-002-beleg` grün; kein Kern-/Schema-/Export-Diff belegt; Handbuch
  nachgezogen; Closure-Notiz. **Die Welle bleibt offen** — sie schließt nach
  [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md).

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** mittel — eine reine Treffer-Funktion, ein dritter Werkzeug-Modus, ein
  Auswahl-Zustand mit drei Lebensdauer-Regeln, eine Melde-Naht, zwölf Orakel-Zeilen (elf mit eigenem
  Sensor).
- **Risiko:** **hoch für den Strang** — Auswählen ist das Interaktions-Muster, das das Produkt
  **nirgends** hat, und der Geschoss-Skopus ist symptomlos.
- **Eigenständig abnehmbar:** der Benutzer wählt eine Wand und **sieht** sie hervorgehoben (§4-8).
  Das ist ein benutzer-sichtbares Ergebnis, kein Zwischenstand.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-07-29)

Report: [`2026-07-29-slice-059-plan.md`](../../../reviews/2026-07-29-slice-059-plan.md) —
**1 HIGH / 6 MEDIUM / 5 LOW / 4 INFO + 21 Negativbefunde, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor. **Der Lauf hat den Slice geteilt**; diese Datei ist die erste Hälfte, die
zweite ist [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md).

| # | Behandlung (soweit diese Hälfte betroffen) |
|---|---|
| **HIGH-1** (die Tinten-Sonde für die Hervorhebung ist **nicht** diskriminierend: gemessen hebt allein der Fang-Marker die Tinte 633 → 689, **ohne** dass eine Hervorhebung existiert — der strukturelle Schutz aus slice-058 §4-8a fehlt hier genau deshalb, weil Auswählen **keine** Mutation ist, also aus derselben Tatsache, mit der der Plan seine Vorbedingung für erfüllt erklärte; **und** eine reine Umfärbung wäre für die Sonde unsichtbar ⇒ rot bei korrekter Implementierung) | **Zeile 8 trägt jetzt ZWEI geprüfte Vorbedingungen** (`snapPreview()` leer an **beiden** Messpunkten; unveränderte Abbildung) **und eine Bauvorschrift**: die Hervorhebung muss **Fläche hinzufügen** — eine Umfärbung ist keine zulässige Umsetzung. **Aus dem richtigen Faktum die halbe Folgerung zu ziehen war der Fehler**, und er steht in §2.3 ausgeschrieben. |
| **MEDIUM-3** (die Gegenproben von §4-3/§4-4 hängen an einer **Iterationsreihenfolge**, die die Zeilen nicht nennen: ist das aktive Geschoss das **erste**, liefert „Filter entfernt" dieselbe Id) | **Beide Vorbedingungen stehen in der Tabellenzeile selbst** — die Lehre aus slice-058 §11c, wörtlich angewandt. |
| **MEDIUM-5** (§2.2 (b) beruft sich auf einen „Geschoss-Wechsel", den die Oberfläche **nicht** hat: `setActiveStorey` hat genau **eine** produktive Aufrufstelle) | §2.2 (b) und §4-7 sagen jetzt, was geprüft wird (**Widget-Vertrag**) und was der benutzer-seitige Auslöser heute ist (der Öffnen-Pfad) — dieselbe Erreichbarkeits-Ehrlichkeit wie bei (c). |
| **MEDIUM-6** (der Schnitt ist zu groß; die Nicht-Teilungs-Begründung **widerspricht der eigenen Zeile §4-13**) | **Geteilt.** Die Begründung „man merkt es nur im Test" war durch die eigene Orakel-Zeile widerlegt: die Auswahl ist **auf der Fläche erkennbar**, also ist diese Hälfte abnehmbar. |
| **LOW-5 (b)** (Press oder Release entscheidet den Treffer? Der Plan sagte „**dem** Klick") | **Entschieden: Loslassen** (§2.1) — dieselbe Stelle wie bei den zwei Bestands-Gesten. |
| **LOW-5 (c)** ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) fehlt im Front-Matter, obwohl die [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Zeile darauf zielt) | In dieser Hälfte gegenstandslos ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) hängt an [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md)); dort behandelt. |

**Positiv bestätigt (21 Negativbefunde), für diese Hälfte relevant:** §2.2 ist in **beiden** Teilen
**gemessen** richtig — `replaceBuilding` meldet genau eine `ModelReplaced`, auch ohne Geschosse und
bei „Neu"; und eine fremde, gültige Id mutiert **lautlos** (kein Wurf, `applied = 300`). Der
Reviewer nennt §4-6 „die schärfste Zeile des Plans, und sie ist gemessen richtig".

**Startbar nach Lauf 1:** nein.

## 11a. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (zweiter Lauf, 2026-07-29)

Report: [`2026-07-29-slice-059a-plan-2.md`](../../../reviews/2026-07-29-slice-059a-plan-2.md) —
**0 HIGH / 3 MEDIUM / 4 LOW / 3 INFO + 20 Negativbefunde, „STARTBAR"**. Unabhängiger Reviewer,
verschieden von Autor und Lauf 1.

**Die zwei Auftragsfragen sind gemessen beantwortet.** Zeile 8 **trägt** — in **beiden** Richtungen:
ein Klick auf die Achsenmitte hält `snapPreview()` leer und Tinte/Zoom/Ecke konstant (633 / 0,09 /
(20,285)) über Press, Release **und** einen ganzen Zug; die vom Plan ungenannte vierte Farbquelle
(die in-Arbeit-Linie) liefert **0 px**, die zwei Vorbedingungen sind also **hinreichend**. Und die
Bauvorschrift ist exakt richtig gefasst: probeweise **Umfärbung** ⇒ `633 → 633` (**+0**), probeweise
**breiterer Stift** ⇒ `633 → 1359` (**+726**). **Keine dritte unfalsifizierbare Fassung.**

| # | Behandlung |
|---|---|
| **MEDIUM-1** (§4-6: die Fixture-Vorbedingung ist **eine Bedingung zu kurz** — die gewählte Wand muss im **aktiven Geschoss des Widgets** liegen; gemessen: fremde Geschoss-Id ⇒ 0 Vorkommen, „Neu"-Gestalt ⇒ 0, in beiden Fällen räumt das **Netz** (c) die Auswahl ab und die Gegenprobe bliebe grün) | **Zweiter Konjunkt in der Zeile.** Bitter und lehrreich: **der naheliegende Fixture-Griff — „den Anlegen-Pfad nachbilden", den §2.2 selbst nennt — ist genau der nicht-diskriminierende.** |
| **MEDIUM-2** (die **dritte** ungenannte Abhängigkeit von §4-3 ist die **Tie-Break-Richtung**; und die von [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E3 **entschiedene**, in der Spezifikation stehende Regel hatte im ganzen Plan **keine** Zeile) | **Neue Orakel-Zeile §4-4a** für den Tie-Break selbst, und §4-3 nennt jetzt **drei** Fixture-Vorbedingungen. Eine entschiedene Regel ohne Sensor ist die Klasse, die dieser Strang wiederholt produziert. |
| **MEDIUM-3** (die dritte Werkzeug-Aktion hatte DoD- und §6-Zeile, aber **keine** Orakel-Zeile — die Vorgänger-Zeile slice-058 §4-9 ist beim Schnitt verlorengegangen) | **Neue Zeile §4-9a** mit **zwei** Gegenproben (Auslösung, Exklusivität) und der Reichweiten-Klausel. **Die Bauart „beim Teilen fällt eine Zeile heraus" ist neu** — sie entstand erst durch den Split. |
| **LOW-1** (die zwei deckungsgleichen Achsen brauchen **verschiedene** Wand-Ids — beim Bau von Hand entsteht Deckungsgleichheit durch Kopieren des Literals **samt `origin`-Id**) | Als Konjunkt (b) in §4-3. |
| **LOW-2** (der Konjunkt „entschieden beim **Loslassen**" hat keine eigene Gegenprobe: bei einem Klick ist Press == Release) | §4-5 verlangt jetzt einen Zug mit **verschiedenen** Ausgängen an beiden Enden. |
| **LOW-3** („Behandlung entfernt ⇒ rot" ist bei §4-7 mehrdeutig und **nicht per Fixture heilbar**) | **Bauvorschrift in der Zeile:** `setActiveStorey` verwirft **unbedingt**, nicht über die Sichtbarkeits-Prüfung. |
| **LOW-4** (§6 verweist auf eine DoD-Prüfzeile, die vollständig nach [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) gewandert ist) | §6 sagt es jetzt — und der Reviewer hat für den Umfang **dieser** Hälfte selbst nachgesehen: Lastenheft und Spezifikation decken ihn **vollständig**. |

**Positiv bestätigt (20 Negativbefunde):** die **Teilung ist sauber** — alle 17 Orakel-Zeilen und 16
DoD-Positionen des ungeteilten Stands haben eine Heimat, und diese Hälfte trägt **nichts Fremdes**;
die Melde-Naht ohne Produktions-Konsumenten ist begründet, terminiert **und** besensort; §2.2 ist in
beiden Teilen gemessen richtig.

**Startbar: JA** — 0 HIGH nach der vorab festgelegten Abbruchregel. Die drei MEDIUM und vier LOW sind
eingearbeitet; sie betreffen **Fixture-Vorbedingungen und eine fehlende Zeile**, keine Bauart.

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
