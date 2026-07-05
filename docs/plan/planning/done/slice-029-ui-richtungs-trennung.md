---
id: slice-029
titel: ui-Adapter driving/driven verzeichnislich trennen — ui/view (driven) + ui/command (driving) mit MeshSource-Naht (a-check-Vorbereitung, Pilot-Befund 2)
status: done
welle: harness-steering
lastenheft_refs: [[LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md)]
---

# Slice 029: ui-Adapter driving/driven trennen — `ui/view/` + `ui/command/` (Pilot-Befund 2)

**Status:** done (2026-07-03) — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
**0 HIGH / 3 MED / 4 LOW / 2 INFO — alle vor Start eingearbeitet**
(MED-1: Aggregat-Rückgabetypen der `MeshSource` **port-frei fixiert**
[`std::map<Id,TriangleMesh>` statt Driving-Port-Structs — tragend fürs
Abnahme-Kriterium] + Konversion als DoD-Punkt; MED-2: §2.8-Zwei-Commit-
Split deklariert; MED-3: fünf done-archive-codepaths-Referenzen →
`d-check:ignore`-Marker; LOW-1..4 + Begründungs-Schärfung der
Presenter-Ablehnung). Unterordner-Variante + MeshSource-Naht vom Review
**bestätigt**; Namenswahl `command/` vs. `driving/` als expliziter
Abnahme-Punkt. [Report](../../../reviews/2026-07-03-slice-029-plan.md).
**Abgenommen** (Projektinhaber, 2026-07-03: „Go"; Namenswahl `command/`
bestätigt — Plan-Empfehlung ohne Widerspruch) und umgesetzt —
**Status: done** (Commits `dd01fc7` Move + `c0a12ae` Naht);
unabhängiges **Code-Review 0 HIGH / 0 MED / 1 LOW**
([Report](../../../reviews/2026-07-03-slice-029-code-review.md));
`make gates` grün (228/228); Closure-Notiz §8.

**Welle:** harness-steering (Quergewerk „a-check-Vorbereitung", Schwester
von slice-028; Gate-Umstellung selbst **nicht Teil**, keine `.a-check.yml`).

**Bezug / Anlass:** a-check-Pilot-Lauf (v0.8.0, Projektinhaber
2026-07-03) — **Befund 2:** `src/adapters/ui/` ist ein
**Misch-Adapter**: `viewer_scene.h` und `viewer_widget.h` inkludieren
**sowohl** `hexagon/ports/driving/view_model_port.h` (ui ruft die App)
**als auch** `hexagon/ports/driven/model_changed_port.h` (ui
implementiert den Beobachter) — am Code verifiziert (2026-07-03). Für
die künftige driving/driven-Richtungsprüfung braucht jedes
Adapter-Verzeichnis **genau eine** Richtung.
**Ursachen-Analyse:** die Mischung ist kein Zufall, sondern das
[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)-Muster
**Push-Notify/Pull-State**: der Beobachter-Callback (driven) **pullt**
im selben Objekt über den `ViewModelPort` (driving) — die Trennung muss
also eine **Naht durch dieses Muster** legen, nicht nur Dateien sortieren.
[ADR-0009](../../adr/0009-gui-framework-qt6.md) (Regel E: Qt nur unter
`src/adapters/ui/` + `src/main.cpp` — bleibt **unverändert**),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Regel B: kein
Adapter→Adapter — ui-interne Unterordner sind **ein** Adapter, kein
B-Fall). `io`/`geometry`/`persistence`/`plugin` sind bereits sauber
einseitig — **dort nichts ändern** (Projektinhaber-Vorgabe).

**Maßstab (Projektinhaber-Vorgabe):** `tools/arch-check.sh` (A–E, P1/P2)
durchgehend grün; alle Gates grün. **Abnahme-Kriterium:** kein
Verzeichnis unter `src/adapters/` mischt driving- und
driven-Port-Includes.

**Autor:** Dietmar Burkard. **Datum:** 2026-07-03.

---

## 1. Ziel und Entscheidung

**Entscheidung (Empfehlung dieses Plans): die vom Piloten empfohlene
Variante — Unterordner INNERHALB von `src/adapters/ui/`:**

- **`src/adapters/ui/{view}/`** = **driven**: die
  `ModelChangedPort`-**Implementierungen** (`ViewerScene`,
  `ViewerWidget`) + Rendering. Der Include folgt der implementierenden
  Klasse (Pilot-Empfehlung). Einziges Port-Include-Vorkommen:
  `hexagon/ports/driven/model_changed_port.h`.
- **`src/adapters/ui/{command}/`** = **driving**: der Sitz aller
  App-Aufrufe aus der ui. Heute lebt dort genau ein Baustein (s. Naht);
  künftige echte GUI-Kommandos (Edit-Werkzeuge → `EditStructurePort`)
  haben damit ihren deklarierten Ort. Einzige Port-Include-Richtung:
  `hexagon/ports/driving/`.
- **`src/adapters/ui/qt_probe.cpp`** bleibt an der ui-Wurzel
  (port-frei/richtungs-neutral — mischt nichts).

**Verworfene Alternative (benannt, Pilot-Hinweis; Begründung nach
Review geschärft):** ein eigener Presenter-Adapter **außerhalb** `ui/`
**beseitigt die Richtungs-Mischung nicht — er verlagert sie nur**:
solange der Presenter selbst Beobachter (driven) ist **und** den
`ViewModelPort` (driving) zieht, mischt sein Verzeichnis genauso; er
bräuchte **dieselbe** MeshSource-Naht. Zusätzlich wäre er entweder ein
neuer Adapter an der Regel-B-Grenze oder verlangte eine
Regel-E-/[ADR-0009](../../adr/0009-gui-framework-qt6.md)-Anpassung
(Gate-/ADR-Prozess ohne Not). Die Unterordner-Variante hält die Naht
adapter-intern und lässt Regel E **unangetastet grün** — deshalb
empfohlen.

**Die Naht durch das Push-Notify/Pull-State-Muster (Kern des Entwurfs):**

1. **`ui/view/mesh_source.h`** (neu, ui-intern, **port-frei**): schmale
   pure-virtual Schnittstelle `MeshSource` — Methodenmenge = exakt der
   heutige Pull-Bedarf der Szene (verifiziert): `wallMesh(WallId)`,
   `allWallMeshes()`, `roofMeshes()`, `slabMeshes()`, `stairMeshes()`.
   **Rückgabetypen sind port-frei fixiert (Review-MED-1, tragend fürs
   Kriterium):** die Port-Ebene liefert die Aggregat-Pulls heute als
   **Driving-Port-Structs** (`WallMesh`/`RoofMesh`/… aus
   `view_model_port.h`) — gäbe `mesh_source.h` diese zurück, trüge
   `view/` wieder einen driving-Include und risse das Abnahme-Kriterium
   erneut. Daher liefern die Aggregat-Methoden
   **`std::map<Id, TriangleMesh>`** (reine Modelltypen — deckungsgleich
   mit dem szenen-internen Halte-Typ), `wallMesh(WallId)` ein
   `std::optional<TriangleMesh>`-Äquivalent des heutigen Einzel-Pulls.
   Deklariert im **view**-Teil, weil sie den Bedarf des Konsumenten
   beschreibt (Dependency-Inversion im Kleinen).
2. **`ui/command/view_model_mesh_source.{h,cpp}`** (neu): implementiert
   `MeshSource` **über den `ViewModelPort`** — das einzige
   driving-Port-Include der ui — und **konvertiert** die
   Port-Structs (`std::vector<WallMesh>` …) in die port-freien
   `MeshSource`-Rückgabetypen (Review-MED-1; die Konversion ist
   DoD-Punkt). Include-Richtung: `command/` → `view/mesh_source.h` +
   `hexagon/ports/driving/view_model_port.h` (eine Richtung je
   Verzeichnis ✓).
3. **`ViewerScene`/`ViewerWidget`** (nach `ui/view/` verschoben):
   halten statt `ViewModelPort&` eine **`MeshSource&`** — ihr einziges
   Port-Include ist der driven `ModelChangedPort`. Der
   [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)-Vertrag
   (Callback pullt, mutiert nie, Re-Entranz strukturell ausgeschlossen)
   bleibt **wörtlich erhalten** — der Pull läuft nur durch die Naht.
4. **Composition Root `src/main.cpp`:** baut
   `command::ViewModelMeshSource source(service)` und
   `view::ViewerWidget viewer(source)`; `subscribe`/`unsubscribe`
   unverändert.

**Include-Richtungs-Bilanz danach (prüfbar per grep):**
`ui/view/**` → nur `ports/driven/` (+ model + Qt); `ui/command/**` →
nur `ports/driving/` (+ model + `view/mesh_source.h`, ui-intern
port-frei); kein Verzeichnis mischt. Regel E unberührt (alles unter
`ui/`), Regel B unberührt (ui-interne Includes sind adapter-intern).

## 2. Definition of Done

- [x] **Struktur:** `viewer_scene.{h,cpp}` + `viewer_widget.{h,cpp}` per
      `git mv` nach `src/adapters/ui/{view}/`; neu
      `src/adapters/ui/view/{mesh_source}.h` (pure-virtual, port-frei,
      nur Modelltypen — Aggregat-Rückgaben `std::map<Id,TriangleMesh>`,
      Review-MED-1) und
      `src/adapters/ui/command/{view_model_mesh_source}.{h,cpp}`
      (`MeshSource` über `ViewModelPort` **inkl. Port-Struct→Modelltyp-
      Konversion**); `src/adapters/CMakeLists.txt` nachgezogen;
      `qt_probe.cpp` bleibt an der Wurzel (port-frei, benannt).
      **Commit-Schnitt ([AGENTS.md §2.8](../../../../AGENTS.md),
      Review-MED-2): Commit i = reiner `git mv` (nur Pfade), Commit ii =
      Include-/Signatur-/Naht-Änderungen** — sonst fällt die
      Rename-Detection unter die 50-%-Schwelle.
- [x] **done-archive-codepaths-Nachzug (Review-MED-3):** fünf
      eingefrorene Archiv-Pläne referenzieren die alten Viewer-Pfade
      **gerootet** (slice-012/013b/014b/015b/016b →
      `src/adapters/ui/viewer_scene.*`) — nach dem Move würden sie das
      codepaths-Gate reißen. Behandlung: **`d-check:ignore`-Marker**
      (Präzedenz slice-008a — bewahrt den historischen Text, statt
      eingefrorene Pläne inhaltlich umzuschreiben; die Marker-Ergänzung
      ist die benannte, minimale Ausnahme vom Freeze).
- [x] **Naht:** `ViewerScene`/`ViewerWidget` konsumieren `MeshSource&`
      statt `ViewModelPort&`; **kein** `ports/driving/`-Include mehr
      unter `ui/view/**`, **kein** `ports/driven/`-Include unter
      `ui/command/**` — Grep-Beleg beider Richtungen in der
      Closure-Notiz (Abnahme-Kriterium). Stale Rollen-Kommentare
      nachgezogen (der `viewer_widget.h`-Kopf nennt das Widget heute
      pauschal „Driving Adapter" — nach der Trennung: view = driven
      Beobachter/Renderer, command = driving Aufruf-Sitz).
- [x] **Verhaltens-Neutralität:** [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)-**Verhaltensvertrag**
      erhalten (Review-LOW-1: der Pull ist künftig adapter-intern durch
      die Naht vermittelt — der Kern↔Adapter-Vertrag [nur `element_id`+`op`
      gepusht, Pull im Callback, keine Mutation] bleibt unberührt;
      Push-Notify/Pull-State, Re-Entranz-Verbot strukturell;
      [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)-AK-Tests
      + Szenen-Surrogat-Tests unverändert grün; genau EIN wirksames
      Szenen-Update je Wand-Meldung bleibt getestet); `make test` grün
      (Testanzahl unverändert — die bestehenden Viewer-Tests ziehen nur
      Include/Verdrahtung nach, das Orakel bleibt); GL-Smoke/`xvfb`
      unberührt; `make acc-002-beleg`-Pfad funktionsfähig (Viewer-API
      `viewer->scene()` bleibt).
- [x] **Spec-first / Doku:** `spec/architecture.md` — ui-Komponenten-
      Beschreibung um die Richtungs-Unterteilung ergänzt (view = driven
      Beobachter/Rendering, command = driving App-Aufrufe;
      meilensteinfrei, ADR-frei im Körper,
      [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths));
      Doku-Pfad-Sweep (codepaths-Gate fängt gerootete Alt-Pfade). Kein
      Lastenheft-Touch, kein ADR (Regel E bleibt — die Alternative hätte
      einen gebraucht), kein Schema.
- [x] `tools/arch-check.sh` **durchgehend grün** (A–E, P1/P2; Regel E
      prüft `src/adapters/ui/` als Präfix — Unterordner bleiben gedeckt);
      `make gates` grün; **keine** `.a-check.yml`; unabhängiges
      Code-Review vor Closure
      ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
      n/a — keine Geometrie); CHANGELOG; Roadmap-Quergewerk-Eintrag
      (gemeinsam mit slice-028); Closure-Notiz mit Grep-Beleg +
      Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/ui/{view}/` (`viewer_scene.{h,cpp}`, `viewer_widget.{h,cpp}` via `git mv`) | Move | driven-Seite (Beobachter-Implementierungen + Rendering) |
| `src/adapters/ui/view/{mesh_source}.h` | neu | ui-interne, port-freie Pull-Schnittstelle (Naht) |
| `src/adapters/ui/command/{view_model_mesh_source}.h` + `.cpp` | neu | driving-Seite: `MeshSource` über `ViewModelPort` |
| `src/adapters/CMakeLists.txt` | ändern | Quell-Pfade + neue Quelle |
| `src/main.cpp` | ändern | Verdrahtung `ViewModelMeshSource` → `ViewerWidget` |
| `tests/adapters/test_viewer_scene.cpp` · `test_viewer_widget.cpp` | ändern | Includes + Konstruktion über die Naht (Fixture erhält `ViewModelMeshSource`-Member vor der Szene; Orakel unverändert) |
| `docs/plan/planning/done-archive/slice-{012,013b,014b,015b,016b}-*.md` | ändern (minimal) | `d-check:ignore`-Marker an gerooteten Alt-Viewer-Pfaden (Review-MED-3, Präzedenz slice-008a) |
| `harness/conventions.md` | ändern | GUI-Adapter-Modus-Zeile: Deklarations-Drift „nur Driving-Ports" korrigiert (Review-LOW-2) |
| `spec/architecture.md` | ändern | ui-Richtungs-Unterteilung deklarieren (Spec-first) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Quergewerk-Eintrag (gemeinsam mit slice-028) |
| `CHANGELOG.md` | ändern (Closure) | Unreleased-Eintrag |
| `docs/reviews/{2026-07-03-slice-029-plan,2026-07-03-slice-029-code-review}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report + Code-Review |

## 4. Trigger

- [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review
  **und** Projektinhaber-Abnahme des Entwurfs; empfohlene Reihenfolge:
  **nach** slice-028 (kleinere Move-Konflikte, ein Quergewerk-Strang) —
  fachlich unabhängig, auch parallel möglich.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Code-Review 0 HIGH → zusammen mit
  slice-028 **Meldung an den Projektinhaber** („Umbauten durch") für den
  a-check-Pilot-Schnitt mit voller Richtungs-Modellierung.

## 6. Risiken und offene Punkte

- **Transitive Include-Frage (benannt):** `ui/command/`-Header
  inkludieren `view/mesh_source.h` (port-frei) — dadurch entsteht
  **keine** driven-Sicht in `command/`; umgekehrt inkludiert `view/`
  **nichts** aus `command/` (Dependency-Inversion: die Schnittstelle
  wohnt beim Konsumenten). Das Abnahme-Kriterium (direkte Port-Includes
  je Verzeichnis) ist damit auch transitiv sauber erfüllt.
- **`ViewerScene` bleibt Qt-frei und display-frei**
  ([ADR-0009](../../adr/0009-gui-framework-qt6.md) (f),
  Szenen-Surrogat der AK-Tests) — der Move nach `view/` ändert daran
  nichts; die Surrogat-Tests bleiben das Orakel der
  Verhaltens-Neutralität.
- **Konstruktor-Signaturen ändern sich** (`ViewModelPort&` →
  `MeshSource&`): betrifft `main.cpp` + zwei Testdateien — kein
  öffentlicher Vertrag über das Repo hinaus; die
  `StructureEditService`-/Port-Ebene ist unberührt (Review-LOW-3).
  **Lebenszeit-Ordnung in `main.cpp` (Review-LOW-4):**
  `ViewModelMeshSource source` wird **nach** `service` und **vor** dem
  Qt-`window`/`viewer` deklariert — das Widget hält eine nicht-besitzende
  `MeshSource&`, die Quelle muss es überleben (dieselbe Ordnung, die
  `service` heute schon einhält).
- **Doppel-Implementierung des Beobachters** (`ViewerWidget` delegiert an
  `ViewerScene`, beide implementieren `ModelChangedPort`): bleibt in
  diesem Slice **unverändert** (nur verschoben) — eine Konsolidierung
  wäre Verhaltens-/Vertrags-Arbeit außerhalb des Auftrags (benannt für
  später, kein Scope-Creep).
- **Benennung `command/` (Abnahme-Punkt):** heute sitzt dort nur die
  Pull-Query (`MeshSource`-Implementierung) — der Name folgt der
  Pilot-Empfehlung und ist zukunftsfest (künftige GUI-Edit-Kommandos →
  `EditStructurePort` haben dort ihren Ort); die Alternative `query/`
  wäre heute präziser, morgen enger; das
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review bevorzugt
  **`driving/`** (richtungs-treu, symmetrisch zu `ports/driving/`,
  CQRS-sauber). Empfehlung des Plans: `command/` (Pilot-Wortlaut) —
  die Namenswahl ist ein expliziter **Abnahme-Punkt** des
  Projektinhabers, kein Blocker.
- **arch-check-Regel E** matcht per Präfix `src/adapters/ui/` — die
  neuen Unterordner sind automatisch gedeckt (verifiziert am
  Regex-Muster `^src/(adapters/ui/|main\.cpp)`); keine Gate-Anpassung
  nötig.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter

- **Modus:** GF (deklariert: `src/adapters/ui/` — die Deklarationszeile
  in [`harness/conventions.md`](../../../../harness/conventions.md)
  §Modus-Deklaration nennt historisch „nur Driving-Ports"; die reale ui
  trägt seit dem Viewer auch die driven Beobachter-Implementierung —
  genau die Unschärfe, die dieser Slice verzeichnislich auflöst).
  **Die Deklarations-Drift wird in diesem Slice mitkorrigiert
  (Review-LOW-2):** die Modus-Tabellen-Zeile des GUI-Adapters wird auf
  „Driving-Ports + driven Beobachter-Implementierung
  (view/command-Richtungs-Trennung)" nachgezogen (die Modus-Tabelle ist
  lebende Deklaration, kein immutabler `MR`-Eintrag). Verhaltensneutraler
  Struktur-Schnitt, Orakel = bestehende AK-/Surrogat-Tests.

### Sub-Area: Test-Infrastruktur / Spec-Schreibung

- **Modus:** je GF; Tests ziehen nur Verdrahtung nach (Orakel
  unverändert); `architecture.md`-Deklaration ist die Spec-first-Hälfte.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-07-03):** Zwei-Commit-Split
(`dd01fc7` = 4× 100-%-Rename; `c0a12ae` = Naht + Nachzüge).
**Abnahme-Kriterium belegt (grep je Adapter-Verzeichnis):**
`io`/`geometry`/`persistence` nur driven, `plugin` nur driving,
ui-Wurzel (`qt_probe`) port-frei, `ui/view/` nur driven, `ui/command/`
nur driving — **kein Verzeichnis mischt**. `MeshSource` port-frei
(Aggregate `std::map<Id,TriangleMesh>`, Review-MED-1); Dependency-
Inversion richtungs-korrekt (`command/` → `view/mesh_source.h`, nie
umgekehrt). **Verhaltens-Neutralität vom Code-Review semantisch
verifiziert** (loadAll/reloadKeyed/Wall-Case äquivalent; der
[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)-Vertrag „genau EIN wirksames Update je Wand-Meldung" bleibt
exakt; Callback→Repaint unverändert); `make gates` grün (228/228,
Testanzahl unverändert), Regel E/B unberührt grün. conventions-
GUI-Modus-Zeile korrigiert (Deklarations-Drift „nur Driving-Ports");
architecture-ui-Zeile + Baum. Namenswahl `command/` vom Projektinhaber
mit dem Go bestätigt. Code-Review **0 HIGH / 0 MED / 1 LOW / 2 INFO**.

**Lerneintrag:** (a) **Marker-Reduktion 5→2 (Code-Review-LOW-1):** von
den fünf im Plan genannten Archiv-Fundstellen brauchten nur die zwei
**literalen** Pfade (012/013b) Marker — das Trio 014b/015b/016b trägt
Brace-Formen, die das codepaths-Gate als Glob behandelt (gate-exempt);
die Umsetzung ist freeze-schonender als der überspezifizierte Plantext.
(b) Die MeshSource-Naht zeigt das Muster „Schnittstelle beim
Konsumenten deklarieren" als Richtungs-Reiniger für
Push-Notify/Pull-State-Beobachter — wiederverwendbar, falls weitere
Misch-Kopplungen auftauchen. (c) Stale „MED-2"-Altkommentar in
reloadKeyed als Gelegenheits-Pflege benannt (Code-Review-INFO-1).

**Nachfolge:** zusammen mit slice-028 → Meldung an den Projektinhaber
(a-check-Pilot-Schnitt); künftige GUI-Edit-Kommandos haben mit
`ui/command/` ihren deklarierten Ort.
