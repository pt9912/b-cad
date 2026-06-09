---
id: slice-003b
titel: OCC-Extrusion (GeometryKernelPort-Adapter) + arch-check Regel C
status: open
welle: welle-1-mvp
lastenheft_refs: [LH-FA-D3-001]
adr_refs: [ADR-0001, ADR-0002]
---

# Slice 003b: OCC-Extrusion (GeometryKernelPort-Adapter) + arch-check Regel C

**Status:** open

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

- [ ] `src/adapters/geometry/occ_geometry_adapter.{h,cpp}` implementiert
      `GeometryKernelPort::extrudeWall` via OCC (BRep-Prisma + Volumen
      über `GProp`/`BRepGProp`); kein OCC-Typ verlässt den Adapter.
- [ ] `tools/arch-check.sh` **Regel C**: kein `#include <*.hxx>` außerhalb
      `src/adapters/geometry/` (schließt die Sensor-Lücke aus ADR-0002
      Finding 2). `harness/README.md`/`AGENTS.md` §arch-check-Vertrag um
      Regel C + Bindung ADR-0002 erweitern.
- [ ] Adapter-Test (mit OCC, LH-FA-D3-001): Volumen ≈ analytisch innerhalb
      `GEOMETRY_TOLERANCE_MM`; Determinismus (zwei Extrusionen identisch).
- [ ] Provokation: ein OCC-`.hxx`-Include in `src/adapters/ui/` lässt
      `make arch-check` rot werden (Regel-C-Nachweis), danach zurückgebaut.
- [ ] `make gates` grün; Closure-Notiz schließt die ADR-0002-Folgepflicht.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/geometry/occ_geometry_adapter.{h,cpp}` | neu | OCC-Extrusion (ADR-0002) |
| `src/adapters/CMakeLists.txt` | ändern | neue `.cpp` in `bcad_adapters` (OCC-Modeling-Libs) |
| `tools/arch-check.sh` | ändern | Regel C (OCC-`.hxx` nur in geometry/) |
| `harness/README.md`, `AGENTS.md` | ändern | arch-check-Vertrag um Regel C + ADR-0002-Bindung |
| `tests/adapters/test_occ_geometry_adapter.cpp` | neu | Volumen + Determinismus (LH-FA-D3-001) |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |

## 4. Trigger

- slice-003a done (Port-Vertrag + Modell stehen).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz geschrieben
  (ADR-0002-Folgepflicht Regel C als geschlossen vermerkt).

## 6. Risiken und offene Punkte

- Geometrie-Determinismus über OCC: Toleranz (`GEOMETRY_TOLERANCE_MM`)
  im Test absichern; OCC kann je nach Version minimal abweichende
  Volumina liefern — Toleranz statt Exact-Match.
- Wand-Verschneidung/boolesche Wandöffnungen (LH-FA-DOR-004/WIN-005)
  erst Folge-Slice (welle-2) — hier nur Einzelwand-Extrusion.

## 7. Closure-Notiz

<!-- Erst nach Abschluss füllen. -->

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
