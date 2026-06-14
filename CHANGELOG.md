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
- slice-013b — Türen + Fenster implementiert (LH-FA-DOR-001..004,
  LH-FA-WIN-001..005, ADR-0011): wand-gehostete `Opening` im Kern;
  automatische Wandöffnung als boolesche Subtraktion (`model::CutPrism`
  vom Kern, `GeometryKernelPort.extrudeFootprint`/`tessellateFootprint`
  um `cutouts` erweitert, `OccGeometryAdapter` via `BRepAlgoAPI_Cut`/
  `TKBO`, kein OCC-Leck); platzieren/verschieben/Parameter (Breite/Höhe/
  Brüstung/Anschlag)/entfernen mit Klemmung (E-VAL-001) + Platzierungs-
  Validierung; `WallGeometryChanged` der Wirtswand (total/transaktional),
  Raumerkennung/Footprint unberührt; 13 AK-Tests, Coverage 93,3 %.
  Öffnungen vorerst nur im Speicher — Persistenz folgt in slice-013c
  (vor erstem Save-Use-Case).
- slice-013c — Öffnungs-Persistenz (LH-FA-DOR-001/WIN-001, LH-FA-BLD-002/003,
  ADR-0011/0006): `SqliteProjectRepository` speichert/lädt Türen + Fenster
  über `openings` + `doors`/`windows`-CTI (in derselben atomaren
  Transaktion nach den Wänden, FK `wall_id`); Round-Trip-AK-Test. Kein
  Schema-Wechsel (ADR-0006 trug die Tabellen bereits). Damit ist ADR-0011
  vollständig umgesetzt (Geometrie 013b + Persistenz 013c).
- slice-014a — Dach von Outline auf AK geschärft (LH-FA-ROF-001..005,
  Lastenheft 0.1.4): Sattel-/Walm-/Pultdach, Neigung 5–60°, Überstand
  0–1500 mm; **Teilumfang rechteckiger Grundriss** (komplexe Polygone
  offen). Spezifikation §1 Dach-Geometrie (Traufrechteck, Konstruktion
  je Typ, Höhenformeln) + §3 Neigungs-/Überstands-Bereiche + Defaults.
  Reine Entscheidung/Spec (kein Code, kein ADR — ADR-0011-Leitplanke).
- slice-014b — Dach implementiert (LH-FA-ROF-001..005, ADR-0011):
  `model::Roof` + `roof_geometry` (analytisches Dach-Netz Sattel/Walm/Pult
  im Kern, kein OCC); `ViewModelPort.roofMeshes`, `EditStructurePort`
  add/setPitch/setOverhang/setType/remove (Klemmung); `RoofChanged`-`op`
  (kein `RoomsChanged`); `ViewerScene` folgt idempotent. 9 AK-Tests; kein
  ADR (ADR-0011-Leitplanke). Persistenz folgt in slice-014c. make gates
  grün (92 Tests, Coverage 91,8 %).
- slice-014c — Dach-Persistenz (LH-FA-ROF-001, LH-FA-BLD-002/003,
  ADR-0011/0006): `SqliteProjectRepository` speichert/lädt Dächer über
  die `roofs`-Tabelle (`roof_type`/`pitch`/`overhang` als Spalten,
  rechteckiger Grundriss als `footprint_json`-Array `[ox,oy,w,d,base_z]`,
  `%.17g`; erste JSON-Ser/De im Repo, gekapselt + total). In derselben
  atomaren Transaktion nach den Geschossen; Round-Trip-AK (nicht-glatter
  double prüft `%.17g`). Kein Schema-Wechsel. Dach-Familie (014a/b/c)
  damit komplett. make gates grün (95 Tests).
- slice-015a — Decken + Fundament von Outline auf AK geschärft
  (LH-FA-SLB-001..003, LH-FA-FND-001..003, Lastenheft 0.1.5): horizontale
  Platten, Deckendicke 100–500 mm, Ausschnitte, Fundamenttiefe
  200–2000 mm, Bodenplatte. Spezifikation §1 Platten-Geometrie (Polygon ×
  Dicke an base_z je slab_type; Ausschnitte als Boolean/`CutPrism`;
  base_z-/Port-Frage an 015b) + §3 Dicke-Bereiche. Reine Entscheidung/Spec
  (kein Code, kein ADR — ADR-0011-Leitplanke; MR-008 lösungsfrei).
- slice-015b — Decken + Fundament implementiert (LH-FA-SLB-001..003,
  LH-FA-FND-001..003, ADR-0011): `model::Slab` (Decke/Fundament/
  Bodenplatte) + pure `slab_geometry` (`slabBaseZ`, `slabCutPrisms`,
  `translateMeshZ`). **base_z ohne Port-Wechsel** (015a-HIGH-1): Kern
  extrudiert/tesselliert unverändert (Volumen z-invariant) und verschiebt
  das Netz auf base_z; Cutout-`CutPrism` z **relativ** `[−ε,Dicke+ε]`,
  Translation **nach** Boolean. `ViewModelPort.slabMeshes()`,
  `EditStructurePort` addSlab/setSlabThickness/addSlabCutout/removeSlab
  (Dicke typabhängig geklemmt), `SlabChanged`-`op` (storey-bezogen);
  `ViewerScene` über gemeinsamen `reloadKeyed`-Helfer (Roof+Slab).
  Spec §1 base_z-Frage geschlossen. **Code-Review danach: 1 HIGH trotz
  grüner Gates** (OCC-Cutout-Boolean ungetestet, `addSlabCutout` setzte die
  Spec-Begrenzung „auf den Platten-Umriss" nicht durch). Nachschärfung:
  `cutoutInsideSlab` lehnt rand-/außenliegende/degenerierte/nicht-endliche
  Ausschnitte ab (innenliegende Löcher sind koplanar-frei, kein lateraler
  Überstand nötig); OCC-Boolean-Naht jetzt getestet. make gates grün
  (103 Tests, Coverage 91,9 %).
- slice-015c — Decken/Fundament-Persistenz (LH-FA-SLB-001/003, LH-FA-FND-001,
  LH-FA-BLD-002/003, ADR-0011/0006): `SqliteProjectRepository` speichert/lädt
  Platten über die `slabs`-Tabelle (`slab_type`/`thickness_mm` als Spalten,
  Grundriss **und Aussparungen** als verschachteltes `polygon_json`
  `[[footprint],[cutout]…]`, `%.17g`; generalisiert das 014c-Flach-Array zu
  Ringen, `std::string`-Builder + balancierter Sub-Array-Scan, total/`E-IO`).
  In derselben atomaren Transaktion nach den Geschossen (FK `storey_id`);
  `slab_type`-Mapper total (Schema ohne CHECK). Cutouts round-trippen
  feldgleich (Domänenfeld, kein NULL); `material_id`/`base_z` bewusst nicht
  persistiert (welle-2-Scope/abgeleitet). Round-Trip-AK (Decke mit Ausschnitt
  + nicht-glatter double; Fundament Ein-Ring; Bodenplatte) + Leer-Pfad- und
  Negativ-Parse-Regression. **Unabhängiger Code-Review danach** (keine HIGH;
  3 MEDIUM gefixt: `stod`-Totalität — Müll-Suffix wurde still verschluckt —,
  Negativ-Parse-Test, Bodenplatte-Mapper-Zweig). Kein Schema-Wechsel.
  **Platten-Familie (015a/b/c) komplett.** make gates grün (105 Tests,
  Coverage 91,9 %).
- slice-016a — Treppen von Outline auf AK geschärft (LH-FA-STR-001..004,
  Lastenheft 0.1.6): gerade einläufige Treppe verbindet zwei Geschosse,
  Stufenanzahl 2–30, Laufbreite 800–2000 mm, immer sichtbares Geländer;
  **Teilumfang gerade einläufig** (Podest/Wendel/Mehrlauf offen). Spezifikation
  §1 `LH-FA-STR-001.a` (analytisches Stufen-Quader-Polyeder im Kern,
  `rise = Geschosshöhe/step_count` abgeleitet, feste +x-Aufstiegsrichtung,
  Geländer als generierte Geometrie ohne Schema-Spalte, `StairChanged` an
  `from_storey` gebunden) + §3 Stair-Wertebereiche. Reine Entscheidung/Spec
  (kein Code, kein ADR — ADR-0011-Leitplanke; MR-008 lösungsfrei). MR-006:
  keine HIGH (3 MED/2 LOW eingearbeitet). Nebenbei: Lastenheft-Header-Versions-
  Drift behoben (Header hing seit slice-012 auf 0.1.2, Historie war bei 0.1.5).
  make gates grün.

### Notes
- Dieses CHANGELOG ist eine bewusste Abweichung von der Kurs-Baseline (die
  Änderungs-Historie verteilt auf Spec-§Historie, ADR-Geschichte, Slice-Closure-
  Notizen und Welle-Results führt). Begründung und Pflege-Regel: `harness/conventions.md` MR-004.
