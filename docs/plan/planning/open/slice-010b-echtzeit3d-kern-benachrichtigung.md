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
Echtzeit-Vertrag **mit der dort in ADR-0008 entschiedenen Mechanik**:
Committete Wand-Mutationen (Anlage, Parameteränderung; der genaue
Melde-Umfang — auch Geschoss-Anlage? Raum-Änderungen? — folgt
ADR-0008 §Umfang) machen den inkrementell aktualisierten Stand des
bestehenden Rebuilds (slice-003a) für die Darstellungs-Schicht
verfügbar. Dieser Plan ist auf das ADR-Ergebnis **parametrisiert** —
er legt Mechanik (Observer/Polling/Queue), Push-vs-Pull und
Reihenfolge nicht selbst fest (Plan-Review 010, F3). Kein Qt, kein
OCC: Die Darstellungs-Seite wird im Test durch ein Double der
ADR-gemäßen Schnittstelle vertreten.

## 2. Definition of Done

- [ ] **Mechanik-Umsetzung gemäß ADR-0008:** die in der ADR
      entschiedene Benachrichtigungs-/Bereitstellungs-Mechanik
      (Observer-Port, Polling-Schnittstelle oder Event-Queue) mit dem
      dort entschiedenen Vertrag (Inhalt/Push-vs-Pull, Umfang,
      Multiplizität/Registrierung, Re-Entranz- und Fehlerverhalten,
      Reihenfolge zur Raum-Re-Detektion) im `StructureEditService`
      verankert; die transaktionale Commit-Garantie aus slice-003a
      bleibt unangetastet (eine fehlschlagende Darstellungs-Seite
      kippt keine committete Mutation — Detailform gemäß ADR-0008).
- [ ] **Akzeptanz-Tests** mit `LH-`-ID im Namen gegen die in 010a
      geschärften AK: Happy (Parameteränderung → der aktualisierte
      Stand ist für die Darstellungs-Seite gemäß ADR-0008-Vertrag
      verfügbar, ohne expliziten Benutzer-Schritt — ob als Push-Inhalt
      oder Pull-Query folgt der ADR, Plan-Review 010, F6), Boundary
      und Negative gemäß geschärftem Lastenheft (z. B.
      verworfene/abgelehnte Mutation → kein neuer Stand/keine Meldung;
      fehlschlagende Darstellungs-Seite → Modell konsistent), mit
      zählendem/prüfendem Double der ADR-gemäßen Schnittstelle.
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag;
      Roadmap-Closure-Zeile nachgezogen.

## 3. Plan (vor Code)

Outline — der implementierende Lauf verfeinert in Schritt 4 (Modul 9).
Die Tabelle zeigt die **Observer-Port-Variante als Platzhalter**;
fällt ADR-0008 anders aus (Polling/Event-Queue), ersetzt der Lauf die
Zeilen entsprechend (Plan-Review 010, F3):

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

- **Hörer-/Abnehmer-Fehlverhalten:** Eine fehlschlagende
  Darstellungs-Seite darf die committete Mutation nicht kippen —
  zusammen mit der Raum-Re-Detektion das **zweite Vorkommen** der
  Post-Commit-Klasse (Zählung wie slice-010a §6: ADR-0007-Entscheidung
  und 009b-Implementierung sind *ein* Vorkommen; Plan-Review 010, F5).
  Falls 010a die Konvention als `MR-<NNN>` festschreibt, hier erstmals
  dagegen implementieren.
- **Reihenfolge der Post-Commit-Schritte** (Re-Detektion ↔
  Benachrichtigung): muss in ADR-0008 entschieden sein, sonst rät die
  Implementierung.
- **Viewer-Strang ausdrücklich nicht hier:** sichtbares 3D-Fenster
  (Qt/OCC) folgt der GUI-Grundsatz-ADR; ACC-002 bleibt offen, bis er
  geliefert ist.
- Mehr-Element-Updates (eine Mutation, mehrere betroffene Solids — z. B.
  künftige Wandverschneidung LH-FA-WAL-006) sind welle-1 nicht relevant;
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
