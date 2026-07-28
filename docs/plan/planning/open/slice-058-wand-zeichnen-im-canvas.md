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

### 2.3 Die offene Frage: wie der Ausgang zum Hinweis kommt (R1)

Die Fehl-Ausgänge brauchen Hinweise (E5), aber **die Texte liegen im Composition-Root** (benannte
Grenze des Fenster-Adapters). Der Ausgang muss also als **Wert** von der Senke bis zur Anzeige. Wo
der Typ dieses Werts wohnt, ist die eine echte Entwurfs-Frage:

| Variante | Konsequenz |
|---|---|
| **A** — Ausgangs-Typ in `ui/view/`, die Senke in `ui/command/` erzeugt ihn | verlangt einen `command/ → view/`-Include. Die Schicht-Kante ist erlaubt, **aber** die laterale Adapter-Regel hat einen **skalaren** Ausnahme-Eintrag; ob dieser Include sie verletzt, ist am Gate zu **messen**, nicht zu vermuten |
| **B** — **kein** gemeinsamer Typ: das Callable liefert `optional<WallId>`; den Entartungs-Fall erkennt der Canvas an seinen **eigenen** zwei Punkten, die übrigen Fehl-Ausgänge teilen einen Hinweis | braucht **keinen** neuen Include und **keine** Gate-Frage; Preis: zwei Ausgänge werden für den Benutzer **ein** Hinweis |

**Empfehlung: B, sofern die Messung nicht zeigt, dass A gate-frei ist.** Begründung: der Unterschied
zwischen „abgelehnt" und „Geometrie fehlgeschlagen" ist für den Benutzer keiner — beide Male ist
**nichts entstanden und das Modell unverändert**, und genau das sagt E5 für beide Zeilen zu. **Die
Entscheidung fällt vor dem Start** (R1), nicht im Vollzug — und sie fällt an einer **Messung**:
`make a-check` mit einem probeweisen Include.

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
| 3 | **Der Fang gilt** — ein Zug, dessen Ende in Fang-Nähe eines Endpunkts liegt, erzeugt eine Wand mit **exakt** dessen mm | `CanvasWidget`, headless | Fang im Wand-Pfad übersprungen ⇒ rot |
| 4 | **Zwei Züge teilen einen gefangenen Punkt exakt** — die Voraussetzung des Eckenschlusses ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)) | Modell-Surrogat | Fang übersprungen ⇒ rot (die Punkte differieren) |
| 5 | **Entarteter Zug ⇒ keine Wand, Modell unverändert, Hinweis** | `CanvasWidget` + Hinweis-Surrogat | Hinweis nicht gemeldet ⇒ rot |
| 6 | **Gesten-Abbruch ⇒ keine Wand, Modell unverändert** | `CanvasWidget`, headless | Abbruch-Behandlung entfernt ⇒ rot |
| 7 | **Kein Wurf verlässt den Ereignis-Pfad** — bei veralteter Geschoss-Id gibt es einen **Hinweis**, keine Ausnahme | `ui/command/`-Senke + `CanvasWidget` | `try`/`catch` entfernt ⇒ rot |
| 8 | **Refresh nur über die Meldung** — nach dem eigenen Kommando rahmt der Canvas **einmal** neu ein, nicht zweimal | **Zähl-Callable in der `PlanPull`-Naht** (Muster slice-055) | zusätzlicher Selbst-Refresh ⇒ **mehr** Pulls ⇒ rot |
| 9 | **Der Modus ist bedienbar und sichtbar** — die Aktion ist auffindbar, sie schaltet den Canvas-Zustand um, und die aktive Aktion ist **markiert** | `MainWindow` (Aktions-Surrogat) + Canvas-Zustand | Verdrahtung entfernt ⇒ rot; Markierung entfernt ⇒ rot |
| 10 | **Die 3D-Sicht folgt** — nach dem Zeichnen meldet der Kern, und der Viewer-Surrogat trägt die neue Wand ([LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)) | Bestands-Viewer-Surrogat | (Netz — die Meldekette ist Bestand) |
| 11 | **Der Hilfslinien-Pfad bleibt unverändert** — freies Zeichnen, Fangen, Anzeige, Entartungs-Ablehnung | Bestands-Orakel | (Regressions-Netz) |

**Zeile 8 ist die Zeile, die man für selbstverständlich hält.** Ein zusätzlicher Selbst-Refresh sähe
im Test **genauso grün** aus wie keiner — die Wand entsteht so oder so. Erst der Pull-Zähler macht den
Unterschied messbar, und genau diese Zusage trennt den Bauteil-Weg vom Zeichen-Weg
([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E6).

**Zeile 7 bricht den Test womöglich ab, statt ihn rot zu machen** — eine geworfene Ausnahme aus einem
Qt-Ereignis-Handler ist kein `EXPECT`-Fehlschlag. Die Gegenprobe ist trotzdem eindeutig; die Closure
hält fest, **wie** der Lauf endet.

## 5. Definition of Done

- [ ] **R1 vor dem Start entschieden** (§2.3) — Variante A **gemessen** (`make a-check` mit
      probeweisem Include) oder B gewählt. **Nicht in den Vollzug verschieben.**
- [ ] **`src/adapters/ui/view/canvas_widget.{h,cpp}`**: Werkzeug-Modus als Widget-Zustand + lesbare
      Eigenschaft; im Wand-Modus ruft der Zug die neue Senke; Gesten-Abbruch; **kein** zusätzlicher
      Selbst-Refresh. Orakel §4-1..3, 5, 6, 8.
- [ ] **`src/adapters/ui/command/`-Wand-Senke** (neu): baut aus dem gefangenen Segment die Wand über
      den Bearbeitungs-Port, **fängt** dessen Würfe und meldet den Ausgang als Wert. Orakel §4-7.
- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: Aktions-Gruppe für den Modus, injizierte
      Callables, aktive Aktion **markiert**. Orakel §4-9.
- [ ] **`src/main.cpp`**: Verdrahtung (Modus-Callables → Canvas, Senke → Port, Hinweis-Texte).
      **Die Texte bleiben hier** — benannte Grenze des Fenster-Adapters.
- [ ] **Tests**: `test_canvas_widget.cpp` (§4-1..3, 5, 6, 8), `test_main_window.cpp` (§4-9), Test der
      neuen Senke (§4-7), Modell-Surrogat für §4-4/10.
- [ ] **Orakel §4-1..9 je mit roter Gegenprobe** im Closure-Text, **einzeln** gemessen; §4-10/11 als
      Netz benannt.
- [ ] **`make a-check` grün** — **mit** ausgeschriebener Aussage, ob eine neue Kante entstanden ist
      (R1). **Kein** Kern-/Persistenz-/Export-/Schema-Diff, am `git diff --stat` belegt.
- [ ] **Benutzerhandbuch**: §4.2 bekommt den Wand-Modus; §1 „Heute möglich" und die 4.1-Tabelle
      nachziehen. **Der Satz „ein Gebäude selbst planen … noch NICHT möglich" wird damit
      teil-unwahr** — er ist zu **präzisieren** (Wände ja, andere Bauteile nein), nicht zu streichen.
      Handbuch-Version + Änderungshistorie.
- [ ] **[ADR-Index](../../adr/README.md)**: die „Zeichnen-Slice"-Folgepflichtzeile auf **erfüllt**.
- [ ] **Lastenheft/Spezifikation: am Artefakt prüfen, ob etwas fehlt** — beides hat
      [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md) geliefert, aber **nicht pauschal
      verneinen**: im 057-Review war genau diese Pauschal-Verneinung falsch.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**;
      **`make acc-002-beleg` grün** (die 3D-Kette wird berührt).

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/view/canvas_widget.{h,cpp}` | ändern | Modus, Wand-Zug, Abbruch |
| `src/adapters/ui/command/`-Wand-Senke `.{h}` | neu | Schreibpfad + Fehler-Barriere |
| `src/adapters/ui/view/main_window.{h,cpp}` | ändern | Aktions-Gruppe |
| `src/main.cpp` | ändern | Verdrahtung + Hinweis-Texte |
| `tests/adapters/test_canvas_widget.cpp`, `tests/adapters/test_main_window.cpp` | ändern | §4 |
| `tests/adapters/`-Test der Wand-Senke `.{cpp}` | neu | §4-7 |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |
| `docs/user/benutzerhandbuch.md` | ändern | §4.2 + §1 + 4.1 + Version |
| `docs/plan/adr/README.md` | ändern | Folgepflichtzeile |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `src/hexagon/**`, `src/adapters/io/**`, `src/adapters/persistence/**`, `spec/**`,
`data-model.yaml`/`schema.sql`, `docs/plan/adr/0021-wand-im-2d-canvas.md`.

## 7. Risiken

- **R1 — der Ausgangs-Typ und die laterale Adapter-Regel** (§2.3). **Vor dem Start zu entscheiden, an
  einer Messung.** Die Regel hat einen **skalaren** Ausnahme-Eintrag; ob ein
  `command/ → view/`-Include ihn verletzt, ist am Gate abzulesen. Präzedenz für die Bauart der
  Entscheidung: der Canvas-Slice hat drei Optionen verglichen und die **gate-freie** gewählt, statt
  die Regel zu weiten.
- **R2 — der Refresh ist gegenläufig zum Vorgänger.** Wer den Hilfslinien-Zug kopiert, baut einen
  zweiten Refresh ein; er fiele **nicht** auf, weil die Wand trotzdem entsteht. Orakel-Zeile 8 ist
  deshalb Pflicht, nicht Kür.
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

- §4-1..9 grün + je diskriminierend belegt; §4-10/11 als Netz grün; `make gates`, `make io-smoke` und
  `make acc-002-beleg` grün; kein Kern-/Schema-/Export-Diff belegt; Handbuch und ADR-Index
  nachgezogen; Closure-Notiz. **Der Wellen-Trigger ist damit zur Hälfte erfüllt** — die zweite Hälfte
  ist [`slice-059`](slice-059-wand-auswaehlen-und-aendern.md).

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** mittel-groß — ein Modus-Zustand, eine zweite Senke mit Barriere, eine
  Aktions-Gruppe, elf Orakel-Zeilen, vier berührte Dateien plus Composition-Root.
- **Phase-Reife:** die Geste, der Fang, die Anzeige und die Fixture-Bauform liegen; die Entscheidungen
  liegen seit [ADR-0021](../../adr/0021-wand-im-2d-canvas.md).
- **Risiko:** mittel — **nicht im Zeichnen** (die Geste existiert), sondern im **Fehler-Ausgang** und
  im **Refresh-Pfad**, die beide gegenläufig zum Vorgänger sind.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung

_(offen — der Lauf steht vor dem Start aus; HIGHs blockieren ihn.)_

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
