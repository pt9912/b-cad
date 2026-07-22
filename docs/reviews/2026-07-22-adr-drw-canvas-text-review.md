# ADR-0019 Text-Review — DRW interaktiver 2D-Zeichen-Canvas

**Datum:** 2026-07-22 · **Reviewer:** unabhängiger Agent (≠ Autor), read-only ·
**Gegenstand:** `docs/plan/adr/0019-drw-2d-canvas.md` (Status Proposed) · Muster
[ADR-0018-Text-Review](2026-07-05-adr-0018-text-review.md) (Reviewer ≠ Autor, HIGH blockiert Accept).

**Verdikt (Erst-Lauf): 1 HIGH / 1 MED / 1 LOW / 3 INFO → blockiert.** Nach Einarbeitung (unten):
**Accept-fähig (0 HIGH offen)** — der Projektinhaber-Accept steht noch aus.

## HIGH-1 (behoben) — Export-Migration braucht eine **neue** a-check-Allow-Kante; die »keine neue Regel / Verschärfung / §2.6 n/a«-Behauptung war maschinell falsch

**Befund:** `.a-check.yml` führt eine **Allow-Liste** von Import-Kanten. `io` hat nur `→ model` und
`→ ports_driven`; `geometry`/`persistence` haben zusätzlich `→ services_geo`. Alle drei 2D-Export-
Adapter (`dxf`/`pdf`/`png_export_adapter`) liegen in `io` und rufen `projectPlan`. Sobald `projectPlan`
nach `src/hexagon/services/geometry/{plan_geometry}` (Glob `services_geo`) zieht, wird der direkte Aufruf
zu `io → services_geo` — **nicht in der Allow-Liste → `make a-check` schlägt fehl**. Der Port-Weg hilft
nicht: `PlanViewPort` ist ein **Driving**-Port, und `io` hat keine `io → ports_driving`-Kante. Das
Ergänzen der Kante ist eine **Erweiterung** der erlaubten io-Importe (Allow-Liste-Relaxation) — die
**Gegen-Polarität** zur »Kante hinzufügen = Verschärfung«-Logik der Forbidden-Matrix, auf die die ADR
sich stützte. Also: (a) eine neue a-check-Zeile IST nötig, (b) sie ist eine **Lockerung**, nicht
»Verschärfung« → **§2.6 ist nicht n/a**; diese ADR ist genau die Autorisierung der Kante.

**Fix (eingearbeitet):** Konsequenzen + Fitness + `architecture.md`-Nachzug + Index-Kopfzeile ehrlich
umgeschrieben: der Refactor-Slice ergänzt `{from: io, to: services_geo}` in `.a-check.yml` (Präzedenz
`geometry`/`persistence → services_geo` — legitime Inward-Kern-Helfer-Nutzung), **ADR-0019 autorisiert
die Kante** ([§2.6](../../harness/conventions.md)). Bonus aus derselben Analyse verankert: die Allow-
Liste erlaubt `ui_view → ports_driving` **nicht** (nur `ui_command`) — der `PlanViewPort`-**Read** des
Canvas läuft daher über eine `ui/command/`-Naht (wie der 3D-`ViewModelPort`-Read), nicht direkt im
Zeichenwidget.

## MED-1 (behoben) — Selbst-Refresh-Loch reicht über »zweite 2D-Sicht« hinaus

**Befund:** `addGuideLine` meldet keinen `op`, `ModelChangeOp` hat kein DRW-Glied; der Canvas
self-refresht nur nach **eigenem** Kommando. Aber `EditDrawingPort` ist auch vom **Plugin-Host**
erreichbar (ADR-0018: »der Plugin-Host erhält den Port … gratis«). Eine Plugin-/Fremd-Adapter-Mutation
bei offenem Canvas → kein `op` → **Canvas stale**. Das ist ein **distinktes** Loch zur »zweiten 2D-
Sicht« und war nicht benannt (die »convenient-now, corner-later«-Form). In v1 verdrahtet die GUI keinen
konkurrierenden Schreiber bei offenem Canvas → Scope-Ehrlichkeit, kein garantierter Bug.
**Fix (eingearbeitet):** Entscheidung 4 + Re-Eval auf »**jede un-benachrichtigte `guide_lines`-Mutation,
die ein offener Canvas beobachten müsste** (inkl. Plugin-Host / anderer Driving-Adapter), nicht nur eine
zweite 2D-Sicht« verbreitert — der Projektinhaber akzeptiert die Grenze wissentlich.

## LOW-1 (behoben) — Entscheidung 7 mit ADR-0009 (f) reconcilen

(f) hielt `tests/e2e/` leer, »bis Interaktion relevant wird« — sie ist es jetzt, aber Surrogat +
`screenToModel` halten die AK display-frei (Widget-`QMouseEvent`, kein e2e-Treiber). **Fix:** ein Satz
ergänzt, warum `tests/e2e/` leer bleiben darf (bis eine nur-e2e-prüfbare AK entsteht, z. B. Selektion).

## INFO (solide, keine Findings)

- **Zwei Zugriffsstile für eine Projektion (Export Direkt-Aufruf vs. Canvas via Port) sind solide +
  präzedenziert:** die 3D-Seite macht es genauso (Geometrie-Adapter rufen `services/geometry` direkt,
  die UI zieht `ViewModelPort`). *(Genau dieser Split verlangt aber die HIGH-1-Kante für die io-Hälfte.)*
- **Entscheidung-2-Prämissen verifiziert:** `projectPlan` ist rein (nur `hexagon/model/*` + STL); der
  Kern-CMake erzwingt No-Deps als Link-Barriere; `services/geometry/` ist die etablierte Helfer-Heimat;
  UI→`io/` ist echt verboten. »Lib-Freiheit maschinell erzwungen« hält für die kern-residente Funktion.
- **Entscheidung 6** hat die Persist-Grenze bereits als Re-Eval gehedged. **Entscheidungen 1/3/5 solide**
  (view/-Platzierung, `MeshSource`-Naht-Analogie, single `setCentralWidget` bestätigt).
- **MR-014-konform:** null `slice-[0-9]`-Tokens im ADR (Body + Geschichte). Alternativen fair (kein
  Strohmann). Proposed/Geschichte-Eintrag korrekt.

---

**Einarbeitung (Autor, 2026-07-22):** HIGH-1 (a-check-Kante `io → services_geo` von der ADR autorisiert;
§2.6 nicht n/a; Read über `ui/command/`), MED-1 (Selbst-Refresh-Grenze auf Plugin-Host/externe
Mutationen verbreitert), LOW-1 (e2e-Reconciliation) — alle in ADR-0019 verankert. **0 HIGH offen →
Accept-fähig; Projektinhaber-Accept ausstehend.**
