# Unabhängiges Code-Review — slice-025c (PNG-Export)

**Datum:** 2026-07-01
**Commit bei Review:** `279014c` (feat(io): slice-025c PNG-Export)
**Reviewer:** unabhängig (Reviewer ≠ Autor — Muster slice-025b; Korrektheit vor Welle-Closure). MR-009 n/a (keine neue Solid-Geometrie), Code-Review dennoch pflichtig.
**Modus:** read-only + **empirische Verifikation** (echter Encoder kompiliert, Ausgabe durch reale Werkzeuge).

## Verdikt

**0 HIGH — Closure-fähig.** Der self-rolled PNG-Encoder erzeugt ein von **libpng tatsächlich
öffenbares**, spec-konformes PNG.

## Empirische Belege (nicht nur gelesen)

Der Reviewer kompilierte `png_writer.cpp` (+ Adapter/`plan_geometry`/`io_atomic_write`)
standalone (nur STL, kein OCC/Qt/zlib) und prüfte die Ausgabe:
- **Python `binascii`/`zlib`** (unabhängige Referenzen, NICHT der Test-Code): Signatur OK;
  alle Chunk-CRCs == `binascii.crc32(typ+daten)`; IHDR 800/600/bd8/ct2/interlace0; zlib-Header
  `78 01`, `%31==0`, FDICT=0; **22 stored-Blöcke**, je `NLEN==~LEN`, nur letzter BFINAL;
  Adler-32 == `zlib.adler32(raw)`; `zlib.decompress` == manuell konkatenierte Rohdaten;
  stride == 600·2401, jede Scanline Filterbyte 0.
- **Echter Reader PIL/libpng 10.2.0:** `Image.open` lädt als `PNG RGB (800, 600)`; out-of-bounds-
  Linie korrekt auf x=0..799 geklemmt.
- **Voller Adapter-Pfad:** 2 Geschosse → 2 unterscheidbare Farben, Fit-to-Canvas zentriert.
- **Degenerierte BBox** (vertikal/horizontal/Punkt/leer): je ein valides, von PIL geöffnetes
  PNG, **kein Absturz / kein Div-durch-0**.

Damit sind alle HIGH-Kandidaten widerlegt: PNG-Validität (libpng-öffenbar, alle Prüfsummen gegen
`binascii`/`zlib` bestätigt, 22-Block-stored-DEFLATE korrekt), Byte-Ordnung, Fit-to-Canvas-Guard
(alle 4 bbox-Kombinationen), Bresenham/Bitmap (bounds-checked, row-major), Atomarität/E-IO-001
(nullbyte-treu), arch-Isolation (OCC/Qt/zlib-frei — durch das Kompilat bewiesen), export-only
(nur ExporterMap, Import → E-IO-003), Refaktor-Regression (alle 6 Exporte + 2 Importe erhalten).

## Findings + Disposition

### LOW-1 — Test-Prüfsummen byte-identisch zur Produktion (kein gemeinsam-kopierter Algorithmus-Bug fiele auf)
`test_png_export.cpp` re-implementiert `oracleCrc32`/`oracleAdler32` (Review-MED-1 des Plans:
eigenständig, nicht via `png_writer`). Der Reviewer merkt an: da beide dieselbe IEEE-/zlib-Spec
korrekt umsetzen, würde ein in **beide** kopierter Algorithmus-Fehler nicht auffallen.
**Disposition: benannte Grenze, kein Fix.** Der empfohlene `<zlib.h>`-Referenz-Check wäre ein
**Test-Dependency** (zlib) → **[ADR-0004](../../spec/lastenheft.md)-Berührung** (neuer Link) → **nicht genommen**
(konsistent mit der PDF-poppler-Entscheidung 025b). Praktisch entschärft: (a) IEEE-Standard,
(b) der Reviewer hat **unabhängig** gegen `binascii`/`zlib` verifiziert (einmaliger empirischer
Cross-Check außerhalb des Gates).

### INFO-1 — Vermeidbare Kopien (Effizienz, kein Bug)
`emitChunk` baut `type + data` und hängt es an `out` (IDAT ~1,44 MB doppelt kopiert); `zlibStored`
erzeugt je Block ein `substr`-Temporary (×22). **Disposition: benannte Grenze** (Export ist
Einmal-Operation, kein Hot-Path; Streaming-CRC = späterer Cleanup-Kandidat).

## Ein-Satz-Verdikt
**Closure-fähig — ja;** 0 HIGH, ein real (libpng) öffenbares, spec-konformes PNG empirisch belegt
(Prüfsummen/Endianness/22-Block-DEFLATE gegen `binascii`/`zlib`); LOW-1 (kein zlib-Test-Dep,
ADR-0004) + INFO-1 (Effizienz) als benannte Grenzen.
