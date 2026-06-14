---
id: slice-016b
titel: Treppen implementieren — gerade einläufige Treppe (analytisches Stufen-Polyeder + Geländer)
status: in-progress
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-STR-001, LH-FA-STR-002, LH-FA-STR-003, LH-FA-STR-004]
adr_refs: [ADR-0001, ADR-0008, ADR-0009, ADR-0011]
---

# Slice 016b: Treppen implementieren — gerade einläufige Treppe

**Status:** done (2026-06-14). MR-006-Plan-Review gelaufen
([Report](../../../reviews/2026-06-14-slice-016b-plan.md) — **keine HIGH**; alle
tragenden Behauptungen gegen den realen Code verifiziert; MED-1/MED-2 +
LOW-1/2/3 eingearbeitet). Implementiert als **Zwei-Commit-Split** (i Domäne +
`stair_geometry` + Kern-AK / ii Integration); DoD vollständig, `make gates` grün
(112 Tests, Coverage 92,2 %). Closure-Notiz §8.

**Welle:** welle-2-bauteile (elfter Slice; zweiter der Treppen-Familie).

**Bezug:** LH-FA-STR-001..004 (slice-016a, Lastenheft 0.1.6),
`spezifikation.md` §1 `LH-FA-STR-001.a` (Treppen-Geometrie) + §3
(Stair-Wertebereiche). **Parametrisiert auf ADR-0011 (#6)** — neuer
Bauteil-Typ, **kein** neuer Grundsatz-ADR. ADR-0009 (analytisches Netz im
Kern, **kein OCC** — Präzedenz `roof_geometry`; das Netz fließt framework-frei
über den `ViewModelPort`), ADR-0008 (Notification), ADR-0001 (Kern führt,
framework-frei).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-14.

**Schnitt-Herkunft:** Implementierungs-Hälfte des Treppen-Strangs (Muster
013b/014b/015b). **Scope:** sichtbare, **in-memory** gerade einläufige Treppe
(Stufenfolge + Geländer). **Persistenz** (`stairs`-Tabelle) ist **slice-016c**.
**Geplante Zwei-Commit-Sequenz** (Muster 015b): **(i)** Domäne + `stair_geometry`
(reines analytisches Netz) + Kern-AK → **(ii)** Service + `ViewModelPort` +
`ModelChangedPort` + `EditStructurePort` + Viewer + Edit-Ops + Szenen-AK.

---

## 1. Ziel

Eine sichtbare gerade einläufige Treppe, die zwei Geschosse verbindet: eine
aufsteigende Stufenfolge (`step_count` Quader) vom Startpunkt in +x-Richtung,
mit Geländer auf Handlaufhöhe, in der 3D-Szene dargestellt und der
ADR-0008-Mechanik folgend. Der Kern bleibt framework-frei und **OCC-frei**
(analytisches Netz wie `roof_geometry`, ADR-0001/0009).

## 2. Definition of Done

- [x] **Domänen-Typen (pure Werte, `src/hexagon/model/`):** `Stair`
      (`StairId`, `from_storey_id`/`to_storey_id: StoreyId`,
      `StairType ∈ {Gerade}` (welle-2-Teilumfang, ein Wert; vorwärts-kompatibel
      für Podest/Wendel), `start: Point2D`, `width_mm`, `step_count: int`,
      `tread_mm`). **Kein `rise`-Feld** (abgeleitet) und **kein Geländer-Feld**
      (immer Teil der Treppe, spez. §1). `Building` gewinnt
      `std::vector<Stair> stairs`. Framework-frei (Regel A).
- [x] **Konstanten (`src/hexagon/model/constants.h`):** `kStairWidthMin/MaxMm`
      (800/2000), `kStairStepCountMin/Max` (2/30, `int`), `kStairTreadMin/MaxMm`
      (210/350), `kStairRiseMin/MaxMm` (140/200, **informativ — kein
      Klemmpunkt**, 016a-LOW-2 / spez. §3), `kStairRailingHeightMm` (900), `kDefaultStairWidthMm`
      (1000), `kDefaultStairStepCount` (15), `kDefaultStairTreadMm` (280) —
      spiegeln spez. §3 (Source Precedence: Spec gewinnt).
- [x] **Reine Kern-Geometrie (`src/hexagon/services/stair_geometry.{h,cpp}`,
      neu):** `stairMesh(const Stair&, double from_storey_height_mm)
      → TriangleMesh` — **analytisches Stufen-Polyeder**, **kein Port/OCC**
      (Muster `roofMesh`). **`rise = from_storey_height_mm / step_count`**
      abgeleitet (spez. §1); Stufe `i` (0-basiert) = Quader
      `x∈[start.x+i·tread, start.x+(i+1)·tread]`, `y∈[start.y, start.y+width]`,
      `z∈[0,(i+1)·rise]` (+x-Aufstieg, `base_z=0` welle-2-Ein-Geschoss-Annahme);
      **Geländer (STR-004):** dünnes Element entlang der Lauf-Seite(n) auf
      `kStairRailingHeightMm` über den Stufenoberkanten, der Stufenfolge folgend.
      **Total:** ungültige Parameter (`step_count < 1`, `width`/`tread <
      kGeometryToleranceMm`, `from_storey_height_mm ≤ kGeometryToleranceMm`) →
      **leeres** Netz (kein Wurf). Ggf. kleine Helfer (`stairRiseMm`,
      `stairRunLengthMm`).
- [x] **`ViewModelPort` um Treppen-Sicht:** `struct StairMesh{stair_id, mesh}` +
      `stairMeshes() → std::vector<StairMesh>` (Muster `roofMeshes`/`slabMeshes`,
      total — degenerierte Treppe → kein Eintrag). Vom `StructureEditService`
      implementiert (liest die `from_storey`-Höhe aus `building.storeys`).
- [x] **`ModelChangedPort` um `StairChanged`** (§5-`op`-Vokabular). **Geschoss-
      Bindung an `from_storey`** (spez. §1, begründet: Anker/`base_z` unten);
      `ModelChange.storey_id = from_storey_id`. **Keine `RoomsChanged`** (Treppen
      berühren die Raumerkennung nicht). `spec/spezifikation.md` §5/§8 ggf. um
      den `op` ergänzen (Mechanik-Doku).
- [x] **`EditStructurePort`-Operationen** (im Service): `addStair`
      (Prototyp: from/to-Geschoss, start, width, step_count, tread; Defaults
      `kDefaultStair…` bestücken den **UI-seitigen Prototyp**, nicht den Service,
      Muster `addRoof`/`addSlab` — LOW-3; `id` vom Service;
      **width/step_count/tread gegen §3 geklemmt**, `E-VAL-001`; **ungültige
      Spanne abgelehnt** — folgt dem **`addSlab`-Validierungsmuster**
      (`find_if`-Storey-Existenz-Prüfung; **nicht** dem `addRoof`-Muster, das die
      Storey nicht prüft — LOW-1), plus `from == to` und `from_storey`-Höhe ≤
      Toleranz → kein Wert), `setStairWidth` (`ParamResult`), `setStairStepCount`
      (**`(StairId,int) → ParamResult`**, `applied_mm` trägt den geklemmten
      Count; **`Clamped`/`Accepted`, nie `Rejected`** — int ist endlich; MED-1),
      `removeStair`. Jede erfolgreiche Mutation meldet `StairChanged`. **Kein
      Geländer-Op** (immer Teil der Treppe, welle-2 — spez. §1).
- [x] **Viewer folgt** (`ViewerScene`): hält Treppen-Netze (`StairId`-Map);
      auf `StairChanged` `stairMeshes()` neu laden, **idempotent** über den
      bestehenden Template-Helfer `reloadKeyed` (Roof+Slab → +Stair). Wand-/
      Dach-/Platten-Pfad additiv unberührt.
- [x] **AK-Tests mit `LH-FA-STR-*` im Namen** (Kern + Service + Szene),
      display-frei: **STR-001** (Netz-z-Span = `from_storey_height_mm` — Oberkante
      = obere Geschossebene **als Geschosshöhe, kein Elevation-Lookup** (`Storey`
      trägt keine Elevation, Ein-Geschoss-Annahme wie `slab_geometry`; LOW-2);
      `step_count` Stufen sichtbar; +x-Aufstieg); **rise abgeleitet**
      (`height/step_count`); **STR-002** (Stufenanzahl geklemmt → `Clamped`; mehr
      Stufen → mehr Quader); **STR-003** (Laufbreite geklemmt → breiteres Netz);
      **STR-004 — Geländer mit ZWEI Sonden (MED-2, gegen Spike-Fehlpass):** (a)
      Geländer-Oberkante z ≈ `Geschosshöhe + kStairRailingHeightMm` (±Toleranz —
      fängt die Handlaufhöhe) **und** (b) das Geländer **folgt dem Lauf**
      (Geländer-Vertices treten auch bei kleinem x auf / Geländer-Bounding-x ≈
      Lauflänge) — eine reine Max-z-Sonde ließe einen einzelnen hohen Spike durch;
      **Negative** (ungültige Spanne `from==to`/unbekanntes Geschoss → keine
      Treppe; `step_count < min` → **`Clamped`** (int, nie `Rejected`), kein
      leeres Krachen); **Folge-Meldung** (`StairChanged` an `from_storey`, **kein**
      `RoomsChanged`; Szene folgt idempotent); **Regression** (Wand-/Dach-/
      Platten-/ROM-/D3-Tests textlich unverändert grün — additiv, **keine**
      Port-Signatur-Migration).
- [ ] arch-check A–E grün (Regel A: Kern OCC-/Qt-frei; Regel E: Qt nur im
      Viewer); `make gates` grün; Closure-Notiz mit Lerneintrag; CHANGELOG
      (MR-004). **Nicht Teil:** Persistenz (`stairs`-Round-Trip → slice-016c);
      Material/Bewehrung; Podest-/U-/L-/Wendeltreppe; freie Rotation (Schema
      ohne Richtungs-Spalte).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{stair}.h` | neu | `Stair`, `StairId`, `StairType` (pure Werte) |
| `src/hexagon/model/building.h` | ändern | `std::vector<Stair> stairs` |
| `src/hexagon/model/constants.h` | ändern | `kStair…`-Wertebereiche/Defaults (spez. §3) |
| `src/hexagon/services/{stair_geometry}.{h,cpp}` | neu | analytisches Stufen-/Geländer-Netz, rise abgeleitet (pur, kein Port) |
| `src/hexagon/ports/driving/view_model_port.h` | ändern | `StairMesh` + `stairMeshes()` |
| `src/hexagon/ports/driving/edit_structure_port.h` | ändern | Stair-Operationen |
| `src/hexagon/ports/driven/model_changed_port.h` | ändern | `StairChanged`-`op` |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Stairs halten, Mesh über `stair_geometry` (from_storey-Höhe), `StairChanged`, Klemmung |
| `src/adapters/ui/viewer_scene.{h,cpp}` | ändern | Treppen-Netze (`StairId`-Map), `StairChanged` → `stairMeshes()` via `reloadKeyed` (idempotent) |
| `spec/spezifikation.md` | ggf. ändern | §5-Span-`op` `StairChanged`; §8-Historie (falls Mechanik nachgezogen) |
| `tests/hexagon/{test_stair_geometry}.cpp` | neu | Kern-AK (rise abgeleitet, z-Span, Stufenanzahl, Geländerhöhe, leer bei degeneriert) |
| `tests/adapters/test_viewer_scene.cpp` | ändern | Szene folgt `StairChanged` idempotent |
| `tests/hexagon/test_structure_edit_service.cpp` o. eigene Datei | ggf. ändern | Stair-Edit-Ops/Klemmung/ungültige-Spanne/Folge-Meldung |
| `tests/CMakeLists.txt` | ändern | neue Testdatei(en) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/2026-06-14-slice-016b-plan.md` | neu | MR-006-Report |

## 4. Trigger

- slice-016a done ✓ (Lastenheft 0.1.6 + Spec §1/§3).
- MR-006-Plan-Review vor Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-016c**
  (Treppen-Persistenz) startbar; danach **Welle-2-Closure** (STR ist das letzte
  Bauteil). Empfohlen vor Closure: Code-Review (geometrielastig — Praxis 3×+
  nach welle-1, fand bei 013b/014b/015b je 1 HIGH trotz grüner Gates).

## 6. Risiken und offene Punkte

- **`rise` abgeleitet braucht die `from_storey`-Höhe:** `stair_geometry` nimmt
  sie als **reinen Parameter** (Kern bleibt pur); der **Service** schlägt sie
  über `from_storey_id` in `building.storeys` nach. Unbekanntes `from_storey` →
  `addStair` abgelehnt (kein Wert). `rise = Höhe/step_count`; `step_count ≥ 1`
  durch Klemmung gesichert (keine Division durch 0).
- **Zwei-Geschoss-Spanne:** `from_storey` (unten) liefert Höhe und `base_z`
  (=0 welle-2); `to_storey` ist Ziel-Semantik (für 016c-Persistenz `restrict`).
  Validierung: `from != to`, beide existieren. **Mehr-Geschoss-Stapelung
  (echte Elevation) später** — die Ein-Geschoss-Annahme ist dieselbe wie bei
  `slab_geometry`.
- **`step_count` ist `int`, `ParamResult` ist `double`-basiert (MED-1
  entschieden):** `setStairStepCount(StairId,int) → ParamResult`; `applied_mm`
  trägt den geklemmten Count (lossless für `[2,30]`), Status `Clamped`/
  `Accepted`, **nie `Rejected`** (int ist endlich) — keine int-Sondervariante,
  hält die `ParamResult`-Familie konsistent (Muster `setWallThickness`).
- **Geländer-AK darf kein Spike sein (MED-2):** „Geländer vorhanden" wird
  **zweifach** belegt — Handlaufhöhe (z ≈ `Geschosshöhe + kStairRailingHeightMm`)
  **und** Lauf-Folge (Geländer auch bei kleinem x / Bounding-x ≈ Lauflänge). Eine
  reine Max-z-Sonde wäre die Klasse „grünes Gate, aber Geometrie-Defekt"
  (013b/014b/015b je 1 HIGH im Code-Review) — daher die schärfere Doppelsonde.
- **`addStair`-Validierung folgt `addSlab`, nicht `addRoof` (LOW-1):** `addRoof`
  prüft die Storey-Existenz **nicht** (speichert nur `prototype.storey_id`),
  `addSlab` prüft sie per `find_if`. `addStair` braucht zwingend die
  `addSlab`-Variante (sonst schlägt die Negative-AK „unbekanntes Geschoss" fehl)
  plus `from != to`.
- **Geländer-Geometrie (analytisch):** dünnes Element entlang der Lauf-Seite(n)
  auf `kStairRailingHeightMm`; welle-2 wählt eine feste Variante (z. B. beidseitig
  oder einseitig) — die AK prüft nur, dass das Geländer **vorhanden** ist
  (Netz-z reicht über die oberste Stufe hinaus). Kein persistierter Eigenzustand
  (016c-relevant: nichts zu round-trippen).
- **Analytisches Netz statt OCC (wie Dach):** anders als die Platte (OCC-Boolean)
  braucht die Treppe **keinen** `GeometryKernelPort`/OCC — reine Quader-Dreiecke
  im Kern (ADR-0009-Vertrag, Präzedenz `roof_geometry`). Kein TKBO-/Link-Risiko.
- **`StairChanged` an `from_storey` (spez. §1, MED-1 aus 016a):** `ViewerScene`
  lädt `stairMeshes()` **projektweit** (eine Treppe ist geschossübergreifend) —
  Idempotenz über `reloadKeyed`, Zähler = tatsächlich geänderte Netze.
- **Persistenz-Lücke (bewusst, 016c):** Treppen nur im Speicher (kein lebender
  Save-Pfad, kein §2.2-Risiko).
- **+x-Richtung fix (Schema ohne Richtungs-Spalte):** freie Rotation offen
  (spez. §1 Teilumfang) — die AK nimmt +x als gegeben an.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (framework-/OCC-frei, analytisches Netz,
  abgeleitete rise, Totalität); Risiko mittel (Stufen-/Geländer-Geometrie +
  Zwei-Geschoss-Spanne — durch analytische AK gedeckt).

### Sub-Area: GUI-Adapter (`src/adapters/ui/`)

- **Modus:** GF; Dichte hoch (Regel E, ViewModelPort-Pull, Idempotenz via
  `reloadKeyed`); Risiko niedrig (vierter Szenen-Element-Typ — additiv).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (LH-ID-Namen, analytische Orakel, Registrierung);
  Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **Domäne + Kern-Geometrie:** `model::Stair` (gerade einläufig,
  `StairType::Gerade`; from/to_storey, start, width, step_count, tread — **kein**
  rise-/Geländer-Feld) + `Building.stairs`; `constants.h` `kStair*` (spez. §3).
  `stair_geometry` (pur, **kein OCC/Port** — Muster `roof_geometry`):
  `stairMesh` (step_count solide Stufen-Quader + **beidseitiges** Geländer auf
  Handlaufhöhe), `stairRiseMm` (= Geschosshöhe/step_count, Division-Schutz),
  `stairRunLengthMm`; **total** (degeneriert → leeres Netz). arch-check Regel A
  (Kern OCC-/Qt-frei).
- **Integration:** `ViewModelPort.stairMeshes()` (**projektweit**, total);
  `ModelChangedPort.StairChanged` (an **`from_storey`** gebunden, MED-1 aus 016a);
  `EditStructurePort` addStair/setStairWidth/setStairStepCount/removeStair —
  width/tread/step_count gegen §3 geklemmt (`E-VAL-001`); **`setStairStepCount`
  `(StairId,int)→ParamResult`, nie `Rejected`** (MED-1); **ungültige
  Zwei-Geschoss-Spanne abgelehnt** (`from==to`/unbekanntes Geschoss/Höhe ≤
  Toleranz — `addSlab`-Validierungsmuster, LOW-1); kein `RoomsChanged`.
  `ViewerScene` lädt Treppen auf `StairChanged` über den gemeinsamen
  `reloadKeyed`-Helfer (Roof+Slab+Stair) idempotent neu.
- **7 AK-Tests `LH-FA-STR-*`:** Kern (rise abgeleitet + verbindet Geschosse,
  z-Span = Geschosshöhe **als Höhe, kein Elevation-Lookup** (LOW-2); Stufenanzahl;
  Laufbreite; **Geländer-Doppelsonde** — Handlaufhöhe **und** folgt dem Lauf, kein
  Spike, MED-2; Negative/Totalität) + Szene (folgt+verbindet+idempotent,
  Entfernen leert; Klemmung step_count/width, ungültige Spanne, `StairChanged`-
  nicht-`Rooms`). Wand-/Dach-/Platten-/ROM-/D3-Tests textlich unverändert grün
  (additiv, keine Port-Signatur-Migration).
- **`make gates` grün** (2026-06-14): docs-check 0, gate-consistency,
  arch-check A–E, lint 0 + suppression-gate, **Tests 112/112** (zuvor 105 →
  Phase i 110 → Phase ii 112), Coverage **92,2 %**. Zwei-Commit-Split.
- **Keine Spec-/ADR-Änderung:** §1 `LH-FA-STR-001.a` + §3 lagen aus 016a
  vollständig vor (Präzedenz 014b: Dach-Implementierung änderte die Spec nicht);
  das Geländer „beidseitig" ⊆ spez. „Lauf-Seite(n)". Persistenz ist 016c.

**Lerneintrag:**

- **Analytisches Bauteil < OCC-Bauteil im Aufwand:** die Treppe ist geometrisch
  reicher als die Platte (Zwei-Geschoss-Spanne, abgeleitete rise, Geländer),
  aber **einfacher zu implementieren** — rein analytische Quader im Kern
  (`appendBox`, Muster `roof_geometry`) statt OCC-Boolean/Port/TKBO. Das
  MR-006-INFO-1 („016b netto einfacher als 015b") bestätigte sich; kein
  Geländer-Split nötig (gleiche Mesh-Funktion/Pfad).
- **Geländer-Doppelsonde (MED-2) verhinderte einen Spike-Durchrutscher vorab:**
  die schärfere AK (Handlaufhöhe + Lauf-Folge) wurde direkt umgesetzt — die
  013b/014b/015b-Klasse „grünes Gate, aber Geometrie-Defekt" wurde diesmal
  **im Plan-Review** statt erst im Code-Review gefangen.
- **`reloadKeyed` trägt den vierten Element-Typ ohne Änderung:** der
  Template-Helfer (Roof+Slab seit 015b) nahm Stair mit einer Lambda-Zeile auf —
  die Idempotenz-Mechanik skaliert mit den storey-/projektbezogenen Bauteilen.

**Restrisiko / Nachfolge:** **slice-016c** (Treppen-Persistenz `stairs`-Tabelle,
from/to_storey-Spanne `restrict`; rise wird abgeleitet, nicht round-getrippt —
Schema trägt `rise_mm`, das aus Geschosshöhe/step_count folgt). **Empfohlen vor
Welle-Closure:** geometrielastiges Code-Review (Praxis 3×+ nach welle-1).
Offen: Podest-/U-/L-/Wendeltreppe, freie Rotation (Schema ohne Richtungs-Spalte),
echte Mehr-Geschoss-Stapelung (Elevation). STR ist das **letzte welle-2-Bauteil**
→ danach Welle-2-Closure (unabhängige Verifikation + `done/welle-2-results.md`
inkl. Carveout-Audit).
