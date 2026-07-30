---
id: slice-058
titel: Wand zeichnen im 2D-Canvas ([LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md))
status: done
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 058: Wand zeichnen im 2D-Canvas

**Status:** done (2026-07-29). Vier
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe
mit vier verschiedenen Reviewern; der vierte meldet **0 HIGH**, damit greift die vorab festgelegte
Abbruchregel. Einarbeitungen in §11 (Lauf 1), §11a (2), §11b (3), §11c (4).

**Welle:** welle-6-interaktiv-planen — **erste Hälfte des Abschluss-Triggers** („eine Wand ist im
2D-Canvas **zeichenbar**"). **Unabhängig von
[`slice-057`](../done/slice-057-lese-naht-bauteil-identitaet.md)**: Zeichnen braucht keine
Bauteil-Identität.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-28.

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md), **Entscheidungen 1, 2, 5, 6, 7, 8, 9, 10, 12, 14**.
Der Canvas kann heute **eine** Geste: Links-Zug erzeugt eine **Hilfslinie**. Das Produkt existiert
für Bauteile.

## 1. Ziel

Im **Wand-Modus** erzeugt derselbe Links-Zug **eine Wand** statt einer Hilfslinie — mit Fang an
beiden Enden, sofort sichtbar in 2D und 3D, überlebt Speichern/Laden und Export. Jeder Fehl-Ausgang
gibt dem Benutzer einen **Hinweis**, und **kein Wurf verlässt den Ereignis-Pfad**.

## 2. Die drei Entwurfs-Fragen dieses Slice

### 2.1 Der Modus lebt im Canvas, die Bedienung im Fenster

Der **Zustand** gehört zum Canvas (er entscheidet, was ein Zug erzeugt) und ist als Widget-Eigenschaft
lesbar — die Nachweis-Naht aus [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E14. Die **Bedienung**
(eine Aktions-Gruppe mit sichtbarer Markierung) gehört ins Fenster, das sie wie die Datei-Aktionen als
**injizierte Callables** bekommt — port-frei, Muster [ADR-0019](../../adr/0019-drw-2d-canvas.md)
Option A. **Das Fenster kennt den Canvas nicht**; der Composition-Root verdrahtet.

### 2.2 Die zweite Senke ist die Fehler-Barriere — und sie ist gegenläufig zur ersten

Der Zeichen-Weg lehnt **wertbasiert** ab. Der Bauteil-Weg **wirft**: bei unbekanntem Geschoss, und —
transaktional **vor** dem Commit — bei fehlschlagender Geometrie. Die neue `ui/command/`-Senke fängt
beides ([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E10). **Wer die Bauform der ersten Senke
kopiert, baut den Fehler ein.**

### 2.3 Wie der Ausgang zum Hinweis kommt — **entschieden, gemessen** (R1)

Die Fehl-Ausgänge brauchen Hinweise (E5), aber **die Texte liegen im Composition-Root** (benannte
Grenze des Fenster-Adapters). Der Ausgang muss also als **Wert** von der Senke bis zur Anzeige. Wo
der Typ dieses Werts wohnt, ist die eine echte Entwurfs-Frage:

| Variante | Konsequenz |
|---|---|
| **A** — Ausgangs-Typ in `ui/view/`, die Senke in `ui/command/` erzeugt ihn | verlangt einen `command/ → view/`-Include. Die Schicht-Kante ist erlaubt, **aber** die laterale Adapter-Regel hat einen **skalaren** Ausnahme-Eintrag; ob dieser Include sie verletzt, ist am Gate zu **messen**, nicht zu vermuten |
| **B** — **kein** gemeinsamer Typ zwischen Canvas und Senke; die **Senke** stellt den Ausgang fest und meldet ihn über ein **eigenes** injiziertes Callable | braucht **keinen** neuen Include und **keine** Gate-Frage |

**Entschieden: Variante B — gemessen, nicht vermutet.** Drei `make a-check`-Läufe: Baseline
**0 Befunde** → probeweiser `command/ → view/`-Include ⇒ **`lateral-adapter`, 1 Befund, Exit ≠ 0** →
Gegenrichtung ⇒ ebenfalls 1 Befund → Rücknahme, Baseline wieder 0. Die Schicht-Kante
`ui_command → ui_view` ist deklariert und wird nicht bemängelt — **die laterale Adapter-Regel schlägt
unabhängig davon zu**, und ihr Ausnahme-Eintrag ist skalar. Variante A ginge nur über eine **Weitung**
dieses Eintrags, also die Gate-Lockerung, die
[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen ausschließt.

**Die Entartung stellt die Senke fest, nicht der Canvas.** Der Kern verwirft unterhalb der
Geometrie-Toleranz (0,1 mm); bei Maximal-Zoom (100 px/mm) liegen **benachbarte Pixel 0,01 mm**
auseinander. Ein Canvas, der „an seinen eigenen zwei Punkten" urteilt, hielte den Zug für gültig,
während der Kern ihn verwirft — **ein falscher Hinweis**.

**Die Senke meldet den Hinweis selbst.** Sie kennt den Ausgang (Rückgabewert **und** gefangene Würfe)
und bekommt vom Composition-Root ein **eigenes** Hinweis-Callable. Der Hinweis-Typ lebt damit in
`ui/command/`, **der Canvas sieht ihn nie**, und es entsteht **kein** verbotener Include. Das Callable
des Canvas bleibt, was es ist: „zeichne eine Wand von A nach B".

**Drei Hinweise, nicht vier:**

| Ausgang | Hinweis |
|---|---|
| angelegt | keiner (die Wand ist der Beleg) |
| **kein Wert zurück** (Null-Länge oder nicht-endlich) | „keine Wand angelegt" |
| **Wurf** — unbekannter Bezug **oder** Geometrie-Fehlschlag | „nicht angelegt, Modell unverändert" |

**Die zwei Wurf-Fälle werden bewusst NICHT getrennt.** Sie wären nur über den **Ausnahme-Typ**
unterscheidbar, und den dokumentiert der Port für diesen Aufruf **gar nicht** — der Schwester-Aufruf
für Öffnungen löst dieselben Fälle sogar wertbasiert. **Auf undokumentiertes Wurf-Verhalten eine
Benutzer-sichtbare Unterscheidung zu bauen, hieße einen Vertrag zu erfinden.** Für den Benutzer ist
die Aussage ohnehin dieselbe: nichts entstanden, Modell unverändert.

## 3. Bewusst NICHT Teil

- **Selektion, Eigenschaften-Bereich, Parameter ändern** — das ist
  [`slice-059a`](../in-progress/slice-059a-wand-auswaehlen.md) (Auswahl) bzw.
  [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) (Parameter).
- **Mehrpunktiger Wandzug.** [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E2 entscheidet **ein
  Segment je Geste**; die Teilumfang-Klausel steht bereits im Lastenheft.
- **Entfernen und Rückgängigmachen.** E12: der **Gesten-Abbruch** ist dabei, das **Löschen** nicht —
  benannte Grenze, bereits im Lastenheft.
- **Ein Zeichenbereich.** E8: die Negative ist als **nicht erreichbar** benannt; hier ist **nichts**
  zu bauen, und das ist der Punkt.
- **Raster, Winkel, weitere Fang-Arten.**
- **Jede Änderung an Kern, Persistenz, Export, Schema.** Eine gezeichnete Wand ist eine gewöhnliche.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Default ist Hilfslinie** — ohne Modus-Wechsel erzeugt ein Zug wie bisher eine Hilfslinie und **keine** Wand | `CanvasWidget`, headless | Default auf Wand ⇒ rot (auch Bestands-Orakel fallen) |
| 2 | **Im Wand-Modus erzeugt derselbe Zug eine Wand und keine Hilfslinie** — die Zusammenspiel-Zeile | `CanvasWidget` + Modell-Surrogat | Modus ignoriert ⇒ rot |
| 3 | **Der Fang gilt an BEIDEN Enden** — je ein Zug, dessen **Anfang** bzw. **Ende** in Fang-Nähe liegt, erzeugt eine Wand mit **exakt** dessen mm (§1 sagt beide zu) | `CanvasWidget`, headless | Fang im Press- bzw. Release-Pfad einzeln übersprungen ⇒ rot |
| 4 | **Zwei Züge teilen einen gefangenen Punkt exakt** — die Voraussetzung des Eckenschlusses ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)) | `CanvasWidget` (die zwei Züge) gegen den **Modell-Zustand des Dienstes** — der Ort ist der Canvas, der Beleg das Modell | Fang im Wand-Pfad übersprungen ⇒ rot (die Punkte differieren) |
| 5 | **Entarteter Zug ⇒ keine Wand, Modell unverändert, Hinweis** — die **Senke** stellt den Ausgang fest, nicht der Canvas (§2.3) | Senken-Test + Hinweis-Surrogat | Hinweis nicht gemeldet ⇒ rot |
| 5a | **Die Hinweis-Anzeige existiert und trägt den gemeldeten Text** | `MainWindow`-Surrogat: das Anzeige-Widget ist auffindbar, sein **Text** ist der gemeldete | Anzeige-Aufruf entfernt ⇒ rot. **Bewusst ohne `isVisible()`-Konjunkt** — den entscheidet `show()`, nicht die Implementierung (auch ein Waisen-Widget ist danach sichtbar); der Text-Konjunkt trägt die Zeile allein |
| 6 | **Gesten-Abbruch ⇒ keine Wand, Modell unverändert** — **beide** Auslöser einzeln: Escape **und** Fokusverlust (E12) | `CanvasWidget`, headless | je Auslöser einzeln entfernt ⇒ rot |
| 7 | **Kein Wurf verlässt den Ereignis-Pfad** — bei veralteter Geschoss-Id gibt es einen **Hinweis**, keine Ausnahme | **`ui/command/`-Senke** (dort liegt die Barriere und dort liegt der Test) | `try`/`catch` entfernt ⇒ rot |
| 8 | **Der Refresh kommt aus der Meldekette** — nach dem Kommando rahmt der Canvas neu ein, **ohne** eigenes Zutun | `CanvasWidget::transform()` vor/nach dem Zug. **Vorbedingung (gemessen): der Zug muss die Bounding-Box VERGRÖSSERN** — das ist die **gegenteilige** Vorbedingung zu 8a, und beide Zeilen liegen in **derselben** Testdatei. Innerhalb der Box steht der Zoom mit **und** ohne Anmeldung still, die Zeile misst dann nichts. **Die Fixture muss den Beobachter selbst anmelden** (`subscribe`) — die Bestands-Fixture tut es nicht | Beobachter-Anmeldung entfernt ⇒ Transformation bleibt stehen ⇒ rot. **Die Gegenprobe ist zugleich der Beleg für E6** (§4, Erläuterung unter der Tabelle) |
| 8a | **Die Wand erscheint sofort im Grundriss** — der abnahmebindende Konjunkt aus [LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), 2D-Hälfte | **Tinten-Sonde am Canvas**, offscreen gerendert. **Vorbedingung (gemessen): der Zug muss INNERHALB der bestehenden Bounding-Box liegen** — dann steht die Abbildung still und der Zuwachs ist nur das neue Segment (gemessen `633 → 993` für die Diagonale `0,0 → 4000,3000`, `633 → 905` für `500,500 → 3500,2500`, je bei Zoom `0,09` und Ecke `(20,285)` unverändert). **Sollwert ist die Richtung, nicht die Zahl** — die hängt am Segment. Außerhalb färbte das Neu-Einrahmen mit | Zeichnen der Wand-Segmente unterdrückt ⇒ rot |
| 9 | **Der Modus ist bedienbar und sichtbar** — die Aktion ist auffindbar, sie ruft das injizierte Callable, und die aktive Aktion ist **markiert** | `MainWindow` (Aktions-Surrogat) | Auslösung entfernt ⇒ rot; Markierung entfernt ⇒ rot. **Reichweite:** geprüft wird der **Fenster-Vertrag**, nicht die produktive Verdrahtung — die liegt im Composition-Root und ist per Konstruktion orakel-los (in kein Testbinary gelinkt) |
| 10 | **Die 3D-Sicht folgt** — nach dem Zeichnen **über die Geste** trägt der Viewer-Surrogat die neue Wand ([LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)) | Bestands-Viewer-Surrogat, **von der Geste ausgelöst** | Der Bestands-Beleg genügt **nicht** — er löst die Mutation direkt am Dienst aus. Diskriminierend wird die Zeile erst, wenn der **Zug** sie auslöst |
| 11 | **Der Hilfslinien-Pfad bleibt unverändert** — freies Zeichnen, Fangen, Anzeige, Entartungs-Ablehnung | Bestands-Orakel | (Regressions-Netz) |
| 12 | **Die gezeichnete Wand überlebt Speichern/Laden und Export** — der letzte Konjunkt aus §1 | **Bestands-Netz** (`make io-smoke`, Persistenz-Runden-Orakel): eine über die Geste erzeugte Wand ist dieselbe wie eine über den Dienst erzeugte, und §4-2 belegt, dass die Geste den Dienst erreicht | (Netz — **kein** eigener Sensor, benannte Entscheidung) |

**Warum Zeile 8 den Zoom misst und nicht Pulls oder Tinte.** Das Neu-Einrahmen hat **genau eine**
Auslöse-Bedingung (`fitted_`), und außer `resizeEvent`/`setActiveStorey` führt nur `onModelChanged`
dorthin. Gemessen an der Bestands-Naht `transform()` mit **einer Wand, die die Bounding-Box
vergrößert** (`0,0 → 40000,30000`):

- **ohne** Beobachter-Anmeldung: `zoom 0,09 → 0,09` — unverändert,
- **mit** Beobachter-Anmeldung: `zoom 0,09 → 0,009`.

**Die Klammer ist tragend, nicht schmückend:** mit einer Wand **innerhalb** der Box misst die Zeile
`0,09 → 0,09` in **beiden** Fällen. Der Zahlenwert selbst hängt an der Größe der zugefügten Wand
(eine kleinere ergab `0,09 → 0,03`) — **die Diskriminierung hängt es nicht**.

Ein Pull-Zähler kann das **nicht** zeigen (Qt koalesziert beliebig viele `update()` zu einem Paint),
die Tinten-Sonde auch nicht (der Canvas pullt bei **jedem** Paint frisch, die Sonde erzwingt einen).

**Die Gegenprobe belegt mehr als die Zeile behauptet.** Gäbe es einen zusätzlichen Selbst-Refresh mit
Neu-Einrahmen, bliebe „Anmeldung entfernt ⇒ Transformation steht still" **grün**. Ein roter Lauf der
Gegenprobe ist damit zugleich der Beleg für [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E6.
**Sensor-los bleibt** ein Selbst-Refresh, der nur `update()` ruft, ohne neu einzurahmen — das ist
nicht der Fall, vor dem E6 warnt (dort geht es um doppeltes Neu-Einrahmen), und es steht hier, weil
die Zusage nur so weit deckt, wie sie messen kann.

## 5. Definition of Done

- [x] **R1 vor dem Start entschieden** (§2.3): Variante A ist **nicht** gate-frei
      (`lateral-adapter` schlägt in **beide** Richtungen an), also **B** — und die **Senke** stellt
      den Ausgang fest, nicht der Canvas.
- [ ] **`src/adapters/ui/view/canvas_widget.{h,cpp}`**: Werkzeug-Modus als Widget-Zustand + lesbare
      Eigenschaft; im Wand-Modus ruft der Zug die neue Senke; Gesten-Abbruch **auf beiden Auslösern**
      (Escape **und** Fokusverlust, E12); **kein** zusätzlicher Selbst-Refresh — E6, **mit Sensor**
      (`transform()`, §4-8). Orakel §4-1..4, 6, 8, 8a.
- [ ] **`src/adapters/ui/command/`-Wand-Senke** (neu): baut aus dem gefangenen Segment die Wand über
      den Bearbeitungs-Port, **fängt** dessen Würfe und meldet den Ausgang als Wert.
      Orakel **§4-5 und §4-7** — der Ausgang wird hier festgestellt, nicht im Canvas.
- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: Aktions-Gruppe für den Modus, injizierte
      Callables, aktive Aktion **markiert**. Orakel §4-9.
- [ ] **Die Hinweis-Anzeige selbst** — im Bestand gibt es **keine**; ohne sie wäre „jeder
      Fehl-Ausgang gibt einen Hinweis" eine Zusage ohne Adressat. Orakel §4-5a.
      **Keine Tinten-Sonde am Fenster** (gemessen: dort zählt sie **120 000 von 120 000** Pixeln als
      Tinte, weil der Hintergrund nicht weiß ist) — sie trägt nur am Canvas, dessen Zeichen-Pfad weiß
      füllt. Die ADR-Folgepflicht „Surrogat **und** Tinten-Sonde" wird deshalb in **§4-8a am Canvas**
      eingelöst; am Fenster tritt die Qt-Eigenschaft an ihre Stelle.
- [ ] **`src/main.cpp`**: Verdrahtung (Modus-Callables → Canvas, Senke → Port, Hinweis-Texte).
      **Die Texte bleiben hier** — benannte Grenze des Fenster-Adapters.
- [ ] **Tests**: `test_canvas_widget.cpp` (§4-1..4, 6, 8, 8a — die Sonde für 8a **innerhalb** der
      bestehenden Bounding-Box) · **Test der neuen Senke (§4-5 UND §4-7)**, denn dort wird der
      Ausgang festgestellt, nicht im Canvas · `test_main_window.cpp` (§4-5a, §4-9) · **ein Test, der
      Canvas und Viewer-Surrogat an denselben Dienst hängt** (§4-4, §4-10).
- [ ] **Orakel §4-1 bis §4-10 (inkl. 5a, 8, 8a) je mit roter Gegenprobe** im Closure-Text,
      **einzeln** gemessen — **zwölf Zeilen tragen einen eigenen Sensor**; §4-11 und §4-12 sind
      **Netz** und werden als solches benannt, nicht als Beleg gebucht.
- [ ] **`make a-check` grün** — **mit** ausgeschriebener Aussage, ob eine neue Kante entstanden ist
      (R1). **Kein** Kern-/Persistenz-/Export-/Schema-Diff, am `git diff --stat` belegt.
- [ ] **Benutzerhandbuch — VIER Stellen:** §4.2 (Wand-Modus) · §1 „Heute möglich" **und** der Satz
      „ein Gebäude selbst planen … noch NICHT möglich" (**präzisieren**: Wände ja, andere Bauteile
      nein) · die 4.1-Aufgaben-Tabelle · **§3** („vier Wege …", zwei Stellen) und die **FAQ**-Antwort
      „Kann ich in der Oberfläche Wände zeichnen? — Noch nicht." Plus Version + Änderungshistorie.
      **Hier wird gesucht, nicht aufgezählt** — dieselbe Fehlerklasse stand seit der Fang-Lieferung
      unbemerkt im Lastenheft.
- [ ] **[ADR-Index](../../adr/README.md)**: die „Zeichnen-Slice"-Folgepflichtzeile auf **erfüllt**.
- [ ] **Lastenheft/Spezifikation: am Artefakt prüfen, ob etwas fehlt** — beides hat
      [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md) geliefert, aber **nicht pauschal
      verneinen**: im 057-Review war genau diese Pauschal-Verneinung falsch.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**;
      **`make acc-002-beleg` grün** — nötig, weil der Beleg das **Demo-Modell** rendert, dessen
      Aufbau im Composition-Root liegt, und dieser Slice dort die Verdrahtung ändert.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/view/canvas_widget.{h,cpp}` | ändern | Modus, Wand-Zug, Abbruch |
| `src/adapters/ui/command/`-Wand-Senke `.{h}` | neu | Schreibpfad + Fehler-Barriere |
| `src/adapters/ui/view/main_window.{h,cpp}` | ändern | Aktions-Gruppe **und die Hinweis-Anzeige** (§4-5a) |
| `src/main.cpp` | ändern | Verdrahtung + Hinweis-Texte |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-1..4, 6, **8, 8a** |
| `tests/adapters/test_main_window.cpp` | ändern | §4-5a, §4-9 |
| `tests/adapters/`-Test der Wand-Senke `.{cpp}` | neu | §4-5 und §4-7 (der Ausgang wird **dort** festgestellt) |
| `tests/adapters/`-Test, der Canvas **und** Viewer-Surrogat an denselben Dienst hängt | neu/ändern | §4-4 und §4-10 |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |
| `docs/user/benutzerhandbuch.md` | ändern | **vier Stellen** (§4.2 · §1 inkl. „noch NICHT möglich" · 4.1-Tabelle · §3 und die FAQ) + Version |
| `docs/plan/adr/README.md` | ändern | Folgepflichtzeile |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt** (`spec/**` steht **unter Vorbehalt** der DoD-Zeile „am Artefakt prüfen, ob etwas
fehlt" — geprüft und vollständig befunden; ändert sich das im Vollzug, gewinnt die DoD-Zeile): `src/hexagon/**`, `src/adapters/io/**`, `src/adapters/persistence/**`, `spec/**`,
`data-model.yaml`/`schema.sql`, `docs/plan/adr/0021-wand-im-2d-canvas.md`.

## 7. Risiken

- **R1 — der Ausgangs-Typ und die laterale Adapter-Regel: ENTSCHIEDEN** (§2.3, am Gate gemessen —
  Variante B). Der Eintrag bleibt stehen, weil das Restrisiko im Vollzug liegt: wer beim Bauen doch
  einen gemeinsamen Typ einführt, fällt in dieselbe Falle. Präzedenz für die Bauart der Entscheidung:
  der Canvas-Slice hat drei Optionen verglichen und die **gate-freie** gewählt, statt die Regel zu
  weiten.
- **R2a — E12 nennt ZWEI Abbruch-Auslöser:** Escape **und** Fokusverlust. Beide sind umzusetzen,
  §4-6 prüft **beide** Wege einzeln.
- **R2 — der Refresh ist gegenläufig zum Vorgänger.** Wer den Hilfslinien-Zug kopiert, baut einen
  zweiten Refresh ein. **Kontrolle: die Gegenprobe zu Orakel-Zeile 8** (§4) — bleibt sie grün,
  obwohl die Beobachter-Anmeldung entfernt wurde, gibt es einen Selbst-Refresh.
- **R3a — eine gezeichnete Wand KANN die Abbildung verschieben** (nur eine, die die Bounding-Box
  ändert). Anders als eine Hilfslinie meldet sie einen `op`; der Canvas rahmt **neu ein**, und
  Bildschirm↔mm verschiebt sich. **Jede Fixture mit aufeinanderfolgenden Zügen muss ihre Positionen
  nach JEDEM Zug neu aus der Transformation rechnen** — hartcodierte Pixel aus dem
  Hilfslinien-Muster wären ab dem zweiten Zug falsch. Neue Ursache derselben Fixture-Klasse wie R4.
- **R3 — das Demo-Modell hat zwei Geschosse mit deckungsgleichen Wänden.** Für **dieses** Slice
  harmlos (gezeichnet wird ins **aktive** Geschoss), aber die Fixture darf daraus keine
  Scheinsicherheit ziehen: eine neue Wand ist an ihrer **Geschoss-Id** zu prüfen, nicht nur an der
  Anzahl.
- **R4 — die Fixture-Falle aus slice-055/057.** Die Canvas-Fixture erzwingt den Fit-to-Bounds seit
  slice-057 **synchron**; jede neue Fixture muss dasselbe tun. **Und sie meldet heute KEINEN
  Beobachter an** — für §4-8 ist `subscribe` im Setup zuzufügen, sonst steht die Transformation in
  jedem Lauf still und die Zeile ist grün wie ihre Gegenprobe. **Ein Test, dessen Konstanten von
  asynchron hergestelltem Zustand abhängen, ist manchmal richtig** — und ein Orakel, das nur manchmal
  misst, täuscht Grün vor.
- **R5 — das Handbuch wird teil-unwahr.** „Ein Gebäude selbst planen ist noch NICHT möglich" stimmt
  nach diesem Slice für **Wände** nicht mehr. Präzisieren, nicht streichen.

**[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
gehört NICHT in diesen Slice** (vier Läufe haben es angemerkt, deshalb steht es hier): das
geometrielastige Code-Review hängt an der **Welle-Closure** und deckt den ganzen Bauteil-Strang ab —
verortet in [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md). Dieser Slice führt es nicht,
und das ist eine Entscheidung, kein Vergessen.

## 8. Trigger

- [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen, Folgepflicht „Zeichnen-Slice", samt
  der Zeile im [ADR-Index](../../adr/README.md).

## 9. Closure-Trigger

- §4-1 bis §4-10 (inkl. 5a, 8, 8a) grün + **je einzeln** diskriminierend belegt; §4-11/§4-12 als Netz
  grün; `make gates`, `make io-smoke` und
  `make acc-002-beleg` grün; kein Kern-/Schema-/Export-Diff belegt; Handbuch und ADR-Index
  nachgezogen; Closure-Notiz. **Der Wellen-Trigger ist damit zur Hälfte erfüllt** — die zweite Hälfte
  sind [`slice-059a`](../in-progress/slice-059a-wand-auswaehlen.md) (Auswahl) und [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md)
  (Parameter — dort hängt der Trigger).

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** mittel-groß — ein Modus-Zustand, eine zweite Senke mit Barriere, eine
  Aktions-Gruppe, eine Hinweis-Anzeige, **vierzehn** Orakel-Zeilen (1–12 inkl. 5a und 8a; davon 11
  und 12 Netz, **zwölf** mit eigenem Sensor), vier berührte Produktions-Dateien plus
  Composition-Root.
- **Phase-Reife:** die Geste, der Fang, die Anzeige und die Fixture-Bauform liegen; die Entscheidungen
  liegen seit [ADR-0021](../../adr/0021-wand-im-2d-canvas.md).
- **Risiko:** mittel — **nicht im Zeichnen** (die Geste existiert), sondern im **Fehler-Ausgang** und
  im **Refresh-Pfad**, die beide gegenläufig zum Vorgänger sind.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-07-28)

Report: [`2026-07-28-slice-058-plan.md`](../../../reviews/2026-07-28-slice-058-plan.md) —
**2 HIGH / 4 MEDIUM / 4 LOW / 4 INFO + 26 Negativbefunde, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor.

**Der Lauf hat die R1-Frage nicht beantwortet, sondern GEMESSEN** — drei `make a-check`-Läufe mit
probeweisen Includes in beide Richtungen, jeweils zurückgenommen. Das ist die Bauart Beleg, die
dieser Plan verlangt hatte, und sie hat die Empfehlung bestätigt **und** korrigiert.

| # | Behandlung |
|---|---|
| **HIGH-1** (Orakel-Zeile 8 kann den Fehler, den sie fangen soll, an der `PlanPull`-Naht **nicht messen**: der Fehlerfall ist ein zusätzliches `update()`, und **Qt koalesziert beliebig viele davon zu EINEM Paint** — 4 Pulls mit **und** ohne Selbst-Refresh; der Bestands-Test sagt das wörtlich in seinem eigenen Kommentar) | **Zeile gestrichen, nicht umformuliert.** An ihre Stelle tritt die **messbare Wirkung**: nach dem Kommando zeigt der Canvas die Wand, getragen allein von der Meldekette. E6 wird umgesetzt und im Code begründet, hat aber **keinen eigenen Sensor** — und das steht jetzt da. **Zusätzlich benannt:** die von E6 genannte Folge („zweimal neu einrahmen") ist **bedingt** — ein bloßes `update()` löst kein Neu-Einrahmen aus. **Der Fehler ist meiner: die Koaleszenz-Warnung steht in einem Test, den ich selbst geschrieben habe.** |
| **HIGH-2** (der Plan sagt „jeder Fehl-Ausgang gibt einen Hinweis" zu, erzeugt aber **keine Anzeige** dafür — im Bestand existiert keine —, führt **keine** Tinten-Sonden-Zeile, vertagt sie auch nicht in §3, und will trotzdem die Folgepflicht-Zeile buchen, die die Hinweis-Zeile **wörtlich** nennt) | **Die Anzeige ist in den Slice aufgenommen** — DoD-Zeile + **neue Orakel-Zeile 5a** mit **zwei** Nachweis-Ebenen (Surrogat **und** Tinten-Sonde), wie die Fang-Anzeige sie etabliert hat. **Eine Zusage ohne Adressat ist keine.** |
| **MEDIUM-1** (§4-9 prüft nur die Verdrahtung, die der Test selbst herstellt; die produktive liegt im Composition-Root und ist orakel-los) | **Reichweite in der Zeile benannt:** geprüft wird der **Fenster-Vertrag**. Das ist die bekannte Klasse (der Composition-Root ist per Konstruktion sensor-los), keine neue Lücke — aber sie wird nicht verschwiegen. |
| **MEDIUM-2** (Variante B ließ den **Canvas** die Entartung an seinen eigenen zwei Punkten erkennen — der Kern verwirft unterhalb **0,1 mm**, und bei Maximal-Zoom liegen benachbarte Pixel **0,01 mm** auseinander ⇒ **falscher Hinweis**) | **Am Artefakt nachgerechnet und bestätigt.** Die Bauform ist korrigiert: **die Senke** stellt den Ausgang fest (Rückgabewert **und** gefangene Würfe) und meldet den Hinweis über ein **eigenes** injiziertes Callable; der Hinweis-Typ lebt in `ui/command/`, **der Canvas sieht ihn nie**. Damit ist B gate-frei **und** korrekt. |
| **MEDIUM-3** (jede gezeichnete Wand meldet einen `op` ⇒ Neu-Einrahmen ⇒ die Bildschirm↔mm-Abbildung **verschiebt sich**; die Zeilen 2/3/4 brauchen aufeinanderfolgende Züge) | **Als R3a aufgenommen** — eine **neue Ursache** derselben Fixture-Klasse wie R4, nicht dieselbe: Positionen sind nach **jedem** Zug neu aus der Transformation zu rechnen. Hartcodierte Pixel aus dem Hilfslinien-Muster wären ab dem zweiten Zug falsch. |
| **MEDIUM-4** (§4-10 ist im Bestand bereits belegt und wird erst diskriminierend, wenn die Wand durch die **Geste** entsteht) | Zeile umformuliert: der Bestands-Beleg genügt **nicht**; die Mutation muss vom **Zug** ausgelöst werden. |

**Positiv bestätigt** (nicht neu prüfen): Schnitt, Abgrenzung (§3) und Quellen-Konsistenz tragen ·
die vom Plan **verlangte** Nicht-Pauschal-Prüfung von Lastenheft und Spezifikation hat der Reviewer
selbst durchgeführt — **beide sind für den Umfang dieses Slice vollständig** (die 056-Lieferung ist
verifiziert) · „unabhängig von der Lese-Naht" trägt · die deklarierte Schicht-Kante
`ui_command → ui_view` existiert (ist aber für alles außer dem skalaren Ausnahme-Eintrag wirkungslos).

**Startbar nach Lauf 1:** nein. Auftrag an Lauf 2: **trägt die Tinten-Sonde am Fenster, und ist die
neue Zeile 8 diskriminierend — oder die zweite unfalsifizierbare Zusage an derselben Stelle?**

## 11a. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (zweiter Lauf, 2026-07-28)

Report: [`2026-07-28-slice-058-plan-2.md`](../../../reviews/2026-07-28-slice-058-plan-2.md) —
**2 HIGH / 4 MEDIUM / 4 LOW / 4 INFO + 23 Negativbefunde, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor ≠ Reviewer des ersten Laufs.

**Beide Fragen sind beantwortet — durch Messung, und beide Male gegen den Plan.**

| # | Behandlung |
|---|---|
| **HIGH-1** (die **neue** Zeile 8 ist **ebenfalls** nicht diskriminierend: gemessen steigt die Tinte **ohne** Beobachter-Anmeldung 633 → 905 bei unveränderter Abbildung — die Gegenprobe bliebe **grün**, weil der Canvas bei **jedem** Paint frisch pullt und die Sonde einen Paint erzwingt; umgekehrt ist „bei gleicher Abbildung" genau dann verletzt, wenn die Meldekette wirkt: 20/285 → 140/210) | **Zeile 8 ist ersatzlos gestrichen.** [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E6 hat **keinen** Sensor, und der Plan behauptet keinen mehr — die Einhaltung ist **durch Lesen** prüfbar, wie die ausdrücklich als „computational nicht prüfbar" geführten Prozess-Regeln. **Kein dritter Versuch.** [**Von Lauf 3 gekippt und selbst nachgemessen — es gibt einen Sensor, siehe §11b und §4. Diese Zelle bleibt als Protokoll stehen, sie ist nicht mehr der Stand.**] |
| **HIGH-2** (die Tinten-Sonde **trägt am Fenster nicht**: gemessen zählt sie in **allen drei** Zuständen **120 000 von 120 000** Pixeln als Tinte, weil der Fenster-Hintergrund nicht weiß ist — sie funktioniert am Canvas nur, weil **dessen** Zeichen-Pfad weiß füllt) | Zeile 5a stützt sich jetzt auf **Qt-Eigenschaften**: das Anzeige-Widget ist **sichtbar** und trägt den gemeldeten **Text**. **Und die zweite Ebene wird hier auch nicht gebraucht:** die Fang-Anzeige brauchte sie, weil **wir** malen — ein Standard-Widget stellt Qt selbst dar. Die Abweichung von der pauschalen „Tinten-Sonde"-Formulierung der Folgepflicht-Zeile gehört in die Closure. |
| **MEDIUM-1** (die DoD buchte §4-5 weiter auf den Canvas, obwohl §2.3 den Ausgang der **Senke** gibt — die Kern-Korrektur aus Lauf 1 war an ihrer **Vollzugs-Stelle** wieder aufgehoben) | DoD nachgezogen: §4-5 **und** §4-7 liegen im Senken-Test. **Dritte Wiederholung derselben Bauart in diesem Projekt: Prosa korrigiert, Vollzugs-Zeile stehen gelassen.** |
| **MEDIUM-2** (welche Ausgänge einen Hinweis teilen, sagte der Plan nicht; die zwei Wurf-Fälle wären nur über **undokumentierte** Ausnahme-Typen trennbar, und die alte Tabellenzeile widersprach dem Korrektur-Absatz) | **Ausgeschrieben: drei Hinweise, nicht vier**, mit Tabelle — und die Begründung, warum die zwei Wurf-Fälle **bewusst** zusammenfallen: auf undokumentiertes Wurf-Verhalten eine benutzer-sichtbare Unterscheidung zu bauen hieße, einen Vertrag zu erfinden. Die widersprüchliche Tabellenzeile ist ersetzt. |
| **MEDIUM-3** (die verschärfte Zeile 10 hatte **keinen Ort** — weder DoD-Testzeile noch Datei-Tabelle nannten einen Platz, an dem Canvas und Viewer-Surrogat an denselben Dienst kommen) | Eigene DoD-Testzeile für §4-4 **und** §4-10. |
| **MEDIUM-4** (der Handbuch-Nachzug war unvollständig: §3 mit **zwei** Stellen und die FAQ-Antwort „Kann ich in der Oberfläche Wände zeichnen? — Noch nicht." werden falsch) | DoD nennt jetzt **vier** Stellen statt zwei — und den Auftrag, zu **suchen** statt aufzuzählen. **Dieselbe Fehlerklasse stand seit der Fang-Lieferung unbemerkt im Lastenheft.** |
| **Lauf-1-LOW-1..4** (in §11 unbehandelt und unbenannt geblieben — vom zweiten Lauf gerügt) | **Nachgeholt:** R2a (E12 nennt **zwei** Abbruch-Auslöser) · die `acc-002-beleg`-Begründung ist präzisiert (nicht „3D-Kette berührt", sondern: der Beleg rendert das Demo-Modell, dessen Aufbau dieser Slice verdrahtet) · `spec/**` in §6 steht unter dem Vorbehalt der DoD-Prüfzeile · §4-4 nennt jetzt die **Komponente** statt „Modell-Surrogat". |

**Positiv bestätigt:** der **Schnitt** trägt · **Variante B trägt am Gate** (Baseline `make a-check`
= 0) · vier der sechs Lauf-1-Behandlungen sind substanziell.

**Startbar:** **nein.** Zwei HIGH verlangen einen **dritten** Lauf. Sein Schwerpunkt ist eng: **ist
Zeile 5a in ihrer neuen, pixel-freien Form diskriminierend** — und **hält die Streichung von Zeile 8
stand**, oder fehlt dem Slice damit eine Zusage, die er trotzdem macht? Zusätzlich: **die Bauart
„Prosa korrigiert, Vollzugs-Zeile stehen gelassen" ist jetzt dreimal aufgetreten** — der Lauf soll
gezielt danach suchen.

## 11b. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (dritter Lauf, 2026-07-28)

Report: [`2026-07-28-slice-058-plan-3.md`](../../../reviews/2026-07-28-slice-058-plan-3.md) —
**2 HIGH / 4 MEDIUM / 4 LOW / 4 INFO + 24 Negativbefunde, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor ≠ Reviewer der Läufe 1 und 2.

**Der Lauf hat den gezielten Auftrag beantwortet — und beide Antworten fielen gegen den Plan.** Der
gesuchte Muster-Befund („Prosa korrigiert, Vollzugs-Zeile stehen gelassen") wurde **gefunden**, und
zwar an der Stelle, die der Plan selbst zuletzt angefasst hatte.

| # | Behandlung |
|---|---|
| **HIGH-1** (der Risiko-Block R2 führte die in §4 **ersatzlos gestrichene** Zeile 8 weiter namentlich als „Pflicht, nicht Kür" — die einzige Stelle, die sagt, wie R2 kontrolliert wird, nannte eine Kontrolle, die es nicht mehr gab) | **R2 nennt jetzt die Kontrolle, die es gibt** — die Gegenprobe zu der wiederhergestellten Zeile 8. **Vierte bis siebte Wiederholung derselben Bauart**, diesmal spiegelverkehrt: nicht die Prosa lief der Vollzugs-Zeile voraus, sondern die Streichung. Wer eine Zeile entfernt, muss ihre **Rückverweise** entfernen. |
| **HIGH-2** (die Streichungs-Begründung „E6 hat KEINEN Sensor, prüfbar nur durch Lesen" ist **gemessen falsch** für die schädliche Hälfte von E6: derselbe Modell-Wechsel ergibt `transform().zoom` mit/ohne Beobachter-Anmeldung **verschiedene** Werte, ablesbar an der Naht, die die Bestands-Fixture ohnehin nutzt) | **Selbst nachgemessen** (Sonde in der Bestands-Fixture, danach zurückgenommen): **ohne** Anmeldung `0,09 → 0,09`, **mit** Anmeldung `0,09 → 0,009`. **Zeile 8 ist in dritter, diesmal messbarer Form wieder da** (§4-8); ihre Gegenprobe ist zugleich der **Beleg für E6**. Was weiterhin sensor-los bleibt — ein Selbst-Refresh **ohne** Neu-Einrahmen —, steht als benannte Grenze da. |
| **MEDIUM-1** (der `isVisible()`-Konjunkt in 5a ist **leer**: er wird von `show()` und der Konstruktions-Reihenfolge entschieden, nicht von der Implementierung — auch ein Waisen-Widget ist nach `show()` sichtbar) | **Konjunkt gestrichen**, der **Text**-Konjunkt trägt die Zeile allein und ist diskriminierend. Keine dritte unfalsifizierbare Zusage an dieser Stelle. |
| **MEDIUM-2** (die §6-Datei-Tabelle folgte der Einarbeitung nicht: die Hinweis-Anzeige hatte keine Zeile, die Handbuch-Zeile nannte weiter „zwei Stellen", der §4-10-Test hatte keinen Ort) | **§6 vollständig nachgezogen** — Fenster-Zeile nennt die Anzeige, Handbuch-Zeile die **vier** Stellen, Canvas- und Fenster-Testdatei sind getrennt und tragen ihre Orakel-Nummern, der §4-4/§4-10-Test hat eine eigene Zeile. |
| **MEDIUM-3** ([LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) verlangt „erscheint sofort **im Grundriss**"; dieser Konjunkt verlor mit Zeile 8 seinen einzigen Sensor) | **Neue Zeile 8a: Tinten-Sonde am Canvas** — mit der **selbst gemessenen Vorbedingung**, dass der Zug **innerhalb** der bestehenden Bounding-Box liegen muss (`633 → 993` Tinte bei unveränderter Abbildung). Außerhalb änderte das Neu-Einrahmen die Tinte mit, und der Zuwachs wäre nicht zuordenbar. |
| **MEDIUM-4** (die ADR-Folgepflicht „Surrogat **und** Tinten-Sonde" sollte als „erfüllt" gebucht werden, obwohl die Sonde **nirgends** im Slice vorkam — auch nicht am Canvas, wohin die eigene Ersatz-Regel des Plans zeigte) | **Mit 8a buchstäblich eingelöst**, an der Stelle, an der wir selbst malen. Am Fenster tritt die Qt-Eigenschaft an ihre Stelle — das bleibt die benannte Abweichung, aber sie ist keine Auslassung mehr. |
| **LOW-1 (Sammler, vier Fälle)** | (a) Orakel-Bereiche in DoD und Closure-Trigger nennen jetzt **§4-1 bis §4-10 inkl. 5a und 8a**, Netz separat · (b) §4-6 prüft **beide** Abbruch-Auslöser einzeln, wie R2a es sagt · (c) §4-7 liegt **nur** im Senken-Test, wie die DoD es zuweist · (d) §4-3 prüft den Fang an **beiden** Enden, wie §1 es zusagt. |
| **LOW-2..4 (Lauf 2), nachgeholt** | R3a ist präzisiert („**kann** verschieben — nur bei geänderter Bounding-Box") · die Zeilen-Zahl in §10 ist auf **dreizehn** korrigiert · der §1-Konjunkt „überlebt Speichern/Laden und Export" hat als **Zeile 12** einen benannten Platz: **Bestands-Netz**, mit ausgeschriebener Begründung, warum der Slice hier keinen eigenen Sensor baut. |

**Positiv bestätigt:** der Schnitt, die Abgrenzung und die R1-Entscheidung tragen weiterhin · die
Korrekturen der Läufe 1 und 2 sind an ihren Vollzugs-Stellen angekommen, **außer** den vier oben
benannten · die Messung „140/210" aus Lauf 2 ist fixture-abhängig und trägt die dortige Folgerung nur
teilweise (INFO-1) — für die **jetzige** Zeile 8 ist das ohne Belang, sie misst den Zoom, nicht die Ecke.

> **Die Lehre, dreimal bezahlt:** zweimal habe ich nach einem vorhandenen **Instrument** gegriffen,
> statt zu fragen, welche **Größe** sich beim Fehler ändert — beim dritten Mal habe ich den Sensor
> für nicht existent erklärt, statt weiterzusuchen. **„Ich habe kein Messmittel" ist eine Aussage
> über mich, nicht über die Prüfbarkeit.** Sie gehört belegt wie jede andere.

**Startbar:** **nein — noch nicht.** Zwei HIGH sind eingearbeitet; nach der stehenden Regel dieses
Projekts verlangt das eine **unabhängige Prüfung der Einarbeitung**. Auftrag an Lauf 4, eng und
terminierend: **trägt die wiederhergestellte Zeile 8 in ihrer dritten Form — und ist 8a mit der
Bounding-Box-Vorbedingung wirklich diskriminierend?** Zusätzlich: **sind die Rückverweise diesmal
vollständig mitgezogen** (die Bauart, die hier siebenmal aufgetreten ist)?

## 11c. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (vierter Lauf, 2026-07-29)

Report: [`2026-07-29-slice-058-plan-4.md`](../../../reviews/2026-07-29-slice-058-plan-4.md) —
**0 HIGH / 2 MEDIUM / 4 LOW / 4 INFO + 18 Negativbefunde, „STARTBAR"**. Unabhängiger Reviewer,
verschieden von Autor und den Läufen 1–3. Auftrag war eng: tragen die Zeilen 8 und 8a, sind die
Rückverweise mitgezogen, und hat die **Entrauschung** etwas Tragendes mitgenommen?

**Die Antwort auf die vierte Frage ist ja — genau einmal.**

| # | Behandlung |
|---|---|
| **MEDIUM-1** (Zeile 8 **trägt** — die zitierten Werte wurden exakt reproduziert —, aber ihre Vorbedingung „der Modell-Wechsel muss die Bounding-Box **vergrößern**" ist mit der Entrauschung entfallen; gemessen steht der Zoom bei einer Wand **innerhalb** der Box mit **und** ohne Anmeldung still, und die Nachbarzeile 8a schreibt im **selben** Testfile die **gegenteilige** Vorbedingung vor) | **Vorbedingung zurückgeholt und in der Tabellenzeile selbst benannt**, mit ausdrücklichem Hinweis auf die Gegenläufigkeit zu 8a. Zusätzlich in R4 aufgenommen, was der Reviewer nebenbei fand: **die Bestands-Fixture meldet gar keinen Beobachter an** — ohne `subscribe` im Setup wäre Zeile 8 grün wie ihre Gegenprobe. |
| **MEDIUM-2** (zwei Verweise im Anweisungsteil zeigen auf ein **§2.4**, das der Plan nicht hat — die Entrauschung hatte denselben Verweis an **einer** von drei Stellen geheilt) | Beide ersetzt (§4). **Achte Wiederholung der Bauart „Rückverweis überlebt seine Zielstelle"** — diesmal von mir selbst erzeugt, beim Aufräumen. |
| **LOW-1** (§7 führte R1 als offene Aufgabe, obwohl §2.3 und die DoD sie als entschieden buchen — **dritte** Nennung derselben Stelle) | R1 steht jetzt als **entschieden** da, mit dem Grund, warum der Eintrag trotzdem bleibt: das Restrisiko liegt im Vollzug. |
| **LOW-2** (beide Orakel-Zählungen stale) | **vierzehn** Zeilen, **zwölf** mit eigenem Sensor. |
| **LOW-3** (die Produktions-DoD-Zeile der Senke buchte nur §4-7, obwohl §4-5 dort ebenfalls festgestellt wird) | **§4-5 und §4-7.** Dieselbe Zuordnung war für die Test-Zeile bereits gelöst — die Produktions-Zeile war übrig geblieben. |
| **LOW-4** (die Tinten-Zahl `993` ist ohne Nennung des Segments nicht nachprüfbar; der Reviewer misst für sein Segment `905`, für die Geste `904`) | **Beide Segmente stehen jetzt dabei**, und die Zeile sagt ausdrücklich: **Sollwert ist die Richtung, nicht die Zahl.** |
| **INFO-3** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) fehlt im Plan — **vierte** Nennung über vier Läufe) | **Ausdrücklich verortet** statt ein fünftes Mal übergangen: es hängt an der Welle-Closure und steht in [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md) (bei der Teilung von slice-059 dorthin gewandert). |

**Positiv bestätigt (18 Negativbefunde):** Zeile 8 ist an der genannten Naht **diskriminierend**
(fünf eigene Messläufe des Reviewers) · Zeile 8a ebenfalls — die Gegenprobe wurde **emuliert**
(Wand angelegt und gemeldet, aber nicht gezeichnet ⇒ Tinte `633 → 633`; gezeichnet ⇒ `905`), und die
Sorge „andere Farbe könnte die Zeile grün halten" ist entkräftet: der einzige weitere Beitrag
(Fang-Marker, +58 px) wird von **jeder** `op`-Meldung strukturell gelöscht · §5 ↔ §6 decken sich
vollständig, alle Orakel-Nummern existieren, R2 ist geheilt · **die Entrauschung hat außer der
Bounding-Box-Klammer nichts Bauleitendes mitgenommen** — alles Übrige war Chronik.

> **Die Lehre dieses Laufs:** Aufräumen ist eine **Änderung**. Ich habe 27 Zeilen entfernt und dabei
> genau die eine Klammer erwischt, die eine Vorbedingung trug — und den Verweis, den ich an einer
> Stelle heilte, an zwei anderen stehen gelassen. **Wer kürzt, prüft die Rückverweise wie nach jeder
> anderen Änderung auch.**

**Startbar: JA** — 0 HIGH nach der vorab festgelegten Abbruchregel. Die zwei MEDIUM und vier LOW sind
eingearbeitet; sie betreffen Setup-Vorbedingungen und Verweise, **keine** Bauart. Ein fünfter Lauf
ist nicht vorgesehen: die Regel lautet 0 HIGH, nicht „bis nichts mehr kommt".

## 12. Closure-Notiz

**Vollzogen 2026-07-29.** `make gates` grün, **387 Tests** (371 vor dem Slice, **+16** neu),
Zeilen-Coverage 92,4 %. Zusätzlich außerhalb des Aggregats: `make io-smoke` grün und
`make acc-002-beleg` grün (9 Wand-Netze gerendert) — nötig, weil dieser Slice die Verdrahtung im
Composition-Root ändert. **Das erzeugte Beleg-BILD ist bewusst nicht committet:**
`docs/plan/planning/done/acc-002-beleg.png` gehört zur dokumentierten Abnahme-Runde von
[`slice-012`](../done-archive/slice-012-eckenschluss-wal006-teil.md); der Lauf hier belegt, dass das Target **funktioniert**,
und ersetzt keinen Abnahme-Nachweis des Projektinhabers. Kein Kern-/Persistenz-/Export-/Schema-Diff — am
`git diff --stat` über `src/hexagon`, `src/adapters/io`, `src/adapters/persistence`, `spec/`,
`data-model.yaml`, `schema.sql` belegt: **leer**.

### Die Orakel-Zeilen, je mit EINZELN gemessener roter Gegenprobe

Jede Sonde mutierte **genau eine** Stelle, lief durch `make test` und wurde zurückgenommen. Eine
Sonde, die zwei Zeilen gleichzeitig kippt, belegt keine von beiden — deshalb steht bei jeder Zeile,
**was** gefallen ist.

| Zeile | Sonde (Mutation) | gefallen |
|---|---|---|
| §4-1 | Default auf `Wall` gestellt | `ADR_0021_E1_DefaultIstHilfslinie` + **zwei Bestands-Orakel** (Hilfslinien-Zug, Fang-Anzeige) |
| §4-2 | Modus ignoriert (immer Hilfslinie) | `LH_FA_WAL_001_HappyPath_ImWandModusEntstehtEineWand` + 5 weitere Wand-Zeilen |
| §4-3a | Fang im **Press**-Pfad übersprungen | `LH_FA_DRW_001_FangGiltAnBeidenEndenDesWandZugs`, `LH_FA_WAL_006_ZweiZuegeTeilen…` |
| §4-3b | Fang im **Release**-Pfad übersprungen | dieselben zwei — **beide Enden einzeln belegt** |
| §4-4 | (dieselbe Sonde wie §4-3, wie der Plan sie vorschreibt) | `LH_FA_WAL_006_ZweiZuegeTeilenDenGefangenenPunktExakt` |
| §4-5 | Ablehnung wird nicht gemeldet | `WallSink.LH_FA_WAL_001_Boundary_EntarteterZugMeldetKeineWand` |
| §4-5a | Anzeige-Aufruf entfernt | `MainWindow.ADR_0021_E5_HinweisAnzeigeTraegtDenGemeldetenText` |
| §4-6a | Escape-Abbruch entfernt | `ADR_0021_E12_EscapeBrichtDieGesteAb` |
| §4-6b | Fokusverlust-Abbruch entfernt | `ADR_0021_E12_FokusverlustBrichtDieGesteAb` |
| §4-7 | `try`/`catch` der Barriere entfernt | **drei** Senken-Zeilen (der Wurf entweicht) |
| §4-8 (Produktion) | `fitted_ = false` aus `onModelChanged` entfernt | `ADR_0021_E6_NeuEinrahmenKommtAusDerMeldekette` |
| §4-8 (Anmeldung) | `subscribe` im Test entfernt | dieselbe Zeile — **die vom Plan vorgeschriebene Gegenprobe** |
| §4-8a | Zeichnen der Wand-Segmente unterdrückt | `LH_FA_WAL_001_HappyPath_WandErscheintSofortImGrundriss` |
| §4-9a | Auslösung der Wand-Aktion entfernt | `ADR_0021_E1_WerkzeugAktionenSindAusloesbarUndMarkiert` |
| §4-9b | Exklusiv-Markierung entfernt | dieselbe Zeile — **beide Konjunkte einzeln** |
| §4-10 | 3D-Szene sieht `WallAdded` nicht | `LH_FA_D3_002_DieGesteErreichtDieDreiDSicht` (+ das Bestands-Netz) |

**§4-11/§4-12 sind Netz, nicht Beleg:** die Hilfslinien-Orakel liefen unverändert grün (Zeile 11);
Persistenz/Export prüfen `make io-smoke` und die Runden-Orakel (Zeile 12) — der Slice hat dort
**keinen** eigenen Sensor gebaut, und das war die benannte Entscheidung.

**Zwei Zeilen teilen eine Sonde, und das steht hier statt es zu verschweigen:** §4-3 und §4-4 fallen
beide auf »Fang übersprungen« — so schreibt der Plan es vor. §4-4 prüft dennoch etwas Eigenes (die
**Gleichheit über zwei Züge**, aus zwei verschiedenen Richtungen mit verschiedenen Pixel-Versätzen
angefahren); dafür gibt es aber keine Mutation, die nur sie kippt.

### Was der Vollzug gegenüber dem Plan geändert hat

1. **Die Barriere fängt `...`, nicht `const std::exception&`.** Der Plan sagte „fängt dessen Würfe";
   die Zusage lautet aber „**kein** Wurf verlässt den Ereignis-Pfad". Eine Barriere, die nur die
   dokumentierten Typen fängt, ist keine — und der Typ trägt hier ohnehin keine Information, weil
   beide Wurf-Quellen denselben Ausgang bekommen (§2.3).
2. **Der Wand-Zug des Canvas gibt nichts zurück** (`std::function<void(Point2D, Point2D)>`). Das ist
   §2.3 in der Signatur: der Canvas **kann** den Ausgang nicht kennen, statt ihn nur nicht zu nutzen.
3. **`setFocusPolicy(Qt::StrongFocus)` ist Teil der Zusage, nicht Beiwerk** — dieselbe Klasse wie
   `setMouseTracking` in slice-055: ohne Fokus stellt Qt **keine** Tasten-Ereignisse zu, der
   Escape-Abbruch wäre im Produkt tot, während ein Test, der `QKeyEvent` direkt zustellt, grün bliebe.
4. **Ein Modus-Wechsel bricht eine laufende Geste ab.** Stand nicht im Plan; ohne die Regel entstünde
   aus einem als Hilfslinie begonnenen Zug eine Wand.
5. **`test_project_open_handler.cpp` mitgezogen** (nicht im Plan): dort liegt der Beleg, dass die
   **produktiven** Senken nach einem Projekt-Laden nachgezogen werden. Ohne die Wand-Senke in dieser
   Kette würfe `addWall` nach **jedem** Laden — die Barriere fänge es, aber der Benutzer bekäme statt
   einer Wand einen Hinweis. Jetzt belegt der Test, dass die Wand auf dem geladenen Stand entsteht.
6. **Das Handbuch brauchte mehr als vier Stellen** (der Plan nannte vier, die DoD verlangte zu
   **suchen**): §1 „Heute möglich" · §1 „noch NICHT möglich" (präzisiert **und** um zwei ehrliche
   Grenzen ergänzt: kein Entfernen/Rückgängig, keine nachträgliche Parameter-Änderung) · **§2.2**
   (Werkzeug-Menü **und** Hinweis-Zeile — die Oberflächen-Beschreibung kannte beides nicht) ·
   **§2.3** (Werkzeug wählen, Zug abbrechen) · §3-Kopf · §3-Wege-Liste Punkt 4 · §3-Hinweis-Block ·
   4.1-Tabelle · 4.2 (Titel + neuer Unterabschnitt) · FAQ. Handbuch **1.6**.

### Reichweite und benannte Grenzen

- **Der Composition-Root bleibt orakel-los** (§4-9): `src/main.cpp` ist in kein Testbinary gelinkt.
  Geprüft ist der **Fenster-Vertrag**, nicht die produktive Verdrahtung Aktion → `setToolMode`.
  Bekannte Klasse, keine neue Lücke — und `make io-smoke`/`make acc-002-beleg` belegen, dass der
  geänderte Root **läuft**.
- **Sensor-los bleibt** ein Selbst-Refresh, der nur `update()` ruft, **ohne** neu einzurahmen. Das ist
  nicht der Fall, vor dem [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E6 warnt (dort: doppeltes
  Neu-Einrahmen), und es steht hier, weil die Zusage nur so weit deckt, wie sie messen kann.
- **Lastenheft und Spezifikation: am Artefakt geprüft, nicht pauschal verneint.** [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md)
  hatte geliefert; nachgesehen wurde diesmal konkret, ob die §6-Vertragszeile („Selbst-Refresh ohne
  `op`") mit E6 unvollständig geblieben ist — sie ist es **nicht**: der 056-Block schreibt den
  **zweigeteilten** Refresh aus (Zeichen-Daten: Selbst-Refresh; Bauteile: nur die Meldung), und die
  Architektur-Tabellenzeile trägt dieselbe Zweiteilung. **Kein Spec-Diff nötig** — diesmal belegt.

### Wellen-Stand

Der Abschluss-Trigger von **welle-6-interaktiv-planen** ist **zur Hälfte** erfüllt: eine Wand ist im
2D-Canvas **zeichenbar**, ohne Kommandozeile. Die zweite Hälfte („**parametrisch änderbar**") ist
[`slice-059a`](../in-progress/slice-059a-wand-auswaehlen.md) (Auswahl) und [`slice-059b`](../open/slice-059b-wand-parameter-aendern.md)
(Parameter). Davor steht dort das
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
des ganzen Bauteil-Strangs — **einschlägig**, anders als bei [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md).
