---
id: slice-055
titel: Fang-Anzeige — sichtbar machen, worauf eingerastet wird ([LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)]
adr_refs: [[ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 055: Fang-Anzeige (DRW-001)

**Status:** open — **zwei
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe
durch** (Lauf 1: 1 HIGH · Lauf 2: 1 HIGH · **Lauf 3: 0 HIGH / 4 MED / 6 LOW / 2 INFO —
„startbar"**); Einarbeitung in §11/§11a/§11b. **Lauf 3 hat die zwei Sensoren, an denen die
Vorläufe rissen, am Artefakt nachgerechnet und für tragfähig befunden.**

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

Steht der Mauszeiger über der 2D-Zeichenfläche und liegt ein Fang-Punkt in Fang-Nähe, ist **erkennbar,
auf welchen Punkt eingerastet würde** — **bevor** der Punkt gesetzt wird, nicht danach. Das gilt in
**beiden** Phasen, in denen der Fang wirkt: beim Setzen des **Anfangs** (ohne gedrückte Taste) **und**
beim Führen des **Endes** (während des Zugs). Liegt kein Fang-Punkt in Reichweite — oder verlässt der
Zeiger die Zeichenfläche —, gibt es keine Anzeige.

**Die Zwei-Phasen-Zusage ist keine Ausweitung, sondern die Bedingung dafür, dass die AK stimmt**
(Lauf-1-MEDIUM-1): `canvas_widget.cpp` fängt an **zwei** Stellen — Press (Anfang) und Release (Ende) —,
und [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) sagt beides zu („Anfang **oder**
Ende in Fang-Nähe"). Eine Anzeige, die nur den Zustand ohne gedrückte Taste abdeckt, machte den ins
Lastenheft geschriebenen Konjunkt für die Hälfte der Bestands-AK falsch.

**Das ist eine benutzer-beobachtbare Zusage und gehört darum ins Lastenheft** (nicht in einen
stillen UI-Zusatz): [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) bekommt einen
AK-Konjunkt — **lösungsfrei**
([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei): das
*Was*, nicht Marker-Form, Farbe oder Größe).

## 2. Wo die Zusage hängt — und warum sie zwei Sensoren braucht

**Der Canvas hat heute keinen Hover-Zustand.** `mouseMoveEvent` kehrt sofort zurück, wenn nicht
gezogen wird (`canvas_widget.cpp`), und `setMouseTracking` ist repo-weit **nirgends** gesetzt — ohne
gedrückte Taste kommt gar kein Move-Ereignis an. Die Anzeige braucht also **beides**: Maus-Verfolgung
einschalten und den aktuellen Fang-Kandidaten halten.

**Damit entsteht neuer Widget-Zustand — bewusst, und mit einer Grenze.** 048b hatte „kein neuer
Zustand" als DoD-Zeile; dieser Slice **fügt genau ein Feld hinzu** (`std::optional<Point2D>
snap_preview_`) und macht es zur **Testnaht**:

```cpp
// display-freier Surrogat-Zustand (Muster E3/E7 der Canvas-ADR, wie screenToModel)
std::optional<hexagon::model::Point2D> snapPreview() const;
```

### Zwei Sensoren, weil die Zusage zwei Hälften hat (Lauf-1-HIGH-1)

Die Vorfassung buchte **drei** Liefergegenstände (Maus-Verfolgung · Preview-Naht · **Marker im
Paint-Pfad**) auf fünf Orakel-Zeilen, von denen **keine** `paintEvent` berührte. Alle fünf wären grün
geblieben, wenn im Paint-Pfad **nichts** gezeichnet würde — während genau das der Konjunkt ist, den
derselbe Slice ins Lastenheft schreibt. Die Vorfassung begründete den Verzicht mit
[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei); **das
trägt nicht**: [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) regelt, was im **Lastenheft-Text** stehen darf, nicht, was ein Test zusichern
darf. Ein Framebuffer-Orakel existiert in diesem Repo bereits (`test_viewer_widget.cpp`,
`grabFramebuffer`).

**Die Zusage zerfällt darum in zwei Hälften, und jede bekommt ihren eigenen Sensor:**

| Hälfte | Sensor | Warum nicht der andere |
|---|---|---|
| **welcher Punkt** würde gefangen | `snapPreview()`-Surrogat, exakte mm | Ein Tinten-Vergleich kann nicht sagen, **welcher** Punkt gemeint ist |
| **dass überhaupt etwas erscheint** | **Tinten-Sonde** auf einem offscreen gerenderten Widget-Bild (`QWidget::render` in ein `QImage`, Muster der PNG-Export-Orakel) | Der Surrogat bliebe grün, wenn nie ein Pixel gesetzt würde |

**Die Kontrolle der Tinten-Sonde variiert den CURSOR, nicht das Modell** (Lauf-2-HIGH-1). Die erste
Einarbeitung wollte „dasselbe Widget an derselben Zeigerposition, einmal **mit** und einmal **ohne**
Fang-Punkt in Reichweite" vergleichen. **Das ist nicht herstellbar, ohne die Störgröße mitzuändern:**
Fang-Punkte **sind** die Endpunkte der `PlanView`-Segmente — ein Modell ohne Fang-Punkt in Reichweite
zeichnet andere Segmente **und** bekommt über `ViewTransform::fit` einen anderen Maßstab. Die
Tinten-Differenz wäre dann nicht dem Marker zurechenbar, und die zugesagte rote Gegenprobe folgte
nicht. Die berufene Präzedenz hält genau diese Störgröße ausdrücklich konstant
(`test_png_export.cpp`: „Gleiche Wand-BBox in beiden Fällen … identischer Maßstab").

**Also andersherum: dieselbe `PlanView`, dieselbe Transformation, zwei Zeiger-Positionen** — eine
**innerhalb**, eine **außerhalb** der Fang-Nähe eines Endpunkts. Gezeichnete Segmente und Maßstab
sind dann **byte-gleich**, der einzige Unterschied ist der Marker. Form, Farbe und Größe bleiben
offen (nur „mehr Tinte im ersten Fall") — die Sonde friert die Gestalt weiterhin nicht ein.

**Und der Marker braucht einen Auslöser.** Der Canvas repaintet heute nur bei `setActiveStorey`,
`onModelChanged`, Press/Move-im-Zug, Release und `wheelEvent`. Eine Anzeige, die der Bewegung folgt,
verlangt ein `update()` im Hover-Zweig — sonst wäre auch ein korrekt gezeichneter Marker unsichtbar.
Das ist eine eigene DoD-Zeile, keine Selbstverständlichkeit.

### Der Preview-Wert ist derselbe wie der gefangene

Anzeige und Einrasten müssen **denselben** `snapTarget`-Aufruf-Vertrag benutzen, sonst zeigt der
Marker auf A und die Linie landet auf B. Deshalb ruft der Hover-Pfad **dieselbe** Funktion mit
**derselben** Konstante (`kSnapThresholdPx`) — und **Orakel-Zeile 4 prüft genau diese Gleichheit** an
einer Position, an der beide laufen.

### Wo die Preview gerechnet wird — entschieden, nicht dem Review überlassen (Lauf-1-MEDIUM-3)

**Entscheidung: im Maus-Ereignis, mit einem `pull_()` an Ort und Stelle** — dieselbe Bauform, die 048b
für Press und Release gewählt hat. Die Vorfassung stellte diese Frage im Risiko-Abschnitt dem Review,
während DoD und Datei-Tabelle sie bereits fixiert hatten; das ist genau die Bewegung, die der
053-Review als „eingearbeitet ≠ entschieden" gerügt hat.

**Die Kosten werden benannt, nicht kleingerechnet:** eine Zeiger-Bewegung erzeugt damit **zwei**
Projektionen — den Hover-Pull und den Pull des ausgelösten Repaints. Jede iteriert
`storeys × (walls + guide_lines)` und baut ein `unordered_set`. **Für diese Frage existiert in diesem
Repo kein Sensor** ([AGENTS §3](../../../../AGENTS.md) führt keinen Performance-Gate), sie ist also
eine Urteilsfrage; das Kriterium der Entscheidung ist **Korrektheit vor Sparsamkeit**: ein
zwischengespeicherter Plan könnte veralten, und ein veralteter Fang-Punkt ist ein falscher Marker.
**Kein ADR-Argument:** [ADR-0019](../../adr/0019-drw-2d-canvas.md) verbietet einen Cache **nicht** —
sie verortet Aid-Zustand sogar ausdrücklich im Widget (Entscheidung 6). Die Vorfassung hat eine
048b-**Slice**-Entscheidung auf ADR-Rang gehoben (Lauf-1-LOW-3); das ist korrigiert. Wird die
Doppel-Projektion je spürbar, ist die Auflösung eine **gemessene** (Preview im Paint-Pfad, Cursor-
Position als Feld) — nicht eine vermutete.

## 3. Bewusst NICHT Teil

- **Raster, Winkelvorgaben** ([`LH-FA-DRW-002`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/003) —
  bleiben zurückgestellt (048b-Closure-Deferral, unverändert).
- **Weitere Fang-Arten** (Schnittpunkt, Mitte, Lot, Tangente) — dito.
- **Eine Anzeige, aus welchem Geschoss der Fang-Punkt stammt.** Die Anzeige macht sichtbar **wo**
  gefangen wird, nicht **woher** der Punkt kommt. Das reicht, um den Sprung erklärbar zu machen;
  eine Geschoss-Kennzeichnung wäre eine eigene Zusage. **Benannte Grenze, kein Versehen.**
- **Eine Bedienmöglichkeit, den Fang abzuschalten oder die Fang-Nähe zu ändern.** Eigene Zusage
  (Einstellungen), eigener Slice.
- **Eine Vorschau der entstehenden Hilfslinie** (Gummiband auf das Fang-Ziel statt auf den Cursor).
  Der Zug zeichnet weiter die Bestands-Linie zwischen den zwei **Pixeln** — sichtbar wird das
  Fang-**Ziel**, nicht die gefangene Linie. Eigene Zusage, hier ausdrücklich nicht mitgeliefert.
- **Jede Änderung an Kern, Persistenz, Export, Schema.** Die Anzeige ist reine Darstellung.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **In Fang-Nähe entsteht eine Anzeige** mit **exakt** den mm des Fang-Ziels (Gleichheit, nicht Nähe) | `CanvasWidget::snapPreview`, headless (Xvfb) | Preview auf die Cursor-mm gesetzt ⇒ rot |
| 2 | **Außerhalb der Fang-Nähe gibt es keine Anzeige** (`nullopt`) | `CanvasWidget`, headless | Preview unbedingt gesetzt ⇒ rot |
| 3 | **Die Anzeige folgt der Bewegung**: von „drin" nach „draußen" verschwindet sie wieder | `CanvasWidget`, headless | Preview nur gesetzt, nie gelöscht ⇒ rot |
| 4 | **Angezeigt wird, worauf tatsächlich eingerastet wird** — an derselben Position liefert der Zug exakt den Punkt, den die Anzeige zuvor nannte | `CanvasWidget` (Hover **und** Zug im selben Test) | Hover-Pfad mit anderem Schwellwert/anderer Quelle ⇒ rot |
| 5 | **Maus-Verfolgung ist eingeschaltet** — ohne sie erreicht den Canvas ohne gedrückte Taste kein Ereignis, und die Zusage wäre im Produkt tot, obwohl der Test (der Ereignisse synthetisiert) grün bliebe | `CanvasWidget::hasMouseTracking`, headless | `setMouseTracking(true)` entfernt ⇒ rot |
| 6 | **Auch während des Zugs** (gedrückte Taste) zeigt die Preview das Fang-Ziel des **Endes** an | `CanvasWidget`, headless | Preview nur im `!dragging_`-Zweig gepflegt ⇒ rot |
| 7 | **Der Zeiger außerhalb der Zeichenfläche zeigt nichts an** — **beide** Wege: `QEvent::Leave` ohne gedrückte Taste **und** eine Zug-Bewegung mit Koordinaten außerhalb `rect()` (dort stellt Qt **kein** `Leave` zu, die Move-Ereignisse laufen weiter — Lauf-2-MEDIUM-2) | `CanvasWidget`, headless | je Weg einzeln entfernt ⇒ rot. **Der zweite Weg braucht eine eigene Fixture** (Lauf-3-MEDIUM-1): nach `ViewTransform::fit` liegt **jeder** Fang-Punkt ≥ 15 px vom Rand (`kMargin = 0.9` ⇒ 5 % Rand je Seite, bei 300 px Höhe = 15 px) — bei 12 px Fang-Nähe ist außerhalb `rect()` **nie** ein Punkt in Reichweite. Die Fixture muss **hineinzoomen** (`wheelEvent`), damit ein Punkt über den Rand wandert |
| 8 | **Es wird wirklich etwas gezeichnet:** **dieselbe** `PlanView` und **dieselbe** Transformation, zwei Zeiger-Positionen — innerhalb der Fang-Nähe trägt das gerenderte Bild **mehr Tinte** als außerhalb. **Nur in der Hover-Phase** (`!dragging_`): im Zug zeichnet der Paint-Pfad die cursor-abhängige in-Arbeit-Linie, dann ist der Marker nicht mehr der einzige Unterschied (Lauf-3-MEDIUM-4) | **Paint-Pfad**, offscreen-Render + Tinten-Sonde | Marker-Zeichnung aus `paintEvent` entfernt ⇒ rot (Zeilen 1–7 blieben grün) |
| 9 | **Die Bewegung löst einen Repaint aus** — ohne ihn bliebe auch ein korrekt gezeichneter Marker im laufenden Betrieb unsichtbar | **Zähl-Callable in der `PlanPull`-Naht**: **eine einzelne** Hover-Bewegung + `processEvents()` ergibt **zwei** Pulls (Hover-Auswertung + ausgelöster Repaint). **Zwei Bedingungen** (Lauf-3-LOW-5): der Zähler ist **nach dem Show-Paint zurückzusetzen**, und die Bewegungen sind **einzeln** zu messen — Qt fasst mehrere `update()` zu **einem** Paint zusammen (n Bewegungen ⇒ n+1 Pulls, nicht 2n) | `update()` im Hover-Zweig entfernt ⇒ **ein** Pull ⇒ rot |
| 10 | **Die Anzeige veraltet nicht bei einer Transformations-Änderung**: Zoom, Resize, Geschoss-Wechsel und Modell-Meldung repainten **ohne** Maus-Ereignis — die in mm gehaltene Preview zeigte danach auf eine falsche Bildschirmstelle | `CanvasWidget`, headless | Invalidierung in `wheelEvent`/`resizeEvent`/`setActiveStorey`/`onModelChanged` entfernt ⇒ rot |
| 10a | **Nach einer Ansichts-Änderung ist die Anzeige weg, obwohl der Fang weiter wirkt** — sie kehrt mit der **nächsten Zeiger-Bewegung** zurück. Das ist die Kehrseite von Zeile 10 und muss in der AK stehen, sonst ist der Happy-Konjunkt in einem erreichbaren Zustand falsch (Lauf-3-MEDIUM-2) | `CanvasWidget`, headless | Rückkehr nach der nächsten Bewegung entfernt ⇒ rot |
| 11 | **Der Zeichen-Pfad bleibt unverändert** — freies Zeichnen, Fangen und die Entartungs-Ablehnung aus 043/048b sind unberührt | Bestands-Orakel `test_canvas_widget.cpp` | (Regressions-Netz, keine eigene Gegenprobe) |

**Zeile 5 ist die Zeile, die man vergisst — Zeile 8 die, die man für erledigt hält.** Der
Interaktions-Test **synthetisiert** `QMouseEvent`s; er bliebe grün, wenn Qt im echten Betrieb nie ein
Move-Ereignis schickte (Zeile 5) **und** er bliebe grün, wenn der Marker nie gezeichnet würde
(Zeile 8). Beide Zeilen prüfen, was der Surrogat strukturell nicht sehen kann. Der Unterschied zum
Surrogat aus 043/048b ist real und war der Grund des HIGH: dort ist der Surrogat
`service.building().guide_lines`, also **Modell**-Zustand mit weiterer Beobachtbarkeit über Persistenz
und Export; `snapPreview()` hätte ohne Zeile 8 **keinen** gedeckten Produktions-Konsumenten.

## 5. Definition of Done

- [ ] **Lastenheft**: [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) bekommt einen
      **Happy-Path-Konjunkt** — „**bevor** der Punkt gesetzt wird, ist erkennbar, auf welchen Punkt
      eingerastet würde, für Anfang **wie** Ende" — und eine **Boundary**-Zeile „liegt kein Punkt in
      Fang-Nähe oder verlässt der Zeiger die Zeichenfläche, gibt es keine Anzeige". **Lösungsfrei**
      ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei):
      keine Marker-Form/Farbe/Größe). Die **zeitliche Bedingung** („bevor") ist Teil der Zusage, nicht
      Beiwerk (Lauf-1-LOW-1).
- [ ] **Der Happy-Path-Halbsatz „Endpunkt P *auf der Zeichenfläche*" ist mitzuschärfen**
      (Lauf-2-MEDIUM-4, **verortet durch Lauf-3-MEDIUM-3**). Die erste Einarbeitung wollte das Wort
      „**sichtbar**" schärfen — **das war der falsche Ort**: die Anforderung **definiert** „sichtbar"
      bereits über die Ebene („Fangbar sind nur sichtbare Punkte; die Sichtbarkeit richtet sich nach
      der Ebene"). Der tragende Widerspruch steckt in „**auf der Zeichenfläche**": slice-048b fängt
      über **alle** Geschosse, also auch auf Punkte, die **nicht dargestellt** sind — und dieser Slice
      macht genau das mit einem Marker sichtbar. **Zu entscheiden und zu schreiben, nicht zu
      verschieben.** *(Nebenbefund, bereits aus 048b bekannt und hier nicht zu heilen: der
      Ebenen-Filter greift in `projectPlan` **nur bei Hilfslinien** — Wand-Achsen tragen keine
      `layer_id`.)*
- [ ] **Lastenheft-Version + Historie** ([MR-010](../../../../harness/conventions.md)/[MR-012](../../../../harness/conventions.md)):
      Header == Version der neu ergänzten Zeile in
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md). **Ausgangsstand heute
      0.1.19; die Zielnummer ist die dann nächste freie** — sie wird beim Vollzug festgestellt, nicht
      hier fixiert, weil [`slice-056`](../open/slice-056-wand-im-canvas-adr-ak.md) ebenfalls schärft und die
      Reihenfolge offen ist (Lauf-1-LOW-5). **Platzierung: unmittelbar nach der `0.1.16`-Zeile** — die
      Tabelle ist **nicht** monoton sortiert, „oberste Zeile" wäre wörtlich befolgt falsch
      (Lauf-1-LOW-2). **Der Sensor dieser Zeile ist die
      [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse,
      nicht `make docs-check`** — d-check prüft keine Feld-Gleichheit.
- [ ] **Spezifikation §1**: der [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001)`.a`-Block
      bekommt die Mechanik — Hover-Verfolgung **und Zug-Phase**, **derselbe** `snapTarget`-Vertrag wie
      der Zeichen-Pfad, Anzeige als **Widget-Zustand** (kein Modell-Datum, kein `op`, kein Schema).
- [ ] **`src/adapters/ui/view/canvas_widget.{h,cpp}`**: Maus-Verfolgung an, `snap_preview_` +
      `snapPreview()`-Naht, **Pflege in beiden Phasen** (Hover **und** Zug), **`leaveEvent`** löscht,
      **`update()`** im Hover-Zweig; **kein** neuer Port, **kein** neuer `op`. Orakel §4-1..7 + 9.
- [ ] **Die Preview wird bei jeder Transformations-Änderung invalidiert** —
      `wheelEvent`/`resizeEvent`/`setActiveStorey`/`onModelChanged` repainten **ohne** Maus-Ereignis;
      ein in mm gehaltener Marker zeigte danach auf die falsche Bildschirmstelle. **Das ist die
      `drag_start_mm_`-Lehre aus 043, hier umgekehrt** (Lauf-2-MEDIUM-3); Orakel §4-10.
- [ ] **Marker im Paint-Pfad** — **eigene DoD-Zeile mit eigenem Sensor** (§4-8, Tinten-Sonde auf
      offscreen gerendertem Widget-Bild, **Kontrolle über die Zeiger-Position**, nicht über das
      Modell). **Nicht** mit den Surrogat-Zeilen gebündelt (Lauf-1-HIGH-1/Lauf-2-HIGH-1).
- [ ] **`tests/adapters/test_canvas_widget.cpp`** erweitert (inkl. **Zähl-Callable** in der
      `PlanPull`-Naht für §4-9); Bestands-Orakel unverändert grün (§4-11).
- [ ] **Orakel §4-1..10a je mit roter Gegenprobe** im Closure-Text, **einzeln** gemessen.
- [ ] **`make a-check` grün ohne neue Kante**; **kein** Kern-/Persistenz-/Export-Diff, belegt am
      `git diff --stat` der Closure (`make schema-check` prüft Schema-Drift, **nicht**
      Unberührtheit — Lauf-1-Präzedenz aus dem 056-Report).
- [ ] **Benutzerhandbuch**: §4.2 nennt die Anzeige; der heutige Satz „Eine Anzeige, **worauf** gerade
      eingerastet wird, gibt es in dieser Version noch nicht" ist zu **ersetzen** (nicht zu ergänzen).
      Handbuch-Version + Änderungshistorie.
- [ ] **[ADR-Index](../../adr/README.md)**: die [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Folgepflichtzeile beschreibt slice-055 heute
      als „als Plan geschnitten (in `open/`)" — beim Vollzug auf „erfüllt" nachziehen
      ([MR-020](../../../../harness/conventions.md), Lauf-1-LOW-4).
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf — **als DoD-Zeile**, nicht nur in der Datei-Tabelle
      (Lauf-1-MEDIUM-4: **drittes** Vorkommen desselben Musters in diesem Strang).
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` + `spec/lastenheft-historie.md` | ändern | AK-Konjunkte + Version (§5) |
| `spec/spezifikation.md` | ändern | §1-Block `.a` um die Anzeige-Mechanik |
| `src/adapters/ui/view/canvas_widget.{h,cpp}` | ändern | Hover- und Zug-Zustand, `leaveEvent`, Außerhalb-Fall, Invalidierung, `update()`, Marker |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-1..10 inkl. Tinten-Sonde + Pull-Zähler |
| `docs/user/benutzerhandbuch.md` | ändern | §4.2 + Version/Historie |
| `docs/plan/adr/README.md` | ändern | Folgepflichtzeile auf „erfüllt" |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | je [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf |

**Nicht berührt:** `src/hexagon/**`, `src/adapters/io/**`, `src/adapters/persistence/**`,
`src/adapters/ui/view/snap.{h,cpp}` (die Auswahl-Funktion bleibt **unverändert** — sie wird nur ein
zweites Mal gerufen), `.a-check.yml`/`.d-check.yml`, `data-model.yaml`/`schema.sql`,
`docs/plan/adr/0019-drw-2d-canvas.md` (der ADR-**Text**; nur der Index wird nachgezogen).

## 7. Risiken

- **R1 — Anzeige und Zug könnten auseinanderlaufen.** Wenn der Hover-Pfad seine eigene Schwelle oder
  eine andere Plan-Quelle bekäme, zeigte der Marker auf einen anderen Punkt als den, der eingerastet
  wird. **Auflösung:** derselbe Aufruf-Vertrag, und Orakel-Zeile 4 prüft die Gleichheit an derselben
  Position — nicht zwei getrennte Tests, die je für sich grün sein können.
- **R2 — zwei Projektionen je Zeiger-Bewegung.** Benannt und **entschieden** (§2), nicht offen:
  Korrektheit vor Sparsamkeit, weil ein veralteter Plan einen falschen Marker erzeugt. **Es gibt
  dafür kein Maß in diesem Repo** — wird es spürbar, ist die Auflösung eine gemessene.
- **R3 — die Anzeige zeigt auf leeren Bildschirm.** Genau das ist ihr Zweck (R5 aus 048b): ein
  Fang-Punkt aus einem nicht dargestellten Geschoss bekommt einen Marker, obwohl dort keine Linie
  gezeichnet ist. **Das ist die gewollte Wirkung, kein Darstellungsfehler** — und der erste Moment, in
  dem ein Benutzer die Geschoss-übergreifende Lesart der Spezifikation überhaupt bemerken kann.
  **Die Vorfassung schob die Klärung auf „danach"; das ist zurückgenommen** (Lauf-2-MEDIUM-4): der
  Slice schärft das Wort „sichtbar" der DRW-001-Negative **mit** (DoD), weil der Marker den
  Widerspruch sonst erst sichtbar **macht** und dann offenlässt. Ob die Geschoss-übergreifende Lesart
  auf Dauer **erwünscht** ist, bleibt eine Spezifikations-Frage — aber sie ist dann **gestellt**, nicht
  verschwiegen.
- **R4 — `hasMouseTracking` ist eine Qt-Eigenschaft, kein Verhalten.** Zeile 5 prüft eine Zusage
  *über* das Framework. Sie ist nötig, ersetzt aber **nicht** die Zeilen 1–3.
- **R5 — die Tinten-Sonde könnte zu grob sein.** Sie belegt „es wird gezeichnet", nicht „an der
  richtigen Stelle". Das ist bewusst: die Stelle sichert Zeile 1 über die mm, die Form bleibt
  ausdrücklich offen ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)). Wer beides in einem Orakel will, friert die Marker-Gestalt ein.
- **R6 — [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  wird für diesen Slice ausdrücklich VERNEINT.** 048a hatte die Pflicht für den Fang-Faden bejaht
  („Fang-Distanz/Nächster-Punkt ist Geometrie"). Dieser Slice rechnet **nichts** Geometrisches neu —
  er ruft `snapTarget` ein zweites Mal und zeichnet das Ergebnis. **Die Aussage steht hier, damit sie
  eine Aussage ist** und kein Schweigen (Lauf-1-INFO-2); die Pflicht greift ohnehin erst vor der
  Welle-Closure.

## 8. Trigger

- [`slice-048b`](../done/slice-048b-drw-001-fangpunkte-impl.md) §3 benennt die Fang-Anzeige als
  eigenen Folge-Slice und §7 R5 als das stärkste Argument dafür; die Closure-Notiz führt sie als
  „nächsten sinnvollen Folge-Slice".

## 9. Closure-Trigger

- §4-Zeilen 1–10a grün + je diskriminierend belegt; §4-11 (Bestands-Orakel) unverändert grün;
  `make gates` + `make io-smoke` grün; kein Kern-/Schema-/Export-Diff am `git diff --stat` belegt;
  Lastenheft-Version, Handbuch und ADR-Index nachgezogen; Closure-Notiz.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** klein-mittel — ein Widget-Feld, drei Ereignis-Zweige, ein Paint-Zusatz,
  neun Orakel-Zeilen. Der Aufwand steckt in der **Kopplung** Anzeige↔Zug (R1) und in der Zwei-Sensor-
  Deckung (§2), nicht im Umfang.
- **Phase-Reife:** Fang-Auswahl und Canvas-Interaktions-Testbarkeit liegen seit 048b bzw. 043.
- **Risiko:** niedrig — additive Darstellung, kein Kern-, Schema- oder Export-Diff; der Zeichen-Pfad
  bleibt unberührt und ist durch die Bestands-Orakel abgesichert.

**Warum AK-Schärfung und Implementierung in EINEM Slice** (Abweichung vom 048a/048b-Muster,
begründet statt stillschweigend): die Schärfung ist **ein** Happy-Konjunkt plus **eine**
Boundary-Zeile über eine Anforderung, die bereits auf AK-Niveau steht — es gibt keine offene
Mechanik-Frage, die ein eigener Schärfungs-Slice erst klären müsste (das war bei 048a anders: dort
war die **ganze** Anforderung Outline). Präzedenz im Repo: `slice-052b` hat Lastenheft-Schärfung und
Implementierung ebenfalls in einem Schnitt geliefert. **Vom ersten Review bestätigt** („der
Ein-Schnitt-Entscheid trägt").

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-07-28)

Report: [`2026-07-28-slice-055-plan.md`](../../../reviews/2026-07-28-slice-055-plan.md) —
**1 HIGH / 4 MEDIUM / 5 LOW / 2 INFO, „nicht startbar"**. Unabhängiger Reviewer ≠ Plan-Autor.

| # | Behandlung |
|---|---|
| **HIGH-1** (die DoD buchte „Marker im Paint-Pfad" auf Orakel §4-1..5, von denen keine `paintEvent` berührt; die [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Begründung für den Verzicht trägt nicht, weil [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) den Lastenheft-**Text** regelt, nicht Test-Zusagen) | **Die Zusage ist in zwei Hälften zerlegt und jede bekommt einen eigenen Sensor** (§2): *welcher* Punkt → Surrogat; *dass etwas erscheint* → **neue Zeile 8**, Tinten-Sonde auf einem offscreen gerenderten Widget-Bild (Muster `test_png_export.cpp`, friert die Marker-Form **nicht** ein). Dazu **neue Zeile 9**: die Bewegung muss überhaupt einen Repaint auslösen — der Canvas repaintet heute im Hover-Zweig nicht. Der Marker hat jetzt eine **eigene DoD-Zeile**. |
| **MEDIUM-1** (die Anzeige war nur für den Hover vor dem Press geplant; das **Ende** wird während des Zugs gefangen, und die Bestands-AK sagt „Anfang **oder** Ende" zu) | §1 sagt die Anzeige jetzt für **beide Phasen** zu, **neue Orakel-Zeile 6** prüft sie bei gedrückter Taste, die DoD verlangt die Pflege in beiden Zweigen. Ohne das wäre der ins Lastenheft geschriebene Konjunkt für die halbe AK falsch gewesen. |
| **MEDIUM-2** (der die Fläche verlassende Zeiger kam nirgends vor; der letzte Marker bliebe stehen) | **Neue Orakel-Zeile 7** (`leaveEvent`, `QEvent::Leave`) + DoD-Zeile; die Boundary-AK nennt den Fall jetzt ausdrücklich. |
| **MEDIUM-3** (R2 stellte dem Review eine Frage, die DoD und Datei-Tabelle bereits entschieden hatten; die Pull-Zahl war zu niedrig angesetzt) | **Entschieden statt gefragt** (§2): Berechnung im Maus-Ereignis mit `pull_()` an Ort und Stelle. Die **zwei** Projektionen je Bewegung stehen benannt da, ebenso, dass es **kein Maß** dafür gibt und die Entscheidung darum eine Urteilsfrage ist (Kriterium: Korrektheit vor Sparsamkeit). R2 ist vom Fragezeichen zur Feststellung geworden. |
| **MEDIUM-4** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report in §6, aber nicht in der DoD — **drittes** Vorkommen nach 048a-LOW-3 und 048b-LOW-4) | **Als DoD-Zeile ergänzt.** Die 3×-Regel ist erfüllt: der Musterbefund gehört ins Harness-Steering (Report-Pflicht als Gate — [`slice-051`](../open/slice-051-review-artefakt-pflicht.md) trägt genau dieses Thema), nicht in einen vierten Einzelfall. **Hier festgehalten, damit der Zähler nicht verlorengeht.** |
| **LOW-1** (der AK-Wortlaut verlor die zeitliche Bedingung aus §1) | „**bevor** der Punkt gesetzt wird" ist jetzt Teil des DoD-Wortlauts. |
| **LOW-2** („oberste Zeile" ist gegen die reale Tabelle nicht ausführbar — sie läuft aufsteigend bis 0.1.16 und trägt danach 0.1.19/0.1.18/0.1.17) | DoD nennt die **Platzierung** konkret („unmittelbar nach der `0.1.16`-Zeile") statt die [MR-010](../../../../harness/conventions.md)-Formulierung blind zu erben. |
| **LOW-3** (R2 schrieb [ADR-0019](../../adr/0019-drw-2d-canvas.md) ein Cache-Verbot zu, das ihr Text nicht enthält) | **Widerrufen und korrigiert:** die Aussage stammt aus der 048b-**Slice**-Entscheidung, nicht aus der ADR; [ADR-0019](../../adr/0019-drw-2d-canvas.md) verortet Aid-Zustand sogar ausdrücklich im Widget. |
| **LOW-4** (ADR-Index-Folgepflichtzeile beschreibt 055 als „in `open/`" und würde mit der Closure stale) | **Neue DoD-Zeile** + `docs/plan/adr/README.md` in der Datei-Tabelle; nur der **Index**, nicht der ADR-Text. |
| **LOW-5** (die fixierte Zielversion 0.1.20 kollidiert mit dem parallel offenen slice-056) | Die Zielnummer ist **nicht mehr fixiert** — „die dann nächste freie, beim Vollzug festgestellt". Ausgangsstand 0.1.19 bleibt als Tatsache benannt. |
| **INFO-1** (E7 vs. E3 vermischt) | Kommentar sagt jetzt „E3/E7", wie der Bestands-Code. |
| **INFO-2** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) kommt im Plan nicht vor, obwohl 048a die Pflicht für den Faden bejaht hat) | **Als R6 ausdrücklich verneint, mit Begründung** — dieser Slice rechnet nichts Geometrisches neu. Eine Aussage statt eines Schweigens. |

**Positiv bestätigt** (nicht neu prüfen): alle Aussagen über den heutigen Canvas stimmen
(`setMouseTracking` repo-weit nirgends gesetzt · `mouseMoveEvent` kehrt ohne Zug zurück · Paint nur
aktives Geschoss vs. `snapTarget` über alle Geschosse) · das zu ersetzende Handbuch-Zitat existiert
wörtlich · Ausgangsversion **0.1.19** ist real und die [MR-010](../../../../harness/conventions.md)-Invariante hält heute · die AK-Schärfung
bleibt **lösungsfrei** · „keine neue a-check-Kante" trägt · die Gegenproben zu §4-1..5 sind im
Bestands-Testaufbau herstellbar · der **Ein-Schnitt-Entscheid (§10) trägt**.

**Startbar nach Lauf 1:** nein — ein HIGH verlangt einen zweiten unabhängigen Lauf. Der Auftrag an ihn
lautete: prüfen, ob die **Tinten-Sonde** (Zeile 8) im Bestands-Testaufbau headless herstellbar ist —
sie ist die einzige Zusage dieses Plans ohne Präzedenz am `CanvasWidget`. **Er hat genau dort
zugeschlagen.**

## 11a. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (zweiter Lauf, 2026-07-28)

Report: [`2026-07-28-slice-055-plan-2.md`](../../../reviews/2026-07-28-slice-055-plan-2.md) —
**1 HIGH / 4 MEDIUM / 5 LOW / 2 INFO, „nicht startbar"**. Unabhängiger Reviewer ≠ Plan-Autor ≠
Reviewer des ersten Laufs.

**Sein Urteil über die erste Einarbeitung: echt bei 11 von 12 Befunden** — entschieden statt
umformuliert, **kein** Befund in den Vollzug delegiert, alle neu hinzugekommenen Bestands-Behauptungen
am Artefakt wahr. Die Ausnahme ist der HIGH selbst.

| # | Behandlung |
|---|---|
| **HIGH-1** (die neue Tinten-Sonde trägt ihren eigenen Kontroll-Aufbau nicht: „mit vs. ohne Fang-Punkt in Reichweite" ist nur über eine **andere `PlanView`** herstellbar, und die ändert die gezeichneten Segmente **und** über `ViewTransform::fit` den Maßstab — die Tinten-Differenz ist nicht dem Marker zurechenbar; die berufene Präzedenz hält genau diese Störgröße konstant) | **Die Kontrolle ist umgedreht:** dieselbe `PlanView`, dieselbe Transformation, **zwei Zeiger-Positionen** (innerhalb/außerhalb der Fang-Nähe). Gezeichnete Segmente und Maßstab sind damit identisch, der einzige Unterschied ist der Marker (§2). **Die Form der Einarbeitung stimmte, die Kontrolle nicht** — genau die Bauart, die dieses Repo aus 052a kennt („eine Gegenprobe, die nichts rot macht"). |
| **MEDIUM-1** (Zeile 9 nannte keinen Beobachtungspunkt; `CanvasWidget` ist `final`, ein Test kann `paintEvent` nicht zählen, `QWidget` exponiert keinen Repaint-Zustand) | **Sensor benannt:** ein **Zähl-Callable in der `PlanPull`-Naht`**. Eine Hover-Bewegung + `processEvents()` ergibt **zwei** Pulls (Hover-Auswertung + ausgelöster Repaint); ohne `update()` bleibt es **einer**. Kein Eingriff in die Klasse nötig. |
| **MEDIUM-2** (die neu zugesagte Zug-Phase öffnet einen Zustand, den Zeile 7 nicht erreicht: bei gedrückter Taste laufen Move-Ereignisse mit Koordinaten **außerhalb** der Fläche weiter, und `QEvent::Leave` kommt dort nicht) | Zeile 7 deckt jetzt **beide** Wege — `Leave` **und** Zug-Bewegung außerhalb `rect()`, je einzeln als Gegenprobe. Die Erweiterung aus Lauf 1 hatte ihre eigene Lücke mitgebracht. |
| **MEDIUM-3** (`snap_preview_` ist ein reiner mm-Wert; `wheelEvent`/`resizeEvent`/`setActiveStorey`/`onModelChanged` repainten **ohne** Maus-Ereignis → der Marker veraltet) | **Neue Orakel-Zeile 10 + DoD-Zeile:** Invalidierung bei jeder Transformations-Änderung. **Das ist die `drag_start_mm_`-Lehre aus 043, hier umgekehrt** — dort musste ein Wert in mm **gehalten** werden, hier muss er **fallen**. |
| **MEDIUM-4** (der neue AK-Konjunkt geht unqualifiziert in eine Anforderung, deren Negative „keinen **sichtbaren** fangbaren Punkt" sagt — während 048b über **alle** Geschosse fängt; R3 verschob die Klärung auf „danach") | **R3 ist zurückgenommen und die Klärung in die DoD gezogen:** „sichtbar" meint die **Ebenen**-Sichtbarkeit (was `projectPlan` filtert), nicht „im Bild dargestellt". **Ein Slice, der einen Widerspruch erst sichtbar macht, darf ihn nicht offenlassen** — und „danach" war genau die Bewegung, die der 053-Review als „eingearbeitet ≠ entschieden" gerügt hat. |

**Kein Lauf-1-Befund musste widerlegt werden** — der zweite Lauf hat keinen der zwölf für falsch
gehalten.

**Startbar nach Lauf 2:** nein. Auftrag an den dritten Lauf: **ist die Tinten-Sonde in ihrer neuen
Kontroll-Form headless herstellbar und diskriminierend — und hält der Pull-Zähler?**

## 11b. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (dritter Lauf, 2026-07-28)

Report: [`2026-07-28-slice-055-plan-3.md`](../../../reviews/2026-07-28-slice-055-plan-3.md) —
**0 HIGH / 4 MEDIUM / 6 LOW / 2 INFO, Verdikt „startbar"**. Unabhängiger Reviewer ≠ Plan-Autor ≠ beide
Vor-Reviewer.

**Die zwei Sensoren, an denen die Vorläufe rissen, halten — am Artefakt nachgerechnet, nicht
plausibilisiert:**

- **Tinten-Sonde:** gleiche `PlanView` ⇒ gleiches `ViewTransform::fit`; im `!dragging_`-Zustand zeichnet
  `paintEvent` **nichts** Cursor-Abhängiges — ohne Marker sind beide Bilder **byte-gleich**, die rote
  Gegenprobe folgt aus dem Aufbau. Headless herstellbar, weil der Canvas ein reines `QWidget` mit
  `QPainter` ist (**kein GL** — anders als der `grabFramebuffer`-Pfad des Viewers); die Testdatei steht
  bereits in `tests/CMakeLists.txt`, **keine** CMake-Änderung nötig.
- **Pull-Zähler:** `pull_` ist konstruktor-injiziert und wird an **genau zwei** Stellen gerufen
  (Paint, Snap) — `final` versperrt hier nichts; `update()` + `processEvents()` löst headless einen
  Paint aus (Bestands-Beleg im Canvas-Test).

| # | Behandlung |
|---|---|
| **MEDIUM-1** (Orakel-Zeile 7, zweiter Weg: nach `fit` liegt **jeder** Fang-Punkt ≥ 15 px vom Rand — `kMargin = 0.9` ⇒ 5 % je Seite, bei 300 px Höhe = 15 px; bei 12 px Fang-Nähe ist außerhalb `rect()` **nie** ein Punkt in Reichweite, die zugesagte Gegenprobe also in der Repo-Fixture **nicht herstellbar**) | Zeile 7 sagt jetzt, dass der zweite Weg eine **eigene Fixture** braucht: **hineinzoomen** (`wheelEvent`), bis ein Punkt über den Rand wandert. **Nachgerechnet, nicht geschätzt** — genau die Bauart Beleg, die dieses Repo von einer Gegenprobe verlangt. |
| **MEDIUM-2** (die neue Invalidierung löscht die Anzeige nach einem Zoom **ohne** Maus-Ereignis, während `snappedModelPos` weiter fängt → der unqualifizierte Happy-Konjunkt ist in einem erreichbaren Zustand falsch) | **Neue Orakel-Zeile 10a** + AK-Konjunkt: nach einer Ansichts-Änderung ist die Anzeige weg und kehrt mit der **nächsten Zeiger-Bewegung** zurück. **Die Kehrseite von Zeile 10 gehört in dieselbe Zusage** — sonst heilt die Invalidierung einen Fehler und erzeugt einen zweiten. |
| **MEDIUM-3** (die „sichtbar"-Schärfung zielt am Problem vorbei: die Anforderung **definiert** „sichtbar" bereits über die Ebene; der tragende Widerspruch steckt in „Endpunkt P **auf der Zeichenfläche**") | **Verortung korrigiert.** Die DoD schärft jetzt den richtigen Halbsatz. **Ein Befund, der beim zweiten Lauf richtig war und beim dritten präziser wurde** — die Lauf-2-Behandlung war nicht falsch, nur am falschen Wort. |
| **MEDIUM-4** (die Kontroll-Bedingung der Sonde gilt nur bei `!dragging_`: im Zug zeichnet der Paint-Pfad die cursor-abhängige in-Arbeit-Linie — der Plan sagt die Anzeige aber für **beide** Phasen zu) | Zeile 8 ist ausdrücklich auf die **Hover-Phase** eingeschränkt. Die Zug-Phase bleibt über die Zeilen 1/4/6 (Surrogat) gedeckt — **die Tinten-Sonde kann sie strukturell nicht tragen**, und das steht jetzt da, statt unausgesprochen zu gelten. |
| **LOW-5** (der Pull-Zähler braucht Bedingungen: Reset nach dem Show-Paint; Qt fasst mehrere `update()` zu **einem** Paint zusammen ⇒ n Bewegungen ergeben n+1, nicht 2n Pulls) | Beide Bedingungen stehen jetzt in Zeile 9. |

**Urteil des Laufs über die Lauf-2-Einarbeitung: überwiegend echt, nicht vollständig** — drei von fünf
aufgelöst (der HIGH tragend), einer halb, einer am falschen Ort; **kein** Befund in den Vollzug
delegiert, aber zwei neue Widersprüche erzeugt (MEDIUM-2/4). **Die LOW/INFO des zweiten Laufs waren in
§11a nicht behandelt** — nachgeholt, soweit sie den Text betreffen.

**Startbar: JA.** Kein HIGH. Die vier MEDIUM sind hier eingearbeitet; ein **vierter Lauf ist nicht
fällig** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start):
nur HIGHs blockieren). **Der Plan hat drei Läufe gebraucht, und alle drei fanden etwas Echtes** — die
zwei ersten am selben Punkt: dem Sensor für „es wird wirklich gezeichnet".

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
