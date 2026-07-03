---
id: slice-013b
titel: Türen + Fenster implementieren — Wandöffnung (Schnitt-Prismen)
status: done
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-DOR-001, LH-FA-DOR-002, LH-FA-DOR-003, LH-FA-DOR-004, LH-FA-WIN-001, LH-FA-WIN-002, LH-FA-WIN-003, LH-FA-WIN-004, LH-FA-WIN-005]
adr_refs: [ADR-0001, ADR-0002, ADR-0008, ADR-0011]
---

# Slice 013b: Türen + Fenster implementieren — Wandöffnung

**Status:** done (2026-06-13). MR-006-Plan-Review gelaufen (keine HIGH,
4 MED/2 LOW eingearbeitet); DoD vollständig, `make gates` grün
(80 Tests, Coverage 93,3 %). Closure-Notiz §8.

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-013b-plan.md) (M/L-IDs) —
keine HIGH; M1 (Persistenz-Lücke kein §2.2-Verstoß, Closure-Auflage),
M2 (Split als Zwei-Commit-Sequenz), M3 (Index zweigeteilt), M4
(ausgelassene Schema-Felder benennen) eingearbeitet.

**Welle:** welle-2-bauteile (zweiter Slice).

**Bezug:** LH-FA-DOR-001..004, LH-FA-WIN-001..005 (Lastenheft 0.1.3,
slice-013a). **Parametrisiert auf ADR-0011** (Hosting/Öffnungs-Modell,
Schnitt-Prismen, `WallGeometryChanged`-Wiederverwendung, Bauteil-
Erweiterungs-Muster). ADR-0002 (OCC-Boolean-Backend), ADR-0008
(Notification), ADR-0001 (Schichtung, Port-Signatur = Kern-Hoheit).
Spec: `spezifikation.md` §1 LH-FA-DOR-004.a/WIN-005.a + §3
Wertebereiche.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Implementierungs-Hälfte des Tür/Fenster-Strangs
(Muster 009a/b, 010a/b, 011a/b). **Scope-Grenze (Welle-Sizing):** dieser
Slice liefert das **sichtbare, in-memory** Verhalten (Öffnung als
Wand-Hohlraum, Viewer folgt, AK-Tests). Die **Persistenz** der Öffnungen
(`openings`/`doors`/`windows`-Round-Trip, ADR-0006-Schema liegt bereit)
ist **slice-013c** — bis dahin sind Öffnungen nur im Speicher (kein
stiller Datenverlust behauptet; explizit in der Closure benannt — und
013c **vor** dem ersten lebenden Save-Use-Case, M1). **Geplante
Zwei-Commit-Sequenz** (M2, nicht erst bei Sitzungs-Kippen): **(i)**
Domäne + Port-Signatur-Migration + Geometrie-Cut (verhaltensneutral mit
leeren Cutouts, identische Orakel, einzeln committbar) → **(ii)**
Service-Öffnungs-Logik + Viewer + AK-Tests.

---

## 1. Ziel

Eine an einer Wand platzierte Tür/ein Fenster bricht die Wand
automatisch durch (Öffnung), in der 3D-Darstellung sichtbar, und folgt
Parameter-/Positions-Änderungen ohne Benutzer-Refresh — auf dem
ADR-0011-Modell (Kern liefert Schnitt-Prismen, `GeometryKernelPort`
subtrahiert per OCC-Boolean). Der Kern bleibt framework-frei (ADR-0001);
Raumerkennung und Footprint/Eckenschluss (slice-012) bleiben unberührt.

## 2. Definition of Done

- [x] **Domänen-Typen (pure Werte, `src/hexagon/model/`):**
      `Opening` (`OpeningId`, `WallId wall_id`, `OpeningKind ∈ {Door,
      Window}`, `offset_mm`, `width_mm`, `height_mm`, `sill_height_mm`,
      Tür-`SwingDirection ∈ {Left, Right}` für den Anschlag,
      LH-FA-DOR-003); `Building` gewinnt `std::vector<Opening> openings`.
      Framework-frei (arch-check Regel A).
- [x] **`GeometryKernelPort` um Schnitt-Prismen erweitert** (ADR-0011 (b),
      Port-Signatur = ADR-0001-Kern-Hoheit): ein `model::CutPrism`
      (`Footprint polygon` + `z_min_mm`/`z_max_mm`) als reiner
      Geometrie-Wert; `extrudeFootprint`/`tessellateFootprint` nehmen
      eine `const std::vector<model::CutPrism>& cutouts` entgegen
      (leere Liste = Verhalten wie vor diesem Slice). Adapter
      (`OccGeometryAdapter`) subtrahiert die Prismen per
      `BRepAlgoAPI_Cut`; total (E-GEO-002 bei Fehlschlag, kein OCC-Leck,
      Regel C). **Signatur-Migration** der bestehenden Aufrufer/Doubles
      mit **identischen Orakeln bei leeren Cutouts** (Regressions-Aussage
      wie slice-012 W3-P1, zweigeteilt + per `make test` belegt — keine
      behauptete Testzahl).
- [x] **`EditStructurePort`-Operationen** für Öffnungen, im
      `StructureEditService` implementiert: platzieren (`addDoor`/
      `addWindow` an Wand + Position), verschieben (Offset),
      Parameter (Breite/Höhe/Brüstung/Anschlag), entfernen — Parameter
      gegen §3 geklemmt (`E-VAL-001`); **Platzierungs-Validierung:**
      Position so geklemmt, dass `[offset, offset+width]` in der
      Wandlänge liegt; passt die Mindestbreite nicht oder fehlt die
      Wirtswand → abgelehnt (keine verwaiste Öffnung, kein Durchbruch
      außerhalb). Öffnungs-Oberkante auf Wandhöhe geklemmt.
- [x] **Öffnungs-Geometrie im Kern, total + transaktional:** der Service
      berechnet je Wand aus ihren Öffnungen die Schnitt-Prismen (Quader
      quer zur Wand, `[offset,offset+width]` × volle Stärke ×
      `[sill,sill+height]`, Überstand ≥ Toleranz) und baut das
      Wand-Solid als Footprint-Extrusion **minus** Prismen — **vor dem
      Commit**; schlägt der Boolean fehl, bleibt das Modell unverändert,
      keine Meldung (Muster slice-012). `wallMesh`/`wallMeshes` (ViewModel)
      liefern das ausgeschnittene Netz.
- [x] **Folge-Meldung:** Öffnungs-Mutation meldet
      `op = WallGeometryChanged` für die **Wirtswand** (kein neuer `op`,
      ADR-0011 (4)); **keine** `RoomsChanged` (Öffnung ändert weder
      Wandachse noch Stärke — Raumerkennung/Footprint unberührt,
      ROM-/WAL-006-AK-Tests bleiben textlich grün). Abgelehnte/entfernte→
      definiertes Melde-Verhalten (entfernen meldet
      `WallGeometryChanged`, abgelehnt meldet nicht).
- [x] **Viewer folgt** (`ViewerScene`): die `WallGeometryChanged`-
      Meldung der Wirtswand zieht das ausgeschnittene Wand-Netz nach
      (Pull + idempotentes Ersetzen — bestehender Pfad seit slice-012;
      Code-Änderung nur falls nötig, sonst per AK-Test belegt).
- [x] **AK-Tests mit `LH-FA-DOR-*`/`LH-FA-WIN-*` im Namen** (Kern gegen
      `AnalyticGeometry`-Double mit Cutout-Volumen-Subtraktion + OCC-
      Adapter + Szene), display-frei:
      **Happy** (Tür/Fenster platziert → Wand-Solid-Volumen um das
      Öffnungs-Volumen verringert; Fenster-Brüstung → Öffnung beginnt
      oberhalb, Wand darunter erhalten);
      **Boundary** (Breite/Höhe/Brüstung am Grenzwert akzeptiert,
      außerhalb geklemmt; Öffnung über Wandende → Position geklemmt;
      Öffnung höher als Wand → auf Wandhöhe geklemmt);
      **Negative** (ohne Wirtswand → abgelehnt; entfernte Öffnung →
      Wandvolumen wieder voll; Wand zu kurz für Mindestbreite →
      abgelehnt);
      **Folge-Meldung** (Öffnungs-Mutation → genau die Wirtswand
      `WallGeometryChanged`, kein `RoomsChanged`; idempotenter
      Szenen-Endzustand);
      **Fehlerfall-Transaktion** (Cut-Wurf über steuerbares Double →
      Modell/Solids/Szene unverändert, keine Meldung);
      **Regression** (ROM-/WAL-006-/D3-Tests ohne Cutout textlich grün;
      migrierte Doubles/OCC-Tests mit identischen Orakeln bei leeren
      Cutouts).
- [x] **`spec/spezifikation.md` §3** ggf. um Defaults (Default-Tür-/
      Fenster-Maße bei Anlage) ergänzt, falls die Anlage Defaults
      braucht (Spec-Drift-Disziplin, Begründung in Closure); `make gates`
      grün; arch-check A–E; Closure-Notiz mit Lerneintrag; CHANGELOG-
      Slice-Eintrag (MR-004); ADR-0011-Folgepflicht im Index auf
      „**erfüllt (Geometrie/Verhalten) → Persistenz slice-013c**"
      präzisiert. **Nicht Teil dieser DoD:** Persistenz der Öffnungen
      (slice-013c), eigenes Tür-Blatt/Fenster-Rahmen-Solid + 2D-Öffnungs-
      Darstellung (ADR-0011 Re-Eval-Trigger).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{opening}.h` | neu | `Opening`, `OpeningId`, `OpeningKind`, `SwingDirection` (pure Werte) |
| `src/hexagon/model/{cut_prism}.h` | neu | `CutPrism` (Footprint + z-Bereich) — reiner Geometrie-Wert für den Port |
| `src/hexagon/model/building.h` | ändern | `std::vector<Opening> openings` |
| `src/hexagon/model/constants.h` | ändern | Tür-/Fenster-/Brüstungs-Konstanten (spez. §3) + ggf. Default-Maße |
| `src/hexagon/ports/driven/geometry_kernel_port.h` | ändern | `extrudeFootprint`/`tessellateFootprint` um `const std::vector<CutPrism>& cutouts` |
| `src/hexagon/ports/driving/edit_structure_port.h` | ändern | Öffnungs-Operationen (add/move/setParam/remove); `ParamResult`-Wiederverwendung |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Öffnungs-Logik: Cutter-Prismen, Wand-Solid mit Cut, transaktional, `WallGeometryChanged` |
| `src/hexagon/services/{opening_geometry}.{h,cpp}` | neu | Cutter-Prisma-Berechnung aus `Opening` + Wand (analog `wall_footprint`, pure Domäne) |
| `src/adapters/geometry/occ_geometry_adapter.{h,cpp}` | ändern | `BRepAlgoAPI_Cut` der Prismen; Signatur-Migration |
| `src/adapters/ui/viewer_scene.cpp` | ggf. ändern | `WallGeometryChanged` trägt schon — nur falls die Wirtswand-Eigen-Op (nicht Nachbar) einen Pfad braucht | <!-- d-check:ignore (historisch: Pfad vor slice-029-Move nach ui/view/) -->
| `tests/hexagon/analytic_geometry_double.h` | ändern | Cutout-Volumen-Subtraktion im Double (Orakel) |
| `tests/hexagon/{test_openings}.cpp` | neu | Kern-AK-Tests DOR/WIN (Happy/Boundary/Negative/Folge/Fehlerfall) |
| `tests/adapters/test_occ_geometry_adapter.cpp` | ändern | Cutout-Subtraktion gegen analytisches Volumen; leere Cutouts = altes Orakel |
| `tests/adapters/test_viewer_scene.cpp` | ändern/neu | Szene folgt Öffnungs-`WallGeometryChanged` |
| `tests/hexagon/test_structure_edit_service.cpp`, `test_wall_footprint.cpp`, `test_room_detection.cpp` | ggf. ändern | Signatur-Migration (leere Cutouts), identische Orakel; Regressions-Aussage W3-P1 |
| `tests/CMakeLists.txt` | ändern | neue Testdateien registrieren |
| `src/main.cpp` | ggf. ändern | nur falls die Composition Root eine neue Verdrahtung braucht (erwartet: nein) |
| `spec/spezifikation.md` | ggf. ändern | §3 Default-Maße, falls Anlage Defaults braucht; §8-Historie |
| `docs/plan/adr/README.md` | ändern | ADR-0011-Folgepflicht-Status (Geometrie/Verhalten erfüllt → Persistenz 013c) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/{2026-06-13-slice-013b-plan}.md` | neu | MR-006-Report |

## 4. Trigger

- slice-013a done ✓ (ADR-0011 `Accepted`, Lastenheft 0.1.3, Spec §1/§3).
- MR-006-Plan-Review vor Implementierungs-Start (HIGH blockiert).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-013c**
  (Öffnungs-Persistenz) wird startbar; die übrigen Welle-Bauteile
  (ROF/SLB/FND/STR) folgen der ADR-0011-Leitplanke (#6).

## 6. Risiken und offene Punkte

- **Port-Signatur-Migration (verhaltensneutral):** Die Erweiterung um
  `cutouts` berührt alle `extrudeFootprint`/`tessellateFootprint`-
  Aufrufer (Service x4, OCC-Adapter, `AnalyticGeometry`-Double) und
  -Tests. Risiko wie slice-012 (W3-P1): die Regressions-Aussage muss
  **zweigeteilt** sein — unberührte Tests textlich grün, migrierte mit
  identischen Orakeln bei leeren Cutouts; per `make test` belegt, keine
  behauptete Zahl. Split-Hälfte (i) ist genau diese verhaltensneutrale
  Migration (einzeln grün committbar).
- **OCC-Boolean-Robustheit:** `BRepAlgoAPI_Cut` kann an Tangential-/
  Rand-Fällen (Öffnung exakt an Wandkante) degenerieren → muss als
  `E-GEO-002` neutral gefangen werden (kein OCC-Typ/-Wurf nach außen,
  Regel C); der Überstand der Prismen ≥ `GEOMETRY_TOLERANCE_MM` (spez.
  §1) hält den Schnitt sauber. Fehlerfall-AK (steuerbares Double) prüft
  die Transaktion, nicht OCC selbst.
- **Persistenz-Lücke (bewusst, slice-013c):** Öffnungen sind nach diesem
  Slice **nur im Speicher** — Speichern/Laden lässt sie fallen, bis
  013c die `openings`/`doors`/`windows`-Abbildung liefert. Das ist ein
  **dokumentierter Increment**, kein stiller Datenverlust: die Closure
  benennt es, und 013c steht als Folge-Trigger. (Abwägung: Persistenz
  jetzt würde den Slice über die Review-Sitzungs-Größe heben.)
- **Default-Maße bei Anlage:** Braucht `addDoor`/`addWindow` Default-
  Breite/Höhe (wie `kDefaultWallThicknessMm`)? Falls ja, sind das neue
  §3-Defaults (Spec-Drift im Slice, Begründung in Closure) — nicht ad
  hoc im Code (Präzedenz slice-003a `DEFAULT_WALL_THICKNESS_MM`).
- **Anschlag ohne Türblatt:** Der Anschlag (`SwingDirection`,
  LH-FA-DOR-003) ist in welle-2 gespeicherte, abfragbare Eigenschaft —
  ohne eigenes Türblatt-Solid (ADR-0011 Re-Eval). Der AK prüft die
  beobachtbare Eigenschaft (Query), nicht ein gerendertes Blatt.
- **Bewusst ausgelassene Schema-Felder (M4):** `swing_angle_deg`,
  `is_external` (Tür) und `frame_material`/`glazing_type`/`u_value`
  (Fenster) aus dem ADR-0006-Schema sind **nicht** Teil des
  welle-2-Domänenmodells — sie kommen mit eigenem Tür-/Fenster-Solid
  bzw. Material (ADR-0011 Re-Eval / welle-3). In der Closure benennen,
  damit kein stiller Lastenheft-Deckungs-Gap entsteht (LH-FA-DOR-003
  verlangt nur den Anschlag als beobachtbare Eigenschaft).
- **Persistenz-Reihenfolge (M1):** slice-013c (Öffnungs-Persistenz) muss
  **vor** dem ersten lebenden Speicher-Use-Case abgeschlossen sein
  (heute ist `SqliteProjectRepository` nicht in `main` verdrahtet, kein
  Save-Pfad — daher kein §2.2-Datenverlust). Closure-Auflage.
- **Raumerkennung/Footprint:** ADR-0011 (5) — Öffnung ändert weder
  Achse noch Stärke; die bestehenden ROM-/WAL-006-AK-Tests sind der
  Sensor, dass nichts kippt (bleiben textlich unverändert grün).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (framework-frei, Port-Signatur=Kern-Hoheit,
  Totalität/Transaktion, ADR-0011); Risiko mittel (Geometrie-Mathematik
  der Cutter-Prismen — durch analytische AK gedeckt).

### Sub-Area: Geometrie-Adapter (`src/adapters/geometry/`)

- **Modus:** GF; Dichte hoch (Regel C, kein OCC-Leck, E-GEO-002-Totalität);
  Risiko mittel (`BRepAlgoAPI_Cut`-Robustheit — Fehlerfall-AK + Überstand).

### Sub-Area: GUI-Adapter (`src/adapters/ui/`)

- **Modus:** GF; Dichte hoch (Regel E, ViewModelPort-Pull, Idempotenz);
  Risiko niedrig (bestehender `WallGeometryChanged`-Pfad).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (LH-ID-Namen, gemeinsames Double mit
  Cutout-Orakel, Registrierung); Risiko mittel (Orakel-Erweiterung —
  durch identische Leer-Cutout-Werte gegengeprüft).

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **DoD vollständig, `make gates` grün** (2026-06-13): docs-check 0,
  gate-consistency, arch-check **A–E** (ADR-0001/0002 — OCC inkl.
  `BRepAlgoAPI_Cut`/`TKBO` bleibt im Geometrie-Adapter, Regel C grün),
  lint 0 + suppression-gate, **Tests 80/80**, **Coverage 93,3 % lines**
  (1064/1140) / 93,5 % functions (Schwelle 70 %).
- **13 neue AK-Tests mit `LH-FA-DOR-*`/`LH-FA-WIN-*` im Namen**
  (Kern gegen `AnalyticGeometry` mit Cutout-Orakel + OCC-Adapter +
  Szene): Happy (Tür/Fenster verringern das Wandvolumen; Fenster-
  Brüstung), Boundary (Breite/Höhe geklemmt, Position auf Wandlänge,
  Öffnung auf Wandhöhe geklemmt), Negative (ohne Wirtswand abgelehnt,
  Wand zu kurz abgelehnt, entfernte Tür schließt die Wand), Verschieben
  (DOR-002, geklemmt), Brüstung nur Fenster (Tür rejected), Anschlag
  gesetzt (Query), Folge-Meldung (genau Wirtswand-`WallGeometryChanged`,
  **kein** `RoomsChanged`), Fehlerfall-Transaktion (werfendes Double →
  Modell/Solids unverändert, keine Meldung) + 2 OCC-Adapter-Tests (reale
  boolesche Subtraktion) + 1 Viewer-Szene-AK (Öffnung folgt).
- **Umsetzung deckungsgleich mit ADR-0011 (a)–(f):** `model::Opening`
  mit `wall_id`-Referenz; `model::CutPrism` (Polygon + z) vom Kern
  (`services/opening_geometry`), `GeometryKernelPort` um `cutouts`
  erweitert (Port-Signatur = Kern-Hoheit), `OccGeometryAdapter`
  subtrahiert per `BRepAlgoAPI_Cut` (fuzzy, kein OCC-Leck);
  total/transaktional (E-GEO-002 → Modell unverändert);
  `WallGeometryChanged` der Wirtswand (kein neuer `op`); Raumerkennung/
  Footprint unberührt.
- **Regressions-Aussage (W3-P1) eingehalten:** Die Port-Signatur-
  Migration (`cutouts`) ist verhaltensneutral bei leerer Liste — die
  ROM-/WAL-006-/D3-Tests ohne Öffnung blieben textlich unverändert grün;
  migrierte Doubles/OCC-Tests behalten identische Leer-Cutout-Orakel.
  Beleg: `make test` 80/80 (zuvor 63).
- **Keine Schema-Schärfung** (ADR-0006 trägt openings/doors/windows
  bereits); `make schema-check` im Strang nicht ausgelöst. Neue §3-
  Defaults (`DEFAULT_DOOR_*`/`DEFAULT_WINDOW_*`) als Spec-Drift im Slice
  (Muster `DEFAULT_WALL_THICKNESS_MM`).

**Lerneintrag:**

- **OCC-Boolean braucht ein eigenes Toolkit:** `BRepAlgoAPI_Cut`/
  `BOPAlgo` linken nur mit **`TKBO`** (Boolean Operations) — der
  Link-Fehler kam erst beim Composition-Root-Link, nicht beim Adapter-
  Kompilieren. Lehre: bei neuer OCC-Funktionsklasse die nötige
  `TK*`-Library gleich in `src/adapters/CMakeLists.txt` mitführen
  (1× kategorisiert; Build-Gate fängt es, aber spät). Kein neues
  apt-Paket (gleiches `libocct`), daher kein `make versions`-Nachzug.
- **Verhaltensneutrale Signatur-Migration zahlt sich aus:** der
  `cutouts`-Default „leer = Stand vor slice-013b" machte die Migration
  von 5 Aufruf-Stellen + 2 Implementierern + Doubles risikoarm (alle
  Alt-AK textlich grün) — Bestätigung der slice-012-W3-P1-Disziplin
  (2. Vorkommen).

**Restrisiko / Nachfolge / Auflagen:**

- **slice-013c (Öffnungs-Persistenz)** wird startbar — und ist **Auflage
  (M1): vor dem ersten lebenden Speicher-Use-Case** abzuschließen.
  Aktuell sind Öffnungen nur im Speicher (kein lebender Save-Pfad:
  `SqliteProjectRepository` ist nicht in `main` verdrahtet) — daher
  **kein** §2.2-Datenverlust, aber die Lücke ist hier bewusst benannt.
- **Bewusst ausgelassen (M4, welle-2-Scope):** `swing_angle_deg`,
  `is_external`, `frame_material`/`glazing_type`/`u_value` (ADR-0006-
  Schema-Felder) sowie eigenes Tür-Blatt/Fenster-Rahmen-Solid und die
  2D-Öffnungs-Darstellung (ADR-0011 Re-Eval-Trigger).
- **Welle-Leitplanke aktiv:** ROF/SLB/FND/STR folgen dem ADR-0011-
  Bauteil-Erweiterungs-Muster (#6).

**Nachtrag Code-Review (2026-06-13,
[Report](../../../reviews/2026-06-13-slice-013b-code-review.md)):**
unabhängiges Code-Review (2 Linsen) nach Closure. Linse B PASS (keine
HIGH). Linse A: **1 HIGH (H1)** — der von `spezifikation.md` §1
geforderte **laterale** Cutter-Überstand war nur in Z realisiert (Cutter
lateral bündig → koplanar, allein fuzzy-gestützt). Behoben: voller
Überstand im Kern (`OPENING_CUT_OVERSHOOT_MM`, spec-konform), Adapter
dumb; Volumen-Orakel klippt jetzt `Footprint ∩ Cutter` (M1);
Totalitäts-Tests (M2) + Zwischen-Shape-Null-Check (L1) + Diagonal-Wand-
OCC-AK. `make gates` grün, **82 Tests**, Coverage 93,6 %.
**Lerneintrag:** ein grünes Gate ≠ Spec-Treue — das Code-Review fand
eine Spec-Abweichung mit realem Robustheitsrisiko, die die
(achsenparallelen) Tests nicht trafen (Modul-11-Beleg). Praxis-Kandidat:
*geometrielastige Slices vor Welle-Closure code-reviewen, nicht erst in
der Welle-Verifikation* (2. Vorkommen nach welle-1).
