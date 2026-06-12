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

Ein sichtbares 3D-Fenster: Der Qt-6-Driving-Adapter abonniert den
ADR-0008-Observer-Port, pullt nach Meldung den aktualisierten Stand
und stellt das extrudierte Gebäudemodell dar — ohne expliziten
Aktualisierungs-Schritt des Benutzers. Damit werden **ACC-002** („Das
Gebäude wird automatisch als 3D-Modell dargestellt") und die
sichtbare Hälfte von **LH-FA-D3-002** erfüllt; der Kern bleibt
framework-frei (ADR-0001).

## 2. Definition of Done

- [ ] **Viewer-Adapter gemäß ADR-0009** unter `src/adapters/ui/`
      (Dateischnitt folgt der ADR — Pflichten a/b/e): subscribe am
      Observer-Port, Pull des Stands, Darstellung des extrudierten
      Modells; keine Mutations-Rückrufe im Callback
      (Re-Entranz-Verbot ADR-0008).
- [ ] **Composition Root** (`src/main.cpp`) verdrahtet Kern +
      Viewer-Adapter gemäß ADR-0009 Pflicht (e).
- [ ] **AK-Tests mit `LH-`-ID im Namen**, display-frei lauffähig
      (DevContainer ist headless — Strategie gemäß ADR-0009, z. B.
      `QT_QPA_PLATFORM=offscreen` oder Adapter-Logik gegen
      Port-Doubles): Happy (committete Parameteränderung →
      dargestellter Stand folgt ohne Benutzer-Schritt), Boundary
      (geklemmte Änderung → dargestellter Stand = Grenzwert),
      Negative (verworfene/abgelehnte Mutation → Darstellung
      unverändert).
- [ ] **ACC-002-Beleg** benutzer-beobachtbar dokumentiert
      (Screenshot/Aufzeichnung des 3D-Fensters mit einem
      ACC-001-Kern-Projekt) — der Abnahme-Punkt ist *sichtbar*,
      nicht nur getestet.
- [ ] **arch-check bleibt grün** — Qt-Includes ausschließlich unter
      `src/adapters/ui/`; falls ADR-0009 Pflicht (b) der
      Darstellungs-Seite OCC-Sicht gewährt, ist die zugehörige
      arch-check-Regel **im Slice** nachzuziehen (Folgepflicht-Muster
      slice-003b).
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag;
      Roadmap-Closure-Zeile (`welle-1v-viewer`) nachgezogen.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/ui/{**}` | neu | Viewer-Adapter (Dateischnitt gemäß ADR-0009) |
| `src/main.cpp` | ändern | Composition Root: Viewer verdrahten |
| `src/adapters/CMakeLists.txt`, `src/CMakeLists.txt` | ändern | Qt-6-Targets gemäß ADR-0009 |
| `tests/adapters/{**}` | neu | display-freie AK-Tests |
| `tools/arch-check.sh` | ggf. ändern | Regel-Nachzug, falls ADR-0009 (b) die OCC-Sicht-Grenze ändert |
| `.devcontainer/` | ggf. ändern | Qt-Runtime/offscreen-Voraussetzungen |

## 4. Trigger

- slice-011a done (ADR-0009 `Accepted`).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, ACC-002-Beleg liegt vor,
  Closure-Notiz geschrieben; Welle-Closure `welle-1v-viewer` wird
  damit erreichbar (Ergebnisnotiz analog
  [`welle-1-results.md`](../done/welle-1-results.md)).

## 6. Risiken und offene Punkte

- **Build-Schwere:** Qt-6-GUI + OCC-Visualisierung im Container ist
  die build-schwerste Kombination des Repos (Präzedenz slice-003b:
  OCC-Teil isolieren hat getragen). Ggf. eigener Spike, falls die
  DevContainer-Anpassung (`.devcontainer/`) mehr als eine Sitzung
  kostet.
- **Headless-Testbarkeit ist die heikelste DoD-Zeile:** Ein
  GUI-Adapter, dessen AK nur manuell prüfbar sind, würde das
  Gate-Prinzip (Sensors statt Behauptung) für diesen Strang
  schwächen — die Test-Strategie ist deshalb ADR-0009-Pflicht und
  nicht Implementierungs-Detail.
- **Echtzeit-Wahrnehmung:** Kein Latenz-Budget vergeben (bewusst,
  slice-010a/011a); wird Trägheit beim ACC-002-Beleg sichtbar, ist
  das eine neue `LH-QA-<NNN>`-Anforderung, kein stiller Fix.
- **Mehr-Element-Updates:** `RoomsChanged` meldet je Wand-Mutation
  unconditional (slice-010b Restrisiko) — der Viewer muss
  Mehrfach-Meldungen idempotent darstellen (kein Flackern als
  AK-Boundary erwägen).

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
