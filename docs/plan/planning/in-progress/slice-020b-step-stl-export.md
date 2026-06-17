---
id: slice-020b
titel: STEP-/STL-Export — geometrie-residenter ModelExporterPort (OCC-DataExchange) + ExchangeFormat-Dispatch
status: in-progress
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 020b: STEP-/STL-Export (geometrie-residenter Exporter)

**Status:** in-progress (2026-06-17). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen
(**0 HIGH / 2 MED / 3 LOW / INFO**; implementierungs-bereit, Start nicht blockiert).
[Report](../../../reviews/2026-06-17-slice-020b-plan.md). Eingearbeitet: **MED-1**
(Decken/Fundament **sind** OCC-Solids — nur **Dächer + Treppen** analytisch → Vernähung;
+ §1-Präzisierungs-Folgepflicht), **MED-2** (`GeometryKernelPort` gibt kein `TopoDS_Shape`
→ Shape-Konstruktionslogik neu aufbauen, nicht „wiederverwenden"), **LOW-1**
(STEP-Fixture mit OCC-Solid-Bauteil; STL-only-Lücke asserted), **LOW-2**
(`importModel`-Switch-Arm für export-only `Step`/`Stl`). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) anwendbar** (Mesh→Shape-
Vernähung = neue Geometrie). Quellen-Behauptungen gegen den echten Geometrie-Adapter verifiziert.

**Welle:** welle-4-austausch (Implementierungs-Hälfte des STEP/STL-Strangs nach
slice-020a-Schärfung; Muster 019b/c für IFC).

**Bezug:** [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) (STEP) + [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006) (STL), AK in slice-020a;
Mechanik in `spezifikation.md` §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005). **Parametrisiert auf
[ADR-0014](../../adr/0014-step-stl-export-backend.md)** (OCC-DataExchange nativ, geometrie-residenter `ModelExporterPort`)
und [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (OCC im Geometrie-Adapter, Regel C) / [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt;
neutraler Wurf). **Kein neuer ADR.** **Toolchain-Beleg erbracht** (ADR-Index: OCC
7.9.2 `libTKDESTEP`/`libTKDESTL`/`libTKRWMesh` + Header vorhanden).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-17.

**Schnitt-Herkunft:** Impl-Hälfte des STEP/STL-Strangs (Muster 019c). Liefert den
**Export** ([LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)/006) — ein b-cad-Modell wird zu einer validen STEP-/
STL-Datei.

---

## 1. Ziel

Der **STEP-/STL-Export** wird lauffähig: ein `model::Building` wird über einen
**geometrie-residenten** `ModelExporterPort`-Adapter als **valide STEP-** (B-Rep)
und **STL-Datei** (Dreiecksnetz) geschrieben — **atomar**, über die
**OCC-DataExchange**-Module ([ADR-0014](../../adr/0014-step-stl-export-backend.md); §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005)). Der Exporter
lebt **ausschließlich** in `src/adapters/geometry/` (OCC nur dort — Regel C); der
Kern bleibt format-frei.

## 2. Definition of Done

- [ ] **`ExchangeFormat`-Erweiterung + `ExchangeService`-Dispatch (Kern).**
      `ExchangeFormat` um `Step`, `Stl` erweitert; `ExchangeService.exportModel`
      dispatcht je Format zum passenden `ModelExporterPort` — **IFC io-resident**
      (`IfcExportAdapter`, slice-019c), **STEP/STL geometrie-resident** (neuer
      Adapter). Mehr-Exporter-Verdrahtung (Format→Port) im Kern; **pure Domäne**
      (kein OCC-/STEP-Symbol im Kern, `arch-check` Regel A). **`importModel`-Switch
      (LOW-2):** die neuen `Step`/`Stl`-Enum-Werte sind **export-only** → der
      `importModel`-`switch` bekommt einen defensiven Arm (Wurf „nicht importierbar",
      kein `ModelImporterPort`), damit der Switch erschöpfend bleibt. `src/main.cpp`
      verdrahtet den geometrie-residenten Exporter (Composition Root;
      `--export-step`/`--export-stl`-Spiegel).
- [ ] **STEP/STL-Exporter in `src/adapters/geometry/` implementiert `ModelExporterPort`**
      (eigener Adapter/eigene Adapter je Format; OCC gekapselt — Regel C): **Building →
      OCC-`TopoDS_Shape`-Assemblierung** → STEP via `STEPControl_Writer` (AP214/242),
      STL via `StlAPI_Writer`/`RWStl` (binär). **Shape-Assemblierung (Kernpunkt, MED-1/2):**
      **Wände UND Decken/Fundament** entstehen über den `GeometryKernelPort` als OCC-Solids
      (Footprint-Extrusion + Boolean-Cuts) — die **Konstruktionslogik** (`makeNetSolid`)
      wird wiederverwendet/refaktoriert; der Port gibt heute nur `model::Solid{volume}`/
      `TriangleMesh` zurück, **kein** `TopoDS_Shape`, der Exporter **baut die Shapes neu auf**
      (MED-2). **Dächer und Treppen** sind **analytische Dreiecksnetze**
      (`roof_geometry`/`stair_geometry`, **kein** OCC-Solid, MED-1) → für STEP zu
      **OCC-Shapes vernäht** (`BRepBuilderAPI_Sewing` o. Ä., faceted Shell/Solid — **neue
      Geometrie-Konstruktion** → [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) ODER benannte STL-only-Lücke (s. §6); für STL
      fließen alle Bauteile direkt als Mesh ein. **Atomar** (Temp + Rename;
      nicht beschreibbarer Zielpfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), `event=io_no_permission`, kein
      Teil-Export); OCC-Fehler/degeneriertes Shape → **neutraler Wurf** (kein OCC-Typ-
      Leck, [ADR-0001](../../adr/0001-hexagonale-architektur.md)). OCC-DataExchange-Toolkits (`TKDESTEP`/`TKDESTL`/`TKRWMesh`)
      in `src/adapters/CMakeLists.txt` gelinkt (**keine** neue `find_package`-Dependency).
- [ ] **AK-Tests [`LH-FA-IO-005`](../../../../spec/lastenheft.md#lh-fa-io-005)/`006` + Adapter-Pfad-Integration.** **STEP:**
      Modell → Datei → **Re-Read-Orakel** (`STEPControl_Reader` liest die Datei wieder,
      die exportierten Bauteile sind als Volumenkörper enthalten — nicht leer). Die
      STEP-Fixture enthält mindestens ein **OCC-Solid-Bauteil** (Wand/Decke); wird der
      STL-only-Fallback für Dächer/Treppen gewählt, ist diese Lücke **asserted**
      (nicht stilles Leer) — LOW-1. **STL:**
      Datei trägt ein nicht-leeres Dreiecksnetz (erwartete Dreiecks-/Shape-Anzahl > 0).
      **Boundary:** 3D-leeres Modell → gültige, (annähernd) leere Datei ohne Wurf.
      **Negative:** nicht beschreibbarer Zielpfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten
      Adapter (Temp-als-nicht-leeres-Verzeichnis, root-fest — Muster slice-019c).
      **Integration über den echten Pfad** `ExchangeService.exportModel` →
      `ModelExporterPort` → geometrie-residenter Adapter. `make gates` grün (arch-check
      inkl. geometry, Coverage ≥ 70 %).
- [ ] **Schicht + Geometrie-Review.** `arch-check` **Regel C** deckt den Exporter
      (OCC in `geometry/`) — **keine** neue arch-check-Regel ([ADR-0014](../../adr/0014-step-stl-export-backend.md)). **Unabhängiges
      Code-Review** (Mapping/Serialisierung/[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Korrektheit). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
      anwendbar** (Geometrie-Assemblierung im Adapter — Mesh→Shape-Vernähung ist neue
      Geometrie-Konstruktion): das Review prüft **geometrische Korrektheit** der
      assemblierten/geschriebenen Shapes (Watertightness/Orientierung der vernähten
      Netze, Vollständigkeit der Bauteile). **Nicht Teil:** DXF/PDF/PNG (eigene ADRs);
      Welle-Verifikation.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/exchange_model_port.{h}` | ändern | `ExchangeFormat` um `Step`/`Stl` |
| `src/hexagon/services/exchange_service.{h,cpp}` | ändern | Dispatch Step/Stl → geometrie-residenter Exporter (Mehr-Exporter) |
| `src/adapters/geometry/step_stl_export_adapter.{h,cpp}` | neu | `ModelExporterPort`-Impl (Building→OCC-Shape→STEP/STL, atomar) |
| `src/adapters/geometry/building_shapes.{h,cpp}` | neu (ggf.) | Building→`TopoDS_Shape`-Assemblierung (Wände-Solids + Mesh→Shape-Vernähung), von der Format-Schreibung trennbar |
| `src/adapters/CMakeLists.txt` | ändern | OCC-DataExchange-Toolkits linken (`TKDESTEP`/`TKDESTL`/`TKRWMesh`) — keine neue Dependency |
| `src/main.cpp` | ändern | geometrie-residenten Exporter verdrahten (`--export-step`/`--export-stl`) |
| `tests/adapters/test_step_stl_export.cpp` | neu | AK (STEP-Re-Read / STL-Mesh) + [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) + Integration |
| `tests/CMakeLists.txt` | ändern | Test registrieren |
| `spec/spezifikation.md` | ggf. ändern | §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005) präzisieren: Dächer/Treppen B-Rep via Vernähung **oder** STL-only-Lücke (MED-1); + `spezifikation-historie.md` |
| `docs/reviews/{…-slice-020b-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **Startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: AK (020a) + Backend ([ADR-0014](../../adr/0014-step-stl-export-backend.md)) + Toolchain-Beleg
  liegen vor; `ModelExporterPort`/`ExchangeService` existieren (slice-019c).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Code-Review + [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH, Closure-Notiz.
  Damit liefert welle-4 STEP+STL; offen für M4: DXF · PDF/PNG · Welle-Verifikation.

## 6. Risiken und offene Punkte

- **STEP-B-Rep-Abdeckung (Kern-Risiko, geometrieschwer; MED-1):** **Wände und
  Decken/Fundament** haben echte OCC-Solids (`GeometryKernelPort`-Footprint-Extrusion +
  Boolean-Cuts); **nur Dächer und Treppen** sind analytische Dreiecksnetze (kein
  OCC-Solid). Für STEP (B-Rep) müssen Dächer/Treppen zu OCC-Shapes **vernäht** werden
  (faceted Shell/Solid) — neue Geometrie-Konstruktion → **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)** (Watertightness/
  Orientierung). **Fallback (benannte Lücke):** STEP exportiert zunächst nur die
  OCC-Solid-Bauteile (Wände + Decken/Fundament), Dächer/Treppen als faceted Shapes
  oder vorerst **STL-only** — im [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)/Impl entscheiden, **ehrlich benennen** (kein
  stiller Teilumfang).
- **§1-Präzisierung (Folgepflicht, MED-1):** das committete §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005)
  liest STEP-B-Rep „wie der Geometrie-Adapter sie baut" für alle 3D-Bauteile — das
  stimmt für Wände/Decken, **nicht** für Dächer/Treppen (analytisch). Der Impl-Slice
  **präzisiert** §1 (Dächer/Treppen via Vernähung ODER STL-only-Lücke) statt die
  Imprecision stillschweigend zu übernehmen; die 020a-AK „3D-Bauteile" ist daran zu
  messen (Spec, nicht Lastenheft — [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).
- **Mehr-Exporter-Dispatch:** `ExchangeService` hält bisher genau einen Exporter
  (`ifc_exporter_`, io-resident). STEP/STL braucht einen zweiten (geometrie-resident);
  Format→Port-Zuordnung (zwei Refs oder Map) im Service — additive Erweiterung, keine
  Port-Semantik-Änderung ([ADR-0014](../../adr/0014-step-stl-export-backend.md) belegt das).
- **OCC-Typ-Leck (Regel A/C):** der Exporter nutzt OCC direkt (erlaubt in `geometry/`),
  darf aber **keinen** OCC-Typ über `ModelExporterPort` lecken (neutraler Wurf, Muster
  `OccGeometryAdapter`/[`E-GEO-002`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)).
- **STEP-Schema (AP214 vs. AP242):** im Impl wählen + im Re-Read-Test fixieren;
  Material/PMI/Assemblies bleiben benannte Lücke ([ADR-0014](../../adr/0014-step-stl-export-backend.md) Re-Eval).
- **Slice-Größe:** Format-Erweiterung + Service-Dispatch + Adapter (Shape-Assemblierung
  + zwei Writer) + Tests — ggf. **Zwei-Commit-Split** (i Building→Shape-Assemblierung +
  Writer, ii Service-Dispatch + Adapter-Verdrahtung + Tests; Muster 019b/c).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Code / Geometrie-Adapter

- **Modus:** GF; **Konventionen-Dichte:** hoch — neuer Driven-`ModelExporterPort` im
  **Geometrie**-Adapter (OCC gekapselt, Regel C; neutraler Wurf); Atomarik aus der
  Persistenz/019c übernommen. **Phase-Reife:** Phase 4 (Geometrie-Adapter etabliert).
  **Evidenz-/Diskrepanz-Risiko:** mittel — die **Mesh→Shape-Vernähung** ist neue
  Geometrie ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)). **Reconciliation:** keine; Folge = DXF/PDF/PNG.

### Sub-Area: Test-Infrastruktur

- **Modus:** GF; **Dichte:** hoch — GoogleTest + `LH-FA-*`-AK-Tests + Re-Read-Orakel
  (`STEPControl_Reader`) + Integrationstest-Muster (019c). **Phase-Reife:** Phase 4.
  **Risiko:** niedrig.

## 8. Closure-Notiz

*(wird bei `done` ausgefüllt — Closure-Kriterien beobachtbar, Lerneintrag in einer
der drei Formen: geschärfte Regel · neuer Sensor · benannte Spec-Lücke.)*
