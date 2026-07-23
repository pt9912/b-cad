---
id: slice-042c
titel: Export-Refactor STEP/STL — Body-Migration auf das Bündel + geometry→services_geo-Kante raus
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 042c: Export-Refactor STEP/STL — Body-Migration auf das `DerivedGeometry`-Bündel

**Status:** done (2026-07-23 — Impl geliefert, `make gates` grün [252 Tests, Coverage 91,5 %], STEP-B-Rep-/STL-Netz-Orakel unverändert grün = Invarianz-Beweis, `make a-check` grün nach Kanten-Entfernung). **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)** 0 HIGH / 2 MED / 2 LOW ([Plan-Report](../../../reviews/2026-07-23-slice-042c-plan.md)) + **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) Code-Review** 0 HIGH ([Report](../../../reviews/2026-07-23-slice-042c-code-review.md)).

**Welle:** welle-5-erweiterung. **Dritter** der fünf [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices
(Familie 042a…e). **Vorgänger:** slice-042b (done). **Hier wandert die aus slice-042a per
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1
verschobene `ExchangeService`-Berechnung hin** — jetzt **konsumiert + von den B-Rep-/STL-Orakeln verifiziert**.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (driven Adapter
serialisieren; Kern liefert die Ableitung im Bündel; **OCC-Montage bleibt im Adapter** — pre-OCC-Primitive,
kein `TopoDS` über den Port), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (OCC nur im
Geometrie-Adapter, Regel C), [ADR-0014](../../adr/0014-step-stl-export-backend.md) (STEP/STL geometrie-resident).

---

## 1. Ziel

Die STEP/STL-Export-Bodies auf das kern-gelieferte `DerivedGeometry`-Bündel umstellen: der `ExchangeService`
**berechnet** die format-relevante Ableitung (Wand/Decke/Dach/Treppe als pre-OCC-Primitive) format-selektiv für
STEP/STL; die Adapter **iterieren das Bündel** und bauen daraus das B-Rep (STEP: `occ_solids`) bzw. das Netz
(STL: `GeometryKernelPort`). Danach fällt die `.a-check.yml`-Kante **`geometry → services_geo`** (kein
Geometrie-Adapter ruft mehr `services/geometry`). **Rein mechanischer, verhaltens-invarianter Umzug** — die
STEP-B-Rep-`CLOSED_SHELL`-Zählung + das binäre STL-Netz-Orakel bleiben byte-/struktur-identisch grün.

**Wichtige Nuance (Naht-Grenze):** die **pure Ableitung** (`wallFootprint`/`wallCutPrisms`/`slabCutPrisms`/
`slabBaseZ`/`roofMesh`/`stairStepBoxes`/`stairMesh` — allesamt **total**, werfen nie) wandert in den
`ExchangeService`. Der **fail-closed `try/catch{continue}`-Skip je Bauteil bleibt im Adapter** — er umschließt
die **OCC-/Tessellations-Konstruktion** (`makeNetSolid`/`meshToSolid`/`makeBoxSolid`/`tessellateFootprint`), die
per Regel C adapter-resident ist und **nicht** in den lib-freien Kern kann. Der Adapter iteriert also die (schon
abgeleiteten) `derived.walls/slabs/roofs/stairs` und behält seinen per-Bauteil-OCC-Skip. Weil die Ableitung
total ist, ist ihr Umzug aus dem `try` heraus **verhaltens-neutral** (sie warf nie); der **Negativ-Test**
(danglender `from_storey_id`/degeneriertes Bauteil → `exportModel` wirft **nicht**) ist das Netz dafür.

## 2. Definition of Done

- [x] **`ExchangeService` berechnet das STEP/STL-Bündel.** In `exportModel` ein format-selektiver Zweig für
      `Step`/`Stl` (parallel zum 042b-`Pdf`/`Png`-Zweig): pro `building.wall`/`slab`/`roof`/`stair` einen
      `DerivedWall`/`DerivedSlab`/`DerivedRoof`/`DerivedStair` befüllen (`wallFootprint`+`height_mm`+`wallCutPrisms`;
      `slab.footprint`+`thickness_mm`+`slabCutPrisms`+`slabBaseZ`; `roofMesh`; `stairStepBoxes` **und** `stairMesh`
      + `rise` (letzteres 042d-Vorrat, hier optional)). **Ein-Eintrag-pro-Bauteil in Modell-Reihenfolge** (die
      Compound-/Netz-Reihenfolge deterministisch halten). services/geometry-Includes im `ExchangeService`
      ergänzen. `services → services_geo` deckt den Aufruf (`.a-check.yml`) — **keine** neue Kante.
- [x] **`storeyHeight` konsolidiert (eine Kopie).** Die totale Höhen-Auflösung (`→ kDefaultStoreyHeightMm`) liegt
      heute als **identische** private Kopie in **beiden** Geometrie-Adaptern. Da nach der Migration **nur** der
      `ExchangeService` sie braucht (die Adapter konsumieren die pre-resolved `baseZ`/`boxes`/`mesh`), wird sie in
      den `ExchangeService` gezogen (private Helfer-Funktion) und die **zwei Adapter-Kopien entfernt** —
      **eine** Wahrheit (löst [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-042a-LOW-2;
      Alternative: geteilter `model/`-Helfer, falls ein zweiter Konsument absehbar — Review entscheidet).
- [x] **STEP-Adapter auf das Bündel** (`step_export_adapter.cpp`). Der `buildSolidCompound`-Bau iteriert
      `derived.walls`→`makeNetSolid(dw.footprint, dw.height_mm, dw.cutPrisms)`, `derived.slabs`→`makeNetSolid` +
      OCC-Lift um `ds.baseZ_mm`, `derived.roofs`→`meshToSolid(dr.mesh)`, `derived.stairs`→`makeBoxSolid` je
      `StepBox`. **Der per-Bauteil-`try/catch{continue}` + `IsNull`-Guards bleiben** (OCC-Skip). services/geometry-
      Includes + die `storeyHeight`-Kopie **entfernt**; `occ_solids`-Montage + `STEPControl_Writer` + atomarer
      Write unverändert.
- [x] **STL-Adapter auf das Bündel** (`stl_export_adapter.cpp`). Die `append*Meshes`-Sammler iterieren
      `derived.*`: Wände/Slabs → `GeometryKernelPort::tessellateFootprint(footprint, height/thickness, cutPrisms)`
      (+ `model::translateMeshZ(mesh, ds.baseZ_mm)` für Slabs), Dächer → `dr.mesh`, Treppen → `ds.mesh`. **Der
      `try/catch{continue}` um `tessellateFootprint` bleibt** (OCC-Skip). services/geometry-Includes + `storeyHeight`
      **entfernt**; `GeometryKernelPort`-Injektion + `buildStl` + atomarer Write unverändert.
- [x] **`.a-check.yml`-Kante `geometry → services_geo` entfernt.** Nach der Migration ruft **kein**
      Geometrie-Adapter mehr `services/geometry` (step/stl waren die einzigen Nutzer; `occ_geometry_adapter` nie;
      `occ_solids` ist intra-`geometry`). **Gegenprobe:** `make a-check` grün **nach** der Entfernung; ein
      Rest-`services/geometry`-Aufruf im geometry-Layer würde jetzt **failen**. `persistence → services_geo`
      **bleibt** (042d).
- [x] **Verhaltens-Invarianz — das Netz muss die neue SERVICE-Berechnung treffen ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1).**
      `make gates` grün; die bestehende **STEP-B-Rep-`CLOSED_SHELL`-Zählung** (Wände/Dach `+1`/Treppe `+step_count`)
      + das **binäre STL-Netz-Orakel** (`84 + 50·triangles`) bleiben **unverändert grün** — **aber sie prüfen nur
      die Adapter-Serialisierung eines vorgefertigten Bündels**, nicht die format-selektive `ExchangeService`-
      Berechnung (die vier Bauteil-Loops + Storey-Auflösung = der Löwenanteil des neuen Codes). **Darum
      verbindlich neu: ein Integrationstest über den ECHTEN `service.exportModel(…, Step/Stl)` mit einem
      VOLL-Modell (Wände + Slab + Dach + Treppe)** und den **starken** strukturellen Orakeln
      (STEP: `CLOSED_SHELL == wandShells + 1 [Dach] + step_count [Treppe]`; STL: **exakte** Dreieckszahl) — nur so
      verifiziert das Netz, dass der Service **jedes** Bauteil mit den **richtigen** Parametern ableitet (fängt:
      vergessenes `derived.stairs`/`slabs`/`roofs`, `s.storey_id` statt `s.from_storey_id`, weggelassene
      `wallCutPrisms`). **Alternative:** `writeStep`/`writeStl` das Bündel über eine reale `ExchangeService`-Instanz
      beziehen lassen (dann fließen die starken Direkt-Orakel durch den Service).
- [x] **Koordinaten-Sonde ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-1).**
      Die Zähl-Orakel sind **positions-blind** (ein falsches `slabBaseZ`/`storeyHeight`/OCC-Lift verschiebt in Z,
      ohne die Zahl zu ändern) — und 042c **verlagert genau diese `baseZ`/`storeyHeight`-Rechnung in den Service**.
      In mindestens einem migrierten Fall eine **Extent-/Vertex-Sonde** (bekannte Z-Ausdehnung bzw. Vertex-Koordinate)
      ergänzen.
- [x] **Negativ-Test — beide Pfade ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-2, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-042a+042b-INFO-1).**
      `exportModel` (STEP **und** STL) wirft **nicht** bei (a) **danglendem `from_storey_id`** (→ `storeyHeight`-
      Fallback `kDefaultStoreyHeightMm`; Bauteil wird **exportiert**) **und** (b) **degeneriertem Bauteil**
      (`step_count=0`/`width=0` → leere Ableitung → **OCC-Skip-Pfad** im Adapter). STEP/STL erhalten ein
      **befülltes**, IFC/DXF ein **leeres** Bündel.
- [x] **Test-Aufrufer-Migration.** Die ~10 direkten `exporter.write(building, DerivedGeometry{}, path)`-Aufrufe in
      `test_step_stl_export.cpp` (leeres Bündel → nach Migration **kein Body**) auf **`writeStep`/`writeStl`-Helfer**
      umstellen (Muster 042b `writePdf`: das Bündel aus **derselben** services-Quelle befüllen → **Byte-Identität**
      geprüft, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-042b-LOW-2).
      Die Integrationstests (`service.exportModel`) laufen unverändert.
- [x] **Doku.** `spec/spezifikation.md` §1 STEP/STL-„Geometrie-Bereitstellung" von „staged" auf **realisiert**
      nachziehen (token-frei, [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)
      vor Gate greppen) + `spezifikation-historie`; [ADR-Index](../../adr/README.md) STEP/STL-Zeile „erfüllt";
      **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
      **`architecture.md` §2 geometry-Adapter-Zeile: `services/geometry` entfernen** (Option A, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2
      — architecture bleibt schritt-genau konsistent mit `.a-check.yml`; die persistence-Zeile + das §1-Diagramm
      bleiben 042d/042e). **[ADR-Index](../../adr/README.md) reconcilen:** 042c = geometry-Kante **+** §2-geometry-
      Zeile; 042e = persistence-Kante + §2-persistence-Zeile + §1-Diagramm (heute weist Index-Zeile 042e beides zu).
      Closure-Notiz. **[MR-020](../../../../harness/conventions.md) Closure-Disziplin:** vor Closure existiert
      slice-042d (Skelett) → erfüllt.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/services/exchange_service.{cpp}` | ändern | STEP/STL-Bündel format-selektiv berechnen; `storeyHeight`-Helfer; services/geometry-Includes |
| `src/adapters/geometry/step_export_adapter.cpp` | ändern | Loops auf `derived.*`; services-Includes + `storeyHeight` raus; OCC-Skip bleibt |
| `src/adapters/geometry/stl_export_adapter.cpp` | ändern | Loops auf `derived.*`; services-Includes + `storeyHeight` raus; OCC-Skip bleibt |
| `.a-check.yml` | ändern | Kante `geometry → services_geo` entfernen (Verschärfung) |
| `tests/adapters/test_step_stl_export.cpp` | ändern | ~10 Direkt-`write` → `writeStep`/`writeStl` (befüllt); Negativ-Test (Total/leeres Bündel) |
| `spec/spezifikation.md` + `-historie.md`, [ADR-Index](../../adr/README.md), `CHANGELOG.md` | ändern | Doku-Nachzug (token-frei) |
| `spec/architecture.md` §2 | ändern | geometry-Adapter-Zeile: `services/geometry` raus (Option A, konsistent mit `.a-check.yml`; persistence-Zeile + §1-Diagramm bleiben 042d/e) — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2 |
| [ADR-Index](../../adr/README.md) | ändern | 042c/042e-Zeilen reconcilen (geometry vs. persistence/§1-Diagramm) |

**Bewusst NICHT Teil:** Persistenz-`rise` + `persistence → services_geo`-Kante (042d); `.a-check.yml`-**Abschluss**
+ `architecture.md` **§1-Diagramm** (042e); die 2D-Projektion war 042b. **Die `DerivedGeometry`-Felder reichen
restlos** (keine Erweiterung).

## 4. Trigger

- **slice-042b done** (`DerivedGeometry` + `ExchangeService`-Format-Selektion-Muster + PlanViewPort vorhanden).
  [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)/[ADR-0014](../../adr/0014-step-stl-export-backend.md) Accepted.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  **Geometrie-schwer → Code-Review vor Welle-Closure** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (**B-Rep-/STL-Orakel unverändert grün** + Negativ-Test = Invarianz-Beweis;
  `make a-check` grün nach Kanten-Entfernung), Closure-Notiz → **slice-042d (Persistenz)** startbar.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — Verhaltens-Invarianz der Ableitung-aus-dem-`try` (der Kern-Prüfstein):** heute umschließt der
  adapter-`try/catch` **sowohl** die services-Ableitung **als auch** die OCC-Montage; nach der Migration läuft die
  Ableitung **ohne** `try` im `ExchangeService`. Das ist **nur dann** invariant, wenn die services-Ableitung
  **total** ist (wirft nie) — der `try` fing bisher nur OCC-Würfe. Die Kartierung bestätigt Totalität
  (`wallFootprint`/`…`/`storeyHeight` werfen nie; der Skip sitzt an `makeNetSolid`/`tessellateFootprint`).
  **Mitigation:** die Impl **verifiziert** die Totalität je Funktion (Vertrag/Test) und der **Negativ-Test**
  (danglender Storey/degeneriert → kein Wurf) netzt es ab. Reißt er → nicht invariant → Stopp.
- **Rest-Risiko #2 — Compound-/Netz-Reihenfolge:** die B-Rep-Zählung ist reihenfolge-robust (Substring-Count),
  aber die Ableitung muss **ein Element je Bauteil in Modell-Reihenfolge** liefern (kein Vor-Filtern degenerierter
  im Service — der Skip gehört an die OCC-Stelle im Adapter, sonst verschiebt sich, welches Bauteil fehlt).
  Mitigation: der Service filtert **nicht**; er leitet je Bauteil ab (auch degenerierte), der Adapter skippt beim
  OCC-Bau — identisch zu heute.
- **Rest-Risiko #3 — `architecture.md` §2 (ENTSCHIEDEN: Option A, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2):**
  042c entfernt die `.a-check.yml`-Kante `geometry → services_geo` — ohne §2-Nachzug wäre `architecture.md` §2
  (geometry-Zeile listet `services/geometry`) **inkonsistent** zur Allow-Liste (zwei normative Quellen; **kein
  Gate** kreuzt §2 gegen `.a-check.yml`). **042c zieht daher die §2-geometry-Zeile mit** (schritt-genau wahr);
  die persistence-Zeile bleibt 042d, das §1-Diagramm 042e (grob-Ebene). Der [ADR-Index](../../adr/README.md)
  (heute: 042e macht beide Kanten + §2/§1) wird auf diese Aufteilung nachgezogen.
- **Rest-Risiko #4 — Test-Migration + das Service-Netz-Loch ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1/042b-LOW-2):**
  die `writeStep`/`writeStl`-Helfer speisen das Bündel aus **derselben** services-Ableitung (nicht handgerollt).
  **Aber Achtung — das allein reicht nicht:** ein Direkt-Helfer (Bündel selbst gebaut, Adapter direkt gerufen)
  **umgeht die `ExchangeService`-Berechnung** und beweist nur, dass der Adapter ein **gegebenes** Bündel treu
  serialisiert — **nicht**, dass der Service das **richtige** Bündel je Bauteil baut. Darum der **verbindliche
  Voll-Modell-Integrationstest über `service.exportModel`** (DoD, MED-1) — er ist das eigentliche Netz über dem
  neuen Code (die vier Bauteil-Loops + Storey-Auflösung).
- **a-check-Gegenprobe:** nach der Kanten-Entfernung `make a-check` grün; die Entfernung ist eine **Verschärfung**
  (Allow-Liste verengt, [§2.6](../../../../AGENTS.md) n/a).
- **Spec-Straten token-frei** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)):
  vor dem Gate greppen (Lerneintrag 032/041a/042a/b).
- **Scope-Größe:** groß (zwei Adapter-Bodies + `ExchangeService`-Berechnung + `storeyHeight` + a-check + Test-
  Migration + Negativ-Test), aber überwiegend mechanisch; die B-Rep-/STL-Orakel + der Negativ-Test sind das Netz.
  Das Review bewertet, ob ein Split (z. B. STEP getrennt von STL) nötig ist — beide teilen aber die
  `ExchangeService`-Berechnung, was gegen einen Split spricht.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Kern + Adapter-Code `src/`

- **Modus:** GF; **Dichte:** hoch (zwei OCC-nahe Adapter-Bodies + kern-seitige Ableitungs-Berechnung + a-check-
  Kante). **Phase-Reife:** welle-5 Refactor. **Risiko:** mittel-hoch — geometrieschwer, aber verhaltens-invariant;
  die B-Rep-/STL-Orakel + Negativ-Test sind das Netz, **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review vor Welle-Closure** Pflicht.

### Sub-Area: Spec/Doku `spec/` + `docs/plan/`

- **Modus:** GF; **Dichte:** mittel (STEP/STL-Datenfluss token-frei; a-check-Kante; ADR-Index/CHANGELOG;
  architecture-§2-Entscheidung). **Risiko:** niedrig-mittel.

## 8. Closure-Notiz

**Closure 2026-07-23.** `make gates` grün (252/252 Tests, Coverage 91,5 %); die STEP-B-Rep-`CLOSED_SHELL`-
Zählung + das binäre STL-Netz-Orakel **unverändert grün** = Invarianz-Beweis; `make a-check` grün **nach** der
Kanten-Entfernung (Gegenprobe). `make schema-check` unberührt.

- **`ExchangeService`-Berechnung + `storeyHeight`-Konsolidierung:** der Service befüllt `derived.walls/slabs/
  roofs/stairs` format-selektiv (ein Eintrag je Bauteil in Modell-Reihenfolge; die reine Ableitung ist total);
  `storeyHeight` liegt als **eine** Wahrheit im Service, die **zwei** Adapter-Kopien entfielen (042a-LOW-2).
- **STEP/STL-Body-Konsum + Skip-Grenze:** die Adapter iterieren `derived.*` und bauen OCC (STEP `occ_solids`) bzw.
  tessellieren (STL `GeometryKernelPort`); der **fail-closed per-Bauteil-`try/catch{continue}`-Skip bleibt im
  Adapter** (OCC-resident, Regel C) — die Ableitung wanderte in den Service, der OCC-Skip nicht. `building`-Param
  in beiden `write` folgenlos ungenutzt.
- **Kante `geometry → services_geo` restlos entfernt** (`.a-check.yml`); **Gegenprobe:** `make a-check` grün, ein
  Rest-`services/geometry`-Aufruf im geometry-Layer würde jetzt failen. `architecture.md` §2-geometry-Zeile
  mitgezogen (Option A, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2
  — schritt-genau konsistent); [ADR-Index](../../adr/README.md) reconcilet (042c = geometry-Kante + §2-geometry;
  042e = `persistence`-Kante + §2-persistence + §1-Diagramm). `persistence → services_geo` **bleibt** (042d).
- **Netz gegen die neue Service-Berechnung ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1):**
  `writeStep`/`writeStl` fahren jetzt über den **echten** `ExchangeService` (die starken B-Rep-/STL-Orakel
  verifizieren die Service-Berechnung mit); neu: **Voll-Modell-Integrationstest** (`CLOSED_SHELL == wallShells +
  1 + 1 + step_count`), **STL-`baseZ`-Z-Sonde** (LOW-1), **Totalitäts-Test beide Pfade** (danglendes Geschoss →
  Fallback+Export; degeneriert → OCC-Skip; LOW-2).
- **Reviews:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  0 HIGH / 2 MED / 2 LOW (alle eingearbeitet) + [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  Code-Review **0 HIGH** (Byte-Identität Zeile-für-Zeile verifiziert; [Report](../../../reviews/2026-07-23-slice-042c-code-review.md)).
- **Lerneintrag (MED-1):** das 042b-`writePdf`-Direkt-Helfer-Muster **umgeht den Service** — auf STEP/STL
  übertragen hätten die starken Orakel nur die Adapter-Serialisierung eines **vorgefertigten** Bündels geprüft,
  nicht die neue Service-Berechnung (den Löwenanteil). Das Review fing es; das Netz fährt jetzt durch den Service.
- **Folge:** **slice-042d** (Persistenz-`rise` kern-seitig + `persistence → services_geo`-Kante raus; Skelett in
  `open/`, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  beim Start) — [MR-020](../../../../harness/conventions.md) Closure-Disziplin erfüllt (042d existiert).
