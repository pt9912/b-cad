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
| **Unsichtbare Ebenen** | `projectPlan` filtert sie **vor** der `PlanView` heraus → strukturell nicht fangbar |
| **Benachrichtigung** | **kein** neuer `op`, keine zusätzliche `ModelChanged`-Meldung |
| **Ablehnung** | **kein** neuer Fehlerfall; Anfang = Ende trifft die bestehende Entartungs-Ablehnung |

**Am Artefakt geprüft (2026-07-27), bevor dieser Plan darauf baut:**
`src/hexagon/services/geometry/plan_projection.cpp` schiebt je Geschoss **Wand-Achsen (:46–53) vor
Hilfslinien (:58–69)** und prüft die Ebenen-Sichtbarkeit (`visible.contains`, :62) **vor** dem
Einfügen. Die in 048a ausgeschriebene Tie-Break-Regel und die „strukturell nicht fangbar"-Zusage sind
also real gedeckt — nicht nur behauptet.

## 2. Wo der Fang liegt — und warum nicht im Widget-Rumpf

**Der Fang wird eine reine Funktion in einer eigenen Datei** (`src/adapters/ui/view/snap.{h,cpp}`),
nicht ein Block in `CanvasWidget::mousePressEvent`:

```cpp
// display-frei, ohne QWidget, ohne QApplication
std::optional<hexagon::model::Point2D> snapTarget(
    const hexagon::model::PlanView& plan, int active_storey_id,
    const ViewTransform& transform, QPoint cursor_px, double threshold_px);
```

**Begründung — die Lehre aus welle-5 §5-2, hier vorab angewandt:** die Zusagen dieses Slice sind
Aussagen über **Auswahl** (nächstgelegener Punkt, Tie-Break, Schwellwert, Sichtbarkeit). Liegen sie im
Widget-Rumpf, sind sie nur über einen Qt-Ereignis-Zug prüfbar — und jede Gegenprobe müsste über
`QApplication` + `sendEvent` laufen. Als freie Funktion sind sie **direkt** und **ohne Qt-Fixture**
diskriminierend, in derselben Bauform wie `ViewTransform` (header-nah, display-frei, [ADR-0019](../../adr/0019-drw-2d-canvas.md) E3).

**Was im Widget bleibt:** der Aufruf an den beiden Stellen, an denen heute `screenToModel` steht
(Press = Anfang, Release = Ende), und die Fang-Nähe als **Widget-Konstante**. Das ist Verdrahtung,
keine Entscheidung.

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
| 4 | **Tie-Break ist die `PlanView`-Reihenfolge** (Wand-Achsen vor Hilfslinien; Anfang vor Ende) bei exakt gleicher Distanz | `snapTarget`, rein | Iteration umgedreht ⇒ rot |
| 5 | **Nur das aktive Geschoss** ist fangbar (der Canvas zeichnet nur dieses) | `snapTarget`, rein | Geschoss-Filter entfernt ⇒ rot |
| 6 | **Unsichtbare Ebene ⇒ kein Fang** — der Punkt ist gar nicht in der `PlanView` | `projectPlan`, Kern | Sichtbarkeits-Filter in `plan_projection` entfernt ⇒ rot (**Bestands-Orakel**, prüfen ob vorhanden; sonst ergänzen) |
| 7 | **Der Zug fängt wirklich**: Maus-Zug endet in Fang-Nähe ⇒ die erzeugte Hilfslinie trägt **exakt** die Ziel-mm | `CanvasWidget`, headless (Xvfb) | `snapTarget`-Aufruf im Release-Pfad entfernt ⇒ rot |
| 8 | **Anfang wird ebenso gefangen** wie das Ende | `CanvasWidget`, headless | Aufruf im Press-Pfad entfernt ⇒ rot |
| 9 | **Entartung**: fängt Anfang **und** Ende denselben Punkt ⇒ **keine** Hilfslinie, Modell unverändert, **kein** neuer Fehlerfall | `CanvasWidget` + Kern-Bestandsregel | die bestehende Ablehnung umgangen ⇒ rot |

**Zeile 7 ist die Zusammenspiel-Zeile** — sie belegt, was die reinen Zeilen 1–5 **nicht** können: dass
der Canvas die Funktion an der richtigen Stelle und mit den richtigen Argumenten ruft. Die reinen
Zeilen allein blieben grün, wenn der Aufruf fehlte; genau dieser Fehler-Typ ist in welle-5 dreimal
aufgetreten.

**Benannte Grenze:** die **visuelle** Rückmeldung (ob der Benutzer *sieht*, dass gefangen wird) ist
nicht Gegenstand — es gibt sie in diesem Slice nicht (§3).

## 5. Definition of Done

- [ ] **`src/adapters/ui/view/snap.{h,cpp}`**: `snapTarget(...)` + benannte Fang-Nähe-Konstante.
      Display-frei, **kein** `QWidget`, **kein** Port-Include; Orakel §4-1..5.
- [ ] **`CanvasWidget` ruft sie** an den zwei bestehenden `screenToModel`-Stellen (Press/Release).
      **Sonst unverändert** — kein neuer Zustand, kein neues Signal; Orakel §4-7/8.
- [ ] **Kein neuer `op`, keine zusätzliche Meldung** — geprüft und **verneint** (048a §Benachrichtigung).
- [ ] **`tests/adapters/test_snap.cpp`** (neu, ohne Qt-Fixture) + Erweiterung von
      `tests/adapters/test_canvas_widget.cpp` (Zug mit Fang, Xvfb); beide Listen in
      `tests/CMakeLists.txt`, `snap.cpp` in `src/adapters/CMakeLists.txt`.
- [ ] **Orakel §4-1..9 je mit roter Gegenprobe** im Closure-Text.
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
| `tests/hexagon/test_plan_projection.cpp` | ändern o. begründet unberührt | §4-6 — **erst prüfen, ob das Bestands-Orakel den Sichtbarkeits-Filter schon diskriminierend deckt** |
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
- **R4 — Bestands-Orakel zu §4-6.** Ob `test_plan_projection.cpp` den Sichtbarkeits-Filter schon
  **diskriminierend** prüft, ist **vor** dem Start zu verifizieren (Gegenprobe: Filter entfernen).
  Ist er nur beiläufig gedeckt, gehört die Zeile ergänzt — sonst ruht die Negative-AK von DRW-001 auf
  einer Annahme.

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

## 11. Closure-Notiz

_(bei Ausführung auszufüllen)_
