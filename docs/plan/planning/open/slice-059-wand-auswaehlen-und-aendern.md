---
id: slice-059
titel: Wand auswählen und parametrisch ändern — der Abschluss-Trigger ([LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003, [ADR-0021](../../adr/0021-wand-im-2d-canvas.md))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 059: Wand auswählen und parametrisch ändern

**Status:** open — **Detail-Schnitt vollzogen** (2026-07-29; die Skelett-Fassung trug nur
Scope-Reservierung + ADR-Bezug, [MR-020](../../../../harness/conventions.md) §3). **Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.**

**Welle:** welle-6-interaktiv-planen — **zweite Hälfte des Abschluss-Triggers.** Mit diesem Slice ist
der Trigger erfüllt: „eine Wand ist im 2D-Canvas zeichenbar **und parametrisch änderbar**, ohne
Kommandozeile." Danach ist die Welle zu **schließen**, nicht weiterzufüllen.

**Setzt voraus:** [`slice-057`](../done/slice-057-lese-naht-bauteil-identitaet.md) (Bauteil-Identität
im Segment + die schmale Parameter-Abfrage) und
[`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) (Werkzeug-Modus, Hinweis-Zeile, zweite
Senke als Barriere) — alle drei werden hier **benutzt**, nicht neu gebaut.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-29.

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md), **Entscheidungen 3, 4, 5, 13, 14, 15, 16, 17**.
Der Canvas kann **zwei** Gesten (Hilfslinie, Wand) und **keine** Auswahl. Eine gezeichnete Wand ist
damit unveränderlich — die Parametrik des Kerns ist über die Oberfläche nicht erreichbar.

## 1. Ziel

Im **Auswahl-Modus** wählt ein Klick **genau eine** Wand des **dargestellten** Geschosses; sie ist
auf der Fläche **erkennbar** hervorgehoben. Ein nicht-modaler **Eigenschaften-Bereich** zeigt ihre
**Stärke** und **Höhe** und nimmt neue Werte an: der **übernommene** Wert ist ablesbar (auch wenn
geklemmt wurde, **mit Nennung des Werts**), eine Ablehnung lässt das Modell unverändert und ist
sichtbar, und die 3D-Darstellung folgt ohne Zutun. **Die Auswahl fällt**, sobald ihr Bezug fallen
könnte.

## 2. Die vier Entwurfs-Fragen dieses Slice

### 2.1 Die Treffer-Prüfung ist der Spiegel des Fangs — mit drei Unterschieden, die zählen

Bauform wie [`snap.h`](../../../../src/adapters/ui/view/snap.h): eine **reine, display-freie**
Funktion in `view/`, die aus `PlanView` + `ViewTransform` + Cursor-Pixel einen Treffer liefert.
Getroffen wird im **Bildschirmraum** mit eigener Pixel-Toleranz (zoom-unabhängige Trefferfläche);
**kein Default** für den Schwellwert (Lehre slice-053: ein Default-Argument versteckt die
Kalibrierung).

| | Fang (`snapTarget`) | Treffer-Prüfung (dieser Slice) |
|---|---|---|
| Geometrie | Abstand zu **Endpunkten** | Abstand zum **Segment** (eine Wand ist eine Strecke, kein Punkt) |
| Geschosse | **alle** (Koordinaten-Aussage) | **nur das dargestellte** (Zeige-Aussage) |
| Segment-Arten | Wand-Achsen **und** Hilfslinien | **nur Wand-Achsen** (`PlanSegmentKind::WallAxis`) |

**Warum die Geschoss-Beschränkung nicht optional ist:** das mitgelieferte Demo-Modell legt in EG und
OG **dieselben vier Außenwände** an. Ohne Beschränkung träfe ein Klick auf eine sichtbare
OG-Außenwand **deterministisch** die unsichtbare des EG — angezeigt **und** geändert würde die
falsche. **Ein reproduzierbar falscher Treffer ist schlimmer als ein zufälliger**, weil er
zuverlässig am Benutzer vorbeigeht.

**Rückgabe ist die starke Id** (`std::optional<model::WallId>`), nicht der Zahlwert: `ui_view → model`
ist eine deklarierte Kante, und eine `int`-Rückgabe wäre an der ersten Verwechslung mit einer
`GuideLineId` still falsch.

### 2.2 Die Lebensdauer der Auswahl braucht DREI Regeln — und der Auslöser für die erste existiert schon

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E17 verlangt das Verwerfen bei (a) Modell-Ersetzung,
(b) Geschoss-Wechsel, (c) Verschwinden aus der Sicht. **Am Artefakt nachgesehen, nicht erraten:**

- **(a) hat einen exakten, vorhandenen Auslöser.** `replaceBuilding` meldet **genau eine**
  Full-Refresh-Meldung mit `op == ModelReplaced` — bei **jedem** Öffnen und **jedem** Anlegen. Die
  Auswahl fällt **auf diese Meldung**, ausdrücklich und ohne Heuristik.
- **(b)** fällt in `setActiveStorey` — die Naht, die der Composition-Root beim Geschoss-Wechsel und
  nach dem Laden ohnehin ruft.
- **(c)** ist ein **Netz**: nach jeder anderen Meldung wird geprüft, ob die gewählte Wand noch in der
  Sicht des aktiven Geschosses vorkommt; sonst fällt sie.

**Warum keine der drei die anderen ersetzt** — das ist der Kern dieser Frage:

- **Nur (c) prüfen wäre die Fehlerklasse, vor der E17 warnt.** Der Id-Zähler wird beim Laden auf das
  Maximum des geladenen Projekts gesetzt, und ein geladenes Projekt trägt **dichte eigene Ids** —
  eine überlebende Auswahl bezeichnet mit hoher Wahrscheinlichkeit eine **andere, existierende**
  Wand. Die Prüfung „kommt sie noch vor?" sagte dann **ja**. Die Fehler-Barriere schwiege ebenfalls:
  `setWallThickness` wirft **nicht** bei einer gültigen fremden Id. **Die Änderung träfe still das
  Falsche.**
- **Nur (a) + (b) wäre ohne Netz**, sobald ein Entfernen-Pfad entsteht (heute gibt es keinen — E12).
- **(b) steckt nicht in (a):** ein Geschoss-Wechsel ersetzt kein Modell.

**Und (c) ist heute strukturell nicht erreichbar** — es gibt kein `removeWall`, und jede
Modell-Ersetzung fällt schon unter (a). Das wird **so benannt** und bekommt **kein** Orakel, das
Erreichbarkeit vortäuscht (Bauform der [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)-E8-Ehrlichkeit
aus [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md)).

### 2.3 Das Eingabe-Feld darf NICHT selbst klemmen — sonst sind zwei Akzeptanzkriterien unerreichbar

[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) verlangt
interaktiv **beides**: „Klemmung sichtbar" (Eingabe außerhalb des Bereichs ⇒ geklemmt **und der
übernommene Wert wird genannt**) und „Ablehnung sichtbar" (ungültige Eingabe ⇒ Modell unverändert +
Hinweis). Daraus folgt die Wahl des Widgets — zwingend:

| Variante | Konsequenz |
|---|---|
| Zahlen-Drehfeld mit Modell-Bereich 50–1000 | **Beide Negativ-AK werden unerreichbar.** Das Widget klemmt die Eingabe selbst, 49 ist nicht eintippbar, „nicht-endlich" gar nicht — der Kern käme nie zum Klemmen, der Hinweis nie zum Erscheinen. Das ist genau das Dritte, das [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E8 verbietet: „die AK-Zeile stehen lassen und nichts dazu bauen" |
| Zahlen-Drehfeld mit weiterem Bereich | erreichbar, aber es gäbe **zwei** Klemm-Autoritäten; welche gegriffen hat, wäre nicht ablesbar |
| **Textfeld + Übernahme** (gewählt) | der **Kern** ist die **einzige** Klemm-Autorität; beide Negativ-AK sind über die Oberfläche erreichbar |

**Übernommen wird bei Abschluss der Eingabe, nicht je Tastendruck.** Ein je Tastendruck übernehmender
Pfad mutierte beim Tippen von „240" zuerst auf 2 (⇒ geklemmt auf 50), dann 24 (⇒ 50), dann 240 — drei
Modell-Mutationen, drei Geometrie-Neubauten, drei Raum-Neuerkennungen und **zwei falsche
Klemm-Hinweise** für **eine** Eingabe. [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E5 sagt „eine
Klemmung beim Tippen darf den Fluss nicht unterbrechen" — das ist erfüllt, weil der Hinweis
**nicht-modal** ist, nicht weil jeder Tastendruck wirkt.

**Nach der Übernahme zeigt das Feld den ÜBERNOMMENEN Wert**, nicht die Eingabe — das ist der
wörtliche AK-Konjunkt („das Eingabefeld zeigt den geklemmten Wert").

### 2.4 Wo die „sofort"-Zusage beobachtbar ist — und wo sie es NICHT ist

[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) sagt „Geometrie
und 3D-Körper aktualisieren sich **sofort**". Auf der **2D-Fläche hat das kein Korrelat**: der Canvas
zeichnet Wand-**Achsen**, und eine Stärken-Änderung bewegt keine Achse. Auch die Bounding-Box bleibt
gleich, also die Abbildung.

**Beobachtbar ist die Zusage an drei Stellen** ([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E4):
am **übernommenen Wert** im Eigenschaften-Bereich, am **3D-Körper** (Viewer-Surrogat) und im
**Export**. **Eine Tinten-Sonde am Canvas für die Stärken-Änderung wäre nicht diskriminierend** — sie
rührte sich nicht, egal ob die Änderung ankam. Das steht hier, weil die Sonde in
[`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) **getragen** hat und die naheliegende
Übertragung darum falsch wäre: **ein Instrument, das anderswo trug, trägt hier nicht.**

**Die Sonde trägt an einer anderen Stelle dieses Slice:** die **Hervorhebung der Auswahl** ist ein
Zeichen-Vorgang auf der Fläche (§4-13). Vorbedingung wie in
[`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) §4-8a: **unveränderte Abbildung** — hier
von selbst erfüllt, weil Auswählen **keine** Modell-Mutation ist (kein `op`, kein Neu-Einrahmen). Das
ist zu **prüfen**, nicht anzunehmen.

## 3. Bewusst NICHT Teil

- **Mehrfach-Auswahl, Bereichs-Geste, Sammel-Änderung.** E3: höchstens **eine** Wand; benannter
  Re-Eval.
- **Gemeinsame Auswahl 2D↔3D, Selektion im Viewport.** E3: zieht den
  [ADR-0009](../../adr/0009-gui-framework-qt6.md)-Re-Eval (AIS/V3d, **Supersedes**-ADR).
- **Wandtyp und Material.** E13: [LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen)
  ist reine Outline ohne AK; beides interaktiv zu bedienen hieße, **fremde Anforderungen in einem
  UI-Strang auf AK-Niveau zu schärfen**.
- **Wand verschieben/teilen** ([LH-FA-WAL-004](../../../../spec/lastenheft.md#lh-fa-wal-004)/005,
  Outline), **Entfernen und Rückgängigmachen** (E12, benannte Grenze im Lastenheft).
- **Andere Bauteile auswählen** (Türen, Fenster, Räume, Treppen, Dächer). Die Lese-Naht trägt seit
  [`slice-057`](../done/slice-057-lese-naht-bauteil-identitaet.md) eine **Art**, aber im Grundriss
  kommen nur Wand-Achsen und Hilfslinien vor.
- **Andocken/Anordnen des Eigenschaften-Bereichs.** E4: ein **fester** Bereich, keine
  Docking-Verwaltung ([LH-FA-UI-001](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui)
  bleibt Outline).
- **Jede Änderung an Kern, Persistenz, Export, Schema.** Auswahl und Modus sind UI-Zustand; die
  Parameter-Mutatoren und die Lese-Naht **existieren**.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Die Treffer-Prüfung ist rein und display-frei** — Abstand zum **Segment**, Grenze fängt (Distanz == Schwellwert trifft noch), leere Sicht ⇒ kein Treffer | neue `view/`-Funktion, **ohne** `QWidget`/`QApplication` (Bauform `test_snap.cpp`) | Endpunkt- statt Segment-Abstand ⇒ rot (ein Klick auf die Mitte einer 4 m langen Achse trifft dann nicht mehr) |
| 2 | **Nur Wand-Achsen sind wählbar** — ein Klick auf eine Hilfslinie wählt **nichts** | dieselbe Funktion, Fixture mit Hilfslinie **und** Wand | `WallAxis`-Filter entfernt ⇒ die Hilfslinie wird gewählt ⇒ rot |
| 3 | **Nur das dargestellte Geschoss** — bei **deckungsgleichen** Achsen in EG und OG trifft der Klick die Wand des **aktiven** Geschosses, an der **Id** geprüft | dieselbe Funktion, Fixture mit zwei Geschossen | Geschoss-Filter entfernt ⇒ die Wand des **anderen** Geschosses wird gewählt ⇒ rot. **Das ist die von [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E16 benannte symptomlose Fehlerklasse** |
| 4 | **Höchstens eine, die nächstgelegene** — bei zwei Achsen in Reichweite gewinnt die nähere; außerhalb der Reichweite ⇒ **kein** Treffer | dieselbe Funktion | Reihenfolge- statt Distanz-Auswahl ⇒ rot |
| 5 | **Der Klick im Auswahl-Modus wählt** — und **nur** dort: im Wand-/Hilfslinien-Modus wählt derselbe Klick nichts, im Auswahl-Modus zeichnet er nichts | `CanvasWidget`, headless, über `selection()` | Modus ignoriert ⇒ rot (**beide** Richtungen einzeln) |
| 6 | **Die Auswahl fällt bei Modell-Ersetzung** — **auch wenn** der neue Stand eine Wand mit **derselben Id** trägt | `CanvasWidget` + Dienst: `replaceBuilding` mit einem Stand, dessen Ids die gewählte **enthalten** | `ModelReplaced`-Behandlung entfernt ⇒ die Auswahl überlebt ⇒ rot. **Die Fixture ist der Punkt:** mit einem Stand **ohne** diese Id wäre die Zeile auch ohne die Behandlung grün (das Netz griffe), und die eigentliche Fehlerklasse bliebe unbelegt |
| 7 | **Die Auswahl fällt beim Geschoss-Wechsel** | `CanvasWidget`, `setActiveStorey` | Behandlung entfernt ⇒ rot |
| 8 | **Der Eigenschaften-Bereich zeigt die Parameter der gewählten Wand** — die Werte stammen aus der **schmalen Abfrage** (slice-057), nicht aus dem Domänen-Objekt | `MainWindow`-Surrogat: die Feld-Inhalte == `wallParams` | Anzeige-Aufruf entfernt ⇒ rot |
| 9 | **Keine Auswahl ⇒ nichts zu ändern** — der Bereich sagt „keine Auswahl" und trägt **keine** Werte einer zuvor gewählten Wand; **auch nach** Öffnen/Anlegen und nach Geschoss-Wechsel | `MainWindow`-Surrogat, im Anschluss an §4-6/§4-7 | Leeren des Bereichs entfernt ⇒ die alten Werte stehen weiter da ⇒ rot. **Abnahmebindend** ([LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) Boundary (Auswahl)) |
| 10 | **Stärke ändern wirkt** — der übernommene Wert steht im Feld, das Modell trägt ihn, und der **Viewer-Surrogat** folgt ohne Zutun ([LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)) | Fenster (Eingabe) → Senke → Dienst → `ViewerScene` am **selben** Dienst | Übernahme-Aufruf entfernt ⇒ rot. **Von der Bedienung ausgelöst**, nicht am Dienst — sonst belegt die Zeile nur Bestand (Lehre slice-058 §4-10) |
| 11 | **Klemmung ist sichtbar UND benannt** — Eingabe 49 ⇒ Modell trägt **50**, das Feld zeigt **50** (nicht 49), und der Hinweis **nennt den übernommenen Wert** | Senken-Test (Ausgang) + `MainWindow`-Surrogat (Feld + Hinweis-Text) | Hinweis ohne Wert ⇒ rot; Feld zeigt die Eingabe ⇒ rot. **Vorbedingung: das Eingabe-Widget klemmt NICHT selbst** (§2.3) — sonst ist die Zeile **unerreichbar** statt rot |
| 12 | **Ablehnung ist sichtbar, das Modell unverändert** — nicht-numerische bzw. nicht-endliche Eingabe ⇒ kein neuer Wert im Modell, Hinweis | Senken-Test + Surrogat | Ablehnungs-Weg entfernt ⇒ rot |
| 13 | **Die Auswahl ist auf der Fläche erkennbar** — die gewählte Achse ist hervorgehoben | **Tinten-Sonde am Canvas** (offscreen). **Vorbedingung, zu prüfen statt anzunehmen:** Auswählen ist **keine** Mutation ⇒ Abbildung unverändert (Zoom **und** Modell-Ecke vergleichen, Muster slice-058 §4-8a) | Hervorhebung entfernt ⇒ Tinte unverändert ⇒ rot |
| 14 | **Höhe ändern gilt gleichlautend** ([LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren)) — Happy, Klemmung (499 ⇒ 500), Ablehnung | wie §4-10..12, **eigene** Zeile | Höhen-Weg auf den Stärke-Mutator verdrahtet ⇒ rot (die Verwechslung wäre sonst still) |
| 15 | **Kein Wurf verlässt den Ereignis-Pfad** — bei **veralteter** Wand-Id gibt es einen **Hinweis**, keine Ausnahme (`setWallThickness` wirft bei unbekannter Id) | **`ui/command/`-Parameter-Senke** (dort liegt die Barriere und dort liegt der Test) | `try`/`catch` entfernt ⇒ rot |
| 16 | **Der Zeichen-Pfad bleibt unverändert** — Hilfslinie, Wand, Fang, Anzeige, Abbruch, Hinweise | Bestands-Orakel | (Regressions-Netz) |
| 17 | **Die geänderte Stärke überlebt Speichern/Laden und Export** | **Bestands-Netz** (`make io-smoke`, Persistenz-Runden-Orakel): eine über den Bereich geänderte Wand ist dieselbe wie eine über den Dienst geänderte, und §4-10 belegt, dass die Bedienung den Dienst erreicht | (Netz — **kein** eigener Sensor, benannte Entscheidung) |

**Fünfzehn Zeilen tragen einen eigenen Sensor** (1–15), zwei sind **Netz** (16, 17).

**Was ausdrücklich KEIN Orakel bekommt** (§2.4): die 2D-Sichtbarkeit einer **Stärken**-Änderung. Der
Canvas zeichnet Achsen; eine Stärken-Änderung bewegt keine. Ein Tinten-Orakel dafür wäre **nicht
diskriminierend** — es bliebe unverändert, egal ob die Änderung ankam.

**Und (c) aus §2.2 bekommt keins:** „die gewählte Wand verschwindet aus der Sicht" ist ohne
Entfernen-Pfad **strukturell nicht erreichbar**. Die Prüfung wird gebaut (Netz für den Tag, an dem
ein Entfernen entsteht) und **so benannt** — kein Orakel, das Erreichbarkeit vortäuscht.

## 5. Definition of Done

- [ ] **`src/adapters/ui/view/`-Treffer-Prüfung** (neu, Bauform `snap.{h,cpp}`): rein, display-frei,
      Segment-Abstand, eigener px-Schwellwert **ohne Default**, Filter auf `WallAxis` **und** aktives
      Geschoss. Orakel §4-1..4.
- [ ] **`src/adapters/ui/view/canvas_widget.{h,cpp}`**: dritter Werkzeug-Modus (**Auswahl**), Klick
      wählt, `selection()` als display-freie Naht, Hervorhebung im Paint-Pfad, **drei**
      Lebensdauer-Regeln (§2.2) und ein injiziertes `SelectionChanged`-Callable (port-frei, Muster
      [ADR-0019](../../adr/0019-drw-2d-canvas.md) Option A). Orakel §4-5..7, 13.
- [ ] **`src/adapters/ui/command/`-Parameter-Lese-Quelle** (neu, Bauform `plan_view_plan_source.h`):
      kapselt `PlanViewPort::wallParams`. **Kein** neuer Port — die Naht existiert seit slice-057.
- [ ] **`src/adapters/ui/command/`-Parameter-Senke** (neu): `setWallThickness`/`setWallHeight` über
      den Bearbeitungs-Port, **fängt** dessen Würfe, meldet Ausgang **und übernommenen Wert** als
      Wert. Orakel §4-11, 12, 15.
- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: Auswahl-Aktion in der Werkzeug-Gruppe +
      **nicht-modaler Eigenschaften-Bereich** (zwei Felder, Übernahme bei Abschluss der Eingabe,
      „keine Auswahl"-Zustand). Orakel §4-8..12, 14.
- [ ] **`src/main.cpp`**: Verdrahtung (Auswahl → Parameter-Anzeige; Feld → Senke → Hinweis **und**
      Anzeige-Nachzug). **Die Texte bleiben hier** — benannte Grenze des Fenster-Adapters.
- [ ] **Tests**: neuer `tests/adapters/`-Test der Treffer-Prüfung (§4-1..4, **ohne** Qt-Fixture) ·
      `test_canvas_widget.cpp` (§4-5..7, 13) · neuer Test der Parameter-Senke (§4-11, 12, 15) ·
      `test_main_window.cpp` (§4-8, 9, 14) · **ein Test, der Fenster, Senke und Viewer-Surrogat an
      denselben Dienst hängt** (§4-10).
- [ ] **Orakel §4-1 bis §4-15 je mit roter Gegenprobe** im Closure-Text, **einzeln** gemessen; §4-16
      und §4-17 als **Netz** benannt, nicht als Beleg gebucht. **Zwei Zeilen tragen eine
      ausgeschriebene Vorbedingung** (§4-11: das Feld klemmt nicht selbst; §4-13: unveränderte
      Abbildung) — ohne sie messen sie nichts.
- [ ] **`make a-check` grün** — **mit** ausgeschriebener Aussage, ob eine neue Kante entstanden ist.
      **Kein** Kern-/Persistenz-/Export-/Schema-Diff, am `git diff --stat` belegt.
- [ ] **Benutzerhandbuch** — **gesucht, nicht aufgezählt** (in slice-058 waren es zehn Stellen statt
      der geplanten vier): mindestens §1 „noch NICHT möglich" (die Sätze zu **Auswahl** und
      **nachträglicher Parameter-Änderung** werden unwahr — sie stehen dort seit slice-058), §2.2
      (Eigenschaften-Bereich), §2.3, die 4.1-Tabelle, ein neuer 4.2-Unterabschnitt, die FAQ. Plus
      Version + Änderungshistorie.
- [ ] **[ADR-Index](../../adr/README.md)**: die Folgepflichtzeilen „Auswahl-/Änderungs-Slice" **und**
      „[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
      vor der Welle-Closure" auf **erfüllt**.
- [ ] **Lastenheft/Spezifikation: am Artefakt prüfen, ob etwas fehlt** — [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md)
      hat geliefert (WAL-002/003 tragen die interaktiven Konjunkte; die Spezifikation trägt
      Treffer-Prüfung, Lebensdauer, Lese-Naht, Rückmeldung und Barriere). **Nicht pauschal
      verneinen:** im 057-Review war genau diese Pauschal-Verneinung falsch.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**;
      **`make acc-002-beleg` grün** — nötig, weil der Beleg das Demo-Modell rendert, dessen Aufbau im
      Composition-Root liegt, und dieser Slice dort die Verdrahtung ändert. **Das erzeugte Bild wird
      NICHT committet** (es gehört zur Abnahme-Runde von
      [`slice-012`](../done-archive/slice-012-eckenschluss-wal006-teil.md); Lehre slice-058).
- [ ] **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
      des Bauteil-Strangs** — **vor** der Welle-Closure, unabhängiger Reviewer: Eckenschluss,
      Nachbar-Rebuild und Raum-Neuerkennung **im interaktiven Pfad**. **HIGHs blockieren die
      Closure.** Es hängt an diesem Slice, weil hier der letzte interaktive Mutator entsteht.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/view/`-Treffer-Prüfung `.{h,cpp}` | neu | §4-1..4 (rein, display-frei) |
| `src/adapters/ui/view/canvas_widget.{h,cpp}` | ändern | Auswahl-Modus, Auswahl-Zustand, Hervorhebung, Lebensdauer |
| `src/adapters/ui/command/`-Parameter-Lese-Quelle `.{h}` | neu | Kapselung von `wallParams` |
| `src/adapters/ui/command/`-Parameter-Senke `.{h}` | neu | Schreibpfad + Barriere + übernommener Wert |
| `src/adapters/ui/view/main_window.{h,cpp}` | ändern | Auswahl-Aktion + Eigenschaften-Bereich |
| `src/main.cpp` | ändern | Verdrahtung + Hinweis-Texte |
| `tests/adapters/`-Test der Treffer-Prüfung `.{cpp}` | neu | §4-1..4 |
| `tests/adapters/`-Test der Parameter-Senke `.{cpp}` | neu | §4-11, 12, 15 |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-5..7, 13 |
| `tests/adapters/test_main_window.cpp` | ändern | §4-8, 9, 14 |
| `tests/CMakeLists.txt` | ändern | zwei neue Testdateien |
| `docs/user/benutzerhandbuch.md` | ändern | **gesucht** (§1 · §2.2 · §2.3 · 4.1 · 4.2 · FAQ) + Version |
| `docs/plan/adr/README.md` | ändern | zwei Folgepflichtzeilen |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Reports | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) vor der Closure |

**Nicht berührt** (`spec/**` unter Vorbehalt der DoD-Prüfzeile): `src/hexagon/**`,
`src/adapters/io/**`, `src/adapters/persistence/**`, `spec/**`, `data-model.yaml`/`schema.sql`,
`docs/plan/adr/0021-wand-im-2d-canvas.md`.

## 7. Risiken

- **R1 — die zwei symptomlosen Fehler dieses Slice.** Geschoss-Skopus (§2.1) und
  Auswahl-Lebensdauer (§2.2) fallen **beide** nicht auf: die falsche Wand würde angezeigt **und**
  geändert, ohne Wurf und ohne Ablehnung. Ihre Orakel (§4-3, §4-6) müssen den Zustand **vor und
  nach** dem Wechsel prüfen, und ihre Fixtures müssen so gebaut sein, dass die **Fehlerklasse selbst**
  fällt — bei §4-6 heißt das: der neue Stand **muss** die gewählte Id enthalten.
- **R2 — die Klemm-Autorität** (§2.3). Ein selbst klemmendes Eingabe-Feld macht zwei abnahmebindende
  AK **unerreichbar** statt rot. Vor dem Bau der Orakel-Zeile 11 ist am Artefakt zu prüfen, dass eine
  Eingabe von 49 den **Kern** erreicht.
- **R3 — die Fixture-Fallen der Vorgänger, kumuliert.** Der Fit muss **synchron** erzwungen werden
  (slice-057), der Beobachter **ausdrücklich** angemeldet werden (slice-058 R4), und Positionen sind
  nach **jeder** Mutation neu aus der Transformation zu rechnen (slice-058 R3a). Eine
  Stärken-Änderung verschiebt die Abbildung **nicht** (§2.4) — das erleichtert die Fixture, ist aber
  zu **prüfen**, nicht anzunehmen.
- **R4 — der Eigenschaften-Bereich ist der erste Ort, an dem das Fenster einen Wert ANNIMMT.** Alle
  bisherigen Fenster-Nähte lösen nur aus (`Action`, `CloseGuard`, `ToolActions`). Ein Eingabefeld
  trägt Zustand, und die Zusage „das Feld zeigt den übernommenen Wert" verlangt, dass das Fenster
  **nach** der Übernahme neu gesetzt wird — der Rückweg ist Teil der Verdrahtung, nicht Beiwerk.
- **R5 — `wallParams` ist total, der Mutator wirft.** Dieselbe Wand-Id liefert lesend `nullopt` und
  schreibend eine Ausnahme. Wer den Lese-Weg als Vorbild für den Schreib-Weg nimmt, baut die fehlende
  Barriere ein — **dieselbe Falle wie in slice-058**, eine Ebene weiter.
- **R6 — das Handbuch wird an den Stellen unwahr, die slice-058 GERADE geschrieben hat.** Dort stehen
  „Auswahl gibt es nicht" und „Stärke/Höhe nachträglich nicht änderbar" als **benannte Grenzen**. Sie
  sind zu **ersetzen**, nicht zu ergänzen.

## 8. Trigger

- [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen, Folgepflicht „Auswahl-/Änderungs-Slice"
  **und** „[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  vor der Welle-Closure", samt den Zeilen im [ADR-Index](../../adr/README.md).

## 9. Closure-Trigger

- §4-1 bis §4-15 grün + **je einzeln** diskriminierend belegt; §4-16/§4-17 als Netz grün;
  `make gates`, `make io-smoke` und `make acc-002-beleg` grün; kein Kern-/Schema-/Export-Diff
  belegt; Handbuch und ADR-Index nachgezogen; Closure-Notiz.
- **Danach die Welle:** der Abschluss-Trigger von welle-6 ist erfüllt — liegt die
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Freigabe
  vor, ist **zu schließen** (M6 buchen), nicht weiterzufüllen. Die Lehre der welle-5-Closure steht im
  Wellen-Block: **schließen, sobald der Trigger erfüllt ist, nicht, wenn die Arbeit ausgeht.**

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas + Fenster)

- **Modus:** GF; **Dichte:** groß — eine reine Treffer-Funktion, ein dritter Werkzeug-Modus, ein
  Auswahl-Zustand mit **drei** Lebensdauer-Regeln, eine Lese-Quelle, eine Senke mit Barriere, ein
  Eingabe-Bereich mit Rückweg, **siebzehn** Orakel-Zeilen (fünfzehn mit eigenem Sensor).
- **Risiko:** **hoch für den Strang** — Auswählen ist das Interaktions-Muster, das das Produkt
  **nirgends** hat, und die zwei benannten Fehler sind symptomlos.
- **Warum trotz der Dichte NICHT geteilt:** eine Auswahl ohne Eigenschaften-Bereich hat **keinen
  benutzer-sichtbaren Nutzen** — man könnte etwas auswählen und nichts damit tun —, und der Bereich
  ohne Auswahl hätte kein Subjekt. Ein Split ergäbe eine Hälfte, deren Abnahmekriterium lautete „es
  ist ausgewählt, aber man merkt es nur im Test". **Der Abschluss-Trigger der Welle verlangt beides
  zusammen.** Findet das
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  den Umfang für eine Sitzung dennoch zu groß, ist der Schnitt entlang §4-1..7 (Auswahl) /
  §4-8..15 (Parameter) vorbereitet — die Orakel-Zeilen sind bereits so gruppiert.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung

_(bei Ausführung auszufüllen)_

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
