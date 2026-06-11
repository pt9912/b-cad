---
id: slice-009
titel: Raum-Autoerkennung (geschlossene Wandzüge → Räume)
status: open
welle: welle-1-mvp
lastenheft_refs: [LH-FA-ROM-001]
adr_refs: [ADR-0001]
---

# Slice 009: Raum-Autoerkennung (geschlossene Wandzüge → Räume)

**Status:** open

**Welle:** welle-1-mvp

**Bezug:** LH-FA-ROM-001, ADR-0001 (+ ADR-0007 „Polygon-Basis" —
entsteht in diesem Slice)

**Autor:** Dietmar Burkard. **Datum:** 2026-06-11.

---

## 1. Ziel

Geschlossene Wandzüge eines Geschosses werden automatisch als Räume
erkannt (LH-FA-ROM-001): Wand-Segmente → Graph (Punkt-Gleichheit über
`GEOMETRY_TOLERANCE_MM`,
[`spec/spezifikation.md` §3](../../../../spec/spezifikation.md#3-defaults-und-konstanten))
→ minimale Zyklen → Raumpolygone mit Fläche. Verschachtelte Zyklen
werden innen/außen getrennt (keine Flächen-Doppelzählung), offene
Wandzüge erzeugen keinen Raum. Reine Kern-Logik (2D, OCC-frei) im
Hexagon (ADR-0001). Die offene Spec-Frage **„Mittellinie vs.
Innenkante"**
([`spec/spezifikation.md` §7](../../../../spec/spezifikation.md#7-offene-punkte))
wird vorab per ADR-0007 entschieden und in der Spezifikation
nachgezogen — ADR schärft die Spezifikation, nie das Lastenheft.

## 2. Definition of Done

- [ ] **ADR-0007 „Polygon-Basis der Raumerkennung"** (Mittellinie vs.
      Innenkante; Trade-offs inkl. Wirkung auf die spätere
      Wohnflächenberechnung LH-FA-EVL-003) accepted; ADR-Index
      aktualisiert; `spec/spezifikation.md` §1 (LH-FA-ROM-001.a)
      präzisiert und der §7-Offene-Punkt geschlossen.
- [ ] **Domain + Service:** `Room` mit starker `RoomId` (Lerneintrag
      slice-003a: Bauteil-Ids als starke Typen), Polygon und Fläche;
      Raumerkennung im Hexagon-Kern (Graph-Aufbau mit
      `GEOMETRY_TOLERANCE_MM`, minimale Zyklen, innen/außen-Trennung);
      degenerierte Geometrie → `E-GEO-002`, Modell unverändert
      (transaktional, Lerneintrag slice-003a).
- [ ] **Akzeptanz-Tests** mit `LH-`-ID im Namen: Happy (geschlossener
      Wandzug → genau ein Raum, Polygon/Fläche korrekt gegen
      analytischen Wert), Boundary (verschachtelte Wandzüge → innerer
      und äußerer Raum getrennt, keine Flächen-Doppelzählung), Negative
      (offener Wandzug → kein Raum, kein Fehler) + `E-GEO-002`-Fall.
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

Outline — der implementierende Lauf verfeinert in Schritt 4 (Modul 9):

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0007-raumerkennung-polygon-basis}.md` | neu | Lösungs-Entscheidung Polygon-Basis (schärft `spezifikation.md`) |
| `spec/spezifikation.md` | ändern | §1 LH-FA-ROM-001.a präzisieren; §7-Punkt schließen |
| `src/hexagon/model/{room,room_id}.h` | neu | Raum-Werttyp, starke Id (Konvention aus slice-003a, drittes Vorkommen → Konventions-Kandidat) |
| `src/hexagon/ports/driving/{detect_rooms_port}.h` | neu | Use-Case-Schnittstelle (analog `EditStructurePort`) |
| `src/hexagon/services/{room_detection_service}.{h,cpp}` | neu | Graph / minimale Zyklen / innen-außen; pure Domäne, kein OCC (ADR-0001) |
| `src/hexagon/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | neue Übersetzungseinheiten/Tests |
| `tests/hexagon/{test_room_detection_service}.cpp` | neu | AK-Tests LH-FA-ROM-001 |

Algorithmus-Anker: `spec/spezifikation.md` §1 (Schritte 1–4: Graph →
Zyklen → innen/außen → offene Züge ohne Raum).

## 4. Trigger

- slice-003a done (Wände als Segmente im Kern verfügbar). ✓

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz geschrieben;
  Roadmap-Closure-Zeile (`welle-1-mvp`) auf done gestellt.

## 6. Risiken und offene Punkte

- **ADR-0007 ist Vorab-Schritt:** ohne entschiedene Polygon-Basis sind
  „korrektes Polygon" und Flächen-Erwartungswerte der Tests nicht
  formulierbar. Erst ADR, dann Tests, dann Implementierung.
- **Wandverbindung (LH-FA-WAL-006) existiert noch nicht:** Zyklen
  entstehen in welle-1 nur über Punkt-Gleichheit endpunkt-verbundener
  Segmente (Toleranz), nicht über echte Verschneidung. T-Stöße und
  überlappende Wände sind Out-of-Scope dieses Slice.
- **Raum-Persistenz Out-of-Scope:** die `rooms`-Tabelle
  (`data-model.yaml`, ADR-0006) wird hier nicht befüllt — Räume sind
  abgeleitete Daten; Persistenz-/Round-Trip-Frage folgt mit
  ROM-002/003 oder eigenem Slice.
- **OTel-Span `bcad.room.detect`** (`spezifikation.md` §5) erst mit dem
  Telemetrie-Adapter (REQ-TEC-006), nicht hier.
- ROM-002/003 (Flächen-/Volumenberechnung als eigene Anforderungen)
  bleiben offen — die hier berechnete Fläche ist Bestandteil der
  Detektion (Polygon-Eigenschaft), keine Wohnflächenberechnung.

## 7. Closure-Notiz

*(bei Closure zu füllen: beobachtbare Kriterien + Lerneintrag)*

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF
- **Konventionen-Dichte:** hoch — AK-Format-Standard, §7-Offene-Punkte-
  Disziplin, ADR-Schärfungs-Regel (`harness/conventions.md` MR-001).
- **Phase-Reife:** Phase 3 (LH-FA-ROM-001 voll ausformuliert, §1-Algorithmus
  Outline mit deklariertem offenem Punkt — genau den schließt der Slice).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Pure-Domain-/Port-Konvention
  (ADR-0001, `arch-check`), starke Bauteil-Ids (slice-003a).
- **Phase-Reife:** Phase 4 (Spec/ADR führen, Code wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Test-Infrastruktur

- **Modus:** GF
- **Konventionen-Dichte:** mittel — GoogleTest-Konvention, `LH-`-ID im
  Test-Namen (seit slice-002/003a gelebt).
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** mittel — analytische
  Flächen-Erwartungswerte hängen an der ADR-0007-Entscheidung.
- **Reconciliation-Aufwand:** keiner (GF).
