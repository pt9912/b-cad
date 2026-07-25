---
id: slice-047
titel: BLD-002/003 benutzer-erfüllbar machen — Projekt speichern/öffnen (CLI + GUI)
status: in-progress
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md)]
---

# Slice 047: Projekt speichern/öffnen benutzer-erfüllbar (CLI + GUI)

**Status:** in-progress (Umbrella — **047a DONE**, **047b implementiert**, Closure offen).

**047a (CLI)** implementiert + committet 2026-07-24 (`--save`/`--open` + promoteter
`services::saveProject`-Use-Case [fail-closed] + echte Provenance-Quelle; io-smoke-Roundtrip grün).

**047b (GUI)** implementiert + committet 2026-07-25 (`15448d2`): Datei-Menü Öffnen/Speichern,
`services::openProject`, `StructureEditService::replaceBuilding` (abgeleitete Zustände + Id-Zähler-Reset),
`ModelChangeOp::ModelReplaced` + Viewer-Voll-Neuaufbau, Neu-Auflösung der eingefrorenen Geschoss-/Ebenen-Ids.

**Unabhängiges Code-Review 2026-07-25** (Reviewer ≠ Autor, Skill-Datei `.harness/skills/reviewer.md`):
**1 HIGH / 10 MEDIUM / 6 LOW / 4 INFO**
([Report](../../../reviews/2026-07-25-slice-047b-code-review.md)) — **alle Findings eingearbeitet**
(`2db4fc6` + Folge-Commit). Der HIGH betraf eine **falsche Spec-Aussage** („beide Wege nutzen denselben
Kern-Use-Case" — für *Öffnen* unwahr, die CLI geht nicht durch `openProject`); MEDIUM-1/2/3 waren
**empirisch belegte Test-Lücken** (Produktionscode zurückgenommen → Tests blieben grün).

**Prozess-Befund (Alt-Last, NICHT geheilt):** für slice-047 existierte **kein**
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review-Report,
obwohl der Plan-Kopf zwei Ergebnisse behauptete — ebenso für slice-045/046a/046b. Regelwerk Modul 10
verlangt „ein Report pro Lauf" unter `docs/reviews/`. Nachträgliche Reports wären eine Fälschung der
Audit-Spur und werden **nicht** geschrieben; die Lücke ist als Alt-Last benannt und der Gate-Nachzug
liegt in [`slice-051`](../open/slice-051-review-artefakt-pflicht.md).

**Welle:** welle-5-erweiterung. **Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

## Traceability-Korrektur ([LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden))

**Befund (aus dem [slice-046a](../done/slice-046a-export-provenance.md)-Review):** BLD-002/003 wurden in welle-1 als
`done` verbucht, sind aber **nicht benutzer-erfüllbar**: die Persistenz-**Mechanik** (atomarer `save`/`load`,
Crash-Recovery, `E-IO`-Fehler) ist implementiert + getestet (slices **008a/008b** + Bauteil-Persistenz), **aber es
gibt keinen Aufruf-Pfad** — weder GUI „Datei → Speichern/Öffnen" noch CLI. Die AK-Klausel **„when Speichern"** ist
damit **nicht demonstrierbar**, und es existiert **keine andere** Anforderung, die die Speichern/Öffnen-Aktion trägt
(BLD-002/003 **sind** sie). → Mechanik ≠ ganze Anforderung; die `done`-Markierung war **verfrüht**. **slice-047 setzt
den fehlenden benutzer-erfüllbaren Teil um** (kein Doppelzählen — das ist die Lücke). Die `done`-Status-Korrektur
wird bei der 047-Closure nachgezogen (welle-Ergebnis-/Traceability-Notiz).

## 1. Ziel

Die vorhandene Persistenz **benutzer-aufrufbar** machen — **beides** (Projektinhaber-Wahl 2026-07-24):
- **GUI (047b):** „Datei → Speichern / Öffnen" (Menü + `QFileDialog`) — die **natürliche** Desktop-CAD-Aktion, die die
  AK „when Speichern" erfüllt.
- **CLI (047a):** `--save <pfad>` / `--open <pfad>` — skriptbar, **io-smoke-testbar**, und speist die
  [slice-046a](../done/slice-046a-export-provenance.md)-Provenance-**„Quelle"** (Basename statt leer).

Beide über **eine** geteilte, testbare Save/Open-Naht (der vom Codebase-Autor vorgesehene `ManageProjectPort`-Use-Case).

## 0. Split ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start): empfohlen)

- **047a — CLI + geteilte Save/Open-Naht** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) 2026-07-24: **0 HIGH**, MED-1/2 [Save ist 3-arg + Ableitungs-Naht]
  + LOW-1 eingearbeitet). **DONE + committet 2026-07-24** — `services::saveProject` promotet (fail-closed, unit-getestet),
  `--save`/`--open` in `main.cpp`, echte Provenance-Quelle, io-smoke-Roundtrip.
- **047b — GUI** („Datei → Speichern/Öffnen" + `StructureEditService`-Modell-Ersetzung + headless GUI-Test).
  **Neuer Scope, eigenes [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor Start** (GUI-Naht + Modell-Replace waren im 047a-Review nicht drin).

„Beides" bleibt erfüllt — zwei sequenzielle Nähte, geteilte Save/Open-Logik.

## 2. Geteilte Naht (Kern, von CLI + GUI genutzt)

- **Save-Use-Case** (`src/hexagon/services/…`, der vorgesehene `ManageProjectPort`): baut `PersistedDerivations` aus
  dem `Building` (`resolveStoreyHeight` + `services::stairRiseMm`, **fail-closed** neutraler `E-IO`-Wurf bei
  danglendem `from_storey`) + ruft `repository.save(building, derived, pfad)` (**3-arg**, atomar). Muster: der
  Test-Helfer `save_project_test_helper.h` (`saveProject`) wird **dorthin promotet** (nicht in coverage-ausgenommenes
  main/GUI-Code inlined). **Unit-getestet** (Ableitung + fail-closed).
- **Open (047b):** `repository.load(pfad) → Building` (existiert, wirft neutral bei fehlend/defekt) → die
  `openProject`-Naht (MED-2) ruft `StructureEditService::replaceBuilding` (HIGH-2: abgeleitete Zustände neu bauen +
  `next_*_id_`-Reset) → Full-Refresh-Notify (HIGH-1: neuer `ModelReplaced`-Op + `ViewerScene`-`loadAll()`).

## 3. Definition of Done

### 047a — CLI + Naht
- [x] **Save-Use-Case** (§2) als testbare Produktions-Naht + Unit-Test (Ableitung + fail-closed `E-IO`).
- [x] **`main.cpp` `--save <pfad>`** ruft den Use-Case; **`--open <pfad>`** ruft `load` → das `Building` ist die
      Export-Quelle; `provenance.source` = **Basename** (kein Struktur-Leak, 046a-R3); ohne `--open` bleibt `source`
      leer (`footerLine()` lässt es weg — LOW-1). Fehler (`load`/`save` werfen neutral) **gefangen** → stderr +
      Exit ≠ 0, kein Crash/Teil-Zustand.
- [x] **Export-Refactor:** der Export-Pfad exportiert einen **übergebenen** `Building` + `ExportProvenance` (main
      wählt geladen-vs-Demo); das bestehende `--export-*`-Demo-Verhalten (ohne `--open`) bleicht unverändert.
- [x] **AK — io-smoke-Roundtrip:** Demo `--save $tmp.bcad` → `--open $tmp.bcad --export-pdf` → PDF trägt die echte
      Quelle (Basename grep-bar, PDF unkomprimiert); `--open` nicht existent → Exit ≠ 0, kein Crash. Load-Korrektheit
      deckt `test_sqlite_project_repository`.

### 047b — GUI (MR-006 2026-07-24: 3 HIGH / 2 MED **in-Plan aufgelöst**, s. u.)
- [x] **`openProject`-Kern-Naht** (MED-2, symmetrisch zu `saveProject`): `services::openProject(service, repository, path)`
      (o. `StructureEditService::replaceBuilding(Building)`, vom Handler gerufen) — **testbar außerhalb** des
      coverage-ausgenommenen main/GUI.
- [x] **`StructureEditService::replaceBuilding(Building)`** (HIGH-2): setzt `building_` **und** baut die abgeleiteten
      Zustände neu — `solids_` für die geladenen Wände, `redetectRooms` je geladenem Geschoss, **jeden `next_*_id_`-
      Zähler über das geladene Maximum** zurücksetzen (sonst kollidiert die erste Mutation nach dem Laden mit
      persistierten Ids). Danach der **Full-Refresh-Notify** (s. u.). **Unit-Test:** nach dem Laden eines
      geroomten Projekts ist `floorArea` nicht leer; die erste `addGuideLine`/`addWall` mintet eine **frische** (nicht
      kollidierende) Id.
- [x] **`ModelReplaced`-Op + Viewer-Refresh** (HIGH-1): ein neuer `ModelChangeOp` (`model_changed_port.h`); der
      **`ViewerScene`** bekommt einen Fall, der `scene_.loadAll()` ruft (der per-op-Viewer refresht sonst NICHT — er
      pullt nur die geänderte Wand; alte Wände blieben stale). Der **`CanvasWidget`** refresht bereits generisch
      (pullt `planView()` bei jedem Notify) — nur der Viewer braucht den neuen Fall.
- [x] **Eingefrorene Ids neu auflösen** (HIGH-3): `main.cpp`/`CanvasWidget`/`EditDrawingGuideLineSink` frieren beim
      Demo-Bau das aktive Geschoss + die Hilfslinien-Ebene ein (by-value). Nach dem Laden sind diese Ids i. d. R.
      ungültig → Canvas malt leer + Hilfslinien-Zeichnen wird abgelehnt. Das aktive Geschoss/die Ebene **nach dem
      Laden neu auflösen** (aus `service.building()`; ggf. eine Canvas-Ebene anlegen) — oder lazy statt captured.
- [x] **`QMainWindow`-Menü „Datei"** ([ADR-0009](../../adr/0009-gui-framework-qt6.md)) mit **Speichern**/**Öffnen** →
      `QFileDialog` (Pfadwahl) → der path-nehmende **Handler** (`openProject`/`saveProject`). Fehler → benutzer-sichtbarer
      `QMessageBox`/Status, kein Crash. Der modale Dialog lebt im coverage-ausgenommenen main (nicht getestet, MED-1).
- [x] **Headless-Test über den Handler** (MED-1, Muster `test_viewer_widget`, Xvfb — **nicht** den modalen Dialog):
      ein gespeichertes Projekt „öffnen" (Handler direkt) → `service.building()` entspricht dem Stand + ein
      abonnierter headless `CanvasWidget`/`ViewerScene` spiegelt ihn; Speichern → Repository-Roundtrip.

### gemeinsam
- [x] **Doku:** [CHANGELOG](../../../../CHANGELOG.md); Spec-Notiz `spec/spezifikation.md` (Speichern/Öffnen benutzer-
      aufrufbar via GUI+CLI, Provenance-Quelle real; lösungsfrei [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).
      **Traceability-Korrektur** der BLD-002/003-`done`-Markierung (welle-Ergebnis-/Status-Notiz). **Kein** ADR-Index-Eintrag.

## 4. Plan (vor Code)

| Datei / Komponente | Naht | Art | Begründung |
|---|---|---|---|
| `src/hexagon/services/…` (Save-Use-Case) | 047a | neu | `saveProject` promotet: `PersistedDerivations` fail-closed + `save` |
| `src/main.cpp` | 047a | ändern | `--save`/`--open`, Use-Case + Repository verdrahten, Fehlerfang, Export-`Building`+Provenance |
| `tools/io-smoke.sh` | 047a | ändern | Save→Open→Export-Roundtrip + Fehlerfall |
| `src/hexagon/services/manage_project.{h,cpp}` | 047b | ändern | `openProject`-Naht (MED-2, symmetrisch zu `saveProject`) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | 047b | ändern | `replaceBuilding` (HIGH-2: `solids_`/`redetectRooms`/`next_*_id_`-Reset) + Notify |
| `src/hexagon/ports/driven/model_changed_port.h` | 047b | ändern | neuer `ModelReplaced`-Op (HIGH-1) |
| `src/adapters/ui/view/viewer_scene.{h,cpp}` | 047b | ändern | `ModelReplaced`-Fall → `scene_.loadAll()` (HIGH-1) |
| `src/adapters/ui/view/canvas_widget.*`, `src/adapters/ui/command/edit_drawing_guide_line_sink.*` | 047b | ändern | eingefrorene Geschoss/-Ebenen-Ids neu auflösen (HIGH-3) |
| `src/main.cpp` | 047b | ändern | „Datei"-Menü (`QMenuBar`) + `QFileDialog`/`QMessageBox` → Handler; Ids nach Laden neu setzen |
| `tests/…` | beide | neu | Save-Use-Case-Unit-Test (047a); headless GUI-Test (047b) |
| `spec/spezifikation.md`, `CHANGELOG.md` | gemeinsam | ändern | Doku + Traceability-Korrektur |

**Bewusst NICHT Teil:** neues Schema/Fehlercode; Projektversionierung ([LH-FA-BLD-004](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung)); IFC/DXF-Provenance-Nachzug.

## 5. Risiken

- **R1 — `main.cpp`/GUI coverage-ausgenommen:** korrektheits-kritische Logik (Save-Ableitung, geladen-vs-Demo) in die
  **testbare Kern-Naht** ziehen; CLI-Verhalten via io-smoke, GUI via headless Widget-Test.
- **R2 — Modell-Ersetzung (047b):** das ganze `Building` zu ersetzen ist ein **neuer** Mutations-Use-Case im
  `StructureEditService` (bisher nur inkrementell) → Full-Refresh-Notify sauber, sonst zeigen Viewer/Canvas Stale-Stand.
- **R3 — Save-3-arg/fail-closed:** `PersistedDerivations` korrekt bauen; danglendes `from_storey` → neutraler
  `E-IO`-Wurf, kein Teil-Save (Muster 042d).
- **R4 — Export-Refactor:** geladen-vs-Demo ohne Bruch des bestehenden `--export-*`-Verhaltens.
- **R5 — GUI-Fehler-UX:** Load-/Save-Fehler benutzer-sichtbar (Dialog/Status), kein Crash.

## 6. Trigger

- Projektinhaber-Fund (2026-07-24): BLD-002/003 nicht benutzer-erfüllbar (kein Aufruf-Pfad), keine andere Anforderung
  deckt es → verfrühte `done`-Markierung; **GUI + CLI** beide gewünscht.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review** je Naht (047a durch; 047b eigenes vor Start), HIGH blockiert.
  GUI/persistenz-nah → **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)** vor Welle-Closure erwägen.
