---
id: slice-004
titel: Toolchain-Pinning + Migration auf 26.04 / node 24
status: open
welle: welle-1-mvp
lastenheft_refs: []
req_tec_refs: [REQ-TEC-009]
adr_refs: [ADR-0004]
---

# Slice 004: Toolchain-Pinning + Migration auf 26.04 / node 24

**Status:** open

**Welle:** welle-1-mvp

**Bezug:** ADR-0004 (Container-/Dependency-Pinning), REQ-TEC-009, Modul 14. Setzt die Empfehlung aus [`spike-001`](../done/spike-001-toolchain-reproduzierbarkeit.md) um.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-08.

---

## 1. Ziel

Reproduzierbare, aktuelle Toolchain gemäß ADR-0004: Base-Images per
`@sha256`-Digest gepinnt, apt-Versionen über `snapshot.ubuntu.com`
fixiert, Migration auf **Ubuntu 26.04 (resolute)** und
**`node:24-alpine`** — mit grünem `make gates`.

## 2. Definition of Done

- [ ] `.devcontainer/Dockerfile`: `FROM ubuntu:26.04@sha256:…` (deps + arch-check); apt-Quellen auf `snapshot.ubuntu.com/<timestamp>` umgestellt.
- [ ] `tools/Dockerfile`: `FROM node:24-alpine@sha256:…`.
- [ ] **`lint`-Blocker gelöst** (clang-tidy 21 findet Standard-Header) — z. B. passende `libstdc++`-Dev-Pakete oder `--extra-arg`/Compiler-Abgleich; dokumentiert.
- [ ] `make gates` (alle 5) grün auf 26.04.
- [ ] Reproduzierbarkeits-Beleg: zwei `make build`-Läufe → identische `dpkg-query`-Versionsliste (Fitness Function aus ADR-0004); Versionsliste als Manifest committet.
- [ ] Closure-Notiz; ADR-0004 → `Accepted` (Index aktualisieren).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.devcontainer/Dockerfile` | update | Digest-Pin + Snapshot-Quellen + 26.04 |
| `tools/Dockerfile` | update | node:24-alpine@digest |
| `Makefile` | update | ggf. Reproduzierbarkeits-Target / Versions-Manifest |
| `harness/` (Versions-Manifest) | neu | erfasste `dpkg-query`-Liste als Audit-Artefakt |
| `docs/plan/adr/0004-*` | update | Proposed → Accepted |

## 4. Trigger

- spike-001 done (ADR-0004 Proposed liegt vor).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün auf gepinntem 26.04, ADR-0004 Accepted, Closure-Notiz.

## 6. Risiken und offene Punkte

- clang-tidy-21/gcc-15-Header-Resolution ist der Hauptaufwand (spike-001-Befund).
- Snapshot-Timestamp wählen, der alle benötigten Pockets (main/universe) trägt.
- Base-Digests können sich seit spike-001 geändert haben — neu auflösen.

## 7. Closure-Notiz

<!-- Erst nach Abschluss füllen: Lerneintrag. -->

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — DevContainer/Gate-Konventionen + ADR-0004.
- **Phase-Reife:** Phase 4 (Toolchain steht; dieser Slice härtet Reproduzierbarkeit + Aktualität).
- **Evidenz-/Diskrepanz-Risiko:** mittel — Versions-Sprung (Qt/OCC/clang-tidy) kann lint/Build berühren; Gegenmittel: `make gates` als Messlatte.
- **Reconciliation-Aufwand:** ein Slice; danach reproduzierbare, gepinnte Toolchain (stabil GF).
