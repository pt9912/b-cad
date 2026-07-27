---
id: slice-052b
titel: „Neues Projekt" — [LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) benutzer-erfüllbar machen
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)]
adr_refs: [[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 052b: „Neues Projekt"

**Status:** open — **abhängig von [`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md)**
(Sitzungs-Zustand + Verdikt-Maschinerie). Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
**2026-07-26 gefahren: 1 HIGH / 11 MEDIUM / 5 LOW / 2 INFO → nicht startbar**
([Report](../../../reviews/2026-07-26-slice-052b-plan.md)); alle Findings eingearbeitet (§12).
**Ein zweiter Lauf vor dem Start.** Zusätzlich hängt die Startbarkeit an
[`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md) (Sitzungs-Zustand) und mittelbar an
[`slice-053`](../done/slice-053-fenster-als-adapter.md) (testbares Fenster).

**Welle:** welle-5-erweiterung. **Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) Lauf 1 + 2 zu slice-052)

Der Slice existiert wegen zweier Review-Befunde, nicht wegen einer Feature-Idee:

1. **[Lauf 1](../../../reviews/2026-07-26-slice-052-plan.md), HIGH-2:**
   [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) („Projekt anlegen")
   fordert die Ungesichert-Rückfrage bereits auf **AK-Niveau** — beim Auslöser „Neues Projekt". Die
   Nachprüfung ergab: **die Aktion existiert im Produkt überhaupt nicht** (`src/main.cpp` kennt sie
   nicht). Die Anforderung ist damit **nicht benutzer-erfüllbar** — exakt die Klasse, die slice-047 für
   [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)
   aufgearbeitet hat: **Mechanik ohne Aufruf-Pfad**.
2. **[Lauf 2](../../../reviews/2026-07-26-slice-052-plan-2.md), HIGH-1 + MEDIUM-11:** der Versuch, den
   Auslöser **nebenbei** in slice-052 mitzunehmen, hinterließ ein **Datenverlust-Loch** (s. §2) und
   sprengte den Schnitt. Projektinhaber-Entscheidung 2026-07-26: **Split**, damit der Auslöser seine
   eigene Zustands-, Anforderungs- und Doku-Arbeit bekommt statt sie zu verstecken.

## 1. Ziel

Der Benutzer kann in der Oberfläche ein **neues, leeres Projekt** anlegen —
[`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) wird erstmals
benutzer-erfüllbar. Der Slice **verhält sich zu allen drei** Akzeptanzkriterien (Happy Path, Boundary,
Negative) — zwei erfüllt er, die dritte **schärft** er (§3). Was er nicht tut: eine AK still stehen
lassen und sich als „erfüllt" buchen.

## 2. Die fünf Löcher, die dieser Slice schließen muss

Vier davon entstehen nicht durch das Anlegen, sondern durch das, was ein Projekt-Wechsel **mit der
Sitzung macht**; **L4** ist die Ausnahme — es ist die Mechanik des Anlegens selbst (Lauf-052b-LOW-1):

- **L1 — die gemerkte Datei (Lauf-2-HIGH-1, blockierend).** `markPersisted` läuft nach Öffnen und
  Speichern; **„Neu" ist keines von beidem**. Ohne eigene Behandlung zeigt `path()` nach „Neu" weiter
  auf das **zuvor geöffnete** Projekt — und „Speichern" schreibt per Zusage **dialoglos** dorthin. Das
  leere Neu-Projekt überschreibt das alte. Die Auflösung (Reset von Pfad **und** Vergleichs-Basis)
  gehört in die Kern-Naht, nicht in den Menü-Handler.
- **L2 — die eingefrorenen Zeichen-Ziel-Ids (Lauf-2-MEDIUM-5).** `EditDrawingGuideLineSink` hält
  Geschoss und Ebene **by value**; slice-047 hat die Neu-Auflösung genau deshalb aus `main` in den
  Use-Case verlegt (Verify-Finding **B4**). „Neu" ersetzt den Modellstand ebenso und braucht dieselbe
  Behandlung — sonst zeigen Canvas und Sink auf einen Stand, den es nicht mehr gibt.
- **L3 — das leere Projekt hat keine Zeichen-Ebene (Lauf-2-MEDIUM-5).** Ein Modellbaum ohne Ebene
  bedeutet: die **einzige** heute erreichbare Benutzer-Mutation (Hilfslinie zeichnen) wird abgelehnt.
  Der Slice erzeugte damit einen Zustand, in dem der Benutzer nichts tun kann. Ob das Neu-Projekt eine
  Ebene bekommt (und ob das mit „Öffnen ist lesend" aus slice-047 kollidiert) ist eine **Entscheidung,
  die dieser Plan treffen muss**, keine Nebenwirkung.

  > **Entschieden 2026-07-27 (Lauf-2-MEDIUM-6): das neue Projekt bekommt genau eine Zeichen-Ebene.**
  > Kein Widerspruch zu „Öffnen ist lesend": jene Regel schützt den **Inhalt einer fremden Datei**
  > davor, still ergänzt zu werden. Hier gibt es keine Datei — b-cad **definiert**, was ein neues
  > Projekt enthält, so wie es auch das eine Geschoss definiert (L4). Die Alternative („ohne Ebene,
  > dafür Meldung") liefert einen Zustand, in dem die **einzige** heute erreichbare Benutzer-Mutation
  > unmöglich ist; ein Slice, der eine Anforderung erstmals benutzer-erfüllbar macht, darf nicht mit
  > einer Sackgasse enden. Die Ebene gehört damit in den **Happy-AK-Wortlaut** (§4), nicht in eine
  > Implementierungs-Fußnote.
- **L4 — die Projekt-Erzeugung ist Mechanik (Lauf-2-MEDIUM-6b).** „Leeres Projekt mit genau **einem**
  Geschoss, Default-Höhe aus der Spezifikation" ist eine fachliche Regel. Sie gehört **nicht** in den
  orakel-losen Composition-Root — dieselbe Auflage wie in slice-047: „Was in `main` bleiben darf, ist
  Dialog, Meldung, Verdrahtung — keine Entscheidung."

- **L5 — der Fenstertitel bleibt auf der alten Datei (eigener Lauf, MEDIUM-8).** slice-047 schreibt den
  Dateinamen beim Öffnen in den Titel (`main.cpp`:308–309), der Start setzt ihn auf „b-cad" (:483). Nach
  „Neu" behauptet die Oberfläche also weiterhin, das zuvor geöffnete Projekt zu zeigen — dieselbe
  Fehl-Aussage wie L1, nur sichtbar statt folgenschwer. Gehört zur selben Reset-Zusage.

## 2.1 Wo „Neu" liegt — entschieden 2026-07-27 (Lauf-2-HIGH-1)

Der Plan-Körper stammt vom 2026-07-26 und wurde seither **nicht** nachgezogen: `Port`,
`ManageProject`, `ProjectMenuHandler`, `FileActions`, `CloseGuard` und `slice-054` hatten darin **null**
Fundstellen — obwohl alle drei Vorläufer inzwischen `done` sind. Das ist keine Formalie: unter
`.a-check.yml` darf **kein** Adapter `hexagon/services/` rufen. Weder die Aktion „Neu" noch der
L1-Reset wären ohne Port aus einem Adapter erreichbar; übrig bliebe der **sensorlose `main.cpp`** —
bei einem **Datenverlust-Pfad** (R1). Das wäre das sechste Auftreten derselben Klasse.

**Entscheidung (Projektinhaber 2026-07-27): beide bestehenden Ports werden um je eine Methode
erweitert** — kein dritter Port, keine neue Schicht-Kante.

| Port | Ergänzung | Warum dort |
|---|---|---|
| `ManageProjectPort` | `newProject()` | **slice-054 hat die Stelle ausdrücklich vorgemerkt** („‚Neues Projekt' ([`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)) kommt mit slice-052b"). `architecture.md` führt anlegen/speichern/laden/versionieren als **eine** Familie |
| `ProjectSessionPort` | `reset(baseline)` | `markPersisted(path, building)` taugt **nicht**: es **verlangt** einen Pfad, den ein neues Projekt nicht hat — und genau das Weiterzeigen des alten Pfads ist L1 |

**Der Aufrufer ist der `ProjectMenuHandler`** (`ui/command/`), nicht `main`: `newProject()` läuft dort
hinter demselben `mayDiscard(ask, ask_target)`, das
[`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md) für Öffnen und Schließen gebaut
hat — die Rückfrage-Kette ist damit **schon geprüft** und wird nicht ein zweites Mal geschrieben.
`main.cpp` liefert nur den Dialog.

**Nachzuziehen ist damit auch `spec/architecture.md`** (Lauf-2-MEDIUM-3): dort steht heute begründet,
„anlegen" sei **nicht** im `ManageProjectPort`-Vertrag. Mit `newProject()` wird die Aussage falsch —
sie ist Teil dieses Slice, nicht Nebenwirkung.

## 3. „EG" ist keine Modell-Eigenschaft (MR-006-HIGH-1)

Die Happy-AK verlangt „ein leeres Projekt mit genau einem Geschoss (**EG**, Default-Höhe …)".
`src/hexagon/model/storey.h` kennt aber nur `id` und `height_mm` — **kein Bezeichnungsfeld**. Die
Persistenz erfindet den Namen beim Schreiben und liest ihn nie zurück; im Modell existiert er nicht.

**Entscheidung (Projektinhaber 2026-07-26): „EG" ist beschreibend, kein Datum.** Gemeint ist das
**erste/unterste** Geschoss eines frischen Projekts. Die AK wird entsprechend **geschärft**
(lösungsfrei, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)),
damit sie kein Feld verlangt, das es nicht gibt — **kein** Schema-Eingriff, **kein** neues Modell-Feld.

**Nebenbefund, benannt statt behoben:** dass die Persistenz einen Geschoss-Namen **schreibt**, den
niemand liest, ist eine eigenständige Inkonsistenz. Sie gehört **nicht** in diesen Slice (sie ist älter
und betrifft das Schema), wird aber hier aktenkundig — Muster der „benannten Lücke".

## 4. Anforderungs-Ebene

[`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) trägt **drei** AK. Der Slice verhält sich zu **allen**
(Lauf-2-MEDIUM-7) — zwei erfüllt er, eine schärft er:

| AK | Wortlaut (gekürzt) | Behandlung |
|---|---|---|
| **Happy Path** | „ein leeres Projekt mit genau einem Geschoss (EG, Default-Höhe) und einem leeren Modellbaum" | **erfüllen** (L4) + **schärfen** („EG" → beschreibend, §3; und der Modellbaum-Wortlaut, s. u.) |
| **Boundary** | „Given ein bereits geöffnetes, ungespeichertes Projekt, when „Neues Projekt", then Rückfrage „Änderungen verwerfen?"" | **erfüllen** über die 052a-Verdikt-Maschinerie; Wertigkeit s. u. |
| **Negative** | „Given kein Schreibrecht im Default-Projektpfad, when Projekt anlegen, then Fehler-Code [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein leerer Projektzustand mit verlorenem Vorgänger" | **geteilt** — s. u. |

**Die Negative-AK trägt zwei Konjunkte, und nur eines ist strittig** (eigener Lauf, MEDIUM-3):

- „**kein leerer Projektzustand mit verlorenem Vorgänger**" ist der eigentliche Datenverlust-Schutz —
  und den **erfüllt** der Slice ohnehin: L1 (Reset), R1 und R3 sorgen dafür, dass ein abgebrochenes oder
  gescheitertes „Neu" den vorherigen Stand nicht verliert. Orakel: §6-Zeilen 1, 2, 5.
- „**Fehler-Code [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)**" setzt einen **Default-Projektpfad** voraus, in den beim Anlegen
  geschrieben wird. Ein speicher-residentes Neu-Projekt schreibt nicht und kann den Code darum nicht
  auslösen — vom Reviewer am Artefakt bestätigt (`sqlite_project_repository.cpp`, `io_atomic_write.cpp`).

**Entscheidung (Projektinhaber 2026-07-26):** das Anlegen bleibt **speicher-resident**; die
[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Klausel wird **geschärft** statt still stehen gelassen (lösungsfrei, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)). „AK
still stehen lassen und den Slice als erfüllt buchen" ist **kein** dritter Weg — genau das war der
slice-047-Befund.

**Was die Schärfung mitzieht** (eigener Lauf, MEDIUM-2): [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) ist in
`spec/spezifikation.md` §4 mit der Bedingung „Projekt anlegen/speichern" beschrieben. Fällt das Anlegen
als Auslöser weg, ist **diese Zeile** nachzuziehen — nicht nur das Lastenheft.

**Der Happy-Wortlaut „leerer Modellbaum" ist an die L3-Entscheidung gebunden** (eigener Lauf,
MEDIUM-4): bekommt das Neu-Projekt eine Zeichen-Ebene, ist der Modellbaum nicht mehr im engsten Sinne
leer (`Layer` ist ein Feld von `Building`). Die L3-Entscheidung ist damit **auch** eine
Anforderungs-Entscheidung und im selben Zug zu formulieren.

**Boundary-Wertigkeit (Lauf-2-MEDIUM-8):** die bestehende AK ist **zweiwertig** („Rückfrage ‚Änderungen
verwerfen?'"), während 052a an seinen Auslösern **dreiwertig** zusagt. Entweder liefert „Neu" nur zwei
Wege — dann ist die Abweichung zu benennen —, oder die Boundary wird auf dreiwertig geschärft. Der Plan
empfiehlt **dreiwertig** (Konsistenz über alle drei Auslöser); die Entscheidung gehört ins nächste
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start).

## 5. Bewusst NICHT Teil

- **Der Sitzungs-Zustand selbst** → [`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md).
  Dieser Slice **benutzt** Verdikt, `saveTarget()` und Antwort-Auswertung; er baut sie nicht.
- **Zuletzt-geöffnet-Liste**, **GUI-Export**, **Projektvorlagen** — eigene Schnitte.
- **Projektversionierung** ([`LH-FA-BLD-004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung)).

## 6. Orakel-Schnitt

Jede Zeile ist **außerhalb** des coverage-ausgenommenen `main` prüfbar — die Fenster-Zeilen über die
[`slice-053`](../done/slice-053-fenster-als-adapter.md)-Naht.

| # | Zusicherung | Diskriminierende Gegenprobe | Herkunft |
|---|---|---|---|
| 1 | **Nach „Neu" ist keine Datei mehr gemerkt**: `saveTarget()` verlangt eine Ziel-Abfrage | Reset entfernt ⇒ rot (alte Datei wird zurückgegeben) | **Lauf-2-HIGH-1 / L1** |
| 2 | **Nach „Neu" ist die Sitzung sauber** (Baseline = das neue leere Projekt) | Baseline nicht zurückgesetzt ⇒ rot | L1 |
| 3 | **Das neue Projekt hat genau ein Geschoss** mit der **benannten Default-Konstante** der Spezifikation | die Konstante durch einen **abweichenden** Wert ersetzt ⇒ rot | AK Happy / L4 |
| 4 | **Nach „Neu" zeigt das Zeichen-Ziel auf das neue Geschoss** — und, **falls L3 „mit Ebene" ergibt**, wird eine Hilfslinie angenommen; ergibt L3 „ohne Ebene", wird sie **vertragsgemäß abgelehnt und gemeldet** | Neu-Auflösung entfernt ⇒ rot | **L2 (B4-Klasse)** |
| 5 | **Ungesicherter Stand + „Neu" ⇒ `AskFirst`**; „abbrechen" ⇒ das alte Projekt bleibt **vollständig** — geprüft am **`ProjectMenuHandler`** (dort liegt die Rückfrage-Kette seit 052a), nicht am Sitzungs-Service allein | Verdikt übersprungen ⇒ rot | AK Boundary / **Lauf-2-HIGH-3** |
| 6 | **„speichern" als Antwort am Auslöser „Neu"** ⇒ erst speichern, dann anlegen; scheitert das Speichern ⇒ **nicht** anlegen — ebenfalls am `ProjectMenuHandler` | Fehler geschluckt und trotzdem angelegt ⇒ rot | Lauf-1-MED-7 / **Lauf-2-HIGH-3** |
| 7 | *(entfällt als Orakel — s. u., Lauf-2-HIGH-2; die Zusage bleibt als benannte Grenze)* | — | **L5** |
| 8 | **Die L3-Entscheidung ist beobachtbar umgesetzt** (Ebene vorhanden **oder** Meldung an den Benutzer, analog `spezifikation.md` „Öffnen ist lesend") | die gewählte Zusage entfernt ⇒ rot | eigener Lauf MEDIUM-7 |

**Zeile 3, zweimal präzisiert.** Lauf 1 verwarf die Gegenprobe „Default-Höhe **hart kodiert** ⇒ rot"
— ein Literal `2500.0` ist vom Konstanten-Wert nicht unterscheidbar. Die Nachbesserung („gegen die
**benannte Konstante** prüfen, die Gegenprobe ändert ihren Wert") diskriminiert aber **ebenso wenig**
(Lauf-2-MEDIUM-1): liest der Test dieselbe Konstante wie die Produktion, wandern beide gemeinsam und
der Test bleibt grün. **Auflösung: der Test führt den Wert der Spezifikation als eigenes Literal.**
Wer die Konstante ändert, ohne die Spezifikation zu ändern, wird rot — und genau das ist die Zusage
(„Höhe **aus der Spezifikation**", nicht „irgendein Default").

**Zeile 4 ist ausdrücklich von der L3-Entscheidung abhängig** (eigener Lauf, MEDIUM-6) — sie prüft in
**beiden** Ausgängen etwas, aber Verschiedenes. Ohne diese Markierung wäre sie eine Zusage, die je nach
Entscheidung ins Leere läuft.

**Zeile 7 entfällt als Orakel — der Fenstertitel ist nicht dort prüfbar, wo der Plan ihn buchte**
(Lauf-2-HIGH-2). `main_window.h` schließt den Fenstertitel **per Vertrag** aus der Fenster-Klasse aus,
und [`slice-053`](../done/slice-053-fenster-als-adapter.md) hat ihn als benannte Grenze in `main.cpp`
belassen — ausdrücklich unter Nennung von „052b-L5". Die **Zusage bleibt** (nach „Neu" darf der Titel
nicht weiter die alte Datei behaupten), sie wird nur als **Grenze** geführt statt als Orakel. Ein
Orakel dafür verlangte, den Titel in die Fenster-Klasse zu ziehen — eine Architektur-Änderung, die
dieser Slice nicht trägt und die 053 begründet abgelehnt hat.

**Benannte Grenze — vollständige Aufzählung** (nachgezogen auf die von 053/052a hinterlassene):
(1) die **modalen Dialoge** selbst; (2) **welcher** Dialog mit welchem **Meldungstext** erscheint;
(3) der **Fenstertitel** samt seinem Reset nach „Neu" (L5, s. o.); (4) die `.bcad`-**Suffix-Ergänzung**;
(5) der **Fenster-Aufbau**. Wie in
[`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md), mit derselben Auflage: **in der
Verdrahtung steht keine Entscheidung.**

## 7. Definition of Done

- [ ] **Projekt-Erzeugung als Kern-Funktion** (L4): leeres `Building` mit genau einem Geschoss,
      Höhe aus der **benannten** Default-Konstante der Spezifikation — **nicht** in `main.cpp`.
- [ ] **Die zwei Port-Ergänzungen** (§2.1): `ManageProjectPort::newProject()` und
      `ProjectSessionPort::reset(baseline)` — je eine Methode, **nur `model/` + Standardbibliothek** im
      Header, **keine** neue Schicht-Kante. Gegenprobe wie in
      [`slice-054`](../done/slice-054-manage-project-port.md): Probe-Include aus `ports/driven/` ⇒
      `make a-check` meldet `wrong-direction`.
- [ ] **Sitzungs-Reset** (L1) im `ProjectSessionService`
      (`src/hexagon/services/project_session.{h,cpp}`, aus
      [`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md) **erweitert** — dessen
      Oberfläche führt bewusst keinen Reset; Lauf-1-MEDIUM-9): „Neu" setzt gemerkte Datei **und**
      Vergleichs-Basis zurück. Orakel §6-1/2.
- [ ] **Menü-Aktion „Neu" im `ProjectMenuHandler`** (`ui/command/`): ruft `newProject()` **hinter**
      dem vorhandenen `mayDiscard(ask, ask_target)` aus 052a — die Rückfrage-Kette wird **nicht neu
      geschrieben**. `MainWindow::FileActions` bekommt einen vierten Eintrag samt Objektnamen.
      Orakel §6-5/6.
- [ ] **Zeichen-Ziel neu auflösen** (L2): dieselbe Naht wie beim Öffnen (slice-047
      `DrawingTargetSinks`); Orakel §6-4.
- [ ] **Fenstertitel folgt dem Wechsel** (L5) — in `main.cpp`, als **benannte Grenze ohne Orakel**
      (Lauf-2-HIGH-2: `main_window.h` schließt den Titel per Vertrag aus, 053 hat ihn dort belassen).
      Die Zusage steht, der Beleg ist Sichtprüfung.
- [ ] **L3 ist entschieden** (§2, Kasten): das neue Projekt bekommt **genau eine Zeichen-Ebene**.
      Sie gehört in den Happy-AK-Wortlaut (§4) und in den §1-Spezifikations-Block. Orakel §6-8.
- [ ] **`spec/architecture.md`**: die §1.1-Port-Zeile sagt heute, „anlegen" sei **nicht** im
      `ManageProjectPort`-Vertrag — mit `newProject()` wird das falsch und ist nachzuziehen; dazu der
      neue `ProjectSessionPort`-Eintrag und der §2.1-Baum (Lauf-2-MEDIUM-3).
- [ ] **Lastenheft:** Happy-AK („EG" → beschreibend, Modellbaum-Wortlaut) und Negative-AK
      ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Klausel) geschärft; Boundary-Wertigkeit entschieden. Header-Version +
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md)
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).
- [ ] **Spezifikation:** §1-Block für die „Neu"-Mechanik mit **eigenem Anforderungs-Anker**
      (Lauf-1-LOW-3) **und** Nachzug der [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung in §4 (Lauf-1-MEDIUM-2) +
      Provenance-Zeile in [`spezifikation-historie.md`](../../../../spec/spezifikation-historie.md).
      **Zusätzlich (Lauf-2-MEDIUM-2): der von 052a gelieferte Block [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b`/`003.b` zählt
      die Auslöser der Rückfrage („Öffnen, Beenden") und die Baseline-Quellen abschließend auf** —
      „Neu" ist ein dritter Auslöser und eine dritte Baseline-Quelle. Der Bestandsblock wird
      **erweitert**, nicht nur ein neuer danebengestellt; sonst widersprechen sich zwei §1-Blöcke.
- [ ] **Boundary-Wertigkeit: entschieden, nicht offen** (Lauf-2-MEDIUM-5) — **dreiwertig**
      (speichern / verwerfen / abbrechen). Seit 052a tragen
      [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)
      und der §1-Block das normativ; „Neu" weicht davon nicht ab.
- [ ] **Benutzerhandbuch** an den Stellen, die den Funktionsstand aufzählen: **§1 „Heute möglich"**,
      **§3 „drei Wege"** (die Zählung wird falsch), **§4.1-Tabelle**, **§2.3** und **4.3**.
      Handbuch-Version + Änderungshistorie. **Reihenfolge beachten** (eigener Lauf LOW-4): 052a führt
      dieselben Stellen für „Speichern" — wer zuletzt liefert, prüft den Stand des anderen mit.
      (**`docs/user/` steht in dieser DoD-Zeile.**)
- [ ] **CHANGELOG**; **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md), eigener Lauf LOW-3); `make schema-check`
      byte-unberührt.

## 8. Plan (vor Code)

*(eigener Lauf MEDIUM-10: der Abschnitt fehlte — alle anderen Pläne im Repo führen ihn.)*

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/manage_project_port.{h}` | **ändern** | `newProject()` (§2.1) |
| `src/hexagon/ports/driving/project_session_port.{h}` | **ändern** | `reset(baseline)` (§2.1) — `markPersisted` verlangt einen Pfad und taugt nicht |
| `src/hexagon/services/project_session.{h,cpp}` | ändern | Reset von Pfad + Vergleichs-Basis (L1) |
| `src/hexagon/services/manage_project.{h,cpp}` | ändern | Erzeugung des leeren Projekts als Kern-Funktion (L4) + Sitzungs-Reset + Zeichen-Ziel-Auflösung (L2) |
| `src/adapters/ui/command/project_menu_handler.{h,cpp}` | **ändern** | `newProject()` hinter dem vorhandenen `mayDiscard` (§2.1) — der Aufrufer, den der Plan bisher nicht hatte |
| `src/adapters/ui/view/main_window.{h,cpp}` | **ändern (klein)** | **nur** ein vierter `FileActions`-Eintrag „Neu" + Objektname. **Nicht** der Titel-Reset (Lauf-2-HIGH-2) |
| `src/adapters/CMakeLists.txt` | ändern | falls eine neue Adapter-Datei entsteht (Lauf-2-MEDIUM-4) |
| `src/main.cpp` | ändern | Verdrahtung + Dialog + Titel-Reset (**keine** Entscheidung) |
| `tests/hexagon/test_project_session.cpp` | ändern | §6-Zeilen 1, 2 (Reset am Service) |
| `tests/hexagon/test_manage_project_port.cpp` | **ändern** | §6-Zeilen 3, 8 (Erzeugung + Ebene, über den Port) |
| `tests/adapters/test_project_menu_handler.cpp` | **ändern** | §6-Zeilen 5, 6 (Rückfrage-Kette am Auslöser „Neu", Lauf-2-HIGH-3) |
| `tests/adapters/test_main_window.cpp` | ändern | §6-Zeile 4 (Zeichen-Ziel) + die vierte Menü-Aktion |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Schärfungen + Header-Version |
| `spec/spezifikation.md`, `spec/spezifikation-historie.md` | ändern | §1-Block + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung + Provenance |
| `spec/architecture.md` | ändern | Kern-Funktion im §2.1-Baum |
| `docs/user/benutzerhandbuch.md` | ändern | fünf Stellen (s. DoD) |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |
| `docs/reviews/`-Reporte | **liegen** | [Lauf 1](../../../reviews/2026-07-26-slice-052b-plan.md) + [Lauf 2](../../../reviews/2026-07-27-slice-052b-plan-2.md) |

**Nicht berührt:** `data-model.yaml`/`schema.sql` (kein persistenter Zustand, **kein** Geschoss-Namensfeld
— §3), `.d-check.yml`/`.a-check.yml` (**keine** neue Kante — die zwei Port-Ergänzungen laufen über
bestehende), `docs/plan/adr/`.

## 9. Risiken

- **R1 — L1 ist ein Datenverlust-Pfad, kein Komfort-Thema.** Ohne den Reset überschreibt „Speichern"
  nach „Neu" das zuvor geöffnete Projekt. Deshalb sind §5-1/2 **Pflicht**-Orakel.
- **R2 — L3 kollidiert mit „Öffnen ist lesend".** slice-047 hat das stille Anlegen einer Ebene beim
  Öffnen bewusst **zurückgenommen** (Code-Review MEDIUM-8), weil der Speicher-Stand sonst vom
  Dateiinhalt abwich. Beim **Neu**-Projekt gibt es keine Datei — das Argument trägt hier also nicht
  automatisch. Die Entscheidung ist zu treffen, nicht zu übertragen.
- **R3 — Reihenfolge Verdikt → Erzeugung.** Die Rückfrage muss **vor** dem Ersetzen des Stands laufen;
  ein „abbrechen" darf nichts anfassen.

## 10. Trigger

- [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) Lauf 1 (HIGH-2) + Lauf 2 (HIGH-1, MEDIUM-5/-7/-8/-9/-11) zu slice-052; Split-Entscheidung des
  Projektinhabers 2026-07-26.

## 11. Closure-Trigger

- Alle §6-Zeilen grün + je einmal diskriminierend belegt; die §3-/§4-Entscheidungen vollzogen und im
  Closure-Text begründet; `make gates` grün.

## 12. MR-006-Einarbeitung (erster eigener Lauf, 2026-07-26)

Report: [`2026-07-26-slice-052b-plan.md`](../../../reviews/2026-07-26-slice-052b-plan.md) —
**1 HIGH / 11 MEDIUM / 5 LOW / 2 INFO, „nicht startbar"**. Der Lauf hat ausdrücklich bestätigt, dass
**keine** Zusage zwischen 052a und 052b verloren ging; die zwei Dinge, die der **Split neu erzeugt**
hat, sind hier aufgenommen (MEDIUM-9, MEDIUM-11).

| # | Behandlung |
|---|---|
| **HIGH-1** („EG" ist kein Modell-Feld) | **§3** neu: „EG" ist beschreibend, die AK wird geschärft — **kein** Schema-Eingriff. Der Nebenbefund (Persistenz schreibt einen Namen, den niemand liest) ist als benannte Lücke aktenkundig. |
| **MEDIUM-1** (§1 „alle drei AK" vs. §3 Weg (b) streicht eine) | §1 sagt jetzt: **zwei erfüllen, eine schärfen** — beide Aussagen sind vereinbar. |
| **MEDIUM-2** (Weg (b) berührt auch die Spezifikation) | DoD nennt den Nachzug der [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung in §4 der Spezifikation. |
| **MEDIUM-3** (Negative-AK hat zwei Konjunkte) | §4 trennt sie: der Datenverlust-Schutz ist **erfüllt** (Orakel §6-1/2/5), nur die Fehler-Code-Klausel wird geschärft. |
| **MEDIUM-4** (L3 berührt „leerer Modellbaum") | §4 benennt die Kopplung; die L3-Entscheidung zieht den Happy-Wortlaut mit. |
| **MEDIUM-5** (Gegenprobe Zeile 3 nicht diskriminierend) | geprüft wird gegen die **benannte Konstante**, die Gegenprobe ändert ihren **Wert**. |
| **MEDIUM-6** (Zeile 4 setzt L3 „mit Ebene" voraus) | Zeile 4 ist als **abhängig** markiert und prüft in **beiden** Ausgängen etwas. |
| **MEDIUM-7** (zwei DoD-Zusagen ohne Orakel) | **§6-Zeilen 6 und 8** neu (dreiwertiger „speichern"-Zweig; L3-Ausgang beobachtbar). |
| **MEDIUM-8** (fünftes Loch: Fenstertitel) | **L5** in §2, **§6-Zeile 7**, eigene DoD-Zeile. |
| **MEDIUM-9** (L1-Reset ohne Träger) | DoD nennt `project_session.*` als **Erweiterung** der 052a-Oberfläche; §8 führt die Datei. |
| **MEDIUM-10** (kein „Plan (vor Code)") | **§8** neu. |
| **MEDIUM-11** (`architecture.md` fehlt) | eigene DoD-Zeile + §8. |
| **LOW-1** (Rahmensatz stimmt für L4 nicht) | §2-Einleitung korrigiert. |
| **LOW-2** | `adr_refs` um [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) + [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) ergänzt. |
| **LOW-3** | Ruhe-Marker-Toggle in der DoD-Gates-Zeile. |
| **LOW-4** (Handbuch-Überschneidung mit 052a) | Reihenfolge-Hinweis in der DoD-Zeile. |
| **LOW-5** (Entwurf-Marker, fehlende Phase-Reife) | „Entwurf" entfernt (§6/§7 sind Maßstab), §13 trägt eine Phase-Reife-Zeile. |
| **INFO-1** (052b hängt an 052a, das selbst nicht startbar ist) | im Kopf deklariert; die Sequenz ist 053 → 052a → 052b. |
| **INFO-2** (die §4-Wahl gehört dem Projektinhaber) | getroffen 2026-07-26, in §3/§4 protokolliert. |

## 13. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (zweiter Lauf, 2026-07-27)

Report: [`2026-07-27-slice-052b-plan-2.md`](../../../reviews/2026-07-27-slice-052b-plan-2.md) —
**3 HIGH / 6 MEDIUM / 5 LOW / 3 INFO, „nicht startbar"**. Unabhängiger Reviewer ≠ Plan-Autor ≠ Autor
des Lauf-1-Reports.

**Ausgangs-Befund, der alle drei HIGH erklärt:** der Plan-Körper war seit `51fd2d2` (2026-07-26)
**inhaltlich unverändert** — die Folge-Commits zogen nur Lifecycle-Pfade der Nachbar-Pläne nach.
`Port`, `ManageProject`, `ProjectMenuHandler`, `FileActions`, `CloseGuard` und `054` hatten **null**
Fundstellen, obwohl [`slice-054`](../done/slice-054-manage-project-port.md),
[`slice-053`](../done/slice-053-fenster-als-adapter.md) und
[`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md) inzwischen geliefert sind. **Alle
drei HIGH sind Fortschreibungen von Lauf-1-MEDIUM, deren Behandlung nur am Text stattfand.**

| # | Behandlung |
|---|---|
| **HIGH-1** (kein Port ⇒ „Neu" und der L1-Reset nur aus dem sensorlosen `main.cpp` erreichbar — bei einem **Datenverlust**-Pfad) | **§2.1 neu**: `ManageProjectPort::newProject()` + `ProjectSessionPort::reset(baseline)`, Aufrufer ist der `ProjectMenuHandler` hinter dem vorhandenen `mayDiscard`. DoD + Datei-Plan durchgezogen. |
| **HIGH-2** (Titel-Reset auf `main_window.*` gebucht, das den Titel per Vertrag ausschließt) | Orakel-Zeile 7 **entfällt**; die Zusage bleibt als **benannte Grenze** in `main.cpp` — 053 hat genau das entschieden, unter Nennung von „052b-L5". |
| **HIGH-3** (Zeilen 5/6 auf `test_project_session.cpp`, obwohl die Rückfrage-Kette seit 052a im `ProjectMenuHandler` liegt) | Zeilen 5/6 auf `tests/adapters/test_project_menu_handler.cpp` umgehängt. **Dieselbe Bauart wie der 052a-Fund**, dessen Gegenprobe grün blieb, weil ein Test die falsche Komponente prüfte. |
| **MEDIUM-1** (Gegenprobe Zeile 3 diskriminiert weiterhin nicht) | der Test führt den Spezifikations-Wert als **eigenes Literal**; wandert die Konstante allein, wird er rot. |
| **MEDIUM-2** (der 052a-§1-Block zählt Auslöser/Baseline-Quellen abschließend auf) | DoD verlangt jetzt die **Erweiterung** des Bestandsblocks, nicht nur einen neuen daneben. |
| **MEDIUM-3** (`architecture.md` behauptet, „anlegen" sei nicht im Vertrag) | eigene DoD-Zeile: Port-Zeile **und** neuer `ProjectSessionPort`-Eintrag **und** §2.1-Baum. |
| **MEDIUM-4** (Handler-Heimat + `src/adapters/CMakeLists.txt` fehlten) | im Datei-Plan ergänzt. |
| **MEDIUM-5** (Boundary-Wertigkeit als offene Frage geführt) | **entschieden: dreiwertig** — seit 052a normativ getragen. |
| **MEDIUM-6** (L3-Entscheidung fehlte im Plan, obwohl die DoD sie verlangt) | **entschieden: das neue Projekt bekommt eine Zeichen-Ebene** (§2, Kasten) — mit Begründung gegen „Öffnen ist lesend". |

**Stand der Lauf-1-Findings** (vom Lauf-2-Reviewer am Artefakt geprüft, nicht an der §12-Tabelle):
**11 von 19 bestätigt aufgelöst**, 4 teilweise, **4 nur scheinbar** — MEDIUM-5 → M-1, MEDIUM-7 →
HIGH-3, MEDIUM-8 → HIGH-2, MEDIUM-9 → HIGH-1.

**Positiv bestätigt** (nicht neu prüfen): L1/L2/L3/L4 beschreiben die Lieferung korrekt · das
[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Argument trägt · R2 („Öffnen ist lesend") ist nicht vorentschieden ·
[MR-008](../../../../harness/conventions.md)/[MR-010](../../../../harness/conventions.md) gewahrt ·
**`docs/user/` steht mit fünf existierenden Stellen ausdrücklich in der Doku-DoD**.

**Startbar:** ja — alle drei HIGH sind aufgelöst, die MEDIUM eingearbeitet, die zwei offenen
Entscheidungen (Port-Verortung, L3) getroffen.

## 14. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** mittel-hoch — eine Menü-Aktion, aber **fünf** Zustands-/Anforderungs-Löcher
  (§2), drei AK-Schärfungen (§4) und eine offene Lösungs-Entscheidung (L3).
- **Phase-Reife:** die Sitzungs-Mechanik kommt aus 052a, das Fenster aus 053 — dieser Slice setzt
  beides zusammen und fügt die Erzeugung hinzu.
- **Risiko:** mittel-hoch — **L1** ist ein Datenverlust-Pfad; **L2** ist eine bereits einmal teuer
  bezahlte Klasse (slice-047 B4).

## 15. Closure-Notiz

**Ausgeführt 2026-07-27.** `make gates` **EXIT=0** (docs-check 0 Befunde / 260 Dateien · a-check 0 ·
arch-check ok · **352/352** Tests · Coverage 91,8 %), `make io-smoke` EXIT=0, `make schema-check`
EXIT=0 (byte-unberührt).

### Was entstanden ist

- **`ManageProjectPort::newProject(sinks)`** und **`ProjectSessionPort::reset(baseline)`** — je eine
  Methode, beide Header weiterhin nur `<std>` + `model/`.
- **`services::newProjectModel()`** — die fachliche Regel im Kern: ein Geschoss mit
  `kDefaultStoreyHeightMm`, eine Zeichen-Ebene.
- **`resolveDrawingTarget`** aus `openProject` herausgelöst (interne Bindung), weil „Neu" denselben
  Schritt braucht — Verhalten unverändert, die 047b/054-Orakel bleiben der Maßstab.
- **`ProjectMenuHandler::newProject(ask, ask_target)`** — ruft `mayDiscard` aus 052a und legt erst
  danach an. Die Rückfrage-Kette wurde **nicht** neu geschrieben.
- **`MainWindow`**: vierter `FileActions`-Eintrag + `kNewActionName`. Sonst unverändert.
- **`main.cpp`**: `newProjectWithTitleReset` (Dialoge + Titel-Reset), sonst Verdrahtung.

### Orakel §6 — je mit roter Gegenprobe (gemessen)

| Zeile | Gegenprobe | Ergebnis |
|---|---|---|
| 1/2 | `session_->reset(...)` in `newProject` entfernt | **1 rot**: `NeuesProjektVergisstDieGemerkteDateiUndIstSauber` |
| 3 | `kDefaultStoreyHeightMm` auf 2600 geändert | **2 rot**: `NeuesProjektHatEinGeschossMitSpezifikationsHoehe` + ein IFC-Import-Test |
| 4 | (durch Zeile 1/2 mitgedeckt; eigener Test `NeuesProjektLoestDasZeichenZielNeuAuf`) | grün, diskriminiert über die Sink-Zusicherungen |
| 5/6 | `mayDiscard`-Aufruf in `newProject` übersprungen | **3 rot**: `NeuAbbrechenLaesstDasAlteProjektStehen`, `NeuLegtNichtAnWennDasSpeichernScheitert`, `NeuMitSpeichernSpeichertZuerst` |
| 8 | die Zeichen-Ebene aus `newProjectModel()` entfernt | **1 rot**: `NeuesProjektHatEineZeichenEbene` |

Zeile 3 diskriminiert jetzt tatsächlich: der Test führt **2500.0 als eigenes Literal**. Nach der
Lauf-1-Nachbesserung („gegen die benannte Konstante prüfen") wäre er mit der Konstante mitgewandert —
das war Lauf-2-MEDIUM-1.

**Zeile 7 (Fenstertitel) hat wie geplant kein Orakel** — sie ist benannte Grenze (Lauf-2-HIGH-2):
`main_window.h` schließt den Titel per Vertrag aus, 053 hat ihn in `main.cpp` belassen. Der
Titel-Reset ist implementiert und sichtgeprüft, nicht orakel-gedeckt.

### Anforderungs-Ebene

**Lastenheft 0.1.19** — alle **drei** BLD-001-AK geschärft: „EG" entfällt (ein Geschoss trägt im
Modell keinen Namen), „leerer Modellbaum" wird zu „ein Geschoss + eine Zeichen-Ebene, sonst ohne
Bauteile" samt der Folge „sofort bezeichenbar", die Boundary wird **dreiwertig**, und die
[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Klausel entfällt — sie setzte einen Default-Projektpfad voraus, in den beim Anlegen
geschrieben wird; das Anlegen ist speicher-resident. **Der eigentliche Schutz bleibt** und zeigt jetzt
auf den real erreichbaren Auslöser (gescheitertes/abgebrochenes Speichern im Verwerf-Ablauf).

**Spezifikation:** neuer §1-Block [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)`.a` **und** — wie Lauf-2-MEDIUM-2 verlangt — der
052a-Block [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b`/`003.b` **erweitert** (dritter Auslöser, dritte Baseline-Quelle), dazu
die [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung in §4. `architecture.md`: Port-Zeile („anlegen" ist jetzt im Vertrag),
`ProjectSessionPort`-Zeile, §2.1-Baum. **Benutzerhandbuch 1.3** an allen fünf zugesagten Stellen,
inklusive der Wege-Zählung in §3 (drei → **vier**).

### Was dieser Slice abschließt

Mit 052b ist die **Vierer-Kette aus dem slice-047-Validations-Rest** vollständig: 054 (Port) → 053
(Fenster/Handler als Adapter) → 052a (Sitzungs-Zustand + Speichern) → 052b („Neu").
[`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) ist erstmals
benutzer-erfüllbar.
