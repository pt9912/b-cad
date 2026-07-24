---
id: slice-046a
titel: Export-Provenance — Datum/Quelle/Version injizierbar + sichtbar (PDF), STEP/STL-Header (die 046a-Naht)
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007), [LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008)]
adr_refs: [[ADR-0016](../../adr/0016-pdf-png-backend.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md), [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)]
---

# Slice 046: Export-Provenance (sichtbar + Metadaten, injizierbar)

**Status:** **done (046a — 2026-07-24, `make gates` grün, 265 Tests, committet).** Umbrella slice-046 auf die
046a-Naht geschnitten (Datum/Version injizierbar + sichtbar PDF + STEP/STL-Header); **046b** (sichtbarer PNG-Titelblock
via Bitmap-Font) + der IFC-`FILE_NAME`/PNG-`tEXt`-Nachzug bleiben **offen** (eigene spätere Slices, kein Skelett —
in dieser Closure-Notiz + Roadmap als Deferral verankert, [MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)).
Der **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
2026-07-24: 1 HIGH / 2 MED** wurde aufgelöst (HIGH-1 via Option A, MED-1/2 eingearbeitet). Determinismus-Kern + PDF/PNG-Kostensplit **bestätigt solide**. **MED-1**
(`BCAD_VERSION` ist PRIVATE auf `bcad_hexagon` → `main.cpp` sieht es nicht; via `bcad_version()`-Accessor) + **MED-2**
(STL-80-Byte-Header muss auf ≤80 geklemmt werden, sonst `size_t`-Underflow/Crash) **eingearbeitet**. **HIGH-1 via Option A
gelöst → startbar:** die „Quelle = SQLite-Dateiname" ist **aktuell unerreichbar** (`main.cpp` lädt nie aus SQLite,
jeder Export ist die in-memory [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Demo). **Gewählt (Projektinhaber
2026-07-24): Option A** — slice-046 liefert **Datum + Version** (echt) + die Quelle-Mechanik mit `(ungespeichert)`-
Fallback; **„Quelle" wird real durch den Folge-Slice [`slice-047`](../open/slice-047-projekt-oeffnen.md)** („Projekt öffnen").
Auflösung s. §0.1. Quergreifende Feature-Slice; **Split empfohlen** (§0).

> **046a IMPLEMENTIERT 2026-07-24 (closure-bereit, `make gates` grün, 265 Tests):** `ExportProvenance`-Vertrag durch
> den Port (`exportModel`/`write`, optional → leer = 044a/045-Sentinels), `main.cpp` füllt echte Werte (Uhr/Version;
> Quelle leer bis slice-047). **Sichtbar** im PDF (Fußzeile) + eingebettet in **STEP** (`FILE_NAME`) und **STL**
> (80-Byte-Header, geklemmt). **Unterscheidbarkeits-AK** (verschiedene Herkunft → verschiedene Datei) über PDF/STEP/STL;
> **slice-045 gefaltet** (statische `/Info`/`tEXt` bleiben). MED-1 via **Reuse** `application_banner()` (kein neuer
> Accessor nötig), MED-2 STL-Klemmung eingearbeitet. **Nachzug (046a-Rest / 046b):** IFC-`FILE_NAME`- + PNG-`tEXt`-
> Injektion (Signatur steht, Rendering offen) + **046b** sichtbarer PNG-Titelblock (Bitmap-Font). **Geschlossen +
> committet 2026-07-24** (slice-045 nach `done/` gefaltet-retired; Sentinel gesetzt). **Netz-Lehre:** STEP ist im
> **selben Prozess** nicht byte-stabil (OCC globaler Entity-Zähler-State) — cross-Prozess deterministisch
> (`golden-check`/`GoldenExport.Step`); In-Prozess-Determinismus-Assertion daher raus, Unterscheidbarkeits-`EXPECT_NE`
> bleibt.

**Welle:** welle-5-erweiterung. **Vorgeschichte:** slice-025b/025c (self-rolled PDF/PNG-Writer), slice-044a
(STEP-Header byte-determiniert + Byte-Golden-Netz + `make golden-regen`), slice-045 (statische PDF/PNG-Metadaten —
**wird von dieser Slice abgelöst/erweitert**, s. §7).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

## 1. Ziel / Anlass

**Projektinhaber-Fund (2026-07-24):** ein Benutzer, der ein exportiertes PDF/PNG **anschaut**, muss erkennen, **aus
welchem Stand** es stammt — **Datum**, **Quelle** (Projektdatei), **b-cad-Version**. Aktuell fehlt das: der auf
Byte-Determinismus optimierte Export (STEP-Sentinel in 044a, statische PDF/PNG-Metadaten in 045) macht Exporte
**verschiedener** Modellstände **byte-identisch** → **nicht unterscheidbar**. Für das Test-Netz optimiert, fürs
Produkt kaputt.

**Auflösung — Determinismus und Herkunft schließen sich NICHT aus (das „SOURCE_DATE_EPOCH"-Muster):** die Herkunft
wird **injiziert**, nicht aus der Uhr im Writer gezogen:
- Die **reinen Writer/Adapter rufen NIE die Uhr** — sie rendern die Provenance, die sie **übergeben** bekommen.
- Der **Composition-Root** (`main.cpp`) füllt die **echten** Werte (Systemuhr, Projektpfad, `BCAD_VERSION`).
- **Tests/Golden** übergeben **feste** Werte → das Byte-Golden bleibt deterministisch (und fängt weiter
  Encoder-Drift, jetzt inkl. der Provenance-Darstellung).

→ Echte Nutzer sehen echtes Datum/Quelle/Version; das Test-Netz funktioniert weiter. Die Uhr wird **einmal** im
Composition-Root berührt — hexagonale Reinheit ([ADR-0001](../../adr/0001-hexagonale-architektur.md)) bleibt.

## 2. Provenance-Vertrag (Werte)

| Feld | Quelle (Produktion) | Golden/Test (fix) | Form |
|---|---|---|---|
| **Datum/Zeit** | Systemuhr (Composition-Root) | fester Wert (z. B. `1970-01-01 00:00`) | vom Root **vorformatierter** String |
| **Quelle** | **Dateiname** der SQLite-Projektdatei (Projektinhaber-Wahl 2026-07-24) — **nur Basename**, kein voller Pfad (kein Verzeichnis-Struktur-Leak in geteilte Dokumente); in-memory/ungespeichert → `(ungespeichert)` | `golden.bcad` o. ä. | String |
| **Version** | Kern-Accessor `bcad_version()` (liest `BCAD_VERSION`; MED-1: `BCAD_VERSION` ist **PRIVATE** auf `bcad_hexagon`, `main.cpp` sieht es nicht direkt — Muster `application_banner()` in `bootstrap_info`) | fester Wert | String |
| ~~Revision~~ | **weggelassen** (b-cad trackt keine Modell-Revision; [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) kann re-eval) | — | — |

**Kern-Reinheit:** `ExportProvenance` ist ein **framework-freier Wert-Struct** (drei `std::string`, alle vom
Composition-Root **vorformatiert** — der Kern/Adapter kennt **kein** `<chrono>`, keinen Pfad-Lookup). Damit bleibt
die Uhr-/Umgebungs-Berührung im Root; a-check-Schichtung unberührt.

## 0. Sizing / Split ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start): Split empfohlen)

Der teure Posten ist der **sichtbare PNG-Titelblock** — der `png_writer` hat **keinen Text-Rasterizer** (nur
`drawLine`), sichtbarer PNG-Text braucht einen **Bitmap-Font**. Empfohlener Schnitt:
- **046a — injizierbare Provenance + sichtbarer PDF-Titelblock + Metadaten (alle Formate):** der `ExportProvenance`-
  Input durch den Port; PDF-Titelblock (billig via `text()`) + PDF-`/Info`+`/CreationDate`; PNG-`tEXt`; STEP/IFC-
  `FILE_NAME` (echtes Datum/Version, **revidiert 044a**); STL-80-Byte-Header; **löst slice-045 auf**. Golden fix.
- **046b — sichtbarer PNG-Titelblock:** ein minimaler **Bitmap-Font** (5×7 o. ä.) im `png_writer` + der gerenderte
  Titelblock. Isolierbar, testbar (Ink-Sonde + Golden).

**„Sichtbar in PDF und PNG" bleibt erfüllt** — über 046a (PDF) + 046b (PNG), zwei sequenzielle Slices.

## 0.1 HIGH-1 — „Quelle" ist ohne „Projekt öffnen"-Fluss unerreichbar (Start-Blocker)

Das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review
fand: `main.cpp` verdrahtet den `SqliteProjectRepository` **nicht** — jeder Export (CLI **und** GUI) baut die
**in-memory `buildAcc001KernDemo`** und exportiert die. Es gibt **keinen** „Projekt aus Datei öffnen"-Fluss; die
Save/Load-Fähigkeit lebt nur im Adapter, un-verdrahtet. → **„Quelle = Dateiname" träfe zu 100 % den
`(ungespeichert)`-Fallback** — ein Drittel des beworbenen Werts ist tot. **HIGH: blockiert den Start bis entschieden.**

**Option A — ehrlich verschlanken (empfohlen für JETZT):** slice-046 liefert **Datum + Version** (beide erreichbar,
sofort echt); **Quelle** bleibt vorerst der `(ungespeichert)`-Fallback und wird als **eigener Vorbedingungs-Slice**
(„Projekt aus SQLite öffnen" — Repository in `main.cpp` verdrahten + `--open <pfad>` + geladenen Basename tracken)
nachgezogen; erst danach wird „Quelle" im Provenance-Block **echt**. Kleinerer, sofort lieferbarer 046-Schnitt.

**Option B — „Projekt öffnen" in slice-046 hineinnehmen:** den `SqliteProjectRepository`-Load in `main.cpp`
verdrahten + `--open`-Pfad + Basename-Tracking, **damit „Quelle" sofort echt** ist. **Materiell größer** als das
§0-Sizing zugibt (ein eigenes Feature „Projekt-Öffnen", quer zur Provenance).

**→ GEWÄHLT (Projektinhaber 2026-07-24): Option A.** slice-046 liefert **Datum + Version** injizierbar + die
Quelle-Mechanik mit `(ungespeichert)`-Fallback; „Quelle" wird **real** durch **[`slice-047`](../open/slice-047-projekt-oeffnen.md)**
(„Projekt aus SQLite öffnen" — schließt zugleich die vom Review aufgedeckte Lücke, dass b-cad **gar keine** gespeicherten
Projekte öffnen kann; dessen letzte DoD-Zeile reicht den geladenen Basename in den 046-Provenance-Vertrag). HIGH-1
damit aufgelöst → **startbar**. (Die restliche Mechanik — Datum/Version, sichtbar PDF/PNG, alle Header — ist unberührt.)

## 3. Definition of Done

### Kern + Verdrahtung (046a)
- [ ] **`ExportProvenance`-Wert-Struct** (`src/hexagon/model/` oder Port-nah; drei vorformatierte `std::string`:
      `date`, `source`, `version`). Framework-frei; **kein** `<chrono>`/Pfad-Lookup im Kern/Adapter.
- [ ] **Export-Port geweitet:** `ExchangeModelPort::exportModel(building, path, format, provenance)` +
      `ModelExporterPort::write(building, derived, path, provenance)`. `ExchangeService` reicht durch. **Bestehende
      Aufrufer/Tests nachgezogen** (Signatur-Änderung ist quergreifend — a-check-/build-Fallout einplanen).
- [ ] **Composition-Root (`main.cpp`) füllt echte Werte:** Datum aus der Systemuhr (**einzige** Uhr-Berührung,
      vorformatiert), Quelle = Basename des geladenen SQLite-Pfads (`(ungespeichert)`-Fallback; Erreichbarkeit s. §0.1),
      Version über den **Kern-Accessor `bcad_version()`** (MED-1 — **nicht** direkt `BCAD_VERSION`, das ist PRIVATE auf
      `bcad_hexagon`; neuer Accessor in `bootstrap_info`, Muster `application_banner()`). `io-smoke`/Demo entsprechend.

### Sichtbar + Metadaten (046a: PDF + alle Header; 046b: PNG sichtbar)
- [ ] **PDF sichtbarer Titelblock** (`pdf_writer`/`pdf_export_adapter`, `text()`): eine dezente Fuß-/Kopfzeile
      „b-cad <version> · <quelle> · <datum>" auf der Seite (neben „M 1:100"). **Plus** `/Info` (`/Producer`/`/Title`)
      **und** `/CreationDate` (jetzt **injiziert erlaubt**).
- [ ] **PNG sichtbarer Titelblock (046b):** minimaler Bitmap-Font im `png_writer`; derselbe Provenance-String
      gerendert. **Plus** `tEXt` (`Software`/`Title`/`Source`/`Date`).
- [ ] **Daten-Format-Header (046a):** STEP/IFC `FILE_NAME` tragen **echtes** Datum + `b-cad <version>` (revidiert
      044a-Sentinel → injiziert); STL nutzt den **80-Byte-Header** für „b-cad <version> <quelle> <datum>“ (bislang
      fixer Banner `"b-cad binary STL export"`) — **MED-2: der zusammengesetzte String MUSS auf ≤80 Byte geklemmt
      werden** (das aktuelle `out.append(80 - banner.size(), '\0')` **underflowt `size_t`** bei > 80 Byte → OOM/Crash),
      danach auf 80 gepolstert; nicht mit `"solid"` beginnen (ASCII-STL-Heuristik). Das STL-Decode-Orakel liest nur
      den uint32-Zähler @ Offset 80 → header-string byte-golden-sicher. DXF optional (Header-Kommentar/TEXT —
      [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) entscheidet Umfang).
- [ ] **Determinismus + Golden:** `goldenModel()`/`golden_gen` übergeben eine **feste** `ExportProvenance` →
      `make golden-regen` re-baseliniert **alle** betroffenen Golden; `GoldenExport.*` + `make golden-check` grün.
      Negative-Determinismus-Sonde umgekehrt: der **feste** Wert **muss** erscheinen (kein Wall-Clock-Leak).

### Tests / Doku
- [ ] **AK je Format:** der Provenance-String (fester Testwert) erscheint **sichtbar** (PDF-Text-Sonde; PNG-Ink/
      Decode-Sonde) **und** in Metadaten/Header; zwei Läufe mit **gleicher** Provenance byte-identisch, mit
      **anderer** Provenance **verschieden** (Unterscheidbarkeits-AK — der Kern-Produktwert).
- [ ] **Spec-Notiz** `spec/spezifikation.md`: Export-Provenance ist **injiziert** (Datum/Quelle/Version), sichtbar
      in PDF/PNG + in Format-Headern; im Test fixiert (lösungsfrei, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).
      **Lastenheft** ggf. schärfen ([LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/[008](../../../../spec/lastenheft.md#lh-fa-io-008)):
      „Export trägt erkennbare Herkunft" ist ein **benutzer-beobachtbares** AK — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)/[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) prüfen die Formulierung.
- [ ] [CHANGELOG](../../../../CHANGELOG.md) + `tests/adapters/golden/README.md` (Provenance-Determinismus-Muster).

## 4. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/model/export_provenance.{h,cpp}` | neu (geplant) | Wert-Struct (date/source/version), framework-frei |
| `src/hexagon/ports/driving/exchange_model_port.h`, `.../driven/model_exporter_port.h` | ändern | Signatur um `provenance` weiten |
| `src/hexagon/services/exchange_service.{h,cpp}` | ändern | durchreichen |
| `src/main.cpp` | ändern | echte Werte füllen (Uhr, Pfad-Basename, `bcad_version()`) |
| `src/hexagon/services/bootstrap_info.{h,cpp}` | ändern | `bcad_version()`-Accessor (liest `BCAD_VERSION`; MED-1) |
| `src/adapters/io/pdf_writer.{h,cpp}`, `src/adapters/io/pdf_export_adapter.cpp` | ändern | sichtbarer Titelblock + `/Info`+`/CreationDate` |
| `src/adapters/io/png_writer.{h,cpp}`, `src/adapters/io/png_export_adapter.cpp` | ändern | **Bitmap-Font (046b)** + sichtbar + `tEXt` |
| `src/adapters/geometry/step_export_adapter.cpp`, `src/adapters/io/ifc_export_adapter.cpp` | ändern | `FILE_NAME` echtes Datum/Version (revidiert 044a) |
| `src/adapters/geometry/stl_export_adapter.cpp` | ändern | 80-Byte-Header trägt Provenance |
| `tests/adapters/golden_model.{h,cpp}`, `golden_gen.cpp`, `test_*_export.cpp` | ändern | feste Provenance + Unterscheidbarkeits-AK |
| `tests/adapters/golden/model.*` | neu erzeugt | re-baseliniert (feste Provenance) |
| `spec/spezifikation.md`, `CHANGELOG.md` | ändern | Doku |

## 5. Determinismus-Auflösung (der Prüfstein)

| | vorher (044a/045) | slice-046 |
|---|---|---|
| Datum | weggelassen/Epoch-Sentinel | **injiziert**: Produktion echt, Golden fix |
| Version/Quelle | `b-cad` fix / keine | **injiziert** |
| Byte-Golden | deterministisch (weil leer) | deterministisch (weil **fixe Input-Provenance**) |
| Unterscheidbarkeit | **nein** (Defekt) | **ja** (verschiedene Provenance → verschiedene Datei) |

## 6. Verhältnis zu 044a / 045

- **slice-045 (statische Metadaten):** wird **aufgelöst/absorbiert** — dieselbe `/Info`/`tEXt`-Mechanik, aber mit
  **injizierten** statt statischen Werten (+ sichtbar). slice-045 bleibt **uncommittet**; seine Diffs gehen in 046a
  auf (kein separater Commit nötig — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) bestätigt die Faltung).
- **slice-044a (STEP-Header, committet `82b5951`):** der Sentinel-Header wird auf **injizierte** Werte umgestellt
  (echtes Datum/Version in Produktion, fix im Golden). Der Byte-Golden + die Struktur-Orakel bleiben tragfähig.

## 7. Risiken

- **R1 — PNG-Bitmap-Font (046b):** neuer Rasterizer = der größte Posten; Umfang klein halten (ein fester 5×7-Font,
  nur ASCII, nur die Titelzeile). Ink-/Decode-Sonde als Netz.
- **R2 — quergreifende Signatur-Änderung:** `exportModel`/`write` um `provenance` zu weiten trifft **alle**
  Export-Adapter + Tests + Composition-Root gleichzeitig (build/a-check-Fallout). Sorgfältig, in einem Zug.
- **R3 — Pfad-Leak:** nur **Basename** (Projektinhaber-Wahl), nie der volle Pfad in geteilte Dokumente.
- **R4 — in-memory-Fallback:** ohne geladene Datei kein Pfad → `(ungespeichert)`; die [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Demo/`io-smoke`
  müssen einen definierten Wert liefern (kein leerer/undefinierter String im Golden).
- **R5 — Determinismus-Regression:** ein versehentlicher Uhr-Aufruf im Writer bräche das Golden **und** die
  Reinheit → die Negative-Sonde (fester Wert erscheint, kein Wall-Clock-Leak) fängt es fail-closed.

## 8. Trigger

- Projektinhaber-Fund (2026-07-24): Exporte sind nicht unterscheidbar; Herkunft (Datum/Quelle/Version) muss
  **sichtbar** sein (PDF **und** PNG).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  Geometrie-/Encoder-nah → **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review** vor Welle-Closure empfohlen.
