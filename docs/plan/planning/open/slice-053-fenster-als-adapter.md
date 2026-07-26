---
id: slice-053
titel: Hauptfenster als testbarer Adapter — Menü und Schließ-Behandlung raus aus dem Composition-Root
status: open
welle: welle-5-erweiterung
lastenheft_refs: []
adr_refs: [[ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 053: Hauptfenster als testbarer Adapter (Struktur-Vorläufer)

**Status:** open — **Struktur-Vorläufer**, verhaltens-invariant. Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start. **Sequenz: VOR [`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md).**

**Welle:** welle-5-erweiterung (Quergewerk / Struktur-Vorbereitung — Muster
[slice-028/029](../done-archive/), die vor der a-check-Umstellung dieselbe Rolle spielten).
**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser — dieselbe Klasse zum dritten Mal

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

## 1. Ziel

Das Hauptfenster wird eine **Adapter-Klasse** in `src/adapters/ui/view/` — wie `CanvasWidget` und
`ViewerWidget` — statt eines lokalen `QMainWindow` im Composition-Root. Danach sind Menü-Aktionen und
das Schließ-Ereignis **headless prüfbar**, und `main.cpp` enthält nur noch **Instanziierung und
Verdrahtung**.

**Verhaltens-invariant:** kein neues Menü, keine neue Aktion, keine geänderte Reaktion. Wer das Programm
bedient, merkt nichts.

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

Der Wert des Slice **ist** die Prüfbarkeit; die Tabelle ist damit sein eigentliches Ergebnis.

| # | Zusicherung | Diskriminierende Gegenprobe |
|---|---|---|
| 1 | Das Fenster ist als Adapter-Klasse **headless konstruierbar** (offscreen, ohne Display) | — (Voraussetzung der übrigen Zeilen) |
| 2 | Ein zugestelltes **`QCloseEvent`** erreicht die Behandlung der Klasse (Beobachtung: ein injizierter Rückfrage-Haken wird gerufen) | Behandlung entfernt ⇒ rot |
| 3 | Die Behandlung kann das Schließen **verhindern** (`ignore()`) — die Naht, an der 052a sein „abbrechen" aufhängt | Ablehnung ignoriert ⇒ rot |
| 4 | Eine **Menü-Aktion** ist programmatisch auslösbar (`trigger()`) und ruft ihren Handler | Verdrahtung entfernt ⇒ rot |
| 5 | **Verhaltens-Invarianz:** die bestehenden Adapter-Tests + `make io-smoke` + der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg-Pfad bleiben grün | — (Regressions-Netz) |

**Benannte Grenze:** die **modalen Dialoge** (`QFileDialog`, `QMessageBox`) bleiben ohne Sensor — sie
blockieren im Test. Sie werden über eine injizierbare Naht aufgerufen, damit der Test an ihrer Stelle
antworten kann; **der Dialog selbst** bleibt ungeprüft. Das ist die Grenze, die slice-047 (B5) korrekt
benannt hatte — und die einzige, die bleibt.

## 4. Definition of Done

- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`** (o. ä.): `QMainWindow`-Ableitung mit Menü-Aufbau
      und `closeEvent`-Behandlung; die Entscheidungen kommen über **injizierte Callables** von außen
      (Muster [ADR-0019](../../adr/0019-drw-2d-canvas.md) Option A / `DrawingTargetSinks` aus
      slice-047) — **keine** Kern-Logik im Widget, **kein** neuer Port.
- [ ] **`src/main.cpp`** enthält für das Fenster nur noch **Instanziierung + Verdrahtung** (Dialoge,
      Meldungstexte); die bestehenden Handler ziehen mit um, **inhaltlich unverändert**.
- [ ] **`tests/adapters/test_main_window.cpp`**: §3-Zeilen 1–4, headless (Muster
      `test_canvas_widget.cpp`), **je mit roter Gegenprobe** im Closure-Text.
- [ ] **Verhaltens-Invarianz belegt:** `make gates` grün, `make io-smoke` grün, `make acc-002-beleg`
      erzeugt ein Bild (der Pfad bleibt intakt — [ADR-0010](../../adr/0010-headless-gl-xvfb.md)).
- [ ] **`make a-check` grün ohne neue Kante:** die Klasse lebt in `ui_view` und darf nur, was
      `.a-check.yml` dort erlaubt. Ergibt sich eine neue Kante, ist der Schnitt falsch — **nicht** die
      Regel (AGENTS [§2.6](../../../../AGENTS.md)).
- [ ] **`spec/architecture.md`** §2.1-Baum: das Fenster im `adapters/ui/view/`-Zweig nennen (der Baum
      zählt die Widgets heute auf).
- [ ] **CHANGELOG** [Unreleased]-Eintrag (Struktur, verhaltens-invariant).
- [ ] **Kein** Lastenheft-/Spezifikations-Eintrag, **kein** Handbuch-Eintrag — nichts wird
      benutzer-sichtbar. (Diese Zeile ist bewusst gesetzt: die Doku-Pflicht wird **geprüft und
      verneint**, nicht vergessen — Lehre aus slice-047 V1.)

## 5. Risiken

- **R1 — der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg hängt am Fenster-Aufbau.** `main.cpp` zeigt heute das Fenster und grabbt den
  Viewer-Tab; die Reihenfolge (`setCurrentWidget(viewer)` **vor** `show()`) ist in slice-043 teuer
  erarbeitet worden, weil sonst der GL-Kontext nicht initialisiert. Der Umzug darf sie nicht brechen —
  §3-Zeile 5.
- **R2 — verhaltens-invariant heißt: keine Gelegenheit nutzen.** Beim Umzug fällt auf, was man
  „gleich mitmachen" könnte. Genau das ist verboten; jede Verhaltensänderung gehört in 052a/052b und
  macht den Diff unprüfbar.
- **R3 — Qt-Ownership.** `QMainWindow` übernimmt Kindschaften; beim Umzug darf keine doppelte
  Freigabe oder ein hängender Zeiger entstehen (die Widgets gehören heute dem `QTabWidget`).

## 6. Trigger

- **[MR-006 Lauf 3](../../../reviews/2026-07-26-slice-052a-plan-3.md) HIGH-1** zu slice-052a
  (Orakel-Zeile nicht herstellbar) + **INFO-2** desselben Laufs (Pflege-Signal: dritte Wiederholung der
  Klasse). Projektinhaber-Entscheidung 2026-07-26: **eigener Vorläufer** statt weiterer Scope-Zuwachs
  in 052a.

## 7. Closure-Trigger

- §3-Zeilen 1–4 grün + je einmal diskriminierend belegt; §3-Zeile 5 (Invarianz) belegt;
  `make gates` + `make io-smoke` + `make acc-002-beleg` grün; Closure-Notiz.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter

- **Modus:** GF; **Dichte:** mittel — eine Klasse, ein Umzug, vier Orakel-Zeilen; der Aufwand steckt in
  der **Invarianz** (R1/R3), nicht im Neubau.
- **Risiko:** niedrig-mittel — kein neues Verhalten, aber der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg-Pfad und die
  Qt-Ownership sind empfindlich.

## 9. Closure-Notiz

_(bei Ausführung auszufüllen)_
