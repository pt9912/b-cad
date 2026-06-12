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
      (AIS/V3d) im Qt-Fenster vs. eigene Tessellation + Qt-3D/OpenGL;
      Konsistenz zu ADR-0002 (OCC hinter `GeometryKernelPort` —
      darf die Darstellungs-Seite OCC-Typen sehen, oder nur
      tessellierte Daten über den Port?);
      (c) **Bindungs-Grenze** — Qt ausschließlich unter
      `src/adapters/ui/`, Kern bleibt framework-frei (ADR-0001);
      arch-check-Folgepflicht analog Regel C (slice-003b) prüfen;
      (d) **Threading/Lebenszyklus** — UI-Thread vs. Kern-Aufrufe,
      Verhältnis zum synchronen Observer-Port und Re-Entranz-Verbot
      aus ADR-0008 (kein Mutations-Rückruf im Callback);
      (e) **Composition-Root-Anbindung** — wie `main.cpp` Adapter und
      Kern verdrahtet (Injektion, Ownership).
      ADR-Index: Zeile in §Übersicht, §Offene-ADR-Themen-Eintrag
      auflösen, Folgepflicht → slice-011b.
- [ ] **`spec/spezifikation.md` §1 nachgezogen:** die
      welle-1v-Operationalisierung von „sichtbar" (3D-Fenster zeigt
      den extrudierten Stand; Aktualisierung über den
      ADR-0008-Vertrag ohne Benutzer-Refresh) — der
      Lastenheft-Wortlaut LH-FA-D3-002 bleibt unverändert (er ist
      seit slice-010a benutzer-beobachtbar und lösungsfrei; Regel
      „Lösung schärft nie das Lastenheft"). + §8-Historie-Zeile.
- [ ] **`spec/architecture.md` geprüft/nachgezogen:** UI-Adapter-
      Schicht-Zeile und `ViewModelPort`-Bezug konsistent zur
      ADR-0009-Entscheidung (insb. Pflicht (b): was fließt über
      welchen Port).
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

- **Steering-Zähler beachten** ([`welle-1-results.md` §5](../done/welle-1-results.md)):
  Ein unabhängiges Plan-Review vor diesem Slice wäre das
  **dritte Vorkommen** der Praxis „Plan-Review vor
  Implementierungs-Start" → Konvention festschreiben
  (AGENTS §5 oder `MR-<NNN>`), nicht nur anwenden.
- **Pflicht (b) trägt das meiste Gewicht:** OCC-Visualisierung direkt
  im Adapter würde die ADR-0002-Grenze (OCC hinter Port) für die
  Darstellungs-Seite neu verhandeln — das ist eine echte
  Architektur-Entscheidung mit Folgen für arch-check; nicht implizit
  in slice-011b entscheiden.
- **Kein Latenz-Budget vergeben:** „sofort" bleibt
  benutzer-beobachtbar definiert (slice-010a); ein Rebuild-/
  Render-Latenz-Budget bräuchte eine neue `LH-QA-<NNN>`-ID.
- **Sitzungs-Umfang:** fünf Pflichten in einer ADR — kippt die
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
