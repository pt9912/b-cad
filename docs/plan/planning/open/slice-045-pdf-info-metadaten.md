---
id: slice-045
titel: PDF- und PNG-Export tragen statische Erzeuger-/Titel-Metadaten — determinismus-neutral
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007), [LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008)]
adr_refs: [[ADR-0016](../../adr/0016-pdf-png-backend.md)]
---

# Slice 045: PDF- und PNG-Export tragen statische Metadaten

**Status:** open (Plan — **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
2026-07-24: 1 HIGH / 1 MED / 1 LOW / 3 INFO → HIGH in-Plan aufgelöst, startbar**). Der HIGH-1 (dateinamen-abgeleiteter
Titel bräche den 044a-Byte-Golden) ist durch die Festlegung auf einen **festen statischen Titel** behoben; MED-1
(PDF-Test-Objektzahl-Konstanten), LOW-1 (Golden-README) + INFO-3 (Spec-Notiz) eingearbeitet. **Zweites unabhängiges
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) (2026-07-24): 0 HIGH → startbar bestätigt**; dessen MED-1 (Sequenzierung — s. §6 R4), LOW-1 (PNG zwei `tEXt`
verbindlich), LOW-2 (Titel direkt asserten), LOW-3 (Escape-Pfad un-getestet) + die „/Info-nicht-XMP"-Begründung
eingearbeitet. **START BLOCKIERT bis [`slice-044a`](../done/slice-044a-golden-export-infra.md)-Closure** (§6 R4).
Kleiner Produktions-Slice.

**Welle:** welle-5-erweiterung. **Vorgeschichte:** slice-025b/025c (self-rolled Vektor-PDF `pdf_writer` +
Raster-PNG `png_writer`), slice-044a (Byte-Golden aller Export-Formate + `make golden-regen`/`golden-check` — die
Infrastruktur, mit der PDF-/PNG-Golden hier byte-genau neu erzeugt + verifiziert werden).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

## 1. Ziel / Anlass

b-cads PDF- **und** PNG-Export tragen **keine Erzeuger-/Titel-Metadaten** — im Viewer sind Erzeuger/Titel leer.
Das ist ein **Produkt-Mangel** (die Dateien entstehen für den Benutzer), zugleich **inkonsistent**: STEP und IFC
nennen `b-cad` als Erzeuger (STEP `originating_system='b-cad'` seit slice-044a; IFC `FILE_NAME`-Autor `b-cad`) —
PDF und PNG nennen nichts.

**Fix:** je Format **statische** Erzeuger-/Titel-Felder:
- **PDF:** ein klassisches `/Info`-Dictionary aus **konstanten Strings** (`/Producer (b-cad)`, `/Creator (b-cad)`,
  `/Title (b-cad Plan-Export)`). **Bewusst kein XMP**-Metadaten-Stream — XMP trägt konventionell eine dynamische
  Dokument-UUID (nicht-deterministisch); das `/Info`-Dict aus Konstanten bleibt byte-stabil (2te-Review-Hinweis).
- **PNG:** **zwei** `tEXt`-Chunks — `Software=b-cad` **und** `Title=b-cad Plan-Export` (verbindlich, spiegelt PDFs
  Producer+Title; LOW-1: nicht „optional").

**Determinismus-Kern (beide):** ausschließlich **konstante** Felder — **kein** `/CreationDate`/`/ID` (PDF), **kein**
`tIME`-Chunk (PNG), kein Wall-Clock-/Hash-Wert. Der Export bleibt byte-deterministisch (die slice-044a-Byte-Golden
+ die slice-025b/025c-Decode-/Roundtrip-Tests bleiben tragfähig; PDF- und PNG-Golden werden **einmal** per
`make golden-regen` neu baseliniert).

**Abgrenzung/Reconciliation:** die slice-044-Determinismus-Tabelle notierte „PDF … kein `/CreationDate`/`/ID`/
`/Producer`" bzw. „PNG … kein `tIME`" — das war eine **Beschreibung des Ist-Zustands**, keine Anforderung.
**Statische** Metadaten sind determinismus-neutral; die Invariante verbietet nur **dynamische** Felder. Dieser
Slice schärft die Regel: *statisch erlaubt, dynamisch verboten.*

## 2. Definition of Done

### PDF
- [ ] **`src/adapters/io/pdf_writer.{h,cpp}`:** ein zusätzliches **indirektes Objekt** `/Info`
      (`<< /Producer (b-cad) /Creator (b-cad) /Title (…) >>`) wird **nach** den Seiten-Objekten angehängt (kein
      Umnummerieren); `object_count`, `xref` (Offset + `0 N`-Zeile) und `trailer` (`/Size` **und** neu
      `/Info <n> 0 R`) sind konsistent nachgezogen. PDF-String-Escaping (`(`/`)`/`\`) beachten. **Keine**
      dynamischen Felder.

### PNG
- [ ] **`src/adapters/io/png_writer.{h,cpp}`:** ein statischer `tEXt`-Chunk (`Software\0b-cad`) wird via
      `emitChunk(out, "tEXt", …)` **nach `IHDR`, vor `IDAT`** eingefügt (gültige Chunk-Position; CRC-32 über
      Typ+Daten wie die anderen Chunks). **Kein** `tIME`-Chunk. Der `IDAT`-stored-DEFLATE-Pfad bleibt unberührt.

### gemeinsam
- [ ] **Fester statischer Titel** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-045-HIGH-1):
      ein **konstanter** Wert (Vorschlag `b-cad Plan-Export`) als `/Title` (PDF) bzw. `tEXt Title=b-cad Plan-Export`
      (PNG), komplett in `pdf_writer.cpp`/`png_writer.cpp`. **NICHT** aus Aufruf/Dateiname ableiten: `golden_gen`
      schreibt `model.pdf` (Stamm `model`), der Byte-Test re-exportiert nach `bcad_golden_model.pdf` (Stamm
      `bcad_golden_model`) → ein dateinamen-abgeleiteter Titel bräche `GoldenExport.{Pdf,Png}ByteIdentical` in
      `make gates`. Damit entfällt jede Titel-/Pfad-Durchreichung durch die Adapter.
- [ ] **Tests** (`tests/adapters/test_pdf_export.cpp` + `test_png_export.cpp`): (a) das Artefakt trägt Erzeuger
      **und Titel** — `/Producer (b-cad)` **+** `/Title (b-cad Plan-Export)` bzw. `tEXt Software=b-cad` **+**
      `tEXt Title=b-cad Plan-Export`; der **Titel direkt** asserted (LOW-2: er trägt HIGH-1, darf nicht nur
      indirekt über den Byte-Golden gedeckt sein); (b) **Negative-Determinismus-Sonde:** **kein**
      `/CreationDate`/`/ID` (PDF) bzw. **kein** `tIME` (PNG) im Artefakt (fail-closed); (c) zwei Läufe byte-identisch.
- [ ] **PDF-Objektzahl-Konstanten nachziehen** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-045-MED-1):
      das zusätzliche `/Info`-Objekt erhöht die Objektzahl um 1 → die **hart kodierten** Erwartungen in
      `test_pdf_export.cpp` anpassen (`EXPECT_EQ(objects, 7)` → 8; `verifyXrefOffsets`-Erwartungen 5 → 6 [leeres
      Modell] bzw. 7 → 8). Der generische `verifyXrefOffsets`-Helfer bleibt gültig und **prüft neu den xref-Offset
      des `/Info`-Objekts mit** (Gratis-Check). **PNG-Seite unberührt:** `parseChunks` ist positions-agnostisch +
      CRC-prüft jeden Chunk → ein neuer `tEXt` passiert das Voll-Decode-Orakel (025c) ohne Änderung.
- [ ] **Golden neu baseliniert:** `make golden-regen` erzeugt `tests/adapters/golden/model.pdf` **und** `model.png`
      neu (die übrigen vier Golden bleiben byte-gleich); `GoldenExport.{Pdf,Png}ByteIdentical` + `make golden-check`
      grün. Der Golden-Diff (nur die neuen Metadaten-Felder) im Review nachvollziehbar (PNG bleibt binär → Größen-/
      Chunk-Diff statt Text).
- [ ] **Gates:** `make gates` grün; `make golden-check` grün; `make io-smoke` grün (PDF/PNG exit 0, nicht leer).
- [ ] **Doku:** [CHANGELOG](../../../../CHANGELOG.md) ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog))
      + `tests/adapters/golden/README.md` (LOW-1: PDF/PNG tragen jetzt statische `/Info`/`tEXt`, weiterhin **keine**
      dynamischen Felder). **Kein** ADR-Index-Eintrag ([ADR-0016](../../adr/0016-pdf-png-backend.md) bleibt gültig,
      nur konkretisiert).
- [ ] **Lösungsfreie Spec-Notiz** (INFO-3, damit das Feature einen Anforderungs-Anker hat): in `spec/spezifikation.md`
      eine benutzer-beobachtbare Zeile „Export-Metadaten sind **statisch** (Erzeuger/Titel), **nie dynamisch**
      (kein Datum/`/ID`/`tIME`)" — lösungsfrei ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei);
      **kein Lastenheft-Eingriff**, [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/[008](../../../../spec/lastenheft.md#lh-fa-io-008) bleiben wörtlich).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/io/pdf_writer.cpp` (+ ggf. `.h`) | ändern | statisches `/Info`-Objekt + xref/trailer nachziehen |
| `src/adapters/io/png_writer.cpp` (+ ggf. `.h`) | ändern | statischer `tEXt`-Chunk (`Software=b-cad`, `Title=…`) nach IHDR |
| `tests/adapters/test_pdf_export.cpp` | ändern | `/Info`-Präsenz + Negative-Sonde + **Objektzahl-Konstanten 7→8 / xref 5→6 / 7→8** (MED-1) |
| `tests/adapters/test_png_export.cpp` | ändern | `tEXt`-Präsenz + Negative-Sonde (`tIME` fehlt); Voll-Decode-Orakel unberührt |
| `spec/spezifikation.md` | ändern | lösungsfreie Zeile „statische Export-Metadaten, keine dynamischen" (INFO-3) |
| `tests/adapters/golden/model.{pdf,png}` | neu erzeugt | Golden re-baseliniert (`make golden-regen`) |
| `CHANGELOG.md` | ändern | Doku-Nachzug |

**Bewusst NICHT Teil:** dynamische Metadaten (Datum/`/ID`/`tIME`) — determinismus-brechend; die **PNG-Dateigröße/
-Kompression** (1,4 MB stored-DEFLATE — orthogonale, separat offene Entscheidung, kein Metadaten-Thema);
DXF/STL-Metadaten (Formate ohne Erzeuger-Feld-Konvention im b-cad-Subset).

## 4. Determinismus-Grundlage (der Prüfstein)

| Feld | Format | erlaubt? | Grund |
|---|---|---|---|
| `/Producer`/`/Creator`/`/Title` (fest) | PDF | ✅ | konstant → byte-stabil |
| `tEXt Software=b-cad` (+ fester `Title`) | PNG | ✅ | konstant → byte-stabil |
| `/CreationDate`/`/ModDate`/`/ID` | PDF | ❌ | Wall-Clock/Hash → nicht-reproduzierbar |
| `tIME`-Chunk | PNG | ❌ | Wall-Clock → nicht-reproduzierbar |

## 5. Trigger

- Produkt-Beobachtung des Projektinhabers (2026-07-24): PDF-/PNG-Viewer zeigen keine Metadaten; Inkonsistenz zu
  STEP/IFC (die `b-cad` nennen).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.

## 6. Risiken

- **R1 — versehentlich dynamisches Feld** (`/ID`, `tIME`, Datum): der Byte-Golden + die Negative-Determinismus-
  Sonde fangen es fail-closed.
- **R2 — Byte-Struktur:** PDF-`xref`-`/Size`/`startxref` müssen nach dem zusätzlichen Objekt exakt stimmen
  (Reader-Öffenbarkeits-Test 025b-MED-1 = Netz); der PNG-`tEXt`-Chunk braucht korrekte Länge + CRC-32 an gültiger
  Position (der PNG-Voll-Decode-Test 025c = Netz).
- **R3 — Golden-Churn:** nur PDF- und PNG-Golden ändern sich; die übrigen vier bleiben byte-gleich.
- **R4 — Sequenzierung: BLOCKIERT bis [`slice-044a`](../done/slice-044a-golden-export-infra.md)-Closure**
  (2te-Review-MED-1). slice-045 re-baseliniert `model.pdf`/`model.png` **auf den 044a-Golden auf**; solange 044a
  implementiert-aber-nicht-committet/geschlossen ist ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH, aber nicht abgenommen), würde 045 frisch
  erzeugte, noch nicht abgenommene Golden re-baselinieren. **Start erst nach 044a → `done`.**
- **R5 — Escape-Pfad bleibt un-getestet** (LOW-3): der ASCII-Titel `b-cad Plan-Export` treibt `escapePdfString`
  (`pdf_writer.cpp`, anon. Namespace) **nicht** in den Escape-Zweig (kein `(`/`)`/`\`) — den Pfad **nicht** als
  „getestet" verbuchen (coverage-neutral, kein Fehler).
