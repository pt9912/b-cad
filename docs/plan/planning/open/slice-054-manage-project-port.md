---
id: slice-054
titel: ManageProjectPort realisieren — die deklarierte Ziel-Form des Projekt-Use-Case
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0012](../../adr/0012-evaluations-architektur.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 054: `ManageProjectPort` realisieren (Struktur-Vorläufer)

**Status:** open — **Struktur-Vorläufer**, verhaltens-invariant. Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start. **Sequenz: 054 → [`slice-053`](slice-053-fenster-als-adapter.md) →
[`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md) →
[`slice-052b`](slice-052b-neues-projekt.md).**

**Welle:** welle-5-erweiterung (Quergewerk / Struktur-Vorbereitung).
**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser — die Wurzel einer vierfach wiederholten Klasse

Seit slice-047 landet in jedem GUI-Slice mindestens ein Review-Finding derselben Bauart: eine
**Entscheidung** liegt im coverage-ausgenommenen `src/main.cpp` und ist damit orakel-los.

| Wann | Finding |
|---|---|
| slice-047 | Verify **B4** — Neu-Auflösung des Zeichen-Ziels als Lambda in `main.cpp`, kein Sensor |
| slice-052, [Lauf 1](../../../reviews/2026-07-26-slice-052-plan.md) | MEDIUM-2/-3 — Ziel-Wahl und Antwort-Auswertung im Menü-Handler |
| slice-052a, [Lauf 3](../../../reviews/2026-07-26-slice-052a-plan-3.md) | **HIGH-1** — zugesagtes Orakel für das Schließ-Ereignis nicht herstellbar |
| slice-053, [Lauf 1](../../../reviews/2026-07-26-slice-053-plan.md) | **HIGH-1** — „Handler ziehen mit um" **und** „keine neue Kante" sind nicht gleichzeitig einlösbar |

**Die Ursache ist strukturell, nicht disziplinarisch.** `.a-check.yml` erlaubt `ui_command` nur
`model`/`ui_view`/`ports_driving` und `ui_view` nur `model`/`ports_driven` — **kein** Adapter darf
`hexagon/services/` rufen. Die Projekt-Use-Cases `services::openProject`/`saveProject` sind aber genau
das: **Services ohne Port**. Damit ist `main.cpp` der **einzige** Ort, der sie aufrufen darf
(`composition_root`), und jede Entscheidung, die an ihnen hängt, landet dort — im orakel-losen Bereich.

**`spec/architecture.md`:76 benennt die Auflösung seit dem Bootstrap selbst:**

> `ManageProjectPort` (**Ziel-Form, noch nicht realisiert**) … ein Treiber-Adapter spricht sie
> **direkt** an statt über eine Port-Abstraktion. **Die Port-Form bleibt das Ziel, sobald ein zweiter
> Treiber sie braucht.**

**Diese Bedingung ist eingetreten:** mit [`slice-053`](slice-053-fenster-als-adapter.md) bekommt das
GUI einen zweiten, **testbaren** Treiber neben dem Composition-Root. Der Slice realisiert also keine
neue Idee, sondern **eine seit dem Bootstrap deklarierte Ziel-Form zum dafür benannten Zeitpunkt**.

## 1. Ziel

Die Projekt-Use-Cases stehen hinter einem **Driving Port** `ManageProjectPort`
(`src/hexagon/ports/driving/`), implementiert von einem Kern-Service. Danach kann ein
**Adapter** (`ui/command/`, Kante existiert) sie aufrufen — statt nur der Composition-Root.

**Verhaltens-invariant:** dieselben Wirkungen, dieselben Fehler, dieselben Meldungen. Kein neues
Verhalten, keine neue Anforderung.

## 2. Bewusst NICHT Teil

- **Jede neue Funktion.** Kein „Speichern", kein „Neu", keine Rückfrage — das sind 052a/052b.
- **Projekt anlegen/versionieren** als *Verhalten*: der Port **darf** die Operationen deklarieren, die
  [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)/[`004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung)
  später brauchen — geliefert wird in diesem Slice **nur**, was heute existiert (Öffnen, Speichern).
  Ein Port mit unimplementierten Methoden wäre eine Lüge im Vertrag.
- **Die CLI-Seite umbauen.** `runHeadlessCli` darf den Port nutzen, muss aber nicht; der
  CLI-`--open`-Pfad lädt bewusst **ohne** Sitzung (spez. §1 zu [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)) und bleibt
  semantisch, wie er ist.
- **Kein neuer ADR.** Die Ziel-Form ist in `spec/architecture.md`:76 deklariert; sie zu realisieren ist
  **Erfüllung**, keine Abweichung. Entsteht beim Schnitt doch eine Grundsatzfrage (z. B. Port-Zuschnitt
  über BLD-001..004), ist sie im [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  zu benennen — **nicht** still zu entscheiden.

## 3. Orakel-Schnitt

| # | Zusicherung | Diskriminierende Gegenprobe |
|---|---|---|
| 1 | **Ein Adapter kann die Use-Cases aufrufen**: ein Test-Doppel in Adapter-Position ruft den Port und bewirkt dasselbe wie heute der direkte Service-Aufruf | Aufruf über den Port entfernt ⇒ rot |
| 2 | **Öffnen über den Port** ersetzt den Sitzungs-Stand und löst das Zeichen-Ziel neu auf (die slice-047-Zusagen gelten unverändert weiter) | Zeichen-Ziel-Auflösung entfernt ⇒ rot (heutiges Orakel bleibt gültig) |
| 3 | **Speichern über den Port** schreibt atomar und wirft **fail-closed** bei danglendem `from_storey` | Wurf entfernt ⇒ rot (heutiges Orakel bleibt gültig) |
| 4 | **Fehler kommen neutral durch** (keine Framework-Typen im Port-Vertrag) | Fehler geschluckt ⇒ rot |
| 5 | **`make a-check` grün ohne neue Kante**: `services → ports_driving` und `ui_command → ports_driving` existieren beide bereits | eine `ui_*` → `services`-Kante wäre der Beweis, dass der Schnitt falsch ist |

**Die bestehenden Orakel bleiben der Maßstab:** `test_manage_project.cpp` und
`test_project_open_handler.cpp` prüfen heute Verhalten, das sich **nicht ändern darf**. Sie ziehen auf
den Port um; ihre Zusicherungen bleiben Wort für Wort dieselben — das **ist** der Invarianz-Beleg
(anders als bei einem reinen `main.cpp`-Umzug, für den es keinen Vorher-Sensor gäbe).

## 4. Definition of Done

- [ ] **`src/hexagon/ports/driving/manage_project_port.{h}`**: Driving Port mit den **heute
      existierenden** Operationen (Öffnen, Speichern), framework-frei, neutrale Fehler.
- [ ] **Kern-Implementierung**: ein Service in `src/hexagon/services/` erfüllt den Port; die heutigen
      freien Funktionen bleiben erhalten **oder** wandern hinein — beides ist zulässig, solange die
      bestehenden Tests **inhaltlich unverändert** grün bleiben (Invarianz-Beleg, §3).
- [ ] **Aufrufer umgestellt**: `src/main.cpp` ruft die Use-Cases über den Port. Der Composition-Root
      **darf** weiterhin direkt rufen — er tut es nicht mehr, damit der Port real erprobt ist.
- [ ] **Orakel §3-1..5** je mit roter Gegenprobe im Closure-Text.
- [ ] **`spec/architecture.md` §1.1**: die `ManageProjectPort`-Zeile von „**Ziel-Form, noch nicht
      realisiert**" auf **realisiert** ziehen, samt der `Ist-Zustand`-Prosa; §2.1-Baum-Klammer
      „(ManageProjectPort: Ziel-Form, s. §1.1)" nachziehen. **Das ist die eigentliche Doku-Zusage
      dieses Slice** — die Datei behauptet den Zielzustand seit dem Bootstrap.
- [ ] **`spec/spezifikation.md`**: der §1-Block [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.a`/[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)`.a` beschreibt die Aufruf-Wege;
      prüfen, ob die Port-Naht dort eine Aussage berührt — **nachziehen oder begründet unberührt
      lassen** (die Entscheidung steht im Closure-Text, nicht implizit).
- [ ] **CHANGELOG** [Unreleased]-Eintrag (Struktur, verhaltens-invariant).
- [ ] **Kein** Lastenheft-Eintrag, **kein** Handbuch-Eintrag — nichts wird benutzer-sichtbar. (Zeile
      bewusst gesetzt: geprüft und **verneint**, nicht vergessen — Lehre aus slice-047 V1.)
- [ ] **`make gates` grün**, **`make io-smoke` grün** (der CLI-Pfad muss unverändert funktionieren),
      `make schema-check` byte-unberührt.

## 5. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/manage_project_port.{h}` | neu | der deklarierte Driving Port |
| `src/hexagon/services/manage_project.{h,cpp}` | ändern | erfüllt den Port (Service-Form) |
| `src/hexagon/CMakeLists.txt` | ändern | zählt Dateien explizit auf |
| `src/main.cpp` | ändern | ruft über den Port statt direkt |
| `tests/hexagon/test_manage_project.cpp` | ändern | zieht auf den Port um, **Zusicherungen unverändert** |
| `tests/adapters/test_project_open_handler.cpp` | ändern | dito |
| `tests/hexagon/test_manage_project_port.cpp` | neu | §3-Zeile 1 (Adapter-Position ruft den Port) |
| `spec/architecture.md` | ändern | §1.1 „Ziel-Form" → realisiert; §2.1-Klammer |
| `spec/spezifikation.md`, `spec/spezifikation-historie.md` | ändern o. begründet unberührt | §1-Aufruf-Wege |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | eigenes [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `.a-check.yml` (**keine** neue Kante — `services → ports_driving` und
`ui_command → ports_driving` existieren), `data-model.yaml`/`schema.sql`, `docs/plan/adr/`,
`spec/lastenheft.md`, `docs/user/`.

## 6. Risiken

- **R1 — Port-Zuschnitt.** `ManageProjectPort` ist in `architecture.md` mit **vier** Aufgaben
  beschrieben (anlegen, speichern, laden, versionieren). Nur zwei existieren. Der Port darf **nicht**
  vorgeben, was es nicht gibt (§2) — die Abweichung zur Architektur-Zeile ist beim Nachziehen
  **explizit** zu formulieren, sonst entsteht dieselbe Doku-Unwahrheit, die slice-047 als F4 fand.
- **R2 — die `openProject`-Signatur trägt bereits eine Naht.** Sie nimmt seit slice-047 die
  `DrawingTargetSinks` (port-freie `std::function`). Ob die in den Port-Vertrag gehören oder außen
  bleiben, ist eine echte Schnitt-Frage — sie entscheidet mit, wie viel [`slice-053`](slice-053-fenster-als-adapter.md)
  danach noch verdrahten muss.
- **R3 — Invarianz-Beleg hängt an den bestehenden Tests.** Ihr Umzug auf den Port darf ihre
  **Zusicherungen** nicht verändern; wer sie „bei der Gelegenheit" umformuliert, verliert genau den
  Beleg, der diesen Slice rechtfertigt.

## 7. Trigger

- **[MR-006 zu slice-053](../../../reviews/2026-07-26-slice-053-plan.md) HIGH-1** (Kanten-Widerspruch)
  + die dreifache Vorgeschichte (Tabelle oben). Projektinhaber-Entscheidung 2026-07-26: **die
  deklarierte Ziel-Form realisieren**, statt die Klasse ein fünftes Mal einzeln zu behandeln.

## 8. Closure-Trigger

- §3-Zeilen grün + je diskriminierend belegt; bestehende Tests inhaltlich unverändert grün;
  `make gates` + `make io-smoke` grün; `architecture.md` §1.1 sagt die Wahrheit; Closure-Notiz.

## 9. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF; **Dichte:** mittel — ein Port, eine Implementierung, ein Aufrufer-Umbau; der Aufwand
  steckt im **Zuschnitt** (R1/R2), nicht im Umfang.
- **Phase-Reife:** die Use-Cases sind seit slice-047 real und getestet; dieser Slice gibt ihnen die
  Form, die die Architektur seit dem Bootstrap vorsieht.
- **Risiko:** niedrig-mittel — verhaltens-invariant mit vorhandenem Vorher-Sensor (anders als bei
  einem reinen `main.cpp`-Umzug).

## 10. Closure-Notiz

_(bei Ausführung auszufüllen)_
