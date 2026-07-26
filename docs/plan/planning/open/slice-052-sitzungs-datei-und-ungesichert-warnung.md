---
id: slice-052
titel: Sitzungs-Datei merken + Warnung vor ungesicherten Änderungen (GUI-Datenverlust-Schutz)
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md)]
---

# Slice 052: Sitzungs-Datei merken + Ungesichert-Warnung

**Status:** open — **Scope-Reservierung** aus der
[slice-047](../done/slice-047-projekt-oeffnen.md)-Validation. Detail-Schnitt + eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
**beim Start**, nicht jetzt.

**Welle:** welle-5-erweiterung. **Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser (Validations-Rest slice-047, 2026-07-26)

Der Projektinhaber hat slice-047 mit **„angenommen mit benanntem Rest"** validiert
([`validator.md`](../../../../.harness/skills/validator.md)). Vier Punkte wurden als Rest benannt;
**zwei davon gehören zusammen** und bilden diesen Slice:

1. **Kein „Speichern" auf die offene Datei.** Es gibt nur **Speichern unter…** — jeder Speichervorgang
   fragt erneut nach einem Pfad. Der zuletzt geöffnete/gespeicherte Pfad wird zwar in den Fenstertitel
   geschrieben, aber **nicht gemerkt**.
2. **Keine Warnung vor ungesicherten Änderungen.** Wer ein anderes Projekt öffnet oder das Fenster
   schließt, verliert seine Änderungen **still** — es gibt keine Rückfrage und keinen Hinweis.

Die beiden hängen an **derselben fehlenden Sache**: die Sitzung hat keinen Zustand „welche Datei bin
ich, und bin ich seit dem letzten Schreiben verändert worden?". Deshalb ein Slice, nicht zwei.

**Warum das ein Datenverlust-Thema ist:** [`harness/README.md`](../../../../harness/README.md)
§Safety nennt Datenverlust am Gebäudemodell den **schärfsten Fehlerfall**. Die bisherige Absicherung
([`LH-QA-005`](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery) Crash-Recovery,
[`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) Atomarität) schützt
gegen Verlust **beim Schreiben**. Hier geht es um den Verlust **ohne** Schreiben — eine Lücke, die
erst entstand, als slice-047 dem Benutzer überhaupt eine Sitzung gab, die er verlieren kann.

## 1. Ziel

Die GUI-Sitzung bekommt zwei benutzer-beobachtbare Eigenschaften:

- **Sie weiß, welche Datei sie ist** → „Speichern" schreibt ohne Pfad-Dialog dorthin zurück;
  „Speichern unter…" bleibt daneben bestehen.
- **Sie weiß, ob sie ungesichert ist** → jede Aktion, die den Sitzungs-Stand verwirft (anderes Projekt
  öffnen, Fenster schließen), fragt vorher nach.

## 2. Lösungsraum (offen — Entscheid beim Start)

**Nicht** vorentschieden. Zwei Punkte sind aber schon jetzt belegbar und sollten den Schnitt leiten:

- **Der „verändert"-Zustand ist bereits beobachtbar.** Der
  [`ModelChangedPort`](../../../../src/hexagon/ports/driven/model_changed_port.h)
  ([ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)) meldet **jede** committete Mutation; ein
  Beobachter, der „seit dem letzten Schreiben kam mindestens eine Meldung" festhält, braucht **keine**
  neue Naht im Kern. Der `ModelReplaced`-Fall aus slice-047 muss den Zustand dabei **zurücksetzen**
  (ein frisch geöffnetes Projekt ist nicht verändert) — das ist der leicht zu übersehende Fall.
- **Wohin gehört der Zustand?** Genau die Frage, an der slice-047 sein blockierendes Verify-Finding
  hatte: was im coverage-ausgenommenen `main.cpp` landet, ist **orakel-los per Konstruktion**. „Welche
  Datei bin ich, bin ich verändert?" ist eine **Entscheidung**, keine Verdrahtung — sie gehört in eine
  testbare Naht (Muster: der `DrawingTargetSinks`-Rückweg aus
  [slice-047](../done/slice-047-projekt-oeffnen.md) §7). In `main` darf bleiben: Dialog, Meldung,
  Verdrahtung.

## 3. Anforderungs-Ebene (Entscheid beim Start)

Die Ungesichert-Warnung hat **heute keine Anforderung**:
[`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) beschreibt das
Speichern selbst, [`LH-QA-005`](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery) den Absturz.
Damit steht dieselbe Frage an wie bei
[`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) in slice-047: **ohne
Kriterium gibt es kein Maß für „erfüllt"**.

Zwei Wege, beide legitim — **Projektinhaber-Entscheidung beim Start**:

- **(a) AK-Ergänzung an [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)**
  (Muster [slice-048a](../done/slice-048a-drw-001-fangpunkte-ak-spec.md) / slice-047):
  eine Boundary-Klausel „Given ungesicherte Änderungen, when Sitzung verwerfen, then Rückfrage".
  Lösungsfrei nach [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei),
  Header-Version nachziehen ([MR-010](../../../../harness/conventions.md)).
- **(b) Als reine UX-Härtung ohne Lastenheft-Berührung** führen und die Grenze in der Closure-Notiz
  benennen. Billiger, aber die Zusage bleibt unmessbar.

**Empfehlung: (a).** Der Rest-Punkt kam aus einer **Validation** — also aus der Bedarfs-Ebene; genau
dort gehört er verankert. slice-047 hat gezeigt, was (b) kostet: die Traceability-Korrektur war für
die AK-lose Anforderung nachträglich nicht belegbar.

## 4. Bewusst NICHT Teil

- **GUI-Export** und **Zuletzt-geöffnet-Liste** — die beiden anderen Rest-Punkte der
  slice-047-Validation. Sie hängen **nicht** am Sitzungs-Zustand und würden den Schnitt sprengen;
  eigene Slices.
- **Projektversionierung** ([`LH-FA-BLD-004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung))
  und **Undo/Redo** ([`LH-QA-003`](../../../../spec/lastenheft.md#lh-qa-003--undoredo)) — „ungesichert"
  ist eine **Ja/Nein**-Eigenschaft der Sitzung, keine Historie. Die Abgrenzung ist im Lastenheft schon
  als offen vermerkt und wird hier **nicht** aufgelöst.
- **Auto-Save / Wiederherstellungs-Datei.** Anderer Mechanismus, anderes Risiko.

## 5. Risiken und offene Punkte

- **R1 — schmale Wirkung, ehrlich zu benennen.** Die **einzige** Modell-Mutation, die dem GUI-Benutzer
  heute offensteht, ist das Zeichnen von Hilfslinien
  ([ADR-0019](../../adr/0019-drw-2d-canvas.md) v1, so auch im
  [slice-047-Verify](../../../reviews/2026-07-25-slice-047-verify.md) §3 vermerkt). Die
  Ungesichert-Warnung schützt also vorerst **eine** Art von Änderung. Sie wird trotzdem gebraucht,
  bevor das interaktive Bauteil-Zeichnen kommt — aber der Slice darf ihren Wert nicht überzeichnen.
- **R2 — der Rücksetz-Fall.** Öffnen, Speichern **und** Speichern unter… setzen „ungesichert" zurück;
  ein **gescheitertes** Speichern **nicht**. Wer das verwechselt, baut genau den stillen Verlust ein,
  den der Slice verhindern soll → braucht ein eigenes Orakel.
- **R3 — Fenster-Schließen ist der schwerste Pfad.** Die Rückfrage beim Schließen hängt am
  Qt-Schließ-Ereignis und damit am modalen, **sensorlosen** Teil
  ([slice-047](../done/slice-047-projekt-oeffnen.md) B5). Der Plan muss beim Schnitt sagen, welcher
  Anteil ein Orakel bekommt und welcher als benannte Grenze bleibt — **nicht** erst die Verifikation.
- **R4 — „Speichern" ohne Dialog ist ein neuer Datenverlust-Pfad.** Es überschreibt eine bestehende
  Datei **ohne Rückfrage** (genau das ist der Zweck). Die Atomarität aus
  [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) trägt das bereits
  — der Slice darf sie nicht umgehen (kein direkter Schreibpfad am Use-Case vorbei).

## 6. Trigger

- **Validations-Rest** aus der [slice-047](../done/slice-047-projekt-oeffnen.md)-Closure (2026-07-26,
  Rolle Projektinhaber). Kein Gate-Befund, kein Review-Finding — ein **Bedarfs**-Befund.

## 7. Closure-Trigger

- Beide Eigenschaften benutzer-beobachtbar + **orakel-gedeckt außerhalb** des coverage-ausgenommenen
  `main`; `make gates` grün; falls Weg (a): Lastenheft-Header + Historie nachgezogen; Handbuch-Abschnitt
  4.3 nachgeführt (**die Doku-DoD-Zeile führt `docs/user/`** — Lehre aus slice-047 V1); Closure-Notiz.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** mittel. **Phase-Reife:** die Persistenz-Mechanik ist reif (welle-1) und der
  Aufruf-Pfad seit slice-047 vorhanden — dieser Slice ergänzt **Sitzungs-Zustand**, keine neue Mechanik.
- **Risiko:** mittel — Datenverlust-nah (R2/R4), und der schwerste Pfad liegt im sensorlosen
  Fenster-Ereignis (R3).

## 9. Closure-Notiz

_(bei Ausführung auszufüllen)_
