---
id: slice-014b
titel: Dach implementieren — Sattel/Walm/Pult über Rechteck-Grundriss
status: next
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-ROF-001, LH-FA-ROF-002, LH-FA-ROF-003, LH-FA-ROF-004, LH-FA-ROF-005]
adr_refs: [ADR-0001, ADR-0008, ADR-0009, ADR-0011]
---

# Slice 014b: Dach implementieren — Sattel/Walm/Pult

**Status:** next (Plan geschrieben; **MR-006-Plan-Review gelaufen
2026-06-13 — keine HIGH, 4 MED/2 LOW eingearbeitet**;
implementierungsbereit).

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-014b-plan.md) — keine HIGH
(OCC-Frage zugunsten Kern-Mesh entschieden: ADR-0009-Verbot ist
GUI-gerichtet, `TriangleMesh` reiner Wert, slice-012-Präzedenz). MED-1
(#6.3-Abweichung explizit), MED-2 (Dach-Idempotenz-Semantik), MED-3
(Konstanten-Konvention), MED-4 (Zwei-Commit-Split), LOW-2 (Pult-Formel)
eingearbeitet.

**Welle:** welle-2-bauteile (fünfter Slice).

**Bezug:** LH-FA-ROF-001..005 (Lastenheft 0.1.4, slice-014a),
`spezifikation.md` §1 `LH-FA-ROF-001.a` (Dach-Geometrie-Algorithmus) +
§3 (Neigungs-/Überstands-Bereiche, Defaults). **Parametrisiert auf
ADR-0011 (#6, Bauteil-Erweiterungs-Muster)** — kein neuer Grundsatz-ADR.
ADR-0008 (Notification), ADR-0009 (Sicht über `ViewModelPort`),
ADR-0001 (Kern führt, framework-frei).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Implementierungs-Hälfte des Dach-Strangs (Muster
013b). **Scope-Grenze (Welle-Sizing):** dieser Slice liefert das
**sichtbare, in-memory** Dach (Mesh + Viewer + AK); die **Persistenz**
(`roofs`-Tabelle, ADR-0006-Schema liegt vor) ist **slice-014c**
(Muster 013b/013c). **Geplante Zwei-Commit-Sequenz** (MED-4, nicht erst
bei Sitzungs-Kippen, da der neue Szenen-Element-Typ mehr Adapter-Code
bringt als 013b): **(i)** Domäne + `roof_geometry` (Mesh) + Kern-AK
gegen analytische Werte — einzeln grün committbar → **(ii)** Service +
`ViewModelPort` + Viewer + Edit-Ops + Szenen-AK.

---

## 1. Ziel

Ein sichtbares Dach: Sattel-, Walm- oder Pultdach über einem
rechteckigen Grundriss, mit parametrischer Neigung und Überstand, in der
3D-Szene dargestellt (`ViewModelPort`/`ViewerScene`), folgt Mutationen
über den ADR-0008-Vertrag. Der Kern bleibt framework-frei (ADR-0001);
die Dach-Geometrie ist ein **analytisches Polyeder** (kein OCC-Boolean/
-Extrusion nötig).

## 2. Definition of Done

- [ ] **Domänen-Typen (pure Werte, `src/hexagon/model/`):** `Roof`
      (`RoofId`, `storey_id`, `RoofType ∈ {Sattel, Walm, Pult}`,
      rechteckiger Grundriss als `origin: Point2D` + `width_mm` (b) +
      `depth_mm` (t), `base_z_mm` (Traufhöhe/Aufstandshöhe),
      `pitch_deg`, `overhang_mm`); `Building` gewinnt
      `std::vector<Roof> roofs`. Framework-frei (arch-check Regel A).
- [ ] **Kern-Geometrie `services/roof_geometry.{h,cpp}`:**
      `TriangleMesh roofMesh(const Roof&)` baut das Dach-Netz
      **analytisch** gemäß `spezifikation.md` §1 `LH-FA-ROF-001.a`
      (Traufrechteck aus Überstand; Pult = eine geneigte Fläche; Sattel
      = First mittig längs, zwei Flächen; Walm = vier Flächen, First um
      den deterministischen Einrückbetrag kürzer; Firsthöhe =
      Formel·`tan(pitch)`). **Total:** degenerierter/zu kleiner Grundriss
      (`< GEOMETRY_TOLERANCE_MM`) → leeres Netz, kein Wurf. Pure Domäne
      (kein OCC/Qt; **ADR-0009-konform, weil der Vertrag „framework-freie
      Dreiecksnetze über `ViewModelPort`" ist — die OCC-Tessellation gilt
      dem extrudierten Wand-Modell, nicht dem analytischen Polyeder**;
      MR-006 prüft diese Lesart).
- [ ] **`ViewModelPort` um Dach-Sicht erweitert:** `roofMeshes()` →
      `std::vector<RoofMesh{roof_id, mesh}>` + `roofMesh(RoofId)` →
      `std::optional<TriangleMesh>` (Muster `wallMeshes`/`wallMesh`,
      total). Vom `StructureEditService` implementiert (ruft
      `roof_geometry`).
- [ ] **`ModelChangedPort` um `RoofChanged`** (§5-`op`-Vokabular,
      `bcad.geometry.rebuild`): meldet `storey_id` (kein spezifischer
      Roof-Id-Bedarf — der Viewer lädt die Dächer des Stands neu).
      `spec/spezifikation.md` §1 D3-002.a um den `op` ergänzt
      (Mechanik = Spec, MR-008). **Keine `RoomsChanged`** (Dächer
      berühren die Raumerkennung nicht).
- [ ] **`EditStructurePort`-Operationen** (im Service): `addRoof`
      (storey + Typ + Grundriss `b×t`/origin/base_z; Default-Neigung/
      -Überstand aus §3), `setRoofPitch`, `setRoofOverhang`,
      `setRoofType`, `removeRoof`. Neigung/Überstand gegen §3 geklemmt
      (`E-VAL-001`); degenerierter Grundriss → abgelehnt (nullopt).
      Jede erfolgreiche Mutation meldet `RoofChanged`.
- [ ] **Viewer folgt** (`ViewerScene`): hält Dach-Netze (eigener
      `RoofId`-Schlüssel); auf `RoofChanged` lädt es `roofMeshes()` neu
      (idempotentes Ersetzen, deckt Anlage/Änderung/Entfernen). Wand-
      Pfad unberührt. **Idempotenz-Semantik (MED-2):** der
      `effectiveUpdates`-Zähler steigt um die Zahl **tatsächlich
      geänderter** Dach-Netze (neu/ersetzt/entfernt); eine **identische
      erneute** `RoofChanged`-Meldung erzeugt **kein** Netz-Wechsel und
      **kein** weiteres wirksames Update (kein Flackern) — der AK prüft
      genau das (Anlehnung an den Wand-Idempotenz-AK, aber element-
      mengenbasiert statt 1-pro-Meldung).
- [ ] **AK-Tests mit `LH-FA-ROF-*` im Namen** (Kern gegen analytische
      Netz-Eigenschaften + Szene), display-frei: **Happy** je Typ
      (Pult: eine geneigte Fläche, Hochkante = `(t+2o)·tan(p)` ±
      Toleranz, LOW-2; Sattel: First mittig, Firsthöhe =
      `(kürzere_eaves/2)·tan(p)` ± Toleranz; Walm: First kürzer als
      Grundriss); **Überstand** (Netz-Bounding-Box ragt um `o` über den
      Grundriss); **Neigung** (steiler → höherer First, messbar);
      **Boundary** (Neigung/Überstand am Grenzwert akzeptiert, außerhalb
      geklemmt); **Negative** (degenerierter Grundriss → kein Netz/
      abgelehnt); **Folge-Meldung** (`RoofChanged`, kein `RoomsChanged`;
      Szene folgt, idempotent); **Regression** (Wand-/ROM-/D3-Tests
      textlich unverändert grün — der Dach-Pfad ist additiv).
- [ ] arch-check A–E grün; `make gates` grün; Closure-Notiz mit
      Lerneintrag; CHANGELOG (MR-004). **Nicht Teil dieser DoD:**
      Persistenz (`roofs`-Round-Trip → slice-014c); Walm über allgemeine
      Polygone (Vollumfang spätere Welle); Material/Dachdeckung.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{roof}.h` | neu | `Roof`, `RoofId`, `RoofType` (pure Werte) |
| `src/hexagon/model/building.h` | ändern | `std::vector<Roof> roofs` |
| `src/hexagon/model/constants.h` | ändern | `kRoofPitchMin/MaxDeg`, `kRoofOverhangMin/MaxMm`, `kDefaultRoofPitchDeg`, `kDefaultRoofOverhangMm` — **`k…`-Code-Konvention + Spec-Mapping-Kommentar** (`// LH-FA-ROF-004/005`), Werte aus spez. §3 (5/60/0/1500, 30/500); kein Drift (MED-3) |
| `src/hexagon/services/{roof_geometry}.{h,cpp}` | neu | analytisches Dach-Netz (3 Typen, pure Domäne) |
| `src/hexagon/ports/driving/view_model_port.h` | ändern | `RoofMesh` + `roofMeshes()`/`roofMesh(RoofId)` |
| `src/hexagon/ports/driving/edit_structure_port.h` | ändern | Roof-Operationen |
| `src/hexagon/ports/driven/model_changed_port.h` | ändern | `RoofChanged`-`op` |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Roofs halten, Mesh über `roof_geometry`, `RoofChanged` melden, Klemmung |
| `src/adapters/ui/viewer_scene.{h,cpp}` | ändern | Dach-Netze (`RoofId`-Map), `RoofChanged` → `roofMeshes()` neu laden |
| `spec/spezifikation.md` | ändern | §1 D3-002.a `RoofChanged`-`op` (Mechanik, MR-008); §8 |
| `tests/hexagon/{test_roof_geometry}.cpp` | neu | Kern-AK je Dachtyp (analytisch) |
| `tests/adapters/test_viewer_scene.cpp` | ändern/neu | Szene folgt `RoofChanged` |
| `tests/hexagon/test_structure_edit_service.cpp` | ggf. ändern | Roof-Edit-Ops/Klemmung/Folge-Meldung (oder eigene Datei) |
| `tests/CMakeLists.txt` | ändern | neue Testdatei(en) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/{2026-06-13-slice-014b-plan}.md` | neu | MR-006-Report |

## 4. Trigger

- slice-014a done ✓ (Lastenheft 0.1.4 + Spec §1/§3).
- MR-006-Plan-Review vor Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-014c**
  (Dach-Persistenz) startbar; danach SLB+FND, STR entlang der
  ADR-0011-Leitplanke.

## 6. Risiken und offene Punkte

- **Dach-Mesh ohne OCC (heikelste Architektur-Frage; MR-006 bestätigt):**
  das Dach ist ein analytisches Polyeder; sein Netz im Kern zu rechnen
  ist ADR-0001-konform (Kern führt Geometrie, vgl. `wall_footprint`
  slice-012) und ADR-0009-konform (der bindende Satz ist „kein OCC in
  der **GUI**" + „framework-freie Netze über `ViewModelPort`"; die
  OCC-Tessellation in ADR-0009 (b) adressiert das **extrudierte
  Wand-Solid**, kein generelles OCC-Mandat). `model::TriangleMesh` ist
  ein reiner Werttyp, im Kern legal erzeugbar. **Abweichung von
  ADR-0011 #6.3 (MED-1):** #6.3 formuliert den Pfad als „Port
  tesselliert (OCC-Backend)"; das Dach weicht bewusst ab — gedeckt durch
  die #6-Klausel „die **Geometrie-Entscheidung** lebt im jeweiligen
  Folge-Slice" (ADR-0011 :142–145). **Kein Adapter-OCC fürs Polyeder.**
  In der Closure als Präzedenz für SLB/FND festhalten.
- **`ViewModelPort`/`ViewerScene` waren wand-zentrisch:** der Dach-Pfad
  ist **additiv** (eigene `roofMeshes`/`RoofId`-Map), die Wand-AK
  bleiben textlich grün (Regressions-Aussage, Muster 013b-W3-P1).
- **`ModelChange` trägt nur `wall_id`:** `RoofChanged` meldet
  storey-bezogen (kein neues Struct-Feld), die Szene lädt die Dächer
  neu — minimal-invasiv; per Idempotenz-AK belegt.
- **Walm-Einrückbetrag/Firsthöhe:** exakt nach §1-Formeln; die AK
  prüfen gegen analytische Erwartungswerte (kein Pixel-Vergleich).
- **Default-Maße bei Anlage:** Neigung/Überstand aus §3 (30°/500 mm);
  der Grundriss `b×t`/`base_z` ist Eingabe (kein Default).
- **Persistenz-Lücke (bewusst, 014c):** Dächer nur im Speicher — kein
  lebender Save-Pfad (wie 013b), kein §2.2-Risiko; 014c vor erstem
  Save-Use-Case.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (framework-frei, Totalität, ADR-0011);
  Risiko mittel (Dach-Geometrie-Mathematik — durch analytische AK
  gedeckt).

### Sub-Area: GUI-Adapter (`src/adapters/ui/`)

- **Modus:** GF; Dichte hoch (Regel E, `ViewModelPort`-Pull, Idempotenz);
  Risiko mittel (Szene um zweiten Element-Typ erweitert — additiv).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (LH-ID-Namen, analytische Orakel,
  Registrierung); Risiko niedrig.
