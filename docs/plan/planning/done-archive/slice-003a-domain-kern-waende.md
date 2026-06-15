---
id: slice-003a
titel: Domain-Kern & Wände (parametrisch, OCC-frei)
status: done
welle: welle-1-mvp
lastenheft_refs: [LH-FA-BLD-001, LH-FA-FLR-001, LH-FA-WAL-001, LH-FA-WAL-002, LH-FA-WAL-003]
adr_refs: [ADR-0001, ADR-0002]
---

# Slice 003a: Domain-Kern & Wände (parametrisch, OCC-frei)

**Status:** done

**Welle:** welle-1-mvp

**Bezug:** LH-FA-BLD-001, LH-FA-FLR-001, LH-FA-WAL-001, LH-FA-WAL-002, LH-FA-WAL-003, ADR-0001, ADR-0002

**Autor:** Dietmar Burkard. **Datum:** 2026-06-09.

**Schnitt-Herkunft:** Aufteilung des ursprünglichen `slice-003` in den
OCC-freien Kern (**003a**, dieser Slice) und den OCC-Extrusions-Adapter
(**003b**) — Begründung im Roadmap-§Historische Trigger-Verschiebungen
(Modul 5: Schnitt nach Lieferwert, der Kern ist einzeln review-bar und
deterministisch ohne Build-schwere OCC-Abhängigkeit).

---

## 1. Ziel

Den framework-freien Domain-Kern und den ersten echten Use-Case liefern:
Gebäude mit Geschoss anlegen, Wände als Segment zeichnen, Wandstärke
(50–1000 mm) und -höhe (500–10000 mm) parametrisch setzen **mit Klemmung
auf den Grenzwert** (`E-VAL-001`). Der `GeometryKernelPort`-Vertrag wird
hier **definiert und vom Service genutzt** (Rebuild nach Parameter-
Änderung), aber **nur über ein Test-Double erfüllt** — der echte
OCC-Adapter folgt in 003b. Damit ist 003a OCC-frei und deterministisch.

## 2. Definition of Done

- [x] `src/hexagon/model/` (`Point2D`, `Segment`, `Solid`, `WallType`,
      `Building`, `Storey`, `Wall`) + `src/hexagon/model/constants.h`
      (Wertebereiche aus [`spec/spezifikation.md` §3](../../../../spec/spezifikation.md#3-defaults-und-konstanten)
      referenziert) — pure Werttypen, kein Qt/OCC/SQLite.
- [x] `EditStructurePort` (driving) + `GeometryKernelPort` (driven,
      Vertrag — keine Implementierung) + `StructureEditService`:
      Geschoss/Wand erzeugen, Stärke/Höhe setzen mit Klemmung
      (`E-VAL-001`, LH-FA-WAL-002/003); Rebuild ruft `extrudeWall` am
      Port und hält das resultierende `Solid`.
- [x] Akzeptanz-Tests Happy/Boundary/Negative für **LH-FA-WAL-002**
      (240 mm Happy + sofortiger Rebuild; 50/1000 mm akzeptiert, 49/1001 mm
      geklemmt + `E-VAL-001`) und **LH-FA-WAL-003** (500/10000 mm;
      499/10001 mm geklemmt), je mit `GeometryKernelPort`-Double (ohne
      OCC). Test-Namen tragen die `LH-`-ID.
- [x] Domänen-Teil von BLD-001/FLR-001/WAL-001: leeres Gebäude mit genau
      einem Default-Geschoss; Wand aus zwei distinkten Punkten mit
      Defaults; Null-Längen-Wand verworfen (WAL-001 Boundary).
- [x] `make gates` grün (lint, test 19/19, coverage 95,8 %); Closure-Notiz.

## 3. Plan (vor Code)

### Port-Vertrag (definiert hier, implementiert in 003b)

```cpp
// hexagon/ports/driven/geometry_kernel_port.h  (framework-frei)
namespace bcad::hexagon::ports::driven {
class GeometryKernelPort {
public:
    virtual ~GeometryKernelPort() = default;
    // Extrudiert das Wand-Footprint (Segment × Stärke) auf Wandhöhe zu
    // einem Solid. Neutraler Rückgabewert model::Solid — KEIN OCC-Typ
    // verlässt den Adapter (ADR-0001/0002).
    virtual model::Solid extrudeWall(const model::Wall& wall) const = 0;
};
}
```

`model::Solid` ist ein neutraler Domänenwert: `struct Solid { double
volume_mm3{}; };`. Für ein gerades Segment gilt analytisch
`volume = länge · stärke · höhe`; das macht den 003b-Adapter über das
gemessene OCC-Volumen gegen `GEOMETRY_TOLERANCE_MM` testbar und das
003a-Double trivial deterministisch.

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{point2d,solid,wall_type,building,storey,wall}.h` | neu | Domain-Modell (OBJ-002/003) |
| `src/hexagon/model/constants.h` | neu | Wertebereiche (spezifikation §3) |
| `src/hexagon/ports/driving/edit_structure_port.h` | neu | Use-Case-Schnittstelle |
| `src/hexagon/ports/driven/geometry_kernel_port.h` | neu | Geometrie-Abstraktion (ADR-0001/0002) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | neu | Validierung/Klemmung + Rebuild über Port |
| `src/hexagon/CMakeLists.txt` | ändern | neue `.cpp` in `bcad_hexagon` |
| `tests/hexagon/test_structure_edit_service.cpp` | neu | AK WAL-002/003 mit Port-Double |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |

Der alte Skelett-Greeting-Pfad (`greet_port`, `greeting_source_port`,
`greeting_service`) bleibt unberührt; er wird in einem späteren Aufräum-
Slice entfernt, sobald der echte Kern trägt.

## 4. Trigger

- slice-002 done (Code-Gates stehen, arch-check grün). ✓

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz geschrieben.

## 6. Risiken und offene Punkte

- WAL-001 Negative (`E-GEO-001`, Eingabe außerhalb Zeichenbereich) setzt
  einen Koordinaten-/Zeichenbereich voraus (UI/Coord-Space) — **nicht**
  in 003a; Domänen-Kern kennt keinen Zeichenbereich.
- BLD-001 Boundary/Negative (Verwerfen-Rückfrage, `E-IO-001`) sind
  UI- bzw. Persistenz-Belang — späterer Slice (welle-1 „speichern/laden").
- Wand-Verbindung/Verschneidung (LH-FA-WAL-006) erst Folge-Slice.

## 7. Closure-Notiz

**Closure-Kriterien (beobachtbar):**
- `make gates` grün auf frischem COPY-Build: lint (clang-tidy 0 Befunde +
  suppression-gate), test 19/19, coverage 95,8 % (Schwelle 70 %).
- Alle AK-Tests tragen die `LH-`-ID im Namen (WAL-002/003 Happy/Boundary/
  Nicht-endlich, BLD-001, FLR-001, WAL-001 inkl. Null-Längen-Boundary).
- `GeometryKernelPort`-Vertrag steht und wird vom Service genutzt
  (transaktionaler Rebuild über Port-Double) — Seam für den OCC-Adapter
  (003b) ist geschlossen.

**Lerneintrag:**
- **Spec-Lücke geschlossen:** Das Lastenheft fordert eine Default-Wandstärke
  (WAL-001), `spec/spezifikation.md` §3 nannte keinen Wert. Ergänzt:
  `DEFAULT_WALL_THICKNESS_MM = 240`; Default-Wandhöhe = Geschosshöhe
  (parametrisch dokumentiert).
- **Geschärfte Regel (Kandidat für Konvention):** Bauteil-Ids sind
  **starke Typen** (`enum class WallId/StoreyId : int`), nicht `int` —
  das Lint-Gate (`bugprone-easily-swappable-parameters`) hat `int`-Ids
  neben `double`-Messwerten als vertauschbar erkannt. Empfehlung: für
  künftige Bauteil-Ids (Door/Window/Room …) dieselbe Konvention; ggf. als
  AGENTS.md-Regel oder `MR-<NNN>` festschreiben, sobald sie sich über
  mehrere Slices bewährt (Steering Loop: erst beim 3. Vorkommen
  verallgemeinern).
- **Geschärfte Regel — transaktionaler Rebuild (E-GEO-002):** Modell-
  Mutationen erst *nach* erfolgreicher `extrudeWall`-Berechnung committen
  (Solid zuerst, dann Parameter/Wand). Schlägt die Geometrie fehl, bleibt
  das Modell unverändert (`spec/spezifikation.md` §4 E-GEO-002). Mit
  werfendem Geometrie-Double getestet. *Review-Finding (Modul 11).*
- **Geschärfte Regel — Finite-Validierung:** Nicht-endliche Parameter
  (NaN/Inf) sind ungültig, nicht klemmbar → `ParamStatus::Rejected`,
  Modell unverändert (statt NaN-Wert/-Solid zu speichern). Dafür trägt
  `ParamResult` jetzt einen Status (`Accepted`/`Clamped`/`Rejected`) statt
  eines `clamped`-Bools. *Review-Finding (Modul 10).*
- **Kein neuer Sensor in 003a** — die OCC-Isolations-Regel C (`arch-check`)
  ist bewusst in **003b** verortet (wo der erste OCC-Code entsteht;
  ADR-0002-Folgepflicht).

**Restrisiko / Nachfolge:** WAL-001 Negative (`E-GEO-001`, Zeichenbereich)
und BLD-001 IO/UI-Pfade bleiben offen (eigene Slices); Einzelwand ohne
Verschneidung (WAL-006 Folge-Slice).

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Pure-Domain-/Port-Konvention in ADR-0001, `spec/architecture.md` §2, `harness/conventions.md`.
- **Phase-Reife:** Phase 4 (Lastenheft + ADR führen; Code wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Test-Infrastruktur

- **Modus:** GF (slice-002 hat die GoogleTest-/Determinismus-Konvention verankert).
- **Konventionen-Dichte:** mittel.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** mittel — AK-Tests müssen `LH-`-ID im Namen tragen.
- **Reconciliation-Aufwand:** keiner.
