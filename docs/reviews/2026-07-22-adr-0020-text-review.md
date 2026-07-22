# ADR-0020 Text-Review — Driven-Adapter serialisieren, Kern liefert Geometrie

**Datum:** 2026-07-22 · **Reviewer:** unabhängiger Agent (≠ Autor), read-only ·
**Gegenstand:** `docs/plan/adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md` (Status Proposed).

**Verdikt (Erst-Lauf): 1 HIGH / 2 MED / 3 LOW / 3 INFO → blockiert.** Nach Einarbeitung (unten):
**0 HIGH offen → Accept-fähig** (Projektinhaber-Accept ausstehend). Die Entscheidungsrichtung ist solide;
alle Kern-Fakten sind gegen den Code verifiziert (INFO-3). Ein tragender Satz war durch **einen** Rest-Aufruf
falsifiziert.

## HIGH-1 (behoben) — `translateMeshZ` kann NICHT in den Kern; die `geometry → services_geo`-Kante überlebte sonst

**Befund:** Der STL-Adapter tesselliert die Slab-Mesh **adapter-seitig** (`GeometryKernelPort.tessellateFootprint`,
z∈[0,thickness], **kein** baseZ-Param) und verschiebt sie **dann** um `baseZ` via `services::translateMeshZ`
(`stl_export_adapter.cpp:74`, Decl `slab_geometry.h:46`). Der Kern hält **kein** Mesh (nur pre-OCC-Primitive +
den Skalar `baseZ`) → `translateMeshZ` ist die **eine** `services::`-Funktion, die **nicht** »in die Kern-
Berechnung wandern« kann. Der ADR sah nur `StepBox → model/` als Typ-Umzug vor und übersah diese Funktion.
Literal umgesetzt bliebe der `slab_geometry.h`-Include im STL-Adapter → `a-check` bräuchte weiter die Kante →
das eigene Fitness-Gate des Refactors scheiterte. (STEP hat kein Analog: es liftet das **Solid** via OCC-`gp_Trsf`.)
**Fix (eingearbeitet):** `translateMeshZ` **nach `model/`** gehoben (pure `TriangleMesh`-z-Verschiebung, adapter-
erreichbar, kein `services/geometry`) — der STL-Adapter wendet sie auf sein eigenes Mesh mit dem gelieferten
Skalar an. Neben `StepBox` als **zweiter** reiner Wert-Umzug benannt; die »nur Aufruf-Ort verschieben«-Prämisse
entsprechend präzisiert.

## MED-1 (behoben) — DXF nutzt `visibleLayerIds`, NICHT `PlanView`

**Befund:** §Kontext #3 behauptete »DXF/PDF/PNG nutzen `PlanView` intra-Adapter«. Falsch: `dxf_export_adapter.cpp`
inkludiert `plan_geometry.h` **nur für `visibleLayerIds`** (`:26`) und iteriert `building.walls`/`.guide_lines`
**direkt** (`:69-100`) — **nur PDF/PNG** rufen `projectPlan`. DXF aufs `PlanView`-Bündel zu zwingen würde die
`ENTITIES`-Reihenfolge (alle Wände global, dann Hilfslinien) durch die per-Geschoss-`PlanView`-Gruppierung
umsortieren → **DXF-Round-Trip-Orakel bräche** (gegen die Byte-Invarianz-Zusage). **Fix (eingearbeitet):** Prämisse
korrigiert; DXFs Direkt-Iteration bleibt **vom Bündel unberührt**, `visibleLayerIds` (Nicht-Geometrie) bleibt io-lokal.

## MED-2 (behoben) — Format-selektiver Mechanismus unspezifiziert; Pull-Provider-Alternative fehlte

**Fix (eingearbeitet):** Der `ExchangeService` berechnet die Ableitung **per-Format** — konsistent mit seiner
bestehenden `ExchangeFormat`-Dispatch-Rolle; IFC (leitet nichts ab) bekommt ein **leeres** Bündel. Die **Pull-
Provider**-Alternative (Exporter zieht selbst) ist ergänzt + **verworfen** (würde Adapter wieder Ableitungen
auslösen lassen — gegen »Adapter serialisieren nur«).

## LOW (behoben)

- **LOW-1 — Persistenz:** `model::Stair` hat **kein** `rise`-Feld (bewusst abgeleitet) → »vor `save` setzen« hat
  keinen Träger; die **save-Signatur-Form** (gelieferte Skalare) als **bevorzugt** benannt.
- **LOW-2 — §1-Diagramm:** die substantielle Unwahrheit ist die **§2-Tabelle** (sie gewährt die Kante); das §1-
  Diagramm ist zu grob, um die Kante zu zeigen. §3 führt jetzt mit der Tabelle.
- **LOW-3 — Big-Bang:** der `write`-Vertrag berührt **alle sechs** Exporter-Signaturen auf einmal (accept-and-
  ignore, Body-Migration je Slice) — in Konsequenzen (a) + Contra benannt.

## INFO (bestätigt)

- **INFO-1 — IFC** ist reiner `model→SPF`-Serialisierer (kein `services/geometry`) → leeres Bündel, ignoriert. Ok.
- **INFO-2 — MR-014 ✓** (null `slice-\d{3}` im Body) + **§2.6 n/a ✓** (Kanten-Entfernung = Allow-Liste-Verengung =
  Verschärfung; die `write()`-Signatur ist ein Code-Vertrag, keine Gate-Regel).
- **INFO-3 — Kern-Fakten verifiziert:** alle `services/geometry`-Rückgaben sind pure `model`-Werte (kein `TopoDS`);
  `StepBox` im `services`-Namespace (Umzug nötig); `occ_geometry_adapter` inkludiert **kein** `services/geometry`;
  `occ_solids.h`-Builder nehmen nur pure `model`-Typen; die Kanten `geometry`/`persistence → services_geo` existieren,
  `io → services_geo` nicht; `stairRiseMm` write-only; die Wand/Decken-Bündel-Repräsentation (extrudierbare Eingaben)
  dient **beiden** STEP + STL, die Treppen-Divergenz (StepBox vs. Mesh) trägt das Bündel durch beide Felder.

---

**Einarbeitung (Autor, 2026-07-22):** HIGH-1 (`translateMeshZ → model/`), MED-1 (DXF nutzt nur `visibleLayerIds`;
Direkt-Iteration bündelfrei), MED-2 (`ExchangeService`-per-Format + Pull-Provider verworfen), LOW-1/2/3 — alle in
ADR-0020 verankert. **0 HIGH offen → Accept-fähig; Projektinhaber-Accept ausstehend.**
