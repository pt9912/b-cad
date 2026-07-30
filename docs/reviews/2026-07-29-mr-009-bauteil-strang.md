# MR-009-Code-Review Bauteil-Strang im interaktiven Pfad (vor der welle-6-Closure)

**Datum:** 2026-07-29 (Lauf 2026-07-30).

**Review-Art:** Geometrielastiges **Code-Review vor der Welle-Closure**
([MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) —
**kein** Plan-Review ([MR-006](../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)).
**HIGH-Findings blockieren die Closure der Welle**, nicht einen Slice.

**Rolle:** unabhängiger Reviewer ≠ Autor, getrennte Session ohne Autoren-Kontext.
**Modell:** Claude Opus 5 (1M).

**Prüfgegenstand:** der Bauteil-Strang im interaktiven Pfad —
[`slice-057`](../plan/planning/done/slice-057-lese-naht-bauteil-identitaet.md) (Bauteil-Identität in der
2D-Lese-Naht) · [`slice-058`](../plan/planning/done/slice-058-wand-zeichnen-im-canvas.md) (Wand zeichnen,
Geste → `addWall`) · [`slice-059a`](../plan/planning/done/slice-059a-wand-auswaehlen.md) (Wand auswählen) ·
[`slice-059b`](../plan/planning/done/slice-059b-wand-parameter-aendern.md) (Stärke/Höhe ändern).
[ADR-0021](../plan/adr/0021-wand-im-2d-canvas.md) §Konsequenzen erklärt
[MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) für diesen
Strang ausdrücklich für einschlägig.

**Nicht Gegenstand:** die Öffnungs-/Dach-/Treppen-/Platten-Geometrie, die Export-Codecs, die Persistenz.
Die Wand-Geometrie selbst ist Bestand aus welle-1/2 — geprüft wurde sie auf den **Wegen**, die dieser
Strang neu erreichbar gemacht hat.

**Eingangs-Kontext:** [`harness/README.md`](../../harness/README.md) · [`AGENTS.md`](../../AGENTS.md) ·
[`harness/conventions.md`](../../harness/conventions.md) (MR-009 vollständig) ·
[`spec/lastenheft.md`](../../spec/lastenheft.md) (LH-FA-WAL-001/002/003/006, LH-FA-ROM-001, LH-FA-D3-002,
LH-FA-DRW-001) · [`spec/spezifikation.md`](../../spec/spezifikation.md) (§1 WAL-006.a, §1 ROM-001.a,
§1 D3-002.a, §3 Wertebereiche) · die vier Slice-Pläne inkl. ihrer Closure-Notizen ·
[ADR-0019](../plan/adr/0019-drw-2d-canvas.md)/[ADR-0021](../plan/adr/0021-wand-im-2d-canvas.md) ·
`src/hexagon/services/geometry/wall_footprint.cpp` · `src/hexagon/services/structure_edit_service.cpp` ·
`src/hexagon/services/room_detection.cpp` · `src/adapters/geometry/occ_geometry_adapter.cpp` ·
`src/adapters/geometry/occ_solids.cpp` · `src/adapters/ui/view/canvas_widget.cpp` ·
`src/adapters/ui/view/snap.cpp` · `src/adapters/ui/view/pick.cpp` ·
`src/adapters/ui/command/edit_structure_wall_sink.h` · `src/adapters/ui/command/wall_param_sink.h` ·
die Geometrie- und Interaktions-Orakel unter `tests/`.

**Eigene Sensor-Läufe (nur `make`, [`AGENTS.md`](../../AGENTS.md) §2.9):**

- `make test` auf dem unveränderten Stand: **EXIT=0, 414/414 Tests** (Basislinie).
- Vier Runden **temporärer Mess-Sonden** (`tests/adapters/test_mr009_probe.cpp`, in
  `tests/CMakeLists.txt` eingehängt), je über `make test` ausgeführt, Messwerte über `ADD_FAILURE()`
  sichtbar gemacht. **Vollständig zurückgenommen** (Datei gelöscht, `tests/CMakeLists.txt` auf den
  gemerkten Inhalt zurückgeschrieben — **nicht** über `git checkout`).
- `make docs-check` auf diesem Report (Ergebnis am Ende).

---

## Zählung

**2 HIGH · 3 MEDIUM · 2 LOW · 2 INFO · 11 Negativbefunde**

| Nr. | Kategorie | Kurzfassung |
|---|---|---|
| HIGH-1 | HIGH | Wand kürzer als der Eck-Sporn ⇒ **selbstschneidender Footprint** ⇒ nicht-manifolder Körper; über die Geste **und** über `setWallThickness` an der **Nachbar**wand erreichbar |
| HIGH-2 | HIGH | Boundary-Zeile „ungleiche Wandhöhen" aus [LH-FA-WAL-006](../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) ist **nicht umgesetzt** — der Footprint ist höhen-blind; mit [`slice-059b`](../plan/planning/done/slice-059b-wand-parameter-aendern.md) erstmals über die Oberfläche erreichbar |
| MEDIUM-1 | MEDIUM | Kein einziges Orakel prüft eine **geometrische Invariante** an einem Wand-Netz (weder Geschlossenheit noch Orientierung) — die von der Regel benannte Lücke, wörtlich |
| MEDIUM-2 | MEDIUM | Das WAL-006-Orakel des interaktiven Pfads prüft nur **Koordinaten-Gleichheit**, nie die Ecke; seine Endlage ist zudem ein **Grad-3-Knoten**, an dem gar keine Ecke entsteht |
| MEDIUM-3 | MEDIUM | `E-GEO-002` greift bei einem selbstschneidenden Footprint nicht: die Degenerations-Prüfung ist **flächen-only** — genau die Prüfung, die die Spezifikation an anderer Stelle ausdrücklich als unzureichend benennt |
| LOW-1 | LOW | Drei Verbraucher melden für denselben kaputten Körper **drei verschiedene Volumina**; keiner meldet einen Fehler |
| LOW-2 | LOW | Zwei deckungsgleiche Wände sind ohne Hinweis anlegbar; Volumen zählt doppelt |
| INFO-1 | INFO | Der Fang wirkt über **alle** Geschosse — spec-konform, aber ein gefangener Punkt auf einem fremden Geschoss erzeugt **keine** Ecke |
| INFO-2 | INFO | `std::abs` auf dem OCC-Volumen verdeckt ein negatives Vorzeichen; hier folgenlos, aber es nimmt einem möglichen Orientierungs-Fehler den Sensor |

---

## HIGH

### HIGH-1 — Eine Wand, die kürzer ist als der Eck-Sporn ihres Nachbarn, wird zu einem selbstschneidenden, nicht-manifolden Körper — und das Modell nimmt sie an

- **kategorie:** HIGH
- **quelle:** [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  (Orientierung/Winding, Totalität bei Degeneration),
  [LH-FA-WAL-001](../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) („Negative (Ablehnung durch das
  Modell): … dann bleibt das Modell **vollständig unverändert**"),
  [LH-FA-WAL-002](../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) (50–1000 mm sind
  **zulässige** Werte), [`E-GEO-002`](../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
- **pfad:** `src/hexagon/services/geometry/wall_footprint.cpp`:84–118 (`cornerAt`, die Begrenzung prüft
  **nur** den Abstand zum gemeinsamen Endpunkt) · `src/adapters/geometry/occ_solids.cpp`:47–59
  (`isDegenerate`) · `src/hexagon/services/structure_edit_service.cpp`:414–436
  (`rebuildAffectedNeighbors` committet den Nachbar-Körper)
- **befund:**

  Die Eck-Konstruktion setzt die Seitenkante der Wand um die **halbe Stärke des Nachbarn** zurück
  (`wall_footprint.cpp`:103–106). `WALL_MITER_LIMIT` begrenzt nur, wie weit ein Eckpunkt **über den
  gemeinsamen Endpunkt hinaus** ragt — **nicht**, wie weit er in die Wand **hinein** greift. Ist die Wand
  kürzer als dieser Rücksprung, wandert der Eckpunkt **hinter das andere Wandende** und das
  Footprint-Polygon wird zur Schleife (Bowtie).

  **Gemessen** (Sonde, rechter Winkel, Stärken 240 mm):

  | Fall | Schwelle | Beispiel-Footprint |
  |---|---|---|
  | **ein** gemitertes Eck | Länge < Nachbar-Stärke/2 = **120 mm** | `len=119` ⇒ `(0,120) (-1,120) (239,-120) (0,-120)` — selbstschneidend |
  | **zwei** gemiterte Ecken (Nachbarn auf derselben Seite) | Länge < Nachbar-Stärke = **240 mm** | `len=239` ⇒ `(120,120) (119,120) (359,-120) (-120,-120)` — selbstschneidend |

  Bei `len=240` bzw. `len=120` liegt die Schwelle exakt (doppelter Punkt, noch unauffällig), ein
  Zehntel darunter schlägt es um. Mit der zulässigen Höchststärke 1000 mm liegt die Schwelle bei
  **500 mm** (ein Eck) bzw. **1000 mm** (zwei Ecken).

  **Was ausgeliefert wird** (Sonde über `StructureEditService` + `OccGeometryAdapter`, Wand 100 mm,
  ein gemitertes Eck):

  ```
  selbstschneidend=JA | OCC-Solid.volume = 60 000 000 | Netz-signVol = 40 000 000
  | Differenz = 20 000 000 (33 %) | kanten-manifold = NEIN (8 schlechte Kanten)
  | tris = 8 (statt 12) | analytisches EVL-Volumen (|shoelace|·h) = 60 000 000
  ```

  Das an die 3D-Sicht und an den STEP-/STL-Export gereichte Netz ist **nicht kanten-manifold**:
  **8 gerichtete Kanten** ohne korrekte Gegenkante (gesunde Wand: **0**) — der Körper ist **offen**, seine beiden
  Schleifen-Lappen sind **gegenläufig orientiert**. Die Normalensumme bleibt dabei 0, weil sich die
  Lappen gegenseitig auslöschen: **eine reine Divergenz-Sonde fängt diesen Fall nicht** (das ist auch
  der Grund, warum MEDIUM-1 die Kanten-Manifold-Prüfung als Ergänzung nennt).

  **Erreichbarkeit — auf beiden neuen Wegen gemessen, nicht abgeleitet:**

  1. **Über die Geste** ([`slice-058`](../plan/planning/done/slice-058-wand-zeichnen-im-canvas.md)).
     Sonde mit dem echten `QMouseEvent`-Pfad: 40 Mausrad-Schritte (Zoom 0,09 → **24,1 px/mm**, der
     Fang-Radius von 12 px entspricht dort **0,50 mm**), dann Druck auf einen gefangenen Endpunkt und
     Loslassen 30 px weiter:

     ```
     gezogene Wand: (4000,3000) -> (4001.226,3000.017), Länge = 1.226 mm, Stärke 240
     fp = (3880,3118.30) (3999.52,3120.01) (4002.94,2880.03) (4120,2881.70)
     selbstschneidend = JA | OCC-vol = 735 814 | Netz-signVol = 490 543
     | manifold = NEIN (8) | tris = 8
     Ausgang, den der Benutzer erhält: Created
     ```

     Der Benutzer bekommt **„angelegt"** gemeldet, nicht den Hinweis, den
     [LH-FA-WAL-001](../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) für den Fall vorsieht,
     dass die Wand nicht gebildet werden kann.

  2. **Über die Parameter-Änderung** ([`slice-059b`](../plan/planning/done/slice-059b-wand-parameter-aendern.md)) —
     und hier trifft es eine Wand, die der Benutzer **gar nicht angefasst hat**. Ausgangslage: drei
     gesunde Wände (4000 mm, **300 mm**, 1000 mm), alle `manifold=ja`, alle Volumina stimmig. Dann
     `setWallThickness(lange Wand, 1000)` — ein **zulässiger** Wert nach
     [LH-FA-WAL-002](../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren):

     ```
     status = Accepted, applied = 1000, wirft = nein
     Wand 1 (geändert)      th=1000  manifold=ja
     Wand 2 (NICHT geändert) th=240  selbstschneidend=JA
                             fp = (3880,500) (3880,180) (4120,420) (4120,-500)
                             OCC-vol = 180 000 000  vs.  Netz-signVol = 120 000 000
                             manifold = NEIN (8 schlechte Kanten), tris = 8
     ```

     `rebuildAffectedNeighbors` (`structure_edit_service.cpp`:414–436) baut den kaputten Nachbar-Körper
     **absichtlich**, `commitNeighborRebuilds` schreibt ihn ins Modell und
     `notifyNeighborRebuilds` meldet ihn als `WallGeometryChanged` an die 3D-Sicht. Es wirft nichts,
     es wird nichts gemeldet, nichts zurückgerollt.

- **warum es blockiert:** Ein committeter Wand-Körper ist **offen und in sich gegenläufig orientiert** —
  genau die zwei Eigenschaften, die
  [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) als
  Blocker benennt. Die Zusage „Modell bleibt vollständig unverändert, wenn die Wand nicht gebildet
  werden kann" ist verletzt: die Wand **wird** gebildet, nur eben kaputt. Und der zweite Weg verletzt
  sie in der schärferen Form — eine legale Parameter-Änderung korrumpiert ein **anderes** Bauteil.
- **verifizierbar:** ja. Reproduktion ohne Qt: zwei Wände `(0,0)-(4000,0)` und `(4000,0)-(4000,300)`
  über `StructureEditService::addWall`, danach `setWallThickness` der ersten auf 1000; dann
  `wallFootprint` der zweiten auf Selbstschnitt und `wallMesh` auf Kanten-Manifold prüfen.
- **Fix-Richtung (nicht Teil des Befunds):** Die Spezifikation nennt das passende Kriterium bereits —
  für den Raum-Offset, `spec/spezifikation.md`:60–64: „Kollaps-Kriterium ist der
  **Kantenrichtungs-Erhalt**: kehrt sich beim Offset die Richtung einer Kante um, ist der Ring kein
  gültiger … (eine reine Flächen-Prüfung „Netto-Fläche ≤ 0" genügt nicht)". Dieselbe Prüfung diskriminiert
  hier **exakt**: eine Sonde, die die Kantenrichtungen des gemiterten Footprints gegen die des stumpfen
  vergleicht, meldet „umgekehrt" **genau** in den selbstschneidenden Fällen und in keinem anderen
  (`len` 600/241/240 ⇒ erhalten; 239/121/120/119/100 ⇒ umgekehrt, jeweils passend zur Eck-Anzahl).
  Der Rückfall auf **stumpf** steht in `cornerAt` bereits als etablierter Weg zur Verfügung.

### HIGH-2 — Die Boundary-Zeile „ungleiche Wandhöhen" aus LH-FA-WAL-006 ist nicht umgesetzt: der Footprint ist höhen-blind

- **kategorie:** HIGH
- **quelle:** [LH-FA-WAL-006](../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), Boundary:
  „Given ungleiche Wandhöhen, then ist die Ecke bis zur niedrigeren Wandhöhe geschlossen, **darüber endet
  die höhere Wand stumpf**." · [LH-FA-WAL-003](../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren)
- **pfad:** `src/hexagon/services/geometry/wall_footprint.cpp`:84–118 (`cornerAt` kennt `height_mm`
  nicht — der einzige Höhen-Zugriff im Kern ist die Extrusion,
  `src/hexagon/services/structure_edit_service.cpp`:512–517) · `spec/spezifikation.md`:71–93
  (der WAL-006.a-Block schreibt die Eck-Konstruktion **ohne** Höhen-Fallunterscheidung aus) ·
  `spec/spezifikation.md`:368–370 („Höhen-Änderungen lassen Footprints unberührt")
- **befund:**

  Der Eckenschluss ist eine reine 2D-Footprint-Regel; der Adapter extrudiert das Polygon über die
  **volle** Wandhöhe (`occ_solids.cpp`:63–77). Treffen zwei Wände ungleicher Höhe in einer Ecke,
  trägt die höhere Wand ihren Eck-Sporn deshalb **bis zu ihrer eigenen Oberkante** — dort, wo die
  niedrigere Wand längst zu Ende ist und die Ecke gar nichts mehr zu schließen hat.

  **Gemessen** über den [`slice-059b`](../plan/planning/done/slice-059b-wand-parameter-aendern.md)-Weg
  (zwei Wände im rechten Winkel, beide h = 2500; dann `setWallHeight(A, 4000)`):

  ```
  A.h = 4000, B.h = 2500
  A gemitert = (0,120) (3880,120) (4120,-120) (0,-120)
  A stumpf   = (0,120) (4000,120) (4000,-120) (0,-120)
  Abtastung im Eck-Kasten 240×240 mm, 61×61 = 3721 Punkte,
  gültig für JEDE Ebene z ∈ (2500, 4000] — dort existiert NUR A:
     Sporn  (A trägt, das stumpfe Ende trägt nicht) = 465 Punkte
     Kerbe  (das stumpfe Ende trägt, A trägt nicht) = 465 Punkte
  ```

  Oberhalb der niedrigeren Wand steht also ein **freitragender Sporn** über dem Nichts, und daneben
  klafft eine gleich große **Kerbe** — statt des zugesagten stumpfen Endes. Beides ist über die volle
  Höhendifferenz von 1500 mm vorhanden.

  **Warum kein Bestands-Orakel das sieht:** Der Eckschnitt ist **flächen-erhaltend**. Das Solid-Volumen
  von A beträgt 3 840 000 000 mm³ — **exakt** so viel wie das stumpfe Ende (4000 · 240 · 4000). Volumen-,
  Bounding-Box- und Dreiecks-Anzahl-Orakel sind gegen diesen Fehler **blind**; er ist ausschließlich an
  der Form sichtbar. Das ist die Begründung von
  [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) im
  Wortlaut.

  **Warum es jetzt zählt.** Innerhalb eines Geschosses waren bisher **alle** Wandhöhen gleich:
  `addWall` setzt `wall.height_mm = sit->height_mm` (`structure_edit_service.cpp`:161), und
  `setWallHeight` hatte **keine** Oberflächen-Anbindung. Erst
  [`slice-059b`](../plan/planning/done/slice-059b-wand-parameter-aendern.md) macht die Höhe einer
  **einzelnen** Wand über den Eigenschaften-Bereich änderbar — der Boundary-Fall entsteht damit erstmals
  im normalen Bedienfluss, mit zwei Tastendrücken.

  In `tests/` existiert **kein** Orakel für diese Boundary-Zeile: `tests/hexagon/test_wall_footprint.cpp`
  deckt Happy/kollinear/Spitzwinkel/Grad-3/fremdes Geschoss ab (:97–212), die Höhe kommt dort nur als
  Melde-Frage vor (:162–186, „Höhen-Änderung meldet keinen Nachbarn"), nie als Geometrie-Frage.
- **warum es blockiert:** Ein ausdrücklicher Boundary-Fall einer Akzeptanzkriterien-Zeile des
  **abnahmebindenden** Lastenhefts (Source Precedence #1) ist falsch umgesetzt und ohne Sensor. Die
  Spezifikation (#2) schreibt an dieser Stelle keine abweichende Regel aus — sie **schweigt** zur Höhe;
  ein Schweigen hebt eine Lastenheft-Zusage nicht auf. Nach der Einstufung des Auftrags („ein
  Boundary-Fall der AK ist falsch umgesetzt") ist das HIGH.
- **verifizierbar:** ja — `wallFootprint` zweier Wände unterschiedlicher `height_mm` liefert dasselbe
  Polygon wie bei gleicher Höhe; `grep -n "height_mm" src/hexagon/services/geometry/wall_footprint.cpp`
  ist leer.
- **Anmerkung zur Zuordnung:** Die Ursache liegt im Kern-Footprint (Erbe von slice-012, welle-1), nicht
  im Diff der vier Slices. Der Befund blockiert trotzdem **diese** Closure — MR-009 prüft den Strang
  gegen die Spezifikation, und der Strang hat den Fall erst erreichbar gemacht. Zwei ehrliche
  Auflösungswege stehen offen: den Fall implementieren (höhen-geschichteter Footprint) **oder** die
  AK-Zeile als Reifephase-Teilumfang explizit zurücknehmen (Lastenheft-Schärfung nach
  [MR-006](../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Muster,
  mit Historien-Zeile). Was nicht geht, ist die Zeile stehen zu lassen und nichts dazu zu bauen.

---

## MEDIUM

### MEDIUM-1 — Kein Orakel des Repos prüft eine geometrische Invariante an einem Wand-Netz

- **kategorie:** MEDIUM
- **quelle:** [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  („die AK-Tests des Slice sollen **geometrische Invarianten** sondieren … nicht nur
  Bounding-Box/Dreiecks-Anzahl")
- **pfad:** `tests/adapters/test_occ_geometry_adapter.cpp`:49–182 (nur Volumen-Vergleiche) ·
  `tests/hexagon/test_stair_geometry.cpp`:201–228 und `tests/hexagon/test_roof_geometry.cpp`:103–118
  (dort existieren die Sonden — für **Treppe** und **Dach**) ·
  `tests/adapters/test_step_stl_export.cpp`:326–361 (`CLOSED_SHELL`-Zählung, aber für Wände nur
  als Basislinie „> 0")
- **befund:** Die Divergenzsatz-Sonde und die Wasserdichtheits-Sonde, die
  [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) als
  Beispiel nennt, existieren im Repo **zweimal** — für `stairMesh` und `roofMesh`, beide vom Kern
  gerechnet. Für **Wand**-Netze, die über `GeometryKernelPort` aus OCC kommen, existiert keine.
  Ein `grep` über `tests/` nach „Divergenz", „manifold", „watertight", „Wasserdicht" trifft
  **ausschließlich** `test_roof_geometry.cpp` (12 Stellen), `test_stair_geometry.cpp` (1) und zwei
  Kommentare in `test_step_stl_export.cpp`. Die Wand-Orakel prüfen Volumen (`test_occ_geometry_adapter.cpp`) und
  Footprint-Punkte (`test_wall_footprint.cpp`) — **kein** einziger prüft, ob der ausgelieferte Körper
  geschlossen und konsistent orientiert ist.
- **wirkung, gemessen:** HIGH-1 wäre von einer solchen Sonde gefangen worden. Die **Kanten-Manifold**-Sonde
  meldet für den kaputten Fall `8 schlechte Kanten` (gesund: 0) und für den gesunden Fall 12 Dreiecke
  gegen 8. Die reine **Divergenz-Sonde allein hätte ihn nicht gefangen** — die Normalensumme bleibt 0,
  weil sich die zwei gegenläufigen Lappen auslöschen. Wer die Regel umsetzt, braucht **beide**:
  Normalensumme ≈ 0 **und** signiertes Netz-Volumen == `Solid.volume_mm3` (die Differenz betrug im
  kaputten Fall 33 %).
- **warum es nicht blockiert:** Es ist eine fehlende Sonde, kein falscher Körper — der falsche Körper ist
  HIGH-1. Nach Behebung von HIGH-1 ist diese Sonde der Regressionsschutz und gehört mit in denselben Fix.

### MEDIUM-2 — Das WAL-006-Orakel des interaktiven Pfads prüft Koordinaten, nicht die Ecke — und endet auf einem Grad-3-Knoten

- **kategorie:** MEDIUM
- **quelle:** [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure),
  [LH-FA-WAL-006](../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden),
  [LH-FA-DRW-001](../../spec/lastenheft.md#lh-fa-drw-001) („beim Bauteil-Zeichnen ist es die Voraussetzung
  dafür, dass zwei Wände … sich zu einer geschlossenen Ecke verbinden")
- **pfad:** `tests/adapters/test_canvas_widget.cpp`:686–716
  (`CanvasWallTool.LH_FA_WAL_006_ZweiZuegeTeilenDenGefangenenPunktExakt`), Fixture :277–318
- **befund:** Der einzige Test des Strangs, der
  [LH-FA-WAL-006](../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) im Namen trägt, prüft
  ausschließlich vier `EXPECT_DOUBLE_EQ` auf Koordinaten (`start_of_second == end_of_first == (4000,3000)`).
  Er prüft die **Voraussetzung** des Eckenschlusses, nie den Eckenschluss. Das ist im Slice-Plan auch so
  benannt — die Lücke ist trotzdem eine.

  Verschärfend: die Fixture legt bereits zwei Wände an, die zweite endet auf **(4000,3000)**
  (`test_canvas_widget.cpp`:279–280). Zug 1 macht daraus einen Grad-2-Knoten, Zug 2 einen **Grad-3-Knoten**.
  Nach der Abgrenzung von [LH-FA-WAL-006](../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)
  („drei oder mehr Wände am selben Punkt … bleiben die Enden unverändert stumpf") entsteht in der
  Endlage dieses Tests **gar keine geschlossene Ecke**. Ein Orakel, das den Eckenschluss dort prüfen
  wollte, müsste erst die Lage ändern.
- **warum es nicht blockiert:** Der Eckenschluss selbst ist auf allen vier Endpunkt-Kombinationen korrekt
  (Negativbefund N-2/N-3) — die Zusage stimmt, nur ihr Sensor im interaktiven Pfad nicht.

### MEDIUM-3 — Die Degenerations-Prüfung vor der Extrusion ist flächen-only und kann einen Selbstschnitt strukturell nicht sehen

- **kategorie:** MEDIUM
- **quelle:** [`E-GEO-002`](../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
  `spec/spezifikation.md`:60–64 (das Kriterium steht dort schon, für den Raum-Offset),
  `spec/spezifikation.md`:86–90 („Begrenzung und Rückfälle (total, wirft nie)")
- **pfad:** `src/adapters/geometry/occ_solids.cpp`:47–59 (`isDegenerate`: Punktzahl, Endlichkeit,
  `polygonArea < GEOMETRY_TOLERANCE_MM²`)
- **befund:** `isDegenerate` nutzt die **vorzeichenlose Shoelace-Fläche**. Für die Bowtie-Polygone aus
  HIGH-1 liefert sie einen völlig unauffälligen Wert — bei der 100-mm-Wand exakt `24 000 mm²`, also
  denselben Wert wie das stumpfe Rechteck. Die Prüfung ist damit strukturell blind gegen genau die
  Klasse, die hier auftritt. Die Spezifikation hat diese Erkenntnis bereits — `spezifikation.md`:60–64
  begründet für die Raumerkennung, warum eine reine Flächen-Prüfung nicht genügt und der
  **Kantenrichtungs-Erhalt** das richtige Kriterium ist. Der Wand-Footprint hat die Lehre nicht
  mitbekommen.
- **warum es nicht blockiert:** eigenständig gemessen ist es die **Ursache** von HIGH-1 auf der
  Adapter-Seite; behoben wird es mit HIGH-1. Es steht getrennt, weil der saubere Ort für den Riegel
  strittig sein kann (Kern-Footprint mit Rückfall auf stumpf vs. `E-GEO-002` im Adapter) — die
  Spezifikation (`spezifikation.md`:86–90) verlangt für den Eckenschluss ausdrücklich **Totalität**
  („wirft nie"), was für den Rückfall auf stumpf **im Kern** spricht.

---

## LOW

### LOW-1 — Drei Verbraucher, drei verschiedene Volumina für denselben Körper

- **kategorie:** LOW
- **pfad:** `src/adapters/geometry/occ_geometry_adapter.cpp`:64–79 (`Solid.volume_mm3`) ·
  `src/hexagon/services/volume_geometry.cpp`:50 (analytisches EVL-Volumen über `polygonArea`) ·
  `src/adapters/geometry/occ_geometry_adapter.cpp`:81–129 (das Netz für 3D-Sicht/Export)
- **befund:** Für die kaputte 100-mm-Wand aus HIGH-1: `Solid.volume_mm3` = 60 000 000, analytisches
  EVL-Volumen = 60 000 000, signiertes Netz-Volumen = **40 000 000**. Zwei Pfade sind sich einig und
  beide falsch, der dritte weicht um 33 % ab. Auf gesunder Geometrie stimmen alle drei **exakt** überein
  (gemessen für sechs Zug-Richtungen und für Längen ≥ Schwelle, Differenz 0). Die Übereinstimmung wäre
  ein billiger und scharfer Sensor — sie wird nirgends geprüft.

### LOW-2 — Zwei deckungsgleiche Wände sind ohne Hinweis anlegbar

- **kategorie:** LOW
- **pfad:** `src/hexagon/services/structure_edit_service.cpp`:135–185 (`addWall` prüft nur Länge und
  Endlichkeit)
- **befund:** Zweimal derselbe Zug `(0,0)-(4000,0)` erzeugt **zwei** Wände. Gemessen: beide Körper
  `vol = 2 400 000 000`, Gesamt-Wandvolumen `4,8 m³` (doppelt gezählt), Räume 0, beide Enden **stumpf**
  (der Eckenschluss fällt korrekt auf kollinear zurück — kein Geometrie-Fehler). Das Modell bleibt
  konsistent; der Benutzer bekommt aber ein doppelt gezähltes Bauteil ohne jeden Hinweis. Weder
  Lastenheft noch Spezifikation sagen zu diesem Fall etwas — deshalb LOW und nicht HIGH.

---

## INFO

### INFO-1 — Der Fang wirkt über alle Geschosse, die Ecke nur innerhalb eines

- `src/adapters/ui/view/snap.cpp`:33–53 iteriert über **alle** `plan.storeys` ohne Filter auf das aktive
  Geschoss. Das ist **spec-konform** und ausdrücklich so gewollt
  ([LH-FA-DRW-001](../../spec/lastenheft.md#lh-fa-drw-001): „gefangen wird auf die markanten Punkte
  **aller Geschosse** des Projekts"), und `spec/spezifikation.md`:1134–1137 benennt die Folge bereits
  („Fangen liefert eine Koordinate, keine Nachbarschaft"). Der Eckenschluss verlangt dagegen dasselbe
  Geschoss (`wall_footprint.cpp`:56–73). Ein Benutzer, der auf einen Punkt eines anderen Geschosses
  fängt, sieht deckungsgleiche Endpunkte und **keine** geschlossene Ecke. Kein Befund — nachgeprüft und
  in Ordnung, hier festgehalten, damit die nächste Runde nicht erneut danach sucht.

### INFO-2 — `std::abs` auf dem OCC-Volumen nimmt einer möglichen Orientierungs-Umkehr den Sensor

- `src/adapters/geometry/occ_geometry_adapter.cpp`:71 gibt `std::abs(properties.Mass())` zurück. Das
  Footprint-Polygon ist **immer im Uhrzeigersinn** orientiert (gemessen: `signedArea` negativ für alle
  sechs geprüften Zug-Richtungen, `−960 000` bzw. `−720 000` bzw. `−240 000` mm²), und OCC dreht das
  Prisma korrekt — das signierte Netz-Volumen ist in allen gesunden Fällen **positiv** und exakt
  (Negativbefund N-1). Der Betrag ist heute also folgenlos; er verdeckt aber genau das Vorzeichen, an
  dem eine künftige Winding-Umkehr auffiele.

---

## Negativbefunde (geprüft und für korrekt befunden)

Das ist die Freigabe-Information: was hier steht, ist **gemessen**, nicht plausibilisiert.

- **N-1 — Orientierung und Geschlossenheit gesunder Wandkörper (F3).** Sechs Zug-Richtungen
  (`+x`, `−x`, `+y`, `−y`, Diagonale, Gegen-Diagonale) über `addWall` + `OccGeometryAdapter`:
  signiertes Netz-Volumen jeweils **positiv und exakt gleich** `Footprint-Fläche · Höhe`
  (2 400 000 000 / 1 800 000 000 / 600 000 000 mm³), Normalensumme **0**, kanten-manifold **ja**,
  12 Dreiecke. **Die Außennormalen zeigen nach außen, der Körper ist geschlossen — richtungsunabhängig.**
  Insbesondere kehrt ein rückwärts gezogener Zug die Orientierung **nicht** um.
- **N-2 — Eckenschluss, alle vier Endpunkt-Kombinationen (F1, Happy).** Die Geste erlaubt jede
  Kombination (`A.end==B.start`, `A.end==B.end`, `A.start==B.start`, `A.start==B.end`). In **allen vier**
  enden beide Wände an **denselben zwei** Eckpunkten; Abtastung des Eck-Kastens mit 39×39 Punkten:
  **leer = 0, doppelt = 0** — kein Loch, keine Überlappung.
- **N-3 — Eckenschluss bei ungleichen Stärken (F1, Boundary).** Fünf Stärke-Paare
  (240/240, 240/500, 500/240, 1000/50, 50/1000), Eck-Kasten mit **9409** versetzten Gitterpunkten je
  Fall (Gitter bewusst irrational versetzt, damit kein Punkt auf der Miter-Diagonale liegt):
  **leer = 0, doppelt = 0** in **allen fünf** Fällen. Die Ecke ist dicht und überlappungsfrei.
- **N-4 — Kollineare Fortsetzung gleicher Stärke (F1, Boundary).** Rückfall auf stumpf, die zwei
  stumpfen Enden stoßen exakt in derselben Ebene — glatter Übergang, gemessen am Footprint
  `(0,120) (1000,120) (1000,−120) (0,−120)`.
- **N-5 — Kollineare Fortsetzung ungleicher Stärke (F1, Boundary).** 240 mm auf 115 mm: beide stumpf,
  Stoßebene `x = 1000` gemeinsam; 200 Abtastpunkte beidseits der Stoßebene über die volle Breite der
  dünneren Wand: **0 Löcher**.
- **N-6 — `WALL_MITER_LIMIT` (F1, Boundary „sehr spitzer Winkel").** Voller Winkel-Sweep 1°…179° über
  vier Stärke-Paare (240/240, 240/1000, 1000/50, 50/50). In **keinem** der 716 Fälle ragt ein Eckpunkt
  weiter als `max(Stärke_A, Stärke_B)` über den gemeinsamen Endpunkt hinaus: größter gemessener
  Überstand 240,0 / 997,3 / 986,5 / 50,0 mm gegen Limits 240 / 1000 / 1000 / 50. Der Umschlagpunkt liegt
  bei 60° (120 der 179 Winkel werden gemitert), unterhalb enden **beide** Wände stumpf — genau die Zusage.
- **N-7 — Abgrenzung Grad ≥ 3, T-Stoß, fremdes Geschoss (F1, Negative).** T-Stoß (Berührung ohne
  gemeinsamen Endpunkt) ⇒ Footprint **identisch** mit dem stumpfen. Grad-3 und fremdes Geschoss sind in
  `tests/hexagon/test_wall_footprint.cpp`:148–157 abgedeckt, der Grad-2→3-Rückbau inkl. Meldung in
  :191–212 — nachgeprüft, korrekt.
- **N-8 — Raum-Neuerkennung nach einer Stärken-Änderung (F2,
  [LH-FA-ROM-001](../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen)).** Geschlossenes
  4-Wand-Rechteck 4000 × 3000, `setWallThickness(untere Wand, 500)`: Räume bleiben **1**, Fläche geht von
  **10,3776 m²** auf **9,8888 m²** — analytisch exakt `3760 mm × 2630 mm`. **Die Innenkante wandert
  korrekt mit, die Fläche stimmt auf den Quadratmillimeter.** Die Ecke bleibt dabei geschlossen
  (Abtastung: leer = 0, doppelt = 0).
- **N-9 — Höhen-Änderung lässt Footprints und Räume unberührt (F2).** `setWallHeight` ändert den
  Footprint **nicht** (Zeichenketten-Vergleich vorher/nachher identisch) und die Raumfläche nicht
  (10,3776 m² → 10,3776 m²) — konform zu `spec/spezifikation.md`:368–370. Die
  Nicht-Nachbar-Meldung ist zusätzlich in `tests/hexagon/test_wall_footprint.cpp`:162–186 belegt.
  *(Die Höhen-**Geometrie** in der Ecke ist davon unberührt falsch — siehe HIGH-2.)*
- **N-10 — Totalität an der Toleranz-Grenze und bei sehr großen Koordinaten (F4).**
  Längen 0,101 / 0,2 / 1,0 mm (knapp über `GEOMETRY_TOLERANCE_MM` = 0,1): Wand entsteht, **kein Wurf**,
  Volumen exakt `Länge · 240 · 2500` (60 600 / 120 000 / 600 000 mm³), Netz vorhanden.
  Koordinaten-Ursprünge 0 / 1e4 / 1e6 / 1e8 / 1e9 / 1e10 / **1e12** mm: Volumen in **allen** Fällen exakt
  2 400 000 000 mm³, signiertes Netz-Volumen (schwerpunkt-bezogen gerechnet) **exakt gleich**,
  Normalensumme 0, kanten-manifold ja, 12 Dreiecke. **Der Canvas darf Positionen außerhalb der
  Bounding-Box erzeugen — die Geometrie hält das aus.** *(Eine erste Sonde meldete hier scheinbar Schrott;
  das war ein Artefakt der ursprungs-bezogenen Volumenformel der **Sonde**, nicht der Software. Der
  Befund wurde nach der Korrektur zurückgezogen — er steht hier, damit die nächste Runde ihn nicht
  wiederholt.)*
- **N-11 — Exakt an der Selbstschnitt-Schwelle (F4).** Bei `len = 120` (Nachbar-Stärke/2) fallen zwei
  Footprint-Punkte zusammen (`(0,120) (0,120) (240,−120) (0,−120)`); OCC verarbeitet den doppelten Punkt
  ohne Wurf und liefert das **korrekte** Volumen 72 000 000 mm³ (= 120 · 240 · 2500), ebenso bei
  `len = 120,0000001`. Die Schwelle selbst ist unkritisch — kaputt wird es erst **darunter** (HIGH-1).

---

## Freigabe-Verdikt

**CLOSURE BLOCKIERT.**

Zwei HIGH-Findings stehen der Buchung von M6 entgegen:

1. **HIGH-1** — auf **beiden** neu gebauten Wegen (Geste aus
   [`slice-058`](../plan/planning/done/slice-058-wand-zeichnen-im-canvas.md), Parameter-Änderung aus
   [`slice-059b`](../plan/planning/done/slice-059b-wand-parameter-aendern.md)) entsteht ein **offener,
   in sich gegenläufig orientierter** Wandkörper, ohne Wurf, ohne Meldung, ohne Rücknahme. Im zweiten
   Fall trifft es ein Bauteil, das der Benutzer nicht angefasst hat.
2. **HIGH-2** — eine Boundary-Zeile von
   [LH-FA-WAL-006](../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) ist nicht umgesetzt und
   wird durch [`slice-059b`](../plan/planning/done/slice-059b-wand-parameter-aendern.md) erstmals im
   Bedienfluss erreichbar.

Der Rest des Strangs ist **sauber gemessen**: Orientierung, Geschlossenheit, Eck-Dichtheit über alle
vier Endpunkt-Kombinationen und fünf Stärke-Paare, das Miter-Limit über den vollen Winkel-Sweep, die
Rückfälle auf stumpf, die Raum-Neuerkennung nach einer Stärken-Änderung und die Totalität an der
Toleranz-Grenze wie bei extremen Koordinaten — elf Negativbefunde, jeder mit Messwert.

Nach Behebung von HIGH-1 und Auflösung von HIGH-2 (implementieren **oder** die AK-Zeile ehrlich als
Teilumfang zurücknehmen) empfiehlt dieser Report, **MEDIUM-1 im selben Zug** zu erledigen: die
Invarianten-Sonde am Wand-Netz ist der Sensor, der beide Klassen künftig fängt — Normalensumme **und**
Netz-Volumen-Gleichheit **und** Kanten-Manifold, denn die Divergenz-Sonde allein hätte HIGH-1
durchgelassen.

---

## Arbeitsbaum

Alle Sonden wurden zurückgenommen: `tests/adapters/test_mr009_probe.cpp` gelöscht,
`tests/CMakeLists.txt` auf den gemerkten Inhalt zurückgeschrieben (**nicht** über `git checkout`).
`git status --short` zeigt außer diesem Report **nichts** — geprüft.
