---
id: spike-001
titel: Toolchain-Reproduzierbarkeit — Base-Version & Dependency-Pinning
status: open
welle: ohne Welle (Spike)
lastenheft_refs: [LH-QA-003]
req_tec_refs: [REQ-TEC-009]
adr_refs: []
---

# Spike 001: Toolchain-Reproduzierbarkeit — Base-Version & Dependency-Pinning

**Status:** open

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

- [ ] Findings-Tabelle: 24.04 vs 26.04 (Paket-Verfügbarkeit, Versionen, `make gates`-Ergebnis).
- [ ] Empfehlung Base + Pinning-Strategie mit Begründung.
- [ ] **ADR-0004** (Container-/Dependency-Pinning) als `Proposed` entworfen; ADR-Index aktualisiert.
- [ ] Folge-Slice(s) für die Umsetzung in `open/` angelegt (z. B. „Base-Digest + Snapshot pinnen").

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
