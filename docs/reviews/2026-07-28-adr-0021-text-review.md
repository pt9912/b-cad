# ADR-0021 Text-Review — Die Wand im 2D-Canvas

**Review-Art:** **Design-Review** (Lösungs-Schnitt gegen die Architektur — Layer, Schnittstellen,
ADR-Verträglichkeit; Reviewer-Skill v1.0 §Review-Arten). **Kein** Plan-Review (der Slice-Plan ist
nicht der Gegenstand), **kein** Code-Review (kein Diff).

**Gegenstand:** [`docs/plan/adr/0021-wand-im-2d-canvas.md`](../plan/adr/0021-wand-im-2d-canvas.md),
Status `Proposed`, 215 Zeilen · **Reviewer:** unabhängiger Agent (≠ Autor, kein Autoren-Kontext),
read-only · **Skill:** [`.harness/skills/reviewer.md`](../../.harness/skills/reviewer.md) v1.0 ·
**Modell:** Claude Opus 5 (1M) · **Datum:** 2026-07-28 ·
**Muster:** [ADR-0019-Text-Review](2026-07-22-adr-drw-canvas-text-review.md) (dieselbe Art).

**Eingangs-Kontext (gelesen, nicht referiert):**
[`CLAUDE.md`](../../CLAUDE.md) · [`AGENTS.md`](../../AGENTS.md) ·
[`harness/README.md`](../../harness/README.md) · [`harness/conventions.md`](../../harness/conventions.md)
(MR-006/008/009/010/014/020/023 sowie MR-011/013/015/016/017/018/021/022) ·
[ADR-Index](../plan/adr/README.md) inkl. Folgepflicht-Block · [ADR-0001](../plan/adr/0001-hexagonale-architektur.md) ·
[ADR-0006](../plan/adr/0006-relationales-schema-design.md) · [ADR-0008](../plan/adr/0008-aenderungs-benachrichtigung.md) ·
[ADR-0009](../plan/adr/0009-gui-framework-qt6.md) · [ADR-0018](../plan/adr/0018-drw-2d-zeichen-daten.md) ·
[ADR-0019](../plan/adr/0019-drw-2d-canvas.md) · [ADR-0020](../plan/adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) ·
[`spec/lastenheft.md`](../../spec/lastenheft.md) · [`spec/spezifikation.md`](../../spec/spezifikation.md) ·
[`.a-check.yml`](../../.a-check.yml) · [`Makefile`](../../Makefile) / [`a-check.mk`](../../a-check.mk) /
[`d-check.mk`](../../d-check.mk) · `tests/e2e/` · Code:
[`edit_structure_port.h`](../../src/hexagon/ports/driving/edit_structure_port.h),
[`plan_view_port.h`](../../src/hexagon/ports/driving/plan_view_port.h),
[`view_model_port.h`](../../src/hexagon/ports/driving/view_model_port.h),
[`evaluate_port.h`](../../src/hexagon/ports/driving/evaluate_port.h),
[`edit_drawing_port.h`](../../src/hexagon/ports/driving/edit_drawing_port.h),
[`structure_edit_service.cpp`](../../src/hexagon/services/structure_edit_service.cpp),
[`plan_view.h`](../../src/hexagon/model/plan_view.h), [`wall.h`](../../src/hexagon/model/wall.h),
[`plan_projection.cpp`](../../src/hexagon/services/geometry/plan_projection.cpp),
[`canvas_widget.h`](../../src/adapters/ui/view/canvas_widget.h)/`.cpp`,
[`view_transform.h`](../../src/adapters/ui/view/view_transform.h),
[`snap.cpp`](../../src/adapters/ui/view/snap.cpp), `src/adapters/ui/command/`,
[`main.cpp`](../../src/main.cpp), [`test_canvas_widget.cpp`](../../tests/adapters/test_canvas_widget.cpp).

**Keine Repo-Mutation außer dieser Datei. Kein `make gates`-Lauf** (Auftrag). `make docs-check` **nicht**
gelaufen — kein Befund hängt daran (Anker wurden gegen die Ziel-Überschriften stichprobenartig
von Hand geprüft, s. Negativbefunde).

---

## HIGH-1 — Fehlende Entscheidung: die **Lese-Naht für die anzuzeigenden Wand-Parameter** existiert nicht und wird nirgends entschieden

- **kategorie:** HIGH
- **quelle:** ADR-0021 Entscheidung 4 / 11 / 13; [ADR-0019](../plan/adr/0019-drw-2d-canvas.md) E2 (2D-Lese-Naht); `AGENTS.md` §2.5 (nach `Accepted` immutabel)
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:53 (E4), `:103` (E11), `:117` (E13); `src/hexagon/ports/driving/` (Port-Inventar)
- **befund:** Entscheidung 4 sagt zu: „Die Parameter der ausgewählten Wand werden in einem festen,
  nicht-modalen Bereich des Fensters **angezeigt** und geändert"; Entscheidung 13 benennt sie als
  Stärke und Höhe. **Es gibt heute keinen Weg, diese Werte zu lesen.** Kein Driving-Port liefert sie:
  `PlanViewPort` liefert `model::PlanView` mit `PlanSegment{x1,y1,x2,y2}` (`src/hexagon/model/plan_view.h`:14–19),
  `ViewModelPort` liefert Dreiecksnetze, `EvaluatePort` liefert Flächen/Volumen/Materialien/Tür-/Fenster-Listen,
  `EditStructurePort` trägt **ausschließlich Mutatoren** (kein `wallThickness`/`wallHeight`-Getter,
  `src/hexagon/ports/driving/edit_structure_port.h`:31–187). **Entscheidung 11 schließt die Lücke
  nicht:** sie erweitert das Segment ausdrücklich nur um eine „optionale Herkunft (**Art + Id**)" und
  begründet das mit Treffer-Prüfung, `WallId` für die Änderung und Beobachtbarkeit — nicht mit dem
  Lesen der Werte. Der einzige heute im Repo praktizierte Weg — ein `const model::Building&`-Callable
  in ein `ui/command/`-Objekt, wie ihn `src/main.cpp`:606 an den `ProjectMenuHandler` reicht — ist
  **genau die Alternative, die Entscheidung 11 verwirft** („Canvas liest `Building` direkt — Contra:
  hebelt die Lese-Naht aus … **Verworfen**", `:161`). Damit steht der Impl-Slice (d) vor einer
  Entscheidung, die die dann immutable ADR nicht getroffen hat: entweder die verworfene Alternative
  bauen, oder eine im ADR nicht vorgesehene Port-/Naht-Erweiterung erfinden. Beobachtbarer
  Nebendruck: `ParamResult.applied_mm` trägt laut Header-Kommentar „bei `Rejected` der **unveränderte
  Ist-Wert**" (`edit_structure_port.h`:25) — ein Mutator-Aufruf mit nicht-endlicher Eingabe wäre der
  naheliegende Ersatz-Getter.
- **verifizierbar:** teilweise — die Abwesenheit ist am Port-Inventar (`src/hexagon/ports/driving/`)
  und an `model::PlanView` **grep-belegt**; ein Gate, das die fehlende Entscheidung meldet, existiert
  nicht (es ist eine Design-Lücke, kein Regelbruch).

## HIGH-2 — Fehlende Entscheidung: der **Geschoss-Skopus der Treffer-Prüfung**; die übernommene Fang-Regel wählt beweisbar unsichtbare Wände

- **kategorie:** HIGH
- **quelle:** ADR-0021 Entscheidung 3 (+ 4, 11, 13); [`LH-FA-DRW-001`](../../spec/lastenheft.md#lh-fa-drw-001) (Fang über alle Geschosse); [ADR-0019](../plan/adr/0019-drw-2d-canvas.md) E1
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:47–49 (E3); `src/adapters/ui/view/snap.cpp`:33; `src/adapters/ui/view/canvas_widget.cpp` (`paintEvent`, Filter `sp.storey_id != active_storey_id_`)
- **befund:** Entscheidung 3 legt die Treffer-Prüfung auf den **Bildschirmraum** über der 2D-Lese-Naht
  fest und übernimmt die Tie-Break-Regel ausdrücklich vom Fang („bei gleicher Distanz der in der
  festen Iterationsreihenfolge **zuerst besuchte** — dieselbe Regel, die der Fang trägt"). Die
  Quelle dieser Naht, `model::PlanView`, trägt **alle** Geschosse (`plan_view.h`:32–39); der Canvas
  **zeichnet** aber nur das aktive (`canvas_widget.cpp`, `paintEvent`: `if (sp.storey_id != active_storey_id_) continue;`),
  und die referenzierte Fang-Regel iteriert im Bestand **ohne Geschoss-Filter** über `plan.storeys`
  (`snap.cpp`:33) — für Hilfslinien ist das vom Lastenheft ausdrücklich gedeckt („gefangen wird auf
  die markanten Punkte **aller Geschosse** des Projekts, dargestellt wird das aktive",
  `spec/lastenheft.md`:641–642). **Ob die Treffer-Prüfung auf das dargestellte Geschoss beschränkt
  ist, steht weder in `## Entscheidung` noch im „Nicht offen"-Block** — und es fällt nicht unter die
  dort delegierte „Exakte Widget-/Signatur-/Konstanten-Gestalt", weil es kein Konstanten-, sondern
  ein Semantik-Schnitt ist. Beim Fang ist der Skopus folgenlos (das Ergebnis ist ein mm-Wert), bei
  der Auswahl entscheidet er über die **Identität**: im Demo-Modell des Repos sind vier EG- und vier
  OG-Wände in 2D **koinzident** (`src/main.cpp`:79–89, u. a. beidseitig `(0,0)–(8000,0)`), die
  Bildschirm-Distanzen sind exakt gleich, und der ausgeschriebene Tie-Break („zuerst besucht",
  Geschosse in Speicherreihenfolge) wählt deterministisch die **unsichtbare EG-Wand**. Entscheidung 4
  zeigte dann die Parameter einer nicht sichtbaren Wand an, Entscheidung 13 änderte sie.
- **verifizierbar:** ja — ein AK-Test „Klick auf eine sichtbare OG-Wand des Demo-Modells ⇒ ausgewählte
  `WallId` gehört zum aktiven Geschoss" (`make test`) würde den Befund zeigen; ein solcher Test
  existiert heute nicht.

## HIGH-3 — Fehlende Entscheidung: die **Lebensdauer der Auswahl über `ModelReplaced`**; die Fehler-Barriere aus Entscheidung 10 greift hier nicht

- **kategorie:** HIGH
- **quelle:** ADR-0021 Entscheidung 3 / 10 / 13; [ADR-0008](../plan/adr/0008-aenderungs-benachrichtigung.md) #3/#4; `spec/spezifikation.md` §1 (`op = ModelReplaced`); `harness/README.md` §Safety („Datenverlust am Gebäudemodell ist der schärfste Fehlerfall")
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:47–49 (E3), `:97-101` (E10); `src/hexagon/services/structure_edit_service.cpp`:373–407 (`replaceBuilding`)
- **befund:** Entscheidung 3 macht die Auswahl zu Canvas-Zustand, der eine **Wand-Identität** hält
  (Entscheidung 14: „ausgewählte Wand-Id" als lesbare Widget-Eigenschaft); Entscheidung 13 erlaubt
  darüber Mutationen. **Was mit dieser Id bei einem Modell-Austausch geschieht, entscheidet die ADR
  nicht** — weder in `## Entscheidung` noch im „Nicht offen"-Block. Entscheidung 10 deckt den Fall
  **nicht** ab: sie fängt den Wurf bei **unbekannter** Id, aber `replaceBuilding` setzt
  `next_wall_id_ = maxIdValue(building_.walls) + 1` (`structure_edit_service.cpp`:395) — ein geladenes
  Projekt trägt seine eigenen, dichten Wand-Ids. Eine nach dem Projekt-Wechsel stehengebliebene
  Auswahl `WallId 5` bezeichnet im neuen Projekt mit hoher Wahrscheinlichkeit eine **existierende,
  andere** Wand: `setWallThickness` wirft nicht, die Barriere schweigt, und die Mutation trifft
  **still eine Wand, die der Benutzer nie ausgewählt hat** (samt Nachbar-Rebuild und
  Raum-Neuerkennung, `structure_edit_service.cpp`:187–217). Das Repo hat diese Klasse für den
  **Zeichen**-Pfad bereits einmal erkannt und gelöst (`DrawingTargetSinks` re-auflösen Geschoss und
  Ebene nach jedem Öffnen, `src/main.cpp`:601–615); Entscheidung 10 zitiert genau diese Erfahrung
  („die Zeichen-Ziele sind injizierte Ids, die nach einem Projekt-Wechsel neu aufgelöst werden"),
  zieht für die **Auswahl** aber keine Konsequenz.
- **verifizierbar:** ja — ein AK-Test „Wand auswählen → Projekt laden → Stärke ändern" (`make test`,
  Muster `tests/adapters/test_project_open_handler.cpp`) würde die Fehl-Mutation zeigen; er existiert
  heute nicht.

---

## MED-1 — Entscheidung 14 begründet sich mit einer Neuheit, die es im Repo nicht gibt (und die dieselbe Entscheidung zwei Absätze später als Präzedenz zitiert)

- **kategorie:** MEDIUM
- **quelle:** ADR-0021 Entscheidung 14; [`LH-FA-DRW-001`](../../spec/lastenheft.md#lh-fa-drw-001) „Happy Path (Anzeige)"/„Boundary (Anzeige)"; `spec/spezifikation.md` §1 `LH-FA-DRW-001.a` („Anzeige des Fang-Ziels")
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:125 und `:130`
- **befund:** Entscheidung 14 trägt ihre Begründung auf der Behauptung, Werkzeug-Modus, Auswahl und
  Hinweis hätten „**anders als jede bisherige Interaktions-Zusage dieses Repos** — kein Korrelat
  außerhalb des Widgets", und „Die bisherigen AK hingen an einem **Modell**-Surrogat, das über
  Persistenz und Export weiter beobachtbar ist". Beides ist am Artefakt falsch: die **Fang-Anzeige**
  ist genau eine solche Zusage — `CanvasWidget::snapPreview()` ist reiner Widget-Zustand
  („kein Modell-Datum, keine Persistenz, kein Schema-Feld, kein `op`",
  `spec/spezifikation.md` §1 `LH-FA-DRW-001.a`), ihr Lastenheft-Konjunkt hat kein Modell-Korrelat,
  und sie wird bereits über **Surrogat + Tinten-Sonde** nachgewiesen
  (`tests/adapters/test_canvas_widget.cpp`:308 `inkPixels`, `:443`
  `LH_FA_DRW_001_MarkerErzeugtTinte`). Dieselbe Entscheidung nennt diese Sonde fünf Zeilen später
  ausdrücklich als Präzedenz („dieselbe zweite Nachweis-Ebene, die die Fang-Anzeige etabliert hat").
  Die **Entscheidung** (zwei Ebenen, `tests/e2e/` bleibt leer) bleibt tragfähig; ihre
  Alleinstellungs-Begründung nicht — und sie wird mit `Accepted` unveränderlich.
- **verifizierbar:** ja — `make test` (die genannten Tests existieren und laufen im Gate).

## MED-2 — Fitness-Function-Zeile „Schema-Unberührtheit" beschreibt `make schema-check` falsch; die Zusicherung diskriminiert nicht

- **kategorie:** MEDIUM
- **quelle:** ADR-0021 §Fitness Function; `AGENTS.md` §3 (`make schema-check`: „`schema.sql` == d-migrate(`data-model.yaml`)"); [ADR-0018](../plan/adr/0018-drw-2d-zeichen-daten.md) §Fitness (korrekte Formulierung); Reviewer-Skill §Klassifikation („Test, dessen Zusicherung nicht diskriminiert")
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:195
- **befund:** Die Zeile lautet „`data-model.yaml` == d-migrate-Erzeugnis; Auswahl/Modus erzeugen
  **kein** Schema-Feld → `make schema-check`". Zwei Abweichungen: (a) **Richtung vertauscht** —
  `data-model.yaml` ist die Quelle, `schema.sql` das d-migrate-Erzeugnis (`AGENTS.md` §3,
  `Makefile:134`); (b) **der Sensor prüft Drift, nicht Abwesenheit**: eine konsistent in *beiden*
  Artefakten ergänzte Spalte (der reguläre Weg über `make schema-regen`, `Makefile:150`) lässt
  `schema-check` **grün** — die zugesicherte Eigenschaft „erzeugen kein Schema-Feld" wird von diesem
  Target nicht gefangen. Die belastbare Formulierung derselben Zusage steht in den Konsequenzen
  („`data-model.yaml`/`schema.sql` bleiben **byte-unberührt**", `:182`) und ist git-, nicht
  gate-verifiziert.
- **verifizierbar:** ja — Gegenprobe: eine Spalte in `data-model.yaml` ergänzen, `make schema-regen`,
  dann `make schema-check` (bleibt grün).

## MED-3 — Die vorgeschriebene `LH-FA-DRW-005`-Teilumfang-Nachziehung ist unvollständig: Entscheidung 3 falsifiziert dort auch „Selektion"

- **kategorie:** MEDIUM
- **quelle:** ADR-0021 §Konsequenzen (a); [`LH-FA-DRW-005`](../../spec/lastenheft.md#lh-fa-drw-005) §Teilumfang; MR-020 (Folgepflicht-Sichtbarkeit)
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:178; `spec/lastenheft.md`:683–684
- **befund:** Die Folgepflicht (a) verlangt, die `LH-FA-DRW-005`-Teilumfang-Klausel
  („interaktives Zeichnen von Bauteilen bleibt offen") nachzuziehen. Die Klausel führt dort aber
  **drei** Posten als offen: „das **interaktive Zeichnen von Bauteilen**, ein **Ebenen-Bedien-Panel**
  und **Selektion** bleiben **ausdrücklich offen** (späterer UI-Umfang), kein stiller Vollumfang"
  (`spec/lastenheft.md`:683–684). Entscheidung 3 liefert **Selektion** — der Posten wird von der
  Folgepflicht nicht genannt und bliebe nach dem AK-Schärfungs-Slice als falsche Aussage stehen
  (das Ebenen-Bedien-Panel bleibt korrekterweise offen und ist im „Nicht offen"-Block benannt).
  Die ADR fixiert die Folgepflicht-Liste mit `Accepted`.
- **verifizierbar:** nein — kein Gate prüft die Vollständigkeit einer Teilumfang-Klausel
  (`docs-check` sieht nur Referenz-Integrität); Träger ist die MR-006-Linse des Schärfungs-Slice.

## MED-4 — „fünf bestehende Aufrufer" (Entscheidung 10) ist am Artefakt nicht reproduzierbar

- **kategorie:** MEDIUM
- **quelle:** ADR-0021 Entscheidung 10; Reviewer-Skill §Klassifikation (Quellen-Behauptung)
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:101
- **befund:** Die Entscheidung stützt „kein Umbau des Port-Vertrags" auf die Zahl: „den wertbasiert
  umzuschreiben beträfe **fünf bestehende Aufrufer**". Gezählt am Repo ergibt sich für
  `addWall`/`setWallThickness`/`setWallHeight`: **2 Produktions-Dateien mit 12 Aufrufstellen**
  (`src/main.cpp` 10, `plugins/example/example_plugin.cpp` 2) und **17 Test-Dateien**. Keine
  Zählweise (Dateien, Aufrufstellen, Produktion/Tests) ergibt fünf; die Bezugsgröße wird im Text
  nicht definiert. Die Entscheidung selbst („die Senke ist die Barriere") trägt auch ohne die Zahl.
- **verifizierbar:** ja — `grep -rn "addWall(\|setWallThickness(\|setWallHeight(" src/ plugins/ tests/`.

## MED-5 — Entscheidung 9: die Eckenschluss-Begründung trägt den übernommenen, geschoss-übergreifenden Fang nicht

- **kategorie:** MEDIUM
- **quelle:** ADR-0021 Entscheidung 9; [`LH-FA-WAL-006`](../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) §Teilumfang; [`LH-FA-DRW-001`](../../spec/lastenheft.md#lh-fa-drw-001)
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:89–95; `src/adapters/ui/view/snap.cpp`:33; `spec/lastenheft.md`:182–185, `:641-642`
- **befund:** Entscheidung 9 dehnt den Endpunkt-Fang auf die Wand-Geste aus („wirkt auf den Wand-Zug
  **wie** auf den Hilfslinien-Zug") und begründet das mit dem Eckenschluss: „[LH-FA-WAL-006] setzt
  einen **gemeinsamen** Endpunkt voraus, dessen Toleranz die Spezifikation auf 0,1 mm festlegt".
  Der geltende `LH-FA-WAL-006`-**Teilumfang** verlangt jedoch einen gemeinsamen Endpunkt „im
  **selben Geschoss** — genau zwei Wände am Punkt" (`spec/lastenheft.md`:182–185), während der
  übernommene Fang die Endpunkte **aller** Geschosse anbietet (`snap.cpp`:33; für Hilfslinien
  ausdrücklich so gewollt, `spec/lastenheft.md`:641–642). Ein Fang auf einen fremd-geschossigen
  Endpunkt erzeugt damit exakt das, wogegen die Begründung argumentiert: eine sichtbar markierte
  Einrast-Stelle **ohne** Eckenschluss. Die ADR entscheidet weder, dass der Wand-Fang auf das aktive
  Geschoss beschränkt wird, noch benennt sie die Restlücke (vgl. HIGH-2 — dort mit
  Identitäts-Folge statt nur Erwartungs-Folge).
- **verifizierbar:** ja — `make test`: ein Zug, dessen Endpunkt nur auf einem fremden Geschoss einen
  Fang-Punkt hat, erzeugt eine Wand mit exakt diesen mm und **keine** Eck-Nachbar-Meldung.
- **Hinweis:** die Grenze war bekannt — [Plan-Report 2](2026-07-28-slice-056-plan-2.md) INFO-2
  („`snapTarget` sieht die **ganze** `PlanView` (alle Geschosse), der Canvas zeichnet nur das aktive
  … Sie wirkt unverändert für einen Wand-Zug") — und ist in der ADR nicht angekommen.

## MED-6 — Entscheidung 10 zählt die Wurf-Quellen unvollständig; der `E-GEO-002`-Ausgang hat in Entscheidung 5 keine Zeile

- **kategorie:** MEDIUM
- **quelle:** ADR-0021 Entscheidung 10 + 5; `spec/spezifikation.md` §4 [`E-GEO-002`](../../spec/spezifikation.md#4-fehler-codes-und-logging-felder); Reviewer-Skill §Klassifikation („unklare Fehlerbehandlung am Rand des Spec-Bereichs (`E-*`-Codes)")
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:99 und `:61-67`; `src/hexagon/services/structure_edit_service.cpp`:151–154, `:166`, `:202`, `:234`
- **befund:** Entscheidung 10 zählt zwei Wurf-Quellen auf („`addWall` wirft bei unbekannter
  Geschoss-Id, `setWallThickness`/`setWallHeight` werfen bei unbekannter Wand-Id"). Es gibt eine
  **dritte**: alle drei Methoden extrudieren **vor** dem Commit (`buildWallSolid` +
  `rebuildAffectedNeighbors`, `structure_edit_service.cpp`:166, `:202`, `:234`); scheitert eine
  Geometrie-Operation, propagiert der Wurf aus der Port-Methode heraus — der Code sagt es wörtlich:
  „Schlägt eine Geometrie-Operation fehl (Wurf), bleibt das Modell unverändert (**E-GEO-002**)"
  (`:151-154`). Für die **Entscheidung** ist das folgenlos (eine Senke, die fängt, fängt auch diesen
  Wurf); folgenreich ist es für **Entscheidung 5**: deren Tabelle definiert das Hinweis-Vokabular für
  genau **drei** Ausgänge (`Clamped` · `Rejected` · verworfene Null-Längen-Wand), Entscheidung 10
  routet aber *jeden* gefangenen Wurf auf ebendiese Hinweis-Zeile („sie fängt den Wurf und meldet ihn
  über die Hinweis-Zeile (Entscheidung 5)"). Für den **vierten** Ausgang — Geometrie-Fehlschlag,
  `E-GEO-002`, Modell unverändert — existiert damit weder eine Hinweis-Zeile noch die Feststellung,
  dass er unerreichbar ist (wie sie Entscheidung 8 für `E-GEO-001` trifft). Der Punkt war benannt:
  [Plan-Report 2](2026-07-28-slice-056-plan-2.md) INFO-3 und
  [Plan-Report 3](2026-07-28-slice-056-plan-3.md) MEDIUM-6; `E-GEO-002` kommt in der ADR **null** mal vor.
- **verifizierbar:** ja — `grep -c "E-GEO-002" docs/plan/adr/0021-wand-im-2d-canvas.md` ⇒ 0;
  die Wurf-Pfade sind im Code belegt.

## MED-7 — Entscheidung 4 begründet sich mit einer Sicht, die die Änderung nicht zeigt; der vorgeschriebene interaktive WAL-002/003-Konjunkt hat kein benanntes Korrelat

- **kategorie:** MEDIUM
- **quelle:** ADR-0021 Entscheidung 4 + §Konsequenzen (a); [`LH-FA-WAL-002`](../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren); MR-008 (AK benutzer-beobachtbar); [ADR-0019](../plan/adr/0019-drw-2d-canvas.md) E7 (Umschalt-Layout)
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:55, `:178`; `src/hexagon/services/geometry/plan_projection.cpp`:50–53; `src/main.cpp`:584–586
- **befund:** Entscheidung 4 verwirft den modalen Dialog mit zwei Gründen: er „**verdeckt die Sicht**"
  und macht „die Änderung erst beim Schließen wirksam". Der zweite trägt; der erste nicht: die Sicht,
  die während der Bedienung offen ist, zeigt die Änderung **nicht**. Die 2D-Zeichenfläche zeichnet
  **Wand-Achsen** (`projectPlan` emittiert `{wall.start, wall.end}`, `plan_projection.cpp`:50–53) —
  eine Stärke-Änderung bewegt die Achse nicht; und der 3D-Körper, den
  [`LH-FA-WAL-002`](../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) als Korrelat der
  „**sofort**"-Zusage benennt, liegt im **anderen** `QTabWidget`-Reiter (`main.cpp`:584–586,
  ADR-0019 E7) und ist während der 2D-Arbeit nicht sichtbar. Das schlägt auf die Folgepflicht (a)
  durch: die ADR schreibt „[LH-FA-WAL-002]/003 je ein **interaktiver Konjunkt**" vor, ohne zu sagen,
  woran der Benutzer die Wirkung sieht — MR-008 verlangt für genau diesen Konjunkt
  benutzer-beobachtbare AK. Der Befund war benannt:
  [Plan-Report 3](2026-07-28-slice-056-plan-3.md) LOW-4.
- **verifizierbar:** ja — `make test`: eine Sonde „Stärke ändern ⇒ `PlanView`-Segmente unverändert"
  ist am Kern-Werttyp direkt prüfbar; die Tab-Sichtbarkeit an `main.cpp`:584–586.

---

## LOW-1 — Ein wörtliches Zitat wird zwei Quellen zugeschrieben; eine zweite Zitat-Stelle ist eine Paraphrase in Anführungszeichen

- **kategorie:** LOW · **quelle:** [ADR-0009](../plan/adr/0009-gui-framework-qt6.md) (f), [ADR-0019](../plan/adr/0019-drw-2d-canvas.md) E7
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:127 und `:9`
- **befund:** `:127` schreibt „**[ADR-0009] (f) und [ADR-0019] Entscheidung 7** halten `tests/e2e/`
  genau bis zu diesem Punkt leer — „bis eine AK entsteht, die nur echt end-to-end prüfbar ist
  (z. B. Selektion / mehrschrittige Interaktion)"". Der zitierte Satz steht **wörtlich nur** in
  ADR-0019 E7; ADR-0009 (f) formuliert eine andere, schwächere Bedingung: „ein Treiber wird mit
  Interaktion/Selektion **relevant**". `:9` gibt diese Stelle als „bis ein Treiber mit
  Interaktion/Selektion relevant wird" in Anführungszeichen wieder — sinnwahrend, aber nicht wörtlich.
  Der Schluss der Entscheidung 14 (der Auslöser tritt ein, die Bedingung „nur e2e prüfbar" ist nicht
  erfüllt) bleibt davon unberührt.
- **verifizierbar:** ja — Textvergleich `docs/plan/adr/0009-gui-framework-qt6.md`:88–91.

## LOW-2 — „löst genau die vier Punkte ein" widerspricht der eigenen Klammer; ADR-0019 führt „Layer-Bedien-Panel" nicht im Re-Eval-Block

- **kategorie:** LOW · **quelle:** [ADR-0019](../plan/adr/0019-drw-2d-canvas.md) §Kontext-Abgrenzung vs. §Re-Evaluierungs-Trigger
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:184
- **befund:** „diese ADR löst **genau die vier Punkte** ein, die sie als Re-Eval führte (Bauteile
  zeichnen, Selektion/Picking — Layer-Panel und Bemaßung **bleiben offen**)" — die Klammer nimmt die
  Aussage des Hauptsatzes zur Hälfte zurück (zwei von vier). Zusätzlich stammt die Vierer-Liste aus
  dem **Abgrenzungs**-Block von ADR-0019 (`0019:32`); der dortige §Re-Evaluierungs-Trigger-Block
  nennt „Layer-Bedien-Panel" **nicht** und führt „Bauteile zeichnen / Selektion-Picking" als **einen**
  Eintrag (`0019:95-98`). Der Kontext-Block der ADR-0021 (`:17`) zitiert die Abgrenzung korrekt und
  wörtlich.
- **verifizierbar:** nein (redaktionell).

## LOW-3 — Entscheidung 7 schreibt die Option-A-Bauform ADR-0019 Entscheidung 5 zu, die eine andere Bauform festlegte

- **kategorie:** LOW · **quelle:** [ADR-0019](../plan/adr/0019-drw-2d-canvas.md) E5; [ADR-Index](../plan/adr/README.md) Folgepflicht-Zeile zu slice-043
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:79
- **befund:** „port-frei in den Canvas verdrahtet — **genau die Bauform**, die [ADR-0019]
  Entscheidung 5 für den ersten UI-Mutator festgelegt hat". ADR-0019 E5 legt fest, dass die Mutation
  über ein `ui/command/`-Objekt läuft — konkretisiert aber „**analog zur bestehenden `MeshSource`-Naht
  (declared `view/`, implemented `command/`)**" (`0019:50`). Der Bestand folgt dieser Konkretisierung
  **nicht**: der Canvas ist über `std::function` verdrahtet („Option A … **kein** `command/ → view/`-Include,
  `adapter_sink` unverändert", ADR-Index-Folgepflicht zu slice-043; `canvas_widget.h`:28–32). Der
  tragende Teil von E5 (Driving-Port nur unter `command/`) ist gewahrt; „genau die Bauform"
  überzeichnet.
- **verifizierbar:** ja — `src/adapters/ui/view/canvas_widget.h`:56–62, `src/main.cpp`:574–580.

## LOW-4 — Zwei der sechs Fitness-Zeilen binden auf **Nicht-Gate**-Targets, ohne das zu vermerken

- **kategorie:** LOW · **quelle:** `AGENTS.md` §3 (`schema-check`/`golden-check`: „**nicht** in `gates` → CI-Befehlsliste"); [ADR-0019](../plan/adr/0019-drw-2d-canvas.md)/[ADR-0018](../plan/adr/0018-drw-2d-zeichen-daten.md) §Fitness (Präzedenz: Gate-Status je Zeile vermerkt)
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:195–196; `Makefile:204` (`gates: docs-check a-check arch-check lint test coverage-gate`)
- **befund:** Die Zeilen „Schema-Unberührtheit → `make schema-check`" und „Additivität der Lese-Naht →
  `make golden-check`" benennen reale Targets (`Makefile:134` bzw. `:189`), die aber **kein**
  `gates`-Member sind. Für die Golden-Zusage existiert daneben die gate-getragene Sonde
  `GoldenExport.*` in `make test` (`tests/adapters/test_golden_export.cpp`:102–119, sechs
  Byte-Vergleiche) — die belastbarere Bindung derselben Aussage. ADR-0018/0019 markierten den
  Gate-Status je Zeile („real"), ADR-0021 nicht.
- **verifizierbar:** ja — `Makefile:204`.

## LOW-5 — Verglichene Alternativen nur für 6 der 14 Entscheidungen, ohne Begründung der Auslassung

- **kategorie:** LOW · **quelle:** ADR-0021 §Kontext (die eigene Methoden-Prämisse) sowie die DoD des Slice-Plans („die vierzehn Fragen … **je entschieden mit verglichenen Alternativen**")
- **pfad:** `docs/plan/adr/0021-wand-im-2d-canvas.md`:21, `:134-166`
- **befund:** Der Kontext begründet den Stopp bei vierzehn damit, dass „ausformulierte Entscheidungen
  **mit Alternativen und Konsequenzen** eine fehlende Entscheidung besser zeigen als eine Tabelle mit
  Überschriften" (`:21`). Der Block `## Verglichene Alternativen` trägt aber nur sechs Einträge
  (zu 1, 2, 3, 6, 11, 14); acht Entscheidungen (4, 5, 7, 8, 9, 10, 12, 13) haben keinen. Fünf davon
  tragen ihre verworfene Option in der Prosa (4, 5, 10, 12, 13); **8 und 9 tragen nirgends eine** —
  und 9 ist genau die Entscheidung, deren Begründung MED-5 als halb-tragend ausweist. Die Auslassung
  wird nicht begründet; für Entscheidung 4 bleibt zudem die im Slice-Plan mitgeführte dritte Option
  („Panel, Dialog **oder Inline-Eingabe**") unerwähnt.
- **verifizierbar:** nein (methodisch/redaktionell).

---

## INFO

- **INFO-1 — Entscheidung 7 erklärt sich selbst zur Nicht-Entscheidung** („Diese ADR stellt das fest,
  sie entscheidet es nicht", `:81`), zählt aber unter den vierzehn. Das ist ehrlich und
  begründet („eine unausgesprochene Selbstverständlichkeit wird in einem Impl-Slice zur Abkürzung");
  kein Handlungsbedarf — nur der Hinweis, dass die Zahl „vierzehn" eine Feststellung mitträgt.
- **INFO-2 — MR-009-Einschlägigkeit ist eine Verschärfung, ihre Begründung beschreibt Bestand.**
  MR-009 bindet an „**neue** Bauteil-/Solid-Geometrie im Kern oder Geometrie-Adapter"; die drei
  Impl-Slices verdrahten die UI an das bestehende `addWall` (Solid-Bau, Nachbar-Rebuild,
  Raum-Neuerkennung sind seit slice-003a/012 im Kern). Mehr Review ist nie ein Defekt — die
  Feststellung ist tragfähig, die Formulierung („`addWall` baut ein Solid") benennt aber
  Bestands-Code, nicht Zuwachs.
- **INFO-3 — Die 20-mm-Rechnung stimmt exakt.** `ViewTransform::zoom{0.05}` px/mm
  (`src/adapters/ui/view/view_transform.h`:21) ⇒ 1 px = 20 mm, gegen `GEOMETRY_TOLERANCE_MM` 0,1
  (`spec/spezifikation.md`:1185) — Faktor 200. Ergänzend: `ViewTransform::fit` überschreibt den
  Default, sobald der Plan Geometrie trägt; für ein 8×6-m-Demo in ~800 px ergibt sich dieselbe
  Größenordnung (~10–14 mm/px). Das Argument hält in beiden Fällen.
- **INFO-4 — Das Golden-Netz ist breiter als es diskriminiert.** Von den sechs Golden konsumieren nur
  **PDF und PNG** die `PlanView`; DXF iteriert `building.walls`/`.guide_lines` direkt
  ([ADR-0020](../plan/adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) MED-1), IFC/STEP/STL
  sehen sie nie. Die Zusage „bleiben die sechs Golden byte-identisch ⇒ kein Encoder sieht die
  Herkunft" bleibt korrekt (ein sehender Encoder würde rot), vier der sechs Zeugen können sie
  strukturell aber nicht brechen.
- **INFO-5 — Ein Folge-Punkt am Re-Eval „Wandtyp/Material interaktiv" (Entscheidung 13).**
  `spec/spezifikation.md` §2.2 führt als offenen Punkt „(a) `wall_types`-Bibliothek vs.
  `WallType`-Enum … Auflösung im WAL-007-Slice" (`:1170`). Fällt der Re-Eval künftig zugunsten
  „Wandtyp gehört zu parametrisch änderbar", wird dieser Punkt mit fällig; die Re-Eval-Zeile
  (`:204`) nennt nur die AK-Schärfung von `LH-FA-WAL-007`. Kein Befund für **diese** ADR
  (Entscheidung 13 hält Wandtyp draußen) — Hinweis für den späteren Schnitt.
- **INFO-6 — Dateiname.** Der Skill nennt `YYYY-MM-DD-slice-NNN[x]-{plan,code-review}.md`; für
  ADR-Text-Reviews lebt im Repo die Form `YYYY-MM-DD-adr-NNNN-text-review.md`
  ([0018](2026-07-05-adr-0018-text-review.md), [0020](2026-07-22-adr-0020-text-review.md),
  [0019-Neuschnitt](2026-07-23-adr-0019-recut-text-review.md)). Dieser Report folgt der Repo-Form
  (Auftrags-Vorgabe). Kandidat für die nächste Skill-Version.

---

## Negativbefunde (geprüft, ohne Befund)

Je Bereich eine Zeile — sichtbar gemachte Abdeckung, nicht Auslassung.

**ADR-Verträglichkeit (Hauptauftrag 2)**

- **[ADR-0019](../plan/adr/0019-drw-2d-canvas.md), alle sieben Entscheidungen** — geprüft, **kein
  Widerspruch**. E1 (eigenes 2D-`view/`-Widget) unberührt; **E2** (eine Quelle für Bildschirm und
  Export) wird von Entscheidung 11 **additiv erweitert**, nicht ersetzt („sie bleibt es"); E3
  (`screenToModel`) unberührt; **E4** (Selbst-Refresh) ist im Text **auf `addGuideLine` geschnitten**
  („nach einem erfolgreichen `addGuideLine` … repaintet er selbst", `0019:48`) — Entscheidung 6 regelt
  den **Wand**-Fall und benennt die Asymmetrie ausdrücklich, sie revidiert E4 nicht; **E5**
  (Kommando-Naht über `ui/command/`) wird angewandt (Detail s. LOW-3); **E6** (Aids = UI-Zustand,
  „AK schärfen eigene spätere Slices") wird von Entscheidung 9 genau auf dem dort vorgesehenen Weg
  fortgeschrieben; **E7** (`tests/e2e/` leer „**bis eine AK entsteht, die nur echt end-to-end prüfbar
  ist**") — die Bedingung ist **nicht** erfüllt (zwei Widget-Ebenen erreichen die Zusagen), nur das
  parenthetische Beispiel tritt ein; Entscheidung 14 behandelt das offen. **Abgrenzungs-/Re-Eval-Block:**
  ADR-0021 löst zwei der vier benannten Punkte ein und lässt Layer-Panel/Bemaßung ausdrücklich offen
  (Formulierungsdefekt s. LOW-2). Der Re-Eval-Wortlaut „`PlanViewPort` um Bauteil-/Treffer-Queries
  erweitern" ist ein Zeiger, keine Bindung; Entscheidung 11 erweitert die Lese-Naht am Werttyp statt
  am Methodensatz und begründet das. **⇒ kein `Supersedes` fällig.**
- **[ADR-0018](../plan/adr/0018-drw-2d-zeichen-daten.md)** — geprüft, kein Widerspruch. §2 „kein `op`"
  gilt ausdrücklich den **Zeichen-Daten** (`0018:43`); Bauteil-`op`s sind ADR-0008-Bestand.
  Konsequenzen-Zeile „MR-009 n/a (Hilfslinie = 2 Punkte, keine neue Solid-Geometrie)" (`0018:82`) —
  wörtlich korrekt zitiert; die Umkehrung für Wände ist eine Verschärfung (INFO-2). Re-Eval
  „Layer-Zuordnung für Bauteile → `entity_layers` aktivieren, Layer cross-cutting" (`0018:102`) —
  korrekt zitiert und ausdrücklich **nicht** ausgelöst (Konsequenzen `:174`); `model::Wall` trägt
  kein Ebenen-Feld (`src/hexagon/model/wall.h`:21–34).
- **[ADR-0009](../plan/adr/0009-gui-framework-qt6.md) (a)–(f), Fitness, Re-Eval** — geprüft, kein
  Widerspruch. (a) Qt Widgets: `QAction`/`menuBar` sind Bestand (`main_window.cpp:5,21-53`); (b)/(c)
  unberührt; (d) single-threaded, „Callback pullt nur und plant ein Repaint, löst nie Mutationen aus"
  — Entscheidung 6 hält das ein (`canvas_widget.cpp` `onModelChanged`: nur `invalidate`/`fit`/`update`);
  (e) Composition-Root-Injektion unberührt. **Re-Eval „Selektion/Picking im Viewport → AIS/V3d, als
  Supersedes-ADR"** meint den **3D-Viewport** (Alternative zu (b), OCC-Visualisierung); eine
  `QPainter`-Auswahl im 2D-Canvas löst ihn nicht aus — Entscheidung 3 hält die 3D-Sicht ausdrücklich
  unberührt und benennt den Trigger für den geteilten Fall. **⇒ kein `Supersedes` fällig.**
- **[ADR-0020](../plan/adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)** — geprüft,
  kein Widerspruch. Bündel-Prinzip (Kern rechnet, Adapter serialisiert) unberührt: die Herkunft
  entsteht kern-seitig in `projectPlan`, kein Adapter leitet ab. Der Re-Eval „das Bündel wird zu
  breit (viele optionale Felder je neuem Format)" (`0020:74`) ist korrekt zitiert; ADR-0021 führt ihn
  als **Beobachtungspflicht, kein Beschluss** — vertretbar, da das Feld in einem **verschachtelten**
  Typ (`PlanSegment`), nicht im Bündel selbst wächst.
- **[ADR-0008](../plan/adr/0008-aenderungs-benachrichtigung.md)** — geprüft, kein Widerspruch.
  #3 (alle committeten Mutationen melden; abgelehnte/verworfene nicht), #4 (Meldung **nach** allen
  Post-Commit-Schritten), #5 (`subscribe`/`unsubscribe`), #6 (Kapselung/Re-Entranz-Verbot) sind von
  Entscheidung 6 vorausgesetzt und im Code belegt (`structure_edit_service.cpp`:175–183; Canvas ist
  Beobachter und wird verdrahtet, `main.cpp`:580, `:666`).
- **[ADR-0006](../plan/adr/0006-relationales-schema-design.md)** — geprüft, kein Widerspruch.
  „Undo-Stack persistiert (`undo_commands`)" ist Entscheidung 4 der ADR (`0006:40-41`) und im Schema
  vorhanden (`spec/data-model.yaml`:285–298), unbedient; `entity_layers` ist vorhanden (`:260`) und
  bleibt schlafend. Entscheidung 12 zitiert beides korrekt.
- **[ADR-0001](../plan/adr/0001-hexagonale-architektur.md)** — geprüft, kein Widerspruch: Canvas =
  Driving Adapter, Kern framework-frei, Abhängigkeit nach innen; Entscheidung 7 hält die Richtung.

**Quellen-Konsistenz (Hauptauftrag 5)**

- **Lastenheft-Zitate** — geprüft, alle korrekt: `LH-FA-WAL-001` Happy („Linienzug mit ≥ 2 Punkten …
  je Segment eine Wand", `:148-150`), Boundary („verworfen, Hinweis", `:151-152`), Negative
  (`E-GEO-001`, `:153-154`); `LH-FA-WAL-002` („aktualisieren sich **sofort**"; „geklemmt + Hinweis",
  `:163-167`) und `LH-FA-WAL-003` (500–10000 mm, „analog", `:169-172`); `LH-FA-WAL-006` Teilumfang
  (`:182-205`); `LH-FA-WAL-007` = Outline ohne AK (`:207-210`); `LH-FA-WAL-004/005` = reine Outline
  (`:174-175`); `LH-FA-DRW-001` Teilumfang führt „Fangen beim Bauteil-Zeichnen" als offen (`:630`);
  `LH-QA-003` „mindestens 1000 Schritte" (`:1120-1124`); `LH-FA-UI-001`/`005` = Outline-Bullets
  (`:1024`, `:1028`); `LH-FA-D3-002` (`:583`); `OBJ-001` „Gebäude ohne tiefe CAD-Kenntnisse
  modellierbar" (`:40`). Die `LH-FA-DRW-005`-Teilumfang-Aussage ist korrekt zitiert, aber unvollständig
  adressiert (MED-3).
- **Spezifikations-Zitate** — geprüft, alle korrekt: §2.1 „*Wandzüge/Polylines* … folgen als
  Erweiterung" betrifft die Wand-**Entität** (`:1123-1125`) und bleibt von Entscheidung 2 unberührt;
  §3 `GEOMETRY_TOLERANCE_MM` = 0,1 (`:1185`); §4 `E-GEO-001` „Eingabe außerhalb des Zeichenbereichs"
  (`:1250`) und `E-VAL-001`-Ablehnungs-Lesart (`:1249`); **§6** — die Canvas-Vertragszeile trägt
  tatsächlich „Selbst-Refresh ohne `op`" (`:1290`) und wird mit Entscheidung 6 unvollständig, wie die
  Folgepflicht sagt. Zusätzlich geprüft: §1 `LH-FA-D3-002.a` (Melde-Reihenfolge), §1
  `LH-FA-DRW-005.a` (Canvas-Naht) und §1 `LH-FA-DRW-001.a` (Fang-Tie-Break) stützen die Entscheidungen
  3/6/9 wortgetreu.
- **Kontext-Behauptungen** — geprüft, korrekt: das Benutzerhandbuch führt „ein Gebäude **selbst
  planen**" unter „In dieser Version noch **NICHT** möglich" (`docs/user/benutzerhandbuch.md`:38–39).
- **Links/Anker der ADR** — 12 Ziele stichprobenartig von Hand gegen die Ziel-Überschriften geprüft
  (WAL-001/002/003/004/006/007, DRW-001/005, QA-003, D3-002, `#3-projektziele`,
  `#4-fehler-codes-und-logging-felder`) — alle auflösbar. **`make docs-check` nicht gelaufen**
  (Auftrag; kein Befund hängt daran).

**Code-Behauptungen (Hauptauftrag 4)**

- **`EditStructurePort`-Signaturen und Wurf-Verhalten** — geprüft, Entscheidung 10 ist **korrekt**:
  `addWall` wirft `std::out_of_range` bei unbekannter Geschoss-Id (`structure_edit_service.cpp`:147–149,
  **nach** der Null-Längen-Prüfung, die `nullopt` liefert), `setWallThickness`/`setWallHeight` werfen
  über `mutableWall` (`:189`, `:221`, `:500-506`). Der Kontrast stimmt ebenfalls: `addGuideLine` lehnt
  **wertbasiert** ab (drei `return std::nullopt`, `:1334-1342`), wirft nie.
- **`model::PlanSegment` / `projectPlan`** — geprüft, Entscheidung 11 ist **korrekt**: vier
  Koordinaten ohne Herkunft (`plan_view.h`:14–19); `projectPlan` mischt Wand-Achsen und sichtbare
  Hilfslinien in **eine** `plan.segments`-Liste je Geschoss (`plan_projection.cpp`:46–70).
- **`CanvasWidget`** — geprüft: ist `ModelChangedPort`-Beobachter (`canvas_widget.h`:53–54) und wird
  subscribed (`main.cpp`:580); der Zoom ist auf `[1e-4, 100]` px/mm **geklemmt**
  (`canvas_widget.cpp`:195–198); ein interaktives **Pan gibt es nicht** (kein Schreibzugriff auf
  `center_x_mm`/`center_y_mm` außerhalb von `ViewTransform::fit`). Entscheidung 8 ist damit tragfähig:
  jede Bildschirmposition bildet auf endliche mm ab, ein begrenzter Zeichenbereich existiert
  nirgends im Repo (einzige Fundstellen von „Zeichenbereich": `lastenheft.md`:153, `spezifikation.md`:1250).
- **`.a-check.yml`-Kanten** — geprüft, Entscheidung 7 ist **korrekt**: `{from: ui_command, to: ports_driving}`
  ist gelistet (`:32`), eine `ui_view → ports_driving`-Kante existiert **nicht**; `adapter_sink` ist
  **skalar** (`adapters/ui/view/mesh_source`, `:19`). Es wird nichts gelockert ⇒ `AGENTS.md` §2.6
  bleibt korrekt als n/a geführt.
- **`tests/e2e/`** — geprüft: enthält nur `.gitkeep`, ist leer. Entscheidung 14 beschreibt den
  Ist-Zustand korrekt.
- **Make-Targets der Fitness Function** — geprüft, **alle vier genannten existieren real**:
  `a-check` (`a-check.mk:11`), `test` (`Makefile:61`), `schema-check` (`:134`), `golden-check` (`:189`);
  die sechste Zeile bindet ehrlich auf ein „Review-Artefakt". Gate-Status s. LOW-4, Regel-Treue s. MED-2.

**Konventionen / Prozess**

- **MR-008 (Lösungsfreiheit dessen, was die ADR ins Lastenheft verweist)** — geprüft, **kein Befund**.
  Die vorgeschriebenen Klauseln benennen durchweg **Eigenschaften**, keine Mechanik: „in dieser
  Ausbaustufe ein Segment je Zeichen-Geste" (Entscheidung 2 sagt das ausdrücklich: „Der Klausel-Text
  nennt die Eigenschaft …, nicht den Slice"), „in dieser Ausbaustufe nicht erreichbar"
  (Entscheidung 8), je ein interaktiver Konjunkt zu WAL-002/003, Teilumfang-Nachzüge zu DRW-001/005.
  Ports, `ParamStatus`-Vokabular, `op`-Semantik, Toleranzen und Widget-Namen bleiben in ADR und
  `spezifikation.md` — die Trennung ist eingehalten.
- **MR-023 (Prozess-/Zeit-Reinheit der Spec-Straten)** — geprüft: die vorgeschriebenen Klauseln
  nennen die Ausbaustufe, nicht die Welle; die ADR selbst enthält **0** `slice-\d{3}`- und **0**
  `[Ww]elle-\d`-Tokens (grep über die ganze Datei inkl. `## Geschichte`) ⇒ MR-014-konform.
- **MR-010** — n/a für diese ADR (kein Lastenheft-Edit); der Header-Nachzug ist Sache des
  AK-Schärfungs-Slice und wird von dessen MR-006-Linse getragen.
- **MR-020 (Folgepflicht-Sichtbarkeit)** — geprüft: der [ADR-Index](../plan/adr/README.md) trägt fünf
  ADR-0021-Folgepflicht-Zeilen (`:76-80`), alle „offen", plus die Index-Kopfzeile (`:25`) ⇒
  `AGENTS.md` §4 („Neue ADRs müssen den ADR-Index aktualisieren") erfüllt.
- **`AGENTS.md` §2.5/§2.6** — geprüft: keine bestehende `Accepted`-ADR wird im Text editiert; keine
  Schwelle, keine Allow-Liste, keine Gate-Regel gelockert.
- **Alternativen-Fairness (Hauptauftrag 3)** — geprüft, **kein Strohmann**. Die sechs
  Alternativen-Blöcke (zu 1, 2, 3, 6, 11, 14) stellen je eine reale Gegenposition mit tragendem
  Contra dar; die schwächste („getrennte Zeichenflächen je Bauteil-Art") ist eine in CAD tatsächlich
  vorkommende Bauform mit korrektem Contra. Die **Vollständigkeit** des Blocks ist ein eigener
  Befund (LOW-5), seine **Fairness** ist ohne Befund.
- **Abweichungen vom Slice-Plan (Einordnung, nicht Maßstab)** — geprüft: die vierzehn
  Entscheidungen der ADR mappen 1:1 auf F1…F14 des Plans; die substanziellen Abweichungen sind
  **begründet** (Entscheidung 1 ergänzt den Auswahl-Modus, den Entscheidung 3 braucht; Entscheidung 13
  ergänzt die Wertebereiche aus dem Port; die AK-Schärfung wandert in den Folgepflicht-Block, wie es
  MR-014 erzwingt — ein ADR-Körper darf keine Slice nennen). **Unbegründet** bleiben zwei: die
  ausgelassenen Alternativen-Blöcke gegen die Plan-DoD „je entschieden **mit verglichenen
  Alternativen**" (LOW-5) und das Fallenlassen der `E-GEO-002`-Wurf-Quelle aus zwei Plan-Reports
  (MED-6).
- **„Ist jede der vierzehn entschieden?" (Hauptauftrag 3)** — geprüft: **keine** Entscheidung
  verschiebt ihre Frage in einen Impl-Slice. Entscheidung 7 erklärt sich selbst zur Feststellung
  (INFO-1). Delegiert wird ausschließlich, was der „Nicht offen"-Block ausdrücklich delegiert
  (Klassennamen, Toleranz-**Werte**, Panel-Layout) — die drei HIGH-Befunde betreffen **nicht**
  delegierte Semantik-Fragen, die schlicht fehlen.

---

## Kategorie-Summary

| Kategorie | Anzahl |
|---|---|
| **HIGH** | **3** |
| MEDIUM | 7 |
| LOW | 5 |
| INFO | 6 |

**Fehlt eine Entscheidung? — JA, drei.** HIGH-1 (Lese-Naht für die anzuzeigenden Wand-Parameter),
HIGH-2 (Geschoss-Skopus der Treffer-Prüfung), HIGH-3 (Lebensdauer der Auswahl über `ModelReplaced`).
**Suchbreite:** je Entscheidung wurde gefragt, welche Eingaben, Identitäten, Skopen, Lebensdauern und
Fehlerausgänge der beschriebene Schnitt erzwingt; jeder Kandidat wurde am Artefakt geprüft
(Port-Inventar `src/hexagon/ports/driving/`, `model::PlanView`/`Wall`, `snap.cpp`, `canvas_widget.cpp`,
`structure_edit_service.cpp`, `main.cpp`, `.a-check.yml`, `data-model.yaml`) und nur geführt, wenn er
weder in `## Entscheidung` noch im „Nicht offen"-Block vorkommt und nicht von der dort erklärten
Delegation (Klassennamen/Konstanten/Panel-Layout) gedeckt ist. **Ausgeschieden** sind u. a.:
Ziel-Geschoss des `addWall`-Aufrufs (von Entscheidung 10s Prämisse „injizierte Ids" mitgetragen und
im Bestand als `DrawingTargetSinks` gelöst), Ebene der gezeichneten Wand (Konsequenzen, „Bauteil-Ebenen
bleiben unberührt"), Default-Stärke/-Höhe und Wandtyp der neuen Wand (Port-Bestand), Modus-Wechsel
während einer Geste (Entscheidung 12 Gesten-Abbruch), Aussehen von Auswahl und Hinweis (ausdrücklich
keine Zusage, Entscheidung 14), Panel-Verhalten ohne Auswahl (delegiertes Layout).
**Die Begründung des Stopps bei vierzehn hält damit nicht ganz:** ausformulierte Entscheidungen zeigen
Lücken tatsächlich besser als eine Tabelle — aber drei Lücken sind so entstanden, dass die ADR die
Frage in einer *anderen* Entscheidung beantwortet zu haben scheint (11 → „Id genügt"; 9 → „wie der
Fang"; 10 → „die Senke fängt"), obwohl sie es je nur für den halben Fall tut. Genau dort, wo die
Ausformulierung **fehlt** — acht Entscheidungen ohne Alternativen-Block (LOW-5) —, sitzen zwei der
sieben MEDIUM (MED-5 zu 9, MED-6 zu 10): der Verdachtsindex des eigenen Verfahrens zeigt in die
richtige Richtung, es wurde nur nicht überall angewandt.

**Ist `Supersedes` fällig? — NEIN.** Gegen ADR-0019 (alle sieben Entscheidungen + Abgrenzungs-/Re-Eval-Block),
ADR-0018 (§2, Konsequenzen, Re-Eval), ADR-0009 ((a)–(f), Fitness, Re-Eval inkl. Selektions-/e2e-Trigger),
ADR-0020 (Bündel-Prinzip, Re-Eval), ADR-0008, ADR-0006 und ADR-0001 wurde **kein** echter Widerspruch
gefunden. Die zwei Stellen, an denen einer denkbar wäre, sind sauber aufgelöst: ADR-0019 E4
(Selbst-Refresh) ist im Quelltext auf `addGuideLine` geschnitten — Entscheidung 6 regelt einen neuen
Fall und benennt die Asymmetrie; ADR-0009 (f)/ADR-0019 E7 knüpfen `tests/e2e/` an die Bedingung
„nur echt end-to-end prüfbar", die Entscheidung 14 begründet verneint (die Zuschreibungs-Ungenauigkeit
dabei ist LOW-1). Die ADR schreibt ADR-0019 fort; die Selbstauskunft „kein `Supersedes`, es gibt
keinen Widerspruch, nur Erweiterung" ist zutreffend.

---

## Verdikt

**Nicht accept-fähig — 3 HIGH blockieren.** Der Schnitt selbst ist tragfähig: die Verträglichkeit mit
allen sieben berührten Accepted-ADRs hält, die Zitate stimmen bis auf zwei redaktionelle Stellen, die
Code-Behauptungen sind am Artefakt durchweg korrekt, die Fitness-Function-Targets existieren real, und
was die ADR ins Lastenheft verweist, ist lösungsfrei. Blockierend ist ausschließlich, **was fehlt**:
drei Entscheidungen, die der beschriebene Lösungs-Schnitt erzwingt und die nach `Accepted` nicht mehr
ergänzbar wären (`AGENTS.md` §2.5) — die Lese-Naht für die Werte, die Entscheidung 4 anzuzeigen
verspricht (HIGH-1); der Geschoss-Skopus der Treffer-Prüfung, den Entscheidung 3 von einer beweisbar
geschoss-übergreifenden Regel erbt (HIGH-2); und die Lebensdauer der Auswahl über den Modell-Austausch,
in dem die Fehler-Barriere aus Entscheidung 10 strukturell nicht greift (HIGH-3).

Nach Einarbeitung der drei HIGH und einer Entscheidung über MED-1 bis MED-7 ist die ADR
**accept-fähig**; der Projektinhaber-Accept bleibt davon unberührt.

**Anmerkung zur Einordnung:** der Slice-Plan
`docs/plan/planning/in-progress/slice-056-wand-im-canvas-adr-ak.md` und die vier Plan-Reports wurden
gelesen, um zu prüfen, ob die ADR **unbegründet** vom Plan abweicht (siehe Negativbefunde). Sie waren
**nicht** der Maßstab: HIGH-1 bis HIGH-3 und MED-1 bis MED-5 stehen in keinem der vier Reports und
sind ausschließlich am Artefakt entstanden. MED-6 und MED-7 sind Wiedergänger — sie standen als
INFO-3/MEDIUM-6 bzw. LOW-4 in den Reports 2 und 3 und haben den ADR-Text nicht erreicht.
