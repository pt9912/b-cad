---
id: slice-046b
titel: Sichtbarer PNG-Titelblock (Bitmap-Font) + PNG-tEXt-Provenance — die 046b-Naht
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008)]
adr_refs: [[ADR-0016](../../adr/0016-pdf-png-backend.md)]
---

# Slice 046b: Sichtbarer PNG-Titelblock (Bitmap-Font)

**Status:** **done (2026-07-24, `make gates` grün 266 Tests, committet).** Sichtbarer PNG-Provenance-Titelblock
(self-rolled 5×7-`png_font.h` + `Bitmap::drawText`, Fußzeile im weißen Unterrand) + injizierte PNG-`tEXt`
(`Date`/`Source`/`Version`, generische Liste → `png_writer` domänen-frei); PDF+PNG teilen `ExportProvenance::footerLine()`.
**Visuell verifiziert** (Golden-PNG zeigt „b-cad test | golden.bcad | 1970-01-01 00:00" lesbar). golden-check/io-smoke grün.
Der **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
2026-07-24: 0 HIGH → startbar**; encoder-nah → **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)** vor Welle-Closure empfohlen. **Entscheidungen (Review-MED/LOW eingearbeitet):**
**MED-1** `encodePng` nimmt eine **generische `(keyword,text)`-`tEXt`-Liste** (in `png_export_adapter` komponiert),
**nicht** den Domänen-`ExportProvenance` — `png_writer` bleibt domänen-frei (`drawText` ebenso). **LOW-2** die Fußzeile
wird in den **weißen Unterrand** gezeichnet (Geometrie liegt innerhalb `kMarginPx`); Ink-Sonde vergleicht **dasselbe
Modell** mit-vs-leer. **LOW-3** der bestehende `provenanceFooter()` (aktuell in `pdf_export_adapter`) wird in einen
**geteilten Helfer** (nahe `ExportProvenance`) gezogen — PDF+PNG teilen die byte-gleiche Zeichenkette (kein Drift).
**LOW-4** **1× (keine Skalierung)** — ~7 px hoch, für eine Fußzeile ausreichend (kein Gate auf Lesbarkeit).
**INFO-6** `setPixel` klemmt bereits (signed-Vergleich) → `drawText` ist out-of-bounds-sicher **by construction**
(Langer-String-Test bleibt als No-Crash-Regression).

**Welle:** welle-5-erweiterung. **Vorgänger:** [`slice-046a`](slice-046a-export-provenance.md) (done) — der
`ExportProvenance`-Vertrag steht, der PNG-Adapter bekommt die Herkunft bereits übergeben (aktuell **ungenutzt**).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

## 1. Ziel / Anlass

slice-046a macht die Export-Herkunft im **PDF sichtbar** (Fußzeile) und in STEP/STL-Headern — im **PNG** aber
**nicht sichtbar** (der `png_writer` hat **keinen Text-Rasterizer**, nur `drawLine`). Der Projektinhaber will die
Herkunft **sichtbar in PDF *und* PNG** (2026-07-24). 046b liefert den fehlenden PNG-Teil: einen **Bitmap-Font** +
die gerenderte Provenance-Zeile im Rasterbild. **Zusätzlich** der aus 046a zurückgestellte **PNG-`tEXt`-Nachzug**
(Datum/Quelle/Version als Metadaten-Chunks, injiziert).

**Determinismus:** unverändert das SOURCE_DATE_EPOCH-Muster — fixe Provenance im Golden → byte-stabil; der Font ist
eine **feste** Tabelle (keine externe Font-Datei, kein Qt). Leere Herkunft → kein Titelblock (Sentinel-Fall).

## 2. Definition of Done

- [ ] **Fester Bitmap-Font** (5×7 o. ä., **ASCII 32–126**) als Konstanten-Tabelle in `png_writer` (o. `png_font.{h,cpp}`);
      self-rolled/public-domain (**kein** Qt, keine externe Datei) — Provenienz/Lizenz im Kommentar (R1). Unbekannte
      Zeichen → definierter Fallback (Leerzelle/Box), **kein** Wurf.
- [ ] **`Bitmap::drawText(PixelPoint origin, const std::string& text, Rgb color)`** (nutzt das private `setPixel` +
      die Font-Tabelle; Zeichen-Advance + Zeilen-Clipping an den Canvas-Rändern, **kein** Out-of-Bounds-Schreiben).
- [ ] **`png_export_adapter` rendert die Provenance-Fußzeile** (dieselbe Komposition wie der PDF-Footer:
      „`<version> | <quelle> | <datum>`", nur nicht-leere Teile) unten ins Bitmap, **wenn** Herkunft injiziert ist.
      Leer → keine Zeile (Sentinel; Alt-PNG byte-gleich).
- [ ] **PNG-`tEXt`-Provenance** (046a-Nachzug): `encodePng` emittiert zusätzlich `tEXt` `Date`/`Source`/`Version`
      (nur nicht-leere), neben den statischen `Software`/`Title` — `encodePng` nimmt die Provenance (o. eine
      `tEXt`-Liste, damit `png_writer` domänen-frei bleibt). **Kein** `tIME`.
- [ ] **Golden re-baseliniert:** `make golden-regen` erzeugt `model.png` neu (feste `goldenProvenance()` → sichtbarer
      Text + `tEXt`); `GoldenExport.PngByteIdentical` + `make golden-check` grün. Die übrigen fünf Golden **unberührt**.
- [ ] **AK** (`test_png_export.cpp`): (a) **Ink-Sonde** — ein PNG mit injizierter Herkunft trägt **mehr Tinte** (die
      Font-Pixel) als ohne (analog der Hilfslinien-Ink-Sonde); (b) **Unterscheidbarkeit** — verschiedene Herkunft →
      verschiedene PNG-Bytes; (c) `tEXt` `Date`/`Source`/`Version` präsent, **kein** `tIME`; (d) das Voll-Decode-Orakel
      (Inflate/Adler/CRC/Filter, `front()==IHDR`/`back()==IEND`) bleibt grün.
- [ ] **Doku:** [CHANGELOG](../../../../CHANGELOG.md) + `tests/adapters/golden/README.md` (PNG trägt jetzt sichtbare
      Herkunft + `tEXt`). **Kein** ADR-Index-Eintrag ([ADR-0016](../../adr/0016-pdf-png-backend.md) unverändert gültig).

## 3. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/io/png_writer.{h,cpp}` | ändern | Font-Tabelle + `Bitmap::drawText`; `encodePng` um `tEXt`-Provenance |
| `src/adapters/io/png_export_adapter.cpp` | ändern | Provenance-Fußzeile ins Bitmap rendern (Param ist schon da) |
| `tests/adapters/test_png_export.cpp` | ändern | Ink-Sonde + Unterscheidbarkeit + `tEXt`-Präsenz + Decode-Orakel grün |
| `tests/adapters/golden/model.png` | neu erzeugt | re-baseliniert (sichtbarer Text + `tEXt`) |
| `CHANGELOG.md` | ändern | Doku |

**Bewusst NICHT Teil:** IFC-`FILE_NAME`-Injektion (eigener kleiner Nachzug, nicht PNG); DXF-Provenance; Font-Skalierung/
Mehrzeiligkeit/Anti-Aliasing (eine feste Größe, eine Zeile).

## 4. Risiken

- **R1 — Font-Tabelle (der Posten):** ~95 Glyphen × 7 Byte; Umfang klein halten (eine feste Größe, ASCII). Lizenz/
  Provenienz sauber (self-rolled o. public-domain 5×7). Ink-Sonde + Golden als Netz.
- **R2 — Clipping:** `drawText` darf **nie** außerhalb des Canvas schreiben (Provenance-String beliebig lang) →
  Advance + Rand-Clip; Test mit langem String.
- **R3 — Golden-Determinismus:** feste Provenance + feste Font-Tabelle → byte-stabil; ein versehentlich
  variabler Wert bräche `golden-check` (Netz).
- **R4 — Lesbarkeit vs. Auflösung:** 800×600, 5×7-Font unten — muss lesbar sein (ggf. 2× skaliert per Pixel-Block).
  Review/`acc`-Sicht entscheidet die Größe.

## 5. Trigger

- Projektinhaber-Wunsch: Herkunft **sichtbar in PDF und PNG** (2026-07-24); PDF ist in 046a erledigt, PNG folgt hier.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
