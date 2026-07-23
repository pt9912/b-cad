# slice-042a + slice-042b Code-Review ([MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure))

**Datum:** 2026-07-23 · **Reviewer:** unabhängiger, **adversarialer** Agent (≠ Autor), read-only ·
**Gegenstand:** der kombinierte Code-Diff `e23e78f..HEAD` über `src/`/`tests/`/`.d-check.yml` — die zwei
verhaltens-invarianten ADR-0020-Refactor-Slices **042a (Kern-Naht)** + **042b (2D-Projektion)**. Frühzeitig
gezogen (vor 042c, das auf der Naht aufbaut), erfüllt [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
für diese zwei Slices vor der Welle-Closure.

**Verdikt: 0 HIGH / 0 MED / 1 LOW / 1 INFO → frei (blockiert die Welle-Closure nicht).** Beide Slices sind
**verhaltens-invariant** und **architektur-sauber**; der Kern-Anspruch hielt der adversarialen Prüfung stand.

## Verifiziert (Widerlegungs-Versuche gescheitert)

- **A — Byte-Identität.** Alt `e23e78f:src/adapters/io/plan_geometry.cpp` vs. neu
  `src/hexagon/services/geometry/plan_projection.cpp` **Zeile für Zeile identisch** (`accumulate`-Box-Init/-
  Erweiterung, Storey-Schleife, **Wände vor Hilfslinien**, `storey_id`-/`visible.contains`-Filter,
  Segment-Konstruktion); `visibleLayerIds`-Rumpf byte-gleich. PDF/PNG-Naht identisch: der `ExchangeService`
  rechnet `derived.plan = projectPlan(building)` mit **demselben** `building`, das an `write` geht; Adapter
  liest `const PlanView& view = derived.plan;`. `translateMeshZ`/`StepBox`-Umzug identisch; die Default-Init
  `double x0_mm;` → `x0_mm{}` ist **strikt sicherer** (kein Pfad las je uninitialisiert). Kein Falschpfad mit
  leerem Bündel: Produktionsweg nur `main.cpp` → `exchange.exportModel` → befüllt `Pdf`/`Png`; alle
  Test-`write`-Aufrufe an PDF/PNG laufen über die `writePdf`/`writePng`-Helfer (befülltes Bündel aus derselben
  `projectPlan`-Quelle). `building` in PDF/PNG folgenlos ungenutzt.
- **B — a-check.** Kein io-Adapter importiert `services/geometry` (pdf/png ziehen `model/plan_view.h`, dxf
  `model/layer_visibility.h` → erlaubte `io → model`-Kante); **keine `io → services_geo`-Kante**. Kern-Bündel
  lib-frei (Link-Barriere intakt). `geometry`/`persistence → services_geo` korrekt noch vorhanden (042c/d).
- **C — Tombstone legitim.** `plan_geometry` real entfernt; `grep` über `src/`/`tests/` → nur Prosa-Kommentare,
  **keine** funktionale Code-Referenz. Keine Schwellen-Lockerung.
- **D — Test-Netz fängt Regressionen.** `test_plan_projection` (Sichtbarkeit + BBox-Erweiterung + Swap-Fang);
  die Format-Selektion IST getestet (`PdfExport.StructureObjectGraphAndPerStoreyContent` +
  `PngExport.FullDecodeStructureAndInk` über den echten `ExchangeService`-Pfad — eine falsche `Pdf||Png`-
  Bedingung würde auffliegen); `PlanViewPortDelegiertAnKernProjektion` nicht tautologisch.
- **E — Naht vollständig.** `StructureEditService` (6 Driving-Ports inkl. `PlanViewPort`); `main.cpp` unverändert
  korrekt; Format-Selektion vollständig; `write`-Vertrag über 6 Exporter konsistent.
- **F — keine Aliasing-/Lebensdauer-/Float-Falle** (`const PlanView& view = derived.plan;` — `derived` lebt über
  den `write`-Aufruf).

## LOW-1 (behoben) — veraltete `plan_geometry`-Kommentare in PDF/PNG

`pdf_export_adapter.h`, `png_export_adapter.{h,cpp}` behaupteten „Reuse `plan_geometry`" / „lebt ausschließlich
hier + in `pdf_writer`/`plan_geometry`" — nach der Hebung existiert kein io-`plan_geometry`; die `PlanView`
kommt aus dem Bündel. Rein dokumentarisch. **Fix (eingearbeitet):** Kommentare auf „PlanView aus dem
`DerivedGeometry`-Bündel (ADR-0019/0020)" nachgezogen.

## INFO-1 (an 042c weitergegeben) — kein Negativ-Test auf leeres Bündel für STEP/STL/IFC/DXF

Kein Test verifiziert explizit, dass der `ExchangeService` diesen Formaten ein **leeres** `derived` reicht. In
042a/b verhaltens-irrelevant (sie ignorieren `derived`); relevant erst ab **042c**, wenn STEP/STL das Bündel
konsumieren. Als Netz-Vormerkung ins 042c-Skelett aufgenommen.

---

**Einarbeitung (Autor, 2026-07-23):** LOW-1 (Kommentar-Nachzug) behoben; INFO-1 als Test-Vormerkung ins
042c-Skelett. **0 HIGH → [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
für 042a+042b erfüllt (vor Welle-Closure).**
