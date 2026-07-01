---
id: slice-025c
titel: PNG-Export — Implementierung (self-rolled Raster-PNG-Encoder, io-resident, [ADR-0016](../../adr/0016-pdf-png-backend.md))
status: in-progress
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0016](../../adr/0016-pdf-png-backend.md)]
---

# Slice 025c: PNG-Export — Implementierung

**Status:** in-progress — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review **0 HIGH / 3 MED / 2 LOW**; Start **nicht blockiert**. MED-1 (Orakel-Prüfsummen im Test eigenständig, keine Encoder-Tautologie), MED-2 (degenerierte-BBox-Boundary-Test), MED-3 (zlib-Header-Check im Orakel), LOW-1 (≥2-Farben-Assert), LOW-2 (Lint-Rationale präzisiert) eingearbeitet. [Report](../../../reviews/2026-07-01-slice-025c-plan.md).

**Welle:** welle-4-austausch (PDF/PNG-Strang, PNG-Hälfte; schließt die 025b-Sizing-Split-Folge).

**Bezug:** [LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008) (PNG-Export), AK bereits geschärft
(slice-025a, Lastenheft 0.1.12) + Mapping entschieden (spez. §1 [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007)).
**Parametrisiert auf [ADR-0016](../../adr/0016-pdf-png-backend.md)** (Option D, self-rolled Raster-PNG-Writer
**io-resident**, export-only, kein Qt/OCC, keine neue Dependency). [ADR-0001](../../adr/0001-hexagonale-architektur.md).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-01.

**Schnitt-Herkunft:** PNG-Hälfte des 025b-Sizing-Splits (PDF=025b [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)/M4 · PNG=025c).
Der Raster-Encoder (**stored-DEFLATE + Adler-32 + CRC-32 + Scanline-Filter**) ist der
distinkte Risiko-Chunk, der von PDF getrennt wurde. **Reuse aus 025b:** `plan_geometry`
(2D-Projektion) + `io_atomic_write` (atomarer binär-treuer Writer) — beide bereits real.

---

## 1. Ziel

Den **PNG-Export lauffähig** machen ([LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008)), end-to-end über den
**echten** Pfad `ExchangeService` → `ModelExporterPort` → `PngExportAdapter`
(`ExchangeFormat::Png`), **innerhalb des von [ADR-0016](../../adr/0016-pdf-png-backend.md) entschiedenen Backends**
(self-rolled Raster-PNG-Writer io-resident, export-only, **unkomprimiert**). Ein
Standard-Bildbetrachter öffnet die Ausgabe als Rasterbild des 2D-Grundrisses (Wand-Achsen).
**Keine neue Dependency, kein Qt.**

**Konkrete Mapping-Entscheidungen dieses Slice** (die §1 dem Impl-Slice überließ):
- **Ein PNG = ein kombiniertes Rasterbild** aller Geschoss-Achsen (ein PNG trägt **ein**
  Bild — anders als das PDF mit einer Seite je Geschoss). **Je Geschoss eine Farbe** (kleine
  Palette, zyklisch nach Geschoss-Index) → die „Wand-Achsen je Geschoss" bleiben im
  kombinierten Bild unterscheidbar. Hintergrund weiß.
- **Feste Leinwand** (z. B. 800×600 px), **Fit-to-Canvas** (seitenverhältnis-erhaltend,
  zentriert, Rand) — PNG trägt **keine** Maßstabs-Anforderung ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) = nur PDF);
  feste Leinwand hält die Datei-Größe **beschränkt** (die ADR benennt „PNG unkomprimiert →
  große Dateien"). Degenerierte/leere Bounding-Box wird **geguardet** (kein Div-durch-0 →
  weiße Leinwand).
- **PNG-Format:** 8-bit **RGB** (color type 2), Filter 0 (None) je Scanline, `IDAT` als
  **zlib-Strom mit unkomprimierten DEFLATE-Blöcken** (≤ 65535 B) + **Adler-32**, **CRC-32**
  je Chunk (Signatur/`IHDR`/`IDAT`/`IEND`).

## 2. Definition of Done

- [ ] **`ExchangeFormat::Png` additiv ergänzt** (`src/hexagon/ports/driving/exchange_model_port.h`)
      — **nur** der Enum-Wert (Kern-Touch, keine Service-/Registry-Architektur-Änderung).
- [ ] **`src/adapters/io/png_writer.{h,cpp}`** — generischer, **format-agnostischer**
      Raster-PNG-Encoder (kennt PNG-**Syntax**: 8-Byte-Signatur, Chunks mit Länge+Typ+
      **CRC-32**, `IHDR`/`IDAT`/`IEND`, `IDAT` = zlib-Header + **stored-DEFLATE**-Blöcke +
      **Adler-32**; **keine** b-cad-Domäne). Enthält eine kleine `Bitmap` (RGB-Puffer,
      `drawLine` via Bresenham [bounds-geklemmt], Hintergrund-Fill) + `encodePng(Bitmap)`.
      Alle Byte-Ordnungen (Chunk-Länge/CRC big-endian, DEFLATE-`LEN`/`NLEN` little-endian,
      Adler big-endian) korrekt. **Reines C++/STL** (arch-check Regel A/B; kein zlib-Link,
      kein Qt/OCC).
- [ ] **`src/adapters/io/png_export_adapter.{h,cpp}`** — `PngExportAdapter :
      ModelExporterPort`; projiziert via **`plan_geometry`** (Reuse), rechnet die
      **Fit-to-Canvas**-Transformation (Modell-mm → Pixel, geguardet), zeichnet je Geschoss
      die Wand-Achsen in der Geschoss-Farbe, encodiert via `png_writer`, schreibt via
      **`io_atomic_write`** (Reuse, [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). **Export-only** (kein Import-Adapter).
- [ ] **Composition Root** (`src/main.cpp`): `PngExportAdapter`-Instanz + `ExporterMap`-
      Eintrag `{ExchangeFormat::Png, &png_exporter}` + `--export-png`-CLI (via `runExportIfRequested`).
      **Nur** ExporterMap (export-only).
- [ ] **`src/adapters/CMakeLists.txt`**: `png_writer.cpp` + `png_export_adapter.cpp` in
      `bcad_adapters`. **Keine** neue `find_package`/apt-Zeile.
- [ ] **`tests/adapters/test_png_export.cpp`** (+ `tests/CMakeLists.txt`): AK
      [`LH-FA-IO-008`](../../../../spec/lastenheft.md#lh-fa-io-008) über den **echten** `ExchangeService`-Pfad —
      - **Happy + voll-Decode-Orakel:** ein **im Test eingebauter PNG-Decoder** (kein
        externer Reader) prüft: 8-Byte-Signatur; `IHDR` (Breite/Höhe == Leinwand, bit depth 8,
        color type 2, Filter/Interlace 0); **CRC-32 je Chunk** korrekt; **zlib-2-Byte-Header**
        (CM==8, `(CMF<<8|FLG) % 31 == 0`, FDICT==0 — Review-MED-3); `IDAT`-zlib-Strom **inflaten**
        (stored-Blöcke, inkl. mehrerer Blöcke > 65535 B) → Raw == `Höhe·(1 + Breite·3)` Bytes,
        **jede Scanline Filterbyte 0**, **Adler-32** == Adler des Raw; **Tinte vorhanden**
        (nicht-weiße Pixel); `IEND`. **Keine** oberflächliche „nicht-leer"-Prüfung.
      - **Orakel-Unabhängigkeit (Review-MED-1):** die Orakel-Prüfsummen werden im **Test
        eigenständig** aus den Spec-Konstanten hergeleitet — CRC-32 (Polynom `0xEDB88320`,
        über **Chunk-Typ+Daten**), Adler-32 (mod 65521, über die **Roh**-Scanlines) — **nicht**
        per Aufruf der `png_writer`-Helfer (sonst prüft der Encoder sich selbst; Muster
        `test_pdf_export` = unabhängiger Parser).
      - **Je-Geschoss-Farbe (Review-LOW-1):** für ein ≥2-Geschoss-Modell mit Wänden trägt
        das Raster **≥2 verschiedene** nicht-Hintergrund-Farben (belegt die „je Geschoss"-
        Unterscheidbarkeit, die das kombinierte Bild AK-konform macht).
      - **Boundary (leer):** leeres Modell → **gültiges**, weißes PNG (valide Chunks/CRC/Adler),
        **kein** Absturz, keine Tinte.
      - **Boundary (degenerierte BBox, Review-MED-2):** ein Modell mit **Geometrie**, aber
        **Breite ODER Höhe 0** (einzelne vertikale bzw. horizontale Wand → `has_geometry==true`,
        `max==min` in einer Achse) → valides PNG, **kein Div-durch-0/Absturz** (deckt den
        Fit-to-Canvas-Guard, den der leere Pfad NICHT übt).
      - **Negative (Integration):** nicht beschreibbarer Zielpfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den
        **echten** Adapter, **kein** Teil-Export (Muster `test_pdf_export`).
      - **Export-only:** `importModel(path, ExchangeFormat::Png)` → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
        (export-only-Lookup-Miss).
- [ ] **`tools/io-smoke.sh`**: `expect_export --export-png "$OUT/s.png"`.
- [ ] **`make gates` grün** (arch-check inkl. `io/`, lint 0, test inkl. neuer AK-Tests,
      coverage ≥ Schwelle) + **`make io-smoke`** grün; `make schema-check` unberührt.
      **Unabhängiges Code-Review vor Welle-Closure** (Muster 025b; **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** — keine
      neue Solid-Geometrie; die Encoder-Byte-Korrektheit trägt das voll-Decode-Orakel + ein
      empirischer Reader-Cross-Check durch den Reviewer). **ADR-Index-Impl-Folgepflicht auf
      „erfüllt durch slice-025b + slice-025c" flippen** (PNG-Hälfte geschlossen; Präzedenz 024a/b).
      Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/exchange_model_port.h` | ändern | `ExchangeFormat` additiv um `Png` |
| `src/adapters/io/png_writer.{h,cpp}` | neu | Raster-PNG-Encoder (Bitmap + Chunks + CRC/Adler/stored-DEFLATE) |
| `src/adapters/io/png_export_adapter.{h,cpp}` | neu | `PngExportAdapter : ModelExporterPort` (Fit-to-Canvas + Geschoss-Farbe) |
| `src/main.cpp` | ändern | `PngExportAdapter` + `ExporterMap`-Eintrag + `--export-png` |
| `src/adapters/CMakeLists.txt` | ändern | zwei neue `io/`-Quellen; keine neue Dependency |
| `tests/adapters/test_png_export.cpp` | neu | AK + voll-Decode-Orakel (inflate/Adler/CRC/Filter) + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) + export-only |
| `tests/CMakeLists.txt` | ändern | Test registrieren |
| `tools/io-smoke.sh` | ändern | `--export-png`-Smoke |
| `docs/plan/adr/README.md` | ändern (Closure) | [ADR-0016](../../adr/0016-pdf-png-backend.md)-Impl-Folgepflicht → „erfüllt durch 025b+025c" |
| `docs/reviews/{2026-07-01-slice-025c-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: [ADR-0016](../../adr/0016-pdf-png-backend.md) accepted,
  AK/§1 geschärft (025a), `plan_geometry`/`io_atomic_write`/`ExporterMap` real (025b/019c/020b/021b).

## 5. Closure-Trigger

- DoD vollständig, `make gates` + `make io-smoke` grün, Code-Review 0 HIGH → **PDF/PNG-Strang
  komplett**; danach Welle-4-Verifikation + Carveout-Audit → `done/welle-4-results.md` → **M4-Closure**.

## 6. Risiken und offene Punkte

- **Self-rolled PNG-Byte-Korrektheit (Kern-Risiko):** eine Byte-/Prüfsummen-Falschheit
  (CRC-32-Polynom/Byte-Ordnung, Adler-32-mod-65521, `NLEN` != ~`LEN`, zlib-Header, Scanline-
  Filterbyte) macht das PNG für Fremd-Leser ungültig. **Gegenmaßnahme:** das **voll-Decode-
  Orakel** inflatet den `IDAT`-Strom, prüft **jede** Scanline-Filterbyte, **Adler-32** und
  **CRC-32 je Chunk** — kein oberflächliches „nicht-leer" ([ADR-0016](../../adr/0016-pdf-png-backend.md)-Projektinhaber-MED,
  Muster 025b). Der Code-Reviewer belegt zusätzlich die reale Reader-Öffenbarkeit empirisch
  (außerhalb des Gates; kein Reader im gepinnten Image → **kein** [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Test-Dependency,
  wie 025b entschieden).
- **stored-DEFLATE-Blockgrenze:** Raw > 65535 B braucht mehrere Blöcke (nur der letzte
  `BFINAL`=1). Bei 800×600×3 + Filter ≈ 1,44 MB → ~23 Blöcke. Die Block-Schleife + `LEN`/
  `NLEN` je Block ist die fehleranfällige Stelle → das Orakel inflatet vollständig.
- **arch-check / keine Dependency:** der PNG-Encoder hat **keinen** externen Header (kein
  zlib/libpng-Link) → Regel A/B isolieren ihn („Regel F gegenstandslos"); `make build` zieht
  keine neue `find_package`/apt-Zeile. Vor dem Gate `grep`-Selbstcheck.
- **Fit-to-Canvas-Guard:** leere/degenerierte Bounding-Box (kein Segment / alle auf einem
  Punkt / Breite oder Höhe 0) → **kein Div-durch-0**: Guard auf weiße Leinwand bzw.
  Default-Skalierung. Boundary-Test deckt das leere Modell.
- **Bresenham-Klemmung:** `drawLine` klemmt auf die Leinwand (kein Out-of-Bounds-Schreiben);
  Fit-to-Canvas hält Achsen ohnehin im Rahmen, die Klemmung ist die Absicherung.
- **Lint (aus 025b gelernt, Review-LOW-2 präzisiert):** `bugprone-easily-swappable-parameters`
  feuert **nur** bei benachbarten gleichtypigen Skalaren, die **separat gespeichert/genutzt**
  werden (025b-Beleg: der Konstruktor `PdfWriter(double,double)` **flaggte** → `PdfPageSize`-
  Struct; `PdfWriter::line(double×4)` mit *zusammen* genutzten Koordinaten wurde unterdrückt).
  Konsequenz: der **`Bitmap`-Konstruktor** nimmt `RasterSize` (separat gespeichert → sonst
  Flag); Zeichen-Koordinaten (zusammen genutzt) sind unkritisch, `PixelSegment`/`Rgb` sind
  saubere Hygiene, kein Gate-Zwang. **Realer Lint-Druck** = `readability-function-cognitive-complexity`
  ≤ 20 (WarningsAsError) → Encoder in kleine Helfer (crc32/adler32/deflate-store/emit-chunk) zerlegt.
- **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a:** keine neue Solid-Geometrie (2D-Raster bestehender Achsen). Trotzdem
  **unabhängiges Code-Review** vor Closure (Encoder-Byte-Korrektheit, Reader-Öffenbarkeit,
  Atomarität, arch-Isolation).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Import/Export-Adapter (`src/adapters/io/`)

- **Modus:** GF; **Dichte:** hoch — Format-Adapter hinter `ModelExporterPort`, Encoder/Adapter-
  Split, Atomarität + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (Reuse `io_atomic_write`), arch-check Regel A/B,
  keine neue Dependency ([ADR-0016](../../adr/0016-pdf-png-backend.md) Option D). **Phase-Reife:** IO Phase 3.
  **Risiko:** mittel (self-rolled Binär-Encoder mit Prüfsummen → voll-Decode-Orakel als Netz).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; **Dichte:** hoch — GoogleTest, AK-ID im Namen, **voll-Decode-Orakel**
  (inflate + Adler + CRC + Filterbytes, nicht „nicht-leer"), Integration über den echten Pfad
  ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)), export-only ([`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

*(wird bei Closure gefüllt — DoD-Nachweis, `make gates`/`make io-smoke`, Code-Review, Lerneintrag)*
