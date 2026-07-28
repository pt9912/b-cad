# ADR-0021: Die Wand im 2D-Canvas — Werkzeug-Modus, Zeichen-Geste, Selektion, Parameter-Bedienung, Rückmeldung und die Nachweis-Naht für reinen UI-Zustand

**Status:** Accepted

**Datum:** 2026-07-28

**Autor:** Dietmar Burkard (Eröffnung des interaktiven Bauteil-Strangs; ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [LH-FA-WAL-001](../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) (Wand zeichnen — die interaktive Erzeugung, die [ADR-0019](0019-drw-2d-canvas.md) §Abgrenzung ausdrücklich aus ihrem Schnitt genommen hat), [LH-FA-WAL-002](../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/[003](../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren) (Stärke/Höhe parametrisch, mit Klemmung **und Hinweis**), [LH-FA-WAL-006](../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) (Eckenschluss — der Grund, warum das Fangen hier nicht optional ist), [LH-FA-DRW-001](../../../spec/lastenheft.md#lh-fa-drw-001) (Fangpunkte — deren Teilumfang „Fangen beim Bauteil-Zeichnen" hier eingelöst wird), [LH-FA-DRW-005](../../../spec/lastenheft.md#lh-fa-drw-005) (Hilfslinien — die bestehende Geste, neben der die neue steht), [LH-FA-D3-002](../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung) (Echtzeitaktualisierung — der Refresh-Vertrag), [OBJ-001](../../../spec/lastenheft.md#3-projektziele) (Gebäude ohne tiefe CAD-Kenntnisse modellierbar), [ADR-0001](0001-hexagonale-architektur.md) (Schichtung — der Canvas ist ein Driving Adapter), [ADR-0006](0006-relationales-schema-design.md) (Schema — `entity_layers` bleibt schlafend, persistierter Undo-Stack bleibt unbedient), [ADR-0008](0008-aenderungs-benachrichtigung.md) (Benachrichtigung — Bauteil-Mutationen melden einen `op`), [ADR-0009](0009-gui-framework-qt6.md) (Qt = Driving/UI, Regel E; (f) hält `tests/e2e/` leer „bis ein Treiber mit Interaktion/Selektion relevant wird" — dieser Strang löst den Auslöser aus und beantwortet ihn), [ADR-0010](0010-headless-gl-xvfb.md) (Headless-GL — die Infrastruktur der Interaktions-AK), [ADR-0018](0018-drw-2d-zeichen-daten.md) (DRW-Fundament — „kein `op`" gilt für Zeichen-Daten, **nicht** für Bauteile; [MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) dort n/a, hier **nicht**), [ADR-0019](0019-drw-2d-canvas.md) (2D-Canvas — **fortgeschrieben, nicht ersetzt**), [ADR-0020](0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (der Kern liefert abgeleitete Geometrie — die Lese-Naht dieser ADR bleibt auf dieses Prinzip geschnitten)

---

## Kontext

Der Canvas kann seit dem 2D-Zeichen-Strang **eine** Geste: Links-Zug erzeugt eine **Hilfslinie**, seit der Fang-Erweiterung millimetergenau und mit sichtbarer Anzeige des Fang-Ziels. Was er **nicht** kann, ist das, wofür das Produkt existiert: ein **Bauteil** anlegen und ändern. Das Benutzerhandbuch führt „ein Gebäude selbst planen" bis heute unter „In dieser Version noch **NICHT** möglich".

**Die Lücke ist ausdrücklich benannt und ausdrücklich vertagt worden.** [ADR-0019](0019-drw-2d-canvas.md) nimmt in ihrem Abgrenzungs-Block wörtlich „**Bauteile interaktiv zeichnen** (Wände/Räume auf dem Canvas), **Layer-Bedien-Panel**, **Selektion/Picking**, **Bemaßung**" aus ihrem Schnitt („benannte Re-Eval-Trigger, nicht dieser Schnitt"). ADRs sind nach `Accepted` immutabel ([`AGENTS.md`](../../../AGENTS.md) §2.5) — die Auflösung ist eine **neue** ADR, kein Nachtrag.

**Was bereits liegt und hier nicht neu entschieden wird:** der `EditStructurePort` bietet `addWall(StoreyId, Segment)`, `setWallThickness` und `setWallHeight` mit einem dreiwertigen `ParamResult` (`Accepted`/`Clamped`/`Rejected`); der Kern rechnet Nachbar-Ecken neu und stößt die Raum-Neuerkennung an; Bauteil-Mutationen melden einen `op` ([ADR-0008](0008-aenderungs-benachrichtigung.md)); der Canvas ist bereits Beobachter und pullt seine Sicht über die 2D-Lese-Naht; der Schreibpfad läuft port-frei über ein `ui/command/`-Objekt ([ADR-0019](0019-drw-2d-canvas.md) Entscheidung 5). **Es fehlt keine Kern-Funktion — es fehlt der Weg dorthin.**

**Siebzehn Fragen entscheidet der Spec-Text nicht.** Acht standen im Slice-Plan; sechs kamen aus vier unabhängigen Plan-Review-Läufen (Bauteil-Fang und Fehler-Barriere · Lese-Naht, Rückweg und Parameter-Umfang · Nachweis-Naht); **drei aus dem Text-Review dieser ADR** (Entscheidungen 15–17). Zwei der sechs standen wörtlich in Konsequenzen- bzw. Re-Eval-Blöcken der ADRs, auf denen dieser Strang aufbaut. Das ist der Grund, warum diese ADR ihre eigenen Konsequenzen so ausführlich führt: **dort steht das, was ein Entscheidungs-Block nicht sagt.**

**Und die drei aus dem Text-Review sind der Beleg dafür, dass die Verlagerung richtig war:** sie fehlten nicht, weil niemand nachgedacht hätte, sondern weil eine **andere** Entscheidung die Frage scheinbar schon beantwortet hatte — „die Id genügt" (11), „wie der Fang" (9), „die Senke fängt" (10) — jeweils nur für den halben Fall. Eine Liste von Frage-Überschriften kann das nicht zeigen; ein ausformulierter Entscheidungs-Text schon.

**Nicht offen** (bewusst außerhalb dieser ADR):

- **Räume, Türen, Fenster, Treppen, Dächer interaktiv** — der Trigger dieses Strangs nennt die **Wand**; alles andere ist eigener Schnitt.
- **Wand verschieben/teilen** ([LH-FA-WAL-004](../../../spec/lastenheft.md#lh-fa-wal-004)/005, reine Outline).
- **Layer-Bedien-Panel, Bemaßung, Schraffur/Gruppen** — weitere Re-Eval-Träger aus [ADR-0018](0018-drw-2d-zeichen-daten.md)/[ADR-0019](0019-drw-2d-canvas.md).
- **Raster und Winkelvorgaben** ([LH-FA-DRW-002](../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/003) — eigene Zeichen-Aids, hier nicht mitentschieden; **nur** der Endpunkt-Fang wird auf Bauteile ausgedehnt (Entscheidung 9).
- **Exakte Widget-/Signatur-/Konstanten-Gestalt** (Klassennamen, Toleranz-Werte, Panel-Layout) legen die Impl-Slices fest.

## Entscheidung

### 1. Werkzeug-Modus — ein benannter, sichtbarer Modus im Fenster; Default bleibt „Hilfslinie"

Der Canvas führt einen **Werkzeug-Modus** (Hilfslinie · Wand · Auswahl), umgeschaltet über eine **`QAction`-Gruppe im Fenster**. Die Geste selbst bleibt, was sie ist — Links-Zug —; **der Modus entscheidet, was daraus entsteht.** Default ist **Hilfslinie**, damit die bestehende Bedienung unverändert weiterläuft.

**Der Modus ist UI-Zustand** (Widget-Eigenschaft), kein Modell-Datum, nicht persistiert. Er ist **keine** Erfüllung von [LH-FA-UI-005](../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) („Anpassbare Werkzeugleisten", Outline): es gibt drei feste Werkzeuge, keine Anpassbarkeit.

### 2. Zeichen-Geste — ein Segment je Zug, mit ausgeschriebener Teilumfang-Klausel

Ein Links-Zug im Wand-Modus erzeugt **genau eine** Wand (Press = Anfang, Release = Ende) — dieselbe Bauform wie die Hilfslinie.

**Das unterschreitet den geltenden Happy Path** von [LH-FA-WAL-001](../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) („Linienzug mit ≥ 2 Punkten … je Segment eine Wand") und **wird deshalb als benannte Teilumfang-Klausel ins Lastenheft geschrieben**, nicht stillschweigend geliefert. Der Klausel-Text nennt die Eigenschaft („in dieser Ausbaustufe ein Segment je Zeichen-Geste"), nicht den Slice.

**Nicht** entschieden wird damit über das Domänen-Modell: „je Segment eine Wand" **ist** der bestehende Einzelsegment-Typ, ein Linienzug mit *n* Punkten bildet sich auf *n−1* Port-Aufrufe ab. Die `spezifikation.md`-§2.1-Klausel „Wandzüge/Polylines folgen als Erweiterung" betrifft die **Wand-Entität**, nicht die Geste, und **bleibt unverändert stehen**.

### 3. Selektion — UI-Zustand, Bildschirmraum-Treffer, genau eine Wand; 2D und 3D bleiben getrennt

Die Auswahl ist **Zustand des Canvas**, **nicht** des Gebäudemodells: kein Schema-Feld, keine Persistenz, kein Round-Trip. Getroffen wird im **Bildschirmraum** mit einer px-Toleranz (wie der Fang — damit ist die Trefferfläche zoom-unabhängig und bedienbar); ausgewählt ist **genau eine** Wand oder keine. Bei mehreren Treffern gewinnt der **nächstgelegene**; bei gleicher Distanz der in der festen Iterationsreihenfolge **zuerst besuchte** — dieselbe Regel, die der Fang trägt, aus demselben Grund (Determinismus statt Zufall).

**Die 3D-Sicht bleibt unberührt:** es gibt **keine** gemeinsame Auswahl zwischen 2D und 3D. Das ist eine benannte Grenze — eine geteilte Auswahl zöge den [ADR-0009](0009-gui-framework-qt6.md)-Re-Eval („Selektion/Picking im Viewport → AIS/V3d neu bewerten, **als Supersedes-ADR**") und ist damit ausdrücklich **nicht** dieser Schnitt.

### 4. Parameter-Bedienung — ein nicht-modaler Eigenschaften-Bereich im Fenster

Die Parameter der ausgewählten Wand werden in einem **festen, nicht-modalen Bereich des Fensters** angezeigt und geändert. Kein modaler Dialog: [LH-FA-WAL-002](../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) sagt „Geometrie und 3D-Körper aktualisieren sich **sofort**" zu — ein Dialog macht die Änderung erst beim Schließen wirksam und **unterbricht die Bedienung bei jedem Wert**.

**Wo die „sofort"-Zusage beobachtbar ist, wird hier ausgeschrieben** — sie hat auf der 2D-Fläche selbst **kein** Korrelat: der Canvas zeichnet Wand-**Achsen**, und eine Stärken-Änderung bewegt keine Achse. Beobachtbar ist sie (a) am **übernommenen Wert** im Eigenschaften-Bereich (inklusive Klemmung, Entscheidung 5), (b) am **3D-Körper** im anderen Reiter, dessen Beobachter-Pfad seit dem Viewer steht, und (c) im **Export**. Die AK-Formulierung muss sich auf diese Korrelate stützen; ohne diesen Satz wäre der vorgeschriebene interaktive Konjunkt beobachtungslos.

Das ist **keine** Erfüllung von [LH-FA-UI-001](../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) („Docking-Fenster", Outline): ein fester Bereich, keine Andock-Verwaltung.

### 5. Rückmeldung — eine nicht-modale Hinweis-Zeile, für **alle drei** Ausgänge

Drei Ausgänge verlangen heute eine sichtbare Reaktion, und **keiner** hat eine. Alle drei bekommen dieselbe Naht — eine nicht-modale Hinweis-Zeile im Fenster:

| Ausgang | Was der Benutzer sieht |
|---|---|
| `ParamStatus::Clamped` ([LH-FA-WAL-002](../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003: „geklemmt **+ Hinweis**") | Hinweis mit dem **tatsächlich übernommenen** Wert; das Eingabefeld zeigt den geklemmten Wert, nicht die Eingabe |
| `ParamStatus::Rejected` ([`E-VAL-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Ablehnungs-Lesart) | Hinweis „unverändert" — das Modell hat sich nicht bewegt |
| Null-Längen-Wand verworfen ([LH-FA-WAL-001](../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) Boundary: „verworfen, **Hinweis**") | Hinweis „keine Wand angelegt" |
| **Geometrie-Fehlschlag** ([`E-GEO-002`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) — der Kern rechnet Solid und Nachbar-Ecken **vor** dem Commit; scheitert das, bleibt das Modell unverändert | Hinweis „nicht angelegt, Modell unverändert" — **derselbe** Weg, denn für den Benutzer ist es dieselbe Beobachtung |

**Kein neuer Fehler-Code**; die Vokabeln bleiben die der Spezifikation. **Kein modaler Dialog** — eine Klemmung beim Tippen darf den Zeichenfluss nicht unterbrechen.

### 6. Refresh — ausschließlich über die `op`-Meldung; **kein** Selbst-Refresh nach dem eigenen Wand-Kommando

`addWall` meldet **nach** allen Post-Commit-Schritten (Nachbar-Rebuild, Raum-Neuerkennung) — der Canvas ist bereits Beobachter und rahmt darauf neu ein. Ein zusätzlicher Selbst-Repaint nach dem eigenen Kommando liefe **zusätzlich** zu dieser Kette und rahmte zweimal neu ein: ein sichtbarer Sprung.

**Die Asymmetrie zu den Zeichen-Daten ist gewollt und wird benannt:** Hilfslinien melden **keinen** `op` ([ADR-0018](0018-drw-2d-zeichen-daten.md) §2, unrevidiert), dort **ist** der Selbst-Refresh die einzige Quelle. Bauteile melden einen — dort ist er einer zu viel. **Dieselbe Geste, zwei Refresh-Wege, aus einem benennbaren Grund.**

### 7. Schreibpfad — die analoge `ui/command/`-Senke am `EditStructurePort` (Feststellung, keine neue Entscheidung)

Der Wand-Schreibpfad entsteht als **`ui/command/`-Senke** über dem `EditStructurePort`, port-frei in den Canvas verdrahtet — genau die Bauform, die [ADR-0019](0019-drw-2d-canvas.md) Entscheidung 5 für den ersten UI-Mutator festgelegt hat. Das `view/`-Widget sieht **keinen** Driving-Port; die Kanten-Allowlist (`ui_command → ports_driving` erlaubt, `ui_view → ports_driving` nicht) erzwingt es maschinell.

**Diese ADR stellt das fest, sie entscheidet es nicht** — sie sagt es, weil eine unausgesprochene Selbstverständlichkeit in einem Impl-Slice zur Abkürzung wird.

### 8. `E-GEO-001` „Eingabe außerhalb des Zeichenbereichs" — auf dem interaktiven Weg **nicht erreichbar**, und das wird geschrieben

Ein begrenzter Zeichenbereich in Modell-mm existiert nicht, und diese ADR erfindet keinen. Der Zoom ist geklemmt, ein interaktives Pan gibt es nicht — **jede** Bildschirmposition bildet auf endliche mm ab. Die Negative-AK von [LH-FA-WAL-001](../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) wird deshalb als **in dieser Ausbaustufe nicht erreichbar** benannt (Reifephase-Klausel im Lastenheft); der Fehler-Code bleibt für den **Import**-Pfad und spätere Eingabearten gültig.

**Was nicht geht, ist das Dritte:** die AK-Zeile stehen lassen und nichts dazu bauen.

### 9. Fangen gilt auch für die Wand-Geste — und die Teilumfang-Zeile wird nachgezogen

Der **Endpunkt-Fang** (sichtbare Wand-Achsen und Hilfslinien) wirkt auf den Wand-Zug wie auf den Hilfslinien-Zug. [LH-FA-DRW-001](../../../spec/lastenheft.md#lh-fa-drw-001) führt „Fangen beim Bauteil-Zeichnen" heute als **offen**; die Zeile wird mit dieser Entscheidung eingelöst.

**Ohne den Fang wäre der Eckenschluss auf dem interaktiven Weg praktisch unerreichbar.** [LH-FA-WAL-006](../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) setzt einen **gemeinsamen** Endpunkt voraus, dessen Toleranz die Spezifikation auf **0,1 mm** festlegt — während ein Bildschirm-Pixel bei Default-Zoom rund **20 mm** entspricht. Zwei Wände von Hand zu einer geschlossenen Ecke zu ziehen wäre Glückssache; die zentrale Bauteil-Zusage des Produkts hinge an der Zoomstufe.

**Raster und Winkelvorgaben bleiben offen** — ausgedehnt wird **nur** der Endpunkt-Fang.

### 10. Fehler-Barriere — die `ui/command/`-Senke fängt die Würfe des Ports

`addWall` wirft bei unbekannter Geschoss-Id, `setWallThickness`/`setWallHeight` werfen bei unbekannter Wand-Id, und die **Geometrie-Berechnung vor dem Commit** kann ebenfalls werfen ([`E-GEO-002`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), transaktional — das Modell bleibt dabei unverändert). Der erste UI-Mutator kannte das nicht: `addGuideLine` lehnt **wertbasiert** ab. Der Fall ist real erreichbar — die Zeichen-Ziele sind injizierte Ids, die nach einem Projekt-Wechsel neu aufgelöst werden.

**Die Senke ist die Barriere:** sie fängt den Wurf und meldet ihn über die Hinweis-Zeile (Entscheidung 5), statt ihn aus einem Qt-Event-Handler laufen zu lassen. **Kein neuer Fehler-Code, kein Umbau des Port-Vertrags** — den wertbasiert umzuschreiben beträfe **jede** bestehende Aufrufstelle des Ports quer durch Produktion und Testbestand und gehört, wenn überhaupt, in einen eigenen Schnitt (Re-Eval). *(Eine konkrete Aufrufer-Zahl stand hier in der ersten Fassung und war am Artefakt nicht reproduzierbar; sie ist entfernt statt korrigiert — die Begründung trägt ohne sie.)*

### 11. Die 2D-Lese-Naht bekommt eine Bauteil-Identität — additiv, im Kern, mit den Export-Golden als Netz

`model::PlanSegment` trägt heute vier Koordinaten und **keine Herkunft**; `projectPlan` mischt Wand-Achsen und sichtbare Hilfslinien in **eine** Liste. Damit hätte eine Treffer-Prüfung nur **anonyme** Segmente, eine Parameter-Änderung keine `WallId`, und die Wand-Zusage wäre nicht als **Wand** beobachtbar.

**Entscheidung: das Segment bekommt eine optionale Herkunft** (Art + Id) — **additiv** im Kern-Werttyp, den die Lese-Naht ohnehin liefert. [ADR-0019](0019-drw-2d-canvas.md) Entscheidung 2 hat diese Naht als **eine** Quelle für Bildschirm **und** Export etabliert; sie bleibt es.

**Die Erweiterung ist ein eigener, vorgelagerter Refactor-Schnitt** — ohne UI-Anteil, mit den 2D-Export-Decode-Orakeln und den **Export-Golden** als Sicherheitsnetz: bleiben die sechs Golden **byte-identisch**, ist bewiesen, dass die Erweiterung additiv ist und kein Encoder sie sieht. Präzedenz ist die Hebung der Projektion selbst, die ebenfalls **vor** dem Canvas lief.

### 12. Rückweg — Gesten-Abbruch ja, Löschen nein; die Grenze wird ausgeschrieben

Eine **laufende** Geste bricht ab (Escape, Fokusverlust) **ohne** Modell-Mutation. Eine **committete** Wand ist in der Oberfläche dieser Ausbaustufe **nicht** rücknehmbar: `EditStructurePort` hat kein `removeWall`, und [LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo) (Undo/Redo, ≥ 1000 Schritte) ist eine **Bestandslücke** — der Undo-Stack existiert im Schema ([ADR-0006](0006-relationales-schema-design.md)), kein Mutations-Pfad bedient ihn.

**Das ist die schärfste Grenze dieses Strangs, und sie steht hier, statt entdeckt zu werden.** Sie wird **nicht** durch ein hastiges `removeWall` geheilt: Löschen ist eine eigene Anforderung mit eigener AK, und ein Löschpfad ohne Undo verschöbe das Problem nur. Was den Benutzer schützt, ist die bestehende Rückfrage vor Datenverlust — ungesicherte Änderungen gehen nicht still verloren.

### 13. „Parametrisch änderbar" heißt **Stärke und Höhe** — Wandtyp und Material bleiben draußen

Änderbar sind die zwei Parameter, die auf AK-Niveau stehen und die der Port klemmend anbietet: **Stärke** ([LH-FA-WAL-002](../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), 50–1000 mm) und **Höhe** ([LH-FA-WAL-003](../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), 500–10000 mm).

**Nicht** dabei: **Wandtyp** ([LH-FA-WAL-007](../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen)) — reine Outline ohne AK; und **Material** — eigene Anforderungsfamilie mit eigener Auswertungs-Bindung. Beide interaktiv zu bedienen hieße, **fremde Anforderungen in einem UI-Strang auf AK-Niveau zu schärfen**; das gehört in ihre eigenen Schnitte.

### 14. Nachweis-Naht für reinen UI-Zustand — Surrogat plus Tinten-Sonde; `tests/e2e/` bleibt leer, und diese ADR sagt warum

Werkzeug-Modus (1), Auswahl (3) und Hinweis (5) haben **kein Korrelat außerhalb des Widgets**: nicht persistiert, nicht exportiert, nicht im Gebäudemodell. Die Interaktions-AK dieses Repos hingen bis zuletzt an einem **Modell**-Surrogat, das über Persistenz und Export weiter beobachtbar ist.

**Die Fang-Anzeige war der erste Fall dieser Art — und sie hat die Antwort bereits geprägt.** Dort hing die Zusage ebenfalls an reinem Widget-Zustand, und es brauchte **zwei** Review-Läufe, bis der zweite Sensor stand. Diese ADR **erbt** das Muster, statt es neu zu erfinden; sie stellt es nur erstmals für einen **ganzen Strang** fest, statt für eine einzelne Anzeige.

**[ADR-0009](0009-gui-framework-qt6.md) (f) und [ADR-0019](0019-drw-2d-canvas.md) Entscheidung 7 halten `tests/e2e/` genau bis zu diesem Punkt leer** — „bis eine AK entsteht, die nur echt end-to-end prüfbar ist (**z. B. Selektion / mehrschrittige Interaktion**)". Dieser Strang liefert beide dort genannten Beispiele. **Der Auslöser tritt also ein — und wird hier beantwortet, statt unterstellt:**

- **Welcher Zustand:** Werkzeug-Modus, ausgewählte Wand-Id und letzter Hinweis werden als **lesbare Widget-Eigenschaften** exponiert (Muster der bestehenden display-freien Nähe). Damit bleibt die AK auf der **Widget-Ebene** prüfbar — mit synthetisierten Ereignissen, ohne GUI-Treiber.
- **Dass etwas erscheint:** wo eine Zusage am **Erscheinen** hängt (Hinweis sichtbar, Auswahl hervorgehoben), kommt die **Tinten-Sonde auf dem offscreen gerenderten Widget** dazu — dieselbe zweite Nachweis-Ebene, die die Fang-Anzeige etabliert hat. Ein Surrogat ohne Produktions-Konsumenten bliebe sonst grün, während im Produkt nichts zu sehen ist.

**`tests/e2e/` bleibt damit leer** — nicht aus Trägheit, sondern weil die zwei Ebenen zusammen die Zusagen tragen. Es füllt sich, wenn eine Zusage entsteht, die **keine** von beiden erreicht (Re-Eval).

### 15. Die Wand-Parameter kommen über eine **eigene, eng geschnittene Detail-Abfrage** der 2D-Lese-Naht — nicht über `PlanSegment` und nicht über das `Building`

Entscheidung 4 sagt zu, die Parameter der ausgewählten Wand **anzuzeigen**. **Kein Driving-Port liefert sie heute:** die 2D-Lese-Naht trägt vier Koordinaten, der 3D-Read Netze, der Auswertungs-Port Flächen und Material, und `EditStructurePort` hat nur Mutatoren. Entscheidung 11 ergänzt ausdrücklich **nur Art und Id**.

**Entscheidung: die 2D-Lese-Naht bekommt eine zweite, schmale Abfrage** — zu einer `WallId` die **änderbaren** Parameter (Stärke, Höhe) als purer Werttyp. Eng geschnitten heißt: **genau das, was Entscheidung 13 änderbar macht** — kein Bauteil-Abbild, kein Material, keine Öffnungen.

- **Nicht in `PlanSegment`:** dieser Werttyp reist im Ableitungs-Bündel zu den Export-Adaptern. Ihn um Parameter zu erweitern, die **nur** ein Panel braucht, wäre genau das Breit-Werden, das [ADR-0020](0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) als Re-Eval führt. Die Herkunft (11) gehört ins Segment, weil das **Bild** sie braucht; die Parameter nicht.
- **Nicht als `const Building&` in die UI:** das Repo praktiziert diesen Weg für den Projekt-Handler (er **vergleicht** ganze Stände), aber für zwei Zahlen wäre es das Aushebeln der Lese-Naht, das Entscheidung 11 für den Treffer-Fall ausdrücklich verwirft. **Dieselbe Begründung muss hier gelten, sonst ist sie dort Rhetorik.**

### 16. Die Treffer-Prüfung ist auf das **dargestellte** Geschoss beschränkt — anders als der Fang, und mit Grund

Die 2D-Lese-Naht trägt **alle** Geschosse, gezeichnet wird nur das aktive. Übernähme die Auswahl die Iteration des Fangs unverändert, wählte sie deterministisch die **erste** Wand in Speicherreihenfolge — bei deckungsgleichen Achsen also eine, die der Benutzer **nicht sieht**. Das ist kein Randfall: das mitgelieferte Demo-Modell legt in Erd- und Obergeschoss **dieselben vier Außenwände** an; jeder Klick auf eine sichtbare Obergeschoss-Wand träfe die unsichtbare des Erdgeschosses — die Entscheidung 4 dann anzeigt und Entscheidung 13 ändert.

**Der Fang bleibt geschoss-übergreifend, die Auswahl wird es nicht.** Die Asymmetrie ist gewollt und benennbar: **Fangen ist eine Aussage über eine Koordinate** — ein Punkt ist ein Punkt, gleich aus welchem Geschoss, und die Spezifikation verlangt es ausdrücklich so. **Auswählen ist eine Aussage über ein Ding, auf das der Benutzer zeigt** — und er kann nur auf zeigen, was er sieht.

**Restlücke, benannt:** der geschoss-übergreifende Fang (Entscheidung 9) kann einen Punkt liefern, an dem im **aktiven** Geschoss keine zweite Wand endet — dann entsteht kein Eckenschluss ([LH-FA-WAL-006](../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) verlangt einen gemeinsamen Endpunkt **im selben Geschoss**). Das ist **kein Fehler**, sondern die Folge davon, dass Fangen Koordinaten liefert und keine Nachbarschaft verspricht; die Fang-Anzeige macht sichtbar, worauf gerastet wurde.

### 17. Die Auswahl fällt, sobald ihr Bezug fallen könnte — Modell-Ersetzung, Geschoss-Wechsel, Verschwinden

Eine Auswahl ist eine **`WallId` im Widget** — eine Identität, die außerhalb des Widgets vergeben wird. Bleibt sie über eine **Modell-Ersetzung** stehen (Projekt öffnen, neues Projekt), bezeichnet sie im neuen Stand mit hoher Wahrscheinlichkeit eine **andere, existierende** Wand: der Id-Zähler wird beim Laden auf das Maximum des geladenen Projekts gesetzt, und ein geladenes Projekt trägt dichte eigene Ids. Die Fehler-Barriere (10) schwiege dabei — `setWallThickness` **wirft nicht** bei einer gültigen fremden Id. **Die Mutation träfe still die falsche Wand.**

**Entscheidung: die Auswahl wird verworfen** bei (a) jeder Modell-Ersetzung, (b) jedem Geschoss-Wechsel (folgt aus 16 — im neuen Geschoss ist sie nicht mehr sichtbar) und (c) wenn die gewählte Wand nicht mehr in der Sicht vorkommt. Der Eigenschaften-Bereich zeigt dann „keine Auswahl", statt Werte einer Wand zu zeigen, die es so nicht mehr gibt.

**Das ist die dritte Ausprägung derselben Regel in diesem Canvas, und sie ist jedes Mal anders ausgefallen** — der Zug-Startpunkt in mm **muss überleben** (eine Transformations-Änderung darf ihn nicht bewegen), die Fang-Anzeige in mm **muss fallen** (nach einer Transformations-Änderung zeigte sie auf die falsche Bildschirmstelle), und eine Identität **muss fallen**, sobald der Bezugsrahmen ersetzt wird. **UI-Zustand, der auf Fremdes verweist, braucht eine ausgesprochene Lebensdauer** — sonst ist sie zufällig.

## Verglichene Alternativen

### Zu 1: Werkzeug-Modus (gewählt) vs. Modifikator-Taste vs. getrennte Flächen

- **Benannter Modus (gewählt) — Pro:** sichtbar und auffindbar (eine Bedienung, die man nicht sieht, existiert für den Benutzer nicht); CAD-Konvention; als Widget-Eigenschaft **prüfbar**; erweiterbar um spätere Werkzeuge. **Contra:** ein Modus-Zustand mehr.
- **Modifikator-Taste (z. B. Umschalt-Zug = Wand) — Contra (entscheidend):** unsichtbar und undokumentierbar außer im Handbuch; kollidiert mit späteren Aids; kein Zustand, den eine AK benennen könnte. **Verworfen.**
- **Getrennte Zeichenflächen je Bauteil-Art — Contra:** dupliziert Sicht, Transformation und Fang-Zustand; der Benutzer verlöre den gemeinsamen Bildbezug. **Verworfen.**

### Zu 2: Ein Segment je Zug (gewählt) vs. mehrpunktiger Zug in einem Rutsch

- **Ein Segment (gewählt) — Pro:** dieselbe Geste wie die bestehende, also nichts Neues zu lernen und nichts Neues zu prüfen; der Trigger dieses Strangs verlangt „**eine** Wand"; **drei** Folgefragen entfallen (Teilerfolg eines Zugs, Abbruch-Semantik mitten in der Kette, Neu-Einrahmen während der Geste). **Contra:** unterschreitet den geltenden Happy Path — deshalb die Teilumfang-Klausel, ausgeschrieben statt stillschweigend.
- **Mehrpunktiger Zug — Pro:** erfüllt die AK wörtlich, bedienfreundlicher für lange Wandzüge. **Contra (entscheidend für v1):** ein abgelehntes Zwischen-Segment hinterließe einen **halben Wandzug** — und ohne Rückweg (Entscheidung 12) ist das ein Zustand, aus dem der Benutzer nicht herauskommt. **Verworfen für v1**, als Re-Eval geführt.

### Zu 3: Auswahl als UI-Zustand (gewählt) vs. Modell-Zustand

- **UI-Zustand (gewählt) — Pro:** Auswahl ist keine Eigenschaft des **Gebäudes**; kein Schema, keine Persistenz, kein Round-Trip, keine Konflikt-Semantik zwischen zwei Sichten. **Contra:** zwei Sichten haben zwei Auswahlen (benannt).
- **Modell-Zustand — Contra (entscheidend):** erzwänge ein Schema-Feld, einen Persistenz-Vertrag und die Frage, was eine geladene Auswahl bedeutet; und machte aus einer Bedienfrage eine Datenfrage. **Verworfen.**

### Zu 6: Nur `op` (gewählt) vs. zusätzlicher Selbst-Refresh

- **Nur `op` (gewählt) — Pro:** eine Quelle, eine Reihenfolge; die Meldung kommt garantiert **nach** Nachbar-Rebuild und Raum-Neuerkennung. **Contra:** der Canvas hängt für sein **eigenes** Kommando an der Benachrichtigungs-Kette (die es für Bauteile aber gibt).
- **Zusätzlicher Selbst-Refresh (wie bei Hilfslinien) — Contra (entscheidend):** zwei Neu-Einrahmungen je Mutation, davon eine vor Abschluss der Post-Commit-Schritte → sichtbarer Sprung. **Verworfen.**

### Zu 11: Herkunft im Segment (gewählt) vs. eigene Treffer-Abfrage vs. `Building` direkt lesen

- **Herkunft im Segment (gewählt) — Pro:** **eine** Quelle für Bild und Treffer (dieselbe Begründung, mit der die Projektion überhaupt vereinheitlicht wurde); additiv; die Sichtbarkeits-Filterung bleibt an einer Stelle; die Export-Golden beweisen die Additivität. **Contra:** ein Kern-Werttyp wächst — Export-Pfad und Golden sind mit-betroffen und brauchen den vorgelagerten Schnitt.
- **Eigene Treffer-Abfrage neben der Projektion — Contra (entscheidend):** zwei Quellen für dasselbe Bild driften; die Sichtbarkeits-Filterung existierte doppelt — genau das Duplikat-Risiko, das die Vereinheitlichung beseitigt hat. **Verworfen.**
- **Canvas liest `Building` direkt — Contra:** hebelt die Lese-Naht aus und bindet die UI an das Domänen-Objekt statt an eine Sicht. **Verworfen.**

### Zu 14: Surrogat + Tinten-Sonde (gewählt) vs. e2e-GUI-Treiber

- **Zwei Ebenen auf Widget-Ebene (gewählt) — Pro:** bleibt in der etablierten Infrastruktur (synthetisierte Ereignisse, headless); die zweite Ebene deckt genau das, was der Surrogat strukturell nicht sehen kann; keine neue Test-Technologie. **Contra:** die Sonde belegt „es erscheint etwas", nicht „es sieht richtig aus" — bewusst, denn das Aussehen ist **keine** Zusage.
- **e2e-GUI-Treiber — Pro:** prüfte die echte Kette. **Contra (entscheidend):** neue Technologie und neue Flakiness-Quelle für Zusagen, die zwei vorhandene Ebenen tragen; der von [ADR-0009](0009-gui-framework-qt6.md) (f) genannte Auslöser fordert eine **Antwort**, nicht zwingend einen Treiber. **Verworfen — mit ausdrücklichem Re-Eval**, falls eine Zusage entsteht, die keine der zwei Ebenen erreicht.

### Zu 15: schmale Detail-Abfrage (gewählt) vs. Parameter ins Segment vs. `Building` in die UI

- **Schmale Abfrage (gewählt) — Pro:** die Lese-Naht bleibt die eine Quelle der UI; der Umfang ist durch Entscheidung 13 **begrenzt** und wächst nicht mit jedem Panel-Wunsch; das Export-Bündel bleibt unberührt. **Contra:** eine zweite Abfrage an derselben Naht.
- **Parameter in `PlanSegment` — Contra (entscheidend):** der Werttyp reist zu den Export-Adaptern; Felder, die nur ein Panel braucht, machen ihn genau so breit, wie es [ADR-0020](0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) als Re-Eval-Anlass führt. **Verworfen.**
- **`const Building&` in die UI — Contra (entscheidend):** hebelt die Lese-Naht aus — dieselbe Begründung, mit der Entscheidung 11 den direkten `Building`-Zugriff für den Treffer-Fall verwirft. Sie hier nicht anzuwenden, machte sie dort zur Rhetorik. **Verworfen.**

### Zu 16: Auswahl nur im dargestellten Geschoss (gewählt) vs. Iteration des Fangs übernehmen

- **Beschränkt (gewählt) — Pro:** der Benutzer wählt, was er sieht; im Demo-Modell ist der Gegenfall **real** (vier deckungsgleiche Achsen in zwei Geschossen). **Contra:** eine Asymmetrie zum Fang, die erklärt werden muss — hier erklärt.
- **Iteration übernehmen — Pro:** eine Regel weniger. **Contra (entscheidend):** ein deterministisch **falscher** Treffer ist schlimmer als ein zufälliger, weil er reproduzierbar am Benutzer vorbeigeht. **Verworfen.**

### Zu 17: Auswahl verwerfen (gewählt) vs. über die Ersetzung retten

- **Verwerfen (gewählt) — Pro:** eine Id ohne ihren Bezugsrahmen ist bedeutungslos; das Verwerfen ist total und braucht keine Heuristik. **Contra:** der Benutzer verliert nach einem Projekt-Wechsel die Auswahl — was korrekt ist, weil es das Projekt auch tut.
- **Retten (z. B. über Koordinaten-Identität) — Contra (entscheidend):** verlangte eine Gleichheits-Heuristik über zwei Modelle und beantwortete die Frage „ist das dieselbe Wand?" ohne Not. **Verworfen.**

## Konsequenzen

- **Positiv:** Der interaktive Bauteil-Weg entsteht **ohne** neue Technologie, **ohne** neue Dependency, **ohne** neue Gate-Regel und **ohne** Bruch einer bestehenden Architektur-Entscheidung: ein Werkzeug-Modus im Fenster, eine zweite `ui/command/`-Senke am vorhandenen Port, eine **additive** Erweiterung der vorhandenen Lese-Naht. Die vom Handbuch selbst benannte Lücke — „ein Gebäude selbst planen" — wird für die **Wand** geschlossen.

- **[MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) ist für diesen Strang EINSCHLÄGIG — und das steht hier, weil die Vorgänger-ADR das Gegenteil festgestellt hat.** [ADR-0018](0018-drw-2d-zeichen-daten.md) erklärte das geometrielastige Code-Review ausdrücklich für **n/a**, mit Begründung: „Hilfslinie = 2 Punkte, **keine neue Solid-Geometrie**". Für Wände gilt exakt das Gegenteil: `addWall` baut ein Solid, rechnet **Nachbar-Ecken** neu ([LH-FA-WAL-006](../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)) und stößt die **Raum-Neuerkennung** an. Ohne diese Feststellung erbten die Impl-Slices stillschweigend eine Verneinung, die für sie falsch ist.

- **Bauteil-Ebenen bleiben unberührt — als Entscheidung, nicht als Auslassung.** Eine interaktiv gezeichnete Wand erbt **nicht** die aktive Zeichen-Ebene des Canvas; Wände tragen weiterhin **keine** Ebenen-Zuordnung. [ADR-0018](0018-drw-2d-zeichen-daten.md) führt „Layer-Zuordnung für Bauteile" als Re-Eval-Trigger, der die polymorphe `entity_layers`-Zuordnung aktivierte und **Layer cross-cutting** machte — eine Schema-Erweiterung, die dieser Strang nicht beschließt und **nicht stillschweigend auslösen darf**. Der Canvas hält beim Zeichnen eine aktive Ebene; ohne diesen Satz wäre die Vererbung eine naheliegende Abkürzung.

- **Entscheidung 11 kann einen fremden Re-Eval ziehen.** `PlanView` reist im `DerivedGeometry`-Bündel; [ADR-0020](0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) führt „**das Bündel wird zu breit** (viele optionale Felder je neuem Format)" als Anlass, das Varianten-/Bedarfs-Modell neu zu bewerten. Eine **einzelne** optionale Herkunft je Segment zieht ihn nicht; eine Folge weiterer Felder täte es. **Beobachtungspflicht, kein Beschluss.**

- **Negativ / Folgepflicht (Slices, Nummern im [ADR-Index](README.md)):** (a) **AK-Schärfungs-Slice** — [LH-FA-WAL-001](../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) um den Block „Interaktive Erzeugung (2D-Zeichenfläche)" + Teilumfang-Klausel (Entscheidung 2) + Erreichbarkeits-Klausel zur Negative (Entscheidung 8); [LH-FA-WAL-002](../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003 je ein interaktiver Konjunkt; [LH-FA-DRW-001](../../../spec/lastenheft.md#lh-fa-drw-001)-Teilumfang um den Bauteil-Fang (Entscheidung 9); [LH-FA-DRW-005](../../../spec/lastenheft.md#lh-fa-drw-005)-Teilumfang-Klausel nachziehen — sie führt **sowohl** „interaktives Zeichnen von Bauteilen" **als auch** „**Selektion**" als offen, und dieser Strang löst **beides** ein; **Spec-Nachzug** `spezifikation.md` §1 (Mapping der vierzehn Entscheidungen) und **§6** (die Canvas-Vertragszeile trägt „Selbst-Refresh **ohne** `op`" — mit Entscheidung 6 wird sie unvollständig); `architecture.md` §1.1 (Canvas-Klausel am `EditStructurePort` + erweiterte Lese-Naht), **meilenstein- und slice-frei**. (b) **Lese-Naht-Slice** (Entscheidung 11), **vorgelagert**, ohne UI-Anteil, Export-Golden als Netz. (c) **Zeichnen-Slice** (Entscheidungen 1, 2, 5, 6, 7, 8, 9, 10, 12, 14). (d) **Auswahl-/Änderungs-Slice** (Entscheidungen 3, 4, 5, 13, 14). (e) [MR-006](../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor jedem Impl-Start; [MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) vor der Welle-Closure (s. o.).

- **Keine neue a-check-Regel, keine gelockerte Allow-Liste.** Der Schreibpfad nutzt die bestehende Kante `ui_command → ports_driving`; das `view/`-Widget bleibt port-frei; der skalare Sink-Eintrag der lateralen Adapter-Regel bleibt unberührt. **[`AGENTS.md`](../../../AGENTS.md) §2.6 n/a** (nichts gelockert).

- **Kein Schema-Diff.** Auswahl und Werkzeug-Modus sind UI-Zustand (Entscheidungen 1/3); Entscheidung 13 führt keine neuen Felder; `entity_layers` und der Undo-Stack bleiben unbedient. `data-model.yaml`/`schema.sql` bleiben **byte-unberührt**.

- **[ADR-0019](0019-drw-2d-canvas.md) wird FORTGESCHRIEBEN, nicht ersetzt.** Ihre Entscheidungen 1–7 bleiben gültig; diese ADR löst genau die vier Punkte ein, die sie als Re-Eval führte (Bauteile zeichnen, Selektion/Picking — Layer-Panel und Bemaßung bleiben offen). **Kein `Supersedes`**; es gibt keinen Widerspruch, nur Erweiterung. Insbesondere bleibt Entscheidung 5 (Kommando-Naht) unverändert und wird in Entscheidung 7 nur angewandt.

- **Rest-Risiko benannt:** die **fehlende Rücknahme** einer committeten Wand (Entscheidung 12) ist die schärfste Grenze des Ergebnisses. Sie ist ehrlich beschrieben statt durch ein Löschen ohne Undo kaschiert — aber sie wird der erste Punkt sein, an dem ein Benutzer anstößt.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| a-check (Schicht-Kanten, laterale Adapter) | `view/` importiert **keinen** Driving-Port; der Wand-Schreibpfad liegt in `ui/command/`; **keine neue Kante** | `make a-check` |
| Interaktions-AK, Ebene 1 (Surrogat) | Werkzeug-Modus, Auswahl und letzter Hinweis sind display-frei lesbar; die Zusagen sind mit synthetisierten Ereignissen prüfbar, **ohne** GUI-Treiber | `make test` |
| Interaktions-AK, Ebene 2 (Tinten-Sonde) | wo eine Zusage am **Erscheinen** hängt, belegt ein offscreen gerendertes Widget-Bild mehr Farbe als ohne — bei **sonst gleichem** Modell und gleicher Abbildung | `make test` |
| Schema-Unberührtheit | **`git diff --stat` über `data-model.yaml`/`schema.sql` ist leer** — Auswahl und Modus erzeugen **kein** Schema-Feld. `make schema-check` ist hier **nicht** der Sensor: es prüft die **Drift zwischen** Modell und erzeugtem DDL (`schema.sql` ist das d-migrate-**Erzeugnis**), eine konsistent nachgezogene Spalte bliebe grün | `git diff --stat` (+ `make schema-check` als Drift-Wächter) |
| Additivität der Lese-Naht (Entscheidung 11) | die sechs Export-Golden bleiben **byte-identisch** — kein Encoder sieht die neue Herkunft | `make golden-check` |
| Geometrie-Korrektheit vor Closure | unabhängiges [MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review (Eckenschluss, Nachbar-Rebuild, Raum-Neuerkennung im interaktiven Pfad) | Review-Artefakt |

## Re-Evaluierungs-Trigger

- **Mehrpunktiger Wandzug** (Entscheidung 2) → Teilerfolg eines Zugs, Abbruch-Semantik mitten in der Kette und Neu-Einrahmen während der Geste **zusammen** entscheiden; zieht die Teilumfang-Klausel zurück.
- **Löschen und/oder Undo in der Oberfläche** (Entscheidung 12) → [LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo) aktivieren, den persistierten Undo-Stack aus [ADR-0006](0006-relationales-schema-design.md) bedienen und den Port um einen Entfernen-Weg erweitern. **Der wahrscheinlichste nächste Trigger.**
- **Bauteile auf Benutzer-Ebenen** → der [ADR-0018](0018-drw-2d-zeichen-daten.md)-Trigger („`entity_layers` aktivieren, Layer wird cross-cutting"); berührt Schema, Export und Sichtbarkeits-Semantik.
- **Wandtyp/Material interaktiv** (Entscheidung 13) → AK-Schärfung von [LH-FA-WAL-007](../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen) bzw. der Material-Familie **vor** der Bedienung.
- **Gemeinsame Auswahl 2D↔3D oder Selektion im 3D-Viewport** (Entscheidung 3) → der [ADR-0009](0009-gui-framework-qt6.md)-Trigger (AIS/V3d, **Supersedes-ADR**).
- **Mehrfach-Auswahl** (Entscheidung 3) → Auswahl-Modell, Bereichs-Geste und Sammel-Parameter-Änderung zusammen entscheiden.
- **Das `DerivedGeometry`-Bündel wird breit** (Entscheidung 11) → der [ADR-0020](0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Trigger (Varianten-/Bedarfs-Modell).
- **Eine Zusage entsteht, die weder Surrogat noch Tinten-Sonde erreicht** (Entscheidung 14) → `tests/e2e/` füllen, [ADR-0009](0009-gui-framework-qt6.md) (f) und [ADR-0019](0019-drw-2d-canvas.md) Entscheidung 7 fortschreiben.
- **Wandzüge als eigene Entität** (mehrsegmentige Wand im Domänen-Modell) → die `spezifikation.md`-§2.1-Klausel einlösen; berührt Persistenz, Export und Eckenschluss.

## Geschichte

| Datum | Ereignis |
|---|---|
| 2026-07-28 | Angelegt als `Proposed`. Vorarbeit: vier unabhängige Plan-Review-Läufe am Slice-Plan (2 HIGH / 1 HIGH / 1 HIGH / 0 HIGH), die die Fragenliste von acht auf vierzehn geführt haben — **F9/F10** (Bauteil-Fang, Fehler-Barriere), **F11/F12/F13** (Lese-Naht, Rückweg, Parameter-Umfang), **F14** (Nachweis-Naht). Danach eine **Lese-Runde** über die Konsequenzen-, Fitness-Function- und Re-Eval-Blöcke von [ADR-0009](0009-gui-framework-qt6.md)/[0018](0018-drw-2d-zeichen-daten.md)/[0019](0019-drw-2d-canvas.md)/[0020](0020-driven-adapter-serialisieren-kern-liefert-geometrie.md): **keine** weitere Entscheidung gefunden, aber drei Verpflichtungen ([MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Einschlägigkeit · Bauteil-Ebenen · [ADR-0020](0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Beobachtungspflicht), die jetzt in §Konsequenzen stehen. **F11 und F14 standen wörtlich in fremden Konsequenzen-/Re-Eval-Blöcken** — der Anlass für die Lese-Runde als Disziplin. |
| 2026-07-28 | **Accepted** nach unabhängigem Text-Review (**3 HIGH / 7 MEDIUM / 5 LOW / 6 INFO**). Die drei HIGH waren durchweg **fehlende** Entscheidungen — Lese-Naht für die anzuzeigenden Parameter, **Geschoss-Skopus** der Treffer-Prüfung, **Lebensdauer** der Auswahl — und alle drei fehlten, weil eine **andere** Entscheidung die Frage scheinbar schon beantwortet hatte, je nur für den halben Fall. Sie sind als Entscheidungen **15–17** aufgenommen. Das Review bestätigte ausdrücklich: **kein** Widerspruch zu einer Accepted-ADR (also **kein `Supersedes`**), die wörtlichen Zitate stimmen, die Code-Behauptungen sind korrekt, die 20-mm-Rechnung stimmt exakt, alle genannten Make-Targets existieren, und was die ADR ins Lastenheft verweist, ist lösungsfrei. Die sieben MEDIUM sind eingearbeitet — darunter eine falsch begründete Alleinstellung (die Fang-Anzeige war der erste Fall dieser Art), eine Fitness-Zeile, die einer Drift-Prüfung eine **Abwesenheits**-Zusage aufgeladen hatte, eine dritte Wurf-Quelle ([`E-GEO-002`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) und die fehlende Beobachtbarkeit der „sofort"-Zusage auf der 2D-Fläche (die Achse bewegt sich bei einer Stärken-Änderung nicht). |
