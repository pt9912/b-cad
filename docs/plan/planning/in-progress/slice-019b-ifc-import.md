---
id: slice-019b
titel: IFC-Import — ModelImporterPort + ExchangeService + IfcImportAdapter (SPF-Subset-Codec)
status: in-progress
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0013](../../adr/0013-ifc-bibliothek.md)]
---

# Slice 019b: IFC-Import (Codec + Adapter + Use-Case)

**Status:** in-progress (2026-06-16). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen
(**0 HIGH / 3 MED / 3 LOW / 3 INFO**; implementierungs-bereit, Start nicht
blockiert) — MED/LOW eingearbeitet.

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)):**
[Report](../../../reviews/2026-06-16-slice-019b-plan.md) — **keine HIGH**; die
zentrale Behauptung „Regel F gegenstandslos / A+B isolieren den hand-gerollten
Codec" als **faktisch korrekt** bestätigt (C/D/E gaten externe Header, Option D
hat keinen). Eingearbeitet: **MED-1** ([`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Code-String wird **im Adapter**
verfasst — Muster `ioCodeFor` der Persistenz, **nicht** im Service; Port-Typ
bleibt neutral), **MED-2/3** (§1 hat **keine** Elevation→Höhe-Regel; `Storey`
trägt kein `elevation_mm` → 019b ergänzt eine **kleine §1-Mapping-Mechanik**
[Geschoss-Höhe aus Elevation-Differenz, Elevation **transient**, nicht
gespeichert]; Spec nicht Lastenheft → [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-zulässig im Impl-Slice), **LOW-1**
(Subset-fremde Entität **vorhanden** → **übersprungen**, kein Wurf — dritte
Klasse explizit). LOW-2 (Adapter-[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) deckt sich nun mit architecture §5),
LOW-3 (Frontmatter-Link-IDs = `ids`-Artefakt, `make docs-check` grün). **INFO-3
übernommen:** Implementierung als **Zwei-Commit-Split** (i Codec/SPF-Reader,
ii Mapping+Service+Tests; Muster 016b) für Sitzungs-Review-barkeit.

**Welle:** welle-4-austausch (zweiter Slice; Implementierungs-Hälfte des
IFC-Strangs nach slice-019a-Schärfung).

**Bezug:** [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) (IFC-Import, in slice-019a auf AK-Niveau geschärft;
Mechanik in `spezifikation.md` §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)). **Parametrisiert auf
[ADR-0013](../../adr/0013-ifc-bibliothek.md)** (IFC-Backend = selbst getragener IFC-SPF-Subset-Codec, Option D) und
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt; Format-Backend adapter-lokal; Port = Kern-Hoheit). **Kein
neuer ADR;** eine **kleine §1-Mapping-Mechanik** (Geschoss-Höhe aus Elevation,
MED-2/3) wird in 019b ergänzt (Spec, **nicht** Lastenheft → [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-zulässig im
Impl-Slice) — sonst Implementierung gegen die entschiedene 019a-Spec.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-16.

**Schnitt-Herkunft:** Implementierungs-Hälfte des IFC-Strangs (Muster
013b/014b/016b). Geschnitten nach **Lieferwert**: dieser Slice liefert den
**Import** ([LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) end-to-end) — eine IFC-Datei wird zu einem b-cad-Modell.
Der **Export** ([LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002), Roundtrip, [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)) ist **slice-019c** (eigener
Lieferwert; IO-001-AK ist ohne Export vollständig prüfbar).

---

## 1. Ziel

Der **IFC-Import** wird lauffähig: eine valide IFC-SPF-Datei im welle-4-Subset
(Geschosse + gerade Wände) wird über den IO-Adapter gelesen und auf ein
`model::Building` abgebildet — **anzahl-treu**, **atomar**, **total** (gemäß
`spezifikation.md` §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)). Der Codec ist **hand-gerollt** (keine
externe IFC-Bibliothek, [ADR-0013](../../adr/0013-ifc-bibliothek.md) Option D) und lebt **ausschließlich** im
IO-Adapter; der Kern bleibt format-frei ([ADR-0001](../../adr/0001-hexagonale-architektur.md)).

## 2. Definition of Done

- [ ] **Ports + `ExchangeService` (Import-Use-Case, Kern).** Neuer Driving-Port
      `ExchangeModelPort` (`importModel(path, format)` → `model::Building`) +
      Driven-Port `ModelImporterPort` (`read(path)` → `model::Building`; bei
      Parse-/Format-Fehler **neutrale `std::runtime_error`** — Muster
      `ProjectRepositoryPort`, **kein Backend-Typ-Leck**, [ADR-0001](../../adr/0001-hexagonale-architektur.md)). Den
      **[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Code-String verfasst der Adapter** (in die neutrale Exception,
      Muster `ioCodeFor` der Persistenz `sqlite_project_repository.cpp` — eine
      Spec-Fehler-Zeichenkette ist **kein** Backend-Typ; deckt sich mit
      architecture §5 „IO-Adapter → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)", MED-1/LOW-2). Der `ExchangeService`
      (`src/hexagon/services/`) implementiert `ExchangeModelPort`, dispatcht je
      `format` zum Importer, **propagiert** den Fehler und loggt
      `event=import_rejected` (kein Teil-Import). **Pure Domäne** — `make arch-check`
      Regel A gewahrt (kein IFC-/SPF-Symbol im Kern). `src/main.cpp` verdrahtet
      den Adapter (Composition Root).
- [ ] **`IfcImportAdapter` (`src/adapters/io/`) implementiert `ModelImporterPort`**
      gemäß §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import): **SPF-Subset-Leser** (ISO-10303-21-Tokenizer +
      Entitäts-Graph nach `#id`) + **Domänen-Mapping** `IfcBuildingStorey`→
      `Storey`, `IfcWall`/`IfcWallStandardCase`→`Wall` (Achs-Repräsentation →
      `start`/`end`; `IfcMaterialLayerSet`-Dicke → `thickness_mm`; Extrusions-/
      Höhenmaß → Wand-`height_mm`; Containment `IfcRelContainedInSpatialStructure`
      → `storey_id`; `IfcRelAggregates` für die Spatial-Kette). **Geschoss-Höhe
      (MED-2/3):** `IfcBuildingStorey.Elevation` wird **transient** gelesen und
      `Storey.height_mm` = Differenz zur nächsthöheren Geschoss-Elevation (oberstes
      Geschoss → Wandhöhe/Default) — **Elevation wird nicht gespeichert** (das
      b-cad-Modell ist höhen-basiert, `Storey` trägt kein `elevation_mm`). Diese
      Mapping-Mechanik wird als **kleiner §1-Zusatz** zu [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) ergänzt
      (Spec, **nicht** Lastenheft → [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-zulässig; + `spezifikation-historie.md`).
      **Drei Eingabe-Klassen (LOW-1):** (a) Subset-Entität → gemappt; (b) **Subset-
      fremde Entität vorhanden** (`IfcDoor`/`IfcWindow`/`IfcRoof`/`IfcSlab`/
      `IfcStair`) → **übersprungen, kein Wurf** (solange die räumliche Pflicht-
      Struktur trägt); (c) Nicht-IFC/kaputt/fehlende Pflicht-Referenz → **Wurf**
      ([`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). **atomar** (vollständiges `Building` oder Wurf, kein Teil-Import);
      strukturlos/leer → leeres `Building` (Totalität). **Hand-gerollt, keine
      externe IFC-Lib** → **keine** neue `find_package`-/CMake-Dependency; nur
      `src/adapters/CMakeLists.txt` um die io-Quelle(n) ergänzt.
- [ ] **Schicht-Isolation belegt (arch-check) — ehrliche Sensor-Deckung.** Der
      hand-gerollte Codec lebt **nur** in `src/adapters/io/`, isoliert durch
      **Regel A** (Kern importiert keinen Adapter) **+ Regel B** (kein Adapter
      importiert einen anderen). Eine **dedizierte Regel F** (analog C/D/E) ist
      **gegenstandslos**: C/D/E gaten **externe Bibliotheks-Header** (OCC/SQLite/
      Qt), Option D hat **keine** externe IFC-Lib. Die [ADR-0013](../../adr/0013-ifc-bibliothek.md)-„io-Regel"-
      Folgepflicht wird damit **auf den externen-Bibliotheks-Re-Eval umdatiert**
      (erst eine adoptierte IFC-Lib braucht ein Header-Gate analog C/D/E) — in
      Closure + ADR-Index dokumentiert (kein Über-Versprechen, Modul 13).
- [ ] **AK-Tests [`LH-FA-IO-001`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) (Happy/Boundary/Negative) + Adapter-Pfad-
      Integrationstest.** Codec-Ebene: minimale `.ifc`-Fixture (Raw-String-Literal,
      kein Datei-Abhängigkeit) → `Building`, **Geschoss-/Wand-Anzahl == Quelle**;
      leere/strukturlose Datei → leeres Modell; Nicht-IFC → Wurf. **Integration
      über den echten Pfad** `ExchangeService`→`ModelImporterPort`→
      `IfcImportAdapter`: Datei → `Building` (Anzahl == Quelle) **und** Nicht-IFC →
      [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter (welle-3-Lehre slice-015b:
      Integrationspfad ungeübt trotz grüner Gates — 019a-[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Verschärfung).
      `make gates` grün (arch-check inkl. io, Coverage ≥ 70 %); **unabhängiges
      Code-Review** (Parsing/Format/Mapping/[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Korrektheit, höhere Latte —
      Klasse slice-015c/017e). **[`MR-009`](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** (kein Geometrie-Erzeugen, nur
      Parameter-Mapping). **Nicht Teil:** slice-019c (Export `ModelExporterPort` +
      SPF-Writer + Roundtrip-AK + [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)); STEP/STL/DXF/PDF/PNG.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/exchange_model_port.{h}` | neu | Driving-Port `importModel(path, format)` |
| `src/hexagon/ports/driven/model_importer_port.{h}` | neu | Driven-Port `read(path)` → Building (neutraler Wurf) |
| `src/hexagon/services/exchange_service.{h,cpp}` | neu | Import-Use-Case: Format-Dispatch + [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Grenze |
| `src/adapters/io/ifc_import_adapter.{h,cpp}` | neu | `ModelImporterPort`-Impl (Mapping) |
| `src/adapters/io/ifc_spf_reader.{h,cpp}` | neu (ggf.) | SPF-Tokenizer + Entitäts-Graph (Codec, von Mapping trennbar) |
| `src/adapters/CMakeLists.txt` | ändern | io-Quelle(n) in `bcad_adapters` (keine neue Dependency) |
| `src/main.cpp` | ändern | `IfcImportAdapter` ↔ `ExchangeService` verdrahten |
| `tests/adapters/test_ifc_import.cpp` | neu | AK + Integrationstest (`.ifc`-Raw-Fixture) |
| `tests/CMakeLists.txt` | ändern | Test registrieren |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) kleiner Zusatz: Geschoss-Höhe aus Elevation-Differenz (MED-2/3) |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-019b) |
| `spec/architecture.md` | ggf. ändern | Ports schon deklariert (§1.2/§4) — voraussichtlich **unverändert**, sonst Begründung in Closure |
| `docs/reviews/{2026-06-16-slice-019b-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: die Spec (§1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)) ist
  mit slice-019a entschieden, das Domänen-Modell (`Building`/`Storey`/`Wall`)
  und der `src/adapters/io/`-Ort liegen vor; **keine** externe Dependency.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Code-Review 0 HIGH, Closure-Notiz mit
  Lerneintrag → **slice-019c** (IFC-Export) wird startbar.

## 6. Risiken und offene Punkte

- **Geschoss-Höhe-Mapping (MED-2/3, im Review entschieden → DoD-2):** §1
  [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) trägt **keine** Elevation→Höhe-Regel und `model::Storey` kein
  `elevation_mm`. **Auflösung:** 019b ergänzt einen **kleinen §1-Zusatz** (Höhe =
  Elevation-Differenz, Elevation transient/nicht gespeichert) — Spec, nicht
  Lastenheft → [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-zulässig im Impl-Slice; + `spezifikation-historie.md`-Zeile.
  Kein Domänen-Schema-Wechsel (höhen-basiertes Modell).
- **Wand-Attribute ohne IFC-Quelle (benannte Lücke):** `Wall.type` (Innen/Aussen/
  Trag) hat keine direkte IFC-Entsprechung → **Default `Innen`**; `material_id` =
  `nullopt` (kein Material-Bibliotheks-Mapping im Subset). Ehrlich benennen, kein
  stiller Vollumfang.
- **SPF-Parser-Robustheit (hand-gerollt, [ADR-0013](../../adr/0013-ifc-bibliothek.md)-Contra-D):** der Subset lehnt
  Real-Datei-Varianten ab; Totalität/Atomarität sind gegen das Subset definiert,
  nicht gegen beliebiges IFC. Re-Eval auf eine echte IFC-Lib bleibt benannt
  ([ADR-0013](../../adr/0013-ifc-bibliothek.md)). Die AK-Fixtures decken den **entschiedenen** Subset, nicht „IFC
  allgemein".
- **arch-check-„Regel F"-Gegenstandslosigkeit (Sensor-Ehrlichkeit):** s. DoD-3 —
  im Review bestätigen lassen, dass A+B die Isolation tragen und keine
  Header-Gate-Regel für Option D existiert (Modul 13: keine Sensor-Deckung
  überversprechen).
- **Slice-Größe (Session-Review-Grenze, INFO-3 → übernommen):** vier neue
  Komponenten (2 Ports + Service + Adapter/Codec + Tests) sprengen realistisch
  **eine** Sitzung. **Adoptiert:** Implementierung als **Zwei-Commit-Split** —
  **i** SPF-Reader/Codec (Tokenizer + Entitäts-Graph, isoliert testbar), **ii**
  Adapter-Mapping + Ports + `ExchangeService` + Tests + Verdrahtung (Muster
  welle-2 016b/015b). Import bleibt **end-to-end** der Lieferwert (DoD-1
  „Ports+Service-Skelett" ist nicht eigenständig wertvoll).
- **[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Grenze (MED-1 → entschieden → DoD-1):** der **Adapter** verfasst
  den [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Code-String in die neutrale Exception (Muster `ioCodeFor`,
  Persistenz; Spec-Fehlercode ist kein Backend-Typ), der **Service** propagiert +
  loggt. Deckt sich mit architecture §5 (IO-Adapter → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)); Port-Typ bleibt
  neutral ([ADR-0001](../../adr/0001-hexagonale-architektur.md)).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Code / IO-Adapter

- **Modus:** GF; **Konventionen-Dichte:** hoch — neuer Driven-Adapter nach Muster
  `persistence`/`geometry` (Port-Impl, neutraler Wurf, kein Backend-Leck);
  `arch-check` A/B + die Adapter-CMake-Konvention tragen. **Phase-Reife:** Phase 4
  (Adapter-Schicht etabliert, mehrfach geübt). **Evidenz-/Diskrepanz-Risiko:**
  niedrig (Greenfield-Adapter, keine Bestands-Divergenz). **Reconciliation:**
  keine; Folge-Slice 019c.

### Sub-Area: Test-Infrastruktur

- **Modus:** GF; **Dichte:** hoch — GoogleTest-Harness + `LH-FA-*`-benannte
  AK-Tests + Integrationstest-Muster (welle-2/-3) stehen; `.ifc`-Fixture als
  Raw-String-Literal (kein neues Fixture-Verzeichnis). **Phase-Reife:** Phase 4.
  **Risiko:** niedrig.

## 8. Closure-Notiz

*(wird bei `done` ausgefüllt — Closure-Kriterien beobachtbar, Lerneintrag in
einer der drei Formen: geschärfte Regel · neuer Sensor · benannte Spec-Lücke.)*
