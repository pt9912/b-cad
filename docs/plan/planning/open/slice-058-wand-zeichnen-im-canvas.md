---
id: slice-058
titel: Wand zeichnen im 2D-Canvas ([LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 058: Wand zeichnen im 2D-Canvas

**Status:** open — **Detail-Schnitt vollzogen** (2026-07-28; die Skelett-Fassung trug nur
Scope-Reservierung + ADR-Bezug, [MR-020](../../../../harness/conventions.md) §3). **Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.**

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

**Entschieden: Variante B — an der Messung, nicht an der Vermutung** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1). Der Reviewer
hat drei `make a-check`-Läufe gefahren: Baseline **0 Befunde** → probeweiser
`command/ → view/`-Include ⇒ **`lateral-adapter`, 1 Befund, Exit ≠ 0** → Gegenrichtung ⇒ ebenfalls
1 Befund → Rücknahme, Baseline wieder 0. **Die Schicht-Kante `ui_command → ui_view` ist deklariert
und wird nicht bemängelt — die laterale Adapter-Regel schlägt unabhängig davon zu**, und ihr
Ausnahme-Eintrag ist skalar. Variante A ginge nur über eine **Weitung** dieses Eintrags, also eine
Gate-Lockerung, die [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen ausdrücklich
ausschließt. Dieselbe Frage hat der Canvas-Slice schon einmal so entschieden.

**Und B bekommt eine andere Bauform als geplant** (Lauf-1-MEDIUM-2). Die Vorfassung wollte den
**Canvas** die Entartung „an seinen eigenen zwei Punkten" erkennen lassen — **das ist falsch**: der
Kern verwirft unterhalb der Geometrie-Toleranz (0,1 mm), und bei Maximal-Zoom (100 px/mm) liegen
**benachbarte Pixel 0,01 mm** auseinander. Der Canvas hielte den Zug für gültig, der Kern verwürfe
ihn — **ein falscher Hinweis**.

**Die Senke meldet den Hinweis selbst.** Sie kennt den Ausgang (Rückgabewert **und** gefangene Würfe)
und bekommt vom Composition-Root ein **eigenes** Hinweis-Callable. Der Hinweis-Typ lebt damit in
`ui/command/`, **der Canvas sieht ihn nie**, und es entsteht **kein** verbotener Include. Das Callable
des Canvas bleibt, was es ist: „zeichne eine Wand von A nach B".

**Wie viele Hinweise es gibt — ausgeschrieben** (Lauf-2-MEDIUM-2): **drei**, nicht vier.

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
  [`slice-059`](slice-059-wand-auswaehlen-und-aendern.md).
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
| 3 | **Der Fang gilt an BEIDEN Enden** — je ein Zug, dessen **Anfang** bzw. **Ende** in Fang-Nähe liegt, erzeugt eine Wand mit **exakt** dessen mm (§1 sagt beide zu; die Vorfassung prüfte nur das Ende) | `CanvasWidget`, headless | Fang im Press- bzw. Release-Pfad einzeln übersprungen ⇒ rot |
| 4 | **Zwei Züge teilen einen gefangenen Punkt exakt** — die Voraussetzung des Eckenschlusses ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)) | `CanvasWidget` (die zwei Züge) gegen den **Modell-Zustand des Dienstes** — der Ort ist der Canvas, der Beleg das Modell (Lauf-1-LOW-4) | Fang im Wand-Pfad übersprungen ⇒ rot (die Punkte differieren) |
| 5 | **Entarteter Zug ⇒ keine Wand, Modell unverändert, Hinweis** — die **Senke** stellt den Ausgang fest, nicht der Canvas (§2.3) | Senken-Test + Hinweis-Surrogat | Hinweis nicht gemeldet ⇒ rot |
| 5a | **Die Hinweis-Anzeige existiert und trägt den gemeldeten Text** | `MainWindow`-Surrogat: das Anzeige-Widget ist auffindbar und sein **Text** ist der gemeldete | Anzeige-Aufruf entfernt ⇒ rot. **Der `isVisible()`-Konjunkt ist gestrichen** (Lauf-3-MEDIUM-1, gemessen): er wird von Qt und der Konstruktions-Reihenfolge entschieden, **nicht** von der Implementierung — ohne `show()` ist **jedes** Kind unsichtbar, mit `show()` **jedes** sichtbar, sogar ein nie eingelayoutetes Waisen-Widget. **Der Text-Konjunkt trägt die Zeile allein** |
| 6 | **Gesten-Abbruch ⇒ keine Wand, Modell unverändert** — **beide** Auslöser einzeln: Escape **und** Fokusverlust (E12) | `CanvasWidget`, headless | je Auslöser einzeln entfernt ⇒ rot |
| 7 | **Kein Wurf verlässt den Ereignis-Pfad** — bei veralteter Geschoss-Id gibt es einen **Hinweis**, keine Ausnahme | **`ui/command/`-Senke** (dort liegt die Barriere und dort liegt der Test) | `try`/`catch` entfernt ⇒ rot |
| 8 | **Der Refresh kommt aus der Meldekette** — nach dem Kommando rahmt der Canvas neu ein, **ohne** eigenes Zutun | `CanvasWidget::transform()` vor/nach dem Zug | Beobachter-Anmeldung entfernt ⇒ Transformation bleibt stehen ⇒ rot. **Die Gegenprobe ist zugleich der Beleg für E6** (§2.4) |
| 8a | **Die Wand erscheint sofort im Grundriss** — der abnahmebindende Konjunkt aus [LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), 2D-Hälfte | **Tinten-Sonde am Canvas**, offscreen gerendert. **Vorbedingung, selbst gemessen:** der Zug muss **innerhalb der bestehenden Bounding-Box** liegen, dann bleibt die Abbildung stehen und der Zuwachs ist **nur** das neue Segment — `633 → 993` Tinte bei unverändertem Zoom `0,09` und unveränderter Modell-Ecke `(20,285)`. Ein Zug **außerhalb** änderte die Tinte auch durch das Neu-Einrahmen und wäre nicht zuordenbar | Zeichnen der Wand-Segmente unterdrückt ⇒ rot |
| 9 | **Der Modus ist bedienbar und sichtbar** — die Aktion ist auffindbar, sie ruft das injizierte Callable, und die aktive Aktion ist **markiert** | `MainWindow` (Aktions-Surrogat) | Auslösung entfernt ⇒ rot; Markierung entfernt ⇒ rot. **Reichweite benannt** (Lauf-1-MEDIUM-1): geprüft wird der **Fenster-Vertrag**, nicht die produktive Verdrahtung — die liegt im Composition-Root und ist **per Konstruktion orakel-los** (in kein Testbinary gelinkt). Das ist die bekannte Klasse, keine neue Lücke |
| 10 | **Die 3D-Sicht folgt** — nach dem Zeichnen **über die Geste** trägt der Viewer-Surrogat die neue Wand ([LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)) | Bestands-Viewer-Surrogat, **von der Geste ausgelöst** | Der Bestands-Beleg genügt **nicht** (Lauf-1-MEDIUM-4): er löst die Mutation direkt am Dienst aus. Diskriminierend wird die Zeile erst, wenn der **Zug** sie auslöst |
| 11 | **Der Hilfslinien-Pfad bleibt unverändert** — freies Zeichnen, Fangen, Anzeige, Entartungs-Ablehnung | Bestands-Orakel | (Regressions-Netz) |
| 12 | **Die gezeichnete Wand überlebt Speichern/Laden und Export** — der letzte Konjunkt aus §1 | **Bestands-Netz, nicht neu gebaut**: `make io-smoke` und die Persistenz-Runden-Orakel. **Begründung, warum das genügt** (Lauf-2-LOW-4): eine über die Geste erzeugte Wand ist für Persistenz und Export **dieselbe** Wand wie eine über den Dienst erzeugte — §4-2 belegt, dass die Geste den Dienst erreicht, ab dort ist es Bestand | (Netz — der Slice fügt hier **keinen** eigenen Sensor hinzu, und das ist eine benannte Entscheidung, keine Lücke) |

**Zeile 8 gibt es wieder — in ihrer dritten und diesmal gemessenen Form.** Die Vorgeschichte gehört
hierher, weil sie die Lehre trägt: zwei Fassungen waren **unfalsifizierbar**, und die dritte
Entscheidung („E6 hat gar keinen Sensor") war **auch falsch**.

- **Fassung 1** wollte „kein zusätzlicher Selbst-Refresh" über einen **Pull-Zähler** belegen. Qt
  koalesziert beliebig viele `update()` eines Durchlaufs zu **einem** Paint — 4 Pulls mit **und** ohne.
- **Fassung 2** wollte die Wirkung über die **Tinten-Sonde** belegen. Gemessen: die Tinte ändert sich
  auch **ohne** Beobachter-Anmeldung, weil der Canvas bei **jedem** Paint frisch pullt und die Sonde
  einen Paint erzwingt.
- **Der Verzicht** („keine Sensor, nur durch Lesen prüfbar") war für die **harmlose** Hälfte richtig
  und für die **schädliche** eine Ausrede (Lauf-3-HIGH-2).

**Die Messung, die es entschieden hat** (Lauf 3 nannte sie, **ich habe sie selbst nachgemessen**, weil
sie meine eigene Absage kippt — Sonde in der Bestands-Fixture, danach zurückgenommen): derselbe
Modell-Wechsel (eine Wand, die die Bounding-Box vergrößert) ergibt `transform().zoom`

- **ohne** Beobachter-Anmeldung: `0,09 → 0,09` — unverändert,
- **mit** Beobachter-Anmeldung: `0,09 → 0,009`.

(Lauf 3 maß `0,09 → 0,03`; der Zahlenwert hängt an der Größe der zugefügten Wand, die **Diskriminierung**
nicht.) Der Grund ist eindeutig und im Quelltext nachlesbar: das Neu-Einrahmen hat **genau eine**
Auslöse-Bedingung (`fitted_`), und außer `resizeEvent`/`setActiveStorey` führt nur `onModelChanged`
dorthin. **Weder Tinte noch Pull-Zahl konnten das zeigen — die Transformation kann es**, und sie ist
über die vorhandene Widget-Naht `transform()` ablesbar, die die Bestands-Fixture ohnehin benutzt.

**Was die Gegenprobe belegt, ist mehr als die Zeile behauptet.** „Beobachter-Anmeldung entfernt ⇒ die
Transformation bleibt stehen ⇒ rot" ist nicht nur die Kontrolle des Orakels, sondern **der Beleg für
E6 selbst**: gäbe es einen zusätzlichen Selbst-Refresh, der neu einrahmt, bliebe die Gegenprobe
**grün** — die Transformation änderte sich ja weiterhin. **Ein grüner Lauf dieser Gegenprobe ist
also der Nachweis, dass der Canvas nach dem eigenen Kommando nichts Eigenes tut.**

**Was weiterhin keinen Sensor hat, und das bleibt so gesagt:** ein Selbst-Refresh, der **nur**
`update()` ruft, ohne neu einzurahmen. Er ist nicht beobachtbar — und er ist auch nicht der Fall, vor
dem [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E6 warnt (dort geht es um das **doppelte
Neu-Einrahmen**). **Die Zusage deckt genau so weit, wie sie messen kann, und sagt das.**

> **Die Lehre, dreimal bezahlt:** ich hatte zwei Instrumente — Pull-Zähler und Tinten-Sonde — und
> habe zweimal nach ihnen gegriffen, statt zu fragen, **welche Größe sich beim Fehler überhaupt
> ändert**. Beim dritten Mal habe ich den Sensor für nicht existent erklärt, statt weiterzusuchen.
> **Die richtige Frage ist nicht „welches Werkzeug habe ich?", sondern „was wäre anders, wenn die
> Zusage verletzt wäre?"** — hier: die Abbildung.

## 5. Definition of Done

- [x] **R1 vor dem Start entschieden** (§2.3) — **gemessen** im [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1: Variante A ist
      **nicht** gate-frei (`lateral-adapter` schlägt an, in **beide** Richtungen), also **B** — mit
      der Korrektur, dass die **Senke** den Ausgang feststellt, nicht der Canvas.
- [ ] **`src/adapters/ui/view/canvas_widget.{h,cpp}`**: Werkzeug-Modus als Widget-Zustand + lesbare
      Eigenschaft; im Wand-Modus ruft der Zug die neue Senke; Gesten-Abbruch **auf beiden Auslösern**
      (Escape **und** Fokusverlust, E12); **kein** zusätzlicher Selbst-Refresh — E6, **mit Sensor**
      (`transform()`, §2.4/§4-8). Orakel §4-1..4, 6, 8, 8a.
- [ ] **`src/adapters/ui/command/`-Wand-Senke** (neu): baut aus dem gefangenen Segment die Wand über
      den Bearbeitungs-Port, **fängt** dessen Würfe und meldet den Ausgang als Wert. Orakel §4-7.
- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: Aktions-Gruppe für den Modus, injizierte
      Callables, aktive Aktion **markiert**. Orakel §4-9.
- [ ] **Die Hinweis-Anzeige selbst** (Lauf-1-HIGH-2) — im Bestand gibt es **keine**. Ohne sie wäre
      „jeder Fehl-Ausgang gibt einen Hinweis" eine Zusage ohne Adressat. Orakel §4-5a.
      **Die Tinten-Sonde ist hier NICHT das zweite Nachweis-Mittel** (Lauf-2-HIGH-2, gemessen): am
      Fenster zählt sie **120 000 von 120 000** Pixeln als Tinte, weil der Fenster-Hintergrund nicht
      weiß ist — sie trägt am Canvas nur, weil **dessen** Zeichen-Pfad weiß füllt. **Und sie wird
      hier auch nicht gebraucht:** die Fang-Anzeige brauchte eine zweite Ebene, weil **wir** malen;
      ein Standard-Anzeige-Widget wird von Qt selbst dargestellt, und dass es **sichtbar** ist und
      **welchen Text** es trägt, sind Qt-Eigenschaften — prüfbar ohne Pixel.
      **Die Folgepflicht-Zeile („Surrogat UND Tinten-Sonde") wird trotzdem buchstäblich eingelöst —
      nur an der richtigen Stelle:** die Sonde sitzt in **§4-8a am Canvas**, wo wir selbst malen, und
      belegt dort, dass die gezeichnete Wand **Farbe erzeugt**. Am Fenster tritt an ihre Stelle die
      Qt-Eigenschaft. **Damit ist die Folgepflicht auf beiden Ebenen erfüllt und nicht nur erklärt**
      (Lauf-3-MEDIUM-4: die Vorfassung buchte sie als „erfüllt", ohne dass die Sonde irgendwo im
      Slice vorkam).
- [ ] **`src/main.cpp`**: Verdrahtung (Modus-Callables → Canvas, Senke → Port, Hinweis-Texte).
      **Die Texte bleiben hier** — benannte Grenze des Fenster-Adapters.
- [ ] **Tests**: `test_canvas_widget.cpp` (§4-1..4, 6, 8, 8a — die Sonde für 8a **innerhalb** der
      bestehenden Bounding-Box, s. §4) · **Test der neuen Senke (§4-5 UND §4-7)** —
      der Ausgang wird **dort** festgestellt, nicht im Canvas (Lauf-2-MEDIUM-1: die Vorfassung buchte
      §4-5 weiter auf den Canvas und hob damit die Kern-Korrektur an ihrer Vollzugs-Stelle wieder
      auf) · `test_main_window.cpp` (§4-5a, §4-9) · **ein Test, der Canvas und Viewer-Surrogat an
      denselben Dienst hängt** (§4-4, §4-10 — Lauf-2-MEDIUM-3: die verschärfte Zeile hatte keinen
      Ort).
- [ ] **Orakel §4-1 bis §4-10 (inkl. 5a, 8, 8a) je mit roter Gegenprobe** im Closure-Text,
      **einzeln** gemessen — **zehn Zeilen tragen einen eigenen Sensor**; §4-11 und §4-12 sind
      **Netz** und werden als solches benannt, nicht als Beleg gebucht.
- [ ] **`make a-check` grün** — **mit** ausgeschriebener Aussage, ob eine neue Kante entstanden ist
      (R1). **Kein** Kern-/Persistenz-/Export-/Schema-Diff, am `git diff --stat` belegt.
- [ ] **Benutzerhandbuch — VIER Stellen, nicht zwei** (Lauf-2-MEDIUM-4): §4.2 bekommt den
      Wand-Modus; §1 „Heute möglich" **und** der Satz „ein Gebäude selbst planen … noch NICHT
      möglich" (zu **präzisieren**: Wände ja, andere Bauteile nein — nicht zu streichen); die
      4.1-Aufgaben-Tabelle; **§3** („vier Wege, mit einem Gebäude zu arbeiten" — zwei Stellen dort
      werden falsch) und die **FAQ**-Antwort „Kann ich in der Oberfläche Wände zeichnen? — Noch
      nicht." Handbuch-Version + Änderungshistorie.
      **Dieselbe Fehlerklasse stand seit der Fang-Lieferung unbemerkt im Lastenheft** — deshalb wird
      hier **gesucht**, nicht aufgezählt, was gerade einfällt.
- [ ] **[ADR-Index](../../adr/README.md)**: die „Zeichnen-Slice"-Folgepflichtzeile auf **erfüllt**.
- [ ] **Lastenheft/Spezifikation: am Artefakt prüfen, ob etwas fehlt** — beides hat
      [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md) geliefert, aber **nicht pauschal
      verneinen**: im 057-Review war genau diese Pauschal-Verneinung falsch.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**;
      **`make acc-002-beleg` grün** — **Begründung präzisiert** (Lauf-1-LOW-2): nicht weil „die
      3D-Kette berührt" würde (sie ist Bestand), sondern weil der Beleg das **Demo-Modell** rendert,
      dessen Aufbau im Composition-Root liegt — und dieser Slice ändert dort die Verdrahtung.

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
| `tests/adapters/`-Test, der Canvas **und** Viewer-Surrogat an denselben Dienst hängt | neu/ändern | §4-4 und §4-10 (Lauf-2-MEDIUM-3: die Zeile hatte keinen Ort) |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |
| `docs/user/benutzerhandbuch.md` | ändern | **vier Stellen** (§4.2 · §1 inkl. „noch NICHT möglich" · 4.1-Tabelle · §3 und die FAQ) + Version |
| `docs/plan/adr/README.md` | ändern | Folgepflichtzeile |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt** (Lauf-1-LOW-3: `spec/**` steht hier **unter Vorbehalt** der DoD-Zeile „am Artefakt
prüfen, ob etwas fehlt" — Lauf 1 hat geprüft und **Vollständigkeit festgestellt**; ändert sich das im
Vollzug, gewinnt die DoD-Zeile): `src/hexagon/**`, `src/adapters/io/**`, `src/adapters/persistence/**`, `spec/**`,
`data-model.yaml`/`schema.sql`, `docs/plan/adr/0021-wand-im-2d-canvas.md`.

## 7. Risiken

- **R1 — der Ausgangs-Typ und die laterale Adapter-Regel** (§2.3). **Vor dem Start zu entscheiden, an
  einer Messung.** Die Regel hat einen **skalaren** Ausnahme-Eintrag; ob ein
  `command/ → view/`-Include ihn verletzt, ist am Gate abzulesen. Präzedenz für die Bauart der
  Entscheidung: der Canvas-Slice hat drei Optionen verglichen und die **gate-freie** gewählt, statt
  die Regel zu weiten.
- **R2a — E12 nennt ZWEI Abbruch-Auslöser** (Lauf-1-LOW-1): Escape **und** Fokusverlust. Der Plan
  sagte nur „Gesten-Abbruch" — beide sind umzusetzen, und §4-6 prüft **beide** Wege einzeln.
- **R2 — der Refresh ist gegenläufig zum Vorgänger.** Wer den Hilfslinien-Zug kopiert, baut einen
  zweiten Refresh ein. **Kontrolliert wird das über die Gegenprobe zu Orakel-Zeile 8** (§2.4): bleibt
  sie grün, obwohl die Beobachter-Anmeldung entfernt wurde, gibt es einen Selbst-Refresh. **Der
  Verweis zeigte in der Vorfassung auf eine Zeile, die es zu diesem Zeitpunkt gar nicht mehr gab**
  (Lauf-3-HIGH-1) — dieselbe Bauart „Prosa korrigiert, Vollzugs-Zeile stehen gelassen", nur
  spiegelverkehrt.
- **R3a — eine gezeichnete Wand KANN die Abbildung verschieben** (präzisiert nach Lauf-2-LOW-1: **nicht
  jede** tut es — nur eine, die die Bounding-Box ändert). Anders als eine Hilfslinie meldet sie
  einen `op`; der Canvas rahmt daraufhin **neu ein**, und Bildschirm↔mm verschiebt sich. **Jede
  Fixture mit aufeinanderfolgenden Zügen muss ihre Positionen nach JEDEM Zug neu aus der
  Transformation rechnen** (Lauf-1-MEDIUM-3) — hartcodierte Pixel aus dem Hilfslinien-Muster wären
  ab dem zweiten Zug falsch. **Das ist eine neue Ursache derselben Fixture-Klasse wie R4, nicht
  dieselbe.**
- **R3 — das Demo-Modell hat zwei Geschosse mit deckungsgleichen Wänden.** Für **dieses** Slice
  harmlos (gezeichnet wird ins **aktive** Geschoss), aber die Fixture darf daraus keine
  Scheinsicherheit ziehen: eine neue Wand ist an ihrer **Geschoss-Id** zu prüfen, nicht nur an der
  Anzahl.
- **R4 — die Fixture-Falle aus slice-055/057.** Die Canvas-Fixture erzwingt den Fit-to-Bounds seit
  slice-057 **synchron**; jede neue Fixture muss dasselbe tun. **Ein Test, dessen Konstanten von
  asynchron hergestelltem Zustand abhängen, ist manchmal richtig** — und ein Orakel, das nur manchmal
  misst, täuscht Grün vor.
- **R5 — das Handbuch wird teil-unwahr.** „Ein Gebäude selbst planen ist noch NICHT möglich" stimmt
  nach diesem Slice für **Wände** nicht mehr. Präzisieren, nicht streichen — und **nicht vergessen**:
  dieselbe Klasse Fehler stand seit der Fang-Lieferung unbemerkt im Lastenheft, bis ein Nachzug sie
  zufällig aufdeckte.

## 8. Trigger

- [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen, Folgepflicht „Zeichnen-Slice", samt
  der Zeile im [ADR-Index](../../adr/README.md).

## 9. Closure-Trigger

- §4-1 bis §4-10 (inkl. 5a, 8, 8a) grün + **je einzeln** diskriminierend belegt; §4-11/§4-12 als Netz
  grün; `make gates`, `make io-smoke` und
  `make acc-002-beleg` grün; kein Kern-/Schema-/Export-Diff belegt; Handbuch und ADR-Index
  nachgezogen; Closure-Notiz. **Der Wellen-Trigger ist damit zur Hälfte erfüllt** — die zweite Hälfte
  ist [`slice-059`](slice-059-wand-auswaehlen-und-aendern.md).

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** mittel-groß — ein Modus-Zustand, eine zweite Senke mit Barriere, eine
  Aktions-Gruppe, eine Hinweis-Anzeige, **dreizehn** Orakel-Zeilen (1–12 inkl. 5a und 8a; 11 und 12
  sind Netz), vier berührte Produktions-Dateien plus Composition-Root.
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
| **HIGH-1** (die **neue** Zeile 8 ist **ebenfalls** nicht diskriminierend: gemessen steigt die Tinte **ohne** Beobachter-Anmeldung 633 → 905 bei unveränderter Abbildung — die Gegenprobe bliebe **grün**, weil der Canvas bei **jedem** Paint frisch pullt und die Sonde einen Paint erzwingt; umgekehrt ist „bei gleicher Abbildung" genau dann verletzt, wenn die Meldekette wirkt: 20/285 → 140/210) | **Zeile 8 ist ersatzlos gestrichen.** [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E6 hat **keinen** Sensor, und der Plan behauptet keinen mehr — die Einhaltung ist **durch Lesen** prüfbar, wie die ausdrücklich als „computational nicht prüfbar" geführten Prozess-Regeln. **Kein dritter Versuch.** [**Von Lauf 3 gekippt und selbst nachgemessen — es gibt einen Sensor, siehe §11b/§2.4. Diese Zelle bleibt als Protokoll stehen, sie ist nicht mehr der Stand.**] |
| **HIGH-2** (die Tinten-Sonde **trägt am Fenster nicht**: gemessen zählt sie in **allen drei** Zuständen **120 000 von 120 000** Pixeln als Tinte, weil der Fenster-Hintergrund nicht weiß ist — sie funktioniert am Canvas nur, weil **dessen** Zeichen-Pfad weiß füllt) | Zeile 5a stützt sich jetzt auf **Qt-Eigenschaften**: das Anzeige-Widget ist **sichtbar** und trägt den gemeldeten **Text**. **Und die zweite Ebene wird hier auch nicht gebraucht:** die Fang-Anzeige brauchte sie, weil **wir** malen — ein Standard-Widget stellt Qt selbst dar. Die Abweichung von der pauschalen „Tinten-Sonde"-Formulierung der Folgepflicht-Zeile gehört in die Closure. |
| **MEDIUM-1** (die DoD buchte §4-5 weiter auf den Canvas, obwohl §2.3 den Ausgang der **Senke** gibt — die Kern-Korrektur aus Lauf 1 war an ihrer **Vollzugs-Stelle** wieder aufgehoben) | DoD nachgezogen: §4-5 **und** §4-7 liegen im Senken-Test. **Dritte Wiederholung derselben Bauart in diesem Projekt: Prosa korrigiert, Vollzugs-Zeile stehen gelassen.** |
| **MEDIUM-2** (welche Ausgänge einen Hinweis teilen, sagte der Plan nicht; die zwei Wurf-Fälle wären nur über **undokumentierte** Ausnahme-Typen trennbar, und die alte Tabellenzeile widersprach dem Korrektur-Absatz) | **Ausgeschrieben: drei Hinweise, nicht vier**, mit Tabelle — und die Begründung, warum die zwei Wurf-Fälle **bewusst** zusammenfallen: auf undokumentiertes Wurf-Verhalten eine benutzer-sichtbare Unterscheidung zu bauen hieße, einen Vertrag zu erfinden. Die widersprüchliche Tabellenzeile ist ersetzt. |
| **MEDIUM-3** (die verschärfte Zeile 10 hatte **keinen Ort** — weder DoD-Testzeile noch Datei-Tabelle nannten einen Platz, an dem Canvas und Viewer-Surrogat an denselben Dienst kommen) | Eigene DoD-Testzeile für §4-4 **und** §4-10. |
| **MEDIUM-4** (der Handbuch-Nachzug war unvollständig: §3 mit **zwei** Stellen und die FAQ-Antwort „Kann ich in der Oberfläche Wände zeichnen? — Noch nicht." werden falsch) | DoD nennt jetzt **vier** Stellen statt zwei — und den Auftrag, zu **suchen** statt aufzuzählen. **Dieselbe Fehlerklasse stand seit der Fang-Lieferung unbemerkt im Lastenheft.** |
| **Lauf-1-LOW-1..4** (in §11 unbehandelt und unbenannt geblieben — vom zweiten Lauf gerügt) | **Nachgeholt:** R2a (E12 nennt **zwei** Abbruch-Auslöser) · die `acc-002-beleg`-Begründung ist präzisiert (nicht „3D-Kette berührt", sondern: der Beleg rendert das Demo-Modell, dessen Aufbau dieser Slice verdrahtet) · `spec/**` in §6 steht unter dem Vorbehalt der DoD-Prüfzeile · §4-4 nennt jetzt die **Komponente** statt „Modell-Surrogat". |

**Positiv bestätigt:** der **Schnitt** trägt · **Variante B trägt am Gate** (Baseline `make a-check`
= 0) · vier der sechs Lauf-1-Behandlungen sind substanziell.

> **Die Lehre dieses Slice, zweimal bezahlt:** ich habe zwei Sensoren erfunden, weil ich zwei
> **Instrumente** hatte — den Pull-Zähler und die Tinten-Sonde. Beide sind **situationsgebunden**.
> Dass ein Instrument anderswo getragen hat, ist **kein Argument** dafür, dass es hier trägt; das
> entscheidet die Messung — **vor** der Zusage, nicht im Review danach.

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
| **HIGH-2** (die Streichungs-Begründung „E6 hat KEINEN Sensor, prüfbar nur durch Lesen" ist **gemessen falsch** für die schädliche Hälfte von E6: derselbe Modell-Wechsel ergibt `transform().zoom` mit/ohne Beobachter-Anmeldung **verschiedene** Werte, ablesbar an der Naht, die die Bestands-Fixture ohnehin nutzt) | **Selbst nachgemessen** (Sonde in der Bestands-Fixture, danach zurückgenommen): **ohne** Anmeldung `0,09 → 0,09`, **mit** Anmeldung `0,09 → 0,009`. **Zeile 8 ist in dritter, diesmal messbarer Form wieder da** (§2.4/§4-8); ihre Gegenprobe ist zugleich der **Beleg für E6**. Was weiterhin sensor-los bleibt — ein Selbst-Refresh **ohne** Neu-Einrahmen —, steht als benannte Grenze da. |
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

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
