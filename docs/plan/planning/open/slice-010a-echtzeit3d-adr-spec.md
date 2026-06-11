---
id: slice-010a
titel: Echtzeit-3D — AK-Schärfung LH-FA-D3-002 & ADR-0008 Benachrichtigung
status: open
welle: welle-1-mvp
lastenheft_refs: [LH-FA-D3-002]
adr_refs: [ADR-0001]
---

# Slice 010a: Echtzeit-3D — AK-Schärfung & ADR-0008 Benachrichtigung

**Status:** open

**Welle:** welle-1-mvp

**Bezug:** LH-FA-D3-002, ADR-0001. **Liefert:** ADR-0008 (entsteht in
diesem Slice; Frontmatter-`adr_refs` wird bei Closure ergänzt, wenn
die ADR-Datei existiert).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-11.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte von LH-FA-D3-002
(Muster slice-009a/b): Die Anforderung ist im Lastenheft nur Outline,
„sofort" braucht ein messbares Kriterium und „sichtbar" eine
entschiedene Mechanik — beides gehört vor die Implementierung
(slice-010b). Der eigentliche 3D-Viewer (Qt/OCC-Darstellung) ist
bewusst **kein** Teil dieses Strangs: er hängt an der noch offenen
Grundsatz-ADR „GUI-Framework-Bindung Qt 6" (ADR-Index §Offene
ADR-Themen) und wird nach der hier getroffenen Scope-Entscheidung
eigenständig geschnitten.

---

## 1. Ziel

LH-FA-D3-002 („Echtzeitaktualisierung — Parameteränderung sofort in 3D
sichtbar") bekommt **prüfbare Akzeptanzkriterien** und eine
**entschiedene Benachrichtigungs-Mechanik** Kern → Darstellung
(ADR-0008), bevor implementiert wird. Vorhandene Basis: der
inkrementelle, transaktionale Rebuild des betroffenen Solids existiert
seit slice-003a — neu sind das messbare „sofort", der
Benachrichtigungs-Vertrag und die explizite Klärung, was „sichtbar"
in welle-1 bedeutet.

## 2. Definition of Done

- [ ] **Lastenheft LH-FA-D3-002 geschärft** (von Outline auf
      Akzeptanz-Niveau — die Schärfung pro Slice ist im
      Lastenheft-Reifephase-Block ausdrücklich vorgesehen):
      Happy/Boundary/Negative mit **messbarem „sofort"** (Vorschlag:
      Rebuild und Benachrichtigung laufen *synchron im Mutationspfad*,
      kein expliziter Refresh-Schritt; konkrete Festlegung im Slice,
      Abgrenzung zum Performance-Budget LH-QA-001) und **explizitem
      „sichtbar"-Scope für welle-1**: Kern-Vertrag (Benachrichtigung an
      die Darstellungs-Schicht) jetzt, sichtbares 3D-Fenster mit dem
      Viewer-Strang — keine stille Uminterpretation des Wortlauts
      (Lehre aus Review-Finding M4, slice-009); die Welle-Zuordnung des
      Viewer-Strangs wird dabei entschieden und in der Roadmap
      (Drift-Tabelle) dokumentiert.
- [ ] **ADR-0008 „Änderungs-Benachrichtigung Kern → Darstellung"
      accepted** (Optionen mit Trade-offs, MADR-Form; mindestens:
      Observer-/Notifikations-Port (driven) · Polling durch den
      Adapter · Event-Queue): Vertrag (was wird gemeldet — Element-Id,
      Operations-Art?), Fehlerverhalten der Hörer-Seite (ein werfender
      Beobachter darf die committete Mutation nicht kippen) und
      Verhältnis zur bestehenden Post-Commit-Mechanik
      (Raum-Re-Detektion, slice-009b). ADR-Index aktualisiert.
- [ ] **`spec/spezifikation.md` §1 präzisiert** (Echtzeit-Absatz in
      LH-FA-D3-001.a bzw. eigener D3-002-Block: Auslösung, Synchronität,
      Benachrichtigungs-Vertrag gemäß ADR-0008) + §8-Historie-Zeile;
      `make gates` grün; Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0008-aenderungs-benachrichtigung}.md` | neu | Lösungs-Entscheidung Benachrichtigungs-Mechanik |
| `docs/plan/adr/README.md` | ändern | Index-Zeile + ggf. Folgepflicht (slice-010b) |
| `spec/lastenheft.md` | ändern | LH-FA-D3-002 von Outline auf AK-Niveau (Reifephase-Klausel) |
| `spec/spezifikation.md` | ändern | §1 Echtzeit/Benachrichtigung präzisieren; §8 Historie |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Scope-Entscheidung Viewer-Strang (Drift-Tabelle) |
| diese Datei + slice-010b | ändern | Frontmatter-`adr_refs` um ADR-0008 ergänzen, sobald die Datei existiert |

## 4. Trigger

- Keiner — sofort startbar (Rebuild-Basis existiert seit slice-003a;
  reine Doku-/Entscheidungsarbeit). ✓

## 5. Closure-Trigger

- DoD vollständig, ADR-0008 `Accepted`, `make gates` grün,
  Closure-Notiz geschrieben → slice-010b wird startbar.

## 6. Risiken und offene Punkte

- **Scope-Entscheidung „sichtbar"/Viewer ist der heikelste Punkt:**
  ACC-002 („Gebäude wird automatisch als 3D-Modell dargestellt") hängt
  real am Viewer. Wird der Viewer-Strang aus welle-1 herausgelöst,
  braucht das einen ehrlichen Roadmap-Eintrag (Drift-Tabelle) — kein
  stilles `done` über den Kern-Vertrag.
- **„Sofort" vs. Budget:** Synchronität (kein Refresh-Schritt) und
  Latenz-Budget (LH-QA-001-Familie) nicht vermischen — das Budget für
  den Rebuild großer Modelle gehört eher zur
  Performance-Zielkomplexität (M3), nicht in die welle-1-AK.
- **Wiederholungsfall „Post-Commit-Schritt":** Die Benachrichtigung ist
  nach der Raum-Re-Detektion (slice-009b) das **zweite Vorkommen**
  einer nicht-werfenden Nachlauf-Mechanik nach transaktionalem Commit —
  Kandidat, die Konvention „ableitende/nachgelagerte Schritte sind
  total" jetzt zu verallgemeinern (Steering Loop: zweites Vorkommen
  beobachtet, im Slice entscheiden, ob `MR-<NNN>` oder noch warten).

## 7. Closure-Notiz

*(bei Closure zu füllen: beobachtbare Kriterien + Lerneintrag)*

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF
- **Konventionen-Dichte:** hoch — AK-Format, Reifephase-Klausel im
  Lastenheft, ADR-Schärfungs-Regel (MR-001).
- **Phase-Reife:** Phase 2 für LH-FA-D3-002 (Outline — genau das hebt
  der Slice auf AK-Niveau); Spezifikation §1 Phase 3.
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF
- **Konventionen-Dichte:** hoch — ADR-Konvention (MADR, Accepted
  immutable, Index-Pflicht), Roadmap-Drift-Disziplin.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).
