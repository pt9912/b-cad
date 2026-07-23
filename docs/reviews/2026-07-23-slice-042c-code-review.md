# slice-042c Code-Review ([MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure))

**Datum:** 2026-07-23 · **Reviewer:** unabhängiger, **adversarialer** Agent (≠ Autor), read-only ·
**Gegenstand:** der 042c-Code-Diff `818f275..HEAD` über `src/`/`tests/`/`.a-check.yml`/`spec/architecture.md`
(STEP/STL-Body-Migration auf das `DerivedGeometry`-Bündel + `ExchangeService`-Berechnung + `storeyHeight`-
Konsolidierung + `geometry → services_geo`-Kante raus). Geometrieschwerster Refactor der
[ADR-0020](../plan/adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Familie.

**Verdikt: 0 HIGH / 0 MED / 0 LOW / 2 INFO → frei.** slice-042c ist **verhaltens-invariant und
architektur-sauber**; der Kern-Anspruch (byte-identisches STEP/STL) hielt der adversarialen Prüfung stand.

## Verifiziert (Widerlegungs-Versuche gescheitert)

- **A — Byte-Identität (Alt 818f275 vs. Neu, Feld-für-Feld gegen `derived_geometry.h`):** Aggregat-Init-
  Reihenfolge (`DerivedWall{footprint, height_mm, cutPrisms}` etc.) trifft die Service-Befüllung exakt. STEP:
  `makeNetSolid(w.footprint, w.height_mm, w.cutPrisms)` = alt; Slab-Lift `s.baseZ_mm` = alt `slabBaseZ(s,
  storeyHeight(...))`; Stair-Box-Koordinaten-Reihenfolge unverändert. STL: `tessellateFootprint`/`translateMeshZ`/
  `r.mesh`/`s.mesh` identisch (dieselben Kern-Funktionen, nur verschobener Aufrufort). **Richtiges Storey-Feld je
  Bauteil** (Slab `s.storey_id`, Stair `s.from_storey_id` — nicht vertauscht). Ein-Eintrag-je-Bauteil in
  Modell-Reihenfolge, kein Vor-Filtern.
- **B — Skip-Grenze:** der fail-closed `try/catch{continue}` bleibt je Bauteil im Adapter um die OCC-Operationen.
  Die in den Service gewanderten Ableitungen sind **total** (grep über alle fünf `services/geometry/*.cpp`: kein
  `throw`/`assert`/`.at()`/`front()`/`back()`) → kein Wurf entsteht im Service, wo früher übersprungen wurde. Der
  `ExportTotality`-Test (degenerierte Null-Wand) bestätigt es.
- **C — a-check:** `geometry → services_geo` entfernt; beide Adapter ohne `services/geometry`-Includes/-Aufrufe
  (nur `occ_solids` intra-layer + `derived_geometry`/`mesh_ops` model). `architecture.md` §2-geometry-Zeile
  konsistent; `persistence → services_geo` bleibt.
- **D — `storeyHeight`:** die Service-Kopie ist zeichengleich zu den zwei entfernten Adapter-Kopien (gleiche
  Schleife + `kDefaultStoreyHeightMm`-Fallback); kein Adapter braucht sie noch.
- **E — Test-Netz trifft die Service-Berechnung:** `writeStep`/`writeStl` fahren über den echten `ExchangeService`;
  `FullModelYieldsAllComponentBRepSolids` erwartet korrekt `wallShells + 1 [Decke] + 1 [Dach] + step_count [Treppe]`
  (Baseline + Voll-Modell teilen dieselben zwei Wände); `stlMaxZ`-Sonde korrekt (Wände <2600, Decke baseZ 2500 +
  Dicke 200 → >2600); Totalitäts-Test deckt **beide** Pfade (Fallback + OCC-Skip) für STEP und STL. Kein
  ungetesteter Regressionspfad (Slab-`cutouts` + from_storey/storey-Trennung korrekt verdrahtet).
- **F — Sonstiges:** `building`-Param folgenlos ungenutzt; `main.cpp` unverändert.

## INFO (nicht blockierend, akzeptiert)

1. `appendRoofMeshes`/`appendStairMeshes` **kopieren** `r.mesh`/`s.mesh` (statt move) — Output bit-identisch, ein
   `std::move` ist unmöglich (`derived` kommt als `const&`); reine Mikro-Perf.
2. Die Service-Ableitung läuft für **alle** Bauteile (auch später übersprungene degenerierte) — der intendierte
   ADR-0020-Entwurf (Kern total, Adapter fail-closed), kein Defekt.

---

**Einarbeitung (Autor, 2026-07-23):** keine Findings zu beheben (0 HIGH/MED/LOW; 2 INFO akzeptiert). **MR-009 für
slice-042c erfüllt (0 HIGH) — vor Welle-Closure.**
