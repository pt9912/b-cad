---
id: slice-052b
titel: „Neues Projekt" — [LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) benutzer-erfüllbar machen
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)]
adr_refs: [[ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 052b: „Neues Projekt"

**Status:** open — **abhängig von [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md)**
(Sitzungs-Zustand + Verdikt-Maschinerie). Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start; dieser Plan ist **nie** eigenständig reviewt worden, sondern aus dem Split von
slice-052 entstanden.

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
benutzer-erfüllbar, **mit allen drei** Akzeptanzkriterien (Happy Path, Boundary, Negative), nicht nur
mit dem bequemen.

## 2. Die vier Löcher, die dieser Slice schließen muss

Sie sind alle in Lauf 2 belegt und **keines** entsteht durch das Anlegen selbst — sie entstehen durch
das, was ein Projekt-Wechsel **mit der Sitzung macht**:

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

## 3. Anforderungs-Ebene

[`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) trägt **drei** AK. Der
Slice muss sich zu **allen** verhalten (Lauf-2-MEDIUM-7 — die Vorfassung adressierte nur zwei und
führte die dritte weder in der DoD noch in der Abgrenzung):

| AK | Wortlaut (gekürzt) | Lage |
|---|---|---|
| **Happy Path** | „when „Neues Projekt", then ein leeres Projekt mit genau einem Geschoss (EG, Default-Höhe) und einem leeren Modellbaum" | **erfüllen** (L4) |
| **Boundary** | „Given ein bereits geöffnetes, ungespeichertes Projekt, when „Neues Projekt", then Rückfrage „Änderungen verwerfen?"" | **erfüllen** über die 052a-Verdikt-Maschinerie |
| **Negative** | „Given kein Schreibrecht im Default-Projektpfad, when Projekt anlegen, then Fehler-Code [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein leerer Projektzustand mit verlorenem Vorgänger" | **offen — s. u.** |

**Die Negative-AK ist beim Start zu klären, nicht zu übergehen.** Sie setzt einen
**Default-Projektpfad** voraus, in den beim Anlegen geschrieben wird; der geplante Entwurf lässt ein
Neu-Projekt jedoch **rein im Speicher** entstehen (kein Schreiben, also auch kein [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). Zwei
Wege, beide legitim, **Entscheidung im [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)**:

- **(a) AK einlösen** — das Anlegen schreibt sofort in einen Default-Pfad; dann ist [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
  erreichbar und die AK erfüllt.
- **(b) AK als nicht mehr zutreffend behandeln** — dann ist sie im Lastenheft zu **schärfen**
  (lösungsfrei, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)),
  weil ein Speicher-Projekt keinen Schreibfehler kennen kann. Eine AK still stehen zu lassen und den
  Slice als „erfüllt" zu buchen ist **kein** dritter Weg — genau das war der slice-047-Befund.

**Boundary-Wortlaut (Lauf-2-MEDIUM-8):** die bestehende AK ist **zweiwertig** („Rückfrage ‚Änderungen
verwerfen?'"), während 052a an seinen Auslösern **dreiwertig** zusagt (speichern / verwerfen /
abbrechen). Entweder der Slice liefert für „Neu" nur zwei Wege — dann ist die Abweichung zu benennen —,
oder die BLD-001-Boundary wird auf dreiwertig geschärft. Auch das ist eine [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Frage, keine
Implementierungs-Laune.

## 4. Bewusst NICHT Teil

- **Der Sitzungs-Zustand selbst** → [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md).
  Dieser Slice **benutzt** Verdikt, `saveTarget()` und Antwort-Auswertung; er baut sie nicht.
- **Zuletzt-geöffnet-Liste**, **GUI-Export**, **Projektvorlagen** — eigene Schnitte.
- **Projektversionierung** ([`LH-FA-BLD-004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung)).

## 5. Orakel-Schnitt (Entwurf — im [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) zu schärfen)

Jede Zeile muss **außerhalb** des coverage-ausgenommenen `main` prüfbar sein.

| # | Zusicherung | Diskriminierende Gegenprobe | Herkunft |
|---|---|---|---|
| 1 | **Nach „Neu" ist keine Datei mehr gemerkt**: `saveTarget()` verlangt eine Ziel-Abfrage | Reset entfernt ⇒ rot (alte Datei wird zurückgegeben) | **Lauf-2-HIGH-1 / L1** |
| 2 | **Nach „Neu" ist die Sitzung sauber** (Baseline = das neue leere Projekt) | Baseline nicht zurückgesetzt ⇒ rot | L1 |
| 3 | **Das neue Projekt hat genau ein Geschoss** mit der Default-Höhe der Spezifikation | Default-Höhe hart kodiert/abweichend ⇒ rot | AK Happy / L4 |
| 4 | **Nach „Neu" zeichnet der Sink in das neue Geschoss** (Zeichen-Ziel neu aufgelöst) | Neu-Auflösung entfernt ⇒ rot | **L2 (B4-Klasse)** |
| 5 | **Ungesicherter Stand + „Neu" ⇒ `AskFirst`**; „abbrechen" ⇒ das alte Projekt bleibt | Verdikt übersprungen ⇒ rot | AK Boundary |
| 6 | *(abhängig von der §3-Entscheidung)* Negative-AK: kein Schreibrecht ⇒ [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein** Verlust des Vorgängers | Fehlerpfad entfernt ⇒ rot | AK Negative |

**Benannte Grenze:** die modalen Dialoge und die Menü-Verdrahtung — wie in
[`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md), mit derselben Auflage: **in der
Verdrahtung steht keine Entscheidung.**

## 6. Definition of Done (Entwurf)

- [ ] **Projekt-Erzeugung als Kern-Funktion** (L4): leeres `Building` mit genau einem Geschoss,
      Default-Höhe aus der Spezifikation — **nicht** in `main.cpp`.
- [ ] **Sitzungs-Reset** (L1): „Neu" setzt gemerkte Datei **und** Vergleichs-Basis zurück; Orakel §5-1/2.
- [ ] **Zeichen-Ziel neu auflösen** (L2): dieselbe Naht wie beim Öffnen (slice-047
      `DrawingTargetSinks`); Orakel §5-4.
- [ ] **Entscheidung zu L3 getroffen und begründet**: bekommt das Neu-Projekt eine Zeichen-Ebene, oder
      bleibt der Benutzer bis zum manuellen Anlegen ohne Zeichen-Möglichkeit? Die Antwort steht im Plan,
      nicht im Code.
- [ ] **Menü-Aktion „Neu"** mit Rückfrage über die 052a-Maschinerie; **Abbrechen** unterlässt.
- [ ] **§3-Entscheidungen vollzogen**: Negative-AK erfüllt **oder** geschärft; Boundary zwei-/dreiwertig
      geklärt. Bei Lastenheft-Berührung: Header-Version + Historie
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).
- [ ] **Spezifikation §1**: Block für die „Neu"-Mechanik mit **eigenem Anforderungs-Anker**
      (Lauf-2-LOW-3 — die Vorfassung sagte nur einen [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)-Block zu, der die „Neu"-Mechanik
      nicht trägt) + Provenance-Zeile.
- [ ] **Benutzerhandbuch** an den Stellen, die den Funktionsstand aufzählen (Lauf-2-MEDIUM-9):
      **§1 „Heute möglich"**, **§3 „drei Wege"** (die Zählung wird falsch), **§4.1-Tabelle**, **§2.3**
      und **4.3**. Handbuch-Version + Änderungshistorie. (**`docs/user/` steht in dieser DoD-Zeile.**)
- [ ] **CHANGELOG**; **`make gates` grün**; `make schema-check` byte-unberührt.

## 7. Risiken

- **R1 — L1 ist ein Datenverlust-Pfad, kein Komfort-Thema.** Ohne den Reset überschreibt „Speichern"
  nach „Neu" das zuvor geöffnete Projekt. Deshalb sind §5-1/2 **Pflicht**-Orakel.
- **R2 — L3 kollidiert mit „Öffnen ist lesend".** slice-047 hat das stille Anlegen einer Ebene beim
  Öffnen bewusst **zurückgenommen** (Code-Review MEDIUM-8), weil der Speicher-Stand sonst vom
  Dateiinhalt abwich. Beim **Neu**-Projekt gibt es keine Datei — das Argument trägt hier also nicht
  automatisch. Die Entscheidung ist zu treffen, nicht zu übertragen.
- **R3 — Reihenfolge Verdikt → Erzeugung.** Die Rückfrage muss **vor** dem Ersetzen des Stands laufen;
  ein „abbrechen" darf nichts anfassen.

## 8. Trigger

- [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) Lauf 1 (HIGH-2) + Lauf 2 (HIGH-1, MEDIUM-5/-7/-8/-9/-11) zu slice-052; Split-Entscheidung des
  Projektinhabers 2026-07-26.

## 9. Closure-Trigger

- Alle §5-Zeilen grün + je einmal diskriminierend belegt; die §3-Entscheidungen vollzogen und im
  Closure-Text begründet; `make gates` grün.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** mittel — eine Menü-Aktion, aber **vier** Zustands-/Anforderungs-Löcher
  (§2) und zwei offene Anforderungs-Entscheidungen (§3).
- **Risiko:** mittel-hoch — **L1** ist ein Datenverlust-Pfad; **L2** ist eine bereits einmal teuer
  bezahlte Klasse (slice-047 B4).

## 11. Closure-Notiz

_(bei Ausführung auszufüllen)_
