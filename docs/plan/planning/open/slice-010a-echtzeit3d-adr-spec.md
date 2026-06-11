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
      Happy/Boundary/Negative **lösungsfrei und benutzer-beobachtbar**
      formuliert — „sofort" heißt: die Darstellung folgt der
      Parameteränderung **ohne expliziten Aktualisierungs-Schritt des
      Benutzers** (kein Refresh-Befehl). **Keine Lösungsmechanik**
      (Benachrichtigung, Synchronität, Port) im Lastenheft-Text — die
      gehört in DoD-3/ADR-0008; sonst würde die ADR-Entscheidung ins
      Lastenheft vorgezogen (Plan-Review 010, F1). Der Wortlaut
      „sichtbar" bleibt benutzer-beobachtbar — das **Welle-Scoping ist
      kein Lastenheft-Inhalt** (F2), es wird in Roadmap und
      Spezifikation dokumentiert (siehe DoD-3 und §3). Ein
      Latenz-Budget wird hier nicht vergeben — es bräuchte eine neue
      `LH-QA-<NNN>`-ID (AGENTS §4) und ist zur
      Performance-Zielkomplexität (M3) abgegrenzt.
- [ ] **ADR-0008 „Änderungs-Benachrichtigung Kern → Darstellung"
      accepted** (Optionen mit Trade-offs, MADR-Form; mindestens:
      Observer-/Notifikations-Port (driven) · Polling durch den
      Adapter · Event-Queue). Entscheidungs-Pflichten:
      (a) **Vertrag/Inhalt** — was wird gemeldet (Element-Id,
      Operations-Art; **Push** des neuen Stands vs. **Pull** per
      Query), Vokabular konsistent zum OTel-Span
      `bcad.geometry.rebuild` (`element_id`, `op` — spez. §5);
      (b) **Hörer-Multiplizität und Registrierung** (OBJ-003: 2D- und
      3D-Sicht hören auf dasselbe Modell; Konstruktor-Injektion vs.
      subscribe, Lebenszyklus);
      (c) **Fehlerverhalten und Re-Entranz** der Hörer-Seite (ein
      fehlschlagender Beobachter darf die committete Mutation nicht
      kippen; darf ein Hörer im Callback in den Service zurückrufen?);
      (d) **Umfang** — welche Mutationen werden gemeldet (auch
      Geschoss-Anlage?), werden Raum-Änderungen (Re-Detektion,
      slice-009b) mitgemeldet, Mehr-Element-Updates (künftige
      LH-FA-WAL-006) nicht verbauen;
      (e) **Reihenfolge** zur bestehenden Post-Commit-Mechanik
      (`redetectRooms`). ADR-Index aktualisiert.
- [ ] **`spec/spezifikation.md` §1 präzisiert** (Echtzeit-Absatz in
      LH-FA-D3-001.a bzw. eigener D3-002-Block): **hier — nicht im
      Lastenheft —** leben Auslösung, Synchronität und der
      Benachrichtigungs-Vertrag gemäß ADR-0008 sowie die
      welle-1-Operationalisierung von „sichtbar" (Kern-Vertrag an die
      Darstellungs-Schicht; das sichtbare 3D-Fenster liefert der
      Viewer-Strang). + §8-Historie-Zeile; `make gates` grün;
      Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0008-aenderungs-benachrichtigung}.md` | neu | Lösungs-Entscheidung Benachrichtigungs-Mechanik |
| `docs/plan/adr/README.md` | ändern | Index-Zeile + ggf. Folgepflicht (slice-010b) |
| `spec/lastenheft.md` | ändern | LH-FA-D3-002 von Outline auf AK-Niveau (Reifephase-Klausel) |
| `spec/spezifikation.md` | ändern | §1 Echtzeit/Benachrichtigung präzisieren; §8 Historie |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Scope-Entscheidung Viewer-Strang vollständig: Drift-Tabelle **+ Welle-Ziel („3D-Extrusion in Echtzeit") + Viewer-Zeile der Closure-Trigger + „Letzte Änderung"-Kopf** (Plan-Review 010, F7) |
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
  braucht das einen **vollständigen** Roadmap-Nachzug (Drift-Tabelle,
  Welle-Ziel, Viewer-Zeile der Closure-Trigger; F7) — kein stilles
  `done` über den Kern-Vertrag. Der Lastenheft-Wortlaut bleibt davon
  unberührt (F2).
- **„Sofort" vs. Budget:** Benutzer-Beobachtbarkeit (kein
  Refresh-Schritt) und Latenz-Budget nicht vermischen — ein
  Rebuild-Latenz-Budget existiert bisher als Anforderung nicht; falls
  nötig, bräuchte es eine neue `LH-QA-<NNN>`-ID (Vergabe beim
  Spec-Schreiben, AGENTS §4) und gehört zur Performance-
  Zielkomplexität (M3), nicht in die welle-1-AK. (Der ererbte
  Lastenheft-Querverweis „vgl. LH-QA-001" betrifft die Projektöffnung,
  kein Rebuild-Budget; F8.)
- **Wiederholungsfall „Post-Commit-Schritt":** Die Benachrichtigung ist
  nach der Raum-Re-Detektion (slice-009b) das **zweite Vorkommen**
  einer nicht-werfenden Nachlauf-Mechanik nach transaktionalem Commit.
  Achtung bei der MR-Entscheidung: der 009-Lerneintrag fasste die
  Klasse enger („**ableitende Berechnungen** sind total") — eine
  Benachrichtigung ist keine ableitende Berechnung; die Verbreiterung
  auf „Post-Commit-Schritte sind total/nicht-werfend" ist selbst Teil
  der Verallgemeinerungs-Entscheidung und gehört explizit in deren
  Begründung (F9). Steering Loop: zweites Vorkommen beobachtet, im
  Slice entscheiden, ob `MR-<NNN>` oder noch warten.

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
