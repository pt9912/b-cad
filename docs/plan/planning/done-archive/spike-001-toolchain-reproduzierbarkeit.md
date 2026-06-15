---
id: spike-001
titel: Toolchain-Reproduzierbarkeit — Base-Version & Dependency-Pinning
status: done
welle: ohne Welle (Spike)
lastenheft_refs: [LH-QA-003]
req_tec_refs: [REQ-TEC-009]
adr_refs: [ADR-0004]
---

# Spike 001: Toolchain-Reproduzierbarkeit — Base-Version & Dependency-Pinning

**Status:** done

**Welle:** ohne Welle (Spike — zeitlich begrenzte Untersuchung, kein Feature).

**Bezug:** REQ-TEC-009 (Docker DevContainer), Modul 14 (Reproduzierbarkeit). Mündet in **ADR-0004**.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-08.

---

## 1. Ziel / Frage

Beantworten, **wie b-cad Lib- und Toolchain-Versionen reproduzierbar
pinnt** und **welche Ubuntu-Base** genutzt wird. Auslöser: `apt-get
install <name>` (ungepinnt) driftet mit dem Live-Archiv; Ubuntu 24.04
ist eine LTS-Generation alt.

## 2. Hypothesen / Optionen (zu evaluieren)

**Base-Version:**
- **Ubuntu 24.04 (Noble)** — aktuell genutzt, verifiziert grün.
- **Ubuntu 26.04** — neueres LTS (Stand 2026). *Offen:* sind
  `qt6-base-dev`, `libocct-*-dev`, `libtbb-dev`, `clang-tidy`, `gcovr`
  dort verfügbar/gleich benannt? Welche Qt6-/OCC-Versionen? Codename?

**Pinning-Strategie:**
- **A — Base-Digest (`FROM ...@sha256`) + apt-Snapshot** (`snapshot.ubuntu.com/<timestamp>`): reproduzierbare apt-Versionen, weiterhin Binärpakete (schnell).
- **B — vcpkg/Conan Manifest + Lockfile** (`builtin-baseline`): exakte Versionskontrolle inkl. transitiver Deps; Qt6/OCC aus Source = sehr lange Builds ohne Binary-Cache.
- **C — nur Base-Digest**: minimal; apt-Versionen driften weiter.

## 3. Vorgehen (Timebox: 1 Tag)

1. `FROM ubuntu:26.04` real bauen: apt-Resolve prüfen, aufgelöste Versionen (`dpkg-query`) erfassen, `make gates` durchlaufen lassen.
2. apt-Snapshot (snapshot.ubuntu.com) prototypisch einbauen, Reproduzierbarkeit zweier Builds vergleichen.
3. vcpkg/Conan-Aufwand für Qt6+OCC grob abschätzen (Build-Zeit, Binary-Cache-Bedarf) — **ohne** vollständige Umsetzung.
4. Base-Digests (ubuntu, node) erfassen.

## 4. Definition of Done (Spike)

- [x] Findings-Tabelle: 24.04 vs 26.04 (Paket-Verfügbarkeit, Versionen, `make gates`-Ergebnis) — siehe §9.
- [x] Empfehlung Base + Pinning-Strategie mit Begründung — §9 + ADR-0004.
- [x] **ADR-0004** (Container-/Dependency-Pinning) als `Proposed` entworfen; ADR-Index aktualisiert.
- [x] Folge-Slice angelegt: [`slice-004`](../done/slice-004-toolchain-pinning-26.04.md).

## 5. Trigger

- slice-002 done (Gates stehen — der Spike kann sie als Reproduzierbarkeits-Messlatte nutzen).

## 6. Closure-Trigger

- ADR-0004 entworfen + Umsetzungs-Slice in `open/`. Der Spike selbst liefert **keinen** Produktionscode (Wegwerf-Prototyp erlaubt).

## 7. Risiken / Hinweise

- 26.04-Pakete sind aus der aktuellen Wissensbasis **nicht** verifizierbar — der Spike muss real bauen.
- vcpkg/OCC-from-source kann den Build drastisch verlangsamen → Binary-Cache-Frage ist Teil der Bewertung.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — DevContainer/Gate-Konventionen in `.devcontainer/`, `Makefile`, MR-003.
- **Phase-Reife:** Phase 4 (Toolchain steht; der Spike schärft die Reproduzierbarkeits-Dimension).
- **Evidenz-/Diskrepanz-Risiko:** niedrig — reiner Untersuchungs-Slice, kein Produktionscode.
- **Reconciliation-Aufwand:** keiner (GF); Ergebnis ist ADR-0004 + Folge-Slice.

## 9. Ergebnis & Empfehlung (Closure)

**Abgeschlossen am:** 2026-06-08. Empirisch erhoben (reale Docker-Builds,
`apt-get install -s`).

**Befund 1 — 26.04 existiert und ist machbar.** `ubuntu:26.04`
(`resolute`) pullbar; **alle** Paketnamen identisch auflösbar, neuere
Versionen:

| Paket | 24.04 noble | 26.04 resolute | `make`-Gate auf 26.04 |
|---|---|---|---|
| cmake | 3.28.3 | 4.2.3 | `build` ✅ |
| clang-tidy | 18 | 21 | `lint` ❌ (s. u.) |
| gcc (build-essential) | 13 (12.10) | 15 (12.12) | `test` ✅ 5/5 |
| qt6-base-dev | 6.4.2 | 6.10.2 | `coverage-gate` ✅ 100 % |
| libocct-* | 7.6.3 | 7.9.2 | `arch-check` ✅ |
| libtbb-dev | 2021.11 | 2022.3 | — |
| libsqlite3-dev | 3.45.1 | 3.46.1 | — |
| libgtest-dev | 1.14.0 | 1.17.0 | — |

**Befund 2 — `lint` ist auf 26.04 der einzige Blocker.** clang-tidy 21
findet die Standard-Header nicht (`'iostream'/'string' file not found`,
clang-21/gcc-15-Include-Mismatch) — **kein** strengerer Check, sondern
Toolchain-Integration. Fix gehört in slice-004.

**Befund 3 — node-Base.** `node:24-alpine` (LTS) verifiziert docs-check
lauffähig; `node:26-alpine` (Current) verfügbar. Empfehlung: **node 24**
(LTS für Werkzeuge).

**Befund 4 — Pinning.** `snapshot.ubuntu.com` erreichbar; `apt-get
update` gegen einen Timestamp-Snapshot funktioniert → Option A (Base-
Digest + Snapshot) ist machbar und schlank. vcpkg/Conan (B) bleibt zu
schwer ohne Binary-Cache (Qt6/OCC aus Source).

**Empfehlung → ADR-0004 (Proposed):** Option A (Digest + Snapshot),
Migration auf 26.04 + node 24, mit clang-tidy-21-Lint-Fix.
**Umsetzung:** [`slice-004`](../done/slice-004-toolchain-pinning-26.04.md).

**Erfasste Digests:** `ubuntu:24.04` `sha256:786a8b55…`, `ubuntu:26.04`
`sha256:f3d28607…` (node-Digest in slice-004 beim Build erfassen).

**Steering-Loop:** Der Spike hat den 26.04-`lint`-Blocker **vor** der
Migration sichtbar gemacht — genau der Zweck (de-risk, kein
Produktionscode geändert).
