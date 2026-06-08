---
id: slice-001
titel: Build-Skelett & DevContainer
status: done
welle: welle-1-mvp
lastenheft_refs: []
req_tec_refs: [REQ-TEC-001, REQ-TEC-004, REQ-TEC-009]
adr_refs: [ADR-0001, ADR-0002, ADR-0003]
---

# Slice 001: Build-Skelett & DevContainer

**Status:** done

**Welle:** welle-1-mvp

**Bezug:** ADR-0001 (hexagonale CMake-Target-Trennung), ADR-0002/0003 (Adapter-Deps), REQ-TEC-001/004/009

**Autor:** Dietmar Burkard. **Datum:** 2026-06-08.

---

## 1. Ziel

Das leere, aber korrekt getrennte hexagonale CMake-Skelett samt
reproduzierbarem Qt/OCC/SQLite-DevContainer herstellen — `make build`
baut grün, ohne dass schon Fachlogik existiert.

## 2. Definition of Done

- [x] Verzeichnis-Layout gemäß [`spec/architecture.md` §2.1](../../../../spec/architecture.md#21-verzeichnis--und-build-struktur) angelegt (`src/hexagon/*`, `src/adapters/*`, `tests/*`).
- [x] CMake-Targets `bcad_hexagon` (ohne externe Deps), `bcad_adapters` (Qt/OCC/SQLite), `b-cad`, `bcad_tests`; `make build` grün im DevContainer (verifiziert, inkl. `ctest` 1/1).
- [x] DevContainer-Dockerfile (Multi-Stage, Qt 6 + OpenCascade + SQLite) baut reproduzierbar (REQ-TEC-009).
- [x] Closure-Notiz mit Steering-Loop-Eintrag (siehe §7).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `CMakeLists.txt`, `src/CMakeLists.txt`, `src/*/CMakeLists.txt` | neu | Target-Trennung als Fitness Function (ADR-0001) |
| `.devcontainer/`, `Dockerfile` | neu | reproduzierbare Toolchain (REQ-TEC-009, Modul 14) |
| `Makefile` (`build`-Target) | update | `make build` ruft Container-Build |

## 4. Trigger

- Bootstrap abgeschlossen; erster Code-Slice der Welle 1.

## 5. Closure-Trigger

- DoD vollständig, `make build` grün im DevContainer, Closure-Notiz geschrieben.

## 6. Risiken und offene Punkte

- OpenCascade- + Qt-Build im Container groß/langsam → Multi-Stage + Layer-Caching (Modul 14).
- Distroless-Runtime mit Qt/OCC-Shared-Libs nicht-trivial — erst relevant für `fullbuild`/Release, nicht hier.

## 7. Closure-Notiz

**Abgeschlossen am:** 2026-06-08.

**Was funktioniert hat:** Die hexagonale Target-Trennung (ADR-0001) ist
doppelt belegt — der dependency-freie Kern `bcad_hexagon` kompiliert
mit reinem `g++` ohne Qt/OCC/SQLite (Host-Verifikation), und der
vollständige DevContainer-Build (`make build`) übersetzt die ganze
Kette inkl. `bcad_adapters` (Qt6/OCC/SQLite-Proben) und läuft `ctest`
1/1 grün. Image-Build erfolgreich.

**Was anders lief als geplant — Steering-Loop-Eintrag:** Der erste
`make build` brach mit `ninja: error: '/usr/lib/.../libtbb.so' …
missing` ab. Ursache: OpenCascades CMake-Target `TKernel` zieht
transitiv **TBB**, aber das Ubuntu-OCC-Paket installiert nur die
Runtime-`libtbb.so.12`, nicht den Dev-Symlink. **Sensor-Wirkung:** Der
neue `make build`-Gate hat genau diese Toolchain-Lücke gefangen, bevor
sie in einen späteren Slice geleckt wäre. **Härtung:** `libtbb-dev` in
die `deps`-Stage des DevContainers aufgenommen. → Generalisierbare
Lehre für ADR-0002-Folgearbeit: OCC-Adapter-Builds brauchen `libtbb-dev`
(Dev-Symlinks), nicht nur die Runtime-Libs.

**Folge-Slices:** keine neuen `open/`-Einträge nötig — slice-002
(Code-Gates) und slice-003 (Domain & Wände) standen bereits geplant.
`make build` wurde regelkonform aus dem „Nicht behauptet"-Block in die
reale Sensors-Tabelle promotet (Promotion-Trigger, Modul 13).

## 8. Sub-Area-Modus-Begründung

(Vier Pflichtkriterien je berührter Sub-Area; Modus-Quelle:
[`harness/conventions.md` §Modus-Deklaration](../../../../harness/conventions.md#modus-deklaration-pro-sub-area).)

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — CMake-Target-Trennung in ADR-0001 §CMake-Targets und `spec/architecture.md` §2.2 verankert; docs-check/Container-Konvention in MR-003.
- **Phase-Reife:** Phase 4 (Vertrag steht: Layout + Target-Trennung sind beschrieben, Code wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig — Doku führt, der Build entsteht erst.
- **Reconciliation-Aufwand:** keiner (GF); kein Folge-/Graduation-Slice nötig.
