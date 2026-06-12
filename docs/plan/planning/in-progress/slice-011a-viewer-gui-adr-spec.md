---
id: slice-011a
titel: Sichtbarer 3D-Viewer — GUI-Grundsatz-ADR Qt 6 & Spec-Operationalisierung ACC-002
status: done
welle: welle-1v-viewer
lastenheft_refs: [LH-FA-D3-002]  # sichtbare Hälfte; ACC-002 hängt an LH-FA-D3-001/002
adr_refs: [ADR-0001, ADR-0008, ADR-0009]  # ADR-0009 in diesem Slice entstanden + accepted
---

# Slice 011a: Sichtbarer 3D-Viewer — GUI-Grundsatz-ADR & Spec

**Status:** done

**Welle:** welle-1v-viewer (gestartet 2026-06-12 — Trigger
„welle-1 done" erfüllt, siehe
[`welle-1-results.md`](../done/welle-1-results.md)).

**Bezug:** REQ-TEC-002 (Qt 6 gesetzt), ADR-0001 (Adapter-Grenze),
ADR-0008 (Benachrichtigungs-Vertrag, den der Viewer konsumiert).
**Entsteht hier:** ADR-0009 „GUI-Framework-Bindung Qt 6 (Driving
Adapter)" — bisher §Offene ADR-Themen im
[ADR-Index](../../adr/README.md).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-12.

**Plan-Reviews (MR-006):**
[Runde 1](../../../reviews/2026-06-12-slice-011-plan.md) (R1-IDs) ·
[Runde 2](../../../reviews/2026-06-12-slice-011b-plan-w2.md)
(W2-IDs, betrifft hier Pflicht (f)/§3) — alle Findings
eingearbeitet.

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

- [x] **ADR-0009 „GUI-Framework-Bindung Qt 6 (Driving Adapter)"
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
      Regel C nachziehen), kein Lückenfüllen (R1-M2);
      (c) **Sensor-Form der Qt-Grenze** — die Bindungs-Grenze selbst
      (Qt nur im UI-Adapter, Kern framework-frei) ist durch ADR-0001
      und `architecture.md` §2 **gesetzt** und wird hier *nicht* neu
      entschieden (keine Doppel-Normativität über ADR-0001-Territorium,
      R1-M2); offen ist die Durchsetzung: neue
      **arch-check-Regel E** (Qt-Includes nur `src/adapters/ui/` +
      Composition-Root-Ausnahme `src/main.cpp`, ggf. künftige
      Plugin-Ausnahme), Muster Regel C/slice-003b — Umsetzung als
      Folgepflicht in slice-011b;
      (d) **Threading/Lebenszyklus und Observer-Verdrahtung** —
      UI-Thread vs. Kern-Aufrufe, Verhältnis zum synchronen
      Observer-Port und Re-Entranz-Verbot aus ADR-0008 (kein
      Mutations-Rückruf im Callback). Dazu der offene
      **Architektur-Konflikt** (R1-M1): der Viewer
      implementiert `ModelChangedPort` aus `ports/driven/`, die
      Schichten-Tabelle in `spec/architecture.md` erlaubt dem
      GUI-Adapter aber nur `model` + `ports/driving`, und die
      Driven-Ports-Tabelle §1.2 listet `ModelChangedPort` gar nicht —
      Import-Spalte erweitern oder Schnittstelle anders schneiden,
      §1.2 nachtragen (DoD-3);
      (e) **Composition-Root-Anbindung** — wie `main.cpp` Adapter und
      Kern verdrahtet (Injektion, Ownership);
      (f) **Headless-/Testbarkeits-Strategie** (R1-H1) — wie werden
      Viewer-AK display-frei prüfbar (offscreen-QPA vs.
      Adapter-Logik gegen Port-Doubles vs. e2e-Treiber), **was ist das
      headless beobachtbare Surrogat für „dargestellt"**
      (Szenen-/ViewModel-Inhalt, gezählter Render-Aufruf,
      Pixel-Grab?), wie entstehen ACC-002-Beleg-Artefakte
      (Erzeugungsweg, Form + Ablage-Ort; manueller Abnahme-Schritt,
      kein Gate), und der **Coverage-Umgang** für headless schwer
      abdeckbaren UI-/Rendering-Code (erwartete Abdeckbarkeit; eine
      Schwellen-/Filter-Anpassung wäre ADR-pflichtig, AGENTS §2.6 —
      W2-P6) — slice-011b referenziert diese Entscheidung nur noch.
      ADR-Index: Index-Zeile ergänzen, §Offene-ADR-Themen-Eintrag
      auflösen, Folgepflicht → slice-011b.
- [x] **`spec/spezifikation.md` §1 nachgezogen:** die
      welle-1v-Operationalisierung von „sichtbar" (3D-Fenster zeigt
      den extrudierten Stand; Aktualisierung über den
      ADR-0008-Vertrag ohne Benutzer-Refresh) — der
      Lastenheft-Wortlaut LH-FA-D3-002 bleibt unverändert (er ist
      seit slice-010a benutzer-beobachtbar und lösungsfrei; Regel
      „Lösung schärft nie das Lastenheft"). + §8-Historie-Zeile.
- [x] **`spec/architecture.md` nachgezogen:** UI-Adapter-Schicht-Zeile
      inkl. Import-Spalte (Observer-Konflikt, Pflicht (d)),
      `ModelChangedPort` in der Driven-Ports-Tabelle §1.2,
      `ViewModelPort`-Bezug konsistent zur ADR-0009-Entscheidung
      (insb. Pflicht (b): was fließt über welchen Port). Ergibt die
      Prüfung an einer Stelle keinen Änderungsbedarf, hält die
      Closure-Notiz das mit Begründung fest — beobachtbar statt
      behauptet (R1-L3).
- [x] `make gates` grün; Closure-Notiz mit Lerneintrag (§8).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0009-gui-framework-qt6}.md` | neu | GUI-Grundsatz-Entscheidung (Pflichten a–f, W2-Q2) |
| `docs/plan/adr/README.md` | ändern | Index-Zeile, §Offene-Themen-Eintrag auflösen, Folgepflicht slice-011b |
| `spec/spezifikation.md` | ändern | §1 Operationalisierung „sichtbar" (welle-1v) + §8 Historie |
| `spec/architecture.md` | ändern | UI-Adapter/`ViewModelPort` konsistent zur ADR |
| diese Datei + slice-011b | ändern | Frontmatter-`adr_refs` um ADR-0009 ergänzen, sobald die Datei existiert |

## 4. Trigger

- welle-1 done ✓ (2026-06-12); Welle `welle-1v-viewer` gestartet ✓
  (Roadmap-Umbuchung 2026-06-12); unabhängiges Plan-Review gemäß
  MR-006 gelaufen ✓ (2026-06-12, Findings eingearbeitet) →
  startbar.

## 5. Closure-Trigger

- DoD vollständig, ADR-0009 `Accepted`, `make gates` grün,
  Closure-Notiz geschrieben → slice-011b wird startbar.

## 6. Risiken und offene Punkte

- **Steering-Zähler geschlossen:** Das unabhängige Plan-Review dieses
  Slice-Paars ([Runde 1](../../../reviews/2026-06-12-slice-011-plan.md):
  3 HIGH/6 MED/9 LOW; [Re-Review
  Runde 2](../../../reviews/2026-06-12-slice-011b-plan-w2.md):
  1 HIGH/7 MED/6 LOW — alle eingearbeitet) war das **dritte
  Vorkommen** der Praxis — Konvention festgeschrieben als
  [`MR-006`](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  + AGENTS §5 Schritt 5 (Zähler-Herkunft:
  [`welle-1-results.md` §5](../done/welle-1-results.md)).
  Lern-Beobachtung aus Runde 2: Der einzige neue HIGH entstand durch
  **Überkorrektur** eines MED-Findings (W2-P1) — „Einarbeitung ist
  selbst review-würdig", 1. Vorkommen, kategorisiert im W2-Report.
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

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- [ADR-0009](../../adr/0009-gui-framework-qt6.md) mit Status
  `Accepted`, alle sechs Pflichten entschieden: (a) **Qt Widgets**,
  (b) **Tessellation über `ViewModelPort`** — kein OCC in der GUI,
  keine Revision von architecture.md §2/Regel B/Regel C,
  (c) arch-check-**Regel E** als Folgepflicht slice-011b,
  (d) single-threaded am Qt-Event-Loop; Import-Präzisierung: UI darf
  `ports/driven/` nur zur Implementierung von
  Beobachter-Schnittstellen importieren (löst R1-M1),
  (e) Konstruktor-Injektion + subscribe/unsubscribe-Lebenszyklus in
  `main.cpp`, (f) Szenen-Surrogat + offscreen-QPA +
  `acc-002-beleg`-Target (kein Gate) + Coverage-Umgang.
  (a)/(b) explizit durch den Projektinhaber gewählt (2026-06-12).
- ADR-Index: Index-Zeile, Folgepflicht-Zeile (Regel E + Gate-Doku +
  Beleg-Target → slice-011b), §Offene-Themen-Eintrag aufgelöst.
- `spec/spezifikation.md` §1 D3-002.a: welle-1v-Operationalisierung
  „sichtbar" + §8-Historie-Zeile (ADR-0009).
- `spec/architecture.md`: `ViewModelPort`-/`GeometryKernelPort`-
  Zeilen um Tessellation präzisiert, `ModelChangedPort` in §1.2
  nachgetragen, GUI-Import-Spalte erweitert — der
  R1-M1-Architektur-Konflikt ist aufgelöst, ohne die
  Abhängigkeits-Richtung zu ändern.
- `make gates` grün (2026-06-12): docs-check 0 ERROR/WARN über alle
  geänderten Artefakte, übrige Gates unverändert grün.

**Lerneintrag:**

- **Constraint-Treue entschied die schwerste Pflicht:** Bei (b) war
  die Option ohne Constraint-Revision (Tessellation über Port)
  zugleich die mit natürlich abfallendem Test-Surrogat — bestehende
  Regeln zuerst lesen macht die Optionen-Bewertung billiger und
  ehrlicher (Bestätigung der R1-M2-Verengung: nur echte Restfragen
  in die ADR).
- **Entscheider-Checkpoint schlank halten:** Nur (a)/(b) waren
  echte Wahlen; (c)–(f) folgten aus Constraints und dem zweifach
  reviewten Plan. Pflichten-Listen vorab in „echte Wahl" vs.
  „Folge-Entscheidung" zu sortieren reduziert die Checkpoint-Last
  des Projektinhabers auf das Tragende.

**Restrisiko / Nachfolge:** Folgepflichten in slice-011b (Regel E,
Gate-Doku-Nachzug, Beleg-Target außerhalb `gates`); Kamera/Shading
bewusst minimal — Erweiterungen nur über die
Re-Evaluierungs-Trigger der ADR (Selektion → AIS-Neubewertung);
ein Render-Latenz-Budget bleibt unvergeben (neue `LH-QA-<NNN>` bei
Bedarf).
