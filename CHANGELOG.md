# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Greenfield-Harness-Bootstrap nach AI-Harness-Kurs (Modul 2): stratifizierte Spec
  (Lastenheft/Spezifikation/Architektur), ADR-0001..0003, Roadmap, Planning-Lifecycle,
  `AGENTS.md`, `harness/` (README + Konventionen), Glossar, Releasing-Outline.
- `make docs-check` — Doku-Link-Validator als erstes reales Gate (vendored, MR-003).
- MIT-Lizenz.
- slice-001 — hexagonales C++-Build-Skelett: CMake-Target-Trennung Kern/Adapter
  (ADR-0001), Qt6/OpenCascade/SQLite-DevContainer, `make build` (Build + ctest).
- slice-002 — Code-Gates als Dockerfile-Target-Stages (keine Bind-Mounts):
  `make arch-check` (hexagonale Schichtung), `make lint` (clang-tidy +
  Suppression-Gate), `make test`, `make coverage-gate` (bootstrap-aware);
  Hello-Hexagon-Port-Roundtrip; Sensors gepromotet.
- spike-001 — Toolchain-Reproduzierbarkeit untersucht (Ubuntu 24.04 vs
  26.04, node-Base, Pinning) → ADR-0004 (Proposed: Digest + apt-Snapshot,
  Migration 26.04/node24) + Folge-slice-004.
- slice-005 — `make gate-consistency`: Doku↔Makefile-Sensor (jeder als
  real dokumentierte `make`-Befehl existiert) — schließt die Drift-Klasse,
  die `docs-check` (nur Links) nicht fängt.
- Harness-Hook-Härtung (MR-005, Rückport aus d-check): inhaltsbasierter
  Gate-Nachweis (Commit ohne Gate-Lauf wird vom Stop-Hook geblockt),
  Guard fail-closed + ohne Permission-Bypass + Sub-Shell-/Flag-Bündel-
  Rekursion, `record-gates` parallel-sicher im `gates`-Rezept.
- slice-012 — Eckenschluss endpunkt-verbundener Wände (LH-FA-WAL-006-
  Teilumfang, Lastenheft 0.1.2; Auslöser: ACC-002-Abnahme-Befund):
  Footprint-Hoheit im Kern (`wall_footprint`, Grad-2-Eckschnitt mit
  `WALL_MITER_LIMIT`-Begrenzung und Rückfall stumpf, total);
  `GeometryKernelPort` auf `extrudeFootprint`/`tessellateFootprint`;
  Mehr-Element-Update `WallGeometryChanged` (Nachbar-Rebuild,
  deterministische Reihenfolge, transaktional über den ganzen Satz);
  8 neue WAL-006-AK-Tests (Kern/Service + Viewer-Szene) inkl.
  Fehlerfall-Transaktion.
- slice-011b — sichtbarer 3D-Viewer (welle-1v, ACC-002 / sichtbare Hälfte
  LH-FA-D3-002): Qt-6-Widgets-Viewer mit orbitierbarer Perspektive;
  Tessellation über `ViewModelPort` (ADR-0009 (b), kein OCC in der GUI);
  Szenen-Surrogat + display-freie AK-Tests; arch-check **Regel E**
  (Qt nur `adapters/ui/` + Composition Root); `make acc-002-beleg`
  (manueller Abnahme-Schritt, kein Gate); ADR-0010 — Headless-GL via
  Xvfb/llvmpipe (offscreen-QPA trägt kein GL); Toolchain + `make
  versions`-Manifest um `xvfb` erweitert (ADR-0004).
- **Welle-1v-viewer abgeschlossen** (Closure 2026-06-13): die *sichtbare*
  Hälfte des Echtzeit-Vertrags steht — **ACC-002 erfüllt**, sichtbare
  Hälfte LH-FA-D3-002. slice-011a/011b + slice-012 in `done/`;
  unabhängige Welle-Verifikation (keine HIGH/MEDIUM), `make gates` grün
  am HEAD (63/63, Coverage 94,2 %). Ergebnisnotiz: `welle-1v-results.md`.
- slice-013a — Bauteil-Hosting & Wandöffnungs-Modell (welle-2-bauteile,
  erster Slice; reine ADR+Spec-Entscheidung, kein Code): **ADR-0011**
  accepted (Öffnung als wand-gehostetes Element; Kern liefert
  Schnitt-Prismen, `GeometryKernelPort` subtrahiert per OCC-Boolean;
  `WallGeometryChanged`-Wiederverwendung; Raumerkennung/Footprint
  unberührt; Bauteil-Erweiterungs-Muster als Welle-Leitplanke).
  Lastenheft 0.1.3: Türen (LH-FA-DOR-001..004) + Fenster
  (LH-FA-WIN-001..005) von Outline auf AK geschärft; Spezifikation §1
  Öffnungs-Algorithmus + §3 Tür-/Fenster-/Brüstungs-Wertebereiche. Keine
  Schema-Schärfung nötig (ADR-0006 trägt openings/doors/windows bereits).

### Notes
- Dieses CHANGELOG ist eine bewusste Abweichung von der Kurs-Baseline (die
  Änderungs-Historie verteilt auf Spec-§Historie, ADR-Geschichte, Slice-Closure-
  Notizen und Welle-Results führt). Begründung und Pflege-Regel: `harness/conventions.md` MR-004.
