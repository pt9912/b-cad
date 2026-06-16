---
id: slice-017c
titel: Auswertung Volumen — EvaluatePort.volume() + EVL-002 (Bauteil-Netto-Volumen, analytisch im Kern)
status: in-progress
welle: welle-3-auswertung
lastenheft_refs: [[LH-FA-EVL-002](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md), [ADR-0012](../../adr/0012-evaluations-architektur.md)]
---

# Slice 017c: Auswertung Volumen — EvaluatePort.volume() + EVL-002

**Status:** in-progress. **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen** (unabhängig, ohne
Autoren-Kontext): 1 HIGH + 3 MED + 2 LOW eingearbeitet
([Report](../../../reviews/2026-06-16-slice-017c-plan.md)) — HIGH-1
(§1-„Dach"-Klausel entfernen) gelöst → **startbar**. Geometrieschwer →
**[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) greift** (unabhängiges Code-Review vor Welle-Closure).

**Welle:** welle-3-auswertung (dritter Slice; zweite Auswertungs-Implementierung,
erstmals geometrie-korrektheits-nah).

**Bezug:** [LH-FA-EVL-002](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung) (Volumenberechnung), spez. §1 [`LH-FA-EVL-001.a`](../../../../spec/lastenheft.md#lh-fa-evl-001--flächenberechnung)
(Volumen-Block). **Implementiert die [ADR-0012](../../adr/0012-evaluations-architektur.md)-#4-Folgepflicht** (analytisches
Netto-Volumen im Kern) für den **Volumen-Teil**. [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md) (Öffnungs-Schnittprismen
— Grundlage des geklemmten Öffnungsvolumens; Platten-/Treppen-Solid als
eigenständiges Element). [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Auswertung = reine Kern-Query, **kein**
`GeometryKernelPort`).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-16.

**Schnitt-Herkunft:** Volumen-Hälfte des Auswertungs-Strangs (Nachfolge 017b
Flächen). **Scope:** `EvaluatePort.volume()` (neu) + EVL-002 **Netto-Material-
Volumen** für **Wand · Decke/Fundament · Treppe** — je Bauteil aus seinem
**analytischen Solid im Kern**, ohne OCC/`GeometryKernelPort` ([ADR-0012](../../adr/0012-evaluations-architektur.md) #4).
**Bewusst NICHT Teil (Projektinhaber-Entscheidung 2026-06-16):**

- **Dach-Volumen** — das Dachmodell (`model::Roof`) ist **dicke-los** (Shell aus
  geneigten Flächen, unten offen); sein einziges berechenbares Solid wäre der
  **umbaute Raum** unter der Dachfläche — **kein Bauteil-/Material-Volumen**. Es
  in dieselbe `total_m3`-Summe wie Wand/Decke/Treppe (echte Material-Solids) zu
  falten, beschädigte die Massenermittlung semantisch. → **benannte welle-3-Lücke
  + Re-Eval-Trigger** (Dach-Dicken-/Material-Semantik); spez. §1 ehrlich nachziehen.
- EVL-004/005/006 (Listen) · Material-Strang (`model::Material`/Zuweisung/Round-Trip)
  · MAT-004 Texturen · DRW (welle-5).

---

## 1. Ziel

Der zweite Auswertungs-Wert: das **Netto-Material-Volumen** des Gebäudes
(EVL-002) in m³, als **read-only-Ableitung** aus dem committeten Modell über
`EvaluatePort.volume()`. **Analytisch im Kern** ([ADR-0012](../../adr/0012-evaluations-architektur.md) #4) — je Bauteil aus
seinem analytischen Solid, **ohne** Adapter-Round-Trip:

- **Wand** = Shoelace-Fläche des **Wand-Footprints** (`wallFootprint`, inkl.
  Eckenschluss WAL-006/slice-012) · `height_mm` **minus** je Öffnung das **real
  entfernte, geklemmte Volumen** `width_mm · wall.thickness_mm · clamped_height`,
  `clamped_height = min(sill+height, Wandhöhe) − max(0, sill)` (spez. §1, [ADR-0012](../../adr/0012-evaluations-architektur.md)
  #4 — **NICHT** das überstands-behaftete Roh-Schnittprisma).
- **Decke/Fundament/Bodenplatte** = (Shoelace-Footprint-Fläche − Σ gültige
  Ausschnitt-Flächen) · `thickness_mm` (spez. §1 SLB-001.a — Ausschnitt reduziert
  das Volumen; Gültigkeit via `cutoutInsideSlab`, wie `slabCutPrisms`).
- **Treppe (gerade einläufig)** = Σ Stufenkörper = `tread_mm · width_mm ·
  storey_height · (step_count+1)/2` (geschlossene Form von Σ_{i=1..n} tread·width·
  i·rise, `rise = from_storey_höhe / step_count`). **Geländer ausgenommen
  (MED-1):** es ist generierte Render-Geometrie — seine **Höhe** ist zwar spez.
  (STR-004, `STAIR_RAILING_HEIGHT_MM`), seine **Streifenstärke** aber eine
  unspez. Render-Konstante (`kRailThicknessMm` = 50 mm, `stair_geometry.cpp`),
  und es trägt **kein Material**; ein Material-Volumen auf einer Render-
  Streifenstärke wäre unehrlich → EVL-002 bemisst den **strukturellen
  Stufenkörper**.

**Kernbeobachtung ([ADR-0012](../../adr/0012-evaluations-architektur.md) #4, kritisch):** `solids_[id].volume_mm3` wird über
`geometry_.extrudeFootprint(...)` befüllt — in Produktion der **OCC-Adapter
(`GProp`)**. EVL-002 **liest dieses Feld NICHT** (das wäre eine driven-
Volumenmessung und kippte „reine Kern-Query"); das Wand-Volumen wird **eigen-
ständig analytisch** im Kern gerechnet (Footprint-Fläche selbst, Öffnungs-
Klemmung selbst). `make arch-check` sichert: kein `GeometryKernelPort`-Aufruf im
Volumen-Pfad.

## 2. Definition of Done

- [x] **Ergebnis-Werttyp `model::VolumeReport`** (`src/hexagon/model/volume_report.h`,
      neu, pure Werte, framework-frei — kein OCC/Qt/SQLite): Summe **und**
      Bauteiltyp-Subtotale, in m³:
      ```cpp
      struct VolumeReport {
          double total_m3{0.0};   // Σ Netto-Material-Volumen (Wand+Decke/Fundament+Treppe)
          double walls_m3{0.0};
          double slabs_m3{0.0};   // Decke + Fundament + Bodenplatte
          double stairs_m3{0.0};
          // Dach bewusst NICHT enthalten (dicke-loses Shell-Modell → kein
          // Bauteil-Volumen; welle-3-Lücke, ADR-0012/spez. §1 EVL-001.a).
      };
      ```
      Die Bauteiltyp-Subtotale machen die **Dach-Absenz strukturell sichtbar**
      (es gibt schlicht kein `roofs_m3`) und geben dem [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Review je Typ ein
      prüfbares Orakel.
- [x] **`EvaluatePort.volume()`** (`src/hexagon/ports/driving/evaluate_port.h`,
      additiv): `virtual model::VolumeReport volume() const = 0;` — gebäudeweit
      (EVL-002 „ein Gebäude mit Bauteilen"). Reine Abfrage; **kein** `…Changed`-`op`,
      **kein** `GeometryKernelPort`-Aufruf ([ADR-0012](../../adr/0012-evaluations-architektur.md) #1/#3/#4).
- [x] **Kern-Volumen-Analytik** (`src/hexagon/services/volume_geometry.{h,cpp}`,
      neu — pure Funktionen, eigenständig unit-testbar; Muster `wall_footprint`/
      `slab_geometry`):
      - `double polygonArea(const model::Footprint&)` — vorzeichenlose Shoelace
        (öffentlich). **MED-3:** dieselbe vorzeichenlose Shoelace existiert
        bereits **zweimal** file-lokal — `polygonArea` in
        `structure_edit_service.cpp` und `absPolygonArea` in `slab_geometry.cpp`.
        Dieser Slice **ersetzt die service-lokale Kopie** durch
        `volume_geometry::polygonArea` (Null-Risiko-Konsolidierung, hält die
        Zahl bei zwei); die `slab_geometry`-Kopie bleibt als benannte
        Drift-Notiz für einen späteren Merge (**nicht** Teil dieses Slice).
      - `double wallNetVolumeMm3(const model::Wall&, const std::vector<model::Wall>&,
        const std::vector<model::Opening>&)` = `polygonArea(wallFootprint(w,walls))
        · w.height_mm − Σ openingClampedVolume`. Öffnungs-Klemmung
        (`clamped_height` wie spez. §1, ⌊0⌋-geklemmt, Volumen ≥ 0).
      - `double slabNetVolumeMm3(const model::Slab&)` = `(polygonArea(footprint) −
        Σ polygonArea(gültiger cutout)) · thickness_mm` (Gültigkeit
        `cutoutInsideSlab`; **nie negativ** — Klemmung auf ≥ 0, Totalität).
      - `double stairNetVolumeMm3(const model::Stair&, double from_storey_height_mm)`
        = `tread · width · h · (step_count+1)/2` (≥ 1 Stufe, endliche/positive
        Maße; sonst 0 — Totalitäts-Kontrakt wie `stairMesh`/`stairRiseMm`).
- [x] **Service implementiert `volume()`** (`StructureEditService`, `const`):
      iteriert `building_.walls` / `.slabs` / `.stairs`, summiert die Kern-
      Analytik je Typ, mm³→m³ (÷ 1e9, lokale `constexpr` wie `kMm2PerM2`); füllt
      `walls_m3`/`slabs_m3`/`stairs_m3` + `total_m3`. Treppen-Höhe via
      `storeyHeight(stair.from_storey_id)`. **Leeres Modell → alle 0** (kein Wurf).
      **Keine Mutation, kein Re-Detect, kein `op`.**
- [x] **AK-Tests mit [`LH-FA-EVL-002`](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung) im Namen** (Kern, OCC-frei über
      `AnalyticGeometry`-Double, **unabhängige analytische Orakel** — nicht die
      Produktionsformel gespiegelt):
      - **Happy (Wand):** freistehende Rechteck-Wand → `walls_m3` = Länge·Stärke·
        Höhe (Orakel `analyticVolume(Wall)`, mm³→m³); `total_m3` = `walls_m3`.
      - **Wand mit Öffnung:** Tür/Fenster → Volumen sinkt um `width·thickness·
        clamped_height` (Tür sill=0 voll; Fenster mit Brüstung geklemmt;
        Boundary: `sill+height` über Wandhöhe → Oberkante geklemmt). Orakel
        unabhängig aus den Öffnungsmaßen gerechnet.
      - **Decke + Ausschnitt:** Platte mit innenliegender Aussparung → `slabs_m3`
        = (Fläche − Ausschnitt)·Dicke. **LOW-1:** gespeicherte `slab.cutouts`
        sind bereits von `addSlabCutout` vorgefiltert — der „Aussparung außerhalb
        des Umrisses → kein Abzug"-Beleg testet daher die **pure
        `slabNetVolumeMm3`-Funktion direkt** mit einem handgebauten Außen-Cutout
        (nicht über `addSlabCutout`, das ihn vorab verwürfe), sonst prüft er den
        `cutoutInsideSlab`-Gate nicht.
      - **Treppe:** bekannte Stufenzahl/Höhe → `stairs_m3` = Σ Stufenkörper
        (geschlossene Form), **Geländer-frei** (Wert hängt nicht von
        `kRailThicknessMm` ab — der [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Beleg).
      - **Gemischtes Gebäude:** Wand+Decke+Treppe → `total_m3` = Summe der
        Subtotale; **Dach im Modell ändert `total_m3` NICHT** (Dach-Ausnahme
        belegt).
      - **Boundary (EVL-002):** leeres Modell → alle Felder 0, kein Wurf;
        **LOW-2:** Treppe mit `from_storey_id` auf ein unbekanntes Geschoss →
        `storeyHeight` 0 → Treppen-Volumen 0, kein Wurf (Totalität explizit
        gepinnt).
      - **read-only/stabil:** zwei `volume()`-Abfragen identisch; `building()`
        unverändert (kein versteckter Re-Detect; Zugriff über `EvaluatePort&`).
      - **Regression:** bestehende EVL-001/003- und Bauteil-Tests textlich
        unverändert grün (additiver Port).
- [x] **spez. §1 [`LH-FA-EVL-001.a`](../../../../spec/lastenheft.md#lh-fa-evl-001--flächenberechnung) (Volumen-Block) nachgezogen — HIGH-1:** im
      Satz „Je Bauteil: **Dach**/Platte/Treppe aus ihrem analytischen Solid"
      **„Dach" entfernen** (→ „Platte/Treppe aus ihrem analytischen Solid") —
      sonst widerspräche §1 sich selbst (Dach gleichzeitig „aus analytischem
      Solid" UND ausgeschlossene Lücke). Dach-Volumen als **benannte
      welle-3-Lücke** (dicke-loses Modell, kein Bauteil-Solid) + Re-Eval-Hinweis
      (Dach erhält ein Volumen erst mit Dicke-/Material-Semantik) ergänzen;
      EVL-002 welle-3 = Wand/Decke/Treppe. + Historie-Zeile in
      [`spec/spezifikation-historie.md`](../../../../spec/spezifikation-historie.md)
      (§8 ist seit [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths) Pointer-Stub). **Lastenheft unberührt** (AK schon auf
      Niveau; Dach-Mechanik ist Lösung → §1, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei); kein [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).
- [x] **[ADR-0012](../../adr/0012-evaluations-architektur.md) unberührt (MED-2):** **kein** Body-Edit (Accepted-immutable,
      AGENTS §2.5). Die Dach-Lücke lebt allein in spez. §1 (Lösungs-Stratum,
      fortschreibbar); die bestehenden [ADR-0012](../../adr/0012-evaluations-architektur.md)-Re-Eval-Trigger
      („Nicht-prismatische/komplexe Volumen", „Exaktes vereinigtes Volumen")
      tragen die Dach-Wiedervorlage ohne Edit mit.
- [x] **`make arch-check` grün** (`VolumeReport`/`volume_geometry`/Service-Pfad
      pure Domäne, kein OCC/Qt/SQLite, **kein `GeometryKernelPort`-Aufruf** fürs
      Volumen); **`make test`** grün; **`make gates`** grün; **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) Code-Review**
      (Volumen-Korrektheit) ohne offene HIGH; Closure-Notiz mit Lerneintrag;
      CHANGELOG ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/volume_report.h` | neu | `VolumeReport` (total + Typ-Subtotale, pure Werte) |
| `src/hexagon/ports/driving/evaluate_port.h` | ändern | `volume()` additiv |
| `src/hexagon/services/volume_geometry.{h,cpp}` | neu | `polygonArea` + `wallNetVolumeMm3`/`slabNetVolumeMm3`/`stairNetVolumeMm3` (pure, testbar) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | `volume()` implementieren: Aggregation über walls/slabs/stairs, mm³→m³ |
| `src/hexagon/CMakeLists.txt` | ändern | `volume_geometry.cpp` ins Kern-Target |
| `tests/hexagon/test_evaluate_volume.cpp` | neu | EVL-002-AK (unabhängige Volumen-Orakel) |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |
| `spec/spezifikation.md` | ändern | §1 EVL-001.a Volumen-Block: Dach-Lücke + Re-Eval |
| `spec/spezifikation-historie.md` | ändern | §8-Historie-Zeile |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `docs/reviews/2026-06-16-slice-017c-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- slice-017b done ✓ (`EvaluatePort` + `AreaReport`, EVL-001/003).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (HIGHs blockieren) — **gelaufen ✓**,
  1 HIGH + 3 MED + 2 LOW eingearbeitet, startbar.
- Projektinhaber-Scope-Entscheidung 2026-06-16: Dach zurückgestellt (Option 1).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review ohne offene HIGH**,
  Closure-Notiz → danach **EVL-004/005/006 (Listen) + Material-Strang**; dann
  Welle-Verifikation + `done/welle-3-results.md`.

## 6. Risiken und offene Punkte

- **Adapter-Volumen-Falle ([ADR-0012](../../adr/0012-evaluations-architektur.md) #4, HIGH-Klasse):** `solids_.volume_mm3` kommt
  aus dem `GeometryKernelPort` (OCC `GProp`) — EVL-002 **darf es nicht lesen**.
  Das Wand-Volumen wird eigenständig analytisch gerechnet; `make arch-check`
  belegt die `GeometryKernelPort`-Freiheit des Volumen-Pfads. **Primäres
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Prüfziel.**
- **Öffnungs-Klemmung statt Roh-Prisma (Geometrie-Korrektheit):** abgezogen wird
  `width·thickness·clamped_height` (real entfernt), **nicht** das per
  `kOpeningCutOvershootMm` überstehende Schnittprisma. Test pinnt einen Fenster-
  Fall mit Brüstung **und** einen mit Überkante-über-Wandhöhe (beidseitige
  Klemmung).
- **Eck-Näherung ([ADR-0012](../../adr/0012-evaluations-architektur.md) #4, bewusst):** Σ Wand-Volumina **doppelzählt** den
  Miter-Sporn endpunkt-verbundener Wände (slice-012-Footprint) — kleine
  Über-Zählung, welle-3 in Kauf genommen; exaktes vereinigtes Volumen
  (Footprint-Union/Geschoss) ist Re-Eval (parallel WAL-006-Vollumfang). In der
  Closure-Notiz benennen, im Test **nicht** als Fehler werten.
- **Treppen-Geländer ausgenommen (Semantik-Entscheidung, MED-1):** das Geländer
  ist generierte Render-Geometrie — Höhe spez. (STR-004, `STAIR_RAILING_HEIGHT_MM`),
  aber Streifenstärke `kRailThicknessMm`=50 mm unspez.-Render; es trägt kein
  Material → nicht im Material-Volumen. Test belegt die Unabhängigkeit von
  `kRailThicknessMm` ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Sentinel). [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) prüft die Entscheidung.
- **Dach-Lücke (Projektinhaber-Scope):** dicke-loses Dachmodell → kein Bauteil-
  Volumen; ehrlich in §1 + [ADR-0012](../../adr/0012-evaluations-architektur.md)-Re-Eval. Test belegt, dass ein Dach
  `total_m3` nicht verändert.
- **Slab-Negativ-Klemmung:** Ausschnitte > Plattenfläche (pathologisch) →
  Volumen auf ≥ 0 klemmen (Totalität, kein negatives Volumen).
- **Überlappende Aussparungen ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) LOW-1, benannte Näherung):** analytisch
  wird die **Summe** der Ausschnitt-Einzelflächen abgezogen (nicht ihre
  Vereinigung) → kleine **Unter-Zählung** im seltenen Überlappungsfall (der reale
  Boolean entfernt die Vereinigung). In spez. §1 als „Aussparungs-Näherung"
  benannt, Re-Eval — Geschwister der Miter-Eck-Näherung; kein Test erzwingt
  überlappende Cutouts.
- **mm³ → m³:** ÷ 1e9 (1 m³ = 1e9 mm³); Test mit `EXPECT_NEAR` (fp-Toleranz).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (neuer Ergebnis-Werttyp + Port-Methode + pure
  Volumen-Analytik je Bauteiltyp, read-only, Totalität, **kein Adapter-Volumen**);
  Risiko **mittel** — geometrie-korrektheits-nah (Öffnungs-Klemmung, Eck-Näherung,
  Treppen-/Platten-Solid) → **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) greift**.

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (AK mit `LH-`-ID, **unabhängige** analytische
  Volumen-Orakel — nicht die Produktionsformel gespiegelt); Risiko niedrig.

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; Dichte mittel (Lösungs-Mechanik-Nachzug §1 + Lücke/Re-Eval,
  lösungsfrei-Disziplin n/a — reine Spec-Ebene); Risiko niedrig (Doku).

## 8. Closure-Notiz

**Ergebnis:** `make gates` grün (EXIT 0; **131 Tests**, +9 EVL-002-AK; Coverage
92,4 % > 70 %; `volume_geometry.cpp` 97 %). `EvaluatePort.volume()` liefert das
gebäudeweite **Netto-Material-Volumen** + Bauteiltyp-Subtotale (`VolumeReport`:
Wand/Decke-Fundament/Treppe), **analytisch im Kern** (`volume_geometry`) — Wand =
Footprint·Höhe − geklemmte Öffnungen, Platte = (Fläche − gültige Ausschnitte)·
Dicke, Treppe = Σ Stufenkörper (geländer-frei), mm³→m³. **Kein
`GeometryKernelPort`, kein `solids_.volume_mm3`-Lesen** (grep-verifiziert,
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)). Dach welle-3 ausgenommen (dicke-los). Die service-lokale `polygonArea`
ist auf `volume_geometry::polygonArea` konsolidiert (MED-3).

**DoD:** alle Haken erfüllt — `VolumeReport` + `volume()` + `volume_geometry`
(3 pure Funktionen), Service-Aggregation, AK [`LH-FA-EVL-002`](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung)
(Wand/Tür/Fenster-Klemmung/Decke+Ausschnitt/pure-Außen-Cutout/Treppe-geländer-frei/
gemischt+Dach-ausgenommen/leer+unbekanntes-Geschoss/read-only), `make arch-check`
+ `make gates` grün, spez. §1 nachgezogen (Dach entfernt + Lücke/Re-Eval),
CHANGELOG.

**Reviews:** **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review** (unabhängig) 1 HIGH + 3 MED + 2 LOW
eingearbeitet (HIGH-1 §1-„Dach"-Klausel; MED-1 Geländer-Wortlaut; MED-2 kein
ADR-Body-Edit; MED-3 polygonArea-Konsolidierung; LOW-1/2 Slab-Pure-Test/Treppe-
Boundary). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review** (unabhängig, geometrieschwer) **0 HIGH —
closure-ready**: alle Formeln per Hand bestätigt (Tür 1,9464 m³; Fenster-Klemmung
2,256 m³; Treppe 5,6 m³ = Σ Stufenkörper), Klemmung statt Roh-Prisma korrekt,
Totalität vollständig, [ADR-0012](../../adr/0012-evaluations-architektur.md)-#4-Invariante grep-verifiziert; ein LOW-1 als
benannte Näherung eingearbeitet.

**Lerneintrag:** Der **kritische** Befund war architektonisch, nicht numerisch:
`solids_.volume_mm3` existiert und trägt das *adapter-gemessene* (OCC-`GProp`)
Wand-Volumen — es zu lesen wäre der bequeme Weg gewesen und hätte „reine
Kern-Query" ([ADR-0012](../../adr/0012-evaluations-architektur.md) #4) **still gekippt, ohne dass ein Test rot würde**. Der
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review verifizierte die Adapter-Herkunft des Feldes, der Plan machte die
eigenständige Kern-Analytik zur Pflicht, `make arch-check` + der [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-grep
sichern sie. **Grünes Gate ≠ Schicht-Treue: die Falle hätte mit gelesenem
Adapter-Volumen genauso grün getestet** — die Architektur-Invariante musste
inferential (Review) + computational (arch-check, kein `GeometryKernelPort` im
Pfad) gehalten werden.

**Restrisiko / Nachfolge:**
- **Überlappende Platten-Aussparungen ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) LOW-1):** Σ Einzelflächen statt
  Vereinigung → kleine Unter-Zählung; in spez. §1 als „Aussparungs-Näherung"
  benannt, Re-Eval (parallel zur Miter-Eck-Näherung).
- **Eck-Näherung (welle-3, benannt):** Σ Wand-Volumina doppelzählt den
  Miter-Sporn endpunkt-verbundener Wände — exaktes vereinigtes Volumen ist
  Re-Eval (parallel WAL-006-Vollumfang).
- **Dach-Volumen-Re-Eval:** sobald das Dach eine Dicke-/Material-Semantik erhält.
- **Folge-Slices:** EVL-004/005/006 (Listen) + Material-Strang
  (`model::Material`/Zuweisung/`material_id`-Round-Trip); dann Welle-Verifikation
  + `done/welle-3-results.md`.
