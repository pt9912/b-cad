# ADR-0002: Geometrie-Kern OpenCascade hinter GeometryKernelPort

**Status:** Proposed

**Datum:** 2026-06-08

**Autor:** Dietmar Burkard

**Bezug:** REQ-TEC-003, LH-FA-WAL-*, LH-FA-D3-001, LH-FA-DOR-004, LH-FA-WIN-005, ADR-0001

---

## Kontext

b-cad braucht einen 3D-Geometrie-Kern für Solids, boolesche Operationen
(Wandöffnungen für Türen/Fenster), Extrusion (2D → 3D, LH-FA-D3-001) und
die Ableitung von Flächen/Volumen (LH-FA-EVL-*). Die Domäne soll diesen
Kern nicht direkt kennen (ADR-0001) — er steht hinter
`GeometryKernelPort`.

## Entscheidung

Wir wählen **OpenCascade (OCC)** als Geometrie-Kern, gekapselt im
Driven Adapter `src/adapters/geometry/` hinter `GeometryKernelPort`.

## Verglichene Alternativen

### Option A — Eigene Geometrie-Engine

- Pro: keine schwere Abhängigkeit, volle Kontrolle.
- Contra: boolesche Solid-Operationen und robuste BREP-Geometrie sind
  jahrelange Arbeit; unrealistisch für b-cad.

### Option B — CGAL

- Pro: robuste, exakte Geometrie.
- Contra: stärker auf Meshes/Polyeder fokussiert; BREP/CAD-Workflows
  und Austauschformate (STEP) weniger direkt; Lizenz (GPL-Teile) prüfen.

### Option C — OpenCascade (gewählt)

- Pro: ausgereifter BREP-Kern; STEP/IGES nativ; Extrusion, boolesche
  Operationen, Solids vorhanden; im CAD-Umfeld etabliert.
- Contra: große Abhängigkeit; eigene API-Konventionen; Build-/Container-
  Aufwand (Docker DevContainer mildert das, REQ-TEC-009).

## Konsequenzen

- Positiv: Kern-Geometrie-Operationen und STEP-Export (LH-FA-IO-005)
  ruhen auf bewährtem Fundament.
- Negativ: OCC-Build erhöht Image-Größe und Build-Zeit; OCC-Typen dürfen
  **nicht** über den Adapter hinaus lecken (ADR-0001).
- Folgepflicht: `GeometryKernelPort`-Vertrag definieren, bevor der
  Adapter Code bekommt; Determinismus der Geometrie-Operationen testen.

## Fitness Function

| Tooling | Regel | Make-Target (geplant) |
|---|---|---|
| Architekturtest | OCC-Header werden nur in `src/adapters/geometry/` eingebunden | `make arch-check` |

## Re-Evaluierungs-Trigger

- Wenn der Build-/Container-Aufwand von OCC die Einstiegshürde
  unverhältnismäßig erhöht.
- Wenn ein Format-Bedarf (z. B. spezielles IFC-Mapping) gegen OCC-Grenzen
  läuft.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-08 | Proposed (aus Architektur-Outline, Bootstrap) | spec/architecture.md §3 |
