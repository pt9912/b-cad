---
id: slice-055
titel: Fang-Anzeige — sichtbar machen, worauf eingerastet wird ([LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)]
adr_refs: [[ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 055: Fang-Anzeige (DRW-001)

**Status:** open — **eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.**

**Welle:** welle-6-interaktiv-planen. **Nicht trigger-bindend** — schließt aber eine Grenze, die
[`slice-048b`](../done/slice-048b-drw-001-fangpunkte-impl.md) selbst als Risiko **R5** benannt und
ausdrücklich an einen Folge-Slice verwiesen hat.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-28.

## Auslöser

[`slice-048b`](../done/slice-048b-drw-001-fangpunkte-impl.md) hat das Einrasten geliefert — **blind**.
Der Benutzer erfährt erst **nach** dem Loslassen, ob und worauf gefangen wurde. Zwei Folgen, beide im
048b-Closure benannt:

1. **R5 — der Fang reicht über das dargestellte Geschoss hinaus.** `snapTarget` sieht die **ganze**
   `PlanView` (der Spezifikation wörtlich folgend, 048b-Lauf-1-HIGH-1), `canvas_widget.cpp` zeichnet
   nur das **aktive** Geschoss. Der Cursor kann auf einen Punkt einrasten, der **nicht im Bild** ist.
   Ohne Anzeige ist das für den Benutzer ein unerklärlicher Sprung.
2. **Die Fang-Nähe ist unsichtbar.** 12 px sind eine Bedien-Entscheidung; ob man „drin" ist, lässt
   sich heute nur durch Ausprobieren herausfinden.

**Entscheidung des Projektinhabers (2026-07-28): die Anzeige wird nicht zurückgestellt.** Die
048b-Closure hatte sie zusammen mit DRW-002/003 als
[MR-020](../../../../harness/conventions.md)-Deferral notiert; **für die Anzeige ist das aufgehoben**
(Raster/Winkel bleiben zurückgestellt). Begründung: sie ist keine neue Funktion, sondern die
**Beobachtbarkeit einer bereits ausgelieferten** — eine Funktion, die nur durch Ausprobieren
erfahrbar ist, ist nur halb geliefert.

## 1. Ziel

Solange der Mauszeiger über der 2D-Zeichenfläche steht und ein Fang-Punkt in Fang-Nähe liegt, ist
**erkennbar, auf welchen Punkt eingerastet würde** — vor dem Klick, nicht danach. Bewegt sich der
Zeiger aus der Fang-Nähe, verschwindet die Anzeige.

**Das ist eine benutzer-beobachtbare Zusage und gehört darum ins Lastenheft** (nicht in einen
stillen UI-Zusatz): [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) bekommt einen
AK-Konjunkt — **lösungsfrei**
([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei): das
*Was*, nicht Marker-Form, Farbe oder Größe).

## 2. Warum das kein reiner Zeichen-Zusatz ist — und wo die Zusage hängt

**Der Canvas hat heute keinen Hover-Zustand.** `mouseMoveEvent` kehrt sofort zurück, wenn nicht
gezogen wird (`canvas_widget.cpp`), und `setMouseTracking` ist nicht gesetzt — ohne gedrückte Taste
kommt gar kein Move-Ereignis an. Die Anzeige braucht also **beides**: Maus-Verfolgung einschalten und
den aktuellen Fang-Kandidaten halten.

**Damit entsteht neuer Widget-Zustand — bewusst, und mit einer Grenze.** 048b hatte „kein neuer
Zustand" als DoD-Zeile; dieser Slice **fügt genau ein Feld hinzu** (`std::optional<Point2D>
snap_preview_`) und macht es zur **Testnaht**:

```cpp
// display-freier Surrogat-Zustand (Muster E7 der Canvas-ADR, wie screenToModel)
std::optional<hexagon::model::Point2D> snapPreview() const;
```

**Warum ein Surrogat und nicht der Framebuffer:** die Zusage lautet „erkennbar, **worauf**
eingerastet würde" — das ist eine Aussage über den **ausgewählten Punkt**, nicht über Pixel. Ein
Framebuffer-Orakel würde Marker-Form und Farbe einfrieren, also genau die Lösungsmechanik, die
[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) aus der
Anforderung heraushält. Der Surrogat-Zustand ist das etablierte Muster der Canvas-Interaktions-AK
(`test_canvas_widget.cpp`, slice-043/048b).

**Der Preview-Wert wird nicht neu gerechnet, sondern ist derselbe.** Anzeige und Einrasten müssen
**denselben** `snapTarget`-Aufruf-Vertrag benutzen, sonst zeigt der Marker auf A und die Linie landet
auf B. Deshalb ruft der Hover-Pfad **dieselbe** Funktion mit **derselben** Konstante
(`kSnapThresholdPx`) — und **Orakel-Zeile 4 prüft genau diese Gleichheit** an einer Position, an der
beide laufen.

## 3. Bewusst NICHT Teil

- **Raster, Winkelvorgaben** ([`LH-FA-DRW-002`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/003) —
  bleiben zurückgestellt (048b-Closure-Deferral, unverändert).
- **Weitere Fang-Arten** (Schnittpunkt, Mitte, Lot, Tangente) — dito.
- **Eine Anzeige, aus welchem Geschoss der Fang-Punkt stammt.** Die Anzeige macht sichtbar **wo**
  gefangen wird, nicht **woher** der Punkt kommt. Das reicht, um den Sprung erklärbar zu machen;
  eine Geschoss-Kennzeichnung wäre eine eigene Zusage. **Benannte Grenze, kein Versehen.**
- **Eine Bedienmöglichkeit, den Fang abzuschalten oder die Fang-Nähe zu ändern.** Eigene Zusage
  (Einstellungen), eigener Slice.
- **Jede Änderung an Kern, Persistenz, Export, Schema.** Die Anzeige ist reine Darstellung.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **In Fang-Nähe entsteht eine Anzeige** mit **exakt** den mm des Fang-Ziels (Gleichheit, nicht Nähe) | `CanvasWidget::snapPreview`, headless (Xvfb) | Preview auf die Cursor-mm gesetzt ⇒ rot |
| 2 | **Außerhalb der Fang-Nähe gibt es keine Anzeige** (`nullopt`) | `CanvasWidget`, headless | Preview unbedingt gesetzt ⇒ rot |
| 3 | **Die Anzeige folgt der Bewegung**: von „drin" nach „draußen" verschwindet sie wieder | `CanvasWidget`, headless | Preview nur gesetzt, nie gelöscht ⇒ rot |
| 4 | **Angezeigt wird, worauf tatsächlich eingerastet wird** — an derselben Position liefert der Zug exakt den Punkt, den die Anzeige zuvor nannte | `CanvasWidget` (Hover **und** Zug im selben Test) | Hover-Pfad mit anderem Schwellwert/anderer Quelle ⇒ rot |
| 5 | **Maus-Verfolgung ist eingeschaltet** — ohne sie erreicht den Canvas ohne gedrückte Taste kein Ereignis, und die Zusage wäre im Produkt tot, obwohl der Test (der Ereignisse synthetisiert) grün bliebe | `CanvasWidget`, headless | `setMouseTracking(true)` entfernt ⇒ rot |
| 6 | **Der Zeichen-Pfad bleibt unverändert** — freies Zeichnen, Fangen und die Entartungs-Ablehnung aus 043/048b sind unberührt | Bestands-Orakel `test_canvas_widget.cpp` | (Regressions-Netz, keine eigene Gegenprobe) |

**Zeile 5 ist die Zeile, die man vergisst.** Der Interaktions-Test **synthetisiert** `QMouseEvent`s
und stellt sie zu — er würde auch dann grün bleiben, wenn Qt im echten Betrieb nie ein Move-Ereignis
schickte. Die Zusage hängt an einer Widget-**Eigenschaft**, nicht am Ereignis-Pfad; also wird die
Eigenschaft geprüft (`hasMouseTracking()`), nicht nur die Wirkung. Das ist dieselbe Bauart wie die
048b-Zeile 7: die reinen Zeilen können nicht zeigen, dass der Aufruf real erreicht wird.

## 5. Definition of Done

- [ ] **Lastenheft**: [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) bekommt einen
      **Happy-Path-Konjunkt** „erkennbar, auf welchen Punkt eingerastet würde" + eine
      **Boundary**-Zeile „außerhalb der Fang-Nähe keine Anzeige" — **lösungsfrei**
      ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei):
      keine Marker-Form/Farbe/Größe). **Version 0.1.19 → 0.1.20** + oberste Zeile in
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md)
      ([MR-010](../../../../harness/conventions.md)/[MR-012](../../../../harness/conventions.md):
      Header == oberste Historie-Zeile).
- [ ] **Spezifikation §1**: der [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001)`.a`-Block
      bekommt die Mechanik — Hover-Verfolgung, **derselbe** `snapTarget`-Vertrag wie der Zeichen-Pfad,
      Anzeige als **Widget-Zustand** (kein Modell-Datum, kein `op`, kein Schema).
- [ ] **`src/adapters/ui/view/canvas_widget.{h,cpp}`**: Maus-Verfolgung an, `snap_preview_` +
      `snapPreview()`-Naht, Marker im Paint-Pfad; **kein** neuer Port, **kein** neuer `op`;
      Orakel §4-1..5.
- [ ] **`tests/adapters/test_canvas_widget.cpp`** erweitert (Hover in/aus der Fang-Nähe, Gleichheit
      Anzeige↔Zug, `hasMouseTracking`); Bestands-Orakel unverändert grün (§4-6).
- [ ] **Orakel §4-1..5 je mit roter Gegenprobe** im Closure-Text, **einzeln** gemessen.
- [ ] **`make a-check` grün ohne neue Kante**; `data-model.yaml`/`schema.sql` **byte-unberührt**
      (`make schema-check`), kein Kern-/Persistenz-/Export-Diff.
- [ ] **Benutzerhandbuch**: §4.2 nennt die Anzeige; der heutige Satz „Eine Anzeige, **worauf** gerade
      eingerastet wird, gibt es in dieser Version noch nicht" ist zu **ersetzen** (nicht zu ergänzen).
      Handbuch-Version + Änderungshistorie.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` + `spec/lastenheft-historie.md` | ändern | AK-Konjunkt + Version (§5) |
| `spec/spezifikation.md` | ändern | §1-Block `.a` um die Anzeige-Mechanik |
| `src/adapters/ui/view/canvas_widget.{h,cpp}` | ändern | Hover-Zustand + Naht + Marker |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-1..5 |
| `docs/user/benutzerhandbuch.md` | ändern | §4.2 + Version/Historie |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `src/hexagon/**`, `src/adapters/io/**`, `src/adapters/persistence/**`,
`src/adapters/ui/view/snap.{h,cpp}` (die Auswahl-Funktion bleibt **unverändert** — sie wird nur ein
zweites Mal gerufen), `.a-check.yml`/`.d-check.yml`, `data-model.yaml`/`schema.sql`,
`docs/plan/adr/`.

## 7. Risiken

- **R1 — Anzeige und Zug könnten auseinanderlaufen.** Wenn der Hover-Pfad seine eigene Schwelle oder
  eine andere Plan-Quelle bekäme, zeigte der Marker auf einen anderen Punkt als den, der eingerastet
  wird. **Auflösung:** derselbe Aufruf-Vertrag, und Orakel-Zeile 4 prüft die Gleichheit an derselben
  Position — nicht zwei getrennte Tests, die je für sich grün sein können.
- **R2 — der Hover-Pfad pullt bei jeder Mausbewegung die `PlanView`.** Das ist häufiger als die zwei
  Pulls pro Zug aus 048b. **Bewertung vor dem Start zu treffen** (Plan-Review-Frage): reicht der Pull
  je Bewegung, oder ist ein Repaint-gekoppelter Pfad nötig? **Ein Cache ist nicht die Antwort** — er
  wäre der Zustand, den [ADR-0019](../../adr/0019-drw-2d-canvas.md) dem Pull-Widget verwehrt. Falls der Pull zu teuer ist, ist die
  ehrliche Alternative, die Preview **im Paint-Pfad** zu berechnen (dort liegt die `PlanView`
  ohnehin) und `mouseMoveEvent` nur die Cursor-Position merken zu lassen.
- **R3 — die Anzeige zeigt auf leeren Bildschirm.** Genau das ist ihr Zweck (R5 aus 048b): ein
  Fang-Punkt aus einem nicht dargestellten Geschoss bekommt einen Marker, obwohl dort keine Linie
  gezeichnet ist. **Das ist die gewollte Wirkung, kein Darstellungsfehler** — und es ist der erste
  Moment, in dem ein Benutzer die Geschoss-übergreifende Lesart der Spezifikation überhaupt bemerken
  kann. Ob sie **erwünscht** bleibt, ist danach eine Spezifikations-Frage, keine Code-Frage.
- **R4 — `hasMouseTracking` ist eine Qt-Eigenschaft, kein Verhalten.** Zeile 5 prüft eine Zusage
  *über* das Framework. Sie ist trotzdem nötig (s. §4), aber sie ersetzt **nicht** die Zeilen 1–3.

## 8. Trigger

- [`slice-048b`](../done/slice-048b-drw-001-fangpunkte-impl.md) §3 benennt die Fang-Anzeige als
  eigenen Folge-Slice und §7 R5 als das stärkste Argument dafür; die Closure-Notiz führt sie als
  „nächsten sinnvollen Folge-Slice".

## 9. Closure-Trigger

- §4-Zeilen 1–5 grün + je diskriminierend belegt; §4-6 (Bestands-Orakel) unverändert grün;
  `make gates` + `make io-smoke` grün; `make schema-check` byte-unberührt; Lastenheft-Version und
  Handbuch nachgezogen; Closure-Notiz.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** klein — ein Widget-Feld, ein Paint-Zusatz, fünf Orakel-Zeilen. Der
  Aufwand steckt in der **Kopplung** Anzeige↔Zug (R1), nicht im Umfang.
- **Phase-Reife:** Fang-Auswahl und Canvas-Interaktions-Testbarkeit liegen seit 048b bzw. 043.
- **Risiko:** niedrig — additive Darstellung, kein Kern-, Schema- oder Export-Diff; der Zeichen-Pfad
  bleibt unberührt und ist durch die Bestands-Orakel abgesichert.

**Warum AK-Schärfung und Implementierung in EINEM Slice** (Abweichung vom 048a/048b-Muster,
begründet statt stillschweigend): die Schärfung ist **ein** Happy-Konjunkt plus **eine**
Boundary-Zeile über eine Anforderung, die bereits auf AK-Niveau steht — es gibt keine offene
Mechanik-Frage, die ein eigener Schärfungs-Slice erst klären müsste (das war bei 048a anders: dort
war die **ganze** Anforderung Outline). Der Schnitt bleibt eine Sitzung.
**Falls das Plan-Review das anders sieht, ist der Split die billigere Korrektur — vor dem Start.**

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung

_(offen — der Lauf steht vor dem Start aus; HIGHs blockieren ihn.)_

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
