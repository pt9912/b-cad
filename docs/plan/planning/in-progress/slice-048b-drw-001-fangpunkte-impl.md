---
id: slice-048b
titel: Fangpunkte implementieren — Eingabe-Quantisierung im 2D-Canvas ([LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 048b: Fangpunkte implementieren (DRW-001)

**Status:** open — Implementierung zur AK-/Spec-Schärfung aus
[`slice-048a`](../done/slice-048a-drw-001-fangpunkte-ak-spec.md). **Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.**

**Welle:** welle-6-interaktiv-planen. **Nicht trigger-bindend** — der Wellen-Trigger ist „eine Wand ist
im 2D-Canvas zeichenbar und parametrisch änderbar"; Fangen wirkt in dieser Ausbaustufe auf
**Hilfslinien**. Der Slice gehört trotzdem hierher: **präzises Zeichnen ist die Voraussetzung dafür,
dass Zeichnen im Canvas überhaupt etwas taugt**, und die Naht, die er baut, trägt später das
Bauteil-Zeichnen. Er verlängert die Welle **nicht** (Lehre der welle-5-Closure §5-1).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-27.

## Auslöser

[`slice-048a`](../done/slice-048a-drw-001-fangpunkte-ak-spec.md) hat
[`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) auf AK-Niveau geschärft
(Lastenheft 0.1.16) und den §1-Block
[`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001)`.a` geschrieben — **Doku ohne Code**.
Der Benutzer kann seither eine Hilfslinie ziehen ([`slice-043`](../done/slice-043-drw-canvas-impl.md)),
aber nur **frei**: jeder Endpunkt landet auf der ungefähren Cursor-Position. Zwei Hilfslinien exakt
aneinanderzusetzen ist damit Glückssache.

**Dieser Slice liefert, was 048a beschrieben hat — nicht mehr.**

## 1. Ziel

Beim interaktiven Zeichnen einer Hilfslinie rastet die Eingabe auf **Endpunkte sichtbarer
Wand-Achsen und sichtbarer Hilfslinien** ein, wenn der Cursor nah genug ist — Anfang **wie** Ende.
Die erzeugte Hilfslinie trägt dann **exakt** die mm des Fang-Ziels, nicht „nahe dran".

**Die Spezifikation hat den Mechanismus bereits entschieden** (§1-Block
[`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001)`.a`) — dieser Plan erfindet ihn nicht
neu, er setzt ihn um:

| Was | Entschieden in 048a |
|---|---|
| **Heimat** | UI-Interaktions-Zustand des **Canvas**, kein Modell-Datum |
| **Quelle der Fang-Punkte** | dieselbe `PlanView`, die der Canvas ohnehin fürs Rendering **pullt** — keine neue Naht, kein neuer Port, keine neue Schicht-Kante |
| **Auswahl** | der **nächstgelegene** Punkt in Fang-Nähe; bei gleicher Distanz die **feste `PlanView`-Iterationsreihenfolge** |
| **Unsichtbare Ebenen** | `projectPlan` filtert sie **vor** der `PlanView` heraus → strukturell nicht fangbar. **Gilt nur für Hilfslinien** (Lauf-1-LOW-2): Wände tragen gar keine `layer_id` und werden bedingungslos aufgenommen (`plan_projection.cpp`:46–54) — für sie ist die Zusage **gegenstandslos**, nicht erzwungen |
| **Benachrichtigung** | **kein** neuer `op`, keine zusätzliche `ModelChanged`-Meldung |
| **Ablehnung** | **kein** neuer Fehlerfall; Anfang = Ende trifft die bestehende Entartungs-Ablehnung |

**Am Artefakt geprüft (2026-07-27), bevor dieser Plan darauf baut:**
`src/hexagon/services/geometry/plan_projection.cpp` schiebt je Geschoss **Wand-Achsen (:46–53) vor
Hilfslinien (:58–69)** und prüft die Ebenen-Sichtbarkeit (`visible.contains`, :62) **vor** dem
Einfügen. Die in 048a ausgeschriebene Tie-Break-Regel und die „strukturell nicht fangbar"-Zusage sind
also real gedeckt — nicht nur behauptet. **Mit einer Einschränkung** (Lauf-1-LOW-2): der
Sichtbarkeits-Filter greift **nur bei Hilfslinien**; Wand-Achsen tragen keine Ebenen-Zuordnung und
sind immer Teil der Projektion.

## 2. Wo der Fang liegt — und warum nicht im Widget-Rumpf

**Der Fang wird eine reine Funktion in einer eigenen Datei** (`src/adapters/ui/view/snap.{h,cpp}`),
nicht ein Block in `CanvasWidget::mousePressEvent`:

```cpp
// display-frei, ohne QWidget, ohne QApplication
std::optional<hexagon::model::Point2D> snapTarget(
    const hexagon::model::PlanView& plan, const ViewTransform& transform,
    QPoint cursor_px, double threshold_px);
```

**Kein Geschoss-Parameter — entschieden 2026-07-27 (Lauf-1-HIGH-1).** Die erste Fassung dieses Plans
verengte die Kandidaten auf das **dargestellte** Geschoss und behauptete zugleich „kein
Spezifikations-Eintrag". Beides zusammen ging nicht:
[`spezifikation.md`](../../../../spec/spezifikation.md):1048–1051 definiert die Fang-Punkte über die
**ganze** `PlanView` („Endpunkte der Wand-Achsen **je Geschoss**"), und der Tie-Break führt eigens
„**Geschosse in Speicherreihenfolge**" (:1059) — ein Konjunkt, das unter einer Verengung unerreichbar
wäre.

**Projektinhaber-Entscheidung: der Spezifikation wörtlich folgen.** `snapTarget` sieht die **ganze**
`PlanView`; die Spezifikation bleibt **unberührt**, der Tie-Break bleibt **vollständig** prüfbar.

**Die Konsequenz wird benannt, nicht verschwiegen** (R5): `canvas_widget.cpp`:60 **zeichnet** nur das
aktive Geschoss. Der Cursor kann damit auf einen Punkt einrasten, der auf dem Bildschirm **nicht zu
sehen** ist. Das ist die bewusst gewählte Lesart der Spezifikation — und der Grund, warum eine
**Fang-Anzeige** (§3) der erste sinnvolle Folge-Slice ist: sie macht sichtbar, worauf gerastet wurde.

**Begründung — die Lehre aus welle-5 §5-2, hier vorab angewandt:** die Zusagen dieses Slice sind
Aussagen über **Auswahl** (nächstgelegener Punkt, Tie-Break, Schwellwert, Sichtbarkeit). Liegen sie im
Widget-Rumpf, sind sie nur über einen Qt-Ereignis-Zug prüfbar — und jede Gegenprobe müsste über
`QApplication` + `sendEvent` laufen. Als freie Funktion sind sie **direkt** und **ohne Qt-Fixture**
diskriminierend, in derselben Bauform wie `ViewTransform` (header-nah, display-frei, [ADR-0019](../../adr/0019-drw-2d-canvas.md) E3).

**Was im Widget bleibt:** der Aufruf an den beiden Stellen, an denen heute `screenToModel` steht
(Press = Anfang, Release = Ende), und die Fang-Nähe als **Widget-Konstante**. Das ist Verdrahtung,
keine Entscheidung.

**Woher der Canvas dort die `PlanView` nimmt** (Lauf-1-MEDIUM-1): heute pullt er sie **nur im
Paint-Pfad** (`canvas_widget.cpp`:47) — Press (:76–86) und Release (:97–114) haben keine.
**Entscheidung: an beiden Stellen `pull_()` rufen**, so wie es der Paint-Pfad ohnehin bei jedem
Repaint tut. Ein zwischengespeicherter Plan wäre **neuer Zustand** und könnte veralten (der Canvas
ist ausdrücklich ein Pull-Widget, [ADR-0019](../../adr/0019-drw-2d-canvas.md)); zwei Pulls pro Maus-Zug sind gegenüber einem Pull pro
Frame kostenlos. **Die DoD-Zusage „kein neuer Zustand" bleibt damit wahr** — sie war in der
Vorfassung nicht geprüft, sondern angenommen.

**Die Fang-Nähe** ist eine **Bildschirm**-Konstante (px), keine §3-Bauteil-Konstante — so von 048a
festgelegt („der konkrete Default gehört in den Impl-Slice"). Vorschlag: **12 px**, weil das der
üblichen Treffer-Toleranz einer Maus-Interaktion entspricht und bei jedem Zoom gleich bleibt (der
Fang wirkt im Bildschirmraum, nicht in mm — sonst wäre er bei weit herausgezoomter Ansicht
unbrauchbar groß). Der Wert steht als benannte Konstante neben `snapTarget`; **die Orakel prüfen
Verhalten relativ zum Schwellwert, nicht den Wert selbst** — er ist ein Bedien-Default, keine Zusage.

## 3. Bewusst NICHT Teil

- **Raster** ([`LH-FA-DRW-002`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)),
  **Winkelvorgaben** ([`003`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)),
  **Bemaßung** ([`004`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)) — eigene
  Slices; 048a führt sie ausdrücklich als offene Reifephase-Grenze.
- **Schnittpunkt-, Mittelpunkt-, Lot-, Tangenten-Fang** — dito.
- **Fangen beim Bauteil-Zeichnen** — es gibt noch kein Bauteil-Zeichnen; das ist der Wellen-Trigger
  von welle-6 und ein eigener Slice.
- **Eine Fang-Anzeige** (Marker unter dem Cursor, Hervorheben des Ziels). Sinnvoll, aber eigene
  Zusage mit eigener Beobachtbarkeit — **nicht** stillschweigend mitliefern. Falls gewünscht: eigener
  Folge-Slice.
- **Jede Änderung an Kern, Persistenz, Export.** Eine gefangene Hilfslinie ist eine **gewöhnliche**
  Hilfslinie (048a §1).

## 4. Orakel-Schnitt — **jede Zeile nennt die Komponente, an der sie diskriminiert**

*(Die Spalte „wo" ist keine Formalie: in welle-5 blieb dreimal eine Gegenprobe grün, weil ein Orakel
die falsche Komponente prüfte — 053, 052a, 052b. Regel-Kandidat der welle-5-Closure §5-2.)*

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Innerhalb der Fang-Nähe rastet es exakt ein**: Cursor nahe einem Achsen-Endpunkt ⇒ `snapTarget` liefert **genau** dessen mm (Gleichheit, nicht Nähe) | `snapTarget`, rein | Rückgabe auf die Cursor-mm geändert ⇒ rot |
| 2 | **Außerhalb wird nicht gefangen** ⇒ `nullopt` | `snapTarget`, rein | Schwellwert-Prüfung entfernt ⇒ rot |
| 3 | **Nächstgelegener gewinnt** bei mehreren Kandidaten in Reichweite | `snapTarget`, rein | „erster Treffer gewinnt" statt Minimum ⇒ rot |
| 4 | **Tie-Break ist die `PlanView`-Reihenfolge**: bei exakt gleicher Distanz gewinnt der **zuerst besuchte** Punkt — je Segment **Anfang vor Ende** (Konjunkt 3) | `snapTarget`, rein | Iteration umgedreht / `<=` statt `<` beim Minimum ⇒ rot |
| 5 | **Alle Geschosse der `PlanView` sind Kandidaten**, und bei gleicher Distanz gewinnt das **früher gespeicherte** Geschoss (Tie-Break-Konjunkt 1) | `snapTarget`, rein | Geschoss-Schleife auf das erste beschränkt / Reihenfolge gedreht ⇒ rot |
| 6 | **Unsichtbare Ebene ⇒ kein Fang** — die Hilfslinie ist gar nicht in der `PlanView` | `projectPlan`, Kern | Sichtbarkeits-Filter entfernt ⇒ rot. **Bestands-Orakel, verifiziert:** `tests/hexagon/test_plan_projection.cpp`:79–85 prüft ihn diskriminierend (ohne `plan_projection.cpp`:62–64 fielen Segment-Zahl **und** beide BBox-Grenzen) — R4 ist damit **erledigt**, die Zeile braucht keine Ergänzung |
| 6a | **Wand-Achsen vor Hilfslinien** in der Projektions-Reihenfolge (Tie-Break-Konjunkt 2) | `projectPlan`, Kern | Einfüge-Reihenfolge in `plan_projection.cpp` gedreht ⇒ rot |
| 7 | **Der Zug fängt wirklich**: Maus-Zug endet in Fang-Nähe ⇒ die erzeugte Hilfslinie trägt **exakt** die Ziel-mm | `CanvasWidget`, headless (Xvfb) | `snapTarget`-Aufruf im Release-Pfad entfernt ⇒ rot |
| 8 | **Anfang wird ebenso gefangen** wie das Ende | `CanvasWidget`, headless | Aufruf im Press-Pfad entfernt ⇒ rot |
| 9 | **Entartung**: fängt Anfang **und** Ende denselben Punkt ⇒ **keine** Hilfslinie, Modell unverändert, **kein** neuer Fehlerfall | `CanvasWidget` + Kern-Bestandsregel | die bestehende Ablehnung umgangen ⇒ rot |
| 10 | **Die gefangene Koordinate überlebt Speichern/Laden identisch** (der von der Spezifikation als *harter Nachweis* benannte AK-Konjunkt) | Bestands-Sensoren, **benannt statt neu gebaut** | s. Absatz unten |

**Zeile 7 ist die Zusammenspiel-Zeile** — sie belegt, was die reinen Zeilen 1–5 **nicht** können: dass
der Canvas die Funktion an der richtigen Stelle und mit den richtigen Argumenten ruft. Die reinen
Zeilen allein blieben grün, wenn der Aufruf fehlte; genau dieser Fehler-Typ ist in welle-5 dreimal
aufgetreten.

**Der Tie-Break zerfällt in drei Konjunkte — und sie liegen an zwei Orten** (Lauf-1-MEDIUM-2). Die
Vorfassung buchte alle drei auf `snapTarget`; das geht nicht, weil `PlanSegment`
(`src/hexagon/model/plan_view.h`:14–19) **keinen Unterscheider** Wand↔Hilfslinie trägt. `snapTarget`
sieht eine flache Liste und kann nur „**zuerst besucht** gewinnt" zusichern (Zeilen 4/5); **dass**
diese Reihenfolge Wand-Achsen vor Hilfslinien führt, entsteht in `projectPlan` und wird dort geprüft
(Zeile 6a). Beide Hälften zusammen ergeben erst die Zusicherung der Spezifikation.

**Zeile 10 wird nicht neu gebaut, sondern begründet auf Bestands-Sensoren gestützt** (Lauf-1-MEDIUM-3).
Die Zusicherung folgt **aus der Bauform**: `snapTarget` gibt die mm des Fang-Ziels **unverändert
weiter** (Kopie aus der `PlanView`, keine Rundung, keine Umrechnung — Zeile 1 prüft genau diese
Gleichheit), und eine gefangene Hilfslinie ist danach eine **gewöhnliche** Hilfslinie. Ihr
Round-Trip ist seit [`slice-032b`](../done/slice-032b-drw-impl.md) durch die
`guide_lines`-Persistenz-Orakel gedeckt und seit [`slice-032c`](../done/slice-032c-drw-export.md)
durch die 2D-Export-Orakel. **Vor dem Start verifiziert (R6, 2026-07-28): sie prüfen wertgleich** —
Persistenz auf allen vier Koordinaten beider Hilfslinien, DXF auf den vier Gruppen-Codes. **Kein
eigener Test nötig**; die Grenze (PDF/PNG belegen nur das Erscheinen, nicht die Koordinate) steht
benannt in R6.

**Benannte Grenze:** die **visuelle** Rückmeldung (ob der Benutzer *sieht*, dass gefangen wird) ist
nicht Gegenstand — es gibt sie in diesem Slice nicht (§3).

## 5. Definition of Done

- [ ] **`src/adapters/ui/view/snap.{h,cpp}`**: `snapTarget(plan, transform, cursor_px, threshold_px)`
      + benannte Fang-Nähe-Konstante. Display-frei, **kein** `QWidget`, **kein** Port-Include,
      **kein** Geschoss-Parameter (§2); Orakel §4-1..5.
- [ ] **`CanvasWidget` ruft sie** an den zwei bestehenden `screenToModel`-Stellen (Press/Release),
      **mit einem `pull_()` an Ort und Stelle** (§2 — dort liegt heute keine `PlanView`).
      **Sonst unverändert** — kein neuer Zustand, kein neues Signal; Orakel §4-7/8.
- [ ] **Kein neuer `op`, keine zusätzliche Meldung** — geprüft und **verneint** (048a §Benachrichtigung).
- [ ] **`tests/adapters/test_snap.cpp`** (neu, ohne Qt-Fixture) + Erweiterung von
      `tests/adapters/test_canvas_widget.cpp` (Zug mit Fang, Xvfb) + **`tests/hexagon/test_plan_projection.cpp`**
      (Zeile 6a, Projektions-Reihenfolge); alle Listen in `tests/CMakeLists.txt`, `snap.cpp` in
      `src/adapters/CMakeLists.txt`.
- [ ] **Orakel §4-1..10 (inkl. 6a) je mit roter Gegenprobe** im Closure-Text — Zeile 10 über die
      **benannten Bestands-Sensoren**, deren Wertgleichheits-Deckung nach R6 **vor** dem Start
      verifiziert ist.
- [ ] **`make a-check` grün ohne neue Kante** — `ui_view → model` besteht; `snap.h` importiert
      `model/plan_view.h` + `model/point2d.h` und **keinen** Port.
- [ ] **`data-model.yaml`/`schema.sql` byte-unberührt** (`make schema-check`), **kein** Kern-,
      Persistenz- oder Export-Diff — eine gefangene Hilfslinie ist eine gewöhnliche.
- [ ] **Benutzerhandbuch**: §4.2 („Hilfslinie zeichnen") beschreibt das Einrasten, §1 „Heute möglich"
      nennt es. Handbuch-Version + Änderungshistorie. (**`docs/user/` steht in dieser DoD-Zeile** —
      Lehre aus slice-047-V1: dieser Slice macht eine Anforderung benutzer-erfüllbar.)
- [ ] **Kein Lastenheft-, kein Spezifikations-Eintrag** — beide hat
      [`slice-048a`](../done/slice-048a-drw-001-fangpunkte-ak-spec.md) geliefert; die
      Fang-Nähe ist eine **Widget**-Konstante und gehört ausdrücklich **nicht** in §3.
      (Geprüft und **verneint**, nicht vergessen.)
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/view/snap.{h,cpp}` | neu | die reine Auswahl-Funktion + Fang-Nähe-Konstante (§2) |
| `src/adapters/ui/view/canvas_widget.{h,cpp}` | ändern | Aufruf an den zwei `screenToModel`-Stellen; sonst unverändert |
| `src/adapters/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | beide Listen zählen Dateien **explizit** auf |
| `tests/adapters/test_snap.cpp` | neu | §4-1..5 — **ohne** Qt-Fixture |
| `tests/adapters/test_canvas_widget.cpp` | ändern | §4-7/8/9 (Zug mit Fang, Xvfb) |
| `tests/hexagon/test_plan_projection.cpp` | **ändern** | §4-6a (Wand-Achsen **vor** Hilfslinien) — neu. §4-6 ist dort **bereits diskriminierend gedeckt** (:79–85, R4 verifiziert) und braucht nichts |
| `docs/user/benutzerhandbuch.md` | ändern | §4.2 + §1 + Version/Historie |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `src/hexagon/**` (außer ggf. dem Test zu §4-6), `src/adapters/io/**`,
`src/adapters/persistence/**`, `.a-check.yml`/`.d-check.yml` (keine neue Kante),
`data-model.yaml`/`schema.sql`, `spec/lastenheft.md`, `spec/spezifikation.md`, `docs/plan/adr/`.

## 7. Risiken

- **R1 — der Fang-Radius ist ein Bedien-Wert, kein Vertrag.** Wer ihn zur Zusage macht, friert eine
  Bedien-Entscheidung ein. Die Orakel prüfen **relativ** zum Schwellwert (innerhalb/außerhalb), nie
  den px-Wert.
- **R2 — Bildschirm- vs. Modellraum.** Der Fang muss im **Bildschirm**raum messen; in mm wäre er bei
  herausgezoomter Ansicht unbrauchbar groß und bei hineingezoomter unbrauchbar klein. Der Zoom-Zustand
  steckt in `ViewTransform` — der wird übergeben, nicht nachgebaut.
- **R3 — die Naht mitten im Zug.** `slice-043` hat teuer gelernt, dass der Zug-Startpunkt in **mm**
  gemerkt werden muss, weil Zoom/Resize mitten im Zug die Transformation ändern
  (`drag_start_mm_`, [MR-009](../../../../harness/conventions.md)-LOW-2). Der Fang beim **Press** muss deshalb **das gefangene mm**
  festhalten — nicht das Pixel und nicht den ungefangenen Wert.
- **R4 — Bestands-Orakel zu §4-6. → ERLEDIGT (Lauf 1 verifiziert).**
  `tests/hexagon/test_plan_projection.cpp`:79–85 prüft den Sichtbarkeits-Filter **diskriminierend**:
  ohne `plan_projection.cpp`:62–64 fielen Segment-Zahl **und** beide BBox-Grenzen. Die Negative-AK
  ruht **nicht** auf einer Annahme; die §6-Zeile bleibt für §4-6 unberührt.
- **R5 — der Fang reicht über das dargestellte Geschoss hinaus.** Bewusst so entschieden (§2, der
  Spezifikation folgend): der Cursor kann auf einen Punkt einrasten, der auf dem Bildschirm nicht zu
  sehen ist. **Benannte Grenze, kein Fehler** — und das stärkste Argument für eine **Fang-Anzeige**
  als Folge-Slice (§3). Wird das als störend empfunden, ist die Auflösung eine
  **Spezifikations-Schärfung**, nicht ein stiller Filter im Code.
- **R6 — Zeile 10 stützt sich auf Bestands-Sensoren. → ERLEDIGT (vor dem Start verifiziert,
  2026-07-28).** Am Artefakt geprüft, nicht angenommen:

  | Sensor | Was er prüft | wertgleich? |
  |---|---|---|
  | `tests/adapters/test_sqlite_project_repository.cpp`:328–344 ([032b](../done/slice-032b-drw-impl.md)) | `EXPECT_DOUBLE_EQ` auf **alle vier** Koordinaten **beider** Hilfslinien; `storey_id ≠ layer_id` fängt zusätzlich einen Spalten-Swap | **ja** |
  | `tests/adapters/test_dxf_export.cpp`:114–131 ([032c](../done/slice-032c-drw-export.md)) | DXF-Gruppen 10/20/11/21 **exakt** gegen 1000/2000/4000/2500 | **ja** |
  | `tests/adapters/test_pdf_export.cpp`:330–335 (032c) | nur die **Anzahl** der `" l\n"`-Operatoren | nein |
  | `tests/adapters/test_png_export.cpp`:321–330 (032c) | nur **mehr Tinte als ohne** | nein |

  **Verdikt: die Zeile trägt, ohne neuen Test.** Der AK verlangt „unverändert nach Speichern/Laden
  **sowie** im 2D-Grundriss-Export" — der harte Konjunkt (Speichern/Laden, identische mm) ist
  wertgleich belegt, und der Export ist es in DXF ebenfalls. PDF/PNG belegen nur das **Erscheinen** —
  was der AK-Text dort auch nur verlangt („dann **erscheint** sie im Artefakt"). **Benannte Grenze:**
  eine Koordinaten-Wertgleichheit im **PDF-/PNG**-Artefakt hat dieses Repo nicht; das ist
  032c-Alt-Bestand und **nicht** Gegenstand dieses Slice — hier festgehalten, statt stillschweigend
  als Wertgleichheit gebucht zu werden.

## 8. Trigger

- [`slice-048a`](../done/slice-048a-drw-001-fangpunkte-ak-spec.md) hat AK und Spezifikation geliefert
  und die Implementierung ausdrücklich als Folge-Slice benannt; die
  [`welle-5`-Closure](../done/welle-5-results.md) §6 führt 048b als übernommenen Faden.

## 9. Closure-Trigger

- §4-Zeilen 1–9 grün + je diskriminierend belegt; `make gates` + `make io-smoke` grün;
  `make schema-check` byte-unberührt; Handbuch nachgezogen; Closure-Notiz.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** klein — eine reine Funktion, zwei Aufrufstellen, neun Orakel-Zeilen.
  Der Aufwand steckt in der **Auswahl-Semantik** (Tie-Break, Schwellwert), nicht im Umfang.
- **Phase-Reife:** AK und Spezifikation liegen seit 048a; der Canvas und seine
  Interaktions-Testbarkeit seit 043.
- **Risiko:** niedrig — additive Eingabe-Quantisierung, kein Kern-, Schema- oder Export-Diff; die
  bestehende Zeichen-Zusage bleibt unberührt (freies Zeichnen außerhalb der Fang-Nähe).

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-07-27)

Report: [`2026-07-27-slice-048b-plan.md`](../../../reviews/2026-07-27-slice-048b-plan.md) —
**1 HIGH / 3 MEDIUM / 6 LOW / 2 INFO, „nicht startbar"**. Unabhängiger Reviewer ≠ Plan-Autor.

| # | Behandlung |
|---|---|
| **HIGH-1** (Orakel-Zeile 5 verengte auf das dargestellte Geschoss, während die Spezifikation :1048–1051 alle Geschosse führt — und die DoD zugleich „kein Spezifikations-Eintrag" behauptete) | **Projektinhaber-Entscheidung: der Spezifikation wörtlich folgen.** `snapTarget` verliert den Geschoss-Parameter, Zeile 5 prüft jetzt den **Geschoss-Konjunkt des Tie-Breaks**. Die Spezifikation bleibt unberührt, die DoD-Verneinung wird damit **wahr**. Die Konsequenz (Fang auf nicht dargestellte Punkte) steht als **R5**. |
| **MEDIUM-1** (an Press/Release liegt keine `PlanView`; die DoD „kein neuer Zustand" verstellte einen der zwei Auswege) | **Entschieden: `pull_()` an beiden Stellen** — wie es der Paint-Pfad bei jedem Repaint tut. Ein Cache wäre neuer Zustand und könnte veralten. §2 begründet es; die DoD-Zusage bleibt wahr, statt angenommen zu sein. |
| **MEDIUM-2** (`PlanSegment` trägt keinen Unterscheider Wand↔Hilfslinie; der Tie-Break-Konjunkt „Wand-Achsen vor Hilfslinien" ist an `snapTarget` unprüfbar) | Der Tie-Break ist in **drei Konjunkte** zerlegt und auf **zwei Orte** verteilt: Zeilen 4/5 an `snapTarget` („zuerst besucht gewinnt", Geschoss-Reihenfolge, Anfang vor Ende), **neue Zeile 6a** an `projectPlan` (Wand-Achsen vor Hilfslinien). |
| **MEDIUM-3** (der von der Spezifikation als *harter Nachweis* benannte AK-Konjunkt „identische mm nach Speichern/Laden + Export" hatte keine Zeile) | **Neue Zeile 10**, gestützt auf **benannte Bestands-Sensoren** (032b-Persistenz, 032c-Export) statt auf einen neuen Test — mit **R6** als Vorbedingung: dass diese Orakel **wertgleich** prüfen, ist **vor** dem Start zu verifizieren. |
| **LOW-2** (der Sichtbarkeits-Filter greift nur bei Hilfslinien; Wände tragen keine `layer_id`) | §1-Tabelle und der Artefakt-Absatz sagen das jetzt — für Wände ist die Zusage **gegenstandslos**, nicht erzwungen. |

**R4 ist durch diesen Lauf erledigt, nicht offen geblieben:** der Reviewer hat verifiziert, dass
`tests/hexagon/test_plan_projection.cpp`:79–85 den Sichtbarkeits-Filter **diskriminierend** deckt.
Die Vorfassung führte das als offene Vorbedingung — sie ist beantwortet.

**Positiv bestätigt** (nicht neu prüfen): die `snapTarget`-Signatur passt an beide
`screenToModel`-Stellen · `drag_start_mm_`/R3 ist korrekt erfasst · die Zeilen 7/8/9 sind im
Bestands-Testbinary herstellbar · `.a-check.yml` trägt „keine neue Kante" · `test_snap.cpp` läuft ohne
Qt-Fixture · die zwei Handbuch-Stellen existieren (§4.2 trägt heute den **gegenteiligen** Satz) · der
vorgeschlagene **12-px-Default lässt die bestehenden `test_canvas_widget`-Erwartungen gültig** (der
nächste Kandidat liegt ≥ ~117 px entfernt) — der Slice bricht die Bestands-Orakel also nicht.

**Startbar:** ja — der HIGH ist aufgelöst, die drei MEDIUM eingearbeitet, R4 erledigt. **R6 bleibt als
benannte Vorbedingung vor dem ersten Commit**, nicht als Vollzugs-Frage.

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
