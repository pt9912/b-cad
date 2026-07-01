---
id: welle-4-results
titel: Welle-Closure — Ergebnisnotiz welle-4-austausch
status: done
welle: welle-4-austausch
---

# Welle-4-Closure — Ergebnisnotiz `welle-4-austausch`

**Welle:** welle-4-austausch. **Zeitraum:** 2026-06-16 bis 2026-07-01.
**Autor:** Dietmar Burkard. **Closure-Datum:** 2026-07-01.

**Welle-Ziel (Roadmap):** b-cad **offen austauschbar** machen
([OBJ-005](../../../../spec/lastenheft.md#3-projektziele)): Import/Export der offenen Formate —
**IFC** ([LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002),
**DXF** ([LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003)/004),
**STEP** ([LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)),
**STL** ([LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)) + **PDF/PNG-Export**
([LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/008). Jedes Format liegt hinter einem
**Driven-Adapter** (`ModelImporterPort`/`ModelExporterPort`, angestoßen über
`ExchangeModelPort`/`ExchangeService`), der Kern bleibt **format-frei**
([ADR-0001](../../adr/0001-hexagonale-architektur.md)). Erfüllt **Meilenstein M4**
(„Offen austauschbar", [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien) IFC-Export +
[ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) maßstäblicher PDF-Plan). Vier
Backend-ADRs: **[ADR-0013](../../adr/0013-ifc-bibliothek.md)** (IFC), **[ADR-0014](../../adr/0014-step-stl-export-backend.md)**
(STEP/STL), **[ADR-0015](../../adr/0015-dxf-backend.md)** (DXF), **[ADR-0016](../../adr/0016-pdf-png-backend.md)** (PDF/PNG).

---

## 1. Closure-Kriterien (beobachtbar)

- **Alle Closure-Trigger der Roadmap erfüllt:** **16 Slices** liegen in
  `done-archive/` (Liste §2), jeder mit `status: done`, eigener Closure-Notiz und
  Lerneintrag. Diese Notiz schließt den letzten Trigger („unabhängige
  Welle-Verifikation + Closure-Notiz inkl. zwingendem Carveout-Audit").
- **`make gates` grün** (frischer, **unabhängig reproduzierter** Lauf 2026-07-01,
  Exit 0): docs-check **0 Befunde**, gate-consistency, arch-check **Regeln A–E**
  (Kern OCC-/Qt-/SQLite-frei; die io-residenten Codecs sind header-frei → Regel A/B,
  „Regel F gegenstandslos"), lint 0 + suppression-gate, **Tests 220/220** (100 %),
  **Coverage 90,7 % lines** (3545/3910) / **100 % functions**, Schwelle 70 %.
  Zusätzlich **`make schema-check` grün** (`schema.sql == d-migrate(data-model.yaml)`,
  kein Drift trotz der ersten Repo-Schema-Änderung `roofs.thickness_mm` in 023c) und
  **`make io-smoke` grün** (CI-only Binary-Smoke: IFC/DXF Export+Re-Import, STEP/STL/
  PDF/PNG Export — je exit 0 + nicht-leere Datei). Vom Verifier **selbst** gelaufen
  (Honesty-Disziplin), nicht aus den Slice-Läufen übernommen.
- **Unabhängige Welle-Verifikation gelaufen** (Reviewer ≠ Autor, eigener Agent ohne
  Autoren-Kontext; §3): **Welle closure-reif, keine HIGH.**

## 2. Gelieferter Umfang (Slices in `done-archive/`)

| Strang | Slices | Ergebnis |
|---|---|---|
| **IFC** ([ADR-0013](../../adr/0013-ifc-bibliothek.md)) | 019a/b/c | **Selbst getragener IFC-SPF-Subset-Codec (Option D)** io-resident; AK-Schärfung ([LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002, Lastenheft 0.1.8) + `SpfReader`/`IfcImportAdapter` + `SpfWriter`/`IfcExportAdapter` + `ExchangeService` (Driving `ExchangeModelPort`). **Roundtrip** Export→Import erhält Anzahl+Geometrie → **[ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)**. Geschoss-Höhe transient/Default (benannte Lücke). Je [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)+Code-Review 0 HIGH |
| **STEP/STL** ([ADR-0014](../../adr/0014-step-stl-export-backend.md)) | 020a/b · 024a/b | **OCC-DataExchange nativ** (keine neue Dependency), **geometrie-residenter** `ModelExporterPort` (Regel C); AK-Schärfung (export-only, 0.1.9) + **STL** (binär, alle 3D-Bauteile) + **STEP** (B-Rep AP214). **024a/b schließen die STEP-B-Rep-Lücke:** Dächer via `BRepBuilderAPI_Sewing` des wasserdichten Netzes (fail-closed), Treppen als analytische OCC-Box-Solids → **alle 3D-Bauteile B-Rep**; Geländer STL-only (render-only). Je [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH |
| **DXF** ([ADR-0015](../../adr/0015-dxf-backend.md)) | 021a/b | **Selbst getragener ASCII-DXF-Subset-Codec (Option D, R12)** io-resident; AK-Schärfung (bidirektional, 2D-Grundriss, 0.1.10) + Reader+Writer (`LINE`/`LWPOLYLINE` je Geschoss-`LAYER`) + `DxfImport`/`DxfExportAdapter`; **Kern-Import-Dispatch auf `ImporterMap`** (symmetrisch zur `ExporterMap`, ADR-Review-MED-1). Import → Default-Höhe/-Dicke (benannte Lücke). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)+Code-Review 0 HIGH |
| **Dach-Volumen** (Voraussetzung STEP-B-Rep) | 023a/b/c | **Neue Anforderung [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006)** (Dachdicke/Volumenkörper, Lastenheft 0.1.11): AK + Geometrie (wasserdichter Schräg-Slab alle 3 Typen + EVL-Dach-Volumen `bx·ty·d`) + Persistenz (`roofs.thickness_mm` — **erste Repo-Schema-Änderung**, via gepinntem d-migrate; no-version-bump regelwerk-konform). Löst die welle-3-Dach-Volumen-Lücke + macht STEP-B-Rep der Dächer möglich. [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH |
| **io-smoke** (Quergewerk) | 022 | `make io-smoke` — CI-only headless Binary-Smoke aller IO-Formate (erste reale LH-Bindung; kein `gates`-Member, Muster `schema-check`). Belegt die coverage-ausgenommene `main.cpp`-CLI-/Composition-Root-Glue |
| **PDF** ([ADR-0016](../../adr/0016-pdf-png-backend.md)) | 025a · 025b | **Selbst getragener Vektor-PDF-Writer (Option D)** io-resident, **kein Qt/OCC**; AK-Schärfung ([LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/008, 0.1.12) + `PdfWriter` + `PdfExportAdapter` — **maßstäblicher (1:100) 2D-Grundriss** je Geschoss-Seite + „M 1:100"-Label → **[ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)**. **Voll-Decode-Orakel** (Byte + Objektgraph/Reader-Öffenbarkeit + je Geschoss) + Maßstabs-/Orientierungs-Sonde; reale Reader-Öffenbarkeit empirisch belegt. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)+Code-Review 0 HIGH |
| **PNG** ([ADR-0016](../../adr/0016-pdf-png-backend.md)) | 025a · 025c | **Selbst getragener Raster-PNG-Encoder (Option D)** io-resident, **kein Qt/OCC/zlib**; `PngWriter` (Bitmap + Bresenham + Chunks/CRC-32/**stored-DEFLATE**/Adler-32) + `PngExportAdapter` — kombiniertes Rasterbild (feste Leinwand, Fit-to-Canvas geguardet, je Geschoss eine Farbe). **Voll-Decode-Orakel** mit **eigenständigen** CRC/Adler/Inflate; libpng-Öffenbarkeit empirisch belegt. Reuse `plan_geometry`/`io_atomic_write` (025b). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)+Code-Review 0 HIGH |

**IO-Oberfläche vollständig:** `ExchangeFormat = {Ifc, Step, Stl, Dxf, Pdf, Png}`;
**Import** IFC+DXF (`ImporterMap`, io-resident), **Export** IFC/DXF (io-resident) +
STEP/STL (geometrie-resident) + PDF/PNG (io-resident) über die `ExporterMap`; der
format-neutrale `ExchangeService` (Kern) dispatcht, der Composition Root verdrahtet
je Format (`--import-*`/`--export-*`).

**Bezug zu welle-1/-2/-3:** welle-1/-1v lieferten Kern + Viewer, welle-2 die
Bauteil-Familien, welle-3 die Auswertung. welle-4 macht das Modell **offen
austauschbar** — additiv über `ExchangeModelPort`/`ExporterMap`, **ohne** den Kern
format-abhängig zu machen; die einzige Kern-Architektur-Erweiterung war die
symmetrische `ImporterMap` (DXF-Import, 021b).

**Nicht Teil der Welle (bewusst, ehrlich benannt):**

- **Format-Subsets:** IFC/DXF decken **Geschosse + gerade Wände** (weitere Bauteile
  übersprungen/nicht geschrieben); DXF-Import → **Default-Höhe/-Dicke** (DXF trägt
  keine); STEP **B-Rep aller 3D-Bauteile** außer Treppen-**Geländer** (render-only,
  STL-only); PDF/PNG = **Achsen-Plan** (keine Wand-Footprint/-Dicke, Räume, Bemaßung,
  Text/Schrift außer PDF-Maßstabs-Label). Alle als **Re-Eval-Trigger** benannt.
- **PDF fester Maßstab 1:100** → Modelle > ~19,5 m laufen aus A4 (Fit-to-Page =
  Re-Eval); **PNG unkomprimiert** (feste Leinwand hält die Größe beschränkt) +
  kombiniertes Bild (PNG kennt keine Mehrseitigkeit). Kein PDF-/PNG-**Reader** im
  gepinnten Image → Sensor = voll-Decode-Selbst-Orakel + einmaliger empirischer
  Reader-Cross-Check durch das Code-Review (kein Reader-Test-Dependency,
  [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)).
- **Zurückgestellt (aus welle-3 / früher):** `wall_type`-Template-Fallback,
  `cost_per_m2`, MAT-004 Texturen, WAL-006-Vollumfang, **slice-006**
  (Drittanbieter-Attribution, bleibt in `open/`), **`DRW`-Modul** (welle-5).

## 3. Review & Verifikation vor Closure

**Zweistufige Review-Disziplin je Slice** (Kurs-Modul 10/11): (a) **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
Plan-Review** vor jedem Implementierungs-Start (unabhängig, HIGH blockiert) — über
alle 16 Slices keine Start-blockierende HIGH offen (mehrere MED/LOW eingearbeitet,
u. a. 023c **2 HIGH** = ungegründete Versions-Entscheidung, **vor Start** behoben);
(b) **Backend-ADR-Text-Reviews** vor Accept (0013–0016, je 0 HIGH); (c)
**geometrielastiges Code-Review** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) für die geometrieschweren
Slices (020b, 023b, 024a/b) — je 0 HIGH; (d) **Code-Review** für die self-rolled
Format-Codecs (019b/c, 021b, 025b/c) — je 0 HIGH, für PDF/PNG mit **empirischem
Reader-Cross-Check** (poppler/ghostscript bzw. libpng/PIL).

**Unabhängige Welle-Verifikation** (Reviewer ≠ Autor, eigener Agent, vor dieser
Closure): selbst `make gates` (Exit 0, 220/220, 90,7 %), `make schema-check` (Exit 0,
kein Drift) **und** `make io-smoke` (alle Formate exit 0) gelaufen; geprüft: alle 16
Slices done mit Closure+Lerneintrag; ADRs 0013–0016 accepted mit Folgepflichten
`erfüllt`; **[ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien) + [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) sachlich erreicht**; IO-Oberfläche vollständig; jede
benannte Lücke **ehrlich** dokumentiert (kein Über-Versprechen); keine aktiven
Carveouts; [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)/010/011/012-Hygiene gewahrt (Lösungsfreiheit, Header==Historie
0.1.12, Spec-Körper ADR-frei, alle Backends in §6/§7 entschieden).

**Ergebnis:** Welle closure-reif, **keine HIGH**. Der Verifier reproduzierte die Gates
**selbst** (`make gates` Exit 0 / 220/220 / 90,7 %; `make schema-check` kein Drift;
`make io-smoke` alle 6 Formate exit 0 inkl. IFC/DXF-Re-Import) — die Zahlen decken sich
mit dieser Notiz.

- **LOW (nicht-blockierend, außerhalb welle-4-Scope — belassen):** `spezifikation.md` §7
  führt weiterhin „Performance-Zielkomplexität der Raumerkennung (M3)" als offenen Punkt.
  Das ist ein **M3-Nachlass**, kein Format-Backend → bewusst **nicht** in dieser
  welle-4-Closure angefasst (Scope-Disziplin; Pflege bei Gelegenheit).

## 4. Carveout-Audit (zwingend bei Welle-Closure)

Geprüft 2026-07-01 (unabhängig in §3 bestätigt):
[`docs/plan/carveouts/README.md`](../../carveouts/README.md) führt **keine aktiven
Carveouts**; kein `CO-*`-Eintrag. Kein Gate war während der Welle strukturell rot,
**keine Architekturregel/Schwelle wurde geschwächt** — die vier neuen Format-Backends
sind additiv (Option D: keine neue Dependency, [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) unberührt; OCC-DataExchange
nutzt bereits gepinnte Toolkits). arch-check trägt unverändert **Regeln A–E** (die
io-residenten Codecs sind header-frei → durch A/B isoliert, „Regel F gegenstandslos");
`make schema-check` ohne Drift trotz der **ersten Repo-Schema-Änderung**
(`roofs.thickness_mm`, 023c — via gepinntem d-migrate, no-version-bump regelwerk-
konform). Coverage blieb hoch (92,7 % welle-3 → **90,7 %** welle-4 bei stark
gewachsenem, gut getestetem Codeumfang — die self-rolled Byte-Codecs tragen
voll-Decode-Orakel). Nichts aufzulösen, nichts nachzutragen.

| Carveout | Status vorher | Status nachher | Aktion |
|---|---|---|---|
| — | keine aktiven | keine aktiven | — (negativer Befund, belegt) |

## 5. Lerneinträge / Steering-Loop-Zähler

Fortschreibung aus [`welle-3-results.md` §5](welle-3-results.md) plus die in welle-4
akkumulierten Praxis-Zähler (Kurs-Regel: 2× kategorisieren, 3× Regel):

1. **„Option D (self-rolled Subset-Codec io-resident) für nicht-native Formate":
   Welle-Muster, 4× bestätigt.** IFC (0013), DXF (0015), PDF + PNG (0016) sind
   nicht-nativ (weder OCC- noch Bibliotheks-nativ ohne [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Berührung) → je ein
   **selbst getragener, header-freier Codec** in `adapters/io/`, isoliert durch
   arch-check **Regel A/B** („Regel F gegenstandslos"). **Gegenfolie STEP/STL** (0014):
   OCC-DataExchange **ist** nativ + geometrie-resident → dort war die native Nutzung
   richtig. Die Format-Residenz folgt der Nativität, nicht der Bequemlichkeit.
2. **„Voll-Decode-Orakel statt ‚nicht-leer' bei self-rolled Byte-Formaten":
   Welle-Muster.** Ein Selbst-Orakel, das nur die Autor-Sicht re-parst, ist blind für
   das, was ein **echter Reader** zum Öffnen braucht (PDF-Objektgraph/`MediaBox`;
   PNG-CRC/Adler/zlib-Header/Filterbytes). Konsequenz, mehrfach in Plan-/ADR-Reviews
   gefangen ([ADR-0016](../../adr/0016-pdf-png-backend.md)-Projektinhaber-MED; 025b-MED-1; 025c-MED-1/2/3): das Orakel
   **decodiert** (Objektgraph bzw. Inflate + eigenständige Prüfsummen), und der
   Code-Reviewer belegt die reale Reader-Öffenbarkeit **einmalig empirisch außerhalb
   des Gates** — **ohne** Reader-Test-Dependency ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Disziplin, wie bei
   poppler/libpng entschieden).
3. **„Sizing-Split an genuinen Risiko-/Umfangs-Gabeln": Welle-Muster.** 024a/b
   (Dächer-Vernähung vs. Treppen-Rekonstruktion), 025b/c (Vektor-PDF vs.
   Raster-Encoder-Risiko) wurden geschnitten, damit jede Review-Sitzung sinnvoll groß
   bleibt; **geteilte Helfer amortisieren** (`occ_solids`; `plan_geometry`/
   `io_atomic_write` — von 025b in 025c **unverändert** reused). ACC-Bindung leitet den
   Schnitt ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) hängt nur an PDF → PDF zuerst).
4. **„Vernähung für wasserdichte Netze, analytische Rekonstruktion für nicht-manifolde"
   (024): Geometrie-Muster.** Der wasserdichte Dach-Slab (023b) wird zu **einem**
   B-Rep-Solid vernäht (fail-closed via `BRepCheck_Analyzer`); die nicht-manifolde
   Treppen-Box-Union wird **analytisch** aus der geteilten Kern-Query rekonstruiert
   statt vernäht. Voraussetzung war die **Dach-Volumen-Initiative** (023): ein
   dicke-loses Display-Netz ist nicht vernähbar — erst [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006)
   (Dach = Volumenkörper) machte STEP-B-Rep möglich.
5. **„Kern-Architektur-Änderung nur wenn nötig, sonst additiv": 1× im ADR-Review
   gefangen.** DXF-**Import** brauchte die symmetrische `ImporterMap` (echte
   Kern-Erweiterung) — der [ADR-0015](../../adr/0015-dxf-backend.md)-Text-Review (MED-1) fing, dass der Import-Dispatch
   **kein** Registry war (Einzel-`switch`). PDF/PNG dagegen waren rein **additiv**
   (ExporterMap-Eintrag + Enum-Wert), da die Registry schon stand. „Erst prüfen, ob
   die Naht schon trägt" — nicht reflexartig erweitern.
6. **„Erste Repo-Schema-Änderung schema-treu + ungegründete Versions-Entscheidung
   gefangen" (023c).** `roofs.thickness_mm` kam über `data-model.yaml` + **gepinntes
   d-migrate** (nie hand-edieren; `schema-check` als Drift-Wächter); der [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
   fing **2 HIGH** (falsche Präzedenz + ungegründete Versions-Entscheidung) — die
   additive Spalte ist via SQL-Default abwärtskompatibel, **kein** Versions-Bump
   (releasing.md §27). Grünes Gate ≠ begründete Entscheidung.
7. **„Lint als Feedforward, nicht als Nachlauf": Welle-Muster.**
   `bugprone-easily-swappable-parameters` (feuert bei **separat gespeicherten** Params
   → `PdfPageSize`/`RasterSize`-Structs) und `readability-function-cognitive-complexity`
   (`main.cpp`-Export-if-Blöcke → **Tabelle+Schleife**; `projectPlan`-Innenschleifen →
   flaches `std::min/max`) wurden auf dem Weg behoben. Die empirische Grenze (Check
   feuert bei separaten, nicht bei *zusammen* genutzten Koordinaten-Params) wurde in
   025c gegen eine zu breite Reviewer-Annahme präzisiert.

**Welle-Lerneintrag:** Die vier Backend-ADRs (0013–0016) teilen **ein** Muster —
**Format-Residenz folgt der Nativität**: nicht-native Formate (IFC/DXF/PDF/PNG) als
header-freie self-rolled Option-D-Codecs io-resident (arch-check A/B, keine
Dependency, [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) unberührt); OCC-native (STEP/STL) geometrie-resident (Regel C).
Der Kern blieb über sechs Formate **format-frei** (nur `ExchangeFormat` + zwei
Registries); die einzige Architektur-Erweiterung (ImporterMap) wurde vom ADR-Review
begründet, nicht reflexartig gesetzt. Die self-rolled Byte-Formate zahlen ihre
Freiheit (keine Dependency) mit **voll-Decode-Orakeln** zurück — grünes Gate ≠
Reader-Öffenbarkeit, daher der empirische Cross-Check.

## 6. Nachfolge

- **M4 (Meilenstein „Offen austauschbar") erreicht** — alle sechs Austauschformate
  (IFC/DXF/STEP/STL/PDF/PNG) liefern; [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien) (IFC-Export-Roundtrip) +
  [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) (maßstäblicher PDF-Plan) erfüllt; Roadmap §Meilensteine mit dieser
  Closure auf **erreicht** gesetzt.
- **Nächste Welle (Planungs-Entscheidung, kein Automatismus):**
  `welle-5-erweiterung` — Plugin-System (`PLG`, [OBJ-004](../../../../spec/lastenheft.md#3-projektziele)), UI-Themes/Docking +
  **2D-Zeichen-Werkzeuge `DRW`** (aus welle-3 zurückgestellt), Mehrsprachigkeit
  ([LH-QA-006](../../../../spec/lastenheft.md#lh-qa-006--mehrsprachigkeit)). Trigger: welle-4 done + Plugin-API-/ABI-ADR.
- **Offen / Teilumfänge für später (alle als Re-Eval-Trigger benannt):**
  **Format-Reichtum** (IFC/DXF-Bibliothek für Blöcke/Bögen/mehr Bauteile;
  PDF-Fit-to-Page/Bemaßung/Textsatz; komprimiertes PNG/3D-Screenshot; DXF-Import
  Höhe/Dicke); **Wandtyp-Bibliothek**; **Dach-**/exakte Volumen-Semantik;
  **`cost_per_m2`**, **MAT-004**, **WAL-006-Vollumfang**; **slice-006**
  (Drittanbieter-Attribution) in `open/`; **`DRW`** (welle-5).
- **Steering offen:** der Header-Versions-Drift-Sensor ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)-Promotion) und der
  Persistenz-Code-Review-Promotion-Kandidat bleiben computational-Tooling-Kandidaten;
  der `io_atomic_write`-Cross-Adapter-Cleanup (IFC/DXF auf den geteilten Writer
  ziehen) + die `emitChunk`-Effizienz sind benannte Code-Cleanup-Kandidaten.
