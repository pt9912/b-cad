---
id: slice-015b
titel: Decken + Fundament implementieren — Platten (base_z via Kern-Translation)
status: done
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-SLB-001, LH-FA-SLB-002, LH-FA-SLB-003, LH-FA-FND-001, LH-FA-FND-002, LH-FA-FND-003]
adr_refs: [ADR-0001, ADR-0002, ADR-0008, ADR-0009, ADR-0011]
---

# Slice 015b: Decken + Fundament implementieren — Platten

**Status:** done (2026-06-13). MR-006-Plan-Review gelaufen (keine HIGH,
2 MED + LOW/INFO eingearbeitet); DoD vollständig. Zwei-Commit-Split
(i Domäne+slab_geometry / ii Integration). **Code-Review danach (Modul 11,
geometrielastig): 1 HIGH gefunden TROTZ grüner Gates** — der OCC-Cutout-
Boolean war ungetestet und `addSlabCutout` setzte die Spec-Begrenzung „auf
den Platten-Umriss" nicht durch. **Nachschärfung** (`cutoutInsideSlab` +
Test der OCC-Naht; Report `docs/reviews/2026-06-13-slice-015b-code-review.md`).
`make gates` grün nach Nachschärfung (103 Tests, Coverage 91,9 %).
Closure-Notiz §8.

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-015b-plan.md) — keine HIGH
(base_z-via-Mesh-Translation tragfähig: Volumen z-invariant, reine
`TriangleMesh`-Operation, keine Wand-Regression). **MED-2** (Cutout-z
muss **relativ** `[−ε, Dicke+ε]` sein, Translation erst NACH dem Boolean
— sonst falscher Schnitt + doppelter Offset; Reihenfolge-Falle behoben),
**MED-1** (base_z-Quelle explizit: Decke = `storey.height_mm` unter
Ein-Geschoss-Annahme; Bodenplatte/Fundament = −Dicke → Oberkante 0),
LOW-2 (Volumen on-demand, kein Solid-Cache), INFO-1 (§1-base_z-Frage in
der Closure schließen) eingearbeitet.

**Welle:** welle-2-bauteile (achter Slice).

**Bezug:** LH-FA-SLB-001..003 + LH-FA-FND-001..003 (Lastenheft 0.1.5,
slice-015a), `spezifikation.md` §1 `LH-FA-SLB-001.a` (Platten-Geometrie)
+ §3 (Dicke-/Tiefe-Bereiche). **Parametrisiert auf ADR-0011 (#6)** —
kein neuer Grundsatz-ADR. ADR-0002 (OCC-Boolean für Ausschnitte),
ADR-0008 (Notification), ADR-0009 (Sicht über `ViewModelPort`),
ADR-0001 (Kern führt; Port-Signatur = Kern-Hoheit).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Implementierungs-Hälfte des Platten-Strangs
(Muster 013b/014b). **Scope:** sichtbare, **in-memory** Platte (Decke +
Fundament + Bodenplatte, mit Ausschnitten); **Persistenz** (`slabs`-
Tabelle) ist **slice-015c**. **Geplante Zwei-Commit-Sequenz:** **(i)**
Domäne + `slab_geometry` (base_z + Cutout-Prismen + Mesh) + Kern-AK →
**(ii)** Service + `ViewModelPort` + Viewer + Edit-Ops + Szenen-AK.

---

## 1. Ziel

Sichtbare horizontale Platten: Decke (auf Geschoss-Oberkante), Fundament
(unter Gelände) und Bodenplatte (Oberkante 0), mit optionalen
Ausschnitten (SLB-003), in der 3D-Szene dargestellt und der
ADR-0008-Mechanik folgend. Der Kern bleibt framework-frei (ADR-0001).

## 2. Definition of Done

- [x] **Domänen-Typen (pure Werte, `src/hexagon/model/`):** `Slab`
      (`SlabId`, `storey_id`, `SlabType ∈ {Decke, Fundament, Bodenplatte}`,
      `footprint: Footprint` (Grundriss-Polygon), `thickness_mm`,
      `cutouts: std::vector<Footprint>` (rechteckige Aussparungen,
      SLB-003)); `Building` gewinnt `std::vector<Slab> slabs`.
      Framework-frei (Regel A).
- [x] **base_z via Kern-Translation — KEIN Port-Signatur-Wechsel
      (Auflösung HIGH-1 aus 015a-Review):** der `GeometryKernelPort`
      (`extrudeFootprint`/`tessellateFootprint` + `CutPrism`) wird
      **unverändert** wiederverwendet (Volumen ist z-invariant; das Netz
      entsteht bei z∈[0,Dicke]). Eine **pure Kern-Funktion**
      `slab_geometry` berechnet (a) `base_z` je `slab_type` (**MED-1,
      Ein-Geschoss-Annahme der Welle:** Decke = `storey.height_mm`
      (Geschoss-Unterkante 0, Oberkante = Höhe); Bodenplatte = −Dicke →
      Oberkante 0; Fundament = −Dicke → Oberkante 0, Tiefe nach unten —
      `Storey` trägt keine Elevation, daher diese Festlegung; Mehr-
      Geschoss-Stapelung später), (b) die Cutout-`CutPrism`s aus den
      Aussparungen mit **z RELATIV zum Extrusions-Solid `[−ε, Dicke+ε]`**
      (NICHT base_z! MED-2), und (c) **verschiebt erst das fertige
      (geschnittene) Platten-Netz um `base_z` in z** (reine Mesh-
      Translation, NACH dem Boolean). **Reihenfolge zwingend (MED-2):**
      extrudieren/schneiden bei z∈[0,Dicke] → dann Netz +base_z; sonst
      schneidet der Boolean an der falschen Höhe und der Offset
      verdoppelt sich. Volumen ist z-invariant (Ausschnitt-AK unberührt
      von der Translation). Wand-Aufrufe bleiben unberührt (kein
      Port-Wechsel; HIGH-1 ohne ADR-0001-Signatur-Eingriff gelöst).
- [x] **`ViewModelPort` um Platten-Sicht:** `slabMeshes()` →
      `std::vector<SlabMesh{slab_id, mesh}>` (Muster `roofMeshes`, total).
      Vom `StructureEditService` implementiert.
- [x] **`ModelChangedPort` um `SlabChanged`** (§5-`op`-Vokabular,
      storey-bezogen; neuer Bauteil-Typ, ADR-0011 #6.4 — wie
      `RoofChanged`). **Keine `RoomsChanged`** (Platten berühren die
      Raumerkennung nicht). `spec/spezifikation.md` §1 D3-002.a um den
      `op` ergänzt (Mechanik, MR-008).
- [x] **`EditStructurePort`-Operationen** (im Service): `addSlab`
      (Prototyp: Typ + footprint + thickness + optionale cutouts;
      degenerierter footprint → abgelehnt; Dicke/Tiefe gegen §3 geklemmt,
      `E-VAL-001`), `setSlabThickness`, `addSlabCutout` (SLB-003,
      rechteckige Aussparung, auf den Platten-Umriss begrenzt),
      `removeSlab`. Jede erfolgreiche Mutation meldet `SlabChanged`.
- [x] **Viewer folgt** (`ViewerScene`): hält Platten-Netze (`SlabId`-
      Map); auf `SlabChanged` `slabMeshes()` neu laden, **idempotent**
      (Zähler = tatsächlich geänderte Netze, Muster RoofChanged/MED-2).
      Wand-/Dach-Pfad additiv unberührt.
- [x] **AK-Tests mit `LH-FA-SLB-*`/`LH-FA-FND-*` im Namen** (Kern +
      Szene), display-frei: **Decke** (Netz-Unterkante auf Geschoss-
      Oberkante; Dicke = Netz-z-Span); **Bodenplatte** (Oberkante z=0);
      **Fundament** (unterhalb 0, Tiefe = z-Span, Oberkante 0);
      **Ausschnitt** (Volumen um das Ausschnitt-Volumen verringert;
      Ausschnitt auf Umriss begrenzt); **Dicke/Tiefe** (Boundary geklemmt);
      **Negative** (degenerierter footprint → abgelehnt); **Folge-Meldung**
      (`SlabChanged`, kein `RoomsChanged`; Szene folgt idempotent);
      **Regression** (Wand-/Dach-/ROM-/D3-Tests textlich unverändert grün —
      additiv, **keine** Port-Signatur-Migration).
- [ ] arch-check A–E grün; `make gates` grün; Closure-Notiz mit
      Lerneintrag; CHANGELOG (MR-004). **Nicht Teil:** Persistenz
      (`slabs`-Round-Trip → slice-015c); Material/Bewehrung; nicht-
      rechteckige Ausschnitte (falls §1 das einschränkt).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{slab}.h` | neu | `Slab`, `SlabId`, `SlabType` (pure Werte) |
| `src/hexagon/model/building.h` | ändern | `std::vector<Slab> slabs` |
| `src/hexagon/model/constants.h` | ändern | `kSlabThicknessMin/MaxMm`, `kFoundationDepthMin/MaxMm`, Defaults (spez. §3, `k…`-Konvention) |
| `src/hexagon/services/{slab_geometry}.{h,cpp}` | neu | base_z je Typ + Cutout-Prismen + Mesh-Translation (pur + Port-Aufruf) |
| `src/hexagon/ports/driving/view_model_port.h` | ändern | `SlabMesh` + `slabMeshes()` |
| `src/hexagon/ports/driving/edit_structure_port.h` | ändern | Slab-Operationen |
| `src/hexagon/ports/driven/model_changed_port.h` | ändern | `SlabChanged`-`op` |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Slabs halten, Mesh über `slab_geometry`/Port, `SlabChanged`, Klemmung |
| `src/adapters/ui/viewer_scene.{h,cpp}` | ändern | Platten-Netze (`SlabId`-Map), `SlabChanged` → `slabMeshes()` neu laden (idempotent) |
| `spec/spezifikation.md` | ändern | §1 D3-002.a `SlabChanged`-`op`; §8 |
| `tests/hexagon/{test_slab_geometry}.cpp` | neu | Kern-AK (base_z je Typ, Dicke, Ausschnitt-Volumen) |
| `tests/adapters/test_viewer_scene.cpp` | ändern | Szene folgt `SlabChanged` |
| `tests/hexagon/test_structure_edit_service.cpp` o. eigene Datei | ggf. ändern | Slab-Edit-Ops/Klemmung/Folge-Meldung |
| `tests/CMakeLists.txt` | ändern | neue Testdatei(en) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/{2026-06-13-slice-015b-plan}.md` | neu | MR-006-Report |

## 4. Trigger

- slice-015a done ✓ (Lastenheft 0.1.5 + Spec §1/§3).
- MR-006-Plan-Review vor Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-015c**
  (Platten-Persistenz) startbar; danach STR (Treppen), dann Welle-2-Closure.

## 6. Risiken und offene Punkte

- **base_z (HIGH-1 aus 015a) — gelöst ohne Port-Wechsel:** Kern-Mesh-
  Translation statt `extrudeFootprint`-Signatur-Erweiterung. Volumen
  (für die Ausschnitt-AK) ist z-invariant → `extrudeFootprint` liefert es
  unverändert; nur das **Netz** wird um `base_z` verschoben (reine
  Operation auf dem `TriangleMesh`-Wert, ADR-0001 Kern-Hoheit). **Wände
  bleiben unberührt** (keine Migration, kleineres Regressionsrisiko als
  013b). MR-006 prüft, ob diese Auflösung HIGH-1 wirklich schließt.
- **Cutout-Prismen-z-Bereich (MED-2, Reihenfolge-Falle):** ein
  Decken-Ausschnitt durchsetzt die volle Platten-Dicke; die `CutPrism`
  spannt **relativ zum Extrusions-Solid** `[−ε, Dicke+ε]` (NICHT
  `[base_z−ε, …]`!), weil zuerst bei z∈[0,Dicke] extrudiert/geschnitten
  und **erst danach** das Netz um `base_z` verschoben wird. Absolute z
  mit base_z würden den Schnitt verfehlen (Footprint-Solid liegt bei
  [0,Dicke]) und die Translation doppelt anwenden. Überstand ε wie bei
  Öffnungen (volumen-neutral).
- **Volumen on-demand (LOW-2):** der Service cacht **kein** Slab-Solid
  (anders als Wände in `solids_`); das Volumen für die Ausschnitt-AK
  wird per `extrudeFootprint`-Aufruf on demand gemessen (Muster Dach:
  reine Query). `slabMeshes()` liefert nur Netze.
- **§1-base_z-Frage schließen (INFO-1):** spez. §1 `LH-FA-SLB-001.a`
  lässt die Port-/Platzierungs-Mechanik offen; die Closure von 015b zieht
  die getroffene Wahl (Mesh-Translation, kein Port-Wechsel) in §1 nach
  (Muster 014b D3-002.a).
- **`ViewModelPort`/`ViewerScene` additiv:** dritter Element-Typ neben
  Wand/Dach; Wand-/Dach-AK bleiben textlich grün (Regressions-Aussage).
- **Decke+Fundament+Bodenplatte als ein `Slab` mit `SlabType`:** hält den
  Code klein; base_z je Typ ist die einzige Divergenz (slab_geometry).
- **footprint-Herkunft:** explizites Polygon (Parameter, wie Dach-
  Rechteck) — Auto-Ableitung aus dem Gebäudeumriss bleibt späterer Ausbau.
- **Persistenz-Lücke (bewusst, 015c):** Platten nur im Speicher (kein
  lebender Save-Pfad, kein §2.2-Risiko).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (framework-frei, base_z-Geometrie,
  Reuse extrudeFootprint, Totalität); Risiko mittel (base_z-/Cutout-
  Mathematik — durch analytische AK gedeckt).

### Sub-Area: GUI-Adapter (`src/adapters/ui/`)

- **Modus:** GF; Dichte hoch (Regel E, ViewModelPort-Pull, Idempotenz);
  Risiko niedrig–mittel (dritter Szenen-Element-Typ — additiv).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (LH-ID-Namen, analytische Orakel,
  Registrierung); Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **Domäne + Kern-Geometrie:** `model::Slab` (Decke/Fundament/
  Bodenplatte, footprint-Polygon, thickness, cutouts); `slab_geometry`
  (pur): `slabBaseZ` je Typ, `slabCutPrisms` z **relativ** `[−ε,Dicke+ε]`,
  `translateMeshZ`. **base_z ohne Port-Wechsel (HIGH-1 gelöst):** der
  Kern ruft `extrudeFootprint`/`tessellateFootprint` unverändert (Volumen
  z-invariant) und verschiebt das fertige Netz um base_z. Reihenfolge
  (MED-2): Cutout relativ → Boolean → Translation. arch-check Regel A.
- **Integration:** `ViewModelPort.slabMeshes()`; `EditStructurePort`
  addSlab/setSlabThickness/addSlabCutout/removeSlab (Klemmung typabhängig
  `E-VAL-001`, degenerierter Grundriss/unbekanntes Geschoss abgelehnt);
  `SlabChanged`-`op` (storey-bezogen, **kein** `RoomsChanged`);
  `ViewerScene` lädt Platten auf `SlabChanged` neu — idempotent über den
  **gemeinsamen Template-Helfer `reloadKeyed`** (Roof+Slab, entfernt
  Duplikation + senkt die `onModelChanged`-Cognitive-Complexity).
- **8 AK-Tests `LH-FA-SLB-*`/`LH-FA-FND-*`:** Kern (base_z je Typ,
  Cutout relativ, Ausschnitt-Volumen, Netz-Translation) + Viewer (Decke
  auf Geschoss-Oberkante via realer OCC, Bodenplatte Oberkante 0, Dicke
  geklemmt, degeneriert abgelehnt, Ausschnitt, `SlabChanged`-nicht-`Rooms`).
- **`make gates` grün** (2026-06-13): docs-check 0, arch-check A–E,
  lint 0 + suppression-gate, **Tests 102/102** (zuvor 99→Phase i, dann
  Integration), Coverage **91,7 %**. Zwei-Commit-Split (i `ad6a730`).
- **Spec §1** Port-base_z-Frage geschlossen (kein Port-Wechsel,
  Mesh-Translation) + `SlabChanged`-`op`; §8.

**Lerneintrag:**

- **HIGH-1 sauber ohne Port-Eingriff gelöst:** Volumen z-invariant +
  Mesh-Translation reicht — die Wand-Aufrufe blieben unberührt (keine
  Signatur-Migration). Die MED-2-Reihenfolge (Cutout relativ, Translation
  nach Boolean) war der korrektheits-kritische Punkt; vorab im
  Plan-Review gefangen, nicht erst im Code.
- **`reloadKeyed`-Helfer:** der zweite storey-bezogene Element-Typ
  (Platte nach Dach) machte die Duplikation sichtbar → Template-Helfer;
  zugleich nötig gegen den Cognitive-Complexity-lint (Build-Gate fing es).

**Restrisiko / Nachfolge:** **slice-015c** (Platten-Persistenz `slabs`-
Tabelle, `polygon_json` wie 014c-`footprint_json`) — Platten derzeit nur
im Speicher (kein lebender Save-Pfad). **Empfohlen vor Welle-Closure:**
Code-Review (geometrielastig, Praxis 3×+ nach welle-1). Offen: nicht-
rechteckige Ausschnitte, Material/Bewehrung, Auto-Ableitung des
Grundrisses aus dem Gebäudeumriss. Danach STR (Treppen), Welle-2-Closure.
