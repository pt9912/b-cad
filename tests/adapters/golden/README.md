# Golden files — Austauschformate (slice-044a)

Byte-genaue Referenz-Ausgaben eines **festen** Test-Modells (`goldenModel()`,
`tests/adapters/golden_model.{h,cpp}`) je Austauschformat. Sie fangen
**Encoder-Drift bei unveränderter Semantik** — die Fehlerklasse, die die
vorhandenen Decode-/Struktur-Orakel (in `tests/adapters/test_*_export.cpp`)
tolerieren. Die beiden Netze sind **komplementär**: Golden = Byte-Drift,
Decode-Orakel = Semantik. Golden ersetzt die Orakel **nicht**.

## Dateien

| Datei | Format | Klasse | Determinismus |
|---|---|---|---|
| `model.ifc` | IFC (SPF) | Text | byte-exakt (Sentinel-Zeitstempel, seq-IDs) |
| `model.dxf` | DXF R12 | Text | byte-exakt (nur `$ACADVER`, keine Handles/Datum) |
| `model.step` | STEP AP214 | Text | byte-exakt **seit slice-044a** (HEADER auf Sentinel fixiert; s. u.) |
| `model.stl` | STL | binär | byte-exakt **nur im gepinnten OCC-Image** (s. Caveat) |
| `model.pdf` | PDF | binär | byte-exakt (self-rolled; statisches `/Info` [`/Producer`/`/Title`], **kein** `/CreationDate`/`/ID`) |
| `model.png` | PNG | binär | byte-exakt (self-rolled; **sichtbarer Provenance-Titelblock** [5×7-Font, 046b] + `tEXt` [`Software`/`Title` statisch, `Date`/`Source`/`Version` injiziert], **kein** `tIME`) |

Die binären Golden (`*.stl`/`*.png`/`*.pdf`) sind in `.gitattributes` als
`binary` markiert (kein CRLF-Mangling, kein Text-Diff).

## Workflow

- **`make golden-regen`** — erzeugt alle sechs Golden neu (dedizierter
  `golden_gen`, dieselbe `goldenModel()`-TU wie der Test, über einen writable
  Mount). Nach jeder bewussten Encoder-/Modell-Änderung aufrufen, dann den Diff
  reviewen und committen.
- **`make golden-check`** — Drift-Gate (CI, **nicht** in `make gates`):
  regeneriert nach `/tmp` und difft byte-genau gegen die committeten Golden.
- Der eigentliche **Byte-Vergleich** läuft als GoogleTest (`GoldenExport.*` in
  `test_golden_export.cpp`) **in `make test`/`make gates`** — er liest die
  committeten Golden und braucht `golden-regen` nicht.

## Export-Herkunft (slice-046)

Die Golden tragen eine **feste** `goldenProvenance()` (Datum `1970-01-01 00:00`, Quelle
`golden.bcad`, Version `b-cad test`) — injiziert über den Export-Port. `golden_gen` und der
Byte-Test linken dieselbe Quelle. **Sichtbar** in `model.pdf` (Fußzeile) und `model.png`
(Titelblock, 5×7-Font, 046b); eingebettet in `model.step` (`FILE_NAME`), `model.stl`
(80-Byte-Header) und `model.png`-`tEXt`. In Produktion füllt der
Composition-Root echte Werte (Systemuhr/Version); im Golden bleibt sie **fix** →
byte-deterministisch (SOURCE_DATE_EPOCH-Muster). Ein Export **ohne** Herkunft (leer) fällt
auf die deterministischen Sentinels zurück.

## STEP-HEADER (slice-044a)

Der OCC-`STEPControl_Writer` brennt normalerweise die Wall-Clock und die
OCC-Version in den ISO-10303-21-HEADER (`FILE_NAME`/`FILE_DESCRIPTION`). Seit
slice-044a fixiert `step_export_adapter.cpp` den HEADER auf feste Sentinel-Werte
(Epoch-Zeitstempel, `b-cad` statt Versions-String) — analog dem
`ifc_spf_writer`-Sentinel. Der Export wird damit byte-reproduzierbar; die
**DATA-Section** (B-Rep-Topologie) bleibt OCC-versions-abhängig.

## STL-/STEP-Caveat (OCC-versions-gebunden, ADR-0004)

`model.stl` (float32-Tessellation, `BRepMesh`) und die **STEP-DATA-Topologie**
hängen an der OCC-Version des gepinnten Build-Images. Ein bewusstes OCC-Upgrade
ändert die Tessellation/Topologie → `make golden-check` und die
`GoldenExport.Stl*`/`Step*`-Tests werden **absichtlich rot** (Signal). Dann:
`make golden-regen` + Diff-Review; die Review entscheidet, ob das STL-Golden
byte-golden bleibt oder auf ein toleranteres Orakel (Dreieckszahl + Extent,
bereits in `test_step_stl_export.cpp`) zurückfällt.

## Fremd-Import-Golden (slice-044b, folgt)

Die **fremd-erzeugten** Import-Golden (IFC/DXF anderer Tools) + ihr
`foreign/PROVENANCE.md` (Quelle/Lizenz/Attribution/`sha256`) sind Gegenstand von
**slice-044b** und liegen dann unter `foreign/`.
