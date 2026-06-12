---
id: slice-011b
titel: Sichtbarer 3D-Viewer — Qt-6-Adapter (Implementierung, ACC-002)
status: open
welle: welle-1v-viewer
lastenheft_refs: [LH-FA-D3-001, LH-FA-D3-002]  # sichtbare Hälfte D3-002; ACC-002
adr_refs: [ADR-0001, ADR-0002, ADR-0008]  # + ADR-0009, sobald in slice-011a entstanden
---

# Slice 011b: Sichtbarer 3D-Viewer — Qt-6-Adapter (Implementierung)

**Status:** open

**Welle:** welle-1v-viewer (gestartet 2026-06-12; dieser Slice
bleibt in `open/`, bis sein Trigger — slice-011a done — erfüllt
ist).

**Bezug:** ACC-002, sichtbare Hälfte LH-FA-D3-002 (AK seit
slice-010a), LH-FA-D3-001 (extrudierter Stand), ADR-0008
(Observer-Port/Pull-State — der Vertrag, den der Viewer konsumiert),
ADR-0001/0002 (Adapter-Grenzen). **Parametrisiert auf ADR-0009**
(slice-011a): UI-Technologie, Rendering-Anbindung, Threading und
Composition-Root-Form legt die ADR fest, nicht dieser Plan
(Muster slice-010b / Plan-Review 010, F3).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-12.

**Schnitt-Herkunft:** Implementierungs-Hälfte des Viewer-Strangs
(Split-Begründung in slice-011a §Schnitt-Herkunft).

**Plan-Reviews (MR-006):**
[Runde 1](../../../reviews/2026-06-12-slice-011-plan.md) (R1-IDs) ·
[Runde 2](../../../reviews/2026-06-12-slice-011b-plan-w2.md)
(W2-IDs) — alle Findings eingearbeitet.

---

## 1. Ziel

Ein sichtbares 3D-Fenster: Der Qt-6-Driving-Adapter registriert sich
via `subscribe` am `StructureEditService` als Implementierung des
`ModelChangedPort` (ADR-0008), pullt nach Meldung den aktualisierten
Stand
und stellt das extrudierte Gebäudemodell dar — ohne expliziten
Aktualisierungs-Schritt des Benutzers. Damit werden **ACC-002** („Das
Gebäude wird automatisch als 3D-Modell dargestellt") und die
sichtbare Hälfte von **LH-FA-D3-002** erfüllt; der Kern bleibt
framework-frei (ADR-0001).

## 2. Definition of Done

- [ ] **Viewer-Adapter gemäß ADR-0009** unter `src/adapters/ui/`
      (Dateischnitt folgt der ADR — Pflichten a/b/e), in zwei
      prüfbaren Hälften (= Split-Pfad §6):
      **(i) Statische Darstellung:** ein geladenes/aufgebautes
      Projekt wird als extrudiertes 3D-Modell dargestellt
      (LH-FA-D3-001-Basis);
      **(ii) Live-Folgen:** der Adapter registriert sich via
      `subscribe` am Service als `ModelChangedPort`-Implementierung
      (Import-Grenze gemäß ADR-0009 Pflicht (d)), pullt nach Meldung
      den Stand und zieht die Darstellung nach (LH-FA-D3-002);
      keine Mutations-Rückrufe im Callback (Re-Entranz-Verbot
      ADR-0008).
- [ ] **Composition Root** (`src/main.cpp`) verdrahtet Kern +
      Viewer-Adapter gemäß ADR-0009 Pflicht (e).
- [ ] **AK-Tests mit `LH-`-ID im Namen**, display-frei lauffähig —
      Strategie und beobachtbares „dargestellt"-Surrogat gemäß
      **ADR-0009 Pflicht (f)** (hier nicht vorentschieden; R1-H1):
      **Split-Hälfte (i):** statische Darstellung — Projekt mit
      Wänden geladen/aufgebaut → Surrogat zeigt den extrudierten
      Stand (LH-FA-D3-001). **Split-Hälfte (ii):** Happy (committete
      Parameteränderung → dargestellter Stand folgt ohne
      Benutzer-Schritt), Boundary (geklemmte Änderung →
      dargestellter Stand = Grenzwert; **Mehrfach-Meldung je
      Mutation → idempotenter Surrogat-Endzustand, genau ein
      wirksames Darstellungs-Update je Meldung** — fester AK gemäß
      Welle-1-Lerneintrag „struktureller Normalfall in die
      Boundary-AK"; Operationalisierung von „kein Flackern" über
      das (f)-Surrogat, W2-P9), Negative (verworfene/abgelehnte
      Mutation → Darstellung unverändert).
- [ ] **ACC-002-Beleg** benutzer-beobachtbar dokumentiert
      (3D-Darstellung eines ACC-001-Kern-Projekts) — Erzeugungsweg,
      Form und Ablage-Ort gemäß ADR-0009 Pflicht (f), **explizit
      als manueller Abnahme-Schritt deklariert, kein Gate**
      (R1-H3). **Abnehmer: der Projektinhaber** (Autor-Rolle,
      W2-P4); der Beleg enthält minimal: verwendetes Projekt,
      sichtbare Soll-Merkmale (extrudierte Wände, erkannter Raum),
      Erzeugungs-Kommando, Commit-Hash, Datum und einen expliziten
      Abnahme-Satz. Der Abnahme-Punkt ist *sichtbar*, nicht nur
      getestet.
- [ ] **Ein realer arch-check-Sensor für die Qt-Grenze existiert
      und ist grün** (Folgepflicht aus ADR-0009 Pflicht (c), Muster
      Regel C/slice-003b) — Geltungsbereich und Ausnahme-Set
      **gemäß der ADR-Entscheidung**, nicht hier fixiert (W2-P2;
      z. B. eine Regel E „Qt-Includes nur `src/adapters/ui/` +
      Composition-Root-Ausnahme `src/main.cpp`, ggf.
      Plugin-Ausnahme"). Hintergrund: die bestehende Regel A prüft
      nur den Kern — ohne neuen Sensor wäre die Qt-Grenze
      unprüfbar (R1-H2). Falls ADR-0009 Pflicht (b) der
      Darstellungs-Seite OCC-Sicht gewährt, wird zusätzlich
      Regel C **im Slice** nachgezogen.
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag;
      CHANGELOG-**Slice**-Eintrag unter `[Unreleased]` (MR-004).
      **Nicht Teil dieser DoD** (W2-P1): die Welle-Closure
      `welle-1v-viewer` — Ergebnisnotiz inkl. zwingendem
      Carveout-Audit und Roadmap-Umbuchung — ist ein **separater
      Schritt nach** slice-011b done, mit unabhängiger
      Verifikation davor (Präzedenz welle-1, siehe §5).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/ui/{**}` | neu | Viewer-Adapter (Dateischnitt gemäß ADR-0009) |
| `src/main.cpp` | ändern | Composition Root: Viewer verdrahten |
| `src/adapters/CMakeLists.txt`, `src/CMakeLists.txt` | ändern | Qt-6-Targets gemäß ADR-0009 |
| `tests/adapters/{**}` | neu | display-freie AK-Tests |
| `tests/CMakeLists.txt` | ändern | neue Tests registrieren (R1-L5) |
| `tests/e2e/{**}` | ggf. neu | e2e-Treiber, falls ADR-0009 Pflicht (f) ihn wählt — sonst Begründung in der Closure-Notiz (R1-L7) |
| `tools/arch-check.sh` | ändern | Qt-Grenz-Sensor gemäß ADR-0009 Pflicht (c) (z. B. Regel E); zusätzlich Regel C, falls Pflicht (b) die OCC-Sicht-Grenze ändert |
| `harness/README.md` §Sensors, `AGENTS.md` §3 | ändern | dokumentierter arch-check-Vertrag wächst um den Qt-Grenz-Sensor (ggf. Regel-C-Revision) — Gate-Doku darf nicht driften (W2-P5) |
| `.devcontainer/` | ändern | Qt-/OCC-Visualisierungs-Pakete (`libocct-visualization-dev` fehlt bisher); Headless-Voraussetzungen **gemäß ADR-0009 Pflicht (f)** (W2-P3) |
| `Makefile` + `harness/toolchain-versions.txt` | ändern | `versions`-Paketliste ist hardcodet — neue Pakete brauchen den ADR-0004-Beleg via `make versions` (R1-M4); + ggf. Beleg-Target gemäß Pflicht (f) — nicht in `gates` aggregiert, kein Gate-Status (W2-P11) |
| `docs/plan/planning/done/{acc-002-beleg}.{*}` | ggf. neu | Platzhalter: ACC-002-Beleg-Artefakt — Form, Endung und Ablage-Ort gemäß ADR-0009 Pflicht (f) (W2-P3; manueller Abnahme-Schritt, DoD-4) |
| `spec/architecture.md`, `spec/spezifikation.md` | ggf. ändern | Spec-Drift-Korrektur, falls der erste echte UI-Adapter Diskrepanzen zu den 011a-Nachzügen aufdeckt — Begründung in der Closure-Notiz (W2-P10) |
| `CHANGELOG.md` | ändern | Slice-Eintrag unter `[Unreleased]` (MR-004; R1-L6) |

## 4. Trigger

- slice-011a done (ADR-0009 `Accepted`).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, ACC-002-Beleg liegt vor und
  ist abgenommen, Closure-Notiz geschrieben → die **Welle-Closure
  `welle-1v-viewer` wird erreichbar**. Sie ist ein **eigener
  Schritt nach diesem Slice** (dritter Roadmap-Trigger, W2-P1):
  unabhängige Verifikation analog welle-1 §3, dann Ergebnisnotiz
  `done/welle-1v-results.md` inkl. zwingendem Carveout-Audit und
  Roadmap-Umbuchung (Präzedenz
  [`welle-1-results.md`](../done/welle-1-results.md)).

## 6. Risiken und offene Punkte

- **Build-Schwere & Split-Pfad:** Qt-6-GUI + OCC-Visualisierung im
  Container ist die build-schwerste Kombination des Repos. Kippt die
  Sitzung, ist der vorbereitete Schnitt (R1-M6): **Hälfte (i)**
  statische Darstellung (Fenster + Rendering + Toolchain) →
  **Hälfte (ii)** Live-Folgen + ACC-002-Beleg — beide sind in
  DoD-1/DoD-3 als (i)/(ii) markiert und einzeln prüfbar (W2-P7;
  Präzedenz 003/009). Die DevContainer-Anpassung ggf. als eigenen
  Spike vorziehen.
- **Headless-Testbarkeit ist die heikelste DoD-Zeile:** entschieden
  wird sie in ADR-0009 Pflicht (f) (slice-011a) — dieser Slice
  referenziert nur. Ein GUI-Adapter, dessen AK nur manuell prüfbar
  sind, würde das Gate-Prinzip (Sensors statt Behauptung)
  schwächen.
- **Echtzeit-Wahrnehmung:** Kein Latenz-Budget vergeben (bewusst,
  slice-010a/011a); wird Trägheit beim ACC-002-Beleg sichtbar, ist
  das eine neue `LH-QA-<NNN>`-Anforderung, kein stiller Fix.
- **Mehrfach-Meldungen sind der strukturelle Normalfall:**
  `RoomsChanged` meldet je Wand-Mutation unconditional (slice-010b
  Restrisiko) — deshalb ist die idempotente Darstellung **fester
  Boundary-AK** (DoD-3), nicht nur eine Erwägung
  (Welle-1-Lerneintrag, R1-L8).
- **Coverage-Risiko:** `coverage-gate` misst `src/` (nur `main.cpp`
  ausgenommen); headless schwer abdeckbarer Qt-/Rendering-Code
  drückt die Linie (aktuell 93,8 %, Schwelle 70 % — Puffer
  vorhanden, aber unbewertet). Eine Schwellen-/Filter-Anpassung
  wäre ADR-pflichtig (AGENTS §2.6) — der legale Ausweg ist Teil von
  ADR-0009 Pflicht (f) (Coverage-Umgang, seit W2-P6 dort
  beauftragt), nicht spät im Slice improvisiert (R1-L9).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Adapter-Implementierung (`src/adapters/ui/`)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — ADR-0001-Schichtregel (arch-check),
  ADR-0008-Vertrag, ADR-0009 (entsteht in 011a).
- **Phase-Reife:** Verzeichnis existiert als Skelett; erster echter
  UI-Adapter.
- **Evidenz-/Diskrepanz-Risiko:** mittel — GUI-Verhalten ist
  schwerer zu verifizieren als Kern-Logik (headless-Strategie).
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — AK-Tests mit `LH-`-ID im Namen,
  display-freie Lauffähigkeit (Surrogat gemäß ADR-0009 Pflicht (f)),
  Registrierung in `tests/CMakeLists.txt` (W2-P8).
- **Phase-Reife:** `hexagon/`-/`adapters/`-Suiten etabliert;
  `e2e/` bisher leer (erster Treiber ggf. in diesem Slice).
- **Evidenz-/Diskrepanz-Risiko:** mittel — neues Surrogat-Orakel
  für „dargestellt".
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Build/Toolchain (`.devcontainer/`, CMake, `Makefile`, `tools/`)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — ADR-0004 (Pinning, Digest+Snapshot),
  `make versions`-Beleg bei Toolchain-Änderung, gate-consistency
  über dokumentierte make-Targets, arch-check-Regelbestand (W2-P8).
- **Phase-Reife:** Phase 4 (gepinnt seit slice-004).
- **Evidenz-/Diskrepanz-Risiko:** mittel — neue Qt-Runtime-Pakete +
  neuer Sensor.
- **Reconciliation-Aufwand:** keiner (GF).
