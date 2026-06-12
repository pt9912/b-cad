---
id: slice-011a
titel: Sichtbarer 3D-Viewer — GUI-Grundsatz-ADR Qt 6 & Spec-Operationalisierung ACC-002
status: open
welle: welle-1v-viewer (geplant — Welle noch nicht gestartet, Roadmap §Nächste Wellen)
lastenheft_refs: [LH-FA-D3-002]  # sichtbare Hälfte; ACC-002 hängt an LH-FA-D3-001/002
adr_refs: [ADR-0001, ADR-0008]  # ADR-0009 (GUI-Grundsatz) entsteht in diesem Slice
---

# Slice 011a: Sichtbarer 3D-Viewer — GUI-Grundsatz-ADR & Spec

**Status:** open

**Welle:** welle-1v-viewer (geplant; Start ist eine
Roadmap-Entscheidung — Trigger „welle-1 done" ist seit 2026-06-12
erfüllt, siehe [`welle-1-results.md`](../done/welle-1-results.md)).

**Bezug:** REQ-TEC-002 (Qt 6 gesetzt), ADR-0001 (Adapter-Grenze),
ADR-0008 (Benachrichtigungs-Vertrag, den der Viewer konsumiert).
**Entsteht hier:** ADR-0009 „GUI-Framework-Bindung Qt 6 (Driving
Adapter)" — bisher §Offene ADR-Themen im
[ADR-Index](../../adr/README.md).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-12.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des Viewer-Strangs
(Muster slice-009a/010a: ADR-Accept ist Review-Checkpoint und gehört
nicht in einen Implementierungs-Slice). Die Scope-Entscheidung
slice-010a hat den sichtbaren Viewer aus welle-1 in die eigene Welle
`welle-1v-viewer` gelöst; dieser Slice liefert deren Vorbedingung.

---

## 1. Ziel

Die GUI-Grundsatz-Entscheidung wird getroffen und dokumentiert
(ADR-0009 accepted), und „sichtbar" bekommt seine
welle-1v-Operationalisierung in der Spezifikation — bevor der
Viewer-Adapter implementiert wird (slice-011b). REQ-TEC-002 setzt
Qt 6; die ADR entscheidet die **Bindungsform**, nicht das Ob.

## 2. Definition of Done

- [ ] **ADR-0009 „GUI-Framework-Bindung Qt 6 (Driving Adapter)"
      accepted** (MADR-Form, Optionen mit Trade-offs).
      Entscheidungs-Pflichten:
      (a) **UI-Technologie innerhalb Qt 6** — Qt Widgets vs.
      Qt Quick/QML für Hauptfenster und 3D-Viewport;
      (b) **Rendering-Anbindung an OCC** — OCC-Visualisierung
      (AIS/V3d) im Qt-Fenster vs. eigene Tessellation + Qt-3D/OpenGL.
      Der Status quo ist hier bereits entschieden:
      `spec/architecture.md` §2 verbietet der GUI den direkten
      OCC-Aufruf, arch-check Regel C erlaubt `*.hxx` nur in
      `src/adapters/geometry/`, und ADR-0002 legt nur das
      Port-Backend fest — eine Pro-OCC-Sicht-Entscheidung ist also
      eine **Revision** dieser Constraints (architecture.md §2 +
      Regel C nachziehen), kein Lückenfüllen (Review 011, Q3);
      (c) **Sensor-Form der Qt-Grenze** — die Bindungs-Grenze selbst
      (Qt nur im UI-Adapter, Kern framework-frei) ist durch ADR-0001
      und `architecture.md` §2 **gesetzt** und wird hier *nicht* neu
      entschieden (keine Doppel-Normativität über ADR-0001-Territorium,
      Review 011, P5); offen ist die Durchsetzung: neue
      **arch-check-Regel E** (Qt-Includes nur `src/adapters/ui/` +
      Composition-Root-Ausnahme `src/main.cpp`, ggf. künftige
      Plugin-Ausnahme), Muster Regel C/slice-003b — Umsetzung als
      Folgepflicht in slice-011b;
      (d) **Threading/Lebenszyklus und Observer-Verdrahtung** —
      UI-Thread vs. Kern-Aufrufe, Verhältnis zum synchronen
      Observer-Port und Re-Entranz-Verbot aus ADR-0008 (kein
      Mutations-Rückruf im Callback). Dazu der offene
      **Architektur-Konflikt** (Review 011, M1/Q1): der Viewer
      implementiert `ModelChangedPort` aus `ports/driven/`, die
      Schichten-Tabelle in `spec/architecture.md` erlaubt dem
      GUI-Adapter aber nur `model` + `ports/driving`, und die
      Driven-Ports-Tabelle §1.2 listet `ModelChangedPort` gar nicht —
      Import-Spalte erweitern oder Schnittstelle anders schneiden,
      §1.2 nachtragen (DoD-3);
      (e) **Composition-Root-Anbindung** — wie `main.cpp` Adapter und
      Kern verdrahtet (Injektion, Ownership);
      (f) **Headless-/Testbarkeits-Strategie** (Review 011, H1/P11) —
      wie werden Viewer-AK display-frei prüfbar (offscreen-QPA vs.
      Adapter-Logik gegen Port-Doubles vs. e2e-Treiber), **was ist das
      headless beobachtbare Surrogat für „dargestellt"**
      (Szenen-/ViewModel-Inhalt, gezählter Render-Aufruf,
      Pixel-Grab?), und wie entstehen ACC-002-Beleg-Artefakte
      (Erzeugungsweg + Ablage-Ort; manueller Abnahme-Schritt, kein
      Gate) — slice-011b referenziert diese Entscheidung nur noch.
      ADR-Index: Index-Zeile ergänzen, §Offene-ADR-Themen-Eintrag
      auflösen, Folgepflicht → slice-011b.
- [ ] **`spec/spezifikation.md` §1 nachgezogen:** die
      welle-1v-Operationalisierung von „sichtbar" (3D-Fenster zeigt
      den extrudierten Stand; Aktualisierung über den
      ADR-0008-Vertrag ohne Benutzer-Refresh) — der
      Lastenheft-Wortlaut LH-FA-D3-002 bleibt unverändert (er ist
      seit slice-010a benutzer-beobachtbar und lösungsfrei; Regel
      „Lösung schärft nie das Lastenheft"). + §8-Historie-Zeile.
- [ ] **`spec/architecture.md` nachgezogen:** UI-Adapter-Schicht-Zeile
      inkl. Import-Spalte (Observer-Konflikt, Pflicht (d)),
      `ModelChangedPort` in der Driven-Ports-Tabelle §1.2,
      `ViewModelPort`-Bezug konsistent zur ADR-0009-Entscheidung
      (insb. Pflicht (b): was fließt über welchen Port). Ergibt die
      Prüfung an einer Stelle keinen Änderungsbedarf, hält die
      Closure-Notiz das mit Begründung fest — beobachtbar statt
      behauptet (Review 011, P10).
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0009-gui-framework-qt6}.md` | neu | GUI-Grundsatz-Entscheidung (Pflichten a–e) |
| `docs/plan/adr/README.md` | ändern | Index-Zeile, §Offene-Themen-Eintrag auflösen, Folgepflicht slice-011b |
| `spec/spezifikation.md` | ändern | §1 Operationalisierung „sichtbar" (welle-1v) + §8 Historie |
| `spec/architecture.md` | ändern | UI-Adapter/`ViewModelPort` konsistent zur ADR |
| diese Datei + slice-011b | ändern | Frontmatter-`adr_refs` um ADR-0009 ergänzen, sobald die Datei existiert |

## 4. Trigger

- welle-1 done ✓ (2026-06-12). **Start der Welle `welle-1v-viewer`
  per Roadmap-Entscheidung** (Umbuchung §Nächste Wellen → §Aktuelle
  Welle) — bis dahin bleibt der Slice in `open/`.

## 5. Closure-Trigger

- DoD vollständig, ADR-0009 `Accepted`, `make gates` grün,
  Closure-Notiz geschrieben → slice-011b wird startbar.

## 6. Risiken und offene Punkte

- **Steering-Zähler geschlossen:** Das unabhängige Plan-Review dieses
  Slice-Paars (2026-06-12; 3 HIGH/6 MED/9 LOW, alle eingearbeitet)
  war das **dritte Vorkommen** der Praxis — Konvention
  festgeschrieben als
  [`MR-006`](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  + AGENTS §5 Schritt 5 (Zähler-Herkunft:
  [`welle-1-results.md` §5](../done/welle-1-results.md)).
- **Pflicht (b) trägt das meiste Gewicht:** OCC-Visualisierung direkt
  im Adapter **revidiert bestehende Constraints**
  (`architecture.md` §2, arch-check Regel C als
  ADR-0002-Folgepflicht) — das ist eine echte
  Architektur-Entscheidung mit Folgen für arch-check; nicht implizit
  in slice-011b entscheiden.
- **Kein Latenz-Budget vergeben:** „sofort" bleibt
  benutzer-beobachtbar definiert (slice-010a); ein Rebuild-/
  Render-Latenz-Budget bräuchte eine neue `LH-QA-<NNN>`-ID.
- **Sitzungs-Umfang:** sechs Pflichten in einer ADR — kippt die
  Review-Sitzung, ist (b) (Rendering-Anbindung) der natürliche
  Split-Punkt (Präzedenz 003/009).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF
- **Konventionen-Dichte:** hoch — MADR, ADR-Index-Pflicht,
  „Lösung schärft nie das Lastenheft".
- **Phase-Reife:** Spezifikation §1 Phase 3; Lastenheft unberührt.
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Wellen-Start-Disziplin,
  Roadmap-Drift-Tabelle.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).
