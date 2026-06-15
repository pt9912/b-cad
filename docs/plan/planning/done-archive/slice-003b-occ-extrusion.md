---
id: slice-003b
titel: OCC-Extrusion (GeometryKernelPort-Adapter) + arch-check Regel C
status: done
welle: welle-1-mvp
lastenheft_refs: [LH-FA-D3-001]
adr_refs: [ADR-0001, ADR-0002]
---

# Slice 003b: OCC-Extrusion (GeometryKernelPort-Adapter) + arch-check Regel C

**Status:** done

**Welle:** welle-1-mvp

**Bezug:** LH-FA-D3-001, ADR-0001, ADR-0002

**Autor:** Dietmar Burkard. **Datum:** 2026-06-09.

**Schnitt-Herkunft:** OCC-Teil des ursprünglichen `slice-003` (siehe
`slice-003a`). Isoliert, weil build-schwer (OCC) und risikobehaftet
(Geometrie-Determinismus) — getrennt review-bar vom Kern.

---

## 1. Ziel

Den `GeometryKernelPort` (Vertrag aus 003a) real über OpenCascade
erfüllen: Wand-Footprint zu einem Solid extrudieren (LH-FA-D3-001), das
Volumen messen und gegen den analytischen Wert (`länge · stärke · höhe`)
absichern. Zusätzlich die in **ADR-0002 §Fitness Function** als
Folgepflicht benannte **arch-check Regel C** ergänzen: OCC-Header
(`*.hxx`) nur in `src/adapters/geometry/`.

## 2. Definition of Done

- [x] `src/adapters/geometry/occ_geometry_adapter.{h,cpp}` implementiert
      `GeometryKernelPort::extrudeWall` via OCC (BRep-Prisma + Volumen
      über `GProp`/`BRepGProp`); kein OCC-Typ verlässt den Adapter.
- [x] `tools/arch-check.sh` **Regel C**: kein `#include <*.hxx>` außerhalb
      `src/adapters/geometry/` (schließt die Sensor-Lücke aus ADR-0002
      Finding 2). `harness/README.md`/`AGENTS.md` §arch-check-Vertrag um
      Regel C + Bindung ADR-0002 erweitern.
- [x] Adapter-Test (mit OCC, LH-FA-D3-001): Volumen ≈ analytisch mit
      **relativer** Toleranz (`expected · 1e-6`) — *nicht*
      `GEOMETRY_TOLERANCE_MM` (das ist eine Längen-/Punkt-Toleranz in mm,
      keine Volumen-Toleranz in mm³, Finding 1); Determinismus (zwei
      Extrusionen bit-identisch).
- [x] **E-GEO-002 abgesichert** (`spec/spezifikation.md` §4): der reale
      Adapter wirft bei degeneriertem Segment eine **neutrale**
      `std::runtime_error` (kein OCC-Typ leakt); ein Integrationstest
      `StructureEditService` + realer `OccGeometryAdapter` belegt die volle
      Kette end-to-end. Der *transaktionale Rebuild* selbst (Modell
      unverändert bei Wurf) ist bereits in **slice-003a** implementiert und
      mit Port-Double getestet (`AddWall/SetThickness…BeiGeometrieFehler`) —
      hier nur der reale Fehlerpfad + Integration.
- [x] Provokation: ein OCC-`.hxx`-Include in `src/adapters/ui/` lässt
      `make arch-check` rot werden (Regel-C-Nachweis), danach zurückgebaut.
- [x] **Folgepflicht kanonisch dokumentiert ohne ADR-0002 zu ändern**
      (Finding 3): ADR-0002 ist Accepted/immutable (AGENTS §2.5). Die
      Erfüllung der Regel-C-Folgepflicht wird im **ADR-Index**
      (`docs/plan/adr/README.md`, mutabel) und in der Closure-Notiz
      vermerkt — keine Supersedes-ADR nötig (die Entscheidung steht
      unverändert; eine benannte Folge-Aufgabe wird nur abgehakt).
- [x] `make gates` grün; Closure-Notiz schließt die ADR-0002-Folgepflicht.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/geometry/occ_geometry_adapter.{h,cpp}` | neu | OCC-Extrusion (ADR-0002) |
| `src/adapters/CMakeLists.txt` | ändern | neue `.cpp` in `bcad_adapters` (OCC-Modeling-Libs) |
| `tools/arch-check.sh` | ändern | Regel C (OCC-`.hxx` nur in geometry/) |
| `harness/README.md`, `AGENTS.md` | ändern | arch-check-Vertrag um Regel C + ADR-0002-Bindung |
| `docs/plan/adr/README.md` | ändern | ADR-0002-Folgepflicht (Regel C) als erfüllt vermerken (Finding 3, ohne ADR-Body zu ändern) |
| `tests/adapters/test_occ_geometry_adapter.cpp` | neu | Volumen (relative Toleranz) + Determinismus + E-GEO-002-Wurf + Integration mit `StructureEditService` (LH-FA-D3-001) |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |

## 4. Trigger

- slice-003a done (Port-Vertrag + Modell stehen).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz geschrieben
  (ADR-0002-Folgepflicht Regel C als geschlossen vermerkt).

## 6. Risiken und offene Punkte

- Geometrie-Determinismus über OCC: **relative** Volumen-Toleranz
  (`expected · 1e-6`) im Test, nicht `GEOMETRY_TOLERANCE_MM` (Längen-Maß).
  OCC kann je nach Version minimal abweichende Volumina liefern —
  relative Toleranz statt Exact-Match (Determinismus-Test prüft nur
  *Wiederholbarkeit* bit-identisch).
- Wand-Verschneidung/boolesche Wandöffnungen (LH-FA-DOR-004/WIN-005)
  erst Folge-Slice (welle-2) — hier nur Einzelwand-Extrusion.

## 7. Closure-Notiz

**Closure-Kriterien (beobachtbar):**
- `make gates` grün: docs-check, gate-consistency, arch-check (inkl.
  Regel C), lint, test **24/24**, coverage 94,2 %.
- Reale OCC-Extrusion liefert das analytische Volumen (±1e-6, richtungs-
  unabhängig) und ist bit-identisch reproduzierbar.
- Regel C als Gate bewiesen: Provokation mit angled-, pfad- und
  quoted-`.hxx`-Form in `adapters/ui/` → `make arch-check` rot, zurückgebaut.

**Lerneintrag:**
- **Neuer Sensor — arch-check Regel C:** OCC-`.hxx` nur in
  `src/adapters/geometry/`. Schließt die ADR-0002-Fitness-Function-Lücke.
  *Regex muss jede Include-Form erfassen* (angled/quoted/Pfad-Präfix) —
  der erste Wurf war zu eng (`<Name.hxx>`) und ließ
  `<opencascade/…hxx>`/`"…hxx"` durch (Review-Finding, Modul 10).
- **ADR-0002-Folgepflicht erfüllt, ohne die immutable ADR zu ändern:**
  Erfüllungsstatus im **ADR-Index** (`docs/plan/adr/README.md` §ADR-
  Folgepflichten) geführt; keine Supersedes-ADR (Entscheidung steht,
  Folge-Aufgabe nur abgehakt). *Geschärfte Konvention:* offene
  ADR-Folgepflichten kanonisch im mutablen Index statusführen.
- **CMake-Lehre:** `${OpenCASCADE_LIBRARIES}` listet auch nicht-installierte
  DRAW-/Viewer-Toolkits → Link-Fehler. Gezielt die benötigten Modeling-
  Toolkits linken (`TKernel … TKPrim`).
- **Dimensionale Toleranz:** Volumen-Vergleich relativ (`expected · 1e-6`),
  *nicht* `GEOMETRY_TOLERANCE_MM` (Längen-Maß in mm) — Review-Finding.

**Restrisiko / Nachfolge:** boolesche Wandöffnungen (Türen/Fenster,
LH-FA-DOR-004/WIN-005) erst welle-2; hier nur Einzelwand-Extrusion ohne
Verschneidung.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Geometrie-Kern-Adapter

- **Modus:** GF
- **Konventionen-Dichte:** mittel-hoch — OCC-Adapter-Konvention in ADR-0002; Port-Vertrag (`GeometryKernelPort`) in 003a festgeschrieben.
- **Phase-Reife:** Phase 4 (Vertrag steht, Adapter wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig-mittel — OCC ist Fremdbibliothek, neu gekapselt (kein Bestandscode); Risiko im Determinismus, nicht in Doku-Drift.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — arch-check-/CMake-Konvention (ADR-0001), MR-003.
- **Phase-Reife:** Phase 4 — Regel C härtet die ADR-0002-Fitness-Function (Sensor wächst mit dem ersten OCC-Code).
- **Evidenz-/Diskrepanz-Risiko:** niedrig — Provokationstest weist Regel C nach.
- **Reconciliation-Aufwand:** keiner.

### Sub-Area: Test-Infrastruktur

- **Modus:** GF
- **Konventionen-Dichte:** mittel.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** mittel — Determinismus-Test als Adapter-Test (mit OCC).
- **Reconciliation-Aufwand:** keiner.
