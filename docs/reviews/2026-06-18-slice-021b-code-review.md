# Code-Review — slice-021b (DXF-Import/-Export-Implementierung)

## Kopf

- **Review-Art:** Unabhängiges Code-Review (Reviewer ≠ Autor) vor Welle-/Slice-Closure.
  [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  ist **nicht** getriggert — 2D-Text-I/O ohne neue Solid-/Bauteil-Geometrie; **Einordnung bestätigt** (siehe §Verifiziert-korrekt).
- **Gegenstand:** Arbeitsbaum-Diff (uncommitted) des DXF-Strangs: io-residenter DXF-Subset-Codec
  (`dxf_reader.{h,cpp}` / `dxf_writer.{h,cpp}`), Mapping-Adapter (`dxf_import_adapter.{h,cpp}` /
  `dxf_export_adapter.{h,cpp}`), Kern-Umbau (`exchange_service.{h,cpp}`, `exchange_model_port.h`:
  `ExchangeFormat::Dxf` + Import-Dispatch auf `ImporterMap`), `main.cpp` (Composition Root +
  `--import-dxf`/`--export-dxf`), Tests (`test_dxf_codec.cpp`, `test_dxf_import.cpp`,
  `test_dxf_export.cpp`) + Ctor-Migration in `test_ifc_import.cpp`/`test_ifc_export.cpp`/`test_step_stl_export.cpp`.
- **Datum:** 2026-06-18.
- **Quellen:** [slice-021b-dxf-impl.md](../plan/planning/in-progress/slice-021b-dxf-impl.md) (Plan/DoD),
  [ADR-0015](../plan/adr/0015-dxf-backend.md) (Backend Option D, io-resident, 2D-Grundriss),
  [LH-FA-IO-003](../../spec/lastenheft.md#lh-fa-io-003)/[LH-FA-IO-004](../../spec/lastenheft.md#lh-fa-io-004) (AK),
  spez. §1 [`LH-FA-IO-003.a`](../../spec/lastenheft.md#lh-fa-io-003) (Mapping) + §4 (Fehler-Codes),
  Vorbild `ifc_export_adapter.cpp` / `ifc_import_adapter.cpp` (atomicWrite-/E-IO-Muster).
- **Methode:** vollständiger Diff + alle untracked Dateien gelesen; `g++ -fsyntax-only` aller fünf
  neuen `.cpp` (sauber); zwei adversariale Probe-Harnesses (Codec + Adapter, gegen die echte Quelle
  gelinkt) zur Verifikation von Roundtrip-Treue, Determinismus, Tokenizer-Grenzen und Negativ-Pfaden.
  **Gate-Stand:** `make gates` läuft in Docker und wurde vom Reviewer **nicht** ausgeführt — die
  Gate-Grün-Behauptung (DoD-7) ist vom Autor zu belegen; statisch ist alles compile-clean.

**Verdikt: 0 HIGH / 2 MED / 4 LOW / 2 INFO.** Keine Korrektheits-, Schichtungs- oder
Test-Orakel-Defekte einer Kern-AK. Die AK ([LH-FA-IO-003](../../spec/lastenheft.md#lh-fa-io-003)/
[LH-FA-IO-004](../../spec/lastenheft.md#lh-fa-io-004): Happy/Boundary/Negative, Roundtrip,
[`E-IO-001`](../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/[`E-IO-003`](../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
Totalität, MED-1/2/3) sind erfüllt. **Closure nicht blockiert.**

## Verifiziert-korrekt

- **Codec-Tokenizer wohlgeformt.** `parse` verwirft ungerade Paarung (`dxf_reader.cpp:124`) und
  nicht-numerische Code-Zeilen (`parseGroupCode` an `dxf_reader.cpp:132` — optionales Vorzeichen +
  reine Ziffern, `std::stoi`-`out_of_range` gefangen → false → Wurf). `contentLines`
  (`dxf_reader.cpp:64`) trimmt **nur führende/abschließende** Leerzeilen; **interne** Leerzeilen
  bleiben erhalten und werden als Code-Zeile geparst → leer → Wurf. Probe bestätigt: interne
  Leerzeile(n) → Wurf (kein stilles Mis-Pairing), leer/whitespace/strukturlos → kein Wurf.
- **Pointer-Stabilität in `parse` ist sicher.** `current = &entities_.back()` (`dxf_reader.cpp:139`)
  wird **bei jedem** `push_back` (`dxf_reader.cpp:138`) sofort neu gesetzt; der `else if`-Zweig
  (`dxf_reader.cpp:141`) wächst nur `current->attributes`, **nie** `entities_`. Kein dangling pointer
  trotz wachsendem Vektor.
- **`dxfReal` locale-frei + round-trip-stabil.** `%.12g` + Komma→Punkt-Normalisierung + erzwungenes
  Dezimalzeichen (`dxf_writer.cpp:12`). Probe: `0.0`/`5000.0`/`±1234.5` round-trippen exakt;
  kein Komma, garantiert ein `.`.
- **Mapping-Korrektheit (Import).** `LINE`→Wand über 10/20/11/21 (`lineAxis`, `dxf_import_adapter.cpp:61`);
  `LWPOLYLINE`→n−1 Wände (`dxf_import_adapter.cpp:115`); `LAYER` (Code 8)→ein `Storey` je distinkter
  Layer in Erst-Erscheinungs-Reihenfolge (`StoreyTable`, `dxf_import_adapter.cpp:75`). Probe „Layer
  B,A,B" → 2 Geschosse, Wand-Storey-Ids `1 2 1` (deterministisch).
- **Mapping-Korrektheit (Export) + MED-3.** Je Wand eine `LINE`, die ihr Geschoss-`LAYER` als
  **Gruppencode 8** trägt (`dxf_export_adapter.cpp:69`) — die Roundtrip-Achse, die der Import liest
  (nicht die `LAYER`-Tabelle). z-Koordinaten 0 (Code 30/31).
- **Roundtrip-Treue.** Probe Export→Re-Import: 2 Geschosse / 3 Wände, per-Geschoss-Anzahl 2+1,
  Achs-Lage exakt (inkl. negativer Koordinaten −1234.5). **Höhe/Dicke** werden — korrekt — **nicht**
  verglichen (Default beim Import, benannte Subset-Lücke); das Orakel `test_dxf_export.cpp:97`-`99`
  prüft `start.x`/`end.x`/`end.y` + per-Geschoss-Anzahl, **nicht** `height_mm`/`thickness_mm`.
- **Kern-Umbau (`ImporterMap`) + MED-1.** Import-Dispatch ist nun symmetrisch zur `ExporterMap`
  (`exchange_service.cpp:18`); Lookup-Miss/`nullptr` → Wurf mit **beiden** Token
  (`E-IO-003` **und** `event=import_rejected`, `exchange_service.cpp:24`) → der bestehende Test
  `ImportOfStlRejectedAsExportOnly` (Token-Doppel-Assertion) bleibt grün, nicht nur kompilierbar.
  STEP/STL bleiben export-only (nicht in der `ImporterMap`, `main.cpp`). Kern bleibt format-frei
  (kein DXF-Symbol in `services/`/`ports/`; arch-check Regel A).
- **Atomarität/Fehler-Codes.** Export: vollständig im Speicher (`buildDxf`), dann Temp+`write`+`fsync`+
  Rename (`atomicWrite`, `dxf_export_adapter.cpp:116`); `errno`→`E-IO-001`/`io_no_permission` inkl.
  EISDIR der open-Phase (`ioCodeForErrno`, `dxf_export_adapter.cpp:93` — byte-identisch zum
  IFC-Vorbild). Probe + Integrationstest `NonWritablePathRejectedWithEIo001` (`.tmp` ist ein
  Verzeichnis) → `E-IO-001`, **kein** Teil-Export, Zielpfad bleibt aus. Import: vollständiges Modell
  zuerst, Wurf → `E-IO-003` (`dxf_import_adapter.cpp:153`), kein Teil-Import.
- **Test-Pfad echt.** Import- **und** Export-Integrationstests laufen durch `ExchangeService` →
  `ModelImporterPort`/`ModelExporterPort` → DXF-Adapter (`ExchangeFormat::Dxf`), nicht nur den
  nackten Adapter (welle-3-Lehre slice-015b).
- **Schichtung/DRY.** Codec STL-/header-frei (kein OCC/externer Header → arch-check Regel A/B,
  „Regel F gegenstandslos"); `atomicWrite`/`ioCodeForErrno`-Duplikat aus `ifc_export_adapter` ist
  die **bewusste** Per-Adapter-Spiegelung (Plan §6, Regel B verbietet Adapter→Adapter-Teilung).
- **Subset ggü. [ADR-0015](../plan/adr/0015-dxf-backend.md):** genau gerade Wand-Achsen (2D),
  Geschoss-Layer, Default-Höhe/-Dicke; Fremd-Entitäten (`CIRCLE`/`ARC`/`HATCH`/…) übersprungen
  (`test_dxf_import.cpp` `SubsetForeignEntitiesAreSkippedNotRejected`). Kein Über-/Unter-Versprechen.
- **MR-009 n/a bestätigt:** keine neue Solid-/Bauteil-Geometrie, kein OCC-Pfad, keine
  Vernähung/Booleans — reines ASCII-Text-Mapping. Geometrie-Code-Review-Trigger greift nicht.

## Findings

### MED-1 — Teil-koordinierte `LINE` wird still verworfen (Orakel-Lücke, keine AK-Verletzung)
`lineAxis` gibt `std::nullopt` zurück, sobald **eine** der vier Koordinaten 10/20/11/21 fehlt
(`dxf_import_adapter.cpp:66`); die Wand wird dann ohne jedes Signal übersprungen, und da
`storeyFor` in diesem Fall **nicht** aufgerufen wird, entsteht auch **kein** (leeres) Geschoss
(Probe „D": LINE ohne Code 21 → 0 Wände / 0 Geschosse). Das ist mit der Totalitäts-/Subset-Skip-
Philosophie vereinbar (eine wohlgeformte-aber-unvollständige Entität ist **Boundary**, kein
`E-IO-003`), aber **kein Test pinnt** diese Kante — ein künftiger Refactor, der versehentlich die
ganze Datei bei einer Teil-`LINE` ablehnt (oder eine Wand mit `(0,0)`-Default-Koordinaten erzeugt),
bliebe unentdeckt. **Fix (vor Merge):** ein Import-Test „valide `ENTITIES` mit einer `LINE` ohne
Code 21 → 0 Wände, **kein** Wurf" ergänzen (pinnt Boundary-vs-Negative an der Entitäts-Grenze,
Schwester zu MED-2-Mitte auf Entitäts- statt Datei-Ebene). MED, nicht HIGH: das Laufzeitverhalten
ist korrekt und spec-konform, nur ungetestet.

### MED-2 — LWPOLYLINE-Import nur per **Anzahl** geprüft, nicht Achs-Lage/Vertex-Reihenfolge
`LwpolylineYieldsSegmentWalls` (`test_dxf_import.cpp:113`) prüft nur `walls.size()==2`; die
Codec-Ebene (`test_dxf_codec.cpp` `LwpolylineVerticesAreExtractedInOrder`) prüft Vertex-Koordinaten,
aber **nicht** die Adapter-Abbildung Vertex-Paar→`Wall.start`/`end`. Ein invertiertes Segment-Mapping
(z. B. `start=pts[i+1]`, `end=pts[i]`) oder eine x/y-Vertauschung im Polyline-Zweig
(`dxf_import_adapter.cpp:117`-`120`) würde durch die Tests rutschen (der `LINE`-Zweig ist über
`test_dxf_import.cpp:64` koordinaten-geprüft, der Polyline-Zweig **nicht**). LWPOLYLINE ist ein
benanntes Subset-Feature (LOW-1 des Plans), kein Kern-AK-Pfad (Export schreibt nur `LINE`) → MED.
**Fix (vor Merge):** im LWPOLYLINE-Test zusätzlich `walls[0].start`/`walls[0].end`/`walls[1].end`
gegen die Vertex-Koordinaten assertieren.

### LOW-1 — `dxfReal` `%.12g` ist nicht round-trip-**exakt** für >12 signifikante Stellen
`%.12g` (`dxf_writer.cpp:14`) round-trippt mm-Koordinaten im Editor-Wertebereich verlustfrei
(getestet bis `±1234.5`), aber ein `double` mit >12 signifikanten Dezimalstellen (z. B. nach
Krummwinkel-Rechnung) verlöre Präzision. Für den 2D-Grundriss-Subset praktisch irrelevant
(Koordinaten sind ganzzahlige/halbzahlige mm); als **benannte** Genauigkeitsgrenze belassbar.
Optional `%.17g` (exakter `double`-Roundtrip) falls je relevant.

### LOW-2 — `strip`/`isBlank` nutzen `std::isspace` ohne `<locale>`-Härtung
`std::isspace` (`dxf_reader.cpp:21`, `dxf_import_adapter.cpp:57`) ist locale-abhängig; mit dem
`static_cast<unsigned char>` ist UB vermieden und in der `"C"`-Default-Locale (Container) korrekt.
Da der Codec ansonsten bewusst locale-frei ist (`dxfReal`), ist die Asymmetrie eine kosmetische
Inkonsequenz, kein Defekt (kein `setlocale`-Aufruf im Programm). Belassbar.

### LOW-3 — `points()` generalisiert auch über `LINE`-Attribute, wird dort aber nie genutzt
`DxfEntity::points()` (`dxf_reader.cpp` Codec) sammelt **alle** aufeinanderfolgenden 10/20-Paare;
auf eine `LINE` angewandt griffe es nur das Start-10/20 (Code 11/21 sind nicht 20). Der Import ruft
`points()` ausschließlich im `LWPOLYLINE`-Zweig (`dxf_import_adapter.cpp:116`), nie auf `LINE` →
kein aktiver Pfad. Reine API-Allgemeinheit; eine Doc-Notiz „nur für Vertex-Listen, nicht
Start/End-Entitäten" am Header schützt vor künftigem Fehlgebrauch.

### LOW-4 — Closure-Doku (DoD-7) noch offen
ADR-Index-Folgepflicht-Zeile „[ADR-0015](../plan/adr/0015-dxf-backend.md) Impl" steht noch auf
**offen**; `CHANGELOG.md` hat keinen slice-021b-Impl-Eintrag; `roadmap.md` führt „DXF-Impl" noch als
offen. Das ist **Commit (iii) Closure** (Plan §3), der laut Schnitt **nach** diesem Code-Review
folgt — kein Befund gegen die Implementierung, nur ein Closure-Checklisten-Reminder.

### INFO
- **INFO-1:** Acht Ctor-Migrationen (`main.cpp` 1 + `test_ifc_import` 2 + `test_ifc_export` 2 +
  `test_step_stl_export` 3) auf die `ImporterMap`-Signatur sind vollständig und konsistent
  (LOW-3 des Plans); STL-Tests verdrahten den IFC-Importer nur als Platzhalter (export-only-Format).
- **INFO-2:** `make gates` (Docker) vom Reviewer nicht ausgeführt; alle fünf neuen `.cpp` sind
  `-fsyntax-only`-clean und die Probe-Harnesses linken/laufen gegen die echte Quelle. Die
  AK-/Roundtrip-/Negativ-Tests sind über den echten `ExchangeService`-Pfad formuliert.

## Ergebnis

**Keine HIGH-Findings.** Roundtrip-Treue (Anzahl je Geschoss + Achs-Lage), Determinismus
(Layer-Erst-Erscheinung), Atomarität (Temp+fsync+Rename, `E-IO-001`/`io_no_permission`),
Import-Totalität (`E-IO-003`/`import_rejected` nur bei nicht wohlgeformtem Strom; leer/strukturlos →
leeres Modell) und der MED-1-Export-only-Vertrag sind verifiziert; der Kern bleibt format-frei.
MR-009 ist korrekt nicht getriggert. **MED-1** (Teil-`LINE`-Boundary ungetestet) und **MED-2**
(LWPOLYLINE-Import nur anzahl-geprüft) sind vor Merge mit je einem Assert zu schließen; LOW-1…4 sind
benannt/kosmetisch/Closure-organisatorisch. **slice-021b ist closure-reif**, sobald MED-1/MED-2
nachgezogen sind und der Autor `make gates` grün belegt.
