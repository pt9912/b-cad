---
id: slice-011b
titel: Sichtbarer 3D-Viewer — Qt-6-Adapter (Implementierung, ACC-002)
status: open
welle: welle-1v-viewer (geplant — Welle noch nicht gestartet, Roadmap §Nächste Wellen)
lastenheft_refs: [LH-FA-D3-001, LH-FA-D3-002]  # sichtbare Hälfte D3-002; ACC-002
adr_refs: [ADR-0001, ADR-0002, ADR-0008]  # + ADR-0009, sobald in slice-011a entstanden
---

# Slice 011b: Sichtbarer 3D-Viewer — Qt-6-Adapter (Implementierung)

**Status:** open

**Welle:** welle-1v-viewer (geplant).

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
      (Dateischnitt folgt der ADR — Pflichten a/b/e): registriert
      sich via `subscribe` am Service als
      `ModelChangedPort`-Implementierung (Import-Grenze gemäß
      ADR-0009 Pflicht (d)), pullt den Stand, stellt das extrudierte
      Modell dar; keine Mutations-Rückrufe im Callback
      (Re-Entranz-Verbot ADR-0008).
- [ ] **Composition Root** (`src/main.cpp`) verdrahtet Kern +
      Viewer-Adapter gemäß ADR-0009 Pflicht (e).
- [ ] **AK-Tests mit `LH-`-ID im Namen**, display-frei lauffähig —
      Strategie und beobachtbares „dargestellt"-Surrogat gemäß
      **ADR-0009 Pflicht (f)** (hier nicht vorentschieden; Review
      011, H1/P11): Happy (committete Parameteränderung →
      dargestellter Stand folgt ohne Benutzer-Schritt), Boundary
      (geklemmte Änderung → dargestellter Stand = Grenzwert;
      **Mehrfach-Meldung je Mutation → idempotente Darstellung,
      kein Flackern** — fester AK gemäß Welle-1-Lerneintrag
      „struktureller Normalfall in die Boundary-AK", Review 011,
      P15), Negative (verworfene/abgelehnte Mutation → Darstellung
      unverändert).
- [ ] **ACC-002-Beleg** benutzer-beobachtbar dokumentiert
      (3D-Darstellung eines ACC-001-Kern-Projekts) — Erzeugungsweg
      und Ablage-Ort gemäß ADR-0009 Pflicht (f) (z. B.
      offscreen-Grab über ein make-Target), Beleg-Artefakt als
      geplante Datei in §3, **explizit als manueller
      Abnahme-Schritt deklariert, kein Gate** (Review 011, H3) —
      der Abnahme-Punkt ist *sichtbar*, nicht nur getestet.
- [ ] **arch-check-Regel E implementiert und grün** (Folgepflicht
      aus ADR-0009 Pflicht (c), Muster Regel C/slice-003b):
      Qt-Includes nur unter `src/adapters/ui/` +
      Composition-Root-Ausnahme `src/main.cpp`. Die bestehende
      Regel A prüft nur den Kern und deckt diese Aussage nicht —
      ohne neue Regel wäre die DoD-Zeile unprüfbar (Review 011,
      H2/Q2). Falls ADR-0009 Pflicht (b) der Darstellungs-Seite
      OCC-Sicht gewährt, wird zusätzlich Regel C **im Slice**
      nachgezogen.
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag;
      **Welle-Closure `welle-1v-viewer`** (Review 011, P8):
      Ergebnisnotiz `done/welle-1v-results.md` analog
      [`welle-1-results.md`](../done/welle-1-results.md) **inkl.
      zwingendem Carveout-Audit**, Roadmap-Umbuchung nach
      §Abgeschlossene Wellen, CHANGELOG-Eintrag (MR-004).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/ui/{**}` | neu | Viewer-Adapter (Dateischnitt gemäß ADR-0009) |
| `src/main.cpp` | ändern | Composition Root: Viewer verdrahten |
| `src/adapters/CMakeLists.txt`, `src/CMakeLists.txt` | ändern | Qt-6-Targets gemäß ADR-0009 |
| `tests/adapters/{**}` | neu | display-freie AK-Tests |
| `tests/CMakeLists.txt` | ändern | neue Tests registrieren (Review 011, P12) |
| `tests/e2e/{**}` | ggf. neu | e2e-Treiber, falls ADR-0009 Pflicht (f) ihn wählt — sonst Begründung in der Closure-Notiz (Review 011, P14) |
| `tools/arch-check.sh` | ändern | neue Regel E (Qt-Grenze, ADR-0009 Pflicht (c)); zusätzlich Regel C, falls Pflicht (b) die OCC-Sicht-Grenze ändert |
| `.devcontainer/` | ändern | Qt-/OCC-Visualisierungs-Pakete (`libocct-visualization-dev` fehlt bisher) + offscreen-Voraussetzungen |
| `Makefile` + `harness/toolchain-versions.txt` | ändern | `versions`-Paketliste ist hardcodet — neue Pakete brauchen den ADR-0004-Beleg via `make versions` (Review 011, M4/P7) |
| `docs/plan/planning/done/{acc-002-beleg}.{png,md}` | neu | ACC-002-Beleg-Artefakt (manueller Abnahme-Schritt, DoD-4) |
| `docs/plan/planning/done/{welle-1v-results}.md` | neu | Welle-Ergebnisnotiz inkl. Carveout-Audit (DoD-6) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Welle-Umbuchung bei Closure (DoD-6) |
| `CHANGELOG.md` | ändern | Welle-Eintrag unter `[Unreleased]` (MR-004; Review 011, P13) |

## 4. Trigger

- slice-011a done (ADR-0009 `Accepted`).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, ACC-002-Beleg liegt vor,
  Closure-Notiz geschrieben; Welle-Closure `welle-1v-viewer` wird
  damit erreichbar (Ergebnisnotiz analog
  [`welle-1-results.md`](../done/welle-1-results.md)).

## 6. Risiken und offene Punkte

- **Build-Schwere & Split-Pfad:** Qt-6-GUI + OCC-Visualisierung im
  Container ist die build-schwerste Kombination des Repos. Kippt die
  Sitzung, ist der vorbereitete Schnitt (Review 011, M6/P9):
  **(i)** statische Darstellung des extrudierten Stands (Fenster +
  Rendering + Toolchain) → **(ii)** Live-Update via Observer +
  ACC-002-Beleg + Welle-Closure (Präzedenz 003/009). Die
  DevContainer-Anpassung ggf. als eigenen Spike vorziehen.
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
  (Welle-1-Lerneintrag, Review 011, P15).
- **Coverage-Risiko:** `coverage-gate` misst `src/` (nur `main.cpp`
  ausgenommen); headless schwer abdeckbarer Qt-/Rendering-Code
  drückt die Linie (aktuell 93,8 %, Schwelle 70 % — Puffer
  vorhanden, aber unbewertet). Eine Schwellen-/Filter-Anpassung
  wäre ADR-pflichtig (AGENTS §2.6) — der legale Ausweg gehört in
  ADR-0009 Pflicht (f) mitgedacht, nicht spät im Slice improvisiert
  (Review 011, P16).

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

### Sub-Area: Build/Toolchain (`.devcontainer/`, CMake)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — ADR-0004 (Pinning, Digest+Snapshot),
  `make versions`-Beleg bei Toolchain-Änderung.
- **Phase-Reife:** Phase 4 (gepinnt seit slice-004).
- **Evidenz-/Diskrepanz-Risiko:** mittel — neue Qt-Runtime-Pakete.
- **Reconciliation-Aufwand:** keiner (GF).
