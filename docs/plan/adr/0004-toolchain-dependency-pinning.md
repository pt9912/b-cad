# ADR-0004: Container-/Dependency-Pinning und Base-Version

**Status:** Accepted

**Datum:** 2026-06-08

**Autor:** Dietmar Burkard

**Bezug:** REQ-TEC-009, Modul 14, [`spike-001`](../planning/done-archive/spike-001-toolchain-reproduzierbarkeit.md)

---

## Kontext

Die DevContainer-/Gate-Toolchain installiert Libs via `apt-get install
<name>` **ohne Versions-Pin**. Jeder `apt-get update` löst gegen das
Live-Ubuntu-Archiv auf → die Versionen (Qt6, OpenCascade, clang-tidy …)
driften über die Zeit, unkontrolliert. Ein Base-Digest-Pin allein
fixiert nur das Basis-Dateisystem, nicht die apt-Versionen. Zudem ist
die Base (Ubuntu 24.04) eine LTS-Generation alt.

`spike-001` hat empirisch erhoben (real gebaut, `apt-get install -s`):

| Paket | 24.04 noble | 26.04 resolute |
|---|---|---|
| cmake | 3.28.3 | 4.2.3 |
| clang-tidy | 18 | 21 |
| build-essential (gcc) | 12.10 (gcc 13) | 12.12 (gcc 15) |
| qt6-base-dev | 6.4.2 | 6.10.2 |
| libocct-* | 7.6.3 | 7.9.2 |
| libtbb-dev | 2021.11 | 2022.3 |
| libsqlite3-dev | 3.45.1 | 3.46.1 |
| libgtest-dev | 1.14.0 | 1.17.0 |

**docs-check-Base (node):** `tools/Dockerfile` nutzt `node:22-alpine`
(LTS-Generation alt). `spike-001` hat `node:24-alpine` real verifiziert
(docs-check läuft) und `node:26-alpine` als verfügbar bestätigt. Stand
Juni 2026 ist **Node 24 das aktive LTS**, Node 26 ist „Current" (LTS erst
Okt 2026) → für ein Werkzeug LTS.

**Gate-Ergebnis auf 26.04:** `build`, `test` (5/5), `coverage-check`
(100 %), `arch-check` **grün**; **`lint` rot** — clang-tidy 21 findet die
Standard-Header nicht (`'iostream'/'string' file not found`,
clang-21/gcc-15-Include-Mismatch). `snapshot.ubuntu.com` ist erreichbar;
`apt-get update` gegen einen Timestamp-Snapshot funktioniert.

## Entscheidung

1. **Reproduzierbarkeit:** Base per **`@sha256`-Digest** pinnen **und**
   apt-Quellen auf **`snapshot.ubuntu.com/<timestamp>`** umstellen
   (Option A). Damit sind sowohl Basis-FS als auch apt-Paketversionen
   reproduzierbar; Updates erfolgen durch bewusstes Hochsetzen von
   Digest + Snapshot-Timestamp.
2. **Base-Versionen:** Migration der App-Toolchain auf **Ubuntu 26.04
   (resolute)** und der docs-check-Base auf **`node:24-alpine` (LTS)** —
   beide per Digest gepinnt. Voraussetzung für 26.04: der `lint`-Blocker
   (clang-tidy-21-Header-Resolution) wird im Migrations-Slice gelöst.

## Verglichene Alternativen

### Option A — Base-Digest + apt-Snapshot (gewählt)

- Pro: apt-Versionen reproduzierbar; bleibt Binärpaket-schnell; geringe Komplexität; Snapshot empirisch erreichbar (spike-001).
- Contra: Snapshot-Timestamp ist ein weiterer zu pflegender Pin.

### Option B — vcpkg / Conan (Manifest + Lockfile)

- Pro: exakte Versionskontrolle inkl. transitiver Deps; Industriestandard.
- Contra: Qt6 + OpenCascade aus **Source** = sehr lange Builds ohne Binary-Cache. Unverhältnismäßig, solange kein Cache (Registry/Artifactory) steht.

### Option C — nur Base-Digest

- Pro: minimal.
- Contra: apt-Versionen driften weiter — löst das Kernproblem nicht.

## Konsequenzen

- Positiv: Zwei Builds mit gleichem Digest + Snapshot ergeben identische
  Paketversionen; Voraussetzung für den späteren Image-Hash-Vertrag (Modul 14).
- Negativ / Folgepflicht: **`lint` muss für clang-tidy 21 repariert
  werden** (Header-Resolution, z. B. passende `libstdc++`-Dev-Pakete oder
  `--extra-arg`/Compiler-Abgleich). Umsetzung inkl. 26.04-Migration in
  **slice-004**.
- Re-Evaluierung von **Option B** (vcpkg/Conan), sobald ein Binary-Cache
  verfügbar ist.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Reproduzierbarkeits-Check | zwei Läufe (gleicher Digest + Snapshot) → identische `dpkg-query`-Versionsliste; Beleg-Manifest `harness/toolchain-versions.txt` | `make versions` (real, slice-004) |
| Image-Hash | `harness/image-hash.txt` aus pinned Build (Modul 14) | `make fullbuild` (geplant) | <!-- d-check:ignore (geplant: entsteht mit make fullbuild, Modul 14) -->

## Re-Evaluierungs-Trigger

- Binary-Cache für Qt6/OCC verfügbar → Option B neu bewerten.
- Neues Ubuntu-LTS → Digest/Snapshot/Base bewusst hochsetzen.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-08 | Proposed (Ergebnis von spike-001) | spike-001 |
| 2026-06-09 | Accepted — umgesetzt in slice-004: 26.04@digest + node:24@digest + apt-Snapshot `20260601T000000Z`; lint-Blocker (clang-tidy 21) via `--gcc-install-dir` gelöst; `make versions` + Manifest belegen Reproduzierbarkeit | slice-004 |
