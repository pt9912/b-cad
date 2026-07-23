---
id: slice-043
titel: DRW-2D-Canvas-Impl — interaktives Zeichenwidget, screenToModel, Kommando-Naht, Composition-Root-Umschaltung
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006), [OBJ-003](../../../../spec/lastenheft.md#3-projektziele)]
adr_refs: [[ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)]
---

# Slice 043: DRW-2D-Canvas-Impl — der interaktive Erzeugungs-Weg

**Status:** open (Plan — **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review 2026-07-23: 1 HIGH / 1 MED / 2 LOW / 6 INFO; MED-1/LOW-1/LOW-2 eingearbeitet, [Report](../../../reviews/2026-07-23-slice-043-plan.md). HIGH-1 gelöst durch Projektinhaber-Entscheidung: Option A (`std::function`-Verdrahtung — null `.a-check.yml`-Änderung, ehrlich, kein ADR-Konflikt; s. §0). → startbar**; Start auf Projektinhaber-Wort).

**Welle:** welle-5-erweiterung (Ziel M5 „Erweiterbar"). **Konsequenz (c)** aus [ADR-0019](../../adr/0019-drw-2d-canvas.md)
(Canvas-Impl-Slice); die vorgelagerten Folgepflichten sind erledigt: **(a) AK-Schärfung** = slice-041a done
([LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) interaktive Erzeugung, Lastenheft 0.1.15), **(b) Lese-Naht-Refactor** = slice-042b done
(`projectPlan`/`PlanView`→Kern, `PlanViewPort`). **Vorgänger:** slice-042d ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Familie komplett — alle
Adapter→Kern-Kanten weg; die UI zieht die 2D-Projektion sauber aus dem Kern).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0019](../../adr/0019-drw-2d-canvas.md) (die **7 Entscheidungen** — Widget-Form, 2D-Lese-Naht,
`screenToModel`, Selbst-Refresh ohne `op`, Kommando-Naht in `ui/command/`, Fang/Raster = UI-Aids [v1 frei],
Umschalt-Layout + Headless-`QMouseEvent`-AK), [ADR-0009](../../adr/0009-gui-framework-qt6.md) (Qt = Driving/UI,
Regel E; Observer pullt+repaint; `view/`↔`command/`-Richtungs-Trennung), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)
(DRW-Mutationen melden **keinen** `op`), [ADR-0010](../../adr/0010-headless-gl-xvfb.md) (Headless via Xvfb),
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) (Fundament-Datenheimat + „kein op").
**[MR-016](../../../../harness/conventions.md) (ADR-Immutabilität)** unberührt — 043 implementiert, ändert keine Accepted-ADR.

---

## 0. Grundsatz-Entscheidung ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) HIGH-1) — Verdrahtung der port-freien UI-Nähte → **Option A gewählt**

> **Entscheidung (Projektinhaber, 2026-07-23): Option A — `std::function`-Verdrahtung.** Null `.a-check.yml`-
> Änderung, ehrlich, kein Konflikt mit der immutablen [ADR-0019](../../adr/0019-drw-2d-canvas.md). Die DoD/Datei-
> Tabelle unten sind auf **A** fixiert (die „nur bei Option B"-Zeilen entfallen). **HIGH-1 gelöst → startbar.**

**Befund (empirisch mit dem realen a-check-Image belegt):** Der `.a-check.yml`-Schlüssel `adapter_sink` ist ein
**skalarer Pfad-Präfix** (`adapters/ui/view/mesh_source`), der **genau eine** Datei von Regel B (kein
Adapter→Adapter) ausnimmt. Der generische Edge `ui_command → ui_view` **reicht nicht** — a-check flaggt
`command/`-Includes von `view/`-Dateien trotzdem als `lateral-adapter`, **außer** die Zieldatei liegt unter dem
`adapter_sink`-Präfix. Das 3D-`MeshSource`-Muster ist **nur** deshalb grün, weil `mesh_source` registriert ist.
Neue `view/`-Sink-Interfaces (`plan_source.h`, `guide_line_sink.h`), die `command/` implementiert, lassen
`make a-check` mit 2 `lateral-adapter`-Verstößen **scheitern** → die Fitness-Function ist so **nicht erreichbar**.

**Optionen (Entscheidung VOR Start):**
- **A — `std::function`-Verdrahtung (empfohlen; null Gate-Änderung, ehrlich).** Der Canvas hält **keine**
  `view/`-Sink-Interface-Header, sondern `std::function<model::PlanView()>` (Read) + `std::function<
  std::optional<model::GuideLineId>(model::Point2D, model::Point2D)>` (Schreib) — nur `<functional>` + Modelltypen.
  Die `ui/command/`-Objekte (halten `PlanViewPort`/`EditDrawingPort` + aktives Geschoss/Ebene) existieren wie
  geplant; der **Composition-Root** (`main.cpp`, darf alles) verdrahtet die Lambdas. **Kein** `command/ → view/`-
  Include → **kein** `lateral-adapter` → **keine** `adapter_sink`-Änderung, **keine** unehrliche Benennung. E5
  (Mutation über ein `command/`-Objekt, nicht im Widget) bleibt erfüllt: das Widget hält nur eine Callable, das
  `command/`-Objekt vermittelt den Port. **Preis:** Idiom-Abweichung vom abstrakt-Basis-`MeshSource`-Muster.
- **B — `adapter_sink` auf `adapters/ui/view` verbreitern (Gate-Config-Änderung).** Verzeichnis-Präfix statt
  Einzeldatei (empirisch → 0 Befunde). **Aber:** Verbreiterung einer Architekturregel-Ausnahme → **[§2.6](../../../../AGENTS.md)**
  (ADR + Carveout, Owner-Beschluss), und **[ADR-0019](../../adr/0019-drw-2d-canvas.md) §Konsequenzen** sagt
  ausdrücklich „fügt **keine** neue Gate-Regel hinzu und lockert **keine** Allow-Liste → §2.6 n/a" → Option B
  **kollidiert mit der immutablen Design-Autorität** und bräuchte einen [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Re-Eval/Owner-Sign-off.
- **C — Sink-Dateien unter den `mesh_source`-Präfix benennen (VERWORFEN).** Config-frei, aber **unehrlich**
  (ein `GuideLineSink` hieße `mesh_source_*`) — Gate-Gaming, nicht akzeptabel.

**Bis zur Entscheidung sind DoD-3/-4 (Read/Schreib-Naht) + die Datei-Tabelle als „Option A" formuliert; bei B
zusätzlich der `.a-check.yml`-Carveout + [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Re-Eval-Notiz.**

## 1. Ziel

Der Nutzer kann eine **Hilfslinie zeichnen** (Maus-Zug auf einer 2D-Zeichenfläche), sie **erscheint sofort** und
geht in denselben durablen/exportierten Zustand wie eine port-erzeugte ([LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)). Der Canvas ist ein neues
2D-`view/`-Widget, das die 2D-Projektion über `PlanViewPort` **pullt** und über `EditDrawingPort` **mutiert** —
beide Driving-Ports strikt über `ui/command/` (Richtungs-Trennung), das Widget selbst hält **keinen** Driving-Port.

**Struktureller Zwilling des 3D-Viewers, ein Novum.** Der Canvas spiegelt exakt die bewährte 3D-Naht:
`ViewerWidget`(`view/`, `ModelChangedPort`-Beobachter) + `MeshSource`(`view/`, port-frei) über
`ViewModelMeshSource`(`command/`, über `ViewModelPort`). **Neu ist allein die Schreib-Richtung:** der Canvas ist
der **erste UI-Mutator** — eine port-freie **Schreib-Naht** (`view/`) über ein `command/`-Objekt am
`EditDrawingPort`. Read-Naht + Beobachter-Refresh + Headless-Test-Muster sind 1:1 aus dem 3D-Viewer übernommen.

## 2. Definition of Done

- [ ] **2D-Canvas-Widget** (`src/adapters/ui/view/{canvas_widget}.{h,cpp}`, Vorschlag-Name — Review bestätigt):
      `QWidget` mit `paintEvent`/`QPainter` (**nicht** `QOpenGLWidget` — 2D-mm-Ebene braucht keinen GL-Kontext;
      vereinfacht den Headless-Test gegenüber dem 3D-Viewer), zeichnet die `PlanView`-Segmente des **aktiven
      Geschosses** (modelToScreen je Segment) + die in-Arbeit-Linie beim Ziehen. **Achtung Typ-Mismatch**
      ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-1):
      `model::StoreyId` ist `enum class : int`, aber `model::StoreyPlan.storey_id` ein **plain `int`** → Filter
      `static_cast<int>(activeStoreyId) == sp.storey_id` (sonst leere Zeichnung). Implementiert
      [ADR-0009](../../adr/0009-gui-framework-qt6.md)-`ModelChangedPort` (Beobachter): `onModelChanged` → queued
      `update()` (Repaint nach committeten `op`-Mutationen, z. B. Wand-Änderung; `ui_view → ports_driven`, erlaubt,
      Muster `ViewerWidget`). Hält **keinen** Driving-Port-Include.
- [ ] **`screenToModel`-Naht (die Testbarkeits-Achse, [ADR-0019](../../adr/0019-drw-2d-canvas.md) E3/E7).** Eine
      **invertierbare, display-freie** 2D-Sicht-Transformation Bildschirm-Pixel ↔ Modell-mm (Pan-mm-Offset +
      Zoom px/mm + Viewport-Größe; **+y-Modell zeigt nach oben** — der PDF/PNG-„kein-Y-Flip"-Präzedenz aus
      slice-025b wahren). **Empfehlung:** als **reiner Werttyp** `view/{view_transform}.h` (`screenToModel(QPoint)
      → model::Point2D` + `modelToScreen(Point2D) → QPointF`), **ohne** Widget-Lebenszyklus/GL konstruier- und
      unit-testbar; das Widget delegiert. Alternative (pure Methoden am Widget) benannt — Review entscheidet.
      **Fit-to-Bounds:** initiale Sicht rahmt die `PlanView`-BBox ein; leeres Modell (`has_geometry==false`,
      degenerierte BBox) → definierte Default-Sicht, leere Fläche (Totalität, kein Div-0 im Zoom-Fit).
- [ ] **Read-Naht (port-frei, Option A).** Der Canvas hält `std::function<model::PlanView()>` (nur `<functional>`
      + Modelltypen, **kein** `view/`-Sink-Header). Das `command/`-Objekt `{plan_view_plan_source}.{h,cpp}` hält den
      `PlanViewPort` und liefert die `PlanView`; der Composition-Root verdrahtet die Lambda. Der Canvas pullt die
      `PlanView` (nur Modelltypen), **nie** den Port direkt — `ui_view → ports_driving` ist verboten.
- [ ] **Schreib-Naht (port-frei, Option A) — das Novum.** Der Canvas hält
      `std::function<std::optional<model::GuideLineId>(model::Point2D, model::Point2D)>` (nur `<functional>` +
      Modelltypen, **kein** `view/`-Sink-Header). Das `command/`-Objekt `{edit_drawing_guide_line_sink}.{h,cpp}`
      hält den `EditDrawingPort` + aktives Geschoss/Ebene, baut das `GuideLine`-Prototyp (aktives Geschoss + aktive
      Ebene + gezogenes Segment) und ruft `addGuideLine(prototype)` (der Service vergibt die `GuideLineId`, Rückgabe
      `optional`); der Root verdrahtet die Lambda. **E5/Richtungs-Trennung** gewahrt: der `EditDrawingPort`-Include
      lebt **nur** in `command/`; das Widget hält den Port **nie** (nur eine Callable). **Kein** `command/ → view/`-
      Include → `make a-check` unverändert grün, **keine** `adapter_sink`-Änderung.
- [ ] **Aktives Geschoss + aktive Ebene (v1-Scope, ehrlich benannt).** `addGuideLine` braucht eine gültige
      `StoreyId` + `LayerId` (sonst Ablehnung — `structure_edit_service.cpp` `layerExists`/`nearPoint`-Prüfungen).
      **v1:** das `command/`-Schreib-Objekt wird mit einem **aktiven `StoreyId` + `LayerId`** konstruiert. **Der
      Composition-Root legt die dedizierte Hilfslinien-Ebene via `addLayer` an und injiziert die
      ZURÜCKGEGEBENE `LayerId`** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-2:
      **keine hartkodierte `LayerId{1}` raten**); Verdrahtungs-Reihenfolge **Ebene anlegen → Id → Sink konstruieren
      → subscribe**, VOR jeder Interaktion. Das aktive **Geschoss** ist unkritisch (ein Default-Geschoss existiert
      ab Service-Konstruktion). **Interaktive Geschoss-/Ebenen-Auswahl** (Panel, Umschalter) ist **NICHT Teil** —
      [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Re-Eval, eigener späterer Slice.
- [ ] **Selbst-Refresh nach eigenem Kommando ([ADR-0019](../../adr/0019-drw-2d-canvas.md) E4, **kein neuer `op`**).**
      Nach erfolgreichem `addGuideLine` (Rückgabe mit Wert) repaintet der Canvas **selbst** (queued `update()`) —
      die [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-§2-„kein op"-Entscheidung bleibt **unrevidiert**.
      **Benannte Grenze:** eine von außen un-benachrichtigte `guide_lines`-Mutation bliebe stale — v1 verdrahtet
      keinen konkurrierenden Schreiber (Ehrlichkeits-Grenze). Der Beobachter-Refresh (DoD-1) deckt `op`-Mutationen
      (Wände etc.); der Selbst-Refresh deckt die eigene `op`-lose Zeichnung.
- [ ] **v1 zeichnet frei ([ADR-0019](../../adr/0019-drw-2d-canvas.md) E6).** Interaktion: **Links-Zug** =
      Hilfslinie (Press=Anfang, Release=Ende, gemappte mm); **Rad** = Zoom (übt die nicht-triviale Transformation).
      **Fang/Raster/Winkel = NICHT Teil** (UI-Aids, eigene Slices) — der Endpunkt ist die geklickte Modell-mm
      **ungefangen**. Pan minimal (Mittel-Zug **oder** deferiert — Review; das AK braucht nur den Zeichen-Zug).
- [ ] **Composition-Root-Umschaltung** (`src/main.cpp`, [ADR-0019](../../adr/0019-drw-2d-canvas.md) E7). Statt des
      einzelnen `setCentralWidget(viewer)`: ein Umschalt-Layout (Vorschlag `QStackedWidget` + `QAction`/Toolbar
      3D↔2D, **oder** `QTabWidget`) trägt **beide** Widgets. Die neuen `command/`-Objekte (Read + Schreib) werden
      **vor** den Widgets deklariert (nicht-besitzende Referenzen, Lebensdauer wie `mesh_source`). **Zweites
      `subscribe`/`unsubscribe`** für den Canvas am Observer-Lebenszyklus. **Der `--acc-002-beleg`-Headless-Pfad
      (`grabFramebuffer` des 3D-Viewers, main.cpp) bleibt funktionsfähig** (Rest-Risiko #2). **Konkreter Fix**
      ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1):
      im Beleg-Zweig `stack->setCurrentWidget(viewer)` **vor** `window.show()` (ein `QOpenGLWidget`, das nicht die
      aktuelle Stack-Seite ist, initialisiert den GL-Kontext beim `show()` nicht → `grabFramebuffer` läge Null).
      **Entlastung:** `--acc-002-beleg` ist ein **manueller Schritt, KEIN Gate**; der **gated** Viewer-Smoke
      (`test_viewer_widget.cpp`) konstruiert `ViewerWidget` **standalone** (nicht im Stack) → vom `main.cpp`-Umbau
      **unberührt**, `make gates` kann über diesen Pfad nicht brechen. Nur der manuelle Beleg braucht die Sorgfalt.
- [ ] **Headless-Interaktions-AK ([ADR-0019](../../adr/0019-drw-2d-canvas.md) Fitness, [ADR-0010](../../adr/0010-headless-gl-xvfb.md)).**
      Neuer Test `tests/adapters/{test_canvas_widget}.cpp` (Muster `test_viewer_widget.cpp`: manuelle
      `QApplication`, `QMouseEvent`-Synthese via `QApplication::sendEvent`, Xvfb):
      - **(Happy)** Press@Screen-A → Move → Release@Screen-B → `service.building().guide_lines` wächst um **eins**;
        die gespeicherten Endpunkte ≈ `screenToModel(A/B)` (Toleranz) — **display-frei** über den Surrogat-Zustand
        (`building()`), **nicht** über den Framebuffer.
      - **(Boundary/Negative)** entarteter Zug (Press==Release, Anfang=Ende) → **keine** Hilfslinie
        (`guide_lines` unverändert; die `addGuideLine`-Entartungs-Ablehnung schlägt als No-op durch).
      - **(screenToModel-Einheit)** direkter Roundtrip `screenToModel(modelToScreen(p)) ≈ p` + eine bekannte
        Punkt-Abbildung bei nicht-triviellem Zoom/Pan (ohne Widget-`show()`/GL).
      - Optional (Review): ein Paint-Smoke (`paintEvent` läuft ohne Absturz auf gefülltem + leerem Modell).
- [ ] **CMake** (`tests/CMakeLists.txt` + `src/adapters/CMakeLists.txt`): die neuen `view/`+`command/`-Quellen in
      `bcad_adapters`, der neue Test in `bcad_adapter_tests`.
- [ ] **a-check/arch-check-Gegenprobe (die Fitness-Function).** `make a-check` grün: **kein** `ui_view →
      ports_driving` (das Widget hält keinen Driving-Port; Read+Schreib laufen port-frei); Qt-Includes nur
      `src/adapters/ui/` + `main.cpp` (`make arch-check`, Regel E unverändert). **Regel-B-Sink (§0, Option A):**
      **kein** `command/ → view/`-Include (der Canvas hält `std::function`, keine `view/`-Sink-Header) → **null**
      `adapter_sink`-Änderung, kein `lateral-adapter`-Verstoß. Der `ModelChangedPort`-Beobachter
      (`ui_view → ports_driven`) ist bereits erlaubt (Muster `ViewerWidget`) — keine neue Kante.
- [ ] **Doku.** `spec/spezifikation.md` §1 Canvas-Naht-Block von „**Design entschieden** (Implementierung folgt)"
      + „das **Canvas-Widget** selbst folgt als eigener Impl-Slice" auf **realisiert** ziehen (token-frei,
      [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)
      vor Gate greppen) + `spezifikation-historie`; `architecture.md` §1.1-Ports-Klauseln sind bereits gesetzt
      (slice-041a) — prüfen, ob eine „kein Canvas in v1"-Formulierung zu kippen ist; [ADR-Index](../../adr/README.md)
      [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Canvas-Impl-Zeile „erfüllt"; **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
      **[MR-020](../../../../harness/conventions.md) Closure-Disziplin:** die verbleibende [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Folgepflicht
      (Fang/Raster/Winkel) ist im ADR-Index als offener Re-Eval geführt → erfüllt (kein Skelett nötig, eigene
      spätere AK-Slices).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/ui/view/{canvas_widget}.{h,cpp}` | neu | 2D-`QWidget`/`QPainter`-Zeichenfläche; `ModelChangedPort`-Beobachter; Maus→`screenToModel`→Schreib-Naht; Selbst-Refresh |
| `src/adapters/ui/view/{view_transform}.h` | neu | reiner, display-frei testbarer `screenToModel`/`modelToScreen`-Werttyp (Pan/Zoom, +y-oben) |
| `src/adapters/ui/command/{plan_view_plan_source}.{h,cpp}` | neu | Read-Impl über `PlanViewPort`; Root verdrahtet die `std::function`-Read-Lambda |
| `src/adapters/ui/command/{edit_drawing_guide_line_sink}.{h,cpp}` | neu | Schreib-Impl über `EditDrawingPort` (aktives Geschoss/Ebene → Prototyp); Root verdrahtet die `std::function`-Schreib-Lambda |
| `src/main.cpp` | ändern | Umschalt-Layout 3D↔2D; Canvas + Read/Schreib-`command/`-Objekte verdrahten; zweites subscribe/unsubscribe; [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Pfad wahren |
| `tests/adapters/{test_canvas_widget}.cpp` | neu | Headless-`QMouseEvent`-AK (Happy/Boundary/screenToModel-Einheit) |
| `src/adapters/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | neue Quellen + Test registrieren |
| `spec/spezifikation.md` §1 + `-historie.md`, `spec/architecture.md`, [ADR-Index](../../adr/README.md), `CHANGELOG.md` | ändern | Doku-Nachzug (planned→realisiert, token-frei) |

**Bewusst NICHT Teil (Re-Eval-Trigger, [ADR-0019](../../adr/0019-drw-2d-canvas.md)):** Fang/Raster/Winkel-AK;
interaktive Geschoss-/Ebenen-Auswahl + Layer-Bedien-Panel; Bauteile (Wände/Räume) interaktiv zeichnen;
Selektion/Picking/Bemaßung; ein DRW-`op` / 2D-Change-Signal für konkurrierende Schreiber. Je eigener Slice.

## 4. Trigger

- **slice-042b done** (`PlanViewPort` + `PlanView` im Kern — die Read-Naht steht) **+ slice-042d done**
  ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Familie komplett; die UI zieht die 2D-Projektion sauber aus dem Kern, kein Adapter→Kernel).
  [ADR-0019](../../adr/0019-drw-2d-canvas.md) Accepted (7 Entscheidungen bindend).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  GUI-Interaktion + erster UI-Mutator → **Code-Review vor Welle-Closure** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (Headless-AK unter Xvfb grün: Happy-Zug legt Hilfslinie mit gemappten mm an,
  entarteter Zug nicht; `screenToModel`-Roundtrip; **[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg unverändert grün**; a-check/arch-check grün =
  **kein** `ui_view → ports_driving`, Qt nur in `ui/`+`main`), Closure-Notiz → DRW-Interaktiv-Strang **v1 fertig**
  (Fang/Raster + Selektion als benannte Folge-Slices).

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — Headless-AK hängt am Display statt am Zustand.** Die AK muss die Interaktion **display-frei**
  belegen. **Mitigation (DoD):** `screenToModel` ist ein **reiner, GL-freier** Werttyp (vom Paint getrennt); die
  AK ankert am **Surrogat-Zustand** `service.building().guide_lines` (+1, gemappte mm), nicht am Framebuffer —
  exakt das [ADR-0019](../../adr/0019-drw-2d-canvas.md)-E7-Fitness-Muster (`test_viewer_scene`-Analog). `QWidget`/
  `QPainter` statt `QOpenGLWidget` entschärft zusätzlich (kein GL-Kontext nötig).
- **Rest-Risiko #2 — Composition-Root-Umschaltung bricht [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien).** Der `--acc-002-beleg`-Pfad rendert den
  3D-Viewer per `grabFramebuffer`. Der Umbau von einem `setCentralWidget` auf ein Umschalt-Layout darf diesen Pfad
  **nicht** brechen (der Viewer muss erreichbar + gerendert bleiben). **Mitigation:** [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Pfad gezielt auf den
  Viewer belassen; nach dem Umbau `make test` (der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg-/Viewer-Smoke) unverändert grün. Lebensdauer der
  nicht-besitzenden `command/`-Referenzen (Deklaration vor den Widgets) sorgfältig.
- **Rest-Risiko #3 — Port-Ausreichung / aktives Geschoss+Ebene.** `PlanViewPort::planView()` liefert nur
  Segmente+BBox je Geschoss — **keine** Picking-/Id-Rückverfolgung (für v1-frei-Zeichnen ausreichend). `addGuideLine`
  braucht gültige `StoreyId`+`LayerId`: v1 fixiert aktives Geschoss (erstes) + eine dedizierte Hilfslinien-Ebene
  (Root-injiziert). **Kein „ebenenloses" Zeichnen** — die aktive Ebene muss existieren (sonst Ablehnung). Review
  bewertet Root-Injektion vs. Lazy-`addLayer` im Sink.
- **Rest-Risiko #4 — Richtungs-Trennung (`ui_view → ports_driving` verboten) + Regel-B-Sink (§0/HIGH-1, gelöst).**
  Der Canvas ist der erste UI-Mutator; die Versuchung, `EditDrawingPort` direkt im Widget zu halten, verletzt die
  a-check-Kante. **Mitigation (Option A):** Read **und** Schreib laufen port-frei über `std::function`-Callables,
  die der Composition-Root aus `command/`-Objekten verdrahtet — **kein** `command/ → view/`-Include, **kein**
  Widget-Driving-Port; `make a-check` als Gegenprobe. **Kein** `adapter_sink`-/Allow-Edge-Change (der skalare
  `adapter_sink` bräuchte sonst eine §2.6-Lockerung — der HIGH-1-Fund; durch Option A umgangen).
- **Rest-Risiko #5 — Selbst-Refresh-Grenze (E4).** Der Canvas synchronisiert nur eigene Kommandos + `op`-Mutationen
  (Beobachter). Eine `op`-lose Fremd-`guide_lines`-Mutation (Plugin-Host) bliebe stale — **v1 verdrahtet keinen
  konkurrierenden Schreiber** (Ehrlichkeits-Grenze, benannter Re-Eval; kein garantierter v1-Bug).
- **Scope:** mittel–groß — neues Widget + zwei port-freie Nähte (Read wiederverwendbar-Muster, **Schreib neu**) +
  Root-Umbau + Headless-AK. Überwiegend Muster-Wiederholung des 3D-Viewers; das Novum (Schreib-Naht + Root-
  Umschaltung) ist die Sorgfalts-Stelle. **Spec token-frei** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter `src/adapters/ui/` + `main.cpp`

- **Modus:** GF; **Dichte:** hoch (neues Widget + Read/Schreib-Naht + Transformation + Root-Umschaltung).
  **Phase-Reife:** welle-5 Interaktiv-Strang. **Risiko:** mittel — Headless-Testbarkeit (display-frei) + [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-
  Erhalt + Richtungs-Trennung sind die Prüfsteine; **Code-Review** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) empfohlen (GUI + erster Mutator).

### Sub-Area: Spec/Doku `spec/` + `docs/plan/`

- **Modus:** GF; **Dichte:** niedrig–mittel (planned→realisiert, token-frei; ADR-Index/CHANGELOG). **Risiko:** niedrig.

## 8. Closure-Notiz

*(bei Closure ausgefüllt: Widget-/Naht-Form [Read + Schreib-Novum], `screenToModel`-Heimat, aktives Geschoss/
Ebene-Auflösung, Root-Umschaltung + [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Erhalt, Headless-AK-Ergebnisse [Happy/Boundary/Einheit],
a-check/arch-check-Gegenprobe [kein `ui_view → ports_driving`], Review-Ergebnisse
[[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) + Code-Review],
Lerneintrag, Folge = Fang/Raster/Winkel + Selektion als benannte Slices.)*
