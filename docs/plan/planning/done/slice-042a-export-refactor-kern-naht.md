---
id: slice-042a
titel: Export-Refactor Kern-Naht — DerivedGeometry-Vertrag + StepBox/translateMeshZ → model/
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 042a: Export-Refactor Kern-Naht — DerivedGeometry-Vertrag

**Status:** done (2026-07-23 — Impl geliefert, `make gates` grün [248 Tests, Coverage 90,7 %], Export-Orakel unverändert grün = Invarianz-Beweis). **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review** 0 HIGH / 1 MED / 2 LOW → alle eingearbeitet ([Report](../../../reviews/2026-07-23-slice-042a-plan.md)).

**Welle:** welle-5-erweiterung. **Erster** der fünf [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices
([ADR-Index](../../adr/README.md), Familie 042a…e): **Kern-Naht/Vertrag** → 2D-Projektion → STEP/STL →
Persistenz → `.a-check.yml`+`architecture.md`-Abschluss.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (Accepted —
driven Adapter serialisieren, der Kern liefert abgeleitete Geometrie als pures `DerivedGeometry`-Bündel
über den Exporter-Port; `StepBox` + `translateMeshZ` → `model/`; alle Adapter→Kernel-Kanten entfallen
**über die Folge-Slices**), [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern-Reinheit, Ziel-Bündel
lib-frei), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (OCC-Montage bleibt im Adapter — kein
`TopoDS_Shape` über den Port), [ADR-0014](../../adr/0014-step-stl-export-backend.md) (STEP/STL geometrie-resident).

---

## 1. Ziel

Die **Naht** legen, über die der Kern abgeleitete Geometrie an die Exporter reicht, **ohne** schon
Verhalten zu ändern. Konkret: die zwei puren Werte, die [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)
nach `model/` hebt, umziehen; den Bündel-Typ einführen; den `ModelExporterPort::write`-Vertrag um das
Bündel weiten (**accept-and-ignore** über alle sechs Exporter); und den `ExchangeService` (vorerst) ein
**leeres** `DerivedGeometry` **durchreichen** lassen. Die **format-selektive Berechnung** der Ableitung
kommt **erst mit Slice 042c** (STEP/STL), wo die Bodies das Bündel konsumieren und die B-Rep-/STL-Orakel
sie **gleichzeitig verifizieren** — statt einer un-genetzten neuen Wurf-Fläche über die adapter-internen
fail-closed-Pfade ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1).
**Kein** Export-Adapter konsumiert das Bündel in diesem Slice; **keine** `.a-check.yml`-Kante wird entfernt
(das tun die Folge-Slices bei der Body-Migration). Ergebnis: ein **rein mechanischer, verhaltens-invarianter**
Umzug — die Export-Artefakte bleiben byte-/struktur-identisch.

## 2. Definition of Done

- [x] **`model::StepBox`.** `StepBox` (heute `services::StepBox`, `src/hexagon/services/geometry/stair_geometry.h:27`,
      reiner 6-`double`-POD) → neuer header-only `src/hexagon/model/{step_box}.h` (Namespace `bcad::hexagon::model`,
      `#pragma once`, Muster der übrigen `model/`-PODs). Alle Verwender nachziehen: `stair_geometry.h/.cpp`
      (`stairStepBoxes`-Rückgabe), `step_export_adapter.cpp`, `tests/hexagon/test_stair_geometry.cpp`.
      `stair_geometry` importiert dann `model::StepBox` (Richtung `services_geo → model`, `.a-check.yml:25` — legal).
- [x] **`model::translateMeshZ`** ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)
      HIGH-1). Die pure `TriangleMesh`-z-Verschiebung (heute `services::translateMeshZ`,
      `src/hexagon/services/geometry/slab_geometry.{h:46,cpp:96}`) → header-only `model/`-Util (z. B.
      `src/hexagon/model/{mesh_ops}.h`, frei von `services/geometry`, adapter-erreichbar). Verwender nachziehen:
      `structure_edit_service.cpp:817`, `stl_export_adapter.cpp:74`, `tests/hexagon/test_slab_geometry.cpp:101`.
      Reine Wert-Operation, kein Modell-Ableiten — der STL-Adapter darf sie auf sein **eigenes** Mesh anwenden.
- [x] **`model::DerivedGeometry`-Bündel.** Neuer header-only `src/hexagon/model/{derived_geometry}.h`: ein pures
      Werttyp-Aggregat, **format-selektiv** befüllbar, das per Bauteil die Primitive trägt, die die Exporter
      künftig konsumieren — Wand `{Footprint, height_mm, CutPrism[]}`, Decke `{Footprint, thickness_mm,
      CutPrism[], baseZ}`, Dach `TriangleMesh`, Treppe `{StepBox[], TriangleMesh, rise_mm}` (die 2D-`PlanView`
      kommt **erst** mit dem 2D-Projektions-Slice hinzu — hier noch nicht). Nur `model/`-Typen + Standardbibliothek
      (lib-frei, Kern-Link-Barriere). Genaue Feld-/Struct-Gestalt legt die Impl fest (die AK bindet nur
      Lib-Freiheit + Verhaltens-Invarianz).
- [x] **`ModelExporterPort::write`-Vertrag geweitet.** `write(const model::Building&, const model::DerivedGeometry&,
      const std::filesystem::path&) const` (`src/hexagon/ports/driven/model_exporter_port.h:26`). **Alle sechs**
      Exporter-Overrides (IFC/DXF/STEP/STL/PDF/PNG) nehmen den Parameter an und **ignorieren ihn** (Body
      unverändert — accept-and-ignore, [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)
      LOW-3). STEP/STL rufen `services/geometry` in **diesem** Slice **weiter direkt** (Body-Migration = Slice 042c).
- [x] **`ExchangeService` reicht ein (leeres) Bündel durch.** `exportModel` (`src/hexagon/services/exchange_service.cpp:33`)
      konstruiert ein **leeres** `model::DerivedGeometry` und reicht es an `write(...)`. Die **format-selektive
      Berechnung** (per-Format-Verzweigung; STEP/STL → extrudierbare Primitive + Treppen, IFC/DXF/PDF/PNG → leer)
      wandert nach **Slice 042c** — dort konsumieren STEP/STL das Bündel und die B-Rep-/STL-Orakel netzen die
      Berechnung ab ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1:
      Berechnung ohne Konsum = null Prüfgewinn bei realer, un-genetzter Wurf-Fläche über die adapter-internen
      fail-closed try/catch-skip + `kDefaultStoreyHeightMm`-Pfade). Composition-Root (`main.cpp`) **unverändert** —
      der Driving-Port `ExchangeModelPort::exportModel` ändert die Signatur **nicht**, nur der Driven-Port `write`
      wächst; ExporterMap-Verdrahtung bleibt.
- [x] **Verhaltens-Invarianz maschinell.** `make gates` grün; **die Export-Decode-/Round-Trip-Orakel bleiben
      unverändert grün** (STEP `CLOSED_SHELL`-Zahl, STL-Netz, IFC/DXF-Round-Trip, PDF/PNG-Decode) — der Beweis,
      dass der Umzug nichts am Artefakt ändert. `make schema-check` unberührt. Kein neuer Test-**Bedarf** außer der
      Anpassung bestehender Tests an die umgezogenen Typen/Signaturen; ein **kleiner** Kern-Test für `DerivedGeometry`
      (Aggregat baubar, lib-frei) ist zulässig.
- [x] **Doku.** `spec/spezifikation.md` §1 (Export-Datenfluss-Umfeld) um die Kern-Naht
      (Kern berechnet, Adapter serialisiert) — **ADR-/slice-token-frei** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md),
      vor Gate greppen); `spezifikation-historie.md` + Header. **architecture §2-Tabelle NICHT** hier (die zieht
      erst Slice 042e, wenn die Kanten real fallen — Byte-Ehrlichkeit). [ADR-Index](../../adr/README.md) Zeile
      [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Kern-Naht-Zeile auf »erfüllt« ziehen; **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
      Closure-Notiz.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{step_box}.h` | neu | `StepBox`-POD nach `model/` ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)) |
| `src/hexagon/model/{mesh_ops}.h` | neu | `translateMeshZ` als `model/`-Util (HIGH-1) |
| `src/hexagon/model/{derived_geometry}.h` | neu | pures Bündel-Aggregat (format-selektiv) |
| `src/hexagon/services/geometry/stair_geometry.{h,cpp}` | ändern | `StepBox`-Umzug nachziehen (Include `model/step_box.h`) |
| `src/hexagon/services/geometry/slab_geometry.{h,cpp}` | ändern | `translateMeshZ`-Umzug nachziehen |
| `src/hexagon/services/structure_edit_service.cpp` | ändern | `translateMeshZ`-Aufruf auf `model/`-Util |
| `src/hexagon/ports/driven/model_exporter_port.h` | ändern | `write`-Signatur um `DerivedGeometry` |
| `src/adapters/io/{ifc,dxf,pdf,png}_export_adapter.{h,cpp}` | ändern | `write`-Override akzeptiert Bündel (ignoriert) |
| `src/adapters/geometry/{step,stl}_export_adapter.{h,cpp}` | ändern | `write`-Override akzeptiert Bündel (ignoriert); `StepBox`/`translateMeshZ`-Include nachziehen |
| `src/hexagon/services/exchange_service.{h,cpp}` | ändern | leeres `DerivedGeometry` konstruieren + an `write` reichen (Berechnung → 042c) |
| `tests/hexagon/test_stair_geometry.cpp`, `test_slab_geometry.cpp` | ändern | umgezogene Typen/Util |
| `tests/hexagon/{test_derived_geometry}.cpp` | neu (klein) | Aggregat baubar, lib-frei |
| `tests/CMakeLists.txt` | ändern | neuen Kern-Test registrieren (explizit gelistet, nicht geglobbt) — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-1 |
| `tests/adapters/*` (Export-Aufrufe) | ändern | `write`-Aufrufe an neue Signatur (Orakel-Inhalt UNVERÄNDERT) |
| `spec/spezifikation.md` + `-historie.md` | ändern | Kern-Naht-Datenfluss (token-frei) |
| [ADR-Index](../../adr/README.md), `CHANGELOG.md` | ändern | Folgepflicht »erfüllt« + Eintrag |

**Bewusst NICHT Teil (Folge-Slices):** 2D-`PlanView` ins Bündel + `plan_geometry`→Kern + `PlanViewPort`
(042b, **entsperrt Canvas**); STEP/STL-Body-Migration auf das Bündel + **`geometry → services_geo`-Kante raus**
(042c); Persistenz-`rise` kern-seitig + **`persistence → services_geo`-Kante raus** (042d); `.a-check.yml`-Kanten-
Entfernung final + `architecture.md` §2-Tabelle/§1-Diagramm wahr (042e).

## 4. Trigger

- **[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) Accepted** (2026-07-23) +
  [ADR-0019](../../adr/0019-drw-2d-canvas.md) Accepted; slice-041a done. Die Folgepflicht-Sequenz ist frei.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
  vor Start** (Reviewer ≠ Autor) — steht aus; HIGH blockiert. Geometrie-berührend → **Code-Review vor
  Welle-Closure** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (**Export-Orakel unverändert grün** = Invarianz-Beweis), Closure-Notiz →
  **Slice 042b (2D-Projektion)** wird startbar.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — ExchangeService-Berechnung ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1, AUFGELÖST via Deferral):**
  Das Review zeigte: die Berechnung im `ExchangeService`, **während** STEP/STL ihre eigenen `services/geometry`-
  Aufrufe noch fahren, brächte **null Prüfgewinn** (Bündel ignoriert, kein Orakel prüft es) bei **realem**
  Invarianz-Risiko — die Adapter kapseln jeden Bauteil-Aufruf in fail-closed try/catch-skip +
  `kDefaultStoreyHeightMm`-Fallback (`step_export_adapter.cpp:41`, `:62`; `stl_export_adapter.cpp:36`, `:52`);
  eine nicht-total reproduzierte Berechnung würfe, wo STEP/STL heute exportieren, und **kein** bestehendes Orakel
  deckt danglenden `from_storey_id`/degeneriertes Bauteil. **Auflösung: die Berechnung wandert nach 042c**
  (Konsum + B-Rep-/STL-Orakel netzen sie gleichzeitig); 042a bleibt rein mechanisch (Typen + Vertrag + leeres
  Bündel). 042a weicht damit von der **wörtlichen** [ADR-Index](../../adr/README.md)-Zeile »ExchangeService
  berechnet…« ab (mutabler Status-Tracker, der Schnitt ist eine Planungswahl), **nicht** von der ADR-Entscheidung.
- **Rest-Risiko #2 — Signatur-Änderung berührt alle sechs Exporter + alle Export-Tests auf einmal** ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)
  LOW-3). Mitigation: mechanischer accept-and-ignore-Durchgang; die Orakel-**Inhalte** (erwartete Bytes/Struktur)
  bleiben unangetastet — nur die `write`-Aufruf-Arität ändert sich. Reißt ein Orakel, ist der Umzug nicht
  invariant → Stopp.
- **Rest-Risiko #3 — `DerivedGeometry`-Feld-Gestalt.** Zu breit/zu eng geschnitten erzwingt Nacharbeit in 042c.
  Mitigation: an den **realen** STEP/STL-Konsumdaten der Kartierung ausgerichtet (Wand/Decke-Primitive, Dach-Mesh,
  Treppe Boxen+Mesh+rise); `PlanView` bewusst später. Kein `TopoDS_Shape`, keine Library (Fitness).
- **042c-Vormerkung ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-2):**
  die totale Höhen-Auflösung `storeyHeight(...)→kDefaultStoreyHeightMm` liegt heute als private Kopie in **beiden**
  Geometrie-Adaptern (`step`/`stl`); zieht 042c die Berechnung in den `ExchangeService`, einen **geteilten
  totalen** Höhen-Helfer vorsehen (oder Storey-Höhe als Bündel-Input) statt einer dritten Kopie — eine Wahrheit,
  weniger Divergenz-Risiko (senkt zugleich das MED-1-Invarianz-Risiko).
- **Kern-Reinheit:** `model::DerivedGeometry`/`StepBox`/`mesh_ops` ziehen **keine** Library — `bcad_hexagon`
  bleibt dep-frei (Link-Barriere, [ADR-0001](../../adr/0001-hexagonale-architektur.md)); kein `TopoDS_Shape`
  im Kern/Port ([ADR-0002](../../adr/0002-geometrie-kern-opencascade.md)). Prüfung: `make build` + `make a-check`.
- **Spec-Straten token-frei** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)):
  §1/§Historie ohne ADR-/Slice-Token im Körper — vor dem Gate greppen (Lerneintrag 032a/c/041a).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Kern + Adapter-Code `src/`

- **Modus:** GF; **Dichte:** hoch (Port-Vertrag berührt alle sechs Exporter + `ExchangeService`; Typ-Umzüge mit
  Verwender-Nachzug). **Phase-Reife:** welle-5 Refactor. **Risiko:** mittel — breite, aber mechanische
  Signatur-Änderung; die Invarianz-Orakel sind das Netz.

### Sub-Area: Spec/Doku `spec/` + `docs/plan/`

- **Modus:** GF; **Dichte:** mittel (Datenfluss-Nachzug token-frei; ADR-Index + CHANGELOG). **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure 2026-07-23.** `make gates` grün (248/248 Tests, Coverage 90,7 %); die Export-Decode-/Round-Trip-
Orakel (STEP/STL/IFC/DXF/PDF/PNG) **unverändert grün** = Invarianz-Beweis. `make schema-check` unberührt.

- **Umgezogene Typen (verhaltens-invariant):** `services::StepBox` → `model::StepBox` (POD, header-only);
  `services::translateMeshZ` → `model::translateMeshZ` (inline `model/mesh_ops.h`, [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)
  HIGH-1) — beide adapter-erreichbar ohne `services/geometry`-Import; Verwender (`stair_geometry`,
  `slab_geometry`, `structure_edit_service`, `step`/`stl`-Export, Tests) nachgezogen.
- **Naht:** `model::DerivedGeometry` (pures lib-freies Bündel-Aggregat, `model/derived_geometry.h`);
  `ModelExporterPort::write(Building, DerivedGeometry, path)` — **alle 6 Exporter** accept-and-ignore;
  `ExchangeService` reicht ein **leeres** Bündel. `main.cpp` unverändert (`ExchangeModelPort::exportModel`
  unberührt). **Keine** `.a-check.yml`-Kante entfernt (`geometry`/`persistence → services_geo` bleiben →
  Folge-Slices 042c/042d/042e).
- **Doku:** Spec §1 STEP/STL-„Schicht" um die Geometrie-Bereitstellung (Kern-Naht, staged, token-frei)
  + `spezifikation-historie`; CHANGELOG; ADR-Index Kern-Naht-Zeile „erfüllt". architecture §2/§1 **unberührt**
  (zieht slice-042e — die Kanten fallen erst dort).
- **Reviews:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  0 HIGH / 1 MED / 2 LOW → alle eingearbeitet ([Report](../../../reviews/2026-07-23-slice-042a-plan.md)).
  Geometrie-berührend → Code-Review vor Welle-Closure ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) offen.
- **Lerneintrag (MED-1):** Eine Berechnung, die niemand konsumiert, bringt **null Verifikations-Gewinn** bei
  **realer** neuer Wurf-Fläche (die STEP/STL-Adapter kapseln jeden Geometrie-Aufruf fail-closed +
  `kDefaultStoreyHeightMm`) — daher wanderte die `ExchangeService`-Berechnung nach 042c, wo Konsum +
  B-Rep-/STL-Orakel sie **gleichzeitig netzen**. 042a blieb rein mechanisch.
- **Folge:** **slice-042b** (2D-Projektion → Kern + `PlanViewPort`, entsperrt Canvas; Skelett in `open/`,
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) beim Start) — [MR-020](../../../../harness/conventions.md) (Closure-Disziplin) erfüllt: die nächste
  Folge-Slice existiert als Plan-Datei.
