---
id: slice-053
titel: Hauptfenster als testbarer Adapter — Menü und Schließ-Behandlung raus aus dem Composition-Root
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)]
adr_refs: [[ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 053: Hauptfenster als testbarer Adapter (Struktur-Vorläufer)

**Status:** open — **Struktur-Vorläufer**, verhaltens-invariant. Eigenes [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
**2026-07-26 gefahren: 2 HIGH / 6 MEDIUM / 5 LOW / 2 INFO → nicht startbar**
([Report](../../../reviews/2026-07-26-slice-053-plan.md)); alle Findings eingearbeitet (§10).
**Ein zweiter Lauf vor dem Start.**

**Abhängigkeit: [`slice-054`](../done/slice-054-manage-project-port.md) muss zuerst laufen** — HIGH-1 hat
belegt, dass „Handler ziehen mit um" und „keine neue Kante" ohne den `ManageProjectPort` nicht
gleichzeitig einlösbar sind. **Sequenz: 054 → 053 →
[`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md) →
[`slice-052b`](slice-052b-neues-projekt.md).**

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
sie rufen aber `hexagon/services/` (`main.cpp`:306/:337), und `ui_view` darf nur `model`/`ports_driven`
(`.a-check.yml`:33–34). „Handler ziehen mit um" **und** „keine neue Kante" war zusammen unmöglich.
Ohne [`slice-054`](../done/slice-054-manage-project-port.md) bliebe nur die **leere Bauform** — dann verspräche
053 weniger, als 052a/052b darauf buchen.

**Verhaltens-invariant:** kein neues Menü, keine neue Aktion, keine geänderte Reaktion.

## 2. Bewusst NICHT Teil

- **Jede neue Funktion.** Kein „Speichern", kein „Neu", keine Ungesichert-Rückfrage — das sind
  [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md) und
  [`slice-052b`](slice-052b-neues-projekt.md). Dieser Slice **ermöglicht** sie, er liefert sie nicht.
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

**Zeile 2/3 sind gegenüber der Vorfassung präzisiert** (Lauf-1-MEDIUM-2): ein per `sendEvent`
zugestelltes `QCloseEvent` prüft die Veto-Wirkung **nicht** — `ignore()` wirkt an `close()`. Geprüft
wird deshalb über `close()` und die **Sichtbarkeit** danach.

**Zeile 5 ist neu** (Lauf-1-MEDIUM-1): ohne sie verspräche 053 nur „die Aktion ruft *irgendeinen*
Handler", während [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md) darauf den **Nachweis
der Verwendung** bucht. Erst mit dem Port ist das prüfbar.

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

## 4. Definition of Done

- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: `QMainWindow`-Ableitung mit Menü-Aufbau und
      `closeEvent`-Behandlung; **kein Port-Include, kein `hexagon/services/`-Include** — alle Aktionen
      laufen über injizierte `std::function`s ([ADR-0019](../../adr/0019-drw-2d-canvas.md) Option A,
      Präzedenz `CanvasWidget`).
- [ ] **`src/adapters/ui/command/project_menu_handler.{h,cpp}`** (o. ä.): was eine Aktion **tut** —
      ruft den [`slice-054`](../done/slice-054-manage-project-port.md)-`ManageProjectPort`. Orakel §3-5.
- [ ] **`src/main.cpp`**: nur noch Instanz, Verdrahtung, Dialoge und Meldungstexte. Die
      Handler-**Rümpfe** ziehen um; die Dialog-Aufrufe bleiben.
- [ ] **Qt-Ownership und Lebensdauer entschieden, nicht nur benannt** (Lauf-1-MEDIUM-4): wem gehören
      `QTabWidget`, Viewer und Canvas nach dem Umzug; wo läuft das
      [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)-`unsubscribe` (heute `main.cpp`:519–520,
      **vor** der Widget-Zerstörung); und wie erreicht der `--acc-002-beleg`-Zweig
      (`main.cpp`:499/:502/:511) das Fenster. Die Antworten stehen im Plan-Vollzug, nicht im Diff.
- [ ] **`tests/adapters/test_main_window.cpp`** + **`tests/adapters/test_project_menu_handler.cpp`**:
      §3-Zeilen 1–5, **je mit roter Gegenprobe** im Closure-Text.
- [ ] **Build-Listen nachgezogen** (Lauf-1-MEDIUM-5): `src/adapters/CMakeLists.txt` und
      `tests/CMakeLists.txt` zählen Dateien **explizit** auf — genau die Linkage-Tatsache, aus der
      dieser Slice entstand.
- [ ] **`make a-check` grün ohne neue Kante**: `ui_view` importiert nur `model`, `ui_command` nur
      `model`/`ports_driving`. Entsteht eine Kante, ist der Schnitt falsch — **nicht** die Regel
      (AGENTS [§2.6](../../../../AGENTS.md)).
- [ ] **`spec/architecture.md`**: die GUI-Adapter-Zeile (:109) nennt für `command/` heute nur die
      „`MeshSource`-Naht" — sie ist um die **Kommando-Handler** zu ergänzen, und der `view/`-Zweig
      des §2.1-Baums um das Fenster (Lauf-1-MEDIUM-6: der Baum zählt die Widgets **nicht** auf, die
      Behauptung der Vorfassung war falsch).
- [ ] **CHANGELOG** [Unreleased]-Eintrag (Struktur, verhaltens-invariant).
- [ ] **Kein** Lastenheft-/Spezifikations-/Handbuch-Eintrag — nichts wird benutzer-sichtbar. (Geprüft
      und **verneint**, nicht vergessen — Lehre aus slice-047 V1.)
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv` — 053 wäre der erste
      `in-progress`-Slice, [MR-017](../../../../harness/conventions.md), Lauf-1-LOW-2);
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
| `src/main.cpp` | ändern | Instanz + Verdrahtung + Dialoge; Handler-Rümpfe raus |
| `tests/adapters/test_main_window.cpp` | neu | §3-Zeilen 1–4 |
| `tests/adapters/test_project_menu_handler.cpp` | neu | §3-Zeile 5 |
| `spec/architecture.md` | ändern | GUI-Adapter-Zeile + §2.1-Baum |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report zum zweiten Plan-Review | neu | Lauf-1-LOW-5 |

**Nicht berührt:** `.a-check.yml` (keine neue Kante), `spec/lastenheft.md`, `spec/spezifikation.md`,
`docs/user/`, `data-model.yaml`/`schema.sql`, `docs/plan/adr/`.

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

- §3-Zeilen 1–5 grün + je einmal diskriminierend belegt; die drei Regressions-Läufe grün;
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

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter

- **Modus:** GF; **Dichte:** mittel — eine Klasse, ein Umzug, vier Orakel-Zeilen; der Aufwand steckt in
  der **Invarianz** (R1/R3), nicht im Neubau.
- **Risiko:** niedrig-mittel — kein neues Verhalten, aber der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg-Pfad und die
  Qt-Ownership sind empfindlich.

## 11. Closure-Notiz

_(bei Ausführung auszufüllen)_
