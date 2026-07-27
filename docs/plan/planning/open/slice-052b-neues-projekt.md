---
id: slice-052b
titel: „Neues Projekt" — [LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) benutzer-erfüllbar machen
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)]
adr_refs: [[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 052b: „Neues Projekt"

**Status:** open — **abhängig von [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md)**
(Sitzungs-Zustand + Verdikt-Maschinerie). Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
**2026-07-26 gefahren: 1 HIGH / 11 MEDIUM / 5 LOW / 2 INFO → nicht startbar**
([Report](../../../reviews/2026-07-26-slice-052b-plan.md)); alle Findings eingearbeitet (§12).
**Ein zweiter Lauf vor dem Start.** Zusätzlich hängt die Startbarkeit an
[`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md) (Sitzungs-Zustand) und mittelbar an
[`slice-053`](../in-progress/slice-053-fenster-als-adapter.md) (testbares Fenster).

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
- **L4 — die Projekt-Erzeugung ist Mechanik (Lauf-2-MEDIUM-6b).** „Leeres Projekt mit genau **einem**
  Geschoss, Default-Höhe aus der Spezifikation" ist eine fachliche Regel. Sie gehört **nicht** in den
  orakel-losen Composition-Root — dieselbe Auflage wie in slice-047: „Was in `main` bleiben darf, ist
  Dialog, Meldung, Verdrahtung — keine Entscheidung."

- **L5 — der Fenstertitel bleibt auf der alten Datei (eigener Lauf, MEDIUM-8).** slice-047 schreibt den
  Dateinamen beim Öffnen in den Titel (`main.cpp`:308–309), der Start setzt ihn auf „b-cad" (:483). Nach
  „Neu" behauptet die Oberfläche also weiterhin, das zuvor geöffnete Projekt zu zeigen — dieselbe
  Fehl-Aussage wie L1, nur sichtbar statt folgenschwer. Gehört zur selben Reset-Zusage.

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

- **Der Sitzungs-Zustand selbst** → [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md).
  Dieser Slice **benutzt** Verdikt, `saveTarget()` und Antwort-Auswertung; er baut sie nicht.
- **Zuletzt-geöffnet-Liste**, **GUI-Export**, **Projektvorlagen** — eigene Schnitte.
- **Projektversionierung** ([`LH-FA-BLD-004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung)).

## 6. Orakel-Schnitt

Jede Zeile ist **außerhalb** des coverage-ausgenommenen `main` prüfbar — die Fenster-Zeilen über die
[`slice-053`](../in-progress/slice-053-fenster-als-adapter.md)-Naht.

| # | Zusicherung | Diskriminierende Gegenprobe | Herkunft |
|---|---|---|---|
| 1 | **Nach „Neu" ist keine Datei mehr gemerkt**: `saveTarget()` verlangt eine Ziel-Abfrage | Reset entfernt ⇒ rot (alte Datei wird zurückgegeben) | **Lauf-2-HIGH-1 / L1** |
| 2 | **Nach „Neu" ist die Sitzung sauber** (Baseline = das neue leere Projekt) | Baseline nicht zurückgesetzt ⇒ rot | L1 |
| 3 | **Das neue Projekt hat genau ein Geschoss** mit der **benannten Default-Konstante** der Spezifikation | die Konstante durch einen **abweichenden** Wert ersetzt ⇒ rot | AK Happy / L4 |
| 4 | **Nach „Neu" zeigt das Zeichen-Ziel auf das neue Geschoss** — und, **falls L3 „mit Ebene" ergibt**, wird eine Hilfslinie angenommen; ergibt L3 „ohne Ebene", wird sie **vertragsgemäß abgelehnt und gemeldet** | Neu-Auflösung entfernt ⇒ rot | **L2 (B4-Klasse)** |
| 5 | **Ungesicherter Stand + „Neu" ⇒ `AskFirst`**; „abbrechen" ⇒ das alte Projekt bleibt **vollständig** | Verdikt übersprungen ⇒ rot | AK Boundary |
| 6 | **„speichern" als Antwort am Auslöser „Neu"** ⇒ erst speichern, dann anlegen; scheitert das Speichern ⇒ **nicht** anlegen | Fehler geschluckt und trotzdem angelegt ⇒ rot | eigener Lauf MEDIUM-7 |
| 7 | **Der Fenstertitel folgt dem Wechsel** (kein Verweis mehr auf die alte Datei) | Titel-Reset entfernt ⇒ rot | **L5** |
| 8 | **Die L3-Entscheidung ist beobachtbar umgesetzt** (Ebene vorhanden **oder** Meldung an den Benutzer, analog `spezifikation.md` „Öffnen ist lesend") | die gewählte Zusage entfernt ⇒ rot | eigener Lauf MEDIUM-7 |

**Zeile 3, präzisiert** (eigener Lauf, MEDIUM-5): die frühere Gegenprobe „Default-Höhe **hart kodiert**
⇒ rot" diskriminierte **nicht** — ein Literal `2500.0` ist vom Konstanten-Wert nicht unterscheidbar.
Geprüft wird deshalb gegen die **benannte Konstante**, und die Gegenprobe ändert ihren **Wert**.

**Zeile 4 ist ausdrücklich von der L3-Entscheidung abhängig** (eigener Lauf, MEDIUM-6) — sie prüft in
**beiden** Ausgängen etwas, aber Verschiedenes. Ohne diese Markierung wäre sie eine Zusage, die je nach
Entscheidung ins Leere läuft.

**Benannte Grenze:** die modalen Dialoge — wie in
[`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md), mit derselben Auflage: **in der
Verdrahtung steht keine Entscheidung.**

## 7. Definition of Done

- [ ] **Projekt-Erzeugung als Kern-Funktion** (L4): leeres `Building` mit genau einem Geschoss,
      Höhe aus der **benannten** Default-Konstante der Spezifikation — **nicht** in `main.cpp`.
- [ ] **Sitzungs-Reset** (L1) **auf `ProjectSession`** (`src/hexagon/services/project_session.*`, aus
      [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md) **erweitert** — dessen Oberfläche
      führt bewusst keinen Reset; eigener Lauf MEDIUM-9): „Neu" setzt gemerkte Datei **und**
      Vergleichs-Basis zurück. Orakel §6-1/2.
- [ ] **Zeichen-Ziel neu auflösen** (L2): dieselbe Naht wie beim Öffnen (slice-047
      `DrawingTargetSinks`); Orakel §6-4.
- [ ] **Fenstertitel folgt dem Wechsel** (L5); Orakel §6-7.
- [ ] **L3-Entscheidung getroffen, begründet und beobachtbar**: bekommt das Neu-Projekt eine
      Zeichen-Ebene, oder bleibt der Benutzer bis zum manuellen Anlegen ohne Zeichen-Möglichkeit — mit
      Meldung? Die Antwort steht im Plan, nicht im Code; sie zieht den Happy-AK-Wortlaut mit (§4).
      Orakel §6-8.
- [ ] **Menü-Aktion „Neu"** mit Rückfrage über die 052a-Maschinerie; **Abbrechen** unterlässt,
      **„speichern"** speichert erst (Orakel §6-5/6).
- [ ] **Lastenheft:** Happy-AK („EG" → beschreibend, Modellbaum-Wortlaut) und Negative-AK
      ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Klausel) geschärft; Boundary-Wertigkeit entschieden. Header-Version +
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md)
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).
- [ ] **Spezifikation:** §1-Block für die „Neu"-Mechanik mit **eigenem Anforderungs-Anker**
      (Lauf-2-LOW-3) **und** Nachzug der [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung in §4 (eigener Lauf MEDIUM-2) +
      Provenance-Zeile in [`spezifikation-historie.md`](../../../../spec/spezifikation-historie.md).
- [ ] **`spec/architecture.md`** §2.1-Baum: die neue Kern-Funktion nennen (eigener Lauf MEDIUM-11 — der
      Baum führt die Kern-Use-Cases namentlich; 052a hat dieselbe Zeile).
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
| `src/hexagon/services/project_session.{h,cpp}` | ändern | Reset von Pfad + Vergleichs-Basis (L1) — die Oberfläche kommt aus 052a und wird hier **erweitert** |
| `src/hexagon/services/manage_project.{h,cpp}` (o. neue Kern-Datei) | ändern | Erzeugung des leeren Projekts als Kern-Funktion (L4) + Zeichen-Ziel-Auflösung (L2) |
| `src/adapters/ui/view/main_window.*` (aus [`slice-053`](../in-progress/slice-053-fenster-als-adapter.md)) | ändern | Menü-Aktion **Neu**, Titel-Reset (L5) — hier, weil nur hier prüfbar |
| `src/main.cpp` | ändern | Verdrahtung + Dialog (**keine** Entscheidung) |
| `tests/hexagon/test_project_session.cpp` | ändern | §6-Zeilen 1, 2, 5, 6 |
| `tests/hexagon/test_manage_project.cpp` | ändern | §6-Zeilen 3, 8 (Erzeugung + L3-Ausgang) |
| `tests/adapters/test_main_window.cpp` | ändern | §6-Zeilen 4, 7 (Zeichen-Ziel, Titel) |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Schärfungen + Header-Version |
| `spec/spezifikation.md`, `spec/spezifikation-historie.md` | ändern | §1-Block + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung + Provenance |
| `spec/architecture.md` | ändern | Kern-Funktion im §2.1-Baum |
| `docs/user/benutzerhandbuch.md` | ändern | fünf Stellen (s. DoD) |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |
| `docs/reviews/`-Report zum zweiten Plan-Review | neu | das zweite [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `data-model.yaml`/`schema.sql` (kein persistenter Zustand, **kein** Geschoss-Namensfeld
— §3), `.d-check.yml`/`.a-check.yml`, `docs/plan/adr/`.

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

## 13. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** mittel-hoch — eine Menü-Aktion, aber **fünf** Zustands-/Anforderungs-Löcher
  (§2), drei AK-Schärfungen (§4) und eine offene Lösungs-Entscheidung (L3).
- **Phase-Reife:** die Sitzungs-Mechanik kommt aus 052a, das Fenster aus 053 — dieser Slice setzt
  beides zusammen und fügt die Erzeugung hinzu.
- **Risiko:** mittel-hoch — **L1** ist ein Datenverlust-Pfad; **L2** ist eine bereits einmal teuer
  bezahlte Klasse (slice-047 B4).

## 14. Closure-Notiz

_(bei Ausführung auszufüllen)_
