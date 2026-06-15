---
id: slice-010a
titel: Echtzeit-3D — AK-Schärfung LH-FA-D3-002 & ADR-0008 Benachrichtigung
status: done
welle: welle-1-mvp
lastenheft_refs: [LH-FA-D3-002]
adr_refs: [ADR-0001, ADR-0008]
---

# Slice 010a: Echtzeit-3D — AK-Schärfung & ADR-0008 Benachrichtigung

**Status:** done

**Welle:** welle-1-mvp

**Bezug:** LH-FA-D3-002, ADR-0001. **Geliefert:**
[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) (in diesem
Slice entstanden und accepted).

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

- [x] **Lastenheft LH-FA-D3-002 geschärft** (von Outline auf
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
      `LH-QA-<NNN>`-ID (AGENTS §4); ggf. als eigener §7-Punkt der
      Spezifikation parken (der bestehende M3-Punkt deckt nur die
      Raumerkennung). Der **ererbte irreführende Querverweis
      „vgl. LH-QA-001"** im LH-FA-D3-002-Eintrag wird beim Schärfen
      **gestrichen/korrigiert** (LH-QA-001 = Projektöffnung, kein
      Rebuild-Budget — Re-Review F8-Rest).
- [x] **ADR-0008 „Änderungs-Benachrichtigung Kern → Darstellung"
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
- [x] **`spec/spezifikation.md` §1 präzisiert** (Echtzeit-Absatz in
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

- **Sitzungs-Umfang an der Obergrenze:** DoD-2 bündelt fünf Facetten
  *einer* Mechanik-Entscheidung (trägt); kippt die Review-Sitzung,
  ist die davon unabhängige Viewer-/Welle-Scope-Entscheidung der
  natürliche Split-Punkt (Präzedenz 003/009; Re-Review LOW-2).
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

**Closure-Kriterien (beobachtbar):**

- ADR-0008 mit Status `Accepted`, Index-Zeile und Folgepflicht-Eintrag
  (Umsetzung → slice-010b); alle fünf Entscheidungs-Pflichten (a)–(e)
  im ADR entschieden (Observer-Port synchron · Push-Notify/Pull-State
  mit `element_id`/`op` · subscribe/unsubscribe mehrfach ·
  Kapselung + Re-Entranz-Verbot · Meldung nach `redetectRooms`;
  Umfang: alle committeten Mutationen inkl. Geschoss-Anlage und
  Raum-Änderungs-Meldung).
- Lastenheft 0.1.1: LH-FA-D3-002 auf AK-Niveau, lösungsfrei und
  benutzer-beobachtbar (Happy/Boundary/Negative); irreführender
  Querverweis „vgl. LH-QA-001" gestrichen (F8-Rest); Historie-Zeile.
- `spec/spezifikation.md` §1 D3-002.a (Auslösung/Synchronität,
  Vertrag, Beobachter-Pflichten, welle-1-Operationalisierung
  „sichtbar") + §8-Historie.
- Roadmap-Nachzug vollständig (F7): Welle-Ziel, Viewer-Trigger-Zeile
  (entschieden), neue Welle `welle-1v-viewer`, Drift-Tabellen-Eintrag,
  ADR-Index §Offene-Themen-Zeile aktualisiert.
- `make gates` grün (docs-check über alle geänderten Artefakte).

**Scope-Entscheidung (die heikle):** Sichtbarer 3D-Viewer →
**eigene Welle `welle-1v-viewer`** nach welle-1 (kein
welle-1-Closure-Trigger). Begründung: GUI-Grundsatz-ADR (Qt 6) fehlt
noch; der M1-Trigger verlangt ACC-001-Kern + Gates, keinen Viewer;
ACC-002 und die sichtbare Hälfte von LH-FA-D3-002 werden in
`welle-1v-viewer` erfüllt — dokumentiert statt still uminterpretiert.
*Diese Zuordnung ist per Roadmap-Drift-Eintrag revidierbar (Veto des
Auftraggebers genügt — dann wandert die Viewer-Welle vor die
welle-1-Closure).*

**Lerneintrag:**

- **MR-Entscheidung zum „Post-Commit total"-Muster: vertagt.** Zweites
  Vorkommen ist beobachtet und kategorisiert (ADR-0008 §Konsequenzen);
  die nötige Klassen-Verbreiterung („ableitende Berechnungen" →
  „Post-Commit-Schritte", F9) spricht dafür, die Konvention erst beim
  dritten Vorkommen zu schreiben — Kurs-Regel: zweimal =
  kategorisieren, dreimal = Regel. Erwarteter dritter Kandidat:
  Autosave-Nachlauf (LH-QA-004) oder OTel-Spans (REQ-TEC-006).
- **Geschärfte Regel (bestätigt):** Die Review-009-Regel „Lösung
  schärft nie das Lastenheft" hat in diesem Slice zweimal gegriffen
  (Plan-Review F1/F2) — die Trennung „AK benutzer-beobachtbar im
  Lastenheft, Mechanik in Spezifikation+ADR" ist jetzt zweifach
  geprobt und Kandidat für eine AGENTS-/MR-Festschreibung beim
  nächsten Schärfungs-Slice.

**Restrisiko / Nachfolge:** Umsetzung in slice-010b (Folgepflicht);
Viewer-Welle braucht als Erstes die GUI-Grundsatz-ADR; ein
Latenz-Budget bleibt bewusst unvergeben (neue `LH-QA-<NNN>` bei
Bedarf).

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
