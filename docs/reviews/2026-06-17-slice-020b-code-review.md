# Code-Review + MR-009 — slice-020b (STEP/STL-Export)

## Kopf

- **Review-Art:** Unabhängiges Code-Review (Reviewer ≠ Autor) **+ MR-009** (geometrie-lastiges Code-Review — geometrische Korrektheit der neuen/geänderten Geometrie-Adapter-Pfade) vor Welle-/Slice-Closure.
- **Gegenstand:** Diff `d6c7774..31e7555` (Commit i STL + Commit ii STEP): neue Geometrie-Exporter (`stl_export_adapter`, `step_export_adapter`), geteiltes `occ_solids`, getrimmter `occ_geometry_adapter`, `ExchangeService`-Registry, `main`-Verdrahtung, AK-Tests, §1-Präzisierung.
- **Datum:** 2026-06-17. **Gate-Stand:** docs-check 121/0, arch-check ok, lint 0, 180/180 Tests, Coverage 90,0 %.

**Verdikt: 0 HIGH / 2 MED / 3 LOW / 3 INFO.** Keine Korrektheits-/Architektur-Defekte; die AK (`LH-FA-IO-005`/`006`, `E-IO-001`, Totalität, benannte STEP-Lücke) sind erfüllt.

## MR-009 — Geometrie-Korrektheit (verifiziert)

- **`occ_solids`-Extraktion verhaltens-erhaltend:** `polygonArea`/`isDegenerate`/`makeFootprintSolid`/`makeCutterSolid`/`makeNetSolid` logisch byte-identisch zur Vorlage (`git show d6c7774:…occ_geometry_adapter.cpp`); Toleranzen + Fuzzy-Wert + Kontrollfluss unverändert; nur Fehlermeldungs-Präfix kosmetisch (`OccGeometryAdapter:`→`OccSolids:`), E-GEO-002 erhalten. extrude/tessellate liefern identische Solids.
- **STL-Netz-Assemblierung spiegelt den Viewer exakt:** Wände `tessellateFootprint(wallFootprint, height, wallCutPrisms)` bei z=0; Decken zusätzlich `translateMeshZ(slabBaseZ)` — wie `StructureEditService.wall/slabMeshes`. Flat-Shading-Normale je Dreieck aus Vertex i0 korrekt (TriangleMesh-Konvention); Wicklung erhalten. Binär-STL: 80-Byte-Header (nicht „solid"), Zähler == Σ`triangleCount()`, 50 Byte/Dreieck, little-endian (amd64). Kein Index-OOB (Netze stammen aus interner Tessellation).
- **STEP-B-Rep korrekt:** Decken-Hub `gp_Trsf(0,0,slabBaseZ)` + `BRepBuilderAPI_Transform(…, copy=true)` = geometrisches Äquivalent zu `translateMeshZ`; Compound via `BRep_Builder`. STEP-Lücke (Dächer/Treppen) konsistent zur §1-Präzisierung + ehrlich benannt (sie gehen ins STL).
- **Totalität:** leeres Modell → STL 84-Byte-Datei / STEP gültige Hülle ohne Wurf; degenerierte Bauteile per `continue`-Skip übersprungen. **Wichtig:** OCC 7.9.2 `Standard_Failure : std::exception` → die per-Bauteil-`catch (std::exception&)` fangen auch rohe OCC-Fehler ab und überspringen das Bauteil (kein Gesamt-Abbruch).

## Findings

### MED-1 — STEP-Temp vor dem Rename nicht fsync't → **behoben**
`step_export_adapter.cpp` nutzte `STEPControl_Writer.Write(tmp)` + `fs::rename` **ohne** fsync (anders als STL/IFC). Crash zwischen Write und Flush könnte ein leeres/abgeschnittenes Ziel sichtbar machen. **Fix eingearbeitet:** Temp nach erfolgreichem Write `::open`+`::fsync`+`::close` vor dem Rename (Muster STL/IFC). MED, nicht HIGH: `E-IO-001` betrifft Schreibrecht/atomares Ersetzen (Rename ist atomar — ein *gescheiterter* Export ersetzt das Ziel nie); nur Crash-mid-write war das Restrisiko.

### MED-2 — (bei Prüfung entfallen) `appendU32LE`-Shift
`(value >> 24) & 0xFFU` auf `uint32_t` ist wohldefiniert — kein Defekt.

### LOW-1 — `atomicWrite`/`ioCodeForErrno` im STL-Adapter dupliziert (vom IFC-Adapter)
Bewusst, da Regel B Code-Teilung über Adapter-Grenzen verbietet (io/ vs. geometry/); je Adapter im Anon-Namespace lokalisiert. Akzeptiert.

### LOW-2 — stale `tests/CMakeLists.txt`-Kommentar (nur „STL") → **behoben** (nennt jetzt STEP+STL).

### LOW-3 — STEP-Test-Orakel `"BREP"` ist ein loses Substring-Orakel
Adäquat für „ein B-Rep-Solid wurde geschrieben" unter der Kein-OCC-im-Test-Vorgabe; verifiziert keine Anzahl. Belassen (AK-Smoke-Test); eine engere Variante (`MANIFOLD_SOLID_BREP`/`ADVANCED_FACE`-Zählung) ist optional.

### INFO
- **INFO-1:** arch-check Regel C scannt **nur `src`**, nicht `tests/` — der Text-Orakel-Ansatz (kein OCC im Test) ist defensiv/stilistisch, nicht gate-erzwungen; trotzdem gute Wahl. `occ_solids.h` (OCC-Header) liegt korrekt unter `src/adapters/geometry/` → Regel C grün.
- **INFO-2:** Registry-Dispatch korrekt (Map-Miss → `E-IO-001`; `importModel`-`switch` erschöpfend über `{Ifc,Step,Stl}`, `-Wswitch` erfüllt; Step/Stl-Import → `E-IO-003` export-only).
- **INFO-3:** Kein OCC-Typ-Leck (STEP-`catch (Standard_Failure&)` → neutraler Wurf; Kern OCC-/STEP-symbolfrei, Regel A grün).

## Ergebnis
**Keine HIGH-Findings.** MED-1 (STEP-fsync) + LOW-2 (Kommentar) vor der Closure eingearbeitet; LOW-1/LOW-3/INFO sind bewusst/kosmetisch. Geometrie-Semantik deckt sich mit dem Viewer, Atomarität ist konsistent, die STEP-Lücke (Dächer/Treppen) ist ehrlich benannt. slice-020b ist closure-reif.
