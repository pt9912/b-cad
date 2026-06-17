---
id: slice-019c
titel: IFC-Export — ModelExporterPort + ExchangeService.exportModel + IfcExportAdapter (SPF-Subset-Writer)
status: in-progress
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0013](../../adr/0013-ifc-bibliothek.md)]
---

# Slice 019c: IFC-Export (SPF-Writer + Adapter + Use-Case)

**Status:** in-progress (2026-06-17). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen
(**0 HIGH / 1 MED / 3 LOW / 3 INFO**; implementierungs-bereit, Start nicht blockiert).
Der zentrale Roundtrip-Anspruch (Export → 019b-Import erhält Anzahl) wurde gegen den
**echten** 019b-Importer verifiziert: die zwei werfenden Pflicht-Referenzen (exakte
`'Axis'`-`IfcPolyline` + `IfcRelContainedInSpatialStructure`) stehen beide im Export-DoD.
Eingearbeitet: **MED-1** (Importer matcht `RepresentationIdentifier` **byte-exakt** →
Writer schreibt exakt `'Axis'`/`'Body'`; vom Roundtrip-Test bewacht), **LOW-1**
([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-§4-Klammer auf „IFC-Export" erweitern), **LOW-2** (architecture §5 um
eine Export→[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Zeile ergänzen). LOW-3/INFO = Bestätigungen (Ports/ADR-
Folgepflicht/[`MR-008`](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)/Links korrekt).
[Report](../../../reviews/2026-06-17-slice-019c-plan.md).
**Unabhängiger Zweit-Pass (2026-06-17, Projektinhaber):** konvergiert (**0 HIGH /
1 MED / 4 LOW**); Roundtrip eigenständig gegen den echten 019b-Importer verifiziert.
**LOW-4 neu** eingearbeitet: die `spezifikation-historie.md`-Zeile ist **erforderlich**
(nicht „ggf."), da der §4-Edit (LOW-1) ein Spec-Stratum berührt ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)) — §3
nachgezogen. INFO-neu (architecture `## Geschichte` ohne [ADR-0013](../../adr/0013-ifc-bibliothek.md)) als
optionaler Zusatz beim LOW-2-Edit vermerkt.

**Welle:** welle-4-austausch (dritter Slice; Export-Hälfte des IFC-Strangs nach
slice-019b-Import). Schließt den IFC-Strang ab → [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien) erfüllbar.

**Bezug:** [LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002) (IFC-Export, in slice-019a auf AK-Niveau geschärft;
Mechanik in `spezifikation.md` §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) — **Sammelblock** Import+Export).
**Parametrisiert auf** [ADR-0013](../../adr/0013-ifc-bibliothek.md) (IFC-Backend = selbst getragener IFC-SPF-Subset-Codec,
Option D; **symmetrisch Lesen UND Schreiben in einer Code-Basis**) und [ADR-0001](../../adr/0001-hexagonale-architektur.md)
(Kern führt; Format-Backend adapter-lokal; Port = Kern-Hoheit). **Kein neuer ADR.**
Eine **kleine §1-Mapping-Mechanik** (Export-Elevation aus kumulierten Geschoss-Höhen,
Spiegel des 019b-Import-Zusatzes) wird ggf. ergänzt (Spec, **nicht** Lastenheft →
[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-zulässig im Impl-Slice).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-17.

**Schnitt-Herkunft:** Export-Hälfte des IFC-Strangs (Muster 019b). Geschnitten nach
**Lieferwert:** dieser Slice liefert den **Export** ([LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002), [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)) — ein
b-cad-Modell wird zu einer re-importierbaren IFC-Datei (Roundtrip). Der **Import**
([LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)) ist mit slice-019b geliefert und ist hier der **Roundtrip-Partner**
(Export → 019b-Import erhält Geschoss-/Wand-Anzahl).

---

## 1. Ziel

Der **IFC-Export** wird lauffähig: ein `model::Building` im welle-4-Subset (Geschosse
+ gerade Wände) wird über den IO-Adapter als **valide IFC-SPF-Datei** (ISO 10303-21,
IFC4) geschrieben — **atomar** (Temp + Rename, kein Teil-Export) und so, dass der
**019b-Import dieselbe Anzahl Geschosse und Wände** zurückgibt (Roundtrip,
[`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)). Der Writer ist **hand-gerollt** (keine externe IFC-Bibliothek,
[ADR-0013](../../adr/0013-ifc-bibliothek.md) Option D) und lebt **ausschließlich** im IO-Adapter; der Kern bleibt
format-frei ([ADR-0001](../../adr/0001-hexagonale-architektur.md)).

## 2. Definition of Done

- [ ] **Ports + `ExchangeService.exportModel` (Export-Use-Case, Kern).** `ExchangeModelPort`
      (driving) wird um `exportModel(building, path, format)` → `void` erweitert; neuer
      Driven-Port `ModelExporterPort` (`write(building, path)` → `void`; bei nicht
      beschreibbarem Zielpfad **neutrale `std::runtime_error`** — Muster
      `ProjectRepositoryPort`/`ModelImporterPort`, **kein Backend-Typ-Leck**, [ADR-0001](../../adr/0001-hexagonale-architektur.md)).
      Den **[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Code-String verfasst der Adapter** (Muster `ioCodeForErrno` der
      Persistenz `sqlite_project_repository.cpp`; eine Spec-Fehler-Zeichenkette ist
      **kein** Backend-Typ). Der `ExchangeService` dispatcht je `format` zum Exporter,
      **propagiert** den Fehler und loggt `event=io_no_permission`. **Pure Domäne** —
      `make arch-check` Regel A gewahrt (kein IFC-/SPF-Symbol im Kern). `src/main.cpp`
      verdrahtet den Adapter (Composition Root; ggf. `--export-ifc <pfad>`-Spiegel zu
      `--import-ifc`).
- [ ] **`IfcExportAdapter` (`src/adapters/io/`) implementiert `ModelExporterPort`**
      gemäß §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import): **SPF-Subset-Writer** (ISO-10303-21-Serializer:
      Kopf + HEADER `FILE_SCHEMA(('IFC4'))` + DATA + ENDSEC + END-ISO; `#id`-Vergabe) +
      **Domänen→IFC-Mapping**: räumliche Struktur `IfcProject` → `IfcBuilding` →
      `IfcBuildingStorey` (Komposition über `IfcRelAggregates`; **Elevation = kumulierte
      Geschoss-Höhe**, `storey[0]=0`), Längeneinheit mm (`IfcUnitAssignment`/`IfcSIUnit`
      `.METRE.`+`.MILLI.`), je Wand `IfcWall` (**nicht** deprecated `IfcWallStandardCase`)
      + `IfcProductDefinitionShape` mit **Axis**-`IfcShapeRepresentation` (`IfcPolyline`
      aus `start`/`end`) + **Body**-`IfcShapeRepresentation` (`IfcExtrudedAreaSolid.Depth`
      = `height_mm`), `IfcMaterialLayerSetUsage`/`IfcMaterialLayerSet`/`IfcMaterialLayer`
      (`LayerThickness` = `thickness_mm`) via `IfcRelAssociatesMaterial`, Wand→Geschoss
      via `IfcRelContainedInSpatialStructure`. **`RepresentationIdentifier` byte-exakt
      `'Axis'` bzw. `'Body'`** (MED-1: der 019b-Importer matcht per exakter String-
      Gleichheit + fester Attribut-Position — abweichende Schreibung/Reihenfolge ließe den
      Re-Import die Achse nicht finden und [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) werfen; vom Roundtrip-Test
      bewacht). **Atomar** (Temp + `fsync` + Rename, Muster
      Persistenz; bei Schreibfehler/Rechte → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), Zielpfad unverändert, **kein**
      Teil-Export). **Hand-gerollt, keine externe IFC-Lib** → **keine** neue
      `find_package`-/CMake-Dependency; nur `src/adapters/CMakeLists.txt` um die io-Writer-
      Quelle(n) ergänzt. **Benannte Subset-Lücke (Spiegel 019b):** nur Geschosse + gerade
      Wände werden geschrieben; Türen/Fenster/Dach/Decken/Treppen/Material-Bibliothek
      **nicht** (`Wall.type`/`material_id`-Override haben keine IFC-Subset-Entsprechung).
- [ ] **Roundtrip-AK [`LH-FA-IO-002`](../../../../spec/lastenheft.md#lh-fa-io-002) + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Negative + Adapter-Pfad-Integration.**
      Writer-Ebene: `Building` (Geschosse + Wände) → SPF-Text, analytisch geprüft
      (well-formed Entitäten); **Roundtrip-Orakel:** export → **019b-Import** ergibt
      **Geschoss-/Wand-Anzahl == Quelle** ([ADR-0013](../../adr/0013-ifc-bibliothek.md) Fitness Function: Roundtrip ohne echten
      Adapter). **Integration über den echten Pfad** `ExchangeService.exportModel` →
      `ModelExporterPort` → `IfcExportAdapter`: Datei schreiben → mit dem echten
      `IfcImportAdapter` re-importieren (Anzahl-Treue) **und** nicht beschreibbarer Pfad →
      [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter, Zielpfad unverändert (welle-3-Lehre
      slice-015b). `make gates` grün (arch-check inkl. io, Coverage ≥ 70 %); **unabhängiges
      Code-Review** (Serialisierung/Format/Mapping/[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Korrektheit, höhere Latte).
      **[`MR-009`](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** (kein Geometrie-Erzeugen, nur Serialisierung).
- [ ] **Schicht-Isolation belegt (arch-check) + [ADR-0013](../../adr/0013-ifc-bibliothek.md)-Export-Folgepflicht eingelöst +
      [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien).** Writer/Exporter lebt **nur** in `src/adapters/io/`, isoliert durch
      **Regel A + B** (Regel F bleibt **gegenstandslos** für Option D — wie 019b
      festgestellt; keine externe IFC-Lib-Header). Ggf. **kleiner §1-Zusatz** (Export-
      Elevation aus kumulierten Höhen, Spiegel des 019b-Import-Zusatzes; Spec, **nicht**
      Lastenheft → [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei); + `spezifikation-historie.md`). ADR-Index: [ADR-0013](../../adr/0013-ifc-bibliothek.md)-**Export**-
      Folgepflicht (`ModelExporterPort` + SPF-Writer + Roundtrip-AK) als **erfüllt**
      markiert. **[ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)** (Export nach IFC erfolgreich) durch den Roundtrip-Test belegt.
      **Nicht Teil:** STEP/STL-Export (OCC-nativ, Schwester-ADR), DXF, PDF/PNG; die
      welle-4-Closure ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)/M4) braucht zusätzlich den PDF-Strang.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/exchange_model_port.{h}` | ändern | `exportModel(building, path, format)` ergänzen |
| `src/hexagon/ports/driven/model_exporter_port.{h}` | neu | Driven-Port `write(building, path)` (neutraler Wurf) |
| `src/hexagon/services/exchange_service.{h,cpp}` | ändern | `exportModel`-Dispatch + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Grenze (Propagation) |
| `src/adapters/io/ifc_export_adapter.{h,cpp}` | neu | `ModelExporterPort`-Impl (Domänen→IFC-Mapping, atomar) |
| `src/adapters/io/ifc_spf_writer.{h,cpp}` | neu (ggf.) | SPF-Serializer (Entitäten → ISO-10303-21-Text), von Mapping trennbar |
| `src/adapters/CMakeLists.txt` | ändern | io-Writer-Quelle(n) in `bcad_adapters` (keine neue Dependency) |
| `src/main.cpp` | ändern | `IfcExportAdapter` ↔ `ExchangeService` verdrahten (ggf. `--export-ifc`) |
| `tests/adapters/test_ifc_export.cpp` | neu | Roundtrip-AK (export→019b-Import) + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) + Integration |
| `tests/adapters/test_ifc_spf_writer.cpp` | neu (ggf.) | Writer-Ebene isoliert (well-formed SPF, Determinismus) |
| `tests/CMakeLists.txt` | ändern | Test(s) registrieren |
| `spec/spezifikation.md` | ggf. ändern | §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) kleiner Zusatz: Export-Elevation aus kumulierten Höhen; **§4 [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Klammer um „IFC-Export" erweitern** (LOW-1) |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-019c) — **erforderlich** (LOW-4): der §4-[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Edit (LOW-1) berührt ein Spec-Stratum → datierte Historie-Zeile Pflicht ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths), Präzedenz 019a/019b) |
| `spec/architecture.md` | ändern | §5 Fehlermodell-Tabelle um eine **Export → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)**-Zeile ergänzen (Spiegel der Import-Zeile, LOW-2) + `## Geschichte`-Provenance-Zeile (architecture-Edit; **optional** dort „Austausch-Ports/IFC Import+Export → [ADR-0013](../../adr/0013-ifc-bibliothek.md)" ergänzen, INFO-neu — vorbestehende Lücke) |
| `docs/plan/adr/README.md` | ändern | [ADR-0013](../../adr/0013-ifc-bibliothek.md)-Export-Folgepflicht → erfüllt |
| `docs/reviews/{2026-…-slice-019c-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **Startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review (0 HIGH): slice-019b (Import) ist `done`
  (Roundtrip-Partner liegt vor), die Spec (§1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)) ist mit 019a/019b
  entschieden, der `src/adapters/io/`-Ort + Codec-Muster liegen vor; **keine** externe
  Dependency.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Code-Review 0 HIGH, Closure-Notiz mit Lerneintrag.
  Damit ist der **IFC-Strang** (Import + Export, [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)) abgeschlossen; offen für die
  welle-4-Closure (M4) bleiben STEP/STL · DXF · PDF/PNG ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)).

## 6. Risiken und offene Punkte

- **Roundtrip-Asymmetrie Höhe (benannt):** der 019b-Import leitet die Geschoss-Höhe aus
  Elevation-**Differenzen** ab, das oberste Geschoss bekommt eine Default-Höhe. Der Export
  schreibt Elevations aus **kumulierten** Höhen. Roundtrip-Treue gilt für die **Anzahl**
  ([`LH-FA-IO-002`](../../../../spec/lastenheft.md#lh-fa-io-002) AK), **nicht** notwendig für die Höhe des obersten Geschosses —
  ehrlich benennen (kein Über-Versprechen exakter Höhen-Roundtrip).
- **Wand-Attribute ohne IFC-Subset-Entsprechung (Spiegel 019b):** `Wall.type` (Innen/
  Aussen/Trag) und `material_id`-Override werden **nicht** geschrieben (nur ein einzelner
  Material-Layer mit `thickness_mm`). Benannte Lücke, kein stiller Vollumfang.
- **Atomarität/[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder):** der Export muss **atomar** sein (Temp + Rename, Muster
  Persistenz [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)); nicht beschreibbarer Pfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
  Zielpfad unverändert. Risiko: der Negative-Test muss einen real nicht beschreibbaren
  Pfad erzeugen (read-only dir / existierendes Verzeichnis als Datei) — Muster
  `test_sqlite_crash_recovery` ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)).
- **§1-Zusatz Export-Elevation ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):** ggf. ergänzen (Spiegel 019b); im Review
  bestätigen, ob §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) die Export-Richtung bereits ausreichend deckt
  („`IfcUnitAssignment` beim Export gesetzt") oder die kumulierte-Elevation-Regel
  expliziert werden muss.
- **Slice-Größe (Session-Review-Grenze):** Port-Erweiterung + Service + Adapter/Writer +
  Tests ≈ eine Sitzung; ggf. **Zwei-Commit-Split** (i SPF-Writer, ii Adapter-Mapping +
  Port + Service-Erweiterung + Tests + Verdrahtung), Muster 019b. Export bleibt
  **end-to-end** der Lieferwert.
- **arch-check-„Regel F" (Sensor-Ehrlichkeit):** unverändert gegenstandslos für Option D
  (A+B tragen die Isolation; bereits in 019b festgestellt + umdatiert). Kein erneutes
  Über-Versprechen.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Code / IO-Adapter

- **Modus:** GF; **Konventionen-Dichte:** hoch — Writer-Spiegel des 019b-Lesers im selben
  Adapter (Port-Impl, neutraler Wurf, kein Backend-Leck); `arch-check` A/B + Adapter-CMake-
  Konvention tragen, atomare Schreib-Mechanik aus der Persistenz übernommen. **Phase-Reife:**
  Phase 4 (Adapter-Schicht etabliert; IO-Codec seit 019b geübt). **Evidenz-/Diskrepanz-
  Risiko:** niedrig (Greenfield-Adapter). **Reconciliation:** keine; schließt den IFC-Strang.

### Sub-Area: Test-Infrastruktur

- **Modus:** GF; **Dichte:** hoch — GoogleTest + `LH-FA-*`-benannte AK-Tests +
  Roundtrip-Orakel (Writer→019b-Reader, kein Datei-Fixture nötig) + Integrationstest-Muster
  (019b). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

*(wird bei `done` ausgefüllt — Closure-Kriterien beobachtbar, Lerneintrag in einer der drei
Formen: geschärfte Regel · neuer Sensor · benannte Spec-Lücke.)*
