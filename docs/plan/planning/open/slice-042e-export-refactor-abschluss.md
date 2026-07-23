---
id: slice-042e
titel: Export-Refactor Abschluss — .a-check.yml-Kanten final + architecture §2/§1 wahr
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [OBJ-003](../../../../spec/lastenheft.md#3-projektziele)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0001](../../adr/0001-hexagonale-architektur.md)]
---

# Slice 042e: Export-Refactor Abschluss — reines Hexagonal maschinell + doku-wahr

**Status:** open — **SKELETT / Scope-Reservierung** ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Ledger,
[ADR-Index](../../adr/README.md)). **Kein Review-fähiger Plan:** Detail-Schnitt +
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
beim Start (nach slice-042d).

**Welle:** welle-5-erweiterung. **Fünfter/letzter** der [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices.
**Vorgänger:** slice-042d. **Schließt** den [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Refactor ab.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (alle
Adapter→Kernel-Kanten weg; §1-Diagramm + §2-Tabelle werden wahr — **Verschärfung**, [§2.6](../../../../AGENTS.md) n/a),
[ADR-0001](../../adr/0001-hexagonale-architektur.md).

---

## 1. Ziel (Skelett)

Nachdem 042c/042d die letzten `services/geometry`-Adapter-Aufrufe beseitigt haben: die verbliebenen
Allow-Kanten (`geometry`/`persistence → services_geo`, soweit von den Vorgängern noch nicht entfernt) final
aus `.a-check.yml` streichen, sodass `services_geo` nur noch die kern-interne Kante `services → services_geo`
trägt. Dann `architecture.md` **§2-Tabelle** (kein Adapter führt `services/geometry`) **und §1-Diagramm**
(reines Hexagonal, kein Adapter→Kernel) doku-wahr ziehen — die in slice-041a bewusst deferierte §2/§1-Wahrheit.

## 2. Scope / bewusst NICHT Teil

- **Teil:** `.a-check.yml`-Kanten-Verschärfung final; architecture §2-Tabelle + §1-Diagramm wahr; `## Geschichte`-
  Provenance ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)).
- **NICHT Teil:** Code-Verhalten (das ist mit 042c/042d schon invariant migriert) — dieser Slice ist
  Gate-Verschärfung + Doku-Wahrheit.

## 3. Vorbedingung

- slice-042d **done** (alle Adapter→`services_geo`-Aufrufe beseitigt).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.

## 4. Invarianz-Netz / Fitness

`make a-check` grün **nach** der Kanten-Entfernung (Gegenprobe: ein Rest-Adapter-Aufruf würde jetzt failen);
`make gates` grün; `make docs-check` grün (architecture §2/§1 konsistent mit `.a-check.yml`). Damit ist der
[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Refactor **komplett** — der
ADR-Index-Folgepflicht-Block ist erledigt.
