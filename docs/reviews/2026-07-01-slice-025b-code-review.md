# Unabhängiges Code-Review — slice-025b (PDF-Export)

**Datum:** 2026-07-01
**Commit bei Review:** `6a4a5b9` (feat(io): slice-025b PDF-Export)
**Reviewer:** unabhängig (Reviewer ≠ Autor, ohne Entstehungs-Kontext — Muster slice-021b; Korrektheit vor Welle-Closure). MR-009 n/a (keine neue Solid-Geometrie), Code-Review dennoch pflichtig.
**Modus:** read-only + **empirische Verifikation** (das echte `pdf_writer.cpp` kompiliert, Ausgabe durch reale Reader geprüft).

## Verdikt

**0 HIGH — Closure-fähig.** Die milestone-kritische Behauptung („valide PDF, die ein
Standard-Reader öffnet", [ACC-004](../../spec/lastenheft.md)) wurde **empirisch** belegt.

## Empirische Belege (nicht nur gelesen)

Das unveränderte `pdf_writer.cpp` wurde kompiliert, mit der Adapter-Zeichenlogik
gefüttert und die Ausgabe durch **echte Reader** geprüft:
- **`pdfinfo` (poppler):** öffnet sauber, 2 bzw. 1 Seite, „Page size 595 x 842 pts (A4)",
  PDF 1.7, keine Warnung.
- **`pdftotext`:** extrahiert „M 1:100" genau einmal je Geschoss-Seite.
- **ghostscript:** rendert beide fehlerfrei (Rahmen + Achsen + Label, Raster hat Tinte).
- **Unabhängiger Byte-Parser:** alle xref-Offsets zeigen exakt auf „N 0 obj", `/Length`
  == reale Stream-Bytes, `trailer /Size` == xref-Size == Objektzahl+1, `startxref` ==
  xref-Offset. **Kein Off-by-one** (Offsets via `out.size()` konstruktiv gesetzt).

Damit sind alle HIGH-Kandidaten widerlegt: PDF-Validität (real öffenbar, byte-exakt),
Maßstäblichkeit (`(72/25.4)/100`, 5000 mm → 141,73 pt, kein Y-Flip, Label synchron),
Atomarität/E-IO-001 (byte-identisch zum dxf-Muster, kein Teil-Export), arch-Isolation
(kein OCC/Qt, domänenfreier Writer), export-only (nur ExporterMap, Import → E-IO-003),
Totalität (leeres Modell → gültige leere Seite, kein Div-durch-0), Orakel-Güte
(voll-Decode, kein Durchrutschen). Strict-Compile (`-Wall -Wextra -Wconversion
-Wsign-conversion -Wshadow`): 0 Warnungen.

## Nachrangige Findings + Disposition

### LOW-1 — Fester Maßstab ohne Fit/Clip: sehr große Modelle laufen aus der MediaBox
`pdf_export_adapter.cpp` — bei 1:100 überschreitet ein Gebäude breiter als ~19,5 m den
A4-Rand (Reader clippen still). Für den M4-Beleg (Demo 8×6 m) und [ACC-004](../../spec/lastenheft.md)
**korrekt** — fester Maßstab ist der bewusst gewählte Vertrag (ADR-0016; Maß-Treue vor
Seiten-Fit). **Disposition: benannte Grenze** (Closure-Notiz + ADR-0016 §Trigger
Fit-to-Page); kein Fix.

### LOW-2 — `fsync`-Rückgabe ignoriert (`io_atomic_write.cpp`)
Ein Medienfehler *während* fsync bliebe unerkannt. **Byte-identisch zum etablierten
dxf/ifc-Muster** (kein Regress). **Disposition: benannte Grenze** (Cross-Adapter-Cleanup-
Kandidat: fsync-Rückgabe für **alle** io-Adapter konsistent prüfen — out of scope hier).

### LOW-3 — Orakel pinnt `trailer /Size` nicht + Maßstabs-Sonde flip-invariant
(a) `/Size` wurde nicht gegen die Objektzahl geprüft; (b) `hypot`-Sonde bliebe bei einem
hypothetischen Y-Flip blind. Beide heute korrekt.
**Disposition: eingearbeitet.** Test gehärtet — `/Size`-Assert (StructureTest) +
`PreservesAxisOrientationNoYFlip` (Modell +Y → Seiten +Y, Vorzeichen-Sonde).

### INFO-1 — Test-Konstante `kMmToPt` dupliziert (Drift durch `(M 1:100)`-Label-Assert abgefangen). Keine Aktion.
### INFO-2 — `pad10`/`object_count`-Ränder erst bei unrealistischen Größen (>10 GB) relevant. Keine Aktion.
### INFO-3 — Pfad „Geschosse ohne Wände" nicht direkt getestet.
**Disposition: eingearbeitet.** `StoreysWithoutWallsProduceFramedPages` ergänzt.

## Ein-Satz-Verdikt
**Closure-fähig — ja;** 0 HIGH, real öffenbare/maßstäbliche/atomare/export-only/arch-konforme
PDFs empirisch belegt; LOW-3 + INFO-3 als Test-Härtung eingearbeitet, LOW-1/-2 als benannte
Grenzen festgehalten.
