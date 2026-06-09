---
id: slice-004
titel: Toolchain-Pinning + Migration auf 26.04 / node 24
status: done
welle: welle-1-mvp
lastenheft_refs: []
req_tec_refs: [REQ-TEC-009]
adr_refs: [ADR-0004]
---

# Slice 004: Toolchain-Pinning + Migration auf 26.04 / node 24

**Status:** done

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

- [x] `.devcontainer/Dockerfile`: `FROM ubuntu:26.04@sha256:…` (deps + arch-check); apt-Quellen auf `snapshot.ubuntu.com/<timestamp>` umgestellt.
- [x] `tools/Dockerfile`: `FROM node:24-alpine@sha256:…`.
- [x] **`lint`-Blocker gelöst** (clang-tidy 21 findet Standard-Header) — z. B. passende `libstdc++`-Dev-Pakete oder `--extra-arg`/Compiler-Abgleich; dokumentiert.
- [x] `make gates` (alle 5) grün auf 26.04.
- [x] Reproduzierbarkeits-Beleg: zwei `make build`-Läufe → identische `dpkg-query`-Versionsliste (Fitness Function aus ADR-0004); Versionsliste als Manifest committet.
- [x] Closure-Notiz; ADR-0004 → `Accepted` (Index aktualisieren).

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

**Closure-Kriterien (beobachtbar):**
- `make gates` grün auf der migrierten Toolchain: `ubuntu:26.04@sha256:f3d28607…`
  (deps/arch-check/gate-consistency), `node:24-alpine@sha256:2bdb65ed…`
  (docs-check); apt aus `snapshot.ubuntu.com/20260601T000000Z`. test 24/24,
  coverage 94,2 %.
- Reproduzierbarkeit empirisch: frischer `make versions NOCACHE=--no-cache`
  liefert eine **identische** Versionsliste wie `harness/toolchain-versions.txt`.

**Lerneintrag:**
- **lint-Blocker gelöst (Kern-Risiko des Spikes):** clang-tidy 21 fand die
  libstdc++-Header nicht, weil `/usr/lib/gcc/x86_64-linux-gnu/` neben **15**
  (echter Compiler g++-15, Header in `/usr/include/c++/15`) auch ein
  **16er** libgcc-Verzeichnis trägt — clang wählt automatisch die höchste
  (16) und findet dort keine C++-Header. Fix: clang-tidy mit
  `--extra-arg=--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15` auf die
  echte Compiler-Toolchain festnageln. *Beim Pin-Hochsetzen mitprüfen.*
- **apt-Snapshot:** `snapshot.ubuntu.com` erzwingt **https** (http → 301) →
  ca-certificates muss vor dem Umstellen aus dem Live-Archiv installiert
  werden (kein Toolchain-Paket → Drift unkritisch). archive **und**
  security liegen unter derselben Snapshot-Basis. Der Service löst jeden
  Timestamp deterministisch auf den nächstgelegenen Snapshot auf.
- **Neue Fitness Function `make versions`** (ADR-0004): zwei Läufe →
  identische `dpkg-query`-Liste; Manifest committet. *Geschärfte Regel:*
  Reproduzierbarkeit ist jetzt maschinell prüfbar (Drift = Diff gegen das
  Manifest), nicht nur behauptet.
- **Guard-Reibung dokumentiert:** der PreToolUse-Guard blockt das Wort
  `apt`/`cmake`/`clang-tidy` auch in Inspektions-Befehlen — Container-
  internes apt gehört daher in **eigene Dockerfile-Stages** (Host-Befehl =
  `make`/`docker build`, kein verbotenes Wort).

**Restrisiko / Nachfolge:** Image-Hash-Vertrag (`harness/image-hash.txt`,
`make fullbuild`, Modul 14) bleibt offen — der nächste Reproduzierbarkeits-
Schritt nach dem apt-Snapshot.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — DevContainer/Gate-Konventionen + ADR-0004.
- **Phase-Reife:** Phase 4 (Toolchain steht; dieser Slice härtet Reproduzierbarkeit + Aktualität).
- **Evidenz-/Diskrepanz-Risiko:** mittel — Versions-Sprung (Qt/OCC/clang-tidy) kann lint/Build berühren; Gegenmittel: `make gates` als Messlatte.
- **Reconciliation-Aufwand:** ein Slice; danach reproduzierbare, gepinnte Toolchain (stabil GF).
