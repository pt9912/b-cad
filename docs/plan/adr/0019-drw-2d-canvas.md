# ADR-0019: DRW interaktiver 2D-Zeichen-Canvas — Zeichenfläche, 2D-Lese-Naht (Kern-Projektion + `PlanViewPort`), Koordinaten-Abbildung, Refresh, Fang/Raster-Heimat

**Status:** Proposed

**Datum:** 2026-07-22

**Autor:** Dietmar Burkard (DRW-Interaktiv-Strang-Eröffnung welle-5-erweiterung; ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [LH-FA-DRW-005](../../../spec/lastenheft.md#lh-fa-drw-005) (Hilfslinien — die interaktive Erzeugung, in [ADR-0018](0018-drw-2d-zeichen-daten.md) §Re-Eval hierher deferiert), [LH-FA-DRW-006](../../../spec/lastenheft.md#lh-fa-drw-006) (Ebenen), [LH-FA-DRW-001](../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/002/003 (Fangpunkte/Raster/Winkel — die Zeichen-Aids, deren Heimat hier benannt wird), [OBJ-003](../../../spec/lastenheft.md#3-projektziele) (durchgängiges Modell — 2D aus demselben Datenmodell), [ADR-0001](0001-hexagonale-architektur.md) (Schichtung — Regel A/B; der Canvas ist ein Driving-Adapter, die 2D-Projektion ist Kern-Rechnung), [ADR-0008](0008-aenderungs-benachrichtigung.md) (Benachrichtigung — DRW-Mutationen melden **keinen** `op`), [ADR-0009](0009-gui-framework-qt6.md) (Qt = Driving/UI, Regel E; single-threaded am Event-Loop; Observer pullt+repaint; `ViewModelPort` als 3D-Lese-Naht — das 3D-Vorbild des hier neuen 2D-Read-Ports), [ADR-0010](0010-headless-gl-xvfb.md) (Headless-GL via Xvfb — die Infrastruktur, unter der der Interaktions-Test läuft), [ADR-0018](0018-drw-2d-zeichen-daten.md) (DRW-Fundament — Datenheimat, `EditDrawingPort`, »kein op«, »2D-Query/ViewModel-Naht wird mit dem Canvas fällig«)

---

## Kontext

Das DRW-Fundament ist geliefert (Datenheimat `model::GuideLine`/`Layer` auf `Building`; Driving-Port `EditDrawingPort` am Anwendungs-Service; SQLite-Persistenz; 2D-Grundriss-Export DXF/PDF/PNG mit Ebenen-Sichtbarkeits-Filter). Was **fehlt**, ist der **interaktive Erzeugungs-Weg**: der Nutzer kann eine Hilfslinie bisher **nicht zeichnen** — sie entsteht nur über den Port (Test/Plugin/Demo). Diese Lücke ist im Fundament durchgängig **ehrlich benannt** und ihre Auflösung wurde von [ADR-0018](0018-drw-2d-zeichen-daten.md) §Re-Evaluierungs-Trigger hierher deferiert: »Interaktiver 2D-Zeichen-Editor … eigener Beschluss an der Regel-E-Grenze ([ADR-0009](0009-gui-framework-qt6.md)): eine **2D-Zeichenfläche** und eine **Lese-/Darstellungs-Naht (2D-Query/ViewModel)** werden dann fällig.«

**Ausgangslage der UI (bindend, nicht hier neu entschieden):** Qt Widgets, Kern framework-frei, Qt nur in `src/adapters/ui/` + `main.cpp` (Regel E); der 3D-Viewer ist ein `QOpenGLWidget` mit Orbit-Kamera, der über den **`ViewModelPort`** (Tessellation) pullt und als `ModelChangedPort`-Beobachter nach committeten Mutationen repaintet ([ADR-0009](0009-gui-framework-qt6.md)); die UI-Schicht ist per Verzeichnis in `view/` (driven/Beobachter) und `command/` (driving) getrennt.

Fünf Beobachtungen prägen die Entscheidung:

1. **Es gibt keine 2D-Lese-Naht.** `ViewModelPort` ist strikt 3D-Tessellation. Die 2D-Grundriss-Projektion existiert nur **io-resident**: `projectPlan(Building) → PlanView` (2D-Segmente je Geschoss **inkl. sichtbarer Hilfslinien** + Bounding-Box) lebt in `src/adapters/io/plan_geometry.*` und speist DXF/PDF/PNG.
2. **Diese Projektion ist bereits reine, framework-freie Kern-Rechnung** — sie importiert nur `hexagon/model/*` + STL (kein OCC/Qt/SQLite/IO). Nur ihr **Wohnort** (der IO-Adapter) ist zufällig, weil der Export der erste Konsument war.
3. **Regel B verbietet Adapter→Adapter.** Der UI-Adapter darf `adapters/io/plan_geometry.h` **nicht** importieren — der Canvas kann die vorhandene Projektion nicht erreichen.
4. **Keine UI-Klasse mutiert das Modell.** Heute hält nur der read-only `ViewModelMeshSource` einen Driving-Port; der Canvas wäre der **erste** UI-Mutator (über `EditDrawingPort`).
5. **DRW-Mutationen melden keinen `op`.** `addGuideLine` ruft `notifyListeners` nicht; `ModelChangeOp` hat kein DRW-Glied ([ADR-0018](0018-drw-2d-zeichen-daten.md) §2). Der bestehende Observer-Pfad würde den Canvas nach einer Zeichnung **nicht** benachrichtigen.

Sieben Lösungsfragen, die der Spec-Text nicht entscheidet.

**Nicht offen** (bewusst außerhalb dieser ADR — Scope-Verengung, Präzedenz der Fundament-/Format-ADRs):

- **Fang/Raster/Winkel-AK** ([LH-FA-DRW-001](../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/002/003) — diese ADR benennt nur die **Heimat** der Aids (Entscheidung 6); ihre benutzer-beobachtbaren AK schärfen **eigene spätere Schärfungs-Slices**. v1 des Canvas zeichnet **frei** (Endpunkt = geklickte Modell-mm).
- **Bauteile interaktiv zeichnen** (Wände/Räume auf dem Canvas), **Layer-Bedien-Panel**, **Selektion/Picking**, **Bemaßung** — benannte Re-Eval-Trigger, nicht dieser Schnitt.
- **Interaktive 3D-Selektion** ([ADR-0009](0009-gui-framework-qt6.md) §Re-Eval, AIS/V3d) — unberührt.
- **Exakte Widget-/Signatur-/Konstanten-Gestalt** (Klassennamen, Port-Methodensatz, Zoom-Grenzen, Snap-Radius-Defaults) legen die **Impl-Slices** fest, nicht diese ADR.

## Entscheidung

1. **Zeichenfläche — ein neues 2D-Canvas-Widget im UI-`view/`-Verzeichnis, nicht der erweiterte 3D-Viewer.** Der 3D-`ViewerWidget` ist ein `QOpenGLWidget` mit fest verdrahteter Orbit-Kamera und 3D-MVP; eine 2D-Zeichenfläche ist ein eigenständiges Widget (2D-`QPainter`-Zeichnung einer mm-Ebene bzw. 2D-ortho). Es lebt neben dem `ViewerWidget` in `src/adapters/ui/view/` (Präzedenz der Richtungs-Trennung: Renderer/Beobachter in `view/`). Beide Sichten teilen sich das Fenster über ein Umschalt-Layout (Entscheidung 7).

2. **2D-Lese-Naht — die reine 2D-Projektion wird in den Kern gehoben (`src/hexagon/services/geometry/plan_geometry.*`) und über einen neuen Driving-Read-Port `PlanViewPort` gelesen. Der Export ruft die reine Kern-Funktion direkt, der Canvas pullt den Port — eine Quelle.** Die Projektion ist reine Domänen-Rechnung (`Building` → 2D-Segmente + sichtbare Hilfslinien + BBox), framework-frei; sie gehört in den framework-freien Kern, nicht in einen Adapter. Der Umzug ist **Regel-B-lösend** (die UI zieht dann aus dem Kern, nicht lateral aus `io/`), **dedupliziert** (UI **und** Export teilen dieselbe Funktion) und **erzwingt die Bibliotheks-Freiheit maschinell**: der Kern-Target `bcad_hexagon` hat per Hard Rule ([ADR-0001](0001-hexagonale-architektur.md)) **keine** externen Abhängigkeiten — jeder Library-Zug dort ist ein Link-Fehler (stärker als die »per Disziplin«-Freiheit in einem Adapter-Target, das selbst Qt/IO linkt).
   - **Heimat:** `src/hexagon/services/geometry/plan_geometry.{h,cpp}` (die etablierte Heimat lib-freier Geometrie-Helfer neben `wall_footprint`/`roof_geometry`); die Werttypen `PlanView`/`StoreyPlan`/`PlanSegment` ziehen mit (pure Structs).
   - **Naht:** ein neuer Driving-Read-Port `PlanViewPort` (`src/hexagon/ports/driving/{plan_view_port}.h`) — das **2D-Analog zum `ViewModelPort`** (den [ADR-0018](0018-drw-2d-zeichen-daten.md) §2 als »2D-Query/ViewModel-Naht« ankündigte); vom Anwendungs-Service implementiert (ruft `projectPlan(building_)`).
   - **Konsum:** der Canvas pullt `PlanViewPort` (wie der 3D-Viewer `ViewModelPort`); die Export-Adapter (DXF/PDF/PNG) rufen die reine Kern-Funktion direkt (sie halten das `Building` in `write()`).
   - **Sequenz (Impl):** die Projektions-Hebung + Export-Migration ist ein **eigenständiger Refactor-Slice, sequenziert VOR** dem Canvas-Widget-Slice — ein **reiner Umzug** (Logik unverändert), der die bestehenden 2D-Export-Decode-Orakel als Sicherheitsnetz grün hält, orthogonal zu jedem Canvas-Code. **Nicht** in den Canvas-Impl gefaltet.

3. **Bildschirm→Modell-Abbildung — der Canvas besitzt eine invertierbare 2D-Sicht-Transformation (Pan/Zoom einer mm-Ebene) mit einer testbaren `screenToModel`-Naht.** Heute existiert nur Modell→Bildschirm (3D-MVP, nicht invertierbar). Der Canvas trägt eine reine 2D-Transformation Bildschirm-Pixel ↔ Modell-mm; die Rückrichtung `screenToModel(Punkt) → model::Point2D` ist eine **eigene, display-freie Naht**, damit die Maus-AK headless über die gemappten mm prüfbar ist (Entscheidung 7), nicht nur über den Framebuffer.

4. **Refresh — Selbst-Refresh nach dem eigenen erfolgreichen Kommando; kein neuer `op`.** Der Canvas ist **Treiber und Betrachter derselben Sicht**: nach einem erfolgreichen `addGuideLine` (Rückgabe mit Wert) repaintet er **selbst** (queued `update()`). Das braucht **keinen** DRW-`op` — die [ADR-0018](0018-drw-2d-zeichen-daten.md)-§2-Entscheidung »kein op« bleibt **unrevidiert**; die 3D-Sicht braucht Hilfslinien ohnehin nicht (sie sind 2D-export-/canvas-only). Eine **zweite gleichzeitige 2D-Sicht** (die ohne `op` synchron bleiben müsste) ist ein benannter Re-Eval — v1 hat genau einen Canvas (self-driven).

5. **Kommando-Naht — der mutierende `EditDrawingPort` wird über ein `ui/command/`-Objekt vermittelt, nicht direkt im `view/`-Widget gehalten.** Der Canvas ist der erste UI-Mutator. Die Richtungs-Trennung (Driving-Port-Include nur unter `ui/command/`) bleibt gewahrt: das `view/`-Zeichenwidget delegiert die Mutation (bei Zeichen-Abschluss `addGuideLine`) an ein `command/`-Kommando-Objekt über dem `EditDrawingPort` — analog zur bestehenden `MeshSource`-Naht (declared `view/`, implemented `command/`).

6. **Fang/Raster/Winkel — UI-Interaktions-Zustand im Canvas, nicht Kern-Modell/Persistenz.** Fangpunkte, Raster und Winkelvorgaben quantisieren die **Eingabe** (die geklickte Bildschirm-Position) zu Modell-mm; der **gefangene mm-Wert** ist das, was an `addGuideLine` übergeben wird. Diese Aids sind **Interaktions-Hilfen der Zeichenfläche** (Widget-Zustand: Raster-Abstand, Fang-Radius, Winkel-Inkrement), **nicht** persistierte Modell-Daten und keine Spec-`§3`-Konstante des Bauteil-Modells. [ADR-0018](0018-drw-2d-zeichen-daten.md) verortete die Zeichen-**Daten** im Kern; die Zeichen-**Aids** sind UI-resident. Ihre benutzer-beobachtbaren AK ([LH-FA-DRW-001](../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/002/003) schärfen **eigene spätere Slices**; v1 zeichnet frei.

7. **Fenster & Headless-Test — Umschalt-Layout 3D↔2D + `QMouseEvent`-Synthese-Test mit Surrogat-Assertion.** Das Hauptfenster (heute ein einzelnes `setCentralWidget`, ohne Menü/Toolbar) erhält ein Layout, das den 3D-Viewer und den 2D-Canvas trägt (Splitter/Stacked + eine `QAction`/Toolbar-Umschaltung) plus je ein `subscribe`/`unsubscribe` am Service-Observer-Lebenszyklus ([ADR-0009](0009-gui-framework-qt6.md) (e)). **Headless** ([ADR-0010](0010-headless-gl-xvfb.md), Xvfb): der Interaktions-Test **synthetisiert `QMouseEvent`** (Press/Move/Release, Muster des bestehenden `ViewerWidget`-GL-Smoke-Tests) und prüft **display-frei** über einen **Surrogat-Zustand**, dass nach dem Zug `building().guide_lines` um eins gewachsen ist — mit den über `screenToModel` **gemappten mm**. Der Canvas muss dazu `screenToModel` + den beobachtbaren Zustand exponieren (Fitness-Function), damit die AK nicht am Framebuffer hängt — das ist der Interaktions-Test, dessen Bedarf [ADR-0009](0009-gui-framework-qt6.md) (f) ankündigte.

## Verglichene Alternativen

### Zu 1: 2D-Canvas als neues Widget (gewählt) vs. 3D-`ViewerWidget` erweitern

- **Neues Widget (gewählt) — Pro:** saubere Trennung 2D-Zeichnen vs. 3D-Orbit; keine Fremd-Verantwortung im GL-Viewer; Richtungs-Trennung `view/` bleibt gewahrt. **Contra:** ein zweites Widget + Umschalt-Layout.
- **3D-Viewer erweitern — Contra (entscheidend):** die Orbit-Kamera/3D-MVP kollidiert mit einer 2D-Zeichen-Ebene; ein Widget mit zwei Modi vermischt Verantwortungen und erschwert den display-freien Test. **Verworfen.**

### Zu 2: Projektion in den Kern heben + `PlanViewPort` (gewählt) vs. UI-Selbst-Projektion vs. `entity_layers`-artige Neu-Implementierung

- **Kern-Hebung + Port (gewählt) — Pro:** Regel-B-sauber (UI zieht aus dem Kern); **eine** Quelle (UI + Export); Bibliotheks-Freiheit **maschinell** erzwungen (Kern-Target ohne Deps); 2D-Analog zum `ViewModelPort` (von [ADR-0018](0018-drw-2d-zeichen-daten.md) §2 angekündigt); die Projektion ist bereits pure Kern-Rechnung. **Contra:** ein Refactor-Slice zieht die Funktion um und migriert den Export (durch die bestehenden Decode-Orakel abgesichert; reiner Umzug).
- **UI-Selbst-Projektion (`const Building&` in der UI, eigene Projektion) — Pro:** Export unangetastet. **Contra (entscheidend):** die (nicht-triviale, gerade um Filter + BBox gehärtete) Projektions-Logik existiert **doppelt** (Export + UI) → Drift-Risiko genau an der frisch gefixten Stelle; die UI hinge am `Building` statt an einer Lese-Naht. **Verworfen.**
- **Neu-Implementierung im Kern (Port, aber Logik neu) — Contra:** ebenfalls Duplikat gegen `plan_geometry`; verwirft die vorhandene, getestete Funktion ohne Not. **Verworfen.**

### Zu 4: Selbst-Refresh (gewählt) vs. DRW-`op` einführen

- **Selbst-Refresh (gewählt) — Pro:** kein Eingriff in [ADR-0018](0018-drw-2d-zeichen-daten.md) §2 (»kein op« bleibt); der Canvas kennt den Erfolg seines eigenen Kommandos synchron; einfachster Pfad für die Ein-Sicht-v1. **Contra:** eine zweite Sicht bliebe ohne `op` nicht automatisch synchron (benannter Re-Eval).
- **DRW-`op` einführen — Pro:** generische Benachrichtigung aller Beobachter. **Contra:** revidiert eine bewusst getroffene Fundament-Entscheidung für einen in v1 nicht existierenden zweiten Beobachter; die 3D-Sicht braucht Hilfslinien nicht. **Verworfen für v1**, als Re-Eval geführt.

## Konsequenzen

- **Positiv:** Der interaktive Erzeugungs-Weg entsteht **ohne** neue Technologie/Dependency, **ohne** Kern-Architektur-Bruch und **ohne** neue Gate-Regel: ein neues 2D-`view/`-Widget + ein `command/`-Kommando am bestehenden `EditDrawingPort` + ein neuer Kern-Read-Port `PlanViewPort` über der in den Kern gehobenen, lib-freien Projektion. Die vom Fundament benannte Lücke »der Nutzer kann noch nicht zeichnen« wird geschlossen. Die 2D-Projektion wird **eine** Quelle für Bildschirm **und** Export.
- **Negativ / Folgepflicht (Slices, Nummern im [ADR-Index](README.md)):** (a) **AK-Schärfungs-Slice** ([LH-FA-DRW-005](../../../spec/lastenheft.md#lh-fa-drw-005) interaktive Erzeugung, lösungsfrei [MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei); benutzer-beobachtbar über die Zeichenfläche) + Spec-`§1`/`§6`-Mapping + `architecture.md`-Nachzug. (b) **Lese-Naht-Refactor-Slice** (Projektion → `hexagon/services/geometry/plan_geometry` + `PlanViewPort` + Export-Migration; die 2D-Export-Decode-Orakel bleiben grün) — **sequenziert VOR** dem Canvas-Impl-Slice, orthogonal, ein reiner Umzug. (c) **Canvas-Impl-Slice(s)** (2D-Widget + `command/`-Naht + `screenToModel` + Maus-Handhabung + Composition-Root-Umschaltung + Headless-`QMouseEvent`-AK-Tests). (d) **Fang/Raster/Winkel** ([LH-FA-DRW-001](../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/002/003) je eigene Schärfungs-/Impl-Slices. [MR-006](../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor jedem Impl-Start; unabhängiges Code-Review vor Welle-Closure.
- **Keine neue arch-check/a-check-Regel:** der Canvas ist ein Qt-`ui/`-Adapter (Regel E deckt Qt-Includes bereits); der `PlanViewPort` ist ein Kern-Port; die Projektions-Hebung **stärkt** die Schichtung (Kern-Reinheit maschinell erzwungen, Regel-B-Verletzung vermieden) — eine **Verschärfung**, keine Lockerung ([AGENTS.md §2.6](../../../AGENTS.md) n/a).
- **`architecture.md`-Nachzug (Folgepflicht des AK-Schärfungs-Slice):** §1.1 Driving-Ports um `PlanViewPort` (2D-Lese-Naht) + den Canvas-Driving-Adapter; Import-Matrix bestätigt (UI zieht Kern-Port, nicht `io/`); `## Geschichte`-Provenance.
- **Rest-Risiko benannt:** die Lese-Naht-Hebung berührt die frisch gelieferte Export-Seite — als **eigener, vorgelagerter** Refactor-Slice mit den Decode-Orakeln als Netz geführt (reiner Umzug). Bliebe der Refactor zu groß, kann er schrittweise ziehen (Kern-Funktion neben `plan_geometry` etablieren, dann Export migrieren, dann `io/plan_geometry` entfernen).
- **[ADR-0001](0001-hexagonale-architektur.md)/0008/0009/0010/0018 bleiben unverändert gültig** — diese ADR baut auf ihnen auf (Schichtung/Kern-Reinheit, Benachrichtigungs-Muster, Qt-Regel-E + `ViewModelPort`-Vorbild, Headless-Infra, Fundament-Datenheimat + »kein op«).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Schichtung (heute real) | Canvas-Widget in `src/adapters/ui/view/`, Kommando in `ui/command/`; 2D-Projektion in `src/hexagon/services/geometry/` (Kern), `PlanViewPort` in `hexagon/ports/driving/`; **kein** UI→`io/`-Import (Regel B) | `make a-check` / `make arch-check` (real) |
| Kern-Reinheit (Refactor-Slice) | die in den Kern gehobene `plan_geometry` zieht **keine** Library — `bcad_hexagon` bleibt ohne `find_package`/Adapter-Dep (Link-Barriere) | `make build` |
| Qt-Grenze (Impl-Slice) | Qt-Includes nur `src/adapters/ui/` + `main.cpp` (Regel E unverändert) | `make arch-check` |
| Export-Invarianz (Refactor-Slice) | die 2D-Export-Decode-Orakel (DXF/PDF/PNG Erscheint/Fehlt) bleiben nach der Projektions-Hebung **unverändert grün** (reiner Umzug) | `make test` |
| Interaktions-AK (Impl-Slice) | synthetisierter `QMouseEvent`-Zug → `building().guide_lines` wächst um eins mit den `screenToModel`-gemappten mm; entarteter Zug (Anfang=Ende) → keine Hilfslinie; display-frei über den Surrogat-Zustand | `make test` (Xvfb, [ADR-0010](0010-headless-gl-xvfb.md)) |

## Re-Evaluierungs-Trigger

- **Zweite gleichzeitige 2D-Sicht** (die ohne `op` synchron bleiben müsste) → Selbst-Refresh (Entscheidung 4) neu bewerten, ggf. DRW-`op` / 2D-Change-Signal.
- **Fang/Raster/Winkel** ([LH-FA-DRW-001](../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/002/003) → eigene AK-/Impl-Slices; falls eine Aid persistierte Projekt-Einstellung wird (statt reiner Interaktions-Zustand), Heimat (Entscheidung 6) neu bewerten.
- **Bauteile interaktiv zeichnen / Selektion/Picking auf dem 2D-Canvas** → 2D-Lese-Naht (`PlanViewPort`) um Bauteil-/Treffer-Queries erweitern; ggf. Zusammenführung mit dem 3D-Selektions-Trigger ([ADR-0009](0009-gui-framework-qt6.md)).
- **Bemaßung** ([LH-FA-DRW-004](../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)) → eigene Entscheidung (annotations-reicher), berührt Canvas-Darstellung + Export-Subset.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-07-22 | Proposed (DRW-Interaktiv-Strang-Eröffnung welle-5 — 2D-Zeichenfläche: neues `view/`-Widget; 2D-Lese-Naht = reine Projektion in den Kern gehoben [`services/geometry/plan_geometry`] + neuer `PlanViewPort` [2D-Analog zu `ViewModelPort`], eigener Refactor-Slice vor dem Canvas; invertierbare `screenToModel`-Naht; Selbst-Refresh ohne DRW-`op`; Kommando über `ui/command/`; Fang/Raster/Winkel = UI-Interaktions-Zustand, eigene Slices; Umschalt-Layout + `QMouseEvent`-Headless-Test; keine neue Gate-Regel) | welle-5 / DRW-Interaktiv-Strang-Eröffnung |
