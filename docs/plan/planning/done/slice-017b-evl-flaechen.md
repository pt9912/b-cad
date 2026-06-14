---
id: slice-017b
titel: Auswertung Flächen — EvaluatePort + EVL-001/003 (Grundfläche/Wohnfläche, Aggregation)
status: done
welle: welle-3-auswertung
lastenheft_refs: [LH-FA-EVL-001, LH-FA-EVL-003]
adr_refs: [ADR-0001, ADR-0007, ADR-0012]
---

# Slice 017b: Auswertung Flächen — EvaluatePort + EVL-001/003

**Status:** done (`make gates` grün, 122 Tests). MR-006-Plan-Review gelaufen
([Report](../../../reviews/2026-06-14-slice-017b-plan.md) — **keine HIGH**;
Schnitt bestätigt; MED-1 Per-Raum-Flächen im `AreaReport`, MED-2 `make
arch-check`-Wortlaut, LOW-1 read-only-stabil, LOW-2 Loch-Ring-Test eingearbeitet).
Closure-Notiz siehe §8.

**Welle:** welle-3-auswertung (zweiter Slice; erste Auswertungs-Implementierung).

**Bezug:** LH-FA-EVL-001 (Flächenberechnung) + LH-FA-EVL-003 (Wohnflächen-
berechnung), spez. §1 `LH-FA-EVL-001.a`. **Implementiert ADR-0012-Folgepflicht**
(EvaluatePort) für den **Flächen-Teil**. ADR-0007 (`Room.net_area_mm2` — die
NETTO-Fläche ist in der Raumerkennung bereits berechnet), ADR-0001 (Auswertung =
reine Query/Driving-Port).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-14.

**Schnitt-Herkunft:** Flächen-Hälfte des Auswertungs-Strangs. **Scope:**
`EvaluatePort` (neu) + EVL-001 (Netto-Grundfläche je Geschoss) + EVL-003
(Wohnfläche). **Reine Aggregation** der bereits berechneten `Room.net_area_mm2`
— **kein** Shoelace-from-scratch (die Shoelace passierte in `room_detection`,
ADR-0007). **Nicht Teil:** EVL-002 (Volumen — geometrie-schwer, analytisch je
Bauteil-Typ → **slice-017c, MR-009 greift dort**); EVL-004/005/006 (Listen);
Material; DRW.

---

## 1. Ziel

Der erste sichtbare Auswertungs-Wert: die **Netto-Grundfläche** je Geschoss
(EVL-001) und die **Wohnfläche** (EVL-003), als **read-only-Ableitung** aus dem
committeten Modell über den neuen `EvaluatePort` (ADR-0012). Da
`Room.net_area_mm2` bereits die ADR-0007-Netto-Fläche trägt, ist die Auswertung
eine **reine Aggregation** (Summe der Raum-Flächen, mm² → m²) — analytisch im
Kern, pull-on-demand, kein `op`, keine Mutation, keine Persistenz.

## 2. Definition of Done

- [ ] **`EvaluatePort` (neu, `src/hexagon/ports/driving/`):** Driving-Port,
      read-only Query (Muster `DetectRoomsPort`). Methoden mindestens:
      `floorArea(StoreyId) → AreaReport` (EVL-001) und `livingArea() → AreaReport`
      (EVL-003, gebäudeweit). Reine Abfrage; **kein** `…Changed`-`op`, **kein**
      `GeometryKernelPort`-Aufruf (ADR-0012). Exakte Signatur = Kern-Hoheit.
- [ ] **Ergebnis-Werttyp (pure Werte, `src/hexagon/model/`):** `AreaReport`
      mit **Per-Raum-Flächen UND Summe** (MED-1, Lastenheft EVL-001 „je Raum
      (und die Summe)"): `{ double total_m2; std::vector<double> room_areas_m2; }`
      (`room_count = room_areas_m2.size()`) — framework-frei, read-only, kein
      OCC/Qt/SQLite.
- [ ] **Service implementiert `EvaluatePort`** (im `StructureEditService`, der
      die erkannten Räume bereits hält — `rooms_`; **`const`-Methoden** → kein
      Re-Detect in der Query, LOW-1): **EVL-001** `floorArea` = je Raum des
      Geschosses `net_area_mm2 / 1e6` (**Per-Raum-Flächen** + Summe `total_m2`);
      **EVL-003** `livingArea` = Summe aller Raum-Netto-Flächen × `kLivingAreaFactor`
      (welle-3 = 1). **Total:** Geschoss ohne Räume / leeres Modell →
      `total_m2 = 0`, leere `room_areas_m2` (kein Wurf).
- [ ] **`LIVING_AREA_FACTOR`-Konstante** (`src/hexagon/model/constants.h`):
      `kLivingAreaFactor = 1.0` (spez. §3; welle-3-Teilumfang, Anrechnungs-
      faktoren offen).
- [ ] **AK-Tests mit `LH-FA-EVL-001`/`LH-FA-EVL-003` im Namen** (Kern,
      display-frei, **analytische Orakel**): **EVL-001** (Building mit Wänden,
      die einen Raum schließen → `floorArea`: **Per-Raum-Fläche UND Summe** =
      bekannte Rechteck-Fläche in m², `EXPECT_NEAR`; `room_areas_m2.size()` = 1);
      **EVL-003** (Wohnfläche = Summe × Faktor); **Boundary** (Geschoss ohne
      geschlossene Räume → `total_m2` 0, leer); **Mehrere Räume** (Per-Raum +
      Summe korrekt); **Loch-Ring (LOW-2)** (Raum mit innenliegendem Wandzug →
      die Netto-Fläche `äußerer Ring − Loch` fließt durch die Aggregation —
      belegt die ADR-0007-Netto-Wiederverwendung; falls die Konstruktion in
      `room_detection` nicht sauber einen Loch-Ring erzeugt, wird die
      Netto-Wiederverwendung stattdessen begründet dokumentiert); **read-only**
      (`building()` nach der Abfrage unverändert **und** Ergebnis über zwei
      Abfragen identisch — kein versteckter Re-Detect, LOW-1). **Regression:**
      bestehende ROM-/Edit-/Viewer-Tests textlich unverändert grün (additiver Port).
- [ ] **`make arch-check` grün** (MED-2: `EvaluatePort`/Report pure Domäne, kein
      OCC/Qt/SQLite, kein `GeometryKernelPort`-Aufruf für die Fläche); `make
      gates` grün; Closure-Notiz mit Lerneintrag; CHANGELOG (MR-004). **MR-009
      n/a für 017b** (reine Aggregation, keine neue Geometrie — greift für
      EVL-002/017c).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{area_report}.h` | neu | `AreaReport` (pure Werte) |
| `src/hexagon/model/constants.h` | ändern | `kLivingAreaFactor` (spez. §3) |
| `src/hexagon/ports/driving/{evaluate_port}.h` | neu | `EvaluatePort` (read-only Query) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | `EvaluatePort` implementieren: `floorArea`/`livingArea` aus `rooms_` (mm²→m²) |
| `tests/hexagon/{test_evaluate}.cpp` | neu | EVL-001/003-AK (analytische Flächen-Orakel) |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/2026-06-14-slice-017b-plan.md` | neu | MR-006-Report |

## 4. Trigger

- slice-017a done ✓ (ADR-0012 accepted, EVL-Spec, `EvaluatePort` in
  `architecture.md`).
- MR-006-Plan-Review vor Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-017c**
  (EVL-002 Volumen — analytisch je Bauteil-Typ, **MR-009 greift**), danach
  Listen (EVL-004/005/006) + Material-Strang.

## 6. Risiken und offene Punkte

- **Schnitt bewusst klein (Flächen-Aggregation):** EVL-001/003 sind dank
  `Room.net_area_mm2` (ADR-0007) reine Summen — der Slice etabliert den
  **`EvaluatePort` + Ergebnis-Werttyp** (die architektonische Folgepflicht) an
  einem risikoarmen, analytisch prüfbaren Fall. Das geometrie-schwere Volumen
  (EVL-002) ist bewusst **017c** (MR-009-relevant: Shoelace-Footprint·Höhe,
  geklemmte Öffnungen, Miter-Eck-Näherung — die ADR-0012-#4-Mechanik).
- **Port-Träger:** `StructureEditService` implementiert `EvaluatePort` (er hält
  `rooms_` aus der Re-Detektion) — wie `DetectRoomsPort`. Ein eigener
  `EvaluationService` wäre denkbar, bräuchte aber Zugriff auf die erkannten
  Räume; der bestehende Service ist die kleinste Lösung (Review prüft).
- **mm² → m²:** `Room.net_area_mm2` ist in mm²; der Report ist in m² (Lastenheft
  „in m²") → ÷ 1 000 000. Im Test mit `EXPECT_NEAR` (Floating-Toleranz).
- **read-only / kein `op` (ADR-0012):** die Auswertung mutiert nichts und meldet
  nichts — ein AK belegt, dass `building()` nach der Abfrage unverändert ist.
- **Wohnfläche-Teilumfang:** `kLivingAreaFactor = 1` (Wohnfläche = Netto-
  Grundfläche); Anrechnungsfaktoren offen (spez. §1/§3, ehrlich benannt).
- **Räume sind Laufzeit-Daten:** `rooms_` wird bei Wand-Mutation re-detektiert
  (ADR-0007); die Auswertung liest den **zuletzt erkannten** Stand (Pull) — kein
  eigener Re-Detektions-Aufruf in der Query.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (neuer Driving-Port + pure Ergebnis-Werttypen,
  read-only, Aggregation, Totalität); Risiko niedrig (reine Summe vorhandener
  Werte, keine neue Geometrie).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (AK mit `LH-`-ID, analytische Flächen-Orakel,
  Registrierung); Risiko niedrig.

## 8. Closure-Notiz

**Ergebnis:** `make gates` grün (122 Tests, +6 EVL-AK). Der `EvaluatePort`
(ADR-0012-Folgepflicht) ist als read-only-Aggregation der Raum-Netto-Flächen
implementiert — `floorArea(StoreyId)` (Per-Raum + Summe je Geschoss, EVL-001),
`livingArea()` (gebäudeweit × `kLivingAreaFactor`, EVL-003) — getragen vom
`StructureEditService`, `const`/pull, kein `op`, kein `GeometryKernelPort`. Neuer
Werttyp `model::AreaReport` (Per-Raum-Flächen + Summe, MED-1). MR-009 n/a (reine
Aggregation, keine neue Geometrie).

**DoD:** alle Haken erfüllt — `EvaluatePort` + `AreaReport` + `kLivingAreaFactor`,
Service-Aggregation mm²→m², AK `LH-FA-EVL-001`/`LH-FA-EVL-003` (Happy/Mehrraum/
Loch-Ring/Boundary/Wohnfläche-gebäudeweit/read-only-stabil), `make arch-check` +
`make gates` grün, CHANGELOG (MR-004).

**Lerneintrag (Steering, Form 3 — Spec-Lücke benannt + Folge-Slice):** Der
Plan-Review (MED-1) deckte auf, dass die EVL-001-Anforderung „Netto-Grundfläche
**je Raum** (und die Summe)" im ersten Ergebnis-Werttyp nur als Summe modelliert
war. Eingearbeitet (Per-Raum-Vektor im `AreaReport`). **Verbleibende Spec-Lücke
(ehrlich benannt):** Die **Wohnflächen-Anrechnungsfaktoren** (DIN 277 / WoFlV,
raum-/nutzungsabhängig) sind bewusst auf `kLivingAreaFactor = 1` gesetzt
(welle-3-Teilumfang, spez. §1/§3) — **Folge-Bedarf**, wenn EVL-003 normgerecht
werden soll. **Folge-Slice-017c** (EVL-002 Volumen, **MR-009 greift dort**:
Shoelace-Footprint·Höhe − geklemmtes Öffnungsvolumen, Miter-Eck-Näherung,
ADR-0012 #4) ist im Closure-Trigger (§5) verankert, damit das geometrie-schwere
Volumen nicht ohne Code-Review durchrutscht.
