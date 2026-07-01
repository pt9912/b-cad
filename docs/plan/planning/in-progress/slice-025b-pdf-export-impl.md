---
id: slice-025b
titel: PDF-Export — Implementierung (self-rolled Vektor-PDF-Writer, io-resident, [ADR-0016](../../adr/0016-pdf-png-backend.md))
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0016](../../adr/0016-pdf-png-backend.md)]
---

# Slice 025b: PDF-Export — Implementierung

**Status:** done (2026-07-01). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review **0 HIGH** ([Plan-Report](../../../reviews/2026-07-01-slice-025b-plan.md)) + unabhängiges **Code-Review 0 HIGH** ([Code-Report](../../../reviews/2026-07-01-slice-025b-code-review.md); reale PDF-Reader-Öffenbarkeit empirisch belegt, LOW-3/INFO-3 als Test-Härtung eingearbeitet). DoD vollständig, `make gates` (215/215) + `make io-smoke` grün, Closure-Notiz §8.

**Welle:** welle-4-austausch (PDF/PNG-Strang, Code-Hälfte; Muster slice-021b [DXF-Impl]).

**Bezug:** [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007) (PDF-Export, maßstäblicher Plan, [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)),
AK bereits geschärft (slice-025a, Lastenheft 0.1.12) + Mapping entschieden (spez. §1
[`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007)). **Parametrisiert auf [ADR-0016](../../adr/0016-pdf-png-backend.md)**
(Option D, self-rolled Vektor-PDF-Writer **io-resident**, export-only, kein Qt/OCC, keine
neue Dependency). [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt; Format-Writer adapter-lokal;
Port-/Dispatch-Mechanik = Kern-Hoheit).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-01.

**Schnitt-Herkunft (Sizing-Split):** Der PDF/PNG-Impl (025a-DoD „= 025b") wird in
**025b = PDF** (Vektor, milestone-kritisch [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) + **025c = PNG**
(Raster, self-rolled DEFLATE/Adler-32/CRC-32 — distinkter Risiko-Chunk) geschnitten
(Muster 024a/b, Owner-Split-Praxis; [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)/M4 hängt nur an PDF). Der
geteilte **Plan-Projektions-Helper** (Building → Wand-Achsen je Geschoss + Bounding-Box)
entsteht hier und wird von 025c wiederverwendet.

---

## 1. Ziel

Der **PDF-Export lauffähig** machen ([LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)), end-to-end über den
**echten** Pfad `ExchangeService` → `ModelExporterPort` → `PdfExportAdapter`
(`ExchangeFormat::Pdf`), **innerhalb des von [ADR-0016](../../adr/0016-pdf-png-backend.md) entschiedenen Backends**
(self-rolled Vektor-PDF-Writer io-resident, export-only, 2D-Achsen-Maßstabsplan). Ein
Standard-PDF-Leser öffnet die Ausgabe als **maßstäblichen 2D-Grundriss** (Wand-Achsen je
Geschoss); der Maßstab ist **fest + dokumentiert**. **Keine neue Dependency, kein Qt.**

**Konkrete Mapping-Entscheidungen dieses Slice** (die §1 dem Impl-Slice überließ,
Muster DXF-Profilversion 021b):
- **Fester Maßstab 1:100**, dokumentiert (Konstante + sichtbares „M 1:100"-Label via
  Standard-Font Helvetica — kein Font-Embedding), Seitenformat **A4** (595×842 pt),
  **eine Seite je Geschoss**, Rand/Rahmen. Modell-mm → PDF-Punkte über
  `pt = mm · (72/25.4) / 100`. Für den welle-4-Umfang (Häuser ~8 m) passt 1:100 auf A4;
  Übergröße = benannte Lücke (Fit-to-Page = Re-Eval).
- **PDF 1.7**, minimale Objektstruktur (Katalog → Seitenbaum → Seiten + Content-Streams +
  eine Helvetica-Font-Ressource), `xref`-Tabelle, `trailer`, `%%EOF`.

## 2. Definition of Done

- [x] **`ExchangeFormat::Pdf` additiv ergänzt** (`src/hexagon/ports/driving/exchange_model_port.h`)
      — **nur** der Enum-Wert (Kern-Touch, keine Service-/Registry-Architektur-Änderung;
      [ADR-0016](../../adr/0016-pdf-png-backend.md)). **`Png` NICHT** (= 025c).
- [x] **`src/adapters/io/pdf_writer.{h,cpp}`** — generischer, **format-agnostischer**
      Vektor-PDF-Writer (kennt PDF-**Syntax**: Objekte, `xref`, `stream`/`Length`, Content-
      Operatoren `m`/`l`/`S`/`re`/`BT`..`ET`/`Tj`; **keine** b-cad-Domäne — Muster
      `dxf_writer`). Emittiert **valides PDF 1.7** (Header, Objekt-Katalog, Seitenbaum,
      je Seite ein Content-Stream mit korrektem `/Length`, Helvetica-Font-Ressource,
      `xref` mit korrekten Byte-Offsets, `trailer` `/Root`+`/Size`, `startxref`, `%%EOF`).
      **Locale-frei** (kein Komma; Muster `dxfReal`).
- [x] **`src/adapters/io/plan_geometry.{h,cpp}`** — **geteilter** Plan-Projektions-Helper
      (format-agnostisch, io-resident, kein OCC/Qt): `Building` → je Geschoss die geraden
      **Wand-Achsen** als 2D-Segmente (Modell-mm) + Gesamt-Bounding-Box. Datenquelle
      `building.storeys` + `building.walls` (`start`/`end`/`storey_id`), Muster
      `dxf_export_adapter`. **Von 025c (PNG) wiederverwendbar.** *(Review-INFO-5: `model::Wall`
      ist ein Einzel-Segment — „gerade Wand-Achsen" = **alle** Wände, kein Filter.)*
- [x] **`src/adapters/io/io_atomic_write.{h,cpp}`** — **geteilter** atomarer,
      **binär-sicherer** Datei-Writer (Temp + `fsync` + Rename; `errno` → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
      `event=io_no_permission` / [`E-IO-002`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) `event=persist_error`), **kein** Teil-Export,
      Zielpfad intakt. Extrahiert das seit IFC/DXF duplizierte Muster als **eine** Wahrheit
      (nur die **neuen** PDF/PNG-Adapter nutzen sie; IFC/DXF-Refaktor = **out of scope**,
      benannt). PDF/PNG sind Byte-Ströme → binär-treu (`::write` roh, wie heute).
      **Byte-gleich zum DXF-Muster übernehmen (Review-LOW-2):** Temp-Namensschema
      `path + ".tmp"` **und** die `ioCodeForErrno`-Abbildung (**EISDIR/ENOTDIR/ENOENT/EACCES/
      EPERM/EROFS → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)** `event=io_no_permission`, sonst [`E-IO-002`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) —
      sonst bricht die referenzierte Negative-Test-Technik (`.tmp`-Verzeichnis → `EISDIR`).
- [x] **`src/adapters/io/pdf_export_adapter.{h,cpp}`** — `PdfExportAdapter :
      ModelExporterPort`; bildet je Geschoss eine A4-Seite: Rahmen + **maßstäblich
      (1:100)** transformierte Wand-Achsen (`plan_geometry`) + „M 1:100"-Label; serialisiert
      über `PdfWriter`; schreibt über `io_atomic_write` ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). **Export-only**
      (kein Import-Adapter). Der PDF-Code lebt **ausschließlich** in `adapters/io/`
      (`arch-check` Regel A/B — kein externer Header, „Regel F gegenstandslos").
- [x] **Composition Root** (`src/main.cpp`): `PdfExportAdapter`-Instanz + `ExporterMap`-
      Eintrag `{ExchangeFormat::Pdf, &pdf_exporter}` + `--export-pdf`-CLI (via bestehendem
      `runExportIfRequested`). **Nur** ExporterMap (export-only; **nicht** in die ImporterMap).
- [x] **`src/adapters/CMakeLists.txt`**: die vier neuen `io/`-Quellen (`pdf_writer`,
      `plan_geometry`, `io_atomic_write`, `pdf_export_adapter`) in `bcad_adapters`.
      **Keine** neue `find_package`/apt-Zeile (reines C++/STL — belegt „keine neue Dependency").
- [x] **`tests/adapters/test_pdf_export.cpp`** (+ `tests/CMakeLists.txt`): AK
      [`LH-FA-IO-007`](../../../../spec/lastenheft.md#lh-fa-io-007) über den **echten** `ExchangeService`-Pfad —
      - **Happy + voll-Decode-Orakel (Byte-Konsistenz UND Objektgraph):** Export erzeugt
        eine Datei; ein **im Test eingebauter PDF-Parser** (kein externer Reader — keiner im
        gepinnten Image, s. §6) prüft **(a) Byte-Konsistenz:** `%PDF-1.`-Header, `xref`-Byte-
        Offsets zeigen auf `N 0 obj`, Content-Stream-`/Length` == reale Stream-Byte-Zahl,
        `trailer`/`startxref`/`%%EOF`; **(b) Objektgraph/Reader-Öffenbarkeit** (schließt die
        Selbst-Orakel-Blindstelle, Review-MED-1): `Catalog`/`/Root` → `/Pages` → `/Kids` je
        Seite auflösbar, je Seite `/MediaBox` == A4 (595×842), `/Contents` → auflösbares
        Stream-Objekt, `/Resources`→`/Font`→`/Helvetica` verdrahtet; **(c) Inhalt je Seite:**
        der Content-Stream **je Geschoss-Seite** trägt die erwartete **Zahl Linien-Operatoren**
        (je Wand-Achse ein `m`+`l`, **pro Seite gegen die Wandzahl des Geschosses** —
        Geschoss-Trennung, Review-LOW-3) + Rahmen-`re` + „M 1:100"-`Tj`.
      - **Maßstabs-Sonde:** eine bekannte Modell-Kante (z. B. 5000 mm) ergibt im
        Content-Stream eine Linie der Länge `5000·(72/25.4)/100` pt (± Toleranz) — belegt
        die **Maßstäblichkeit** ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)).
      - **Boundary:** leeres Modell → **gültige** PDF (mind. eine ~leere Seite / valider
        Katalog+xref+EOF), **kein** Absturz.
      - **Negative (Integration):** nicht beschreibbarer Zielpfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
        (`io_no_permission`) durch den **echten** Adapter, **kein** Teil-Export (Muster
        `test_dxf_export` `NonWritablePathRejectedWithEIo001`).
      - **Export-only:** `importModel(path, ExchangeFormat::Pdf)` → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
        (export-only-Lookup-Miss, identisch STEP/STL — belegt „kein Import-Adapter").
- [x] **`tools/io-smoke.sh`**: `expect_export --export-pdf "$OUT/s.pdf"` (Binary-Smoke der
      `main.cpp`-Glue; exit 0 + nicht-leere Datei). **PNG** folgt in 025c.
- [x] **`make gates` grün** (arch-check inkl. `io/`, lint 0, test inkl. neuer AK-Tests,
      coverage ≥ Schwelle) + **`make io-smoke`** grün lokal; `make schema-check` unberührt
      (kein Schema). **Unabhängiges Code-Review vor Welle-Closure** (Muster 021b;
      **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** — keine neue **Solid**-Geometrie, nur 2D-Projektion; die
      Maßstab-/Projektions-Korrektheit trägt die Maßstabs-Sonde + das voll-Decode-Orakel).
      Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/exchange_model_port.h` | ändern | `ExchangeFormat` additiv um `Pdf` |
| `src/adapters/io/pdf_writer.{h,cpp}` | neu | generischer Vektor-PDF-Writer (Syntax) |
| `src/adapters/io/plan_geometry.{h,cpp}` | neu | geteilte 2D-Plan-Projektion (Wand-Achsen je Geschoss + bbox) |
| `src/adapters/io/io_atomic_write.{h,cpp}` | neu | geteilter atomarer binär-sicherer Writer ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) |
| `src/adapters/io/pdf_export_adapter.{h,cpp}` | neu | `PdfExportAdapter : ModelExporterPort` (Mapping + Maßstab + Label) |
| `src/main.cpp` | ändern | `PdfExportAdapter` + `ExporterMap`-Eintrag + `--export-pdf` |
| `src/adapters/CMakeLists.txt` | ändern | vier neue `io/`-Quellen; keine neue Dependency |
| `tests/adapters/test_pdf_export.cpp` | neu | AK + voll-Decode-Orakel + Maßstabs-Sonde + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) + export-only |
| `tests/CMakeLists.txt` | ändern | Test registrieren |
| `tools/io-smoke.sh` | ändern | `--export-pdf`-Smoke |
| `docs/reviews/{2026-07-01-slice-025b-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: [ADR-0016](../../adr/0016-pdf-png-backend.md) accepted,
  AK/§1 geschärft (025a), `ModelExporterPort`/`ExchangeService`/`ExporterMap` real
  (slice-019c/020b/021b).

## 5. Closure-Trigger

- DoD vollständig, `make gates` + `make io-smoke` grün, Code-Review 0 HIGH,
  Closure-Notiz → **025c (PNG)** wird startbar; danach Welle-4-Verifikation → **M4**.

## 6. Risiken und offene Punkte

- **Self-rolled PDF-Validität + Selbst-Orakel-Blindstelle (Kern-Risiko, Review-MED-1):**
  eine minimale Byte-Falschheit (`xref`-Offset, `/Length`, fehlendes `%%EOF`) **oder** ein
  unvollständiger Objektgraph (fehlende `/MediaBox`, lose `/Contents`-Referenz) macht das
  PDF für Fremd-Leser ungültig, obwohl es syntaktisch „nicht-leer" ist. **Externer-Reader-
  Entscheidung (geschlossen):** im gepinnten Image liegt **kein** PDF-Validator (Probe:
  `grep poppler|qpdf|ghostscript|mupdf .devcontainer/Dockerfile` → leer); einen als Test-
  Dependency aufzunehmen wäre eine **[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Berührung** (neuer apt-Eintrag) →
  **jetzt nicht genommen**. **Sensor stattdessen:** das **verschärfte voll-Decode-Orakel**
  prüft Byte-Konsistenz **UND** Objektgraph/Reader-Öffenbarkeit (DoD (a)+(b)+(c)) — schließt
  die konkreten Blindstellen, die ein Reader zum Öffnen braucht. Ein realer-Reader-
  Cross-Check bleibt **benannte Lücke** (eigener [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Beschluss, falls je gefordert).
- **Maßstäblichkeit ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien), milestone-kritisch):** der Maßstab muss korrekt +
  dokumentiert sein. **Gegenmaßnahme:** feste 1:100-Konstante + sichtbares „M 1:100"-Label
  + **Maßstabs-Sonde** (bekannte Kante → erwartete pt-Länge). Fit-to-Page/Übergröße =
  benannte Lücke + Re-Eval.
- **Helvetica-Text ohne Embedding:** Helvetica ist ein PDF-Standard-Font (Base-14), per
  `/BaseFont /Helvetica` referenzierbar **ohne** Font-Datei — kein Embedding, keine
  Dependency. Nur das kurze „M 1:100"-Label; **keine** allgemeine Textsatz-Fähigkeit
  (benannte Lücke).
- **arch-check / keine Dependency:** der PDF-Writer hat **keinen** externen Header → Regel
  A/B isolieren ihn (kein neues Gate, „Regel F gegenstandslos", [ADR-0016](../../adr/0016-pdf-png-backend.md)); `make build`
  zieht keine neue `find_package`/apt-Zeile. Vor dem Gate `grep`-Selbstcheck (kein
  OCC-/Qt-Include in den neuen `io/`-Dateien).
- **`io_atomic_write`-Extraktion (Scope-Grenze):** nur die **neuen** PDF/PNG-Adapter
  nutzen den geteilten Writer; **IFC/DXF bleiben** auf ihren lokalen Kopien (Refaktor =
  out of scope, sonst Slice-Aufblähung — benannt für späteren Cleanup).
- **Coverage:** die neuen Writer/Adapter/plan_geometry sind gut testbar (reine Byte-/
  Geometrie-Logik) → AK-Tests decken sie; `main.cpp`-Glue ist coverage-ausgenommen, aber
  `io-smoke` belegt sie (Muster DXF).
- **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a:** keine neue Solid-/Bauteil-Geometrie (2D-Projektion bestehender
  Wand-Achsen). Trotzdem **unabhängiges Code-Review** vor Closure (Muster 021b): PDF-
  Byte-Korrektheit, Maßstab, Atomarität, arch-Isolation.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Import/Export-Adapter (`src/adapters/io/`)

- **Modus:** GF; **Konventionen-Dichte:** hoch — Format-Adapter hinter `ModelExporterPort`,
  Writer/Adapter-Split (Muster `dxf_writer`/`dxf_export_adapter`), Atomarität + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
  arch-check Regel A/B (io-resident, kein OCC/Qt), keine neue Dependency ([ADR-0016](../../adr/0016-pdf-png-backend.md)
  Option D). **Phase-Reife:** IO Phase 3 (Impl). **Risiko:** mittel (self-rolled Byte-Format
  → voll-Decode-Orakel als Netz).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; **Dichte:** hoch — GoogleTest, AK-ID im Namen, **voll-Decode-Orakel**
  (nicht „nicht-leer"), Maßstabs-Sonde, Integration über den echten Pfad ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)),
  export-only ([`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Lookup-Miss). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-07-01):**

- **PDF-Export lauffähig** ([LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007), [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)):
  self-rolled Vektor-`PdfWriter` (Objektgraph Katalog/Seitenbaum/Content-Stream, xref/
  trailer/`%%EOF`, korrektes `/Length`) + `PdfExportAdapter` (je Geschoss eine A4-Seite,
  **fester Maßstab 1:100** + „M 1:100"-Label via Helvetica Base-14, Rahmen, Wand-Achsen) +
  geteilter `plan_geometry` (2D-Projektion) + geteilter `io_atomic_write` (atomar, binär-treu,
  [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). `ExchangeFormat::Pdf` additiv; Composition Root `--export-pdf` + `ExporterMap`
  (export-only). **Keine neue Dependency, kein Qt/OCC** (arch-check Regel A/B grün).
- **Voll-Decode-Orakel** (Review-MED-1): `test_pdf_export.cpp` prüft Byte-Konsistenz
  (xref-Offsets → „N 0 obj", `/Length` == Stream-Länge, `trailer /Size`, `startxref`/`%%EOF`)
  **und** Objektgraph/Reader-Öffenbarkeit (Catalog→Pages→Kids, `/MediaBox`==A4, `/Contents`→
  Stream, `/Font`→Helvetica) **und** Inhalt je Geschoss-Seite (Linien == Wandzahl) + Maßstabs-
  Sonde (5000 mm → 141,73 pt) + **Orientierungs-Sonde** (kein Y-Flip) + Boundary + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) +
  export-only. 7 PDF-Tests; `make gates` grün (215/215, Coverage 90,4 %), `make io-smoke` grün.
- **Unabhängiges Code-Review 0 HIGH** ([Report](../../../reviews/2026-07-01-slice-025b-code-review.md)):
  reale Reader-Öffenbarkeit **empirisch** belegt (poppler/pdftotext/ghostscript); LOW-3 +
  INFO-3 als Test-Härtung eingearbeitet; LOW-1/-2 als benannte Grenzen.

**Lerneintrag:**

- **Reales-Reader-Cross-Check außerhalb des gepinnten Images (Sensor-Ehrlichkeit):** das
  Image trägt keinen PDF-Validator → ein Reader-Test wäre eine [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Berührung
  (nicht genommen); Sensor ist das **verschärfte Objektgraph-Selbst-Orakel**. Der Code-Reviewer
  hat die reale Öffenbarkeit **einmalig empirisch** belegt (poppler/gs außerhalb des Gates) —
  das de-riskt [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) direkt, ohne eine Laufzeit-Dependency einzuziehen.
- **Benannte Grenzen (kein Über-Versprechen):** fester Maßstab 1:100 → Modelle > ~19,5 m
  laufen aus A4 (Fit-to-Page = [ADR-0016](../../adr/0016-pdf-png-backend.md)-Re-Eval); `fsync`-Rückgabe ungeprüft (byte-gleich zum
  dxf/ifc-Muster, Cross-Adapter-Cleanup-Kandidat); PNG + allgemeiner Textsatz + 3D-Screenshot
  = spätere Slices/Re-Eval.
- **Sizing-Split trägt:** PDF (025b, milestone-kritisch [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) getrennt vom Raster-Encoder-Risiko (PNG,
  025c). Der geteilte `plan_geometry`/`io_atomic_write` steht für 025c bereit.

**Restrisiko / Nachfolge:** **025c (PNG-Export)** wird startbar — Raster-`PngWriter`
(stored-DEFLATE + Adler-32 + CRC-32) rastert denselben Plan (`plan_geometry`-Reuse),
`PngExportAdapter` + `ExchangeFormat::Png` + `io_atomic_write`-Reuse + io-smoke;
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review davor. Danach Welle-4-Verifikation + Carveout-Audit →
`done/welle-4-results.md` → **M4** ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) durch PDF erfüllt). ADR-Index-Impl-
Folgepflicht bleibt bis 025c „offen" (PDF-Hälfte durch 025b, Präzedenz 024a/b).
