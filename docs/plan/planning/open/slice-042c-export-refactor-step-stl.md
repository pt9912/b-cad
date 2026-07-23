---
id: slice-042c
titel: Export-Refactor STEP/STL — Body-Migration auf das Bündel + geometry→services_geo-Kante raus
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 042c: Export-Refactor STEP/STL — Body-Migration auf das `DerivedGeometry`-Bündel

**Status:** open — **SKELETT / Scope-Reservierung** ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Ledger,
[ADR-Index](../../adr/README.md)). **Kein Review-fähiger Plan:** Detail-Schnitt +
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
beim Start (nach slice-042b).

**Welle:** welle-5-erweiterung. **Dritter** der fünf [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices.
**Vorgänger:** slice-042b.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (Bündel-Konsum;
OCC-Montage bleibt im Adapter, pre-OCC-Primitive), [ADR-0014](../../adr/0014-step-stl-export-backend.md).

---

## 1. Ziel (Skelett)

Die STEP/STL-Export-Bodies auf das kern-gelieferte `DerivedGeometry`-Bündel umstellen (die
`services/geometry`-Aufrufe wandern in die kern-seitige `ExchangeService`-Berechnung — die aus slice-042a
hierher **verschobene** Berechnung, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1);
die `occ_solids`-Montage bleibt im Adapter). Danach **`geometry → services_geo`-Kante aus `.a-check.yml`
entfernen**.

## 2. Scope / bewusst NICHT Teil

- **Teil:** `ExchangeService` befüllt das STEP/STL-Bündel format-selektiv (geteilter **totaler** Höhen-Helfer
  statt dritter Kopie — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-2);
  STEP/STL konsumieren; `geometry → services_geo`-Kante raus.
- **NICHT Teil:** Persistenz-`rise` (042d); die finale architecture-§2-Tabelle/§1-Diagramm-Wahrheit (042e,
  dort fällt die letzte Kante). 2D-Projektion war 042b.

## 3. Vorbedingung

- slice-042b **done**.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  Geometrie-schwer → Code-Review vor Welle-Closure ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).

## 4. Invarianz-Netz

Die STEP-B-Rep-Zählung (`CLOSED_SHELL`) + das binäre STL-Netz-Orakel + die Integration über den
`ExchangeService` bleiben **unverändert grün** — **hier** verifizieren sie die kern-seitige Berechnung
(der eigentliche Prüfgewinn, wegen dessen die Berechnung von 042a hierher gezogen wurde). Zusätzlich ein
Total-Orakel (danglender `from_storey_id`/degeneriertes Bauteil → `exportModel` wirft **nicht**).
**Netz-Vormerkung (042a+042b-Code-Review INFO-1):** ein Negativ-Test, dass der `ExchangeService` STEP/STL das
**korrekt befüllte** (bzw. IFC/DXF das leere) Bündel reicht — ab hier konsum-relevant.
