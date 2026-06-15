# ADR-0002: Geometrie-Kern OpenCascade hinter GeometryKernelPort

**Status:** Accepted

**Datum:** 2026-06-08

**Autor:** Dietmar Burkard

**Bezug:** [REQ-TEC-003](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec), LH-FA-WAL-*, [LH-FA-D3-001](../../../spec/lastenheft.md#modul-3d-modellierung-d3), [LH-FA-DOR-004](../../../spec/lastenheft.md#lh-fa-dor-004--wandöffnung-automatisch-erzeugen), [LH-FA-WIN-005](../../../spec/lastenheft.md#lh-fa-win-005--wandöffnung-automatisch-erzeugen), ADR-0001

---

## Kontext

b-cad braucht einen 3D-Geometrie-Kern für Solids, boolesche Operationen
(Wandöffnungen für Türen/Fenster, [LH-FA-DOR-004](../../../spec/lastenheft.md#lh-fa-dor-004--wandöffnung-automatisch-erzeugen)/WIN-005) und Extrusion
(2D → 3D, [LH-FA-D3-001](../../../spec/lastenheft.md#modul-3d-modellierung-d3)). Die Domäne soll diesen Kern nicht direkt kennen
(ADR-0001) — er steht hinter `GeometryKernelPort`.

## Entscheidung

Wir wählen **OpenCascade (OCC)** als Geometrie-Kern, gekapselt im
Driven Adapter `src/adapters/geometry/` hinter `GeometryKernelPort`.

**Scope (bewusst eng).** Diese Entscheidung legt **nur** das Backend des
`GeometryKernelPort` fest: Solids, Extrusion ([LH-FA-D3-001](../../../spec/lastenheft.md#modul-3d-modellierung-d3)), boolesche
Operationen / Wandöffnungen ([LH-FA-DOR-004](../../../spec/lastenheft.md#lh-fa-dor-004--wandöffnung-automatisch-erzeugen)/WIN-005). **Nicht** Teil dieser
ADR:

- **STEP-/Format-Export** ([LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005) u. a.) — gehört hinter
  `ModelExporterPort` in `src/adapters/io/` (`spec/architecture.md` §1.2)
  und damit in eine eigene **IO/Export-ADR**. Ob ein Exporter OCC nutzt,
  ist dort zu entscheiden (inkl. der Adapter-Grenzen-Frage Geometrie↔IO).
- **Flächen-/Volumen-Auswertung** (LH-FA-EVL-*) — kein pauschaler Scope
  hier. Läuft später eine konkrete Methode über `GeometryKernelPort`,
  wird der Port-Vertrag im **jeweiligen Slice** geschärft, nicht hier
  vorgeprägt.

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
  Aufwand (Docker DevContainer mildert das, [REQ-TEC-009](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec)).

## Konsequenzen

- Positiv: Kern-Geometrie-Operationen (Solids, Extrusion, boolesche
  Operationen) ruhen auf bewährtem Fundament.
- Negativ: OCC-Build erhöht Image-Größe und Build-Zeit; OCC-Typen dürfen
  **nicht** über den Adapter hinaus lecken (ADR-0001).
- Folgepflicht: `GeometryKernelPort`-Vertrag definieren, bevor der
  Adapter Code bekommt; Determinismus der Geometrie-Operationen testen.

## Fitness Function

| Tooling | Regel (heute real durchgesetzt) | Make-Target |
|---|---|---|
| Architekturtest | Kern `src/hexagon/` bindet **kein** OCC (`*.hxx`)/Qt/SQLite ein und importiert nicht aus `adapters/`; kein Adapter importiert einen anderen Adapter | `make arch-check` (real) |

**Folgepflicht (Sensor-Lücke ehrlich benannt).** Die schärfere Regel
„OCC-Header (`*.hxx`) **nur** in `src/adapters/geometry/`" ist von
`make arch-check` heute **nicht** abgedeckt: Regel A prüft nur den Kern,
Regel B nur Adapter-zu-Adapter — ein OCC-Header in z. B.
`src/adapters/ui/` bliebe ungefangen
([`tools/arch-check.sh`](../../../tools/arch-check.sh)). Diese Isolation
wird in **slice-003b** als eigene `arch-check`-**Regel C** ergänzt (der
Slice führt den ersten echten OCC-Code ein und stützt sich darauf). Bis
dahin gilt sie als Konvention, **nicht** als Gate — bewusst nicht als
abgedeckt behauptet (Kurs-Modul 13: keine Sensor-Abdeckung
überversprechen).

## Re-Evaluierungs-Trigger

- Wenn der Build-/Container-Aufwand von OCC die Einstiegshürde
  unverhältnismäßig erhöht.
- Wenn ein Format-Bedarf (z. B. spezielles IFC-Mapping) gegen OCC-Grenzen
  läuft.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-08 | Proposed (aus Architektur-Outline, Bootstrap) | spec/architecture.md §3 |
| 2026-06-09 | Accepted — Scope auf `GeometryKernelPort`-Backend verengt (STEP-Export → IO/Export-ADR, EVL ausgenommen); Fitness Function ehrlich, `arch-check`-Regel C als Folgepflicht für slice-003b | slice-003-Review (Findings 1–3, vor dem Schnitt in 003a/003b) |
