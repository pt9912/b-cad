---
id: slice-053
titel: Hauptfenster als testbarer Adapter — Menü und Schließ-Behandlung raus aus dem Composition-Root
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 053: Hauptfenster als testbarer Adapter (Struktur-Vorläufer)

**Status:** done (2026-07-27) — **Struktur-Vorläufer**, verhaltens-invariant. **Zwei
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe
gefahren:** Lauf 1 (2026-07-26, 2 HIGH / 6 MED / 5 LOW / 2 INFO,
[Report](../../../reviews/2026-07-26-slice-053-plan.md)) → §9; Lauf 2 (2026-07-27, **1 HIGH** / 5 MED /
4 LOW / 2 INFO, [Report](../../../reviews/2026-07-27-slice-053-plan-2.md)) → §10. **Beide eingearbeitet.**

**Abhängigkeit erfüllt: [`slice-054`](../done/slice-054-manage-project-port.md) ist seit 2026-07-27
`done`** — der `ManageProjectPort` existiert. Lauf-1-HIGH-1 hatte belegt, dass „Handler ziehen mit um"
und „keine neue Kante" ohne ihn nicht gleichzeitig einlösbar sind. **Dieser Plan ist auf den
tatsächlich gelieferten Port geprüft** (Lauf 2), nicht auf den erwarteten. **Sequenz: 054 → 053 →
[`slice-052a`](../in-progress/slice-052a-sitzungs-zustand-und-speichern.md) →
[`slice-052b`](../open/slice-052b-neues-projekt.md).**

**Welle:** welle-5-erweiterung (Quergewerk / Struktur-Vorbereitung — Muster
slice-028/029 (in `done/`), die vor der a-check-Umstellung dieselbe Rolle spielten).
**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser — dieselbe Klasse, viermal

Der Slice entsteht nicht aus einer Feature-Idee, sondern aus einem **wiederkehrenden Review-Befund**:

| Wann | Befund |
|---|---|
| slice-047, Verify **B4** | Die Neu-Auflösung des Zeichen-Ziels lag als Lambda in `main.cpp` — **kein** Sensor führte sie aus (Gegenprobe: Aufruf entfernt → 280/280 grün). Aufgelöst, indem der Schritt in den Kern-Use-Case wanderte. |
| slice-052, [MR-006 Lauf 1](../../../reviews/2026-07-26-slice-052-plan.md) MEDIUM-2/-3 | Ziel-Wahl beim Speichern und Auswertung der dreiwertigen Antwort lagen im Menü-Handler — nicht diskriminierend prüfbar. Aufgelöst, indem beide **Entscheidungen** in den Kern wanderten. |
| slice-052a, [MR-006 Lauf 3](../../../reviews/2026-07-26-slice-052a-plan-3.md) **HIGH-1** | Die zugesagte Orakel-Zeile für das **Schließ-Ereignis** ist unter dem eigenen Datei-Plan **nicht herstellbar**: `main.cpp` ist in **kein** Testbinary gelinkt (`src/CMakeLists.txt`:6 baut sie nur ins `b-cad`-Executable; `tests/CMakeLists.txt` linkt `bcad_adapters`/`bcad_hexagon`). |

**Das Muster:** dreimal wurde eine **Entscheidung** aus `main.cpp` herausgezogen — und dreimal blieb die
**Verdrahtung** dort, weil es keinen anderen Ort gibt. Beim Schließ-Ereignis reicht das nicht mehr: die
Rückfrage vor Datenverlust **ist** an das Ereignis gebunden, und ein Ereignis ohne Träger-Klasse hat
keinen Sensor. Der Reviewer-Skill nennt genau das den Pflege-Fall („bei dreimaligem Auftreten desselben
Findings … gibt es eine Fitness Function, die es prüfen würde?").

**Warum es überhaupt geht:** das Repo stellt Qt-Ereignisse längst headless zu —
`tests/adapters/test_canvas_widget.cpp`:108–116 (`QMouseEvent` + `QApplication::sendEvent`, der
Hilfslinien-Zug aus slice-043) und `tests/adapters/test_viewer_widget.cpp`:55–67. Die Testbarkeit
scheiterte nie am **Ereignis**, sondern an der **Verortung**.

## 1. Ziel — und wo was liegt (nach HIGH-1 entschieden)

Das Hauptfenster wird eine **Adapter-Klasse**; die Menü-**Handler** werden es ebenfalls. Beides an
**verschiedenen** Orten, weil `.a-check.yml` die Richtungen trennt:

| Was | Wohin | Warum das trägt |
|---|---|---|
| **Fenster** (`QMainWindow`, Menü-Aufbau, `closeEvent`) | `src/adapters/ui/view/` (`ui_view`, driven) | Es importiert **keinen** Port: die Aktionen rufen injizierte `std::function`s. Präzedenz ist `CanvasWidget` — es löst seit slice-043 über eine `std::function` einen **Schreib**-Vorgang aus, ohne einen Driving-Port zu kennen ([ADR-0019](../../adr/0019-drw-2d-canvas.md) Option A). `architecture.md`:109 („kein Unterverzeichnis mischt beide Richtungen") ist eine Regel über **Imports**, und die bleibt gewahrt. |
| **Handler** (was eine Aktion *tut*) | `src/adapters/ui/command/` (`ui_command`, driving) | Dort ist `→ ports_driving` erlaubt — und ab [`slice-054`](../done/slice-054-manage-project-port.md) gibt es mit `ManageProjectPort` einen Port, den ein Handler rufen **darf**. |
| **Verdrahtung + modale Dialoge** | `src/main.cpp` | Instanz erzeugen, Handler-Methoden als `std::function` ins Fenster geben, Dialoge stellen. **Keine Entscheidung.** |

**Das war der HIGH-1 des ersten Laufs:** die Vorfassung wollte die Handler nach `ui_view` mitnehmen —
sie riefen damals `hexagon/services/`, und `ui_view` darf nur `model`/`ports_driven`
(`.a-check.yml`:33–34). „Handler ziehen mit um" **und** „keine neue Kante" war zusammen unmöglich.
**Seit slice-054 ist es möglich:** das Menü ruft heute `project.openProject(...)` (`main.cpp`:311) und
`project.saveProject(...)` (:341) über den Port — die Zeilen-Belege der Vorfassung (:306/:337) sind
überholt (Lauf-2-MEDIUM-1).

## 1.1 Die Senken — die eine Verdrahtung, die 054 an 053 delegiert hat (Lauf-2-HIGH-1)

`openProject` nimmt die **`DrawingTargetSinks`** als **Methoden-Parameter**
(`src/hexagon/ports/driving/manage_project_port.h`); slice-054 hat das ausdrücklich so entschieden und
die Weitergabe an 053 delegiert (054 §2.1/R2). **Die Vorfassung dieses Plans erwähnte sie mit keinem
Wort** — und genau daran hing der gefährlichste Befund des zweiten Laufs:

> Der Port trug einen **Default** (`sinks = {}`). Ein Handler, der die Senken vergisst, hätte
> kompiliert und eine gültige `DrawingTargetResolution` geliefert; §3-Zeile 5 prüft nur, **dass** der
> Port gerufen wird, und das einzige bestehende Adapter-Orakel
> (`tests/adapters/test_project_open_handler.cpp`:177–193) baut seine Senken **selbst**. Der Verlust
> der Zeichen-Ziel-Neuauflösung — **slice-047-Verify-B4, derselbe Fehler** — wäre unter **allen**
> zugesagten Sensoren grün geblieben. In dem Slice, dessen Zweck Prüfbarkeit ist.

**Entschieden (Projektinhaber 2026-07-27) — beide Hälften:**

1. **Der Default fällt.** `openProject(path, sinks)` ohne `= {}` — Vergessen wird ein
   **Compile-Fehler**, der stärkste verfügbare Sensor. Ein Eingriff in das Artefakt eines
   geschlossenen Slice, bewusst und hier protokolliert: er **verschärft** den Vertrag, ändert kein
   Verhalten, und die zwei Aufrufstellen ohne Senken (in `tests/hexagon/test_manage_project_port.cpp`)
   reichen künftig explizit `{}` — sichtbar statt stillschweigend.
2. **Der Handler führt die Senken** (Konstruktor) **und reicht sie durch**; das Fenster kennt sie
   nicht. Dafür **§3-Zeile 6** mit roter Gegenprobe — ein Compile-Fehler deckt nur das Vergessen, nicht
   das Durchreichen **leerer** Senken.

**Verhaltens-invariant:** kein neues Menü, keine neue Aktion, keine geänderte Reaktion.

## 2. Bewusst NICHT Teil

- **Jede neue Funktion.** Kein „Speichern", kein „Neu", keine Ungesichert-Rückfrage — das sind
  [`slice-052a`](../in-progress/slice-052a-sitzungs-zustand-und-speichern.md) und
  [`slice-052b`](../open/slice-052b-neues-projekt.md). Dieser Slice **ermöglicht** sie, er liefert sie nicht.
- **Die 3D-/2D-Widgets** (`ViewerWidget`, `CanvasWidget`) und ihre Verdrahtung — unverändert.
- **Der `--acc-002-beleg`-Pfad und die CLI** (`runHeadlessCli`) — unberührt; der
  [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg muss **bit-gleich** erzeugbar
  bleiben (§5 R1).
- **Keine Lastenheft-/Spezifikations-Änderung** — es ändert sich kein benutzer-beobachtbares Verhalten.

## 3. Orakel-Schnitt

| # | Zusicherung | Diskriminierende Gegenprobe |
|---|---|---|
| 1 | Das Fenster ist als Adapter-Klasse **headless konstruierbar** (Xvfb wie die übrigen Adapter-Tests, [ADR-0010](../../adr/0010-headless-gl-xvfb.md)) | — (Voraussetzung der übrigen Zeilen) |
| 2 | **`close()` auf dem Fenster** ruft die Schließ-Behandlung (Beobachtung: ein injizierter Haken wird gerufen) | Behandlung entfernt ⇒ rot |
| 3 | Sagt der Haken „nicht schließen", **bleibt das Fenster sichtbar** — die Naht, an der 052a sein „abbrechen" aufhängt | `ignore()` weggelassen ⇒ rot |
| 4 | Eine **Menü-Aktion** ist über `trigger()` auslösbar und ruft ihre injizierte `std::function` | Verdrahtung entfernt ⇒ rot |
| 5 | Der **Handler** (`ui/command/`) ruft für „Öffnen"/„Speichern" den [`slice-054`](../done/slice-054-manage-project-port.md)-Port — geprüft gegen ein Port-Doppel | Aufruf entfernt ⇒ rot |
| 6 | Der Handler **reicht die `DrawingTargetSinks` durch**: das Port-Doppel bekommt beim Öffnen **gesetzte** Callables und meldet die Ids des geladenen Stands an die injizierten Senken (§1.1) | Handler reicht `{}` statt der geführten Senken ⇒ rot. **Nicht** ausreichend als alleiniger Schutz: das reine *Weglassen* fängt seit §1.1-Entscheidung 1 bereits der Compiler |

**Zeile 2/3 sind gegenüber der Vorfassung präzisiert** (Lauf-1-MEDIUM-2): ein per `sendEvent`
zugestelltes `QCloseEvent` prüft die Veto-Wirkung **nicht** — `ignore()` wirkt an `close()`. Geprüft
wird deshalb über `close()` und die **Sichtbarkeit** danach.

**Zeile 5 ist neu** (Lauf-1-MEDIUM-1): ohne sie verspräche 053 nur „die Aktion ruft *irgendeinen*
Handler", während [`slice-052a`](../in-progress/slice-052a-sitzungs-zustand-und-speichern.md) darauf den **Nachweis
der Verwendung** bucht. Erst mit dem Port ist das prüfbar.

**Zeile 6 ist neu** (Lauf-2-HIGH-1, s. §1.1) — und sie ist die Zeile, die diesen Slice vor der
Wiederholung von slice-047-B4 bewahrt: der Aufruf allein sagt nichts darüber, **womit** gerufen wird.

**Zur Verhaltens-Invarianz, ehrlich** (Lauf-1-**HIGH-2**): einen **Vorher**-Sensor für den Menü-Teil
gibt es **nicht** — `make io-smoke` kehrt vor dem GUI-Aufbau zurück (`main.cpp`:432–434, so auch im
[slice-047-Verify](../../../reviews/2026-07-25-slice-047-verify.md) festgehalten), die Adapter-Tests
linken `main.cpp` nicht, und `make acc-002-beleg` deckt den Fenster-**Aufbau**, nicht das Menü. Die
Vorfassung behauptete „Invarianz **belegt**" und nannte drei Sensoren, von denen keiner den
umgezogenen Code ausführt. Richtig ist: die §3-Tabelle ist die **erste** Deckung, die dieser Code je
hatte. Was die drei Läufe belegen, ist die Invarianz **der umliegenden Pfade** — nicht die des
Umzugs selbst. Das ist die benannte Grenze dieses Slice, kein Beleg.

**Benannte Grenze — vollständig aufgezählt** (Lauf-1-MEDIUM-3, vierte Wiederholung dieser Klasse laut
Reviewer-Pflege-Signal, deshalb hier einzeln): (1) die **modalen Dialoge** selbst (`QFileDialog`,
`QMessageBox`) — sie blockieren im Test und werden über eine injizierbare Naht gerufen; (2) die
**Meldungstexte** und die Entscheidung, *welcher* Dialog erscheint; (3) der **Fenstertitel**
(052b-L5 zieht ihn nach); (4) die `.bcad`-**Suffix-Ergänzung** beim Speichern-Dialog; (5) der
**Fenster-Aufbau** in `main` (Tabs, Größe, Titel beim Start). Alles davon bleibt in `main.cpp` und ohne
Sensor — **keine Entscheidung darunter**.

**Was der Umzug damit tatsächlich ist — ehrlich beziffert** (Lauf-2-MEDIUM-5): zieht man die fünf
Grenz-Punkte ab, wandern aus den beiden Menü-Lambdas im Kern **zwei Port-Aufrufe** samt der
Senken-Weitergabe (§1.1) und der Ergebnis-Auswertung in den Handler. Dialog-Aufruf, Abbruch-Prüfung
(`path.isEmpty()`), Suffix-Ergänzung, Meldungstexte, Titel-Setzung und die `try/catch`-Anzeige bleiben
in `main.cpp`. **Der Gewinn dieses Slice ist nicht die Menge des verschobenen Codes, sondern dass das
Verschobene erstmals einen Sensor hat** — plus die Träger-Klasse für das Schließ-Ereignis, an der 052a
seine Rückfrage aufhängt. Wer mehr erwartet, erwartet 052a/052b.

## 4. Definition of Done

- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: `QMainWindow`-Ableitung mit Menü-Aufbau und
      `closeEvent`-Behandlung; **kein Port-Include, kein `hexagon/services/`-Include** — alle Aktionen
      laufen über injizierte `std::function`s ([ADR-0019](../../adr/0019-drw-2d-canvas.md) Option A,
      Präzedenz `CanvasWidget`).
- [ ] **`src/adapters/ui/command/project_menu_handler.{h,cpp}`** (o. ä.): was eine Aktion **tut** —
      ruft den [`slice-054`](../done/slice-054-manage-project-port.md)-`ManageProjectPort`. Orakel §3-5.
- [ ] **`src/main.cpp`**: nur noch Instanz, Verdrahtung, Dialoge und Meldungstexte. Die
      Handler-**Rümpfe** ziehen um; die Dialog-Aufrufe bleiben.
- [ ] **Qt-Ownership und Lebensdauer — hier entschieden, nicht im Vollzug** (Lauf-1-MEDIUM-4,
      Lauf-2-MEDIUM-3: die Vorfassung hatte die Entscheidung nur *verschoben*, und das Plan-Review
      prüft **vor** dem Start). Die drei Antworten:
  - **Widgets:** unverändert. `MainWindow` bekommt die fertigen `QTabWidget`-Kinder vom
    Composition-Root **injiziert** (`setCentralWidget` überträgt das Ownership wie heute an das
    Fenster, `main.cpp`:481–487). 053 baut die Widgets **nicht** selbst — sonst zöge der 3D-/2D-Aufbau
    in einen Slice, der ihn laut §2 ausdrücklich nicht anfasst.
  - **`unsubscribe`:** bleibt im Composition-Root (`main.cpp`:527–528, **vor** der Widget-Zerstörung,
    [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) #5). Begründung: der
    `StructureEditService` gehört dem Root und **überlebt** das Fenster; ein Abmelden im Fenster-Dtor
    würde die Reihenfolge umkehren und die Invariante an eine Qt-Zerstörungsreihenfolge binden.
  - **`--acc-002-beleg`:** der Zweig braucht `setCurrentWidget(viewer)` **vor** `show()` (`main.cpp`:507
    f., in slice-043 teuer erarbeitet). `MainWindow` bekommt dafür einen schmalen Zugriff auf den
    aktiven Tab; die Reihenfolge selbst bleibt im Root. R1 bleibt damit ein Risiko der **Reihenfolge**,
    nicht der **Erreichbarkeit**.
- [ ] **`tests/adapters/test_main_window.cpp`** + **`tests/adapters/test_project_menu_handler.cpp`**:
      §3-Zeilen 1–6, **je mit roter Gegenprobe** im Closure-Text.
- [ ] **Der Port-Default fällt** (§1.1): `openProject(path, sinks)` ohne `= {}` in
      `src/hexagon/ports/driving/manage_project_port.h`; die zwei senkenlosen Aufrufe in
      `tests/hexagon/test_manage_project_port.cpp` reichen explizit `{}`. **Verschärfung des Vertrags,
      keine Verhaltensänderung** — und kein Gate wird gelockert
      (AGENTS [§2.6](../../../../AGENTS.md) n/a).
- [ ] **Build-Listen nachgezogen** (Lauf-1-MEDIUM-5): `src/adapters/CMakeLists.txt` und
      `tests/CMakeLists.txt` zählen Dateien **explizit** auf — genau die Linkage-Tatsache, aus der
      dieser Slice entstand.
- [ ] **`make a-check` grün ohne neue Kante**: `ui_view` importiert nur `model`, `ui_command` nur
      `model`/`ports_driving`. Entsteht eine Kante, ist der Schnitt falsch — **nicht** die Regel
      (AGENTS [§2.6](../../../../AGENTS.md)).
- [ ] **`spec/architecture.md` — beide Hälften der GUI-Adapter-Zeile** (:109; Lauf-1-MEDIUM-6 +
      Lauf-2-MEDIUM-4): (a) `command/` nennt heute nur die „`MeshSource`-Naht" → um die
      **Kommando-Handler** ergänzen; (b) **`view/` ist als „driven: Beobachter-Implementierungen …
      + Rendering" beschrieben — ein Menü-Fenster ist weder das eine noch das andere.** Die
      Verantwortlichkeit ist so zu fassen, dass sie das port-freie Fenster **deckt**, ohne die
      Richtungs-Regel aufzuweichen (die bleibt eine Aussage über **Imports**). Dazu der `view/`-Zweig
      des §2.1-Baums.
- [ ] **CHANGELOG** [Unreleased]-Eintrag (Struktur, verhaltens-invariant).
- [ ] **Kein** Lastenheft-/Spezifikations-/Handbuch-Eintrag — nichts wird benutzer-sichtbar. (Geprüft
      und **verneint**, nicht vergessen — Lehre aus slice-047 V1.)
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`: nach der 054-Closure ist
      `in-progress/` wieder leer und der Sentinel gesetzt — 053 ist damit erneut der **erste**
      Slice und entfernt ihn im selben Commit, [MR-017](../../../../harness/conventions.md);
      Lauf-1-LOW-2, Begründung nach Lauf-2-LOW-1 richtiggestellt);
      **`make io-smoke` grün**; **`make acc-002-beleg`** erzeugt ein Bild
      ([ADR-0010](../../adr/0010-headless-gl-xvfb.md)). **Diese drei belegen die umliegenden Pfade,
      nicht den Umzug selbst** (§3).

## 5. Plan (vor Code)

*(Lauf-1-MEDIUM-5: der Abschnitt fehlte.)*

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/view/main_window.{h,cpp}` | neu | Fenster als Adapter-Klasse, port-frei |
| `src/adapters/ui/command/project_menu_handler.{h,cpp}` | neu | was die Aktionen tun; ruft den 054-Port |
| `src/adapters/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | explizite Datei-Listen |
| `src/main.cpp` | ändern | Instanz + Verdrahtung + Dialoge; die **zwei Port-Aufrufe** raus (§3, Umfang ehrlich beziffert) |
| `src/hexagon/ports/driving/manage_project_port.h` | **ändern** | der Default `sinks = {}` fällt (§1.1) — Vergessen wird Compile-Fehler |
| `tests/hexagon/test_manage_project_port.cpp` | **ändern** | die zwei senkenlosen Aufrufe reichen explizit `{}` |
| `tests/adapters/test_main_window.cpp` | neu | §3-Zeilen 1–4 |
| `tests/adapters/test_project_menu_handler.cpp` | neu | §3-Zeilen 5 **und 6** |
| `spec/architecture.md` | ändern | GUI-Adapter-Zeile (**`command/` und `view/`**) + §2.1-Baum |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Reporte | **liegen** | [Lauf 1](../../../reviews/2026-07-26-slice-053-plan.md) + [Lauf 2](../../../reviews/2026-07-27-slice-053-plan-2.md) (Lauf-1-LOW-5) |

**Nicht berührt — begründet:**

- `.a-check.yml` (keine neue Kante), `spec/lastenheft.md`, `spec/spezifikation.md`, `docs/user/`,
  `data-model.yaml`/`schema.sql`, `docs/plan/adr/`.
- **`tests/adapters/test_project_open_handler.cpp`** (Lauf-2-LOW-3) — es prüft den **Kern**-Use-Case
  über die freien Funktionen und baut seine Senken selbst; der neue Handler-Test tritt **daneben**,
  nicht an seine Stelle. Genannt, damit die Nicht-Berührung geprüft und nicht übersehen ist.
- **`slice-052a`/`slice-052b`** (Lauf-2-MEDIUM-2) — beide buchen noch auf
  `src/adapters/ui/view/main_window.*` bzw. auf dem verworfenen `sendEvent`-Mechanismus, und die von
  053 geschaffene Handler-Heimat kommt bei ihnen nicht vor. **Entscheidung (Projektinhaber
  2026-07-27): nachgezogen wird beim Start des jeweiligen Slice** — ihr eigener
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
  steht ohnehin aus (052a: vierter, 052b: zweiter) und prüft dann gegen das, was 053 **geliefert** hat.
  Dasselbe Muster, das diesen Lauf 2 nötig gemacht hat — kein Pflegeaufwand auf Vorrat.

## 6. Risiken

- **R1 — der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg hängt am Fenster-Aufbau.** `main.cpp` zeigt heute das Fenster und grabbt den
  Viewer-Tab; die Reihenfolge (`setCurrentWidget(viewer)` **vor** `show()`) ist in slice-043 teuer
  erarbeitet worden, weil sonst der GL-Kontext nicht initialisiert. Der Umzug darf sie nicht brechen —
  §3-Zeile 5 bzw. das Regressions-Netz.
- **R2 — verhaltens-invariant heißt: keine Gelegenheit nutzen.** Beim Umzug fällt auf, was man
  „gleich mitmachen" könnte. Genau das ist verboten; jede Verhaltensänderung gehört in 052a/052b und
  macht den Diff unprüfbar.
- **R3 — Qt-Ownership.** `QMainWindow` übernimmt Kindschaften; beim Umzug darf keine doppelte
  Freigabe oder ein hängender Zeiger entstehen (die Widgets gehören heute dem `QTabWidget`).

## 7. Trigger

- **[MR-006 Lauf 3](../../../reviews/2026-07-26-slice-052a-plan-3.md) HIGH-1** zu slice-052a
  (Orakel-Zeile nicht herstellbar) + **INFO-2** desselben Laufs (Pflege-Signal: dritte Wiederholung der
  Klasse). Projektinhaber-Entscheidung 2026-07-26: **eigener Vorläufer** statt weiterer Scope-Zuwachs
  in 052a.

## 8. Closure-Trigger

- §3-Zeilen 1–6 grün + je einmal diskriminierend belegt; die drei Regressions-Läufe grün;
  `make gates` + `make io-smoke` + `make acc-002-beleg` grün; Closure-Notiz.

## 9. MR-006-Einarbeitung (erster Lauf, 2026-07-26)

Report: [`2026-07-26-slice-053-plan.md`](../../../reviews/2026-07-26-slice-053-plan.md) —
**2 HIGH / 6 MEDIUM / 5 LOW / 2 INFO, „nicht startbar"**.

| # | Behandlung |
|---|---|
| **HIGH-1** (Handler-Umzug vs. „keine neue Kante") | **[`slice-054`](../done/slice-054-manage-project-port.md)** vorgeschaltet: mit `ManageProjectPort` darf ein `ui/command/`-Handler die Use-Cases rufen. §1 legt die Verortung jetzt **dreiteilig** fest (Fenster `view/` port-frei · Handler `command/` über den Port · `main` nur Verdrahtung). |
| **HIGH-2** (Invarianz-Sensoren führen den Code nicht aus) | §3 sagt jetzt das Gegenteil der Vorfassung: einen **Vorher**-Sensor gibt es **nicht**; die §3-Tabelle ist die **erste** Deckung dieses Codes. Die drei Läufe belegen die **umliegenden** Pfade. |
| **MEDIUM-1** (053 verspricht weniger, als 052a bucht) | **§3-Zeile 5** neu: der Handler ruft den Port — erst damit ist der „Nachweis der Verwendung" herstellbar. |
| **MEDIUM-2** (`ignore()` nicht über `sendEvent` prüfbar) | §3-Zeilen 2/3 prüfen über **`close()`** und die **Sichtbarkeit** danach. |
| **MEDIUM-3** (Grenze unvollständig) | §3 zählt **fünf** Punkte einzeln auf (Dialoge, Meldungstexte, Titel, `.bcad`-Suffix, Fenster-Aufbau). |
| **MEDIUM-4** (Ownership/[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)/`unsubscribe` unentschieden) | eigene DoD-Zeile mit den drei konkreten Fundstellen — **zu entscheiden**, nicht zu benennen. |
| **MEDIUM-5** (kein „Plan (vor Code)", CMake-Listen fehlen) | **§5** neu; beide `CMakeLists.txt` in DoD und Plan. |
| **MEDIUM-6** (`architecture.md`-Begründung falsch) | die DoD nennt jetzt die **richtige** Zeile (:109, GUI-Adapter) **und** den §2.1-Baum; die Behauptung „der Baum zählt die Widgets auf" ist zurückgenommen. |
| **LOW-1** („offscreen" vs. [ADR-0010](../../adr/0010-headless-gl-xvfb.md)) | §3-Zeile 1 spricht von **Xvfb** wie die übrigen Adapter-Tests. |
| **LOW-2** | Ruhe-Marker-Toggle in der DoD-Gates-Zeile (053 wäre der erste `in-progress`-Slice). |
| **LOW-3** | Verweis auf slice-028/029 ohne falsches Verzeichnis. |
| **LOW-4** | `lastenheft_refs` trägt [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien). |
| **LOW-5** | der eigene Report steht als Artefakt im Plan (§5). |
| **INFO-1** (vierte Wiederholung „Grenze als vollständig deklariert") | die Aufzählung ist einzeln nummeriert statt pauschal — und die **Ursache** der Klasse ist mit 054 adressiert. |
| **INFO-2** (der Code kommt erstmals unter `coverage-gate`) | benannt: der Umzug hebt bisher coverage-ausgenommenen Code in den gemessenen Bereich; ein Absinken der Quote ist **erwartbar** und kein Regress. |

## 10. MR-006-Einarbeitung (zweiter Lauf, 2026-07-27)

Report: [`2026-07-27-slice-053-plan-2.md`](../../../reviews/2026-07-27-slice-053-plan-2.md) —
**1 HIGH / 5 MEDIUM / 4 LOW / 2 INFO, „nicht startbar"**. Unabhängiger Reviewer ≠ Plan-Autor ≠ Autor
des Lauf-1-Reports. **Prüfrahmen dieses Laufs:** der Plan entstand **vor** slice-054 — geprüft wurde
deshalb gegen den **tatsächlich gelieferten** Port, nicht gegen den erwarteten.

| # | Behandlung |
|---|---|
| **HIGH-1** (Senken kommen im Plan nicht vor; Verlust bliebe unter allen Sensoren grün) | **§1.1 neu** + **§3-Zeile 6** neu + zwei DoD-Zeilen. Beide Hälften entschieden: der Port-Default fällt (Compile-Fehler beim Vergessen), der Handler führt und reicht die Senken durch (Orakel mit roter Gegenprobe). |
| **MEDIUM-1** (Plan beschreibt das **vor**-054-`main.cpp`) | Zeilen-Belege auf den Ist-Stand gezogen: Menü ruft `project.openProject` (:311) / `project.saveProject` (:341); `unsubscribe` :527–528; `--acc-002-beleg` :507 f.; Fenster-Aufbau :481–487. |
| **MEDIUM-2** (052a/052b buchen auf überholten Pfaden) | **Nicht** in 053 nachgezogen — Entscheidung mit Begründung in §5 „Nicht berührt". |
| **MEDIUM-3** (Lauf-1-MEDIUM-4 nur **scheinbar** erledigt: „zu entscheiden" in den Vollzug verschoben) | Die drei Antworten stehen jetzt **im Plan**: Widgets injiziert (053 baut sie nicht), `unsubscribe` bleibt im Root (der Service überlebt das Fenster), `--acc-002-beleg` über einen schmalen Tab-Zugriff bei unveränderter Reihenfolge. |
| **MEDIUM-4** (nur die `command/`-Hälfte der `architecture.md`-Zeile nachgezogen) | Die DoD nennt jetzt **beide** Hälften — die `view/`-Verantwortlichkeit („driven: Beobachter + Rendering") deckt ein Menü-Fenster nicht. |
| **MEDIUM-5** (Umfang des Umzugs beschönigt) | §3 beziffert ihn ehrlich: **zwei Port-Aufrufe** + Senken + Ergebnis-Auswertung; der Gewinn ist der **Sensor**, nicht die Code-Menge. |
| **LOW-1** (Toggle-Begründung überholt) | richtiggestellt: nach der 054-Closure ist `in-progress/` wieder leer, 053 ist erneut der erste Slice. |
| **LOW-2** (`adr_refs` ohne [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)/[ADR-0001](../../adr/0001-hexagonale-architektur.md)) | ergänzt. |
| **LOW-3** (`test_project_open_handler.cpp` ungenannt) | als **begründet unberührt** aufgenommen. |
| **LOW-4** (Kopf/Zeitform noch vor der 054-Lieferung) | Kopf sagt jetzt: 054 ist `done`, der Plan ist gegen den gelieferten Port geprüft. |

**Stand der Lauf-1-Findings** (vom Lauf-2-Reviewer am Artefakt nachgeprüft, nicht der Behauptung
geglaubt): beide HIGH **echt aufgelöst** · MEDIUM-2/-3/-5 und alle fünf LOW aufgelöst · MEDIUM-1 und
MEDIUM-6 **teilweise** (jetzt zu Ende geführt) · MEDIUM-4 **nur scheinbar** (jetzt entschieden).

**Startbar:** ja — der HIGH ist aufgelöst, alle MEDIUM/LOW eingearbeitet. Ein dritter Lauf ist nicht
erforderlich: die Änderungen präzisieren den bestehenden Schnitt und fügen einen Sensor hinzu, sie
wechseln keine Lösungsrichtung.

## 11. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter

- **Modus:** GF; **Dichte:** mittel — eine Klasse, ein Umzug, sechs Orakel-Zeilen; der Aufwand steckt in
  der **Invarianz** (R1/R3), nicht im Neubau.
- **Risiko:** niedrig-mittel — kein neues Verhalten, aber der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg-Pfad und die
  Qt-Ownership sind empfindlich.

## 12. Closure-Notiz

**Ausgeführt 2026-07-27.** `make gates` **EXIT=0** (docs-check 0 Befunde / 258 Dateien · a-check 0 ·
arch-check ok · **301/301** Tests · Coverage 91,5 %), `make io-smoke` EXIT=0, **`make acc-002-beleg`
EXIT=0** (`1276x753, 9 Wand-Netze` — die in slice-043 erarbeitete Reihenfolge
`setCurrentWidget(viewer)` **vor** `show()` hat den Umzug überlebt, R1).

### Was entstanden ist

- **`src/adapters/ui/view/main_window.{h,cpp}`** — `MainWindow` (`QMainWindow`), port-frei: Menü-Aufbau
  + `closeEvent`. Aktionen sind injizierte `std::function`s, das Schließ-Veto ein injizierter
  `CloseGuard`. **Kein** Port-Include, **kein** `command/`-Include.
- **`src/adapters/ui/command/project_menu_handler.{h,cpp}`** — `ProjectMenuHandler` am
  `ManageProjectPort`. **Die erste reale `ui_command`-Datei am Port** — damit sieht `make a-check` die
  Kante `ui_command → ports_driving` an einem Artefakt statt nur in der Konfiguration. Das ist der
  Beleg, den slice-054 ausdrücklich offen gelassen hatte (054 §1).
- **`src/main.cpp`** — `installFileMenu` ist zu `makeFileActions` geworden: Dialoge, Meldungstexte,
  Titel, Suffix und `try/catch` bleiben, der Port-Aufruf zieht in den Handler.
- **`tests/adapters/test_main_window.cpp`** (5 Tests) + **`test_project_menu_handler.cpp`** (5 Tests),
  beide in `bcad_adapter_tests`; Build-Listen in `src/adapters/CMakeLists.txt` und
  `tests/CMakeLists.txt` nachgezogen.
- **Port gehärtet** (§1.1): `openProject(path, sinks)` ohne Default.

### Orakel §3 — je mit roter Gegenprobe (gemessen)

| # | Gegenprobe | Ergebnis |
|---|---|---|
| 1 | — (Voraussetzung; belegt durch `IsConstructibleHeadlessWithCentralWidget`) | grün |
| 2/3 | `closeEvent`-Behandlung entfernt | **2 rot**: `CloseRunsTheCloseGuard`, `CloseGuardVetoKeepsTheWindowVisible` |
| 4 | Menü-Verdrahtung der Öffnen-Aktion entfernt | **1 rot**: `TriggeringMenuActionsCallsTheInjectedHandlers` |
| 5 | Handler ruft `saveProject` nicht mehr | **2 rot**: `SaveAsCallsThePortWithTheGivenPath`, `ErrorsPropagateNeutrally` |
| 6 | Handler reicht `{}` statt der gehaltenen Senken durch | **2 rot**: `HandlerForwardsTheDrawingTargetSinks`, `SinksAreForwardedOnEveryOpen` |
| §1.1 | Senken beim Port-Aufruf **weggelassen** | **Compile-Fehler** (gemessen): `project_menu_handler.cpp:10:32: error: no matching function for call to '…ManageProjectPort::openProject(const std::filesystem::path&)'` |

Die letzte Zeile ist der Beleg, dass die Härtung wirkt: was der zweite Plan-Review als still
verlierbar identifiziert hat, ist jetzt nicht mehr übersetzbar.

### Abweichung vom Plan — eine, benannt

Der Plan sagte für den `--acc-002-beleg`-Zweig einen „schmalen Zugriff auf den aktiven Tab" zu. **Der
wurde nicht gebraucht:** der Composition-Root erzeugt `tabs`, `viewer` und `canvas` selbst und behält
seine Zeiger; das Fenster bekommt die fertigen Tabs nur injiziert. Die Ownership-Entscheidung (§4)
gilt damit unverändert — sie brauchte weniger Mechanik als angenommen. **Kein** Accessor auf
`MainWindow`.

Zusätzlich: die `FileActions` reichen beim Auslösen das **Fenster selbst** als Dialog-Eltern durch
(`std::function<void(QWidget*)>`). Die erste Fassung hielt stattdessen einen nachgereichten
Zeiger-auf-Zeiger im Root — `misc-const-correctness` hat ihn im `lint`-Gate beanstandet, und die
Auflösung ist die bessere Bauform: nur das Fenster kennt sich zum Auslöse-Zeitpunkt.

### Was dieser Slice **nicht** belegt (unverändert aus §3)

Einen **Vorher**-Sensor für den Menü-Teil gibt es nicht — die §3-Tabelle ist die **erste** Deckung
dieses Codes. `make io-smoke` kehrt vor dem GUI-Aufbau zurück, `make acc-002-beleg` deckt den
Fenster-**Aufbau**. Beide belegen die **umliegenden** Pfade, nicht den Umzug selbst. Coverage bleibt
bei 91,5 % — der Code kam erstmals in den gemessenen Bereich (INFO-2 des ersten Laufs), ohne die Quote
zu senken.

### Doku

`spec/architecture.md`: die GUI-Adapter-Zeile nennt jetzt **beide** Richtungen vollständig — `view/`
deckt ausdrücklich Fenster/Widgets, deren Aktionen über injizierte Callables laufen, `command/` die
Kommando-Handler an Driving Ports; dazu der Satz, dass die Richtungs-Regel eine Aussage über
**Imports** ist (ein `view/`-Fenster darf ein Kommando *auslösen*, solange es den Port nicht *kennt*).
§2.1-Baum entsprechend. `CHANGELOG` [Unreleased]. **Kein** Lastenheft-/Spezifikations-/Handbuch-Eintrag
— geprüft und verneint: kein benutzer-sichtbares Verhalten ändert sich.
