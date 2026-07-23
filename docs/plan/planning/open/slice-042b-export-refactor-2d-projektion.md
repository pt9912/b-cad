---
id: slice-042b
titel: Export-Refactor 2D-Projektion — plan_geometry in den Kern + PlanViewPort (entsperrt Canvas)
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-003](../../../../spec/lastenheft.md#3-projektziele), [OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0001](../../adr/0001-hexagonale-architektur.md)]
---

# Slice 042b: Export-Refactor 2D-Projektion — Kern-Hebung + `PlanViewPort`

**Status:** open — **SKELETT / Scope-Reservierung** ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Ledger,
[ADR-Index](../../adr/README.md)). **Kein Review-fähiger Plan:** der Detail-Schnitt + das
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
folgen **beim Start** (nach Abschluss von slice-042a), da der genaue Schnitt vom 042a-Ergebnis abhängt.

**Welle:** welle-5-erweiterung. **Zweiter** der fünf [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices
(Familie 042a…e). **Vorgänger:** slice-042a (Kern-Naht). **Deckt zugleich [ADR-0019](../../adr/0019-drw-2d-canvas.md)s
Lese-Naht-Refactor** (dieselbe Hebung).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (2D-Projektion
kern-geliefert) + [ADR-0019](../../adr/0019-drw-2d-canvas.md) (2D-Lese-Naht = Kern-Projektion + `PlanViewPort`,
2D-Analog zum `ViewModelPort`).

---

## 1. Ziel (Skelett)

Die reine 2D-Grundriss-Projektion (`projectPlan` → `PlanView`, heute io-resident) in den framework-freien
Kern (`services/geometry`) heben und über einen neuen Driving-Read-Port `PlanViewPort` lesbar machen. Die
2D-Exporter (PDF/PNG) beziehen die `PlanView` vom Kern (im `DerivedGeometry`-Bündel bzw. Kern-Funktion),
DXF iteriert weiter direkt + `visibleLayerIds`. **Entsperrt den DRW-Canvas** (der Canvas zieht `PlanViewPort`).

## 2. Scope / bewusst NICHT Teil

- **Teil:** Projektions-Hebung (reiner Umzug, Logik unverändert) + `PlanViewPort` + 2D-Export auf die Kern-Quelle.
- **NICHT Teil:** Canvas-Widget-Impl selbst (eigener Slice nach [ADR-0019](../../adr/0019-drw-2d-canvas.md)); STEP/STL (042c); Persistenz (042d);
  `.a-check.yml`/architecture-Abschluss (042e).

## 3. Vorbedingung

- slice-042a **done**.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  Geometrie-berührend → Code-Review vor Welle-Closure ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).

## 4. Invarianz-Netz

Die 2D-Export-Decode-Orakel (DXF/PDF/PNG Erscheint/Fehlt) bleiben nach der Hebung **unverändert grün**
(reiner Umzug). Reißt eines → nicht invariant → Stopp.
