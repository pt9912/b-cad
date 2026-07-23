---
id: slice-042d
titel: Export-Refactor Persistenz — rise kern-seitig + persistence→services_geo-Kante raus
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md)]
---

# Slice 042d: Export-Refactor Persistenz — `rise` kern-seitig geliefert

**Status:** open — **SKELETT / Scope-Reservierung** ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Ledger,
[ADR-Index](../../adr/README.md)). **Kein Review-fähiger Plan:** Detail-Schnitt +
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
beim Start (nach slice-042c).

**Welle:** welle-5-erweiterung. **Vierter** der fünf [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices.
**Vorgänger:** slice-042c.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (Persistenz erhält
den abgeleiteten `rise`-Skalar analog dem Bündel — **save-Signatur-Form**, `Stair` trägt kein `rise`-Feld,
[MR-016](../../../../harness/conventions.md) beachten [Accepted-ADR immutabel]), [ADR-0003](../../adr/0003-persistenz-sqlite.md).

---

## 1. Ziel (Skelett)

Der Kern liefert den abgeleiteten `rise` je Treppe kern-seitig; der Persistenz-Adapter erhält ihn als
gelieferten Skalar (nicht selbst aus `services/geometry` ableiten). Danach **`persistence → services_geo`-
Kante aus `.a-check.yml` entfernen** (der `stair_geometry`-Import fällt aus dem SQLite-Adapter).

## 2. Scope / bewusst NICHT Teil

- **Teil:** `rise`-Bereitstellung kern-seitig (save-Signatur-Form); `persistence → services_geo`-Kante raus;
  atomares Save unberührt.
- **NICHT Teil:** die finale architecture-§2/§1-Wahrheit (042e). STEP/STL war 042c, 2D war 042b.

## 3. Vorbedingung

- slice-042c **done**.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.

## 4. Invarianz-Netz

Die SQLite-Round-Trip-/Crash-Recovery-Tests (inkl. `rise_mm`-Persistenz) bleiben **unverändert grün** —
der Skalar-Wert je Treppe ist byte-identisch, nur sein **Berechnungs-Ort** wandert in den Kern.
