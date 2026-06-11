---
id: slice-010b
titel: Echtzeit-3D — Kern-Benachrichtigung (Implementierung)
status: open
welle: welle-1-mvp
lastenheft_refs: [LH-FA-D3-002]
adr_refs: [ADR-0001]
---

# Slice 010b: Echtzeit-3D — Kern-Benachrichtigung (Implementierung)

**Status:** open

**Welle:** welle-1-mvp

**Bezug:** LH-FA-D3-002, ADR-0001, ADR-0008 (entsteht in slice-010a;
Frontmatter-`adr_refs` wird nach dessen Closure ergänzt).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-11.

**Schnitt-Herkunft:** Implementierungs-Hälfte von LH-FA-D3-002
(Split-Begründung in slice-010a §Schnitt-Herkunft). Qt-/OCC-frei —
der sichtbare 3D-Viewer ist ein eigener Strang nach der
GUI-Grundsatz-ADR (Scope-Entscheidung aus slice-010a).

---

## 1. Ziel

Der Hexagon-Kern erfüllt den in slice-010a geschärften
Echtzeit-Vertrag: Jede committete Modell-Mutation (Wand anlegen,
Parameter ändern) führt **synchron im Mutationspfad** — gemäß
ADR-0008 — zu einer Benachrichtigung an die Darstellungs-Schicht
(driven Port), mit dem inkrementell aktualisierten Solid-Stand aus dem
bestehenden Rebuild (slice-003a). Kein Qt, kein OCC: Die
Darstellungs-Seite wird im Test durch ein Port-Double vertreten.

## 2. Definition of Done

- [ ] **Port + Service-Integration gemäß ADR-0008:** Notifikations-Port
      (driven) mit dem in der ADR entschiedenen Vertrag (mindestens:
      betroffenes Element, Operations-Art); `StructureEditService`
      benachrichtigt **nach** dem transaktionalen Commit jeder
      Wand-Mutation (Reihenfolge zur Raum-Re-Detektion gemäß ADR-0008);
      ein werfender Beobachter kippt die committete Mutation nicht
      (Fehlerverhalten gemäß ADR-0008).
- [ ] **Akzeptanz-Tests** mit `LH-`-ID im Namen gegen die in 010a
      geschärften AK: Happy (Parameteränderung → Benachrichtigung mit
      aktualisiertem Solid, synchron, ohne expliziten Abruf), Boundary
      und Negative gemäß geschärftem Lastenheft (z. B.
      verworfene/abgelehnte Mutation → keine Benachrichtigung;
      werfender Beobachter → Modell konsistent), mit Double, das
      Benachrichtigungen zählt und prüft.
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag;
      Roadmap-Closure-Zeile nachgezogen.

## 3. Plan (vor Code)

Outline — der implementierende Lauf verfeinert in Schritt 4 (Modul 9);
Port-Name und Vertrag folgen ADR-0008:

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driven/{model_changed_port}.h` | neu | Notifikations-Vertrag (Name/Inhalt gemäß ADR-0008) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Benachrichtigung nach Commit (Reihenfolge zu redetectRooms gemäß ADR-0008) |
| `src/hexagon/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | neue Übersetzungseinheiten/Tests (falls .cpp nötig) |
| `tests/hexagon/{test_model_change_notification}.cpp` | neu | AK-Tests LH-FA-D3-002 mit zählendem Port-Double |

## 4. Trigger

- slice-010a done — ADR-0008 `Accepted`, LH-FA-D3-002-AK geschärft.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz geschrieben;
  Roadmap-Closure-Zeile (`welle-1-mvp`) nachgezogen.

## 6. Risiken und offene Punkte

- **Hörer-Fehlverhalten:** Wirft der Beobachter, darf die committete
  Mutation nicht kippen — drittes Vorkommen der „total"-Klasse nach
  Raum-Re-Detektion (009b) und ADR-0007-Entscheidung; falls 010a die
  Konvention als `MR-<NNN>` festschreibt, hier erstmals dagegen
  implementieren.
- **Reihenfolge der Post-Commit-Schritte** (Re-Detektion ↔
  Benachrichtigung): muss in ADR-0008 entschieden sein, sonst rät die
  Implementierung.
- **Viewer-Strang ausdrücklich nicht hier:** sichtbares 3D-Fenster
  (Qt/OCC) folgt der GUI-Grundsatz-ADR; ACC-002 bleibt offen, bis er
  geliefert ist.
- Mehr-Element-Updates (eine Mutation, mehrere betroffene Solids — z. B.
  künftige Wandverschneidung WAL-006) sind welle-1 nicht relevant;
  Vertrag sollte sie aber nicht verbauen (ADR-0008-Trade-off).

## 7. Closure-Notiz

*(bei Closure zu füllen: beobachtbare Kriterien + Lerneintrag)*

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Pure-Domain-/Port-Konvention
  (ADR-0001, `arch-check`), transaktionale Mutation + nicht-werfende
  Post-Commit-Schritte (slice-003a/009b).
- **Phase-Reife:** Phase 4 (nach 010a führen Spec/ADR auch für die
  Benachrichtigung; Code wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Test-Infrastruktur

- **Modus:** GF
- **Konventionen-Dichte:** mittel — GoogleTest-Konvention, `LH-`-ID im
  Test-Namen, zählende Port-Doubles (Muster aus 003a/009b).
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig — Erwartungswerte folgen den
  in 010a geschärften AK.
- **Reconciliation-Aufwand:** keiner (GF).
