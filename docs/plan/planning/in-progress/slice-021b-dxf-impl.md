---
id: slice-021b
titel: DXF-Import/-Export — Implementierung (io-residenter DXF-Subset-Codec + Adapter + Import-Dispatch-Kern-Erweiterung)
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003), [LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0013](../../adr/0013-ifc-bibliothek.md), [ADR-0015](../../adr/0015-dxf-backend.md)]
---

# Slice 021b: DXF-Import/-Export — Implementierung

**Status:** in-progress. **Vor Start:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review (Reviewer ≠ Autor) — **HIGHs blockieren den Start**.

**Welle:** welle-4-austausch (DXF-Strang, Implementierungs-Hälfte; Muster
slice-019b/c [IFC]). Folgt auf [slice-021a](../done-archive/slice-021a-dxf-ak-spec.md) (AK-Schärfung, done).

**Bezug:** [LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003) (DXF-Import, AK) +
[LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004) (DXF-Export, AK) +
`spec/spezifikation.md` §1 [`LH-FA-IO-003.a`](../../../../spec/lastenheft.md#lh-fa-io-003)
(Mapping). **Innerhalb [ADR-0015](../../adr/0015-dxf-backend.md)** (Backend entschieden:
selbst getragener DXF-Subset-Codec **Option D**, io-resident, 2D-Grundriss);
[ADR-0013](../../adr/0013-ifc-bibliothek.md) (Option-D-/io-resident-Präzedenz, Codec-Muster IFC-SPF),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt; Format-Codec adapter-lokal;
Port-/Dispatch-Mechanik = Kern-Hoheit → **dieser** Slice fixiert die Import-Dispatch-Form).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-18.

**Schnitt-Herkunft:** Implementierungs-Hälfte des DXF-Strangs (Muster 019b/c). Die
AK + das Mapping sind in slice-021a entschieden; hier entsteht der lauffähige Codec +
die zwei Adapter + die Kern-Dispatch-Erweiterung + die AK-/Roundtrip-/Integrationstests.

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start), Reviewer ≠ Autor):**
**0 HIGH / 3 MED / 3 LOW / 2 INFO — Start nicht blockiert**
([Report](../../../reviews/2026-06-18-slice-021b-plan.md)). Eingearbeitet:
**MED-1** (DoD-1: der `ImporterMap`-Lookup-Miss trägt [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) **mit** `event=import_rejected`;
der bestehende Test `ImportOfStlRejectedAsExportOnly` muss **grün bleiben**, nicht nur
kompilieren), **MED-2** (DoD-3/6b: die Negative/Boundary-Mitte „valide Gruppencode-Paare ohne
`ENTITIES`" ist als **Boundary** gepinnt + getestet), **MED-3** (DoD-4: je `LINE` trägt das
Geschoss-`LAYER` als **Gruppencode 8** — die Roundtrip-Achse der Geschoss-Anzahl),
**LOW-1** (DoD-3: `LWPOLYLINE` → je Segment eine Wand), **LOW-2** (§3/DoD-7:
`architecture.md`-Nachzug explizit geprüft), **LOW-3** (§6: vier Dateien / **acht**
Ctor-Aufrufe). INFO = Bestätigungen.

---

## 1. Ziel

Die in [slice-021a](../done-archive/slice-021a-dxf-ak-spec.md) geschärften DXF-Anforderungen
[LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003) (Import) +
[LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004) (Export) **lauffähig** machen —
**innerhalb** des von [ADR-0015](../../adr/0015-dxf-backend.md) entschiedenen Backends:
ein **io-residenter ASCII-DXF-Subset-Codec** (symmetrischer Reader + Writer, Muster
IFC-SPF), zwei Adapter hinter `ModelImporterPort`/`ModelExporterPort`, ein neues
Austauschformat `ExchangeFormat::Dxf` und die **Kern-Erweiterung des Import-Dispatch**
([ADR-0015](../../adr/0015-dxf-backend.md)-Review-MED-1). Subset = **gerade Wand-Achsen
als 2D-Linien je Geschoss-`LAYER`**; Import → **Default-Höhe/-Dicke** (DXF trägt keine,
benannte Lücke).

## 2. Definition of Done

- [x] **DoD-1 — Kern: `ExchangeFormat::Dxf` + Import-Dispatch als Registry.**
      `exchange_model_port.h`: Enum um `Dxf` erweitert (+ Kommentar). `exchange_service.{h,cpp}`:
      der Import-Dispatch wird **symmetrisch zur bestehenden `ExporterMap`** auf eine
      **`ImporterMap` (`Format → const ModelImporterPort*`)** umgestellt — der `switch` und die
      Einzel-Referenz `ifc_importer_` entfallen; `importModel` schlägt den Importer in der Map
      nach (fehlt/`nullptr` → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
      `event=import_rejected`; deckt das bisherige „STEP/STL export-only"-Verhalten). **Pure Domäne**
      (kein DXF-/SPF-Symbol; [arch-check](../../../../harness/README.md#sensors-feedback-gates) Regel A).
      **Begründung der Form** ([ADR-0001](../../adr/0001-hexagonale-architektur.md)-Kern-Hoheit,
      [ADR-0015](../../adr/0015-dxf-backend.md)-Review-MED-1): Registry statt zweiter Referenz =
      die etablierte `ExporterMap`-Symmetrie, skaliert auf weitere Importer. **MED-1:** die
      Lookup-Miss-Message führt **beide** Token ([`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) **und** `event=import_rejected`) —
      der bestehende Test `ImportOfStlRejectedAsExportOnly` (`test_step_stl_export.cpp`,
      Token-Doppel-Assertion) muss **grün bleiben**, nicht nur kompilieren.
- [x] **DoD-2 — DXF-Codec io-resident (`adapters/io/`), isoliert testbar.** Generischer
      ASCII-DXF-Gruppencode-Codec, **format-agnostisch** (kennt keine b-cad-Domäne; Muster
      `ifc_spf_reader`/`ifc_spf_writer`): `DxfReader` (Tokenizer Gruppencode/Wert-Paare →
      `SECTION`/`ENTITIES`-Struktur) + `DxfWriter` (Sektionen + `LAYER`-Tabelle + `ENTITIES`
      akkumulieren → vollständiger DXF-Text, deterministisch). **Ziel-Profil:** ASCII-DXF
      **R12 (AC1009)**, `LINE` als Achs-Entität (versions-robust; `LWPOLYLINE` beim Import
      zusätzlich toleriert, Export nur `LINE` — slice-021a-LOW-1). **Kein** externer/OCC-Header
      (Regel A/B isolieren ihn — „Regel F gegenstandslos", wie IFC). **Kein** neuer
      `find_package`/apt-Eintrag (reines C++/STL — DoD-7).
- [x] **DoD-3 — `DxfImportAdapter` (`ModelImporterPort`).** Mappt das ENTITIES-`LINE`/
      `LWPOLYLINE`-Subset auf gerade Wände: je Achs-Linie eine `Wall` (start/end aus Gruppencode
      10/20 bzw. 11/21); `LWPOLYLINE` mit n Vertices → **n−1** gerade Wände je Vertex-Paar (LOW-1), `LAYER` (Code 8) → Geschoss (ein `Storey` je distinkter Layer,
      deterministische Reihenfolge); **Default-Höhe/-Dicke** (`kDefaultStoreyHeightMm`/
      `kDefaultWallThicknessMm`). **Atomar** (vollständiges In-Memory-`Building` zuerst, Übergabe
      erst nach fehlerfreiem Parse). **Totalität** (spez. §1): leere/strukturlose DXF → leeres
      `Building` **ohne Wurf**; nicht-DXF/kaputt (Gruppencode-Strom nicht wohlgeformt) → neutrale
      `std::runtime_error` mit vorangestelltem [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
      **kein Teil-Import**. Subset-Skip (DIMENSION/HATCH/BLOCK/INSERT/TEXT/ARC/CIRCLE/3D)
      übersprungen. **MED-2 (scharfe Negative/Boundary-Kante):** ein **wohlgeformter**
      Gruppencode-Strom **ohne** `ENTITIES`/Achs-Entitäten zählt als **Boundary** (leeres Modell,
      kein Wurf); **nur** ein **nicht** wohlgeformter Strom (nicht-numerische Code-Zeile / ungerade
      Paarung) ist Negative ([`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)).
- [x] **DoD-4 — `DxfExportAdapter` (`ModelExporterPort`).** Domäne → DXF: je Geschoss eine
      `LAYER`, je gerade Wand eine `LINE` (Achse, z=0), **die ihr Geschoss-`LAYER` als
      Gruppencode 8 trägt** (MED-3 — Roundtrip-Achse der Geschoss-Anzahl; der Import liest Code 8
      je Entity, nicht die `LAYER`-Tabelle). **Atomar** (Temp + `fsync` + Rename,
      Muster `ifc_export_adapter`); nicht beschreibbarer Zielpfad → neutrale `std::runtime_error`
      mit [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
      (`event=io_no_permission`), **kein Teil-Export**, Zielpfad bleibt intakt. Leeres Modell →
      gültige, (annähernd) leere DXF.
- [x] **DoD-5 — Composition Root (`src/main.cpp`).** `DxfImportAdapter`/`DxfExportAdapter`
      konstruiert; `ImporterMap` `{Ifc, Dxf}` + `ExporterMap` um `{Dxf, …}` ergänzt. **CLI-Parität**
      `--import-dxf <pfad>` / `--export-dxf <pfad>` (headless belegbar, Muster `--import-ifc`/
      `--export-ifc`).
- [x] **DoD-6 — AK-Tests + Roundtrip + Integration über den ECHTEN Pfad.** Test-Namen tragen
      [`LH-FA-IO-003`](../../../../spec/lastenheft.md#lh-fa-io-003)/
      [`LH-FA-IO-004`](../../../../spec/lastenheft.md#lh-fa-io-004):
      (a) **Codec isoliert** (`test_dxf_codec`: Writer→Reader-Roundtrip, ohne Adapter/Building);
      (b) **Import-AK** (`test_dxf_import`): Happy (Wand-Anzahl == Quelle, Default-Höhe/-Dicke),
      Boundary (leere/strukturlose DXF → leeres Modell), Negative (nicht-DXF →
      [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Import),
      Subset-Skip + **MED-2-Mitte** (wohlgeformte Gruppencode-Paare ohne `ENTITIES` → leeres
      Modell, kein Wurf) — **über `ExchangeService` → `ModelImporterPort` → `DxfImportAdapter`**
      (welle-3-Lehre slice-015b, [ADR-0015](../../adr/0015-dxf-backend.md) Fitness Function);
      (c) **Export-AK + Roundtrip** (`test_dxf_export`): Export → Re-Import erhält **Wand-Achsen-
      Anzahl je Geschoss + Achs-Lage** (nicht Höhe/Dicke), Boundary (leeres Modell → gültige DXF),
      Negative (nicht beschreibbarer Pfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
      durch den echten Adapter) — **über `ExchangeService` → `ModelExporterPort`**. Die drei
      bestehenden Tests, die `ExchangeService` konstruieren (`test_ifc_import`/`test_ifc_export`/
      `test_step_stl_export`), werden auf die neue `ImporterMap`-Signatur nachgezogen.
- [x] **DoD-7 — Gates grün, keine neue Dependency, Doku/Index nachgezogen.** `make gates` grün
      (arch-check inkl. `io/`, build **ohne** neuen `find_package`/apt, lint 0, AK-Tests grün,
      coverage-gate ≥ Schwelle, docs-check, gate-consistency); `make schema-check` unberührt.
      **Unabhängiges Code-Review** (Reviewer ≠ Autor) vor Closure — **0 HIGH** (Parität IFC-io-Slices
      019b/c; **kein** [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Geometrie-Trigger,
      da 2D-Text-I/O ohne neue Solid-Geometrie — siehe §6). **Closure:**
      [ADR-Index](../../adr/README.md)-Folgepflicht-Zeile „[ADR-0015](../../adr/0015-dxf-backend.md) Impl"
      abhaken; `CHANGELOG.md` (ein Eintrag, [MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog));
      `roadmap.md`-Fortschritt (DXF-Impl ✓); Closure-Notiz §8 mit Lerneintrag. **Kein** Lastenheft-/
      Spec-Inhalt geändert (in slice-021a erledigt) — Impl ändert **nur Code + Index/Changelog**.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/exchange_model_port.h` | ändern | `enum ExchangeFormat` um `Dxf`; Kommentar (DXF = io-resident, Import+Export) |
| `src/hexagon/services/exchange_service.h` | ändern | `ImporterMap`-Alias; Ctor `(ImporterMap, ExporterMap)`; Member `importers_` statt `ifc_importer_` |
| `src/hexagon/services/exchange_service.cpp` | ändern | `importModel` = Map-Lookup (fehlt → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)); `switch` entfällt |
| `src/adapters/io/dxf_reader.{h,cpp}` | neu | generischer DXF-Gruppencode-Tokenizer (format-agnostisch) |
| `src/adapters/io/dxf_writer.{h,cpp}` | neu | generischer DXF-Emitter (Sektionen + `LAYER` + `ENTITIES`) |
| `src/adapters/io/dxf_import_adapter.{h,cpp}` | neu | `ModelImporterPort`: DXF→`Building` (atomar/total, [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) |
| `src/adapters/io/dxf_export_adapter.{h,cpp}` | neu | `ModelExporterPort`: `Building`→DXF (atomar Temp+Rename, [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) |
| `src/main.cpp` | ändern | Adapter konstruieren + verdrahten; `--import-dxf`/`--export-dxf` |
| `src/adapters/CMakeLists.txt` | ändern | 4 neue `io/dxf_*.cpp` zu `bcad_adapters` |
| `tests/adapters/test_dxf_codec.cpp` | neu | Codec-Roundtrip isoliert |
| `tests/adapters/test_dxf_import.cpp` | neu | Import-AK + Integration |
| `tests/adapters/test_dxf_export.cpp` | neu | Export-AK + Roundtrip + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) |
| `tests/adapters/test_ifc_import.cpp`, `test_ifc_export.cpp`, `test_step_stl_export.cpp` | ändern | `ExchangeService`-Ctor auf `ImporterMap` nachziehen |
| `tests/CMakeLists.txt` | ändern | 3 neue `test_dxf_*.cpp` |
| `docs/plan/adr/README.md` | ändern (Closure) | [ADR-0015](../../adr/0015-dxf-backend.md)-Folgepflicht „Impl" → erfüllt |
| `CHANGELOG.md` | ändern (Closure) | ein Eintrag DXF-Impl |
| `docs/plan/planning/in-progress/roadmap.md` | ändern (Closure) | DXF-Impl ✓ |
| `spec/architecture.md` | ggf. ändern (LOW-2) | Nachzugsbedarf prüfen (`ImporterMap`/DXF-Adapter; Ports deklariert → vsl. unverändert), sonst `## Geschichte`-Provenance |
| `docs/reviews/2026-06-18-slice-021b-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |
| `docs/reviews/{2026-06-18-slice-021b-code-review}.md` | neu (Closure) | unabhängiges Code-Review |

**Commit-Schnitt** (Muster 019b/c, 020b): **(i)** DXF-Codec (`dxf_reader`/`dxf_writer`) +
`test_dxf_codec` isoliert; **(ii)** Adapter + Kern-`ImporterMap` + Composition Root +
Import-/Export-AK-Tests + Integration; **(iii)** Closure (ADR-Index/CHANGELOG/roadmap/
Closure-Notiz + Code-Review).

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review.
  [ADR-0015](../../adr/0015-dxf-backend.md) (accepted) + die AK ([slice-021a](../done-archive/slice-021a-dxf-ak-spec.md))
  sind die Vorbedingung; die IO-Ports sind deklariert, das Exporter-Registry existiert (slice-020b).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, unabhängiges Code-Review 0 HIGH, Closure-Notiz mit
  Lerneintrag → der **DXF-Strang** ist abgeschlossen. Danach (welle-4):
  Dächer/Treppen-STEP-B-Rep ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) ·
  PDF/PNG ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) · Welle-4-Verifikation → M4.

## 6. Risiken und offene Punkte

- **Kern-Signatur-Änderung (`ImporterMap`):** der `ExchangeService`-Ctor ändert sich
  (`ifc_importer&` → `ImporterMap`). Betrifft **vier Dateien / acht Ctor-Aufrufe** (main.cpp 1 +
  `test_ifc_import` 2 + `test_ifc_export` 2 + `test_step_stl_export` 3; LOW-3). Gefangen vom
  Compiler (`make build`) + den Tests; bewusste Wahl der symmetrischen Form
  ([ADR-0015](../../adr/0015-dxf-backend.md)-Review-MED-1, [ADR-0001](../../adr/0001-hexagonale-architektur.md)-Kern-Hoheit).
- **DXF-Format-Erkennung (Negative-AK):** „nicht-DXF/kaputt" braucht ein **mechanisches
  Ablehnungs-Prädikat** (Gruppencode-Strom: jede Code-Zeile eine Ganzzahl, gefolgt von einem
  Wert; sonst → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). Abgrenzung zu Boundary
  (leer/whitespace → leeres Modell, **kein** Wurf) wird in den Tests gepinnt. Risiko: ein zu
  laxes Prädikat akzeptiert Müll als „leere DXF". Mitigation: Negative-Test mit echter
  Nicht-DXF-Eingabe (z. B. Prosa) + Boundary-Test mit leerer/SECTION-loser DXF.
- **Roundtrip-Treue (nur Anzahl + Achs-Lage):** Export→Import erhält **nicht** Höhe/Dicke
  (DXF trägt sie nicht; Import defaultet) — das Test-Orakel prüft **Achsen-Anzahl je Geschoss
  + Achs-Koordinaten**, nicht `height_mm`/`thickness_mm`. Spec-konform (§1 benannte Lücke),
  aber Orakel darf **nicht** versehentlich Höhe/Dicke vergleichen.
- **Geschoss↔Layer-Treue:** Export schreibt je Geschoss eine Layer; Import rekonstruiert ein
  Geschoss je distinkter Layer. Roundtrip-Treue der **Geschoss-Anzahl** hängt daran, dass jede
  Geschoss-Layer mindestens eine Wand trägt (leere Geschosse erzeugen keine ENTITIES → gehen im
  Roundtrip verloren). **Benannte Lücke** — Test nutzt Geschosse mit Wänden; leere-Geschoss-Verlust
  wird in der Closure-Notiz benannt (analog IFC-/Subset-Lücken).
- **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) nicht getriggert:**
  es greift bei **neuer Solid-/Bauteil-Geometrie** (Kern/Geometrie-Adapter). DXF ist
  **2D-Text-I/O** ohne neue Solid-Geometrie → formal n/a (kein Geometrie-Trigger). Dennoch
  **unabhängiges Code-Review** vor Closure (Parität zu den IFC-io-Slices 019b/c, die ebenfalls
  eines hatten) — der Plan-Reviewer bestätigt die Einordnung.
- **DRY-Beobachtung (kein Scope-Aufriss):** der atomare Schreibpfad (`atomicWrite`/`ioCodeForErrno`)
  existiert bereits in `ifc_export_adapter` (und geometrie-resident in STEP/STL). DXF **spiegelt** ihn
  (etablierte Per-Adapter-Duplikation). Eine Auslagerung nach `io/atomic_file_write.*` ist eine
  **benannte, zurückgestellte** Vereinfachung (eigener Cleanup-Slice/`/simplify`), **nicht** Teil
  dieses Slice — sonst wächst der Diff über den AK-Umfang.
- **Spec/Lastenheft unverändert:** alle Vertrags-/Mapping-Aussagen sind in slice-021a (done)
  festgeschrieben; dieser Slice ändert **keine** Spec — nur Code + Index/CHANGELOG. (Falls die
  Implementierung eine Spec-Lücke aufdeckt, ist das ein **separater** Schärfungs-Schritt, nicht
  stilles Nachschärfen hier.)

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Import/Export-Adapter (`src/adapters/io/`)

- **Modus:** GF; **Konventionen-Dichte:** hoch — Format-Adapter-Konvention je Format hinter
  Importer/Exporter-Port (Muster IFC slice-019b/c), Codec **io-resident** (kein OCC,
  [arch-check](../../../../harness/README.md#sensors-feedback-gates) Regel A/B), atomarer
  Schreibpfad, [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/
  [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Konvention.
  **Phase-Reife:** Phase 4 (IFC-Vorbild etabliert). **Evidenz-/Diskrepanz-Risiko:** niedrig
  (entschiedenes Backend + AK). **Reconciliation:** keiner.

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF; **Dichte:** hoch (Pure-Domain-Regel, Port-Schnittstellen, kein Adapter-Import;
  Dispatch = Kern-Hoheit). **Phase-Reife:** Phase 4. **Risiko:** niedrig — die `ImporterMap`-Form
  spiegelt die bestehende `ExporterMap`.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-18):**

- **Codec (DoD-2):** io-residenter ASCII-DXF-Subset-Codec (R12/AC1009) — `DxfReader`
  (Tokenizer + Wohlgeformtheits-Grenze) + `DxfWriter` (Paar-Syntax, locale-freie Reals),
  format-agnostisch, **keine neue Dependency**. 8 isolierte Codec-Tests.
- **Adapter (DoD-3/4):** `DxfImportAdapter` (LINE/LWPOLYLINE → gerade Wände, ein Geschoss je
  Layer, Default-Höhe/-Dicke; atomar/total, [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) +
  `DxfExportAdapter` (je Wand eine LINE mit Geschoss-`LAYER` als Code 8, atomar Temp+fsync+Rename,
  [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)).
- **Kern (DoD-1):** `ExchangeFormat::Dxf` + Import-Dispatch auf eine **`ImporterMap`**
  (symmetrisch zur `ExporterMap`) — der IFC-`switch`-Sonderfall entfällt; STEP/STL bleiben
  export-only via Lookup-Miss ([`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
  **mit** `event=import_rejected`, MED-1); Kern format-frei (arch-check Regel A).
- **Composition Root (DoD-5):** beide DXF-Adapter verdrahtet, `--import-dxf`/`--export-dxf`.
- **Tests (DoD-6):** 14 neue Tests (Codec isoliert + Import-/Export-AK + Roundtrip + Integration
  über den echten `ExchangeService`-Pfad); 3 bestehende io-Tests auf `ImporterMap` migriert
  (bleiben grün, inkl. `ImportOfStlRejectedAsExportOnly`).
- **Gates (DoD-7):** `make gates` grün (201/201, Coverage 89,9 %, arch-check/lint/docs-check 0);
  `make schema-check` unberührt. **Unabhängiges Code-Review 0 HIGH**
  ([Report](../../../reviews/2026-06-18-slice-021b-code-review.md); 2 MED Test-Orakel-Lücken —
  teil-koordinierte LINE / LWPOLYLINE-Achs-Lage — vor Closure mit Asserts geschlossen).
  ADR-Index-Folgepflicht „[ADR-0015](../../adr/0015-dxf-backend.md) Impl" abgehakt; `CHANGELOG.md`
  + `roadmap.md` nachgezogen; `spec/architecture.md` **unverändert** (Ports/Komponenten +
  DXF-Provenance bereits aus slice-021a; `ImporterMap` ist internes Service-Detail — LOW-2).

**Lerneintrag:**

- **ImporterMap-Symmetrie (MED-1-Auflösung):** der Import-Dispatch wurde als Registry
  (`Format → ModelImporterPort*`) gespiegelt zur `ExporterMap` — der export-only-Vertrag von
  STEP/STL bleibt als **Lookup-Miss mit beiden Token** erhalten (kein impliziter `switch`-Zweig).
  Kern-Signatur-Änderung über **acht** Ctor-Stellen, vom Compiler fail-closed gefangen.
- **Benannte Subset-/Verlust-Lücken (DXF-2D):** DXF trägt keine Höhe/Dicke → Import defaultet;
  Roundtrip erhält nur Achsen-Anzahl + -Lage; leere Geschosse (Layer ohne Wand) gehen im
  Roundtrip verloren; Räume/Bemaßung/Schraffur/Blöcke/Text/Bögen/3D = Subset-Skip. Muster der
  IFC-Import-Geschoss-Höhe (transient/Default).
- **Akzeptierte LOWs (Code-Review):** `dxfReal` `%.12g` nicht round-trip-exakt jenseits 12
  signifikanter Stellen (mm-Subset irrelevant); `std::isspace` UB-frei via `unsigned char`-Cast
  (C-Locale korrekt); `DxfEntity::points()` generisch, aktiv nur für LWPOLYLINE.
- **DRY-Beobachtung (zurückgestellt):** der atomare Schreibpfad (`atomicWrite`/`ioCodeForErrno`)
  spiegelt `ifc_export_adapter` — Auslagerung nach `io/atomic_file_write.*` bleibt ein eigener
  Cleanup-Kandidat (`/simplify`), nicht in diesem Slice.

**Restrisiko / Nachfolge:** Dächer/Treppen-STEP-B-Rep
([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) ·
PDF/PNG ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) · Welle-4-Verifikation →
`done/welle-4-results.md` (M4).
