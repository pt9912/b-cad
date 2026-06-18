# MR-009 Code-Review — slice-023b (Dach-Volumen Impl — geschlossener Schräg-Slab)

## Kopf

- **Review-Art:** [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  — unabhängiges, **geometrielastiges** Code-Review **vor Welle-Closure**
  (Reviewer ≠ Autor, kein Autoren-Kontext). Prüfung der **Geometrie-Korrektheit
  gegen die Spezifikation**: Wasserdichtheit, Orientierung/Winding (Außennormalen),
  Maß-/Volumen-Exaktheit, Totalität bei Degeneration, Test-Orakel-Tiefe.
  **HIGH-Findings blockieren die Closure.**
- **Gegenstand:** slice-023b im Arbeitsbaum (noch nicht committet): `roofMesh`
  liefert statt der offenen Dachfläche einen geschlossenen, wasserdichten
  Schräg-Slab der Dicke `d` (Oberseite + vertikal um `d` versetzte Unterseite +
  Rand-Kanten-Seitenwände, alle drei Typen Sattel/Walm/Pult inkl. Walm-Zeltdach);
  `Roof.thickness_mm` + EditService-Default/Klemmung + `setRoofThickness`-Setter;
  EVL-Dach-Volumen `roofNetVolumeMm3 = bx·ty·d`.
- **Datum:** 2026-06-18.
- **Quellen (selbst verifiziert):** `roof_geometry.cpp:58-237`
  (`buildTopShell`/`closeSlabDownward`/`vertexKey`/`undirectedEdge`/`roofMesh`);
  `roof.h:35` (`thickness_mm`); `constants.h:43-45` (`kRoofThickness*` 50/500/200);
  `structure_edit_service.cpp:308-315` (`volume()`-Dach-Aggregation),
  `:699-710` (`addRoof`-Default/clamp), `:742-754` (`setRoofThickness`),
  `:56-68` (`evaluateParam`/`Range`); `volume_geometry.cpp:88-105`
  (`roofNetVolumeMm3`); `volume_report.h:14-26,52` (`roofs_m3`, Kommentar);
  `edit_structure_port.h:97-101` (Port-Erweiterung);
  `stl_export_adapter.cpp:79-87` (`roofMesh`-Konsument, jetzt geschlossen);
  `tests/hexagon/test_roof_geometry.cpp:64-227` (`isWatertight`/`signedVolumeMm3`/
  Slab-Invarianten/Apex/Totalität); `tests/hexagon/test_evaluate_volume.cpp:194-247`;
  `tests/adapters/test_viewer_scene.cpp:256-283`; `spezifikation.md:189-216`
  (§1 ROF-001.a Dicke-Block: **vertikal**, geschlossen/wasserdicht/außen-orientiert,
  analytisch **ohne** `Solid.volume_mm3`), `:738-740` (§3 `ROOF_THICKNESS_*`);
  `lastenheft.md:459-473` (`LH-FA-ROF-006`).
- **Eigenständige rechnerische Verifikation:** Replikation des **exakten**
  `roof_geometry`-Algorithmus (`buildTopShell` + `closeSlabDownward`) in einem
  Stand-Alone-Programm; Kanten-/Manifold-Bilanz + Divergenzsatz-Volumen über
  15 Parametrierungen (alle 3 Typen, beide Achsen-Orientierungen, Zeltdach,
  steil/flach, `d`∈{50,200,500}, Überstand 0, First < `d`); plus
  Test-Orakel-Penetration (gezielt invertierte Seitenwand) und µm-Merge-Analyse.

## Verdikt

**0 HIGH / 0 MED / 2 LOW / 2 INFO** — **Closure aus Geometrie-Sicht freigegeben.**

Die Geometrie ist für **alle drei Typen einschließlich des Walm-Zeltdach-Apex**
**wasserdicht** (jede ungerichtete Kante genau 2 Flächen), **außen-orientiert**
(positives signiertes Volumen) und **maßgenau** — das signierte Volumen des
geschlossenen Netzes ist in **jedem** geprüften Fall **bit-exakt gleich** der
analytischen EVL-Formel `bx·ty·d`. Kein HIGH, kein MED. Die zwei LOWs sind
Test-Orakel-Härtungen (nach Merge), die INFOs Dokumentations-Notizen.

## Verifiziert-korrekt (mit konkreter Geometrie-Prüfung)

Die Replikation des echten Codes ergab über 15 Grundrisse/Parameter durchgängig
`watertight=YES` (min = max = 2 Kanten-Nutzungen, Netz nicht leer) und
`signedVol > 0`, **und** signiertes Volumen == `bx·ty·d` auf 1e-6 m³:

| Fall | tris | Kanten | min/max | signiertes Vol | `bx·ty·d` |
|---|---|---|---|---|---|
| Pult | 12 | 18 | 2/2 | 12,600000 m³ | 12,600000 |
| Sattel `bx≥ty` | 20 | 30 | 2/2 | 12,600000 | 12,600000 |
| Sattel `bx<ty` | 20 | 30 | 2/2 | 12,600000 | 12,600000 |
| Walm `bx≥ty` | 20 | 30 | 2/2 | 12,600000 | 12,600000 |
| Walm `bx<ty` | 20 | 30 | 2/2 | 12,600000 | 12,600000 |
| **Walm Zeltdach** (quadr.) | **16** | **24** | **2/2** | **9,800000** | **9,800000** |
| Walm Zeltdach (Überstand 0) | 16 | 24 | 2/2 | 7,200000 | 7,200000 |
| Walm fast-quadr. ±1 mm | 20 | 30 | 2/2 | 9,801400 | 9,801400 |
| Sattel steil 60° | 20 | 30 | 2/2 | 12,600000 | 12,600000 |
| Sattel `d=50` | 20 | 30 | 2/2 | 3,150000 | 3,150000 |
| Sattel `d=500` Neigung 45° | 20 | 30 | 2/2 | 31,500000 | 31,500000 |
| Sattel Überstand 0 | 20 | 30 | 2/2 | 9,600000 | 9,600000 |
| Sattel flach, First < `d` | 20 | 30 | 2/2 | 8,000000 | 8,000000 |

**Wasserdichtheit (Linse 1).** Bestätigt für jeden Typ. Der **Zeltdach-Apex**
ist der heikelste Fall: bei quadratischem Grundriss kollabieren beide Walm-
Trapeze (`r1 == r2 == Apex`), die kollabierten Dreiecke werden in `appendTriangle`
über die `mag < 1e-9`-Schwelle **verworfen** (`roof_geometry.cpp:39-41`). Die
Bilanz bleibt trotzdem geschlossen (16 Dreiecke, 24 Kanten, alle Nutzung 2),
weil sich für das Apex-Pyramidendach Top-, Bottom- und Wand-Flächen **konsistent**
zählen — die Zähl-Inkonsistenz-Sorge (Top verwirft, Wand zählt nicht) tritt nicht
ein, da die Rand-Kanten-Zählung (`:197-206`) **dieselben** verworfenen Dreiecke
nie als Kanten-Beitrag aufnimmt: die Schleife zählt Kanten der Oberseiten-Vertices,
und eine Apex-Kante `corner→apex` wird von zwei nicht-degenerierten Dreiecken
geteilt → innen → keine Wand. Korrekt.

**µm-Kanonisierung (Linse 1, Verschmelzung).** Geprüft: geteilte analytische
Vertices (Firstpunkte `r1`/`r2`, Apex, Trauf-Ecken) werden über **identische
Formeln** berechnet und sind damit **bit-identisch** → sie runden auf denselben
µm-Schlüssel (`vertexKey`, `:167-171`), kein Spalt durch Float-Drift. Umgekehrt:
über alle geprüften Parameter (inkl. krummer Neigungen 37,123456° und
versetztem Ursprung) lag **kein** Paar **verschiedener** Vertices im selben
µm-Bucket (max-Spread innerhalb eines Schlüssels = 0,000000 mm). Bei
rechteckigem Grundriss sind verschiedene Vertices stets um ≥ Inset/Halbspanne
(Millimeter) getrennt; eine Sub-µm-Kollision ist mit gültig geklemmten
Parametern nicht erreichbar. **Kein Loch / kein non-manifold.**

**Orientierung (Linse 2).** Bestätigt durch positives signiertes Volumen **und**
durch die Gleichheit mit `bx·ty·d`: Wäre **irgendeine** Wand (oder Top/Bottom)
invertiert, würde sich die Divergenzsatz-Summe ändern und nicht mehr exakt
`bx·ty·d` ergeben. Die Wand-Wicklung `appendQuad(p, p↓, q↓, q)` (`:218`) ergibt
für eine CCW-von-oben-Oberseiten-Fläche (Inneres links von p→q) eine
**Außennormale rechts** der Kante — verifiziert über die globale, vorzeichen-
exakte Volumen-Gleichheit. Oberseite zeigt +z (`hi == axisBounds.hi`),
Unterseite liegt bei `−d` (Test `:217`).

**EVL-Volumen (Linse 3).** `roofNetVolumeMm3 = bx·ty·d` mit
`bx = width+2·overhang`, `ty = depth+2·overhang` (`volume_geometry.cpp:94-104`)
ist das **korrekte Material-Volumen** des vertikal-dicken Slabs: weil der Offset
**vertikal** ist, ist die projizierte Grundfläche jeder Slab-„Säule" gleich der
Trauf-Grundfläche und die Höhe überall `d` — der schräge Deckel verschiebt
Material nur, ändert das Integral `∫d` über die projizierte Fläche nicht. Die
**bit-exakte Übereinstimmung** des unabhängig per Divergenzsatz gemessenen
geschlossenen-Netz-Volumens mit `bx·ty·d` (Tabelle oben) **beweist** die
Konsistenz von Formel und tatsächlicher Slab-Geometrie — kein „geneigte
Oberseiten-Fläche · d"-Fehler. Totalität: `bx/ty < Toleranz` oder `d ≤ 0` → 0
(`:99-103`), konsistent mit `roofMesh`s Leer-Netz (`:233`).

**Totalität/Konsistenz (Linse 5).** `roofMesh` liefert bei degeneriertem
Grundriss (leere Oberseite) **oder** nicht-endlicher/nicht-positiver Dicke ein
leeres Netz, kein Wurf (`:233`). `addRoof` setzt Default 200 bei fehlender
(`≤0`/nicht-endlicher) Dicke, sonst `std::clamp` auf [50,500] (`:702-708`) —
deckungsgleich mit der `pitch`/`overhang`-Anlage; `setRoofThickness` nutzt den
realen `evaluateParam` mit `Range{50,500}` (`:742-754`), kann also nie `d ≤ 0`
speichern (die `d ≤ 0`-Guards sind defensiv für rohe Structs/Direktaufruf).
STL-Export (`stl_export_adapter.cpp:82`) konsumiert `roofMesh` unverändert,
jetzt geschlossen — `appendRoofMeshes` filtert Leer-Netze bereits ab; kein
Verhaltensbruch.

## Findings

### LOW-1 — Orakel `isWatertight` + `signedVolume>0` fängt eine **lokal invertierte Einzelwand nicht** (`test_roof_geometry.cpp:90-110,131-153`)

Penetrationstest (echtes Wicklungs-Layout, eine Pult-Seitenwand bewusst
invertiert): die Mannigfaltigkeit bleibt geschlossen (das Invertieren eines Quads
ändert nicht, **welche** ungerichteten Kanten es berührt → jede Kante weiter
Nutzung 2) **und** das Gesamt-`signedVolume` bleibt > 0 (eine kleine Wand drückt
das globale Vorzeichen nicht). Gemessen: korrekter Slab 1,2600e10; je
invertierte Wand 1,2000e10 / 4,6667e9 / 4,8000e9 / 1,2133e10 — alle > 0,
alle watertight.

**Keine HIGH-Eskalation:** Der **Produktionscode ist korrekt** (alle Wände gleich
gewickelt; bewiesen durch die exakte `signedVol == bx·ty·d`-Gleichheit, die eine
einzelne Inversion **sofort** zerstören würde). Das ist eine **Test-Orakel-Tiefe**-
Lücke (genau die slice-016b-Lehre, MR-009 Linse 4): die beiden vorhandenen Sonden
würden eine *künftige* lokale Wicklungs-Regression durchlassen.

**Fix (nach Merge):** eine **lokale** Orientierungs-Sonde ergänzen — z. B. „jede
Seitenwand-Normale ist horizontal (≈ 0 z) **und** zeigt vom Slab-Zentroid radial
nach außen (`n·(face_center − centroid) > 0`)", oder die stärkere Invariante
`signedVolume == bx·ty·d` (auf Toleranz) als Test, die genau die globale **und**
jede lokale Inversion fängt (sie ist im Review als Beweis schon genutzt). Da
nur ein Test-Härtungs-Schritt, blockiert er die Closure nicht.

### LOW-2 — `appendTriangle`-Degeneracy-Schwelle ist eine **Flächen**-Schwelle, kein Längen-/Toleranz-Bezug (`roof_geometry.cpp:38-41`)

`mag < 1e-9` verwirft anhand der **doppelten Dreiecksfläche** (Kreuzprodukt-Betrag,
mm²). Für den Apex-Kollaps (mm-große Kanten) ist die Trennung robust (verworfene
Dreiecke haben Fläche ~0, gültige ~10^6 mm²) — im Review verifiziert. Die Schwelle
ist aber **nicht** an `kGeometryToleranceMm` (0,1 mm) gekoppelt; bei extrem schmalen,
aber nicht-quadratischen Walm-Grundrissen (Inset nahe, aber nicht exakt Halbspanne)
könnte ein **sehr dünnes** First-Trapez (Fläche knapp über 1e-9, Kanten < 0,1 mm)
prinzipiell als gültig durchgehen und ein quasi-degeneriertes, aber kanten-konsistentes
Dreieck erzeugen. **Real nicht erreichbar:** der Zeltdach-Branch fängt `rx1 ≤ rx0`
(`:123-126`/`:139-142`) und die ±1-mm-Fast-Quadrat-Fälle sind im Review wasserdicht.
Vorbestehendes Verhalten (slice-014b), durch diesen Slice nicht verschlechtert.

**Fix (optional, nach Merge):** die Schwelle dokumentieren oder den Zeltdach-Kollaps
zusätzlich an `kGeometryToleranceMm` (First-Spanne `|rx1−rx0| < Toleranz` →
Apex) statt nur an `rx1 ≤ rx0` koppeln, damit „nahezu Punkt" deterministisch
zum Apex fällt. Kein Korrektheits-Defekt im geprüften Parameterraum.

### INFO-1 — `vertexAt`/Indices setzen Flat-Shading (kein Vertex-Sharing) voraus

`closeSlabDownward` liest `top.positions` über `top.indices` (`:179-182`); das
funktioniert nur, weil `buildTopShell` Flat-Shading ohne geteilte Vertices
erzeugt (jedes Dreieck eigene Vertices, `triangle_mesh.h`-Konvention). Korrekt
und konsistent mit der Repo-Konvention; rein dokumentarisch festgehalten, falls
ein künftiger Top-Builder Vertices teilt.

### INFO-2 — `<vector>`-Include in `roof_geometry.cpp:8` ungenutzt

`closeSlabDownward` nutzt `std::map`/`std::array`/`std::pair`, kein `std::vector`
direkt (das Netz kommt aus `TriangleMesh`). Harmlos; `clang-tidy`
(`misc-include-cleaner`) im `lint`-Gate ist die zuständige Instanz, nicht dieses
Review.

## Ergebnis

**Geometrie-Verdikt: 0 HIGH / 0 MED / 2 LOW / 2 INFO — Closure freigegeben.**

Die slice-023b-Dach-Geometrie erfüllt `LH-FA-ROF-006` und §1 `LH-FA-ROF-001.a`:
geschlossener Schräg-Slab mit **vertikalem** Offset, **wasserdicht** und
**außen-orientiert** für **alle drei Typen einschließlich des Walm-Zeltdach-Apex**,
mit **maßgenauem** Material-Volumen `bx·ty·d` analytisch im Kern (ohne
`Solid.volume_mm3`). Die Wasserdichtheit, Orientierung und Volumen-Exaktheit sind
**rechnerisch gegen den echten Code bestätigt** (Kanten-/Manifold-Bilanz +
Divergenzsatz == `bx·ty·d`), nicht nur plausibilisiert. `make test` grün (204/204).
Die beiden LOWs (Test-Orakel-Härtung gegen lokale Wand-Inversion; Apex-Schwellen-
Dokumentation) sind **nach** dem Merge zu erledigen und blockieren die
Welle-Closure **nicht**.
