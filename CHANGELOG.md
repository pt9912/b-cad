# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed
- slice-042c — **Export-Refactor STEP/STL: Body-Migration auf das `DerivedGeometry`-Bündel** (welle-5;
  dritte von fünf ADR-0020-Folgepflichten; **intern, verhaltens-invariant**). Der `ExchangeService` berechnet
  die 3D-Bauteil-Ableitung (pre-OCC-Primitive) format-selektiv für STEP/STL und reicht sie im Bündel; die
  Export-Adapter **iterieren `derived.*` + serialisieren** (STEP baut B-Rep via `occ_solids`, STL tesselliert
  über den `GeometryKernelPort`) — die reine Ableitung ist kern-total, der fail-closed-Skip degenerierter
  Bauteile bleibt adapter-resident (OCC, Regel C). Die Geschoss-Höhen-Auflösung (`storeyHeight`) liegt als **eine**
  Wahrheit im Service (die zwei Adapter-Kopien entfielen). **Die Schicht-Kante `geometry → services_geo` ist aus
  `.a-check.yml` + `architecture.md` §2 entfernt** (kein Geometrie-Adapter ruft mehr `services/geometry`;
  Verschärfung). Die STEP-B-Rep-`CLOSED_SHELL`-Zählung + das binäre STL-Netz-Orakel bleiben **unverändert grün**
  (Invarianz-Beweis); neu: ein **Voll-Modell-Integrationstest** über den echten Service (Wände+Decke+Dach+Treppe),
  eine STL-`baseZ`-Koordinaten-Sonde und ein Totalitäts-Test (danglendes Geschoss + degeneriertes Bauteil). 252
  Tests, Coverage 91,5 %. **MR-006 0 HIGH; MR-009 Code-Review 0 HIGH** (Byte-Identität Zeile-für-Zeile verifiziert).
  `persistence → services_geo` bleibt (Folge-Slice 042d).
- slice-042b — **Export-Refactor 2D-Projektion: `plan_geometry` in den Kern + `PlanViewPort`** (welle-5;
  zweite von fünf ADR-0020-Folgepflichten; zugleich ADR-0019-Lese-Naht-Refactor; **intern, verhaltens-
  invariant**). Die reine 2D-Grundriss-Projektion (`projectPlan`/`PlanView`) ist aus dem io-Adapter in den
  Kern gehoben (`hexagon/services/geometry/plan_projection` bzw. `hexagon/model/plan_view.h`); der neue
  Driving-Read-Port **`PlanViewPort`** (2D-Analog zum `ViewModelPort`, vom `StructureEditService` implementiert)
  liefert sie — **entsperrt** den DRW-Canvas (Konsument = späterer Canvas-Slice). PDF/PNG beziehen die
  `PlanView` kern-berechnet im `DerivedGeometry`-Bündel (`ExchangeService` befüllt sie format-selektiv, Writer
  serialisieren nur); **DXF unberührt** (`visibleLayerIds` als reiner Filter nach `hexagon/model/`, geteilt von
  Kern-Projektion und DXF — **keine** `io → services_geo`-Kante). `io/plan_geometry` entfernt (Tombstone in
  `.d-check.yml` für die historische Referenz der immutablen ADR-0018 + slice-032c). Export-Decode-Orakel
  (DXF/PDF/PNG) **unverändert grün** = Invarianz-Beweis; 249 Tests, Coverage 90,7 %. **MR-006 0 HIGH; MR-009
  Code-Review (042a+042b) 0 HIGH** (Byte-Identität Zeile-für-Zeile verifiziert). Keine `.a-check.yml`-Kante
  entfernt (Folge-Slices 042c–e).
- slice-042a — **Export-Refactor Kern-Naht: `DerivedGeometry`-Vertrag** (welle-5; erste von fünf
  ADR-0020-Folgepflichten; **intern, verhaltens-invariant**). Der `ModelExporterPort::write`-Vertrag
  trägt jetzt ein kern-berechnetes `DerivedGeometry`-Bündel (pre-OCC-Primitive je Bauteil, format-selektiv)
  — driven Export-Adapter serialisieren nur, sie leiten keine Geometrie ab (ADR-0020). `StepBox` +
  `translateMeshZ` nach `model/`-Kern gehoben (header-only, lib-frei); alle 6 Exporter nehmen das Bündel
  **accept-and-ignore** an; der `ExchangeService` reicht (vorerst) ein **leeres** Bündel — die
  format-selektive Berechnung + der STEP/STL-Konsum folgen mit slice-042c (MR-006-MED-1). **Keine**
  `.a-check.yml`-Kante entfernt (Folge-Slices 042c–e). Export-Decode-/Round-Trip-Orakel (STEP/STL/IFC/DXF/
  PDF/PNG) **unverändert grün** = Invarianz-Beweis; 248 Tests, Coverage 90,7 %. MR-006 0 HIGH/1 MED/2 LOW.

### Added
- slice-041a — **DRW interaktiver 2D-Canvas: Grundsatz-ADRs accepted + interaktive Erzeugungs-AK**
  (welle-5 DRW-Interaktiv-Strang; ADR-0019/ADR-0020; reine Doku/Entscheidung, kein Code). **ADR-0020**
  (driven Adapter serialisieren, der Kern liefert abgeleitete Geometrie als `DerivedGeometry`-Bündel;
  alle Adapter→Kernel-`.a-check.yml`-Kanten entfallen; `architecture.md` §2/§1 werden wahr) und
  **ADR-0019** (2D-Canvas: neues `view/`-Widget, 2D-Lese-Naht = Projektion in den Kern + neuer
  `PlanViewPort`, `screenToModel`, Selbst-Refresh ohne `op`, Kommando via `command/`, Fang/Raster =
  UI-Aids) beide **Accepted** (je unabhängiges Text-Review 0 HIGH). ADR-0019 wurde **auf ADR-0020 neu
  geschnitten** → die 2D-Projektion wird kern-geliefert, **keine** `io → services_geo`-Kante (die
  frühere Kanten-Autorisierung entfällt, §2.6 n/a). `LH-FA-DRW-005` um die **interaktive Erzeugungs-AK**
  ergänzt (Nutzer zieht eine Hilfslinie mit der Maus auf einer 2D-Zeichenfläche → erscheint sofort,
  ebenen-zugeordnet, überlebt Speichern/Laden + Export; Boundary Anfang=Ende → keine Hilfslinie;
  Negative ohne sichtbare/aktive Ebene → nichts bzw. Verweis; Lastenheft **0.1.15**). Spec §1
  `LH-FA-DRW-005.a` Canvas-Naht-Mapping (7 Entscheidungen) + §6 Grenzzeile; `architecture.md` §1.1
  (`PlanViewPort` + `EditDrawingPort`-Canvas-Klausel) + Provenance. MR-006 0 HIGH; ADR-Text-Reviews 0 HIGH.
- slice-032c — **DRW-Export: Hilfslinien im 2D-Grundriss sichtbar** (welle-5 DRW-Strang; ADR-0018;
  schließt die DRW-Fundament-Sequenz 032a→b→c). Der 2D-Grundriss-Export zeichnet jede Hilfslinie auf
  **sichtbarer** Ebene (DXF `LINE` auf dem Geschoss-`LAYER`, PDF/PNG als 2D-Segment); eine **unsichtbare**
  Ebene wirkt als **Export-Filter** (Hilfslinie fehlt im Artefakt). Der Sichtbarkeits-Filter ist eine
  geteilte format-agnostische Quelle (`visibleLayerIds` in `plan_geometry`; PDF/PNG via `projectPlan`
  inkl. Bounding-Box-Erweiterung, DXF via denselben Filter) — kein Format-Drift; der Geschoss-`LAYER`
  bleibt unverändert. Decode-Orakel je Format (Erscheint/Fehlt: DXF roher Reader, PDF ` l\n`-Zählung,
  PNG Tinte). CLI-Demo trägt eine Hilfslinie (io-smoke-Übung; ACC-002-3D-Beleg unberührt). Damit sind
  die Export-Teilklauseln von LH-FA-DRW-005 (Happy „erscheint im Artefakt" / Negative) benutzer-
  beobachtbar. MR-006 0 HIGH.
- slice-032b — **DRW-Impl: 2D-Zeichen-Daten durabel** (welle-5 DRW-Strang, erster Code-Slice; ADR-0018).
  Kern-Werttypen `model::Layer`/`GuideLine` auf `Building`; neuer Driving-Port `EditDrawingPort`
  (implementiert vom bestehenden `StructureEditService` — eigener Port, geteiltes Objekt, Muster
  `EvaluatePort`) für Ebenen (anlegen/umbenennen/sichtbar/löschen) + Hilfslinien (anlegen/löschen).
  Ablehnungen als E-VAL-001-Rejection (leerer/projekt-doppelter Ebenen-Name, entartete Hilfslinie,
  unbekannte Ebene/Geschoss, `restrict`-Löschschutz referenzierter Ebene), **kein** neuer `op`. Neue
  `guide_lines`-Tabelle (FK `layer_id` `restrict`) + `layers`-Aktivierung; `schema.sql` via neuem
  **`make schema-regen`**-Target regeneriert (`schema-check` grün); SQLite-Persistenz-Round-Trip
  (Ebenen vor Hilfslinien, ID-/feld-erhaltend). AK-Tests Kern + Persistenz. Export-Sichtbarkeit folgt
  in 032c. MR-006 0 HIGH (L1/L2/INFO-1 eingearbeitet; `make schema-regen` = L2).

### Changed
- slice-032a — **DRW-Fundament: 2D-Zeichen-Daten AK-Schärfung** (welle-5 DRW-Strang eröffnet;
  ADR-0018 Accepted). LH-FA-DRW-005/006 (Hilfslinien + Ebenen) Outline → AK (Lastenheft 0.1.14;
  **Teilumfang-/Ehrlichkeits-Klausel** Canvas-los im AK-Körper); Spezifikation §1-Sammelblock
  LH-FA-DRW-005.a + §2.2 welle-5-Tabellen-Prosa + §4 E-VAL-001-Ablehnungs-Lesart + §6 interne
  Grenz-Vertragszeile + Subset-Grenze DXF/PDF/PNG autorisiert um Hilfslinien; data-model.yaml-
  Kommentar-Heilung; architecture.md §1.1 EditDrawingPort. Reine Doku, kein Code (Impl = 032b,
  Export = 032c). MR-006 0 HIGH (2 MED/4 LOW eingearbeitet; MED-1 Beobachtbarkeit tragfähig ohne Canvas).
- slice-038 — **Harness-Gate-Fahrplan-Abschluss** (harness-steering-Quergewerk; kein Gate, kein ADR; MR-019).
  `make doc-trace` (`--trace`, Requirements-Traceability-Matrix) als **advisory Report** eingeordnet — ein
  Live-Korpus-Teilausschnitt (nur `###`-Anforderungen + glatte `slice-\d{3}`-Ids; b-cads Buchstaben-Sub-Slices
  unsichtbar → die 44 „WAISEN" sind überwiegend Nicht-Defekte, **nicht** Freeze-Folge). **`--require-complete`
  verschoben** auf den Vollständigkeits-Meilenstein (Voll-Spec-Backlog; nicht verworfen). Damit die
  d-check-Modul-Adoptions-Linie (033–037 Gates + Report) **abgeschlossen**. MR-006 1 HIGH behoben
  (Ursachen-Umbasierung Freeze→Suffix-Blindheit).
- slice-037 — **Fresh-Clone-Gate (d-check-Modul `tracked`)** (harness-steering-Quergewerk; kein ADR —
  Verschärfung, §2.6 n/a; MR-018). Eine **neue** Referenz-Integritäts-Regel wird **computational**: jedes
  auflösbare, existierende Link-/Bild-Ziel ist im **git-Index getrackt** (`target-untracked` — untrackte
  Ziele fehlen auf jedem frischen Klon). **Range-frei → `make gates`-Member** (ins `.d-check.yml`-`modules:`-
  Set; erster `.git`-lesender Member; CI-vakuum-grün → Wert nur lokal auf dem schmutzigen Baum).
  `exempt-targets: []`. Evidence-first: Baseline 0, Positiv `target-untracked`, kein Doppelbefund
  (fehlend bleibt `links`). MR-006 **0 HIGH / 1 MED** (Kriterium range-frei statt git-frei) + 3 LOW.
- slice-036 — **Planning-Lifecycle-Gate (d-check-Modul `planning`)** (harness-steering-Quergewerk;
  kein ADR — Verschärfung, §2.6 n/a; MR-017). Eine **neue** Lifecycle-Invariante wird **computational**:
  der Ruhe-Sentinel `Keine offenen Slices` im Roadmap-`## Aktuelle Welle`-Block steht genau dann, wenn
  kein `slice-*` in `in-progress/` liegt (`planning-drift`). **Hermetisch → erster über den Harness-Gate-
  Fahrplan adoptierter `make gates`-Member** (ins `.d-check.yml`-`modules:`-Set, **nicht** CI-only wie
  commits/vcs). Ruhe-Marker-Toggle reitet im `git mv`-Commit (AGENTS §2.8 gewahrt + dort referenziert).
  Evidence-first: Baseline 0, volle Wahrheitstafel (beide iff-Richtungen). MR-006 **0 HIGH / 2 MED**
  (M1/M2) + 3 LOW eingearbeitet.
- slice-035 — **ADR-Immutabilitäts-Gate (d-check-Modul `vcs`)** (harness-steering-Quergewerk; kein
  ADR — Verschärfung, §2.6 n/a; MR-016). AGENTS §2.5 (Accepted-ADRs immutabel) wird **computational**:
  `make doc-immutable` (git-Diff des ADR-**Core** über eine Range, d-check-Modul `vcs`, `core-drift-vcs`;
  `head-allow` erlaubt `Superseded by ADR-NNNN`) — **CI-only-Sensor** (Muster `doc-commits`), **nicht**
  in `make gates`. Die `## Geschichte`-Provenance bleibt mutabel. Evidence-first: Positiv `core-drift-vcs`,
  Negativ (Geschichte) 0, Voll-Historie (315 Commits) 0. MR-006 **1 HIGH behoben** (zweite §2.5-Fundstelle
  in `docs/plan/adr/README.md` §Konventionen — dieselbe Lehre wie slice-034) + M1/L1/L2 eingearbeitet.
- slice-034 — **Commit-Traceability-Gate (d-check-Modul `commits`)** (harness-steering-Quergewerk;
  kein ADR — Verschärfung/Prozess-Gate, §2.6 n/a; MR-015). Die AGENTS-§4-Traceability-Regel wird
  **computational**: `make doc-commits` (git-Range, d-check-Modul `commits`, `id-patterns`
  `slice-\d{3}`/`ADR`/`MR`/`LH`, `exempt-pattern` Merge/Revert) prüft, dass jede Commit-Message
  einer Range eine Kennung trägt — **CI-only-Sensor** (Muster `schema-check`/`io-smoke`), **nicht**
  in `make gates` (git-abhängig, nicht-hermetisch). Evidence-first: 0 kennungslose Commits in
  `HEAD~30..HEAD`. **Beide** Regel-Fundstellen (AGENTS §4 **+** harness/README §Traceability rules)
  symmetrisch um `slice-*`/`MR-*` ergänzt (kein Regel↔Gate-Drift). MR-006 **1 HIGH behoben**
  (zweite §4-Fundstelle) + 2 MED/2 LOW eingearbeitet. `make gates`/`make docs-check` grün.
- slice-033 — **d-check v0.37.1 + Matrix-Verschärfung `adr→slice`** (harness-steering-Quergewerk;
  kein ADR — Verschärfung, §2.6 n/a; MR-014). **docs-check** ist von der `tools/Dockerfile`-FROM-Stage
  auf **`d-check.mk`** umgestellt (`include`, Bind-Mount, `DCHECK_DIGEST` **v0.37.1**; Muster
  `a-check.mk`/slice-030; `tools/Dockerfile` entfernt, codepaths-`ignore-refs`-Tombstone;
  Pin-Sprung-Fallout = **0** gemessen) und das `matrix`-Modul um die Kante **`adr → slice`**
  erweitert: ein ADR-Körper nennt keine Slice-Kennung (Stable Dependencies / no-downward; Folgepflicht-
  Zuordnung lebt im mutablen ADR-Index). Umsetzung: `token: slice-\d{3}` (Klartext-Token) +
  Grandfathering der immutablen ADRs 0001–0017 + `<!-- d-check:status-provenance -->`-Ausnahmemarker.
  Die **30** vorbestehenden Spec-Straten-Slice-Tokens sind remediert (Reifephase-Provenance markiert,
  substanzielle Fachverweise selbsttragend umformuliert, `architecture.md` slice-frei). MR-006 **0 HIGH**
  (3 MED eingearbeitet); `make docs-check`/`make gates` grün.
- slice-031 — **lint-Härtung II: `misc-const-correctness` + `modernize-use-nodiscard`
  scharfgeschaltet** (harness-steering-Quergewerk, Fortsetzung slice-027; kein ADR —
  Verschärfung, §2.6 n/a). Die zwei in slice-027 als wertvolle Folge-Kandidaten
  zurückgestellten Checks sind aus der `.clang-tidy`-Exclude-Liste (Checks +
  WarningsAsErrors) entfernt und ihre **23 Befunde verhaltensneutral behandelt**
  (kein NOLINT): **const-correctness (13)** = `const`-Qualifikation lokaler Variablen
  (OCC-Builder, `std::ifstream`, `model::Opening&`→`const&` im Kern, Composition-Root
  in `main.cpp`); **use-nodiscard (10)** = `[[nodiscard]]` an const-Accessoren
  (`room_detection`, `ifc_spf_reader`, `sqlite` `handle`/`get`, Plugin-`name()`),
  plus die Basis `plugin_api/plugin.h` `name()` für Override-Konsistenz (ABI-neutral).
  Dry-Run-Beleg `docs/reviews/2026-07-04-slice-031-dryrun.md`; MR-006 (zwei
  unabhängige Agenten) 0 HIGH / 2 MED / 2 LOW (eingearbeitet). `readability-inconsistent-declaration-parameter-name`
  (19, kosmetisch) bleibt zurückgestellt. `make gates` grün, `make test` 228/228 unverändert.
- slice-027 — **lint-Härtung: kuratierte clang-tidy-Familien-Erweiterung (evidence-first)**
  (harness-steering-Quergewerk; kein ADR — Verschärfung, §2.6 n/a). `make lint` aktiviert
  zusätzlich zum Bestand (bugprone-*/clang-analyzer-*/cognitive-complexity) die 7 Familien
  **cert-* / readability-* / performance-* / modernize-* / misc-* / cppcoreguidelines-* /
  portability-***. Vorgehen: Dry-Run aller Familien im gepinnten Container (clang-tidy 21.1.8)
  → **Befund-Matrix (1630 Befunde**, Beleg `docs/reviews/2026-07-04-slice-027-dryrun.md`),
  dann kuratierte Aktivierung. **8 Checks aktiviert + 10 Befunde verhaltensneutral gefixt**
  (convert-member-functions-to-static, special-member-functions [Rule-of-5 `Db`/`Stmt`],
  use-anyofallof, simplify-boolean-expr, container-data-pointer, unnecessary-copy-initialization,
  use-std-numbers [`std::numbers::pi`], rvalue-reference-param-not-moved → `const&`). Durch
  Familien-Aktivierung mit Exclude-Liste sind **alle 0-Befund-Checks scharf**
  (Falsifizierbarkeits-Invariante). **24 Auslassungen begründet** im `.clang-tidy`-Kommentar:
  Alias-Dedup (Magic-Numbers); Idiom-Kollisionen (reinterpret-cast [SQLite-C-API], vararg
  [snprintf-Codecs], owning-memory [Plugin-ABI], cert-err33-c [sichere snprintf],
  no-recursion [SPF-Parser]); Wert-Structs; Bounds-false-positives; Groß-/Stil per
  Umfangs-Deckel (identifier-length 544 u. a.). Zurückgestellte wertvolle Folge-Kandidaten:
  const-correctness (14), nodiscard (10), inconsistent-parameter-name (19). `make gates`
  grün, `make test` 228/228 unverändert; keine Suppression (AGENTS.md §2.4).
- slice-030 — **Architektur-Gate umgestellt: a-check (externes digest-gepinntes Image)
  übernimmt A–E + Schicht-Kanten + driving/driven; `tools/arch-check.sh` → P-Rest**
  (harness-steering-Quergewerk, MR-013 nach Muster MR-007 = docs-check→d-check;
  **keine ADR** — keine §2.6-Lockerung, a-check ist strenger). b-cad ist **erster
  a-check-Pilot-Konsument** (a-check-Meilenstein M3). Das primäre Architektur-Gate
  läuft jetzt über **a-check v0.9.0** (`ghcr.io/pt9912/a-check@sha256:0378211f…`,
  netzlos `--network none`, read-only Bind-Mount) mit deklarativer `.a-check.yml`
  (Vollrichtung: Kern-Reinheit A, laterale Adapter B inkl. `adapter_sink`-MeshSource-Naht,
  Tech-Kapselung OCC/SQLite/Qt/`dlfcn.h`-Include C/D/E, **neu** Schicht-Kanten +
  driving/driven-Richtung) + `a-check.mk` (digest-gepinnt, `include` im `Makefile`,
  `make a-check` als `gates`-Member). `tools/arch-check.sh` behält nur den **P-Rest**,
  den der kanten-basierte a-check strukturell nicht sieht: das `dlopen`/`dlsym`/`dlclose`-
  **Funktionsaufruf**-Muster (P1-Aufruf; der `dlfcn.h`-Include liegt bei a-check) + die
  feine Quote-vs-Angle-Import-Allowlist für `plugins/`+`src/plugin_api/` (P2). Regel
  A/B/C/D/E + P2b (Qt/OCC/SQLite im `plugins/`-Baum) aus dem Skript **entfernt**
  (a-check übernimmt, empirisch belegt). `tools/gate-consistency.sh` scannt zusätzlich
  `a-check.mk` (Include-Awareness). **MR-006** 0 HIGH / 3 MED / 1 LOW (alle
  eingearbeitet: Quell-/Test-Kommentar-Nachzug ~22 A–E-Zuschreibungen → a-check
  [P1/P2 bleiben arch-check]; Makefile-Kopf-Bind-Mount-Ausnahme; Plan-Datei-ID-Hygiene).
  Verifikation: `make a-check` 0 Befunde/Exit 0; Gegenprobe a `QWidget`→`core-impurity`/Exit 1;
  Gegenprobe b `dlopen(`-Aufruf außerhalb Host → `arch-check.sh` feuert; `make gates` grün.

### Added
- slice-028 + slice-029 — **a-check-Vorbereitung: Architektur-Struktur richtungs-rein**
  (harness-steering-Quergewerk; Pilot-Befunde des a-check-Schwester-Tools v0.8.0,
  Projektinhaber-Abnahme der Entwürfe vor Umsetzung): **028** reklassifiziert die fünf
  reinen Berechnungs-Kerne (`opening/roof/slab/stair_geometry`, `wall_footprint`) per
  `git mv` nach `src/hexagon/services/geometry/` (Option B: `model/` bleibt header-only
  [ADR-0017-Symbol-Naht-Prämisse], §2.1-Konvention unberührt, Kante glob-deklarierbar;
  `architecture.md`-Tabelle heilt die vorbestehende Verletzung durch die 11
  Adapter-Includes). **029** trennt den ui-Misch-Adapter verzeichnislich:
  `ui/view/` = driven (Beobachter-Implementierungen + Rendering), `ui/command/` =
  driving — mit port-freier **`MeshSource`-Naht** durch das ADR-0008-Push-Notify/
  Pull-State-Muster (Aggregate als `std::map<Id,TriangleMesh>`, Konversion in
  `ViewModelMeshSource`); Regel E unangetastet. Beleg je Slice: kein
  `src/adapters/**`-Include auf `hexagon/services/*` außerhalb `geometry/`; kein
  Adapter-Verzeichnis mischt driving/driven. Je Zwei-Commit-Split (`MR-006`-Review-
  Auflage: §2.8 gilt allgemein), je `MR-006` (028: 1 HIGH + 2 MED eingearbeitet;
  029: 3 MED eingearbeitet) + unabhängiges **Code-Review 0 HIGH / 0 MED**;
  `make gates` durchgehend grün (228/228, arch-check A–E/P1/P2). Verhaltens-neutral
  (Orakel: unveränderte AK-/Invarianten-/Surrogat-Tests).
- slice-026b — **Plugin-System lauffähig: Host, API, Beispiel-Plugin, Regel P**
  (`LH-FA-PLG-001`..004, welle-5-erweiterung, ADR-0017): b-cad lädt/entlädt Shared-Library-
  Plugins **zur Laufzeit** über den **Plugin-Host** (Driving Adapter `src/adapters/plugin/`,
  dlfcn-Monopol): versionierter `extern-"C"`-Handshake **exakt/fail-closed** vor jedem
  C++-Kontakt (Ablehnung ohne Init nennt erwarteten+vorgefundenen Vertragsstand),
  7-Stufen-Lifecycle, **Fehler-Barriere** je Host→Plugin-Übergang (`E-PLG-001`,
  `plugin_rejected`/`plugin_error`), Fehlerpfad = **Isolieren ohne Entladen**.
  **Plugin-API** `src/plugin_api/` (header-only; invalidierbarer `PluginContext` mit
  Port-Subset v1 = `EditStructurePort`+`EvaluatePort` — dieselbe Klemmung/Validierung wie
  manuelle Eingaben, kein Nebeneingang). `plugins/`-Baum: Beispiel-Plugin (Edit+Klemm-Beleg)
  + 4 Test-Fixtures als CMake-`MODULE` **ohne Kern-Linkage**; **Symbol-Naht =
  `ENABLE_EXPORTS`** (Kern bleibt statisch; Beleg: Exception aus realer `.so` im Host
  gefangen). `--plugin`-CLI. **arch-check-Regel P** (P1 dlfcn-Monopol inkl. `plugins/`;
  P2 Import-Grenze inkl. Angle-Include-Verbot) + lint-Scope um `plugins/` (Verschärfung);
  Gate-Doku (AGENTS §3/harness-README) nachgezogen. 8 AK-Tests mit **realer `.so` durch den
  echten Host**. `MR-006` 0 HIGH (1 MED + 3 LOW eingearbeitet) + unabhängiges
  **Code-Review 0 HIGH** (3 MED + 2 LOW vor Closure behoben, u. a. Shutdown-Wurf-Test +
  P2-Angle-Härtung). `make gates` grün (228/228, Coverage 90,3 %); **keine neue Dependency**
  (dlopen = glibc). Beide ADR-0017-Folgepflichten (Impl, Regel P) erfüllt →
  **OBJ-004-Pfad für M5 frei** (GUI-Plugin-Verwaltung = benannte Lücke, UI-Strang).
- slice-026a — **Plugin-System AK-geschärft + Spec-Mapping entschieden** (`LH-FA-PLG-001`..004,
  welle-5-erweiterung, ADR-0017; reine Doku/Entscheidung, kein Code): Lastenheft **0.1.13** —
  die vier Plugin-Anforderungen von Outline auf **lösungsfreie, benutzer-beobachtbare AK**
  (Laden/Entladen zur Laufzeit ohne Neustart; unpassender versionierter Vertragsstand →
  Ablehnung **vor jeder Wirkung**; Lifecycle beobachtbar, jeder Fehlerpfad → Plugin wirkt
  nicht weiter; Sandbox: wohlgeformtes Fehlverhalten → isoliert/entladen bei unverändertem
  Modell, gleiche Prüf-/Klemm-Regeln, **Ehrlichkeits-Klausel** Crash-Recovery `LH-QA-005`
  **oder** Silent-Corruption ohne Schutz). Spezifikation: **§1** `LH-FA-PLG-001.a`-Sammelblock
  (Plugin-Host als Driving Adapter, versionierter Handshake exakt/fail-closed, 7-stufiger
  Lifecycle, Port-Vermittlung pull-only ohne Beobachter-Zugang, Threading synchron im
  Hauptthread, Fehler-Barriere, Sandbox-Grenze ehrlich benannt), **§4** `E-PLG-001` = **ein
  Code, zwei Log-Events** (`plugin_rejected` Load/Handshake vs. `plugin_error` Laufzeit),
  **§5** Span `bcad.plugin.lifecycle`, **§6** Plugin-API-Vertragszeile. `.d-check.yml`
  ids-Familie um `PLG` erweitert (Verschärfung). `MR-006`-Plan-Review **0 HIGH** (1 MED +
  3 LOW + 3 INFO, alle eingearbeitet). ADR-0017-Folgepflicht „AK-Schärfung + Spec-Nachzug"
  erfüllt → PLG-Impl-Slice (026b) startbar.
- **welle-4-austausch abgeschlossen — Meilenstein M4 „Offen austauschbar" erreicht**
  (2026-07-01, `OBJ-005`): b-cad ist **offen austauschbar** — alle sechs Formate hinter
  Driven-Adaptern (Kern format-frei): **IFC** Import+Export (`ADR-0013`, `ACC-003`-Roundtrip),
  **STEP/STL** Export (`ADR-0014`, B-Rep aller 3D-Bauteile), **DXF** Import+Export (`ADR-0015`),
  **PDF** (`ADR-0016`, maßstäblicher Plan, `ACC-004`) + **PNG**. 16 Slices (019a–025c), vier
  Backend-ADRs. Unabhängige Welle-Verifikation **0 HIGH** (Gates selbst reproduziert: 220/220,
  90,7 %; `schema-check`/`io-smoke` grün); keine aktiven Carveouts. Closure-Notiz:
  `docs/plan/planning/done/welle-4-results.md`.
- slice-025c — **PNG-Export lauffähig: self-rolled Raster-Grundriss** (`LH-FA-IO-008`,
  welle-4, ADR-0016): schließt den PDF/PNG-Strang. Ein **self-rolled Raster-PNG-Encoder**
  (`png_writer`, io-resident, kein Qt/OCC/**zlib** — reines C++/STL, arch-check Regel A/B)
  erzeugt ein kombiniertes Rasterbild des 2D-Grundrisses: feste Leinwand 800×600,
  **Fit-to-Canvas** (geguardet gegen degenerierte Bounding-Box), **je Geschoss eine Farbe**.
  `IDAT` = zlib-Header + **stored-DEFLATE**-Blöcke + Adler-32, CRC-32 je Chunk, Bresenham-
  Rasterizer. `PngExportAdapter` nutzt `plan_geometry` + `io_atomic_write` (025b) wieder;
  `ExchangeFormat::Png` additiv; `--export-png` + `ExporterMap` (export-only). **Voll-Decode-
  Orakel** mit **eigenständigen** CRC-32/Adler-32/Inflate (keine Encoder-Tautologie) +
  degenerierte-BBox-Guard + Boundary + `E-IO-001` + export-only; `make gates` grün (220/220,
  90,7 %) + `make io-smoke` (`--export-png`). `MR-006` + unabhängiges **Code-Review je 0 HIGH**
  (reale libpng-Öffenbarkeit + binascii/zlib **empirisch** belegt). `main.cpp`-Export-Dispatch
  auf Tabelle+Schleife refaktoriert (cognitive-complexity ≤ 20). **Damit sind alle
  welle-4-Format-Backends (IFC/DXF/STEP/STL/PDF/PNG) geliefert.**
- slice-025b — **PDF-Export lauffähig: self-rolled Vektor-Maßstabsplan** (`LH-FA-IO-007`,
  `ACC-004`, welle-4, ADR-0016): erster echter Code des PDF/PNG-Strangs. Ein **self-rolled
  Vektor-PDF-Writer** (`pdf_writer`, io-resident, kein Qt/OCC, **keine neue Dependency** —
  reines C++/STL, arch-check Regel A/B) erzeugt je Geschoss eine A4-Seite mit **maßstäblichem
  (fest 1:100)** 2D-Grundriss (gerade Wand-Achsen) + Rahmen + „M 1:100"-Label (Helvetica
  Base-14, kein Embedding). `PdfExportAdapter` (Domänen→Plan-Mapping) + geteilte Helfer
  `plan_geometry` (2D-Projektion) + `io_atomic_write` (atomarer binär-treuer Writer,
  `E-IO-001`) — beide für 025c/PNG wiederverwendbar. `ExchangeFormat::Pdf` additiv (nur Enum,
  keine Registry-Architektur-Änderung); Composition Root `--export-pdf` + `ExporterMap`
  (export-only, Import → `E-IO-003`). **Voll-Decode-Orakel** im Test (Byte-Konsistenz +
  Objektgraph/Reader-Öffenbarkeit + Linien je Geschoss-Seite) + Maßstabs-Sonde
  (5000 mm → 141,73 pt) + Orientierungs-Sonde + Boundary + `E-IO-001` + export-only; 7 PDF-Tests,
  `make gates` grün (215/215, 90,4 %) + `make io-smoke` (`--export-pdf`). `MR-006` +
  unabhängiges **Code-Review je 0 HIGH** (reale Reader-Öffenbarkeit **empirisch** belegt:
  poppler/pdftotext/ghostscript). **Damit ist `ACC-004` (maßstäblicher PDF-Plan) erfüllt.**
  Sizing-Split: PNG = Folge-Slice 025c.
- slice-025a — **PDF/PNG-Export AK-Schärfung + Spec-Mapping** (`LH-FA-IO-007`/`LH-FA-IO-008`,
  `ACC-004`, welle-4): schärft die letzten IO-Format-Anforderungen von Outline auf AK
  (**export-only, 2D-Achsen-Maßstabsplan**, Lastenheft 0.1.12) und löst die `ADR-0016`-Folgepflicht
  ein. Parametrisiert auf **`ADR-0016`** (PDF/PNG-Export-Backend **accepted** 2026-07-01: selbst
  getragene Vektor-PDF-/Raster-PNG-Writer, **Option D**, io-resident, export-only, kein Qt, keine
  neue Dependency; unabh. Text-Review 0 HIGH + Projektinhaber-Durchsicht). §1 `LH-FA-IO-007.a`
  (Sammelblock: self-rolled Writer io-resident, maßstäblicher Achsen-Plan, export-only — Import-Request
  → generisches `E-IO-003`; atomarer Export `E-IO-001`, binär-sicher); **§6** zwei neue
  PDF/PNG-Vertragszeilen; **§7** PDF/PNG chirurgisch aus der Offene-Backends-Klausel → **alle
  IO-Backends entschieden**; **§4** `E-IO-001` um PDF/PNG-Export; `architecture.md`-Provenance. Reine
  Doku, kein Code (Impl = Folge-Slice 025b). `MR-006`-Plan-Review **0 HIGH** (2 LOW eingearbeitet).
  `make gates` grün (docs-check 0).
- slice-024b — **Treppen STEP-B-Rep: analytische Box-Solids (Stufen), Geländer ausgelassen**
  (`LH-FA-IO-005`/`LH-FA-STR-001`, welle-4): schließt die **zweite** Hälfte der STEP-Lücke —
  **damit sind alle 3D-Bauteile B-Rep**. Die Treppen-**Stufen** werden als analytische OCC-Box-Solids
  je Stufe exportiert (`makeBoxSolid`/`BRepPrimAPI_MakeBox`), **nicht** aus dem flachen `stairMesh`
  vernäht (das ist eine nicht-manifolde Box-Union). Neue pure Kern-Query `stairStepBoxes` als **eine
  Box-Wahrheit** für `stairMesh` (Display/STL) **und** den STEP-Export; `stairMesh` darauf refaktoriert
  (verhaltens-identisch). Das **Geländer** (render-only) ist **ausgelassen** (bleibt im STL) — §1
  geschärft. AK **OCC-frei**: Treppe trägt **genau `step_count`** zusätzliche `CLOSED_SHELL` bei (belegt
  zugleich die Geländer-Auslassung); + direkter `stairStepBoxes`-Box-Truth-Test. `MR-006` (gemeinsam)
  + **`MR-009` 0 HIGH** (Refaktor byte-identisch verifiziert; 2 LOW/1 INFO eingearbeitet). `make gates`
  grün (208/208, arch-check Regel C, docs-check 0).
- slice-024a — **Dächer STEP-B-Rep: wasserdichtes Netz → vernähtes Solid** (`LH-FA-IO-005`/
  `LH-FA-ROF-006`, welle-4): schließt die **Dach-Hälfte** der in slice-020b benannten STEP-Lücke
  (Dächer/Treppen waren STL-only). Der seit 023b wasserdichte `roofMesh` wird im geometrie-residenten
  STEP-Export zu **einem** OCC-B-Rep-Solid **vernäht** (neuer `meshToSolid` in `occ_solids`:
  `BRepBuilderAPI_Sewing` → geschlossene Shell → `MakeSolid`), **fail-closed** geprüft
  (`BRepCheck_Analyzer`; eine nicht geschlossen vernähbare Shell wird übersprungen — kein Schrott-Solid
  im STEP). `buildSolidCompound` ergänzt die Dach-Solids; `TKShHealing` gelinkt (gleiche OCC-Distribution,
  **keine** neue Dependency, `ADR-0014`). §1 `LH-FA-IO-005.a` geschärft (Dächer B-Rep; Treppen-Lücke
  bleibt benannt bis 024b). AK-Test **OCC-frei** (Re-Read-Text-Orakel: Dach trägt genau eine zusätzliche
  `CLOSED_SHELL`/`MANIFOLD_SOLID_BREP` bei — topologische Wasserdichtheit, kein „nicht-leer"). `MR-006`
  (gemeinsam) HIGH-1 vor Start behoben; `MR-009`-Geometrie-Code-Review vor Welle-Closure offen. `make gates`
  grün (206/206, arch-check Regel C, docs-check 0). **Treppen-Hälfte = 024b.**
- slice-023c — **Dach-Thickness-Persistenz: `roofs.thickness_mm` Round-Trip** (`LH-FA-ROF-006`/
  `LH-FA-BLD-002`/`003`, welle-4, Dach-Volumen-Initiative): schließt die in 023b benannte Persistenz-Lücke
  — die in 023b eingeführte `Roof.thickness_mm` (Dach-Volumenkörper) überlebt jetzt Speichern/Laden.
  **Erste Schema-Änderung im Repo:** `spec/data-model.yaml` bekommt `roofs.thickness_mm` (decimal 12,3,
  `default: 200` == `DEFAULT_ROOF_THICKNESS_MM`), `schema.sql` via **gepinntem d-migrate** neu erzeugt
  (`"thickness_mm" REAL DEFAULT 200`); `insertRoofs`/`loadRoofs` binden/lesen die Spalte in der atomaren
  Transaktion. Round-Trip-AK um nicht-glatte Dicke erweitert + Default-Pfad-Sonde (Zeile ohne Dicke lädt
  als `kDefaultRoofThicknessMm`). **No-Version-Bump** regelwerk-konform (`releasing.md` §27
  Abwärtskompat-Zweig via SQL-Default; `ADR-0006`-Schema). `MR-006` 2 HIGH (falscher Präzedenz/ungegründete
  Versions-Entscheidung) **behoben** vor Start, `MR-009` n/a; `make schema-check` + `make gates` grün
  (205/205, Coverage 90,3 %). **Nachfolge: STEP-B-Rep Dächer/Treppen = 024.**
- slice-023b — **Dach-Volumen: geschlossener Schräg-Slab + EVL-Dach-Volumen** (`LH-FA-ROF-006`/
  `LH-FA-EVL-002`, welle-4, Dach-Volumen-Initiative): `roofMesh` baut statt der dicke-losen Fläche einen
  **geschlossenen, wasserdichten Volumenkörper-Slab** der Dicke `d` (Oberseite + vertikal versetzte
  Unterseite + Rand-Seitenwände; alle 3 Typen Sattel/Walm/Pult inkl. Zeltdach-Apex). `Roof.thickness_mm`
  + `setRoofThickness`-Setter (Default/Klemmung mit `ParamStatus::Clamped`). **EVL-Netto-Volumen** =
  `bx·ty·d` analytisch im Kern (`roofs_m3`, ohne `Solid.volume_mm3`) — schließt die welle-3-Dach-Lücke.
  Geometrische Invarianten-Tests (wasserdicht: jede Kante 2 Flächen; außen-orientiert; **Volumen ==
  `bx·ty·d`**). `MR-006` 0 HIGH + **`MR-009` 0 HIGH** (Geometrie über 15 Parametrierungen repliziert,
  signiertes Volumen bit-exakt); `make gates` grün (204/204, Coverage 90,2 %). **Persistenz
  (`roofs.thickness_mm`) = 023c, STEP-B-Rep = 024.**
- slice-023a — **Dach-Volumen AK-Schärfung + Spec-Geometrie** (`LH-FA-ROF-006`, welle-4,
  Dach-Volumen-Initiative): **neue Lastenheft-Anforderung** — ein Dach hat eine **Dicke** und ist ein
  **Volumenkörper** (lösungsfrei, Standard-Dicke, Grenzwert-Klemmung, Totalität); löst die
  ROF↔`LH-FA-IO-005`-Inkonsistenz (Export als Volumenkörper) auf. Lastenheft **0.1.11**. spez. §1
  `LH-FA-ROF-001.a`: geschlossener **Schräg-Slab** der Dicke `d` (Oberseite + vertikal versetzte
  Unterseite + geschlossene Trauf-/Giebel-Ränder, wasserdicht/außen-orientiert, alle 3 Typen);
  Dach-Volumen **analytisch im Kern** (ohne `Solid.volume_mm3`); §3 `ROOF_THICKNESS_*` (50/500/200 mm).
  **Vertragserweiterung** (Projektinhaber-autorisiert). `MR-006` 0 HIGH; reine Doku. Voraussetzung für
  EVL-Dach-Volumen + STEP-B-Rep der Dächer (Folge-Slices 023b/c, 024).
- slice-022 — **io-smoke: headless Binary-Smoke aller IO-Formate** (`make io-smoke`,
  welle-4-Quergewerk, CI-only, `LH-FA-IO-001`…`006`): startet das gebaute `b-cad`-Binary
  headless (xvfb) je Format — **IFC/DXF** Export+Re-Import, **STEP/STL** Export — und prüft
  exit 0 + nicht-leere Datei (fail-closed je-Aufruf-Guard in `tools/io-smoke.sh` + Negativ-
  Selbsttest `BCAD_SMOKE_SELFTEST=1`). Belegt die sonst coverage-ausgenommene `main.cpp`-CLI-/
  Composition-Root-Glue der IO-Pfade. **Kein `gates`-Member** (GUI-Binary unter xvfb →
  CI-Befehlsliste, Muster `acc-002-beleg`/`schema-check`); **erste reale LH-Bindung** der
  Sensors-Zusatzklasse. `MR-006` 0 HIGH; **kein Produktions-Code** (die `--import-*`/
  `--export-*`-Flags existieren seit slice-019c/020b/021b). Doku: `harness/README.md` §Sensors +
  `AGENTS.md` §3 + `harness/conventions.md`.
- slice-021b — **DXF-Import/-Export lauffähig** (`LH-FA-IO-003`/`004`, welle-4, `ADR-0015`):
  io-residenter, selbst getragener **ASCII-DXF-Subset-Codec** (R12/AC1009; `DxfReader`/
  `DxfWriter`, `LINE`/`LWPOLYLINE`, format-agnostisch — **keine neue Dependency**) +
  `DxfImportAdapter`/`DxfExportAdapter` hinter `ModelImporterPort`/`ModelExporterPort`
  (`adapters/io/`); **2D-Grundriss** = gerade Wand-Achsen je Geschoss-`LAYER`, Import →
  Default-Höhe/-Dicke (DXF trägt keine — benannte Lücke). Kern: `ExchangeFormat::Dxf` + der
  **Import-Dispatch auf eine `ImporterMap`** umgestellt (symmetrisch zur bestehenden
  `ExporterMap`; STEP/STL bleiben export-only via Lookup-Miss → `E-IO-003`/`import_rejected`).
  Export atomar (Temp+fsync+Rename → `E-IO-001`), Import atomar/total (`E-IO-003`; leer/
  strukturlos → leeres Modell). Composition Root + `--import-dxf`/`--export-dxf`. **Roundtrip**
  = Wand-Achsen-Anzahl je Geschoss + Achs-Lage (nicht Höhe/Dicke). 14 neue Tests (Codec-
  Roundtrip isoliert + AK/Integration über den echten `ExchangeService`-Pfad), 3 io-Tests auf
  `ImporterMap` migriert. `MR-006` 0 HIGH + unabhängiges Code-Review 0 HIGH (2 MED Test-Orakel-
  Lücken geschlossen); `make gates` grün (201/201, Coverage 89,9 %). **DXF-Strang abgeschlossen.**
- slice-021a — **DXF-Import/-Export AK-Schärfung + Spec-Mapping** (`LH-FA-IO-003`/`004`,
  welle-4, `ADR-0015`): `LH-FA-IO-003`/`004` von Outline auf AK-Niveau (lösungsfrei,
  **bidirektional, 2D-Grundriss** — Export → valide DXF, von einem Standard-Leser als
  2D-Grundriss/Wand-Achsen je Geschoss lesbar; Import → gerade Wände, Anzahl-Treue,
  **Standard-Höhe/-Dicke** [DXF trägt keine — benannte Lücke wie IFC-Geschoss-Höhe];
  Roundtrip = Achsen-Anzahl, nicht Höhe/Dicke; `E-IO-003`/`E-IO-001` atomar; Teilumfang
  gerade Wände als 2D-Achsen je Geschoss, Räume/Bemaßung/Schraffur/Blöcke/Text/Bögen/3D
  übersprungen/nicht geschrieben), Lastenheft 0.1.10. spez. §1 `LH-FA-IO-003.a` (selbst
  getragener ASCII-DXF-Subset-Codec **io-resident**, kein OCC — wie IFC; gerade
  Wand-Achsen je Geschoss-`LAYER`, Import-Defaults, atomarer Import In-Memory-zuerst).
  §6 neue DXF-Vertragszeile, §7 DXF chirurgisch aus der Offene-Backends-Klausel
  (PDF/PNG bleiben offen), §4 `E-IO-001` um STEP/STL/DXF-Export erweitert (latente
  `ADR-0014`-Lücke parallel geschlossen). **`ADR-0015` „DXF-Backend" accepted**
  (selbst getragener DXF-Subset-Codec Option D, OCC kann kein DXF; Text-Review 0 HIGH).
  `MR-006` 0 HIGH; reine Doku/Entscheidung.
- slice-020b — **STEP-/STL-Export lauffähig** (`LH-FA-IO-005`/`006`, welle-4, `ADR-0014`):
  ein `model::Building` wird als **valide STL** (binär, Dreiecksnetz **aller** 3D-Bauteile)
  und **STEP** (B-Rep-Volumenkörper der OCC-Solid-Bauteile = Wände + Decken/Fundament,
  AP214) geschrieben — über **geometrie-residente** `ModelExporterPort`-Adapter
  (`adapters/geometry/stl_export_adapter`, `step_export_adapter`; OCC-DataExchange nativ,
  **keine** neue Dependency), **atomar** (Temp+`fsync`+Rename → `E-IO-001`, kein
  Teil-Export). Geteilter OCC-Solid-Builder `occ_solids` (aus `occ_geometry_adapter`
  ausgelagert, keine Duplikation). `ExchangeService` auf ein **Exporter-Registry**
  (Format→Port) umgestellt; `ExchangeFormat::Step`/`Stl` (export-only → Import wirft
  `E-IO-003`); Composition Root `--export-step`/`--export-stl`. **Benannte STEP-Lücke:**
  Dächer/Treppen sind analytische Netze ohne OCC-Solid → STL-verlustfrei, STEP-B-Rep
  via Mesh-Vernähung ist ein Folge-Inkrement (spez. §1 `LH-FA-IO-005.a`). MR-006-Plan-
  Review + Code-Review/MR-009 je 0 HIGH; `make gates` grün (180/180, Coverage 90,0 %).
- slice-020a — **STEP-/STL-Export AK-Schärfung + Spec-Mapping** (`LH-FA-IO-005`/`006`,
  welle-4, `ADR-0014`): `LH-FA-IO-005`/`006` von Outline auf AK-Niveau (lösungsfrei,
  **export-only** — valide STEP-/STL-Datei, von einem Standard-Leser als Volumenkörper/
  Dreiecksnetz lesbar; `E-IO-001`, kein Teil-Export; Boundary leer→gültig-leer; Teilumfang
  3D-Bauteile, Material/Farbe ausgespart), Lastenheft 0.1.9. spez. §1 `LH-FA-IO-005.a`
  (OCC-DataExchange nativ, **geometrie-residenter** `ModelExporterPort`, Regel C). §6
  STEP/STL-Vertragszeile, §7 STEP/STL chirurgisch aus der Offene-Backends-Klausel
  (DXF/PDF/PNG bleiben offen). **`ADR-0014` „STEP/STL-Export-Backend" accepted**
  (OCC-DataExchange nativ, keine neue Dependency, geometrie-residente
  `ModelExporterPort`-Naht; Text-Review 0 HIGH). `MR-006` 0 HIGH; reine Doku/Entscheidung.
- slice-019c — **IFC-Export lauffähig** (`LH-FA-IO-002`, `ACC-003`, welle-4, `ADR-0013`
  Option D): ein `model::Building` (Geschosse + gerade Wände) wird als valide IFC4-SPF-
  Datei geschrieben — **atomar** (Temp+`fsync`+Rename), so dass der 019b-Import dieselbe
  **Geschoss-/Wand-Anzahl** zurückgibt (Roundtrip). Neuer hand-gerollter **IFC-SPF-Writer**
  (`adapters/io/ifc_spf_writer`, Spiegel des Readers — symmetrisch Lesen+Schreiben,
  **keine** externe IFC-Lib) + **Domänen→IFC4-Mapping** (`adapters/io/ifc_export_adapter`:
  `Storey`→`IfcBuildingStorey` mit kumulierter Elevation, `Wall`→`IfcWall`+Achs-`IfcPolyline`
  +`IfcExtrudedAreaSolid`+`IfcMaterialLayerSet`+`IfcRelContainedInSpatialStructure`,
  Spatial-Komposition `IfcRelAggregates`, `IfcUnitAssignment` mm; `IfcWall` **nicht**
  deprecated `IfcWallStandardCase`). Kern-Use-Case erweitert: `ExchangeModelPort.exportModel`
  + neuer Driven-Port `ModelExporterPort` + `ExchangeService` (pure Domäne, `arch-check` A/B);
  Composition Root verdrahtet (`--export-ifc <pfad>`). Nicht beschreibbarer Zielpfad →
  `E-IO-001` (`event=io_no_permission`, **kein** Teil-Export, Zielpfad unverändert). **Benannte
  Subset-Grenzen** (Spiegel 019b): nur Geschosse+Wände; oberste Geschoss-Höhe nicht
  roundtrip-treu (Default); Body trägt nur die Extrusions-Höhe; `Wall.type`/`material_id`
  ohne IFC-Subset-Entsprechung. Spec §1-Export-Zusatz (Elevation aus kumulierten Höhen) +
  §4-`E-IO-001`-Klammer + architecture §5/`Geschichte`; `MR-009` n/a. Zwei unabhängige
  `MR-006`-Plan-Reviews (Subagent + Projektinhaber, 0 HIGH) + unabh. Code-Review.
- slice-019b — **IFC-Import lauffähig** (`LH-FA-IO-001`, welle-4, `ADR-0013` Option D):
  eine valide IFC-SPF-Datei im welle-4-Subset (Geschosse + gerade Wände) wird über den
  IO-Adapter in ein `model::Building` gelesen — **anzahl-treu**, **atomar**, **total**
  (spez. §1 `LH-FA-IO-001.a`). Neuer **hand-gerollter IFC-SPF-Codec**
  (`adapters/io/ifc_spf_reader` — ISO-10303-21-Tokenizer + Entitäts-Graph, **keine**
  externe IFC-Lib/Dependency) + **Domänen-Mapping** (`adapters/io/ifc_import_adapter`:
  `IfcBuildingStorey`→`Storey`, `IfcWall`/`IfcWallStandardCase`→`Wall` über
  Achs-Polyline/`IfcMaterialLayerSet`/Extrusions-Höhe/`IfcRelContainedInSpatialStructure`).
  Neuer Kern-Use-Case: Driving-Port `ExchangeModelPort` + Driven-Port `ModelImporterPort`
  + `ExchangeService` (pure Domäne, kein IFC-/SPF-Symbol im Kern — `arch-check` A/B);
  Composition Root verdrahtet (`--import-ifc <pfad>`). Nicht-IFC/kaputt → `E-IO-003`
  (`event=import_rejected`, **kein** Teil-Import); leer/strukturlos → leeres Modell;
  Subset-fremde Entität (`IfcDoor`/…) übersprungen. **Geschoss-Höhe** = Elevation-Differenz
  (MED-2/3, spez. §1-Zusatz, `MR-008`-zulässig; Elevation transient). **Benannte
  Subset-Grenzen:** keine Placement-Komposition (Achse in Geschoss-/Identity-Koord.),
  `IfcRelAggregates` nicht traversiert (Geschosse direkt enumeriert), `Wall.type`→`Innen`,
  `material_id`→`nullopt`. `MR-009` n/a (kein Geometrie-Erzeugen). Unabhängiges
  `MR-006`-Plan-Review (0 HIGH); Zwei-Commit-Split (Codec / Mapping+Service+Tests).
- slice-019a — **IFC-Import/Export AK-Schärfung + Spec-Mapping** (`LH-FA-IO-001`/002,
  welle-4): `LH-FA-IO-001`/002 von Outline auf AK-Niveau (lösungsfrei — Import/Export
  von Geschossen + geraden Wänden, Anzahl-Treue, Roundtrip, `E-IO-003`/`E-IO-001`;
  Teilumfang explizit, weitere Bauteile übersprungen/nicht geschrieben), Lastenheft
  0.1.8. spez. §1 `LH-FA-IO-001.a` (IFC-SPF-Subset-Mapping, IFC4-Export
  `IfcWall`+`IfcMaterialLayerSetUsage` / Import IFC4+IFC2x3, atomar). `ADR-0013`-
  Folgepflicht eingelöst (§6/§7-Nachzug); **`MR-011`**: ADR-Provenance nur in
  `*-historie.md`, Spec-Körper ADR-frei. Unabhängiges `MR-006`-Plan-Review (0 HIGH,
  3 LOW eingearbeitet). Reine Doku/Entscheidung, kein Code; gates grün (145/145).
- **welle-4-austausch gestartet** + **`ADR-0013` (IFC-Bibliothek) accepted** (2026-06-16):
  erster Schritt der Austausch-Welle. `ADR-0013` entscheidet das **IFC-Backend** —
  ein **selbst getragener, vendierter IFC-SPF-Subset-Codec** in `src/adapters/io/`
  (Option D: kein Bibliotheks-Zukauf jetzt — `ADR-0004`-konform, kein vcpkg/Conan/
  Source-Build), Export **IFC4** (`IfcWall`+`IfcMaterialLayerSetUsage`) / Import
  IFC4+IFC2x3-Subset (`IfcProject`→`IfcSite`→`IfcBuilding`→`IfcBuildingStorey` +
  Standard-Wände), atomar (`E-IO-003`, kein Teil-Import). OCC (`ADR-0002`) liefert
  STEP/STL nativ, **kann aber kein IFC** → STEP/STL/DXF/PDF/PNG bleiben
  **Schwester-ADRs**. **Re-Eval auf IfcOpenShell/web-ifc**, sobald voller
  IFC-Reichtum nötig + `ADR-0004`-konform installierbar. **Zwei unabhängige
  Review-Runden vor Accept** (Subagent + Projektinhaber): **0 HIGH**, 5 MED + 4 LOW
  eingearbeitet (u. a. `IfcWallStandardCase`-IFC4-Deprecation → `IfcWall`;
  Fitness Function um Adapter-Pfad-Integrationstest erweitert; Spec-§6/§7-Nachzug
  als Folgepflicht; Re-Eval-Trigger beobachtbar gemacht). Report
  `docs/reviews/2026-06-16-adr-0013-text-review.md`; Folgepflichten im ADR-Index.
  Roadmap: welle-3 → Abgeschlossene Wellen, **welle-4-austausch** nun §Aktuelle Welle.
  Reine Doku/Entscheidung (kein Code); `make gates` grün (145/145, Coverage 92,7 %).
- **Welle-3-auswertung abgeschlossen** (Closure 2026-06-16): das Gebäudemodell ist
  **auswertbar** — Flächen (Shoelace-Raum-Netto + Wohnfläche, `LH-FA-EVL-001`/003),
  **Volumen analytisch im Kern** (`LH-FA-EVL-002`: Wand/Decke/Treppe; Dach dicke-los
  zurückgestellt), **Material-System** (`LH-FA-MAT-001`/002/003/005, projekt-eigen über
  `EditStructurePort`, `on_delete: restrict`-treu, **NULL-sicher** round-trippt über
  SQLite ohne Schema-Drift), **Listen** (`LH-FA-EVL-004`/005/006: Material-Menge = Σ
  Netto-Volumen, Tür-/Fensterlisten) + **Kosten** (`LH-FA-MAT-006`: `Menge × cost_per_m3`)
  — alles reine read-only-Ableitung über `EvaluatePort` (`ADR-0012`: **kein**
  `GeometryKernelPort`/`Solid.volume_mm3`). **7 Slices** (017a–017g) in `done-archive/`,
  getragen von der **`ADR-0012`-Leitplanke** ohne neuen Grundsatz-ADR je Auswertung.
  **Meilenstein M3 „Auswertbar" erreicht**. Unabhängige Welle-Verifikation (0 HIGH, 1 LOW
  behoben) + Carveout-Audit (keine aktiven); geometrielastiges Code-Review für 017c
  (Volumen, 0 HIGH) + Code-Review (höhere Latte) für 017e (Persistenz, 0 HIGH).
  `wall_type`-Template-Fallback bewusst zurückgestellt (welle-4+, Projektinhaber-
  Entscheidung). `make gates` grün am HEAD (145/145, Coverage 92,7 %), `make schema-check`
  grün. Ergebnisnotiz: `welle-3-results.md`.
- slice-017g — **Kosten-Auswertung** (`LH-FA-MAT-006`, welle-3): die Materialliste
  (`EvaluatePort.materialList()`) trägt je Material die **Kosten** =
  `quantity_m3 × cost_per_m3` (`MaterialLine.cost`) + die **Projekt-Material-
  Kosten-Summe** (`MaterialReport.total_cost`). Material **ohne** `cost_per_m3` →
  `cost` = `std::nullopt` (kein Kostenkennwert ≠ kostenlos), trägt 0 zur Summe.
  `cost_per_m2` (Flächen-Kosten) welle-3 zurückgestellt (benannte Lücke — EVL-004
  führt Volumen). Additive Erweiterung der 017f-`MaterialReport` (kein neuer Port/
  Werttyp). Unabhängiges `MR-006`-Review (0 HIGH); `MR-009` n/a. **Schließt die
  Material-Kennwert-Nutzung.**
- slice-017f — **EVL-Listen** (`LH-FA-EVL-004/005/006`, welle-3 — **schließt das
  EVL-Modul**): `EvaluatePort.materialList()` (Materialliste: je effektivem
  Material die Bauteil-Anzahl + Menge = Σ Netto-Volumen m³ über Wand+Decke,
  Reuse `effectiveMaterial`/`volume_geometry`; Bauteile ohne Material nicht
  gruppiert; **Dach ausgenommen** — Volumen welle-3 zurückgestellt, benannte
  Lücke; deterministisch nach `MaterialId`), `doorList()`/`windowList()`
  (Tür-/Fensterlisten mit Maßen). Neue pure Werttypen `MaterialReport`/
  `MaterialLine`/`DoorLine`/`WindowLine`. Read-only-Aggregation (kein `op`, kein
  `GeometryKernelPort`). spez. §1 nachgezogen (EVL-004-Menge = Volumen,
  `roofs`-Lücke). Unabhängiges `MR-006`-Review (0 HIGH, 1 LOW eingearbeitet);
  `MR-009` n/a.
- slice-017e — **Material-Persistenz** (`LH-FA-MAT-001/003`, `LH-FA-BLD-002`, welle-3):
  die `materials`-Bibliothek + die `material_id`-Zuweisung an Wand/Dach/Decke
  round-trippen im `SqliteProjectRepository` (`insertMaterials`/`loadMaterials` +
  `material_id` in insert/load von walls/roofs/slabs), in der bestehenden atomaren
  Speicher-Transaktion (materials **vor** den Bauteilen — FK). **NULL-sicher**
  über zentrale `bind/columnOptional*`-Helfer (`sqlite3_column_type == SQLITE_NULL`
  → `std::nullopt`; kein „NULL → 0"). ID-Erhalt → `material_id` zeigt nach Reload
  aufs richtige Material. **Kein Schema-Wechsel** (`materials` + `material_id`-FKs
  lagen vor, `make schema-check` grün); `materials.density` ohne Domänenfeld →
  nicht round-getrippt (benannte Lücke, löst 017d-LOW-1). Unabhängiges
  `MR-006`-Plan-Review (0 HIGH) + **unabhängiges Code-Review** (höhere Latte:
  Parsing/Schema-Drift/stille Verfälschung).
- slice-017d — **Material-System** (`LH-FA-MAT-001/002/003/005/006`, welle-3,
  in-memory): `model::Material` (pure Werte, Kennwerte U-Wert/Kosten) + projekt-
  eigene Material-Verwaltung/-Zuweisung über `EditStructurePort`
  (`addMaterial`/`updateMaterial`/`removeMaterial`/`set{Wall,Roof,Slab}Material`)
  und read-only Material-Liste/Auflösung über `EvaluatePort` (`materials()`,
  `effectiveMaterial(...)` — Quelle für EVL-004/006). `removeMaterial` ist
  `on_delete: restrict`-treu (noch zugewiesenes Material nicht löschbar);
  Material-Mutationen sind **op-frei** (Pull-Konsum). **Override-Auflösung**
  geliefert, `wall_type`-Template-Fallback zurückgestellt (kein material-
  tragender Wandtyp im Domänenmodell). Port-Reconciliation in `architecture.md`
  (`MaterialLibraryPort` driven = externer Katalog). Kein neuer ADR/keine
  Persistenz (→ slice-017e). Unabhängiges `MR-006`-Review (1 HIGH + 1 MED + 2 LOW
  eingearbeitet); `MR-009` n/a.
- slice-017c — **Volumen-Auswertung** (`LH-FA-EVL-002`, welle-3): `EvaluatePort.volume()`
  liefert das gebäudeweite **Netto-Material-Volumen** in m³ + Bauteiltyp-Subtotale
  (`VolumeReport`: Wand · Decke/Fundament · Treppe), **analytisch im Kern** (neue
  `services/volume_geometry`: Wand = Footprint·Höhe − geklemmte Öffnungen; Platte =
  (Fläche − Ausschnitte)·Dicke; Treppe = Σ Stufenkörper, geländer-frei) — **kein**
  `GeometryKernelPort`/`Solid.volume_mm3` (reine Kern-Query). **Dach zurückgestellt**
  (dicke-loses Modell, benannte Lücke + Re-Eval; spez. §1). Unabhängiges `MR-006`-Review
  (1 HIGH + 3 MED + 2 LOW eingearbeitet); `MR-009` greift.
- slice-018c — **Per-ID-Anker-Präzision** (`harness-steering`): die 17 in slice-018b
  auf das Geschwister-Heading verlinkten Bullet-Sub-IDs (`LH-FA` D3/IO/ROM/WAL) per
  **Inline-HTML-Anker** (`<a id>`, d-check v0.9.0) auf präzise Per-ID-Anker gehoben.
  `tools/idlink.py` lernt HTML-Anker (höchste Präzedenz, `sect`-Regex gehärtet) +
  normalisiert bestehende ID-Links (idempotent, self-healing); 7 Repoints (6 Sub-ID +
  1 `MR-009`-Self-Heal). Negativ-Beleg: 6 anchor-missing ohne Anker. Unabhängiges
  `MR-006`-Review (3 HIGH + 2 MED eingearbeitet).
- slice-018b — **Voll-Korpus-Link-Hygiene** (`MR-011`): `ids` über alle 7 ID-Familien
  im Live-Korpus (link-policy always), done-archive via scope.ignore raus. 718
  Kennungs-Vorkommen aufgelöst — 480 Links (normativer Korpus, Kapitel-/Heading-Anker
  + Cross-Stratum) + 238 Backticks (Logs CHANGELOG/reviews/*-historie via exempt-paths).
  Neuer Generator tools/idlink.py (d-check Slugify-Port, d-check als Orakel);
  unabhängiges `MR-006`-Review (2 HIGH + 3 MED + LOW eingearbeitet). Deckte `LH-QA-007`
  als undefinierte ID-Referenz auf (Datei-Fallback + Folge-Punkt).
- slice-018a — **Doku-Referenz-Gate** (`harness-steering`, `MR-011`): `done-archive/`
  für abgeschlossene Slice-/Spike-Pläne (31 per reinem `git mv` archiviert;
  `*-results.md` + acc-002-Artefakte bleiben in `done/`); `docs-check` um die
  d-check-Module `matrix`/`ids`/`spans`/`hostpaths` erweitert. **Regelwerk-
  Referenz-Richtung Spec→ADR** computational durchgesetzt: `matrix` (Link-Form)
  + `ids` (nackt/Inline-Code, spec-gescopt, `link-policy: always`) — die ~72
  Spec→ADR-Körper-Referenzen in `architecture.md`/`spezifikation.md`/
  `lastenheft.md` aufgelöst (Körper entfernt, Historie-Tabellen nach
  `spec/*-historie.md` ausgelagert, `## Geschichte`-Provenance-Rand in
  `architecture.md`); `AGENTS.md` §2.7
  reconcilet (Architektur-Körper ADR-referenzfrei). Folge-Slice slice-018b
  (Voll-Korpus-`ids`, alle 7 ID-Familien).
- Greenfield-Harness-Bootstrap nach AI-Harness-Kurs (Modul 2): stratifizierte Spec
  (Lastenheft/Spezifikation/Architektur), `ADR-0001`..0003, Roadmap, Planning-Lifecycle,
  `AGENTS.md`, `harness/` (README + Konventionen), Glossar, Releasing-Outline.
- `make docs-check` — Doku-Link-Validator als erstes reales Gate (vendored, `MR-003`).
- MIT-Lizenz.
- slice-001 — hexagonales C++-Build-Skelett: CMake-Target-Trennung Kern/Adapter
  (`ADR-0001`), Qt6/OpenCascade/SQLite-DevContainer, `make build` (Build + ctest).
- slice-002 — Code-Gates als Dockerfile-Target-Stages (keine Bind-Mounts):
  `make arch-check` (hexagonale Schichtung), `make lint` (clang-tidy +
  Suppression-Gate), `make test`, `make coverage-gate` (bootstrap-aware);
  Hello-Hexagon-Port-Roundtrip; Sensors gepromotet.
- spike-001 — Toolchain-Reproduzierbarkeit untersucht (Ubuntu 24.04 vs
  26.04, node-Base, Pinning) → `ADR-0004` (Proposed: Digest + apt-Snapshot,
  Migration 26.04/node24) + Folge-slice-004.
- slice-005 — `make gate-consistency`: Doku↔Makefile-Sensor (jeder als
  real dokumentierte `make`-Befehl existiert) — schließt die Drift-Klasse,
  die `docs-check` (nur Links) nicht fängt.
- Harness-Hook-Härtung (`MR-005`, Rückport aus d-check): inhaltsbasierter
  Gate-Nachweis (Commit ohne Gate-Lauf wird vom Stop-Hook geblockt),
  Guard fail-closed + ohne Permission-Bypass + Sub-Shell-/Flag-Bündel-
  Rekursion, `record-gates` parallel-sicher im `gates`-Rezept.
- slice-012 — Eckenschluss endpunkt-verbundener Wände (`LH-FA-WAL-006`-
  Teilumfang, Lastenheft 0.1.2; Auslöser: `ACC-002`-Abnahme-Befund):
  Footprint-Hoheit im Kern (`wall_footprint`, Grad-2-Eckschnitt mit
  `WALL_MITER_LIMIT`-Begrenzung und Rückfall stumpf, total);
  `GeometryKernelPort` auf `extrudeFootprint`/`tessellateFootprint`;
  Mehr-Element-Update `WallGeometryChanged` (Nachbar-Rebuild,
  deterministische Reihenfolge, transaktional über den ganzen Satz);
  8 neue WAL-006-AK-Tests (Kern/Service + Viewer-Szene) inkl.
  Fehlerfall-Transaktion.
- slice-011b — sichtbarer 3D-Viewer (welle-1v, `ACC-002` / sichtbare Hälfte
  `LH-FA-D3-002`): Qt-6-Widgets-Viewer mit orbitierbarer Perspektive;
  Tessellation über `ViewModelPort` (`ADR-0009` (b), kein OCC in der GUI);
  Szenen-Surrogat + display-freie AK-Tests; arch-check **Regel E**
  (Qt nur `adapters/ui/` + Composition Root); `make acc-002-beleg`
  (manueller Abnahme-Schritt, kein Gate); `ADR-0010` — Headless-GL via
  Xvfb/llvmpipe (offscreen-QPA trägt kein GL); Toolchain + `make
  versions`-Manifest um `xvfb` erweitert (`ADR-0004`).
- **Welle-1v-viewer abgeschlossen** (Closure 2026-06-13): die *sichtbare*
  Hälfte des Echtzeit-Vertrags steht — **`ACC-002` erfüllt**, sichtbare
  Hälfte `LH-FA-D3-002`. slice-011a/011b + slice-012 in `done/`;
  unabhängige Welle-Verifikation (keine HIGH/MEDIUM), `make gates` grün
  am HEAD (63/63, Coverage 94,2 %). Ergebnisnotiz: `welle-1v-results.md`.
- slice-013a — Bauteil-Hosting & Wandöffnungs-Modell (welle-2-bauteile,
  erster Slice; reine ADR+Spec-Entscheidung, kein Code): **`ADR-0011`**
  accepted (Öffnung als wand-gehostetes Element; Kern liefert
  Schnitt-Prismen, `GeometryKernelPort` subtrahiert per OCC-Boolean;
  `WallGeometryChanged`-Wiederverwendung; Raumerkennung/Footprint
  unberührt; Bauteil-Erweiterungs-Muster als Welle-Leitplanke).
  Lastenheft 0.1.3: Türen (`LH-FA-DOR-001`..004) + Fenster
  (`LH-FA-WIN-001`..005) von Outline auf AK geschärft; Spezifikation §1
  Öffnungs-Algorithmus + §3 Tür-/Fenster-/Brüstungs-Wertebereiche. Keine
  Schema-Schärfung nötig (`ADR-0006` trägt openings/doors/windows bereits).
- slice-013b — Türen + Fenster implementiert (`LH-FA-DOR-001`..004,
  `LH-FA-WIN-001`..005, `ADR-0011`): wand-gehostete `Opening` im Kern;
  automatische Wandöffnung als boolesche Subtraktion (`model::CutPrism`
  vom Kern, `GeometryKernelPort.extrudeFootprint`/`tessellateFootprint`
  um `cutouts` erweitert, `OccGeometryAdapter` via `BRepAlgoAPI_Cut`/
  `TKBO`, kein OCC-Leck); platzieren/verschieben/Parameter (Breite/Höhe/
  Brüstung/Anschlag)/entfernen mit Klemmung (`E-VAL-001`) + Platzierungs-
  Validierung; `WallGeometryChanged` der Wirtswand (total/transaktional),
  Raumerkennung/Footprint unberührt; 13 AK-Tests, Coverage 93,3 %.
  Öffnungen vorerst nur im Speicher — Persistenz folgt in slice-013c
  (vor erstem Save-Use-Case).
- slice-013c — Öffnungs-Persistenz (`LH-FA-DOR-001`/WIN-001, `LH-FA-BLD-002`/003,
  `ADR-0011`/0006): `SqliteProjectRepository` speichert/lädt Türen + Fenster
  über `openings` + `doors`/`windows`-CTI (in derselben atomaren
  Transaktion nach den Wänden, FK `wall_id`); Round-Trip-AK-Test. Kein
  Schema-Wechsel (`ADR-0006` trug die Tabellen bereits). Damit ist `ADR-0011`
  vollständig umgesetzt (Geometrie 013b + Persistenz 013c).
- slice-014a — Dach von Outline auf AK geschärft (`LH-FA-ROF-001`..005,
  Lastenheft 0.1.4): Sattel-/Walm-/Pultdach, Neigung 5–60°, Überstand
  0–1500 mm; **Teilumfang rechteckiger Grundriss** (komplexe Polygone
  offen). Spezifikation §1 Dach-Geometrie (Traufrechteck, Konstruktion
  je Typ, Höhenformeln) + §3 Neigungs-/Überstands-Bereiche + Defaults.
  Reine Entscheidung/Spec (kein Code, kein ADR — `ADR-0011`-Leitplanke).
- slice-014b — Dach implementiert (`LH-FA-ROF-001`..005, `ADR-0011`):
  `model::Roof` + `roof_geometry` (analytisches Dach-Netz Sattel/Walm/Pult
  im Kern, kein OCC); `ViewModelPort.roofMeshes`, `EditStructurePort`
  add/setPitch/setOverhang/setType/remove (Klemmung); `RoofChanged`-`op`
  (kein `RoomsChanged`); `ViewerScene` folgt idempotent. 9 AK-Tests; kein
  ADR (`ADR-0011`-Leitplanke). Persistenz folgt in slice-014c. make gates
  grün (92 Tests, Coverage 91,8 %).
- slice-014c — Dach-Persistenz (`LH-FA-ROF-001`, `LH-FA-BLD-002`/003,
  `ADR-0011`/0006): `SqliteProjectRepository` speichert/lädt Dächer über
  die `roofs`-Tabelle (`roof_type`/`pitch`/`overhang` als Spalten,
  rechteckiger Grundriss als `footprint_json`-Array `[ox,oy,w,d,base_z]`,
  `%.17g`; erste JSON-Ser/De im Repo, gekapselt + total). In derselben
  atomaren Transaktion nach den Geschossen; Round-Trip-AK (nicht-glatter
  double prüft `%.17g`). Kein Schema-Wechsel. Dach-Familie (014a/b/c)
  damit komplett. make gates grün (95 Tests).
- slice-015a — Decken + Fundament von Outline auf AK geschärft
  (`LH-FA-SLB-001`..003, `LH-FA-FND-001`..003, Lastenheft 0.1.5): horizontale
  Platten, Deckendicke 100–500 mm, Ausschnitte, Fundamenttiefe
  200–2000 mm, Bodenplatte. Spezifikation §1 Platten-Geometrie (Polygon ×
  Dicke an base_z je slab_type; Ausschnitte als Boolean/`CutPrism`;
  base_z-/Port-Frage an 015b) + §3 Dicke-Bereiche. Reine Entscheidung/Spec
  (kein Code, kein ADR — `ADR-0011`-Leitplanke; `MR-008` lösungsfrei).
- slice-015b — Decken + Fundament implementiert (`LH-FA-SLB-001`..003,
  `LH-FA-FND-001`..003, `ADR-0011`): `model::Slab` (Decke/Fundament/
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
- slice-015c — Decken/Fundament-Persistenz (`LH-FA-SLB-001`/003, `LH-FA-FND-001`,
  `LH-FA-BLD-002`/003, `ADR-0011`/0006): `SqliteProjectRepository` speichert/lädt
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
- slice-016a — Treppen von Outline auf AK geschärft (`LH-FA-STR-001`..004,
  Lastenheft 0.1.6): gerade einläufige Treppe verbindet zwei Geschosse,
  Stufenanzahl 2–30, Laufbreite 800–2000 mm, immer sichtbares Geländer;
  **Teilumfang gerade einläufig** (Podest/Wendel/Mehrlauf offen). Spezifikation
  §1 `LH-FA-STR-001.a` (analytisches Stufen-Quader-Polyeder im Kern,
  `rise = Geschosshöhe/step_count` abgeleitet, feste +x-Aufstiegsrichtung,
  Geländer als generierte Geometrie ohne Schema-Spalte, `StairChanged` an
  `from_storey` gebunden) + §3 Stair-Wertebereiche. Reine Entscheidung/Spec
  (kein Code, kein ADR — `ADR-0011`-Leitplanke; `MR-008` lösungsfrei). `MR-006`:
  keine HIGH (3 MED/2 LOW eingearbeitet). Nebenbei: Lastenheft-Header-Versions-
  Drift behoben (Header hing seit slice-012 auf 0.1.2, Historie war bei 0.1.5).
  make gates grün.
- slice-016b — Treppen implementiert (`LH-FA-STR-001`..004, `ADR-0011`): gerade
  einläufige Treppe in-memory + sichtbar. `model::Stair` (from/to_storey, start,
  width, step_count, tread; rise abgeleitet, Geländer intrinsisch) +
  `Building.stairs`; pure `stair_geometry` (**analytisches Stufen-Quader-
  Polyeder + beidseitiges Geländer im Kern, KEIN OCC/Port** — Muster
  `roof_geometry`; `rise = Geschosshöhe/step_count`, +x-Aufstieg, total).
  `ViewModelPort.stairMeshes()` (projektweit), `EditStructurePort`
  addStair/setStairWidth/setStairStepCount/removeStair (width/tread/step_count
  geklemmt; `setStairStepCount` int→`ParamResult`, nie `Rejected`; ungültige
  Zwei-Geschoss-Spanne `from==to`/unbekannt abgelehnt — `addSlab`-Muster),
  `StairChanged`-`op` an `from_storey` (kein `RoomsChanged`); `ViewerScene` über
  `reloadKeyed` (Roof+Slab+Stair). 7 AK-Tests `LH-FA-STR-*` (Kern: rise
  abgeleitet/z-Span/Stufenanzahl/Laufbreite/Geländer-Doppelsonde/Totalität;
  Szene: folgt+verbindet+idempotent, Klemmung/Spanne/Meldung). Zwei-Commit-Split
  i/ii. Persistenz folgt in slice-016c. make gates grün (112 Tests, Coverage
  92,2 %). **Unabhängiges geometrielastiges Code-Review danach** (Modul 11, vor
  Welle-Closure; keine HIGH — Normalen-Winding/rise-Exaktheit/Stufen-Bündigkeit/
  Geländer korrekt): 3 MED gefixt — `stairRunLengthMm`-Totalitäts-Guard +
  zwei Test-Sonden (invertierte Normalen via Divergenzsatz, Stufen-Bündigkeit),
  116 Tests.
- slice-016c — Treppen-Persistenz (`LH-FA-STR-001`..003, `LH-FA-BLD-002`/003,
  `ADR-0011`/0006): `SqliteProjectRepository` speichert/lädt Treppen über die
  `stairs`-Tabelle (from/to_storey, stair_type, start_x/y, width, step_count,
  tread als Spalten/`bind_double`/`bind_int`). **`rise_mm` write-derived** (via
  Kern-Helfer `stairRiseMm`, from_storey-Höhe aus `building.storeys`; Lookup
  total → `E-IO` bei fehlendem Geschoss) und beim Laden **nicht** zurückgelesen
  — Gegenstück zu den 015c-Cutouts: `rise` ist abgeleitet (wie roofs-`height_mm`),
  nicht domänen-getragen; Geländer render-only. `name` `NULL` (welle-2-Scope).
  In derselben atomaren Transaktion nach den Geschossen (FK from/to_storey
  RESTRICT); `stair_type`-Mapper total (Schema ohne CHECK). Round-Trip-AK
  (nicht-glatter double → `bind_double` exakt) + Leer-Pfad- + Negativ-Parse-Test
  (`stair_type` korrumpiert → `E-IO`). Erster `services`-Include im Persistenz-
  Adapter (Reuse der rise-Formel statt Duplikat). Kein Schema-Wechsel.
  **Treppen-Familie (016a/b/c) komplett → welle-2 closure-reif.** `MR-006`: keine
  HIGH (2 MED eingearbeitet). make gates grün (114 Tests).
- **Welle-2-bauteile abgeschlossen** (Closure 2026-06-14): alle parametrischen
  Bauteile über die Wände hinaus — Türen/Fenster (automatische Wandöffnung),
  Dach (Sattel/Walm/Pult), Decken/Fundament (Platten + Ausschnitte), Treppen
  (gerade einläufig) — je Familie Lastenheft-AK-Schärfung + Implementierung
  (Domäne/Geometrie/Viewer/Edit-Ops) + Persistenz; **12 Slices** (013–016) in
  `done/`, getragen von der **`ADR-0011`-(#6)-Leitplanke** ohne neuen Grundsatz-ADR
  je Bauteil. **Meilenstein M2 erreicht** + `ACC-001`-Bauteil-Hälfte. Unabhängige
  Welle-Verifikation (keine HIGH; 1 MED Frontmatter-Status + 1 LOW DoD-Checkboxen
  behoben) + Carveout-Audit (keine aktiven); geometrielastige Code-Reviews je
  Familie (013b/014b/015b je 1 HIGH gefixt, 016b keine HIGH — Test-Orakel
  gehärtet). `make gates` grün am HEAD (116/116, Coverage 92,3 %). Ergebnisnotiz:
  `welle-2-results.md`.
- **Steering-Festschreibung `MR-009` + `MR-010`** (`harness/conventions.md`,
  Pointer in `AGENTS.md` §4/§5; vor welle-3): **`MR-009`** „geometrielastiges
  Code-Review vor Welle-Closure" — 4× Vorkommen (013b/014b/015b je 1 HIGH trotz
  grüner Gates, 016b keiner); unabhängiges Code-Review der Geometrie-Korrektheit
  gegen die Spec (Winding/Bündigkeit/Maße/Totalität), HIGH blockiert Closure +
  geometrische Invarianten-Sonden in den AK-Tests. **`MR-010`** „Lastenheft-Header-
  Version == oberste §9-Historie-Zeile" — 3× Drift (013a/014a/015a, erst 016a
  korrigiert); `MR-006`-Linse prüft den Header-Nachzug, computational d-check-/
  make-Regel als Promotion-Ziel.
- slice-017a — Auswertung & Material: Entscheidung/Schärfung (welle-3-Leitplanke,
  `LH-FA-EVL-001`..006 + `LH-FA-MAT-001`/002/003/005/006, Lastenheft 0.1.7).
  **`ADR-0012` „Evaluations-Architektur" accepted** (neuer `EvaluatePort`
  read-only/pull; Fläche = Shoelace-Raum-Netto, **Netto-Volumen analytisch im
  Kern** = Footprint·Höhe − geklemmtes Öffnungsvolumen, **kein OCC-`GProp`**;
  Material nur konsumierte Eingabe). Zwei unabhängige Reviews: `MR-006`-Plan-Review
  des Projektinhabers (1 HIGH — ADR auf Evaluation verengt, Material→Spec) +
  **unabhängiger ADR-Text-Review** (1 HIGH — Volumen-Falle Roh-Prisma/Miter-
  Doppelzählung gefixt; `architecture.md`-Nachzug EVL→`EvaluatePort`;
  `windows.frame_material`-Freitext aus EVL-004 ausgenommen). Spec §1
  `LH-FA-EVL-001.a` + §2.1 `model::Material`/FK-Autorität + §3 `LIVING_AREA_FACTOR`.
  Reine Entscheidung/Spec (kein Code; `MR-008` lösungsfrei; `MR-010` Header). DRW →
  welle-5. make gates grün (116 Tests).
- slice-017b — Auswertung Flächen: **`EvaluatePort` implementiert** (erste
  Auswertungs-Implementierung, `LH-FA-EVL-001`/003, `ADR-0012`). `floorArea(StoreyId)`
  (Netto-Grundfläche je Raum + Summe je Geschoss) und `livingArea()` (Wohnfläche
  gebäudeweit) als **read-only-Aggregation** der bereits in der Raumerkennung
  berechneten `Room.net_area_mm2` (`ADR-0007`), mm²→m²; reiner Ergebnis-Werttyp
  `model::AreaReport` (Per-Raum + Summe), `kLivingAreaFactor=1` (spez. §3); `const`
  ⇒ kein `op`, kein `GeometryKernelPort`, keine Mutation. Vom `StructureEditService`
  getragen (hält `rooms_`). Unabhängiger `MR-006`-Plan-Review (keine HIGH; MED-1
  Per-Raum-Flächen, MED-2 `make arch-check`-Wortlaut, LOW-1 read-only-stabil, LOW-2
  Loch-Ring eingearbeitet). **`MR-009` n/a** (reine Aggregation, keine neue
  Geometrie — greift für EVL-002 Volumen/017c). make gates grün (122 Tests).

### Notes
- Dieses CHANGELOG ist eine bewusste Abweichung von der Kurs-Baseline (die
  Änderungs-Historie verteilt auf Spec-§Historie, ADR-Geschichte, Slice-Closure-
  Notizen und Welle-Results führt). Begründung und Pflege-Regel: `harness/conventions.md` `MR-004`.
