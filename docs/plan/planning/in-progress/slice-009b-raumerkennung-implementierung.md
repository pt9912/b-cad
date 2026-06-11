---
id: slice-009b
titel: Raum-Autoerkennung — Implementierung (geschlossene Wandzüge → Räume)
status: open
welle: welle-1-mvp
lastenheft_refs: [LH-FA-ROM-001]
adr_refs: [ADR-0001, ADR-0007]
---

# Slice 009b: Raum-Autoerkennung — Implementierung

**Status:** open

**Welle:** welle-1-mvp

**Bezug:** LH-FA-ROM-001, ADR-0001,
[ADR-0007](../../adr/0007-raumerkennung-geometrie-basis.md)
(entstanden in slice-009a).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-11.

**Schnitt-Herkunft:** Implementierungs-Hälfte des ursprünglichen
`slice-009` (Split-Begründung in slice-009a §Schnitt-Herkunft,
Roadmap-Drift-Tabelle 2026-06-11).

---

## 1. Ziel

Geschlossene Wandzüge eines Geschosses werden **automatisch** als Räume
erkannt (LH-FA-ROM-001): Bei Modell-Mutation (Wand anlegen/ändern)
löst der Service die Erkennung aus — Wand-Segmente → Graph
(Punkt-Gleichheit über `GEOMETRY_TOLERANCE_MM`,
[`spec/spezifikation.md` §3](../../../../spec/spezifikation.md#3-defaults-und-konstanten))
→ minimale Zyklen → Räume mit Polygon-Basis und
Verschachtelungs-Repräsentation **gemäß ADR-0007** (innen/außen ohne
Flächen-Doppelzählung); offene Wandzüge erzeugen keinen Raum. Reine
Kern-Logik (2D, OCC-frei) im Hexagon (ADR-0001).

## 2. Definition of Done

- [ ] **Domain + Service:** `Room` mit starker `RoomId` (Lerneintrag
      slice-003a), Repräsentation gemäß ADR-0007; Erkennung im
      Hexagon-Kern (Graph mit `GEOMETRY_TOLERANCE_MM`, minimale Zyklen,
      innen/außen-Trennung nach ADR-0007); **Auto-Re-Detektion**: der
      `StructureEditService` stößt die Erkennung nach
      Wand-Mutationen an (LH-FA-ROM-001 „automatisch … when er
      geschlossen wird"), Abfrage über Driving-Port; Fehlerfall gemäß
      der in 009a spezifizierten `E-GEO-002`-Bedingung transaktional
      (Modell unverändert, Lerneintrag slice-003a).
- [ ] **Akzeptanz-Tests** mit `LH-`-ID im Namen, Erwartungswerte gemäß
      ADR-0007-Basis: Happy (Wandzug wird durch letzte Wand geschlossen
      → genau ein Raum entsteht ohne expliziten Abruf, Polygon/Fläche
      analytisch korrekt), Boundary (verschachtelte Wandzüge → innerer
      und äußerer Raum getrennt, Summe der Flächen ohne Doppelzählung —
      prüft die ADR-0007-Repräsentation), Negative (offener Wandzug →
      kein Raum, kein Fehler) + `E-GEO-002`-Fall gemäß 009a-Spec (oder
      begründeter Entfall, falls 009a „kein E-GEO-002" festlegt).
- [ ] `make gates` grün; Closure-Notiz mit Lerneintrag;
      Roadmap-Closure-Zeile nachgezogen.

## 3. Plan (vor Code)

Outline — der implementierende Lauf verfeinert in Schritt 4 (Modul 9):

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{room,room_id}.h` | neu | Raum-Werttyp gemäß ADR-0007, starke Id (Konvention aus slice-003a, drittes Vorkommen → Konventions-Kandidat) |
| `src/hexagon/services/{room_detection}.{h,cpp}` | neu | Graph / minimale Zyklen / innen-außen; pure Domäne, kein OCC (ADR-0001) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Auto-Re-Detektion nach Wand-Mutationen (LH-FA-ROM-001 Happy: Raum entsteht beim Schließen) |
| `src/hexagon/ports/driving/{detect_rooms_port}.h` | neu | Abfrage-Schnittstelle für erkannte Räume (analog `EditStructurePort`) |
| `src/hexagon/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | neue Übersetzungseinheiten/Tests |
| `tests/hexagon/{test_room_detection}.cpp` | neu | AK-Tests LH-FA-ROM-001 |

Algorithmus-Anker: `spec/spezifikation.md` §1 in der durch 009a
geschärften Fassung (Polygon-Basis, Verschachtelungs-Repräsentation,
`E-GEO-002`-Bedingung, Endpunkt-Knoten-Einschränkung).

## 4. Trigger

- slice-009a done — ADR-0007 `Accepted`, Spec §1 geschärft. ✓
  (2026-06-11; damit startbar.)

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz geschrieben;
  Roadmap-Closure-Zeile (`welle-1-mvp`) auf done gestellt.

## 6. Risiken und offene Punkte

- **Wandverbindung (LH-FA-WAL-006) existiert noch nicht:** Zyklen
  entstehen in welle-1 nur über Punkt-Gleichheit endpunkt-verbundener
  Segmente (Toleranz) — in 009a auch so in der Spec festgehalten.
  T-Stöße und überlappende Wände sind Out-of-Scope dieses Slice.
- **Raum-Persistenz Out-of-Scope:** die `rooms`-Tabelle
  (`spec/data-model.yaml`, ADR-0006) wird hier nicht befüllt — Räume
  sind in welle-1 abgeleitete Laufzeit-Daten. Die Spannung „abgeleitet
  vs. persistiertes Schema" ist dokumentiert und wird mit ROM-002/003
  oder einem eigenen Persistenz-Slice aufgelöst (Round-Trip-Frage).
- **OTel-Span `bcad.room.detect`** (`spezifikation.md` §5) erst mit dem
  Telemetrie-Adapter (REQ-TEC-006), nicht hier.
- **Auto-Re-Detektion und Performance:** vollständige Neu-Erkennung
  nach jeder Mutation ist in welle-1 akzeptabel (kleine Modelle); die
  Performance-Zielkomplexität bleibt als §7-Punkt offen (M3) —
  inkrementelle Erkennung ist dort zu entscheiden, nicht hier.
- ROM-002/003 (Flächen-/Volumenberechnung als eigene Anforderungen)
  bleiben offen — die hier berechnete Fläche ist Bestandteil der
  Detektion (Polygon-Eigenschaft), keine Wohnflächenberechnung.

## 7. Closure-Notiz

*(bei Closure zu füllen: beobachtbare Kriterien + Lerneintrag)*

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Pure-Domain-/Port-Konvention
  (ADR-0001, `arch-check`), starke Bauteil-Ids (slice-003a),
  transaktionale Mutation (slice-003a).
- **Phase-Reife:** Phase 4 (Spec/ADR führen — nach 009a auch für die
  Raumerkennung; Code wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Test-Infrastruktur

- **Modus:** GF
- **Konventionen-Dichte:** mittel — GoogleTest-Konvention, `LH-`-ID im
  Test-Namen (seit slice-002/003a gelebt).
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig — analytische
  Erwartungswerte sind nach 009a (ADR-0007) eindeutig ableitbar.
- **Reconciliation-Aufwand:** keiner (GF).
