---
id: slice-047
titel: BLD-002/003 benutzer-erfüllbar machen — Projekt speichern/öffnen (CLI + GUI)
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md)]
---

# Slice 047: Projekt speichern/öffnen benutzer-erfüllbar (CLI + GUI)

**Status:** **done** (Closure 2026-07-26, s. §8) — Umbrella über **047a** (CLI) + **047b** (GUI).

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

**Unabhängige Verifikation 2026-07-25** (Verifier ≠ Implementer ≠ Reviewer, Skill-Datei `.harness/skills/verifier.md`):
11 DoD-Zeilen — **8 bestätigt, 0 widerlegt, 3 unbelegt**; **Verdikt: nicht fertig**
([Report](../../../reviews/2026-07-25-slice-047-verify.md)). Alle Findings eingearbeitet — s. §7.

**Prozess-Befund (Alt-Last, NICHT geheilt):** für slice-047 existierte **kein**
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review-Report,
obwohl der Plan-Kopf zwei Ergebnisse behauptete — ebenso für slice-045/046a/046b. Regelwerk Modul 10
verlangt „ein Report pro Lauf" unter `docs/reviews/`. Nachträgliche Reports wären eine Fälschung der
Audit-Spur und werden **nicht** geschrieben; die Lücke ist als Alt-Last benannt und der Gate-Nachzug
liegt in [`slice-051`](../open/slice-051-review-artefakt-pflicht.md).

**Welle:** welle-5-erweiterung. **Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

## Traceability-Korrektur ([LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden))

**Befund (aus dem [slice-046a](slice-046a-export-provenance.md)-Review):** BLD-002/003 wurden in welle-1 als
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
  [slice-046a](slice-046a-export-provenance.md)-Provenance-**„Quelle"** (Basename statt leer).

Beide über **eine** geteilte, testbare Save/Open-Naht (der vom Codebase-Autor vorgesehene `ManageProjectPort`-Use-Case).

## 0. Split ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start): empfohlen)

- **047a — CLI + geteilte Save/Open-Naht** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) 2026-07-24 — **Ergebnis-Behauptung ohne Report-Artefakt, s. Prozess-Befund**:
  **0 HIGH**, MED-1/2 [Save ist 3-arg + Ableitungs-Naht]
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

### 047b — GUI (MR-006 2026-07-24: 3 HIGH / 2 MED **in-Plan aufgelöst** — Ergebnis **unbelegbar**, s. Prozess-Befund)
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
      Laden neu auflösen** (aus `service.building()`) — **im Use-Case, nicht im Aufrufer** (s. Verify-B4).
      **Nicht** anlegen, was fehlt: Öffnen ist lesend (Code-Review MEDIUM-8 hat die frühere „ggf. eine Canvas-Ebene
      anlegen"-Variante zurückgenommen; der Benutzer wird stattdessen auf die fehlende Ebene hingewiesen).
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
- [x] **Benutzerhandbuch** ([`docs/user/benutzerhandbuch.md`](../../../user/benutzerhandbuch.md), Nachtrag 2026-07-26 —
      **im ursprünglichen Plan vergessen**, s. §7 V1): das Datei-Menü als eigener Abschnitt, die Übersichts-Tabelle +
      FAQ korrigiert (sie verneinten ein Datei-Menü) und der **Unterschied der beiden Öffnen-Wege** benannt
      (Menü = Sitzungs-Wechsel, `--open` = Export-Quelle).
- [x] **[LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)-AK-Schärfung** (Nachtrag 2026-07-26,
      Projektinhaber-Entscheidung — s. §7): Outline → AK-Niveau (Happy/Boundary/Negative), lösungsfrei/benutzer-
      beobachtbar ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)),
      Header-Version + Historie nachgezogen ([MR-010](../../../../harness/conventions.md)/[MR-012](../../../../harness/conventions.md)).
      Ohne sie hätte die Traceability-Korrektur für BLD-003 **kein Maß**.

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

## 7. Verify-Einarbeitung (2026-07-26)

Grundlage: [Verify-Report 2026-07-25](../../../reviews/2026-07-25-slice-047-verify.md) (Verdikt **nicht fertig**;
8 bestätigt / 0 widerlegt / 3 unbelegt). Zusätzlich **V1** — ein Befund, den weder Code-Review noch Verify finden
konnten, weil er außerhalb der DoD lag (Projektinhaber-Fund 2026-07-26).

| # | Klasse | Befund | Behandlung |
|---|---|---|---|
| **C1** | MEDIUM (blockierend) | `spec/spezifikation.md` normierte die Export-Quelle als „via `--open`/**GUI** geöffnetes Projekt" — einen GUI-Export gibt es nicht | **erledigt 2026-07-25**: der Satz nennt jetzt die CLI und sagt ausdrücklich, dass ein GUI-Export nicht existiert |
| **B4** | MEDIUM (blockierend) | die Neu-Auflösung der eingefrorenen Ids lag als Lambda im coverage-ausgenommenen `main` — **kein** Sensor führte sie aus (Gegenprobe CP-5: Aufruf entfernt → 280/280 grün) | **strukturell behoben**: der Schritt gehört jetzt zum Use-Case (`openProject` löst selbst auf und meldet über port-freie `DrawingTargetSinks`); `main` verdrahtet nur noch die Senken + übersetzt das Ergebnis in einen Hinweis. **Gegenprobe wiederholt: 4 Tests rot.** |
| **B2** | MEDIUM | die Zusage „**jeden** Id-Zähler" war nur für 5 von 9 diskriminierend (CP-1: Resets für `opening`/`roof`/`slab`/`stair` entfernt → 280/280 grün) | neuer Test `ReplaceBuildingResetsComponentIdCounters` (vier weitere Element-Arten mit paarweise verschiedenen geladenen Maxima). **Gegenprobe wiederholt: rot.** |
| **B5** | LOW (bewusst) | das Datei-Menü selbst hat keinen Sensor (modaler Dialog) | **bleibt** — im Plan so angesagt (MED-1). Der Anteil ohne Orakel ist durch B4 aber **kleiner** geworden: das Menü enthält nur noch Dialog + Meldungs-Text, die Entscheidungen liegen in getesteten Nähten. In der Closure-Notiz benannt. |
| **F1** | MEDIUM | `17da627` zerlegte die Roadmap still und legte zwei unverlinkte Dateien an, eine mit Tippfehler (`d-ckeck.md`) | Datei umbenannt; beide Dateien aus dem Roadmap-Kopf **verlinkt**; der Umbau ist hier benannt |
| **F2** | LOW | Spec: `ModelReplaced` „trägt keine Element-Kennung", der Code setzt eine informative Geschoss-Id | Spec präzisiert (informativ, **kein** Filter-Kriterium) — die Aussage deckt sich jetzt mit `model_changed_port.h` |
| **F3** | LOW | Plan-Kopf behauptet [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Ergebnisse ohne Report-Artefakt | beide Behauptungen **als unbelegbar markiert** (§0 + 047b-DoD-Überschrift); nachträgliche Reports wären eine Fälschung der Audit-Spur — Gate-Nachzug bleibt [`slice-051`](../open/slice-051-review-artefakt-pflicht.md) |
| **F4** | INFO | `spec/architecture.md` §1.2 nennt zwei Ports, die es nicht gibt, und lässt `ModelChangedPort` aus | Verzeichnisbaum ehrlich gemacht (auch die `services/`-Zeile, die denselben Defekt trug) |
| **V1** | **MEDIUM (neu)** | das [Benutzerhandbuch](../../../user/benutzerhandbuch.md) beschrieb Speichern/Öffnen als **reine Kommandozeilen-Aufgabe**; die FAQ verneinte ein Datei-Menü ausdrücklich. Ursache: die gemeinsame DoD-Zeile nannte `CHANGELOG` + `spec/`, aber **nicht** `docs/user/` — deshalb konnte auch der Verifier es nicht als DoD-Verletzung sehen | Handbuch-Abschnitt 4.3 neu, Übersicht/FAQ/Fehlerbehebung korrigiert, Handbuch-Version 1.1; DoD-Zeile ergänzt |
| **Anforderungs-Ebene** | MEDIUM | die Korrektur trägt belegt für [BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern); für [BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) war sie **unbelegbar** (Outline ohne AK) | **Projektinhaber-Entscheidung 2026-07-26: AK-Schärfung nachziehen** (statt die Grenze nur zu benennen) — Lastenheft 0.1.17. Der [`LH-FA-BLD-003.a`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)-§1-Block steht damit nicht mehr auf einem AK-losen Fundament |

**Lehre (Netz):** ein Schritt, der im Composition-Root steht, ist per Konstruktion orakel-los — `main.cpp` ist in kein
Testbinary gelinkt und coverage-ausgenommen. Die Frage ist deshalb nicht „wie testen wir `main`?", sondern „**gehört
dieser Schritt überhaupt dorthin?**". Bei B4 lautete die Antwort nein: die Neu-Auflösung ist Teil des Öffnen-
Use-Case, nicht seiner Verdrahtung. Was in `main` bleiben darf, ist Dialog, Meldung, Verdrahtung — keine Entscheidung.

## 8. Closure-Notiz

**Ausgeführt 2026-07-26.** Sensor-Läufe (selbst gefahren, nur `make`-Targets — [AGENTS §2.9](../../../../AGENTS.md)):

| Lauf | Exit | Kennzahlen |
|---|---|---|
| `make gates` | **0** | `docs-check`/d-check **247 Dateien, 0 Befunde** · `a-check` **0** · `arch-check` ok · `lint` 0 + „suppression-gate ok" · `test` **285/285** · `coverage-gate` **lines 91,4 % (4078/4460)**, Schwelle 70 % |
| `make io-smoke` | **0** | 6 Formate + IFC/DXF-Re-Import + Persistenz-Roundtrip („Provenance-Quelle = haus.bcad", „--open fehlend -> exit!=0") |
| Gegenprobe B2 | **rot** | vier Id-Zähler-Resets (`opening`/`roof`/`slab`/`stair`) entfernt → `ReplaceBuildingResetsComponentIdCounters` fällt (vorher: 280/280 grün) |
| Gegenprobe B4 | **rot** | Neu-Auflösung in `openProject` entfernt → **4** Tests fallen, darunter der Adapter-Test `CanvasAndSinkFollowLoadedIds` (vorher: 280/280 grün) |

Beide Mutationen zurückgenommen (`git diff` sauber vor dem Commit).

**Geliefert (über 047a/047b hinaus — die Verify-Einarbeitung):**

- **Orakel statt Zusage:** die Neu-Auflösung des Zeichen-Ziels ist vom Composition-Root in den Use-Case
  gewandert (`DrawingTargetSinks`, port-frei); die vier fehlenden Id-Zähler-Resets sind gedeckt. Damit trägt
  **jede** abgehakte DoD-Zeile ein Orakel — **außer B5** (s. u.).
- **Lastenheft 0.1.17:** [`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) Outline → AK.
- **Benutzerhandbuch 1.1:** das Datei-Menü existiert jetzt auch in der Doku.
- **Vier Doku-Wahrheits-Korrekturen:** Spec-Export-Quelle (C1), `ModelReplaced`-Geschoss-Id (F2), Plan-Kopf-
  Behauptungen (F3), `architecture.md`-Verzeichnisbaum (F4).

**Benannte Grenze (bleibt offen, bewusst):** das **Datei-Menü selbst** hat keinen Sensor (B5) — der modale
`QFileDialog` ist im Plan ausdrücklich als untestbar deklariert (MED-1). Sein Verlust würde von `make gates`
nicht gemeldet. Der ungedeckte Anteil ist durch die B4-Verschiebung aber **kleiner** geworden: im Menü stehen
nur noch Dialog, Meldungstext und Verdrahtung.

**Validation (Rolle Projektinhaber, [`validator.md`](../../../../.harness/skills/validator.md), 2026-07-26):**

- `entscheidung`: **angenommen mit benanntem Rest**
- `begruendung`: die Handlung, die vorher unmöglich war, ist möglich — App starten, **Datei → Öffnen…**, eine
  `.bcad` wählen, und 3D-Ansicht wie Grundriss zeigen das geladene Projekt; **Datei → Speichern unter…** schreibt
  es atomar zurück. Mechanik ≠ Anforderung war der Ausgangsbefund; der Aufruf-Pfad schließt genau diese Lücke.
- `rest`: kein „Speichern" auf die zuletzt geöffnete Datei (nur „Speichern unter…"), **keine** Warnung vor
  ungesicherten Änderungen beim Öffnen, **kein** GUI-Export, **keine** Zuletzt-geöffnet-Liste. Der Rest blockiert
  den Nutzen nicht, ist aber im Handbuch (4.3) **benannt**, damit ihn kein Benutzer als Fehler erlebt.
- `folge`: [`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md) (+ [`slice-052b`](../done/slice-052b-neues-projekt.md)) in `open/` — Sitzungs-Datei
  merken (Speichern ohne Pfad-Dialog) + Ungesichert-Warnung. GUI-Export und Zuletzt-geöffnet-Liste bleiben
  eigene Schnitte.

**Traceability-Korrektur:** [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)
waren in welle-1 verfrüht `done` (Mechanik ohne Aufruf-Pfad) — vermerkt in
[`welle-1-results.md`](welle-1-results.md). Beide sind **jetzt** benutzer-erfüllbar, für BLD-003 seit
0.1.17 auch **messbar**. [`ACC-005`](../../../../spec/lastenheft.md#7-abnahmekriterien) ist damit End-to-End
demonstrierbar (alles außer dem modalen Dialog orakel-gedeckt).

**Lerneintrag:** Die teuerste Lücke dieses Slice war **keine** Code-Lücke. Drei unabhängige Prüf-Läufe
(Plan-Review, Code-Review, Verify) haben das falsch beschriebene Benutzerhandbuch **nicht** gefunden — der
Verifier prüft gegen die DoD, und die DoD nannte `docs/user/` nicht. Eine Prüfung kann nur finden, was ihr
Maßstab enthält; ein Slice, der eine Anforderung **benutzer-erfüllbar** macht, muss `docs/user/` in seiner
Doku-Zeile führen. Zweite Lehre: `main.cpp` ist orakel-los per Konstruktion — Entscheidungen gehören dort nicht
hin (s. §7).

**Folge:** slice-047 ist geschlossen; der laufende Faden ist die
[DRW-Aids-Kampagne](slice-049-spec-straten-prozess-rein-welle.md)-Sequenz bzw. `slice-048b`
(DRW-001-Impl). Der Prozess-Nachzug „Review-Artefakt-Pflicht" bleibt
[`slice-051`](../open/slice-051-review-artefakt-pflicht.md).
