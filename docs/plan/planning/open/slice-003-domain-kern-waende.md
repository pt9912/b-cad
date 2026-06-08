---
id: slice-003
titel: Domain-Kern & Wände (parametrisch + Extrusion)
status: open
welle: welle-1-mvp
lastenheft_refs: [LH-FA-BLD-001, LH-FA-FLR-001, LH-FA-WAL-001, LH-FA-WAL-002, LH-FA-WAL-003, LH-FA-D3-001]
adr_refs: [ADR-0001, ADR-0002]
---

# Slice 003: Domain-Kern & Wände (parametrisch + Extrusion)

**Status:** open

**Welle:** welle-1-mvp

**Bezug:** LH-FA-BLD-001, LH-FA-FLR-001, LH-FA-WAL-001, LH-FA-WAL-002, LH-FA-WAL-003, LH-FA-D3-001, ADR-0001, ADR-0002

**Autor:** Dietmar Burkard. **Datum:** 2026-06-08.

---

## 1. Ziel

Den framework-freien Domain-Kern und den ersten echten Use-Case
liefern: Projekt mit Geschoss anlegen, Wände zeichnen mit
parametrischer Stärke (50–1000 mm) und Höhe (500–10000 mm) und die
Wände über den `GeometryKernelPort` (OCC-Adapter) in 3D extrudieren.

## 2. Definition of Done

- [ ] `src/hexagon/model/` (Building, Storey, Wall) + `EditStructurePort` + `StructureEditService`: Wand erzeugen, Stärke/Höhe setzen mit Klemmung (`E-VAL-001`, LH-FA-WAL-002/003).
- [ ] `GeometryKernelPort` + OCC-Adapter: Extrusion (LH-FA-D3-001); Service-Tests mit Geometrie-Port-Double (ohne OCC).
- [ ] Akzeptanz-Tests Happy/Boundary/Negative für LH-FA-WAL-002 (50/1000 mm, 49/1001 mm), Wertebereiche aus [`spec/spezifikation.md` §3](../../../../spec/spezifikation.md#3-defaults-und-konstanten) referenziert.
- [ ] `make gates` grün; Closure-Notiz.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{building,storey,wall}.h` | neu | Domain-Modell (OBJ-002/003) |
| `src/hexagon/ports/driving/edit_structure_port.h` | neu | Use-Case-Schnittstelle |
| `src/hexagon/ports/driven/geometry_kernel_port.h` | neu | Geometrie-Abstraktion (ADR-0001) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | neu | Validierung + Rebuild |
| `src/adapters/geometry/occ_geometry_adapter.{h,cpp}` | neu | OCC-Extrusion (ADR-0002) |
| `tests/hexagon/…`, `tests/adapters/…` | neu | AK als Tests + Geometrie-Determinismus |

## 4. Trigger

- slice-002 done (Code-Gates stehen, arch-check grün).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz geschrieben.

## 6. Risiken und offene Punkte

- Wand-Verbindungs-/Verschneidungsgeometrie (LH-FA-WAL-006) erst in Folge-Slice — hier nur Einzelwände + Extrusion.
- Geometrie-Determinismus über OCC: Toleranz (`GEOMETRY_TOLERANCE_MM`) im Test absichern.

## 7. Closure-Notiz

<!-- Erst nach Abschluss füllen: Lerneintrag (geschärfte Regel / neuer Sensor / Spec-Lücke). -->

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Pure-Domain-/Port-Konvention in ADR-0001, `spec/architecture.md` §2, `harness/conventions.md`.
- **Phase-Reife:** Phase 4 (Lastenheft + ADR führen; Code wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Geometrie-Kern-Adapter

- **Modus:** GF
- **Konventionen-Dichte:** mittel-hoch — OCC-Adapter-Konvention in ADR-0002; Port-Vertrag (`GeometryKernelPort`) wird mit diesem Slice festgeschrieben.
- **Phase-Reife:** Phase 2→4 (Port-Vertrag wird kohärent).
- **Evidenz-/Diskrepanz-Risiko:** niedrig-mittel — OCC ist Fremdbibliothek, aber neu gekapselt (kein Bestandscode); Risiko liegt im Determinismus, nicht in Doku-Drift.
- **Reconciliation-Aufwand:** keiner (GF). *Hinweis:* würde übernommener OCC-Bestandscode eingeführt, kippte diese Sub-Area nach BF mit Graduation-Trigger.

### Sub-Area: Test-Infrastruktur

- **Modus:** GF (vorausgesetzt slice-002 hat die Test-Konvention in `harness/conventions.md` verankert).
- **Konventionen-Dichte:** mittel — GoogleTest-/Determinismus-Konvention aus slice-002.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** mittel — AK-Tests müssen `LH-`-ID im Namen tragen.
- **Reconciliation-Aufwand:** keiner, sofern slice-002 abgeschlossen.
