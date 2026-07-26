---
id: slice-052
titel: Sitzungs-Datei merken + Warnung vor ungesicherten Änderungen (GUI-Datenverlust-Schutz)
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md)]
---

# Slice 052: Sitzungs-Datei merken + Ungesichert-Warnung

**Status:** open — **review-reif** (2026-07-26), Scope-Reservierung aus der
[slice-047](../done/slice-047-projekt-oeffnen.md)-Validation. **Fünf Fragen sind bereits entschieden**
(§2 Lösung · §3 Anforderungs-Ebene · §4 Abgrenzung · §5 Wirkungs-Breite · §6 Orakel-Schnitt), DoD (§7) und
Datei-Plan (§8) stehen — der Plan ist damit **review-reif**. Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
**vor dem Start** — die Vorab-Entscheidungen sind sein Prüfgegenstand, nicht sein Ersatz.

**Welle:** welle-5-erweiterung. **Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser (Validations-Rest slice-047, 2026-07-26)

Der Projektinhaber hat slice-047 mit **„angenommen mit benanntem Rest"** validiert
([`validator.md`](../../../../.harness/skills/validator.md)). Vier Punkte wurden als Rest benannt;
**zwei davon gehören zusammen** und bilden diesen Slice:

1. **Kein „Speichern" auf die offene Datei.** Es gibt nur **Speichern unter…** — jeder Speichervorgang
   fragt erneut nach einem Pfad. Der zuletzt geöffnete/gespeicherte Pfad wird zwar in den Fenstertitel
   geschrieben, aber **nicht gemerkt**.
2. **Keine Warnung vor ungesicherten Änderungen.** Wer ein anderes Projekt öffnet oder das Fenster
   schließt, verliert seine Änderungen **still** — es gibt keine Rückfrage und keinen Hinweis.

Die beiden hängen an **derselben fehlenden Sache**: die Sitzung hat keinen Zustand „welche Datei bin
ich, und bin ich seit dem letzten Schreiben verändert worden?". Deshalb ein Slice, nicht zwei.

**Warum das ein Datenverlust-Thema ist:** [`harness/README.md`](../../../../harness/README.md)
§Safety nennt Datenverlust am Gebäudemodell den **schärfsten Fehlerfall**. Die bisherige Absicherung
([`LH-QA-005`](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery) Crash-Recovery,
[`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) Atomarität) schützt
gegen Verlust **beim Schreiben**. Hier geht es um den Verlust **ohne** Schreiben — eine Lücke, die
erst entstand, als slice-047 dem Benutzer überhaupt eine Sitzung gab, die er verlieren kann.

## 1. Ziel

Die GUI-Sitzung bekommt zwei benutzer-beobachtbare Eigenschaften:

- **Sie weiß, welche Datei sie ist** → „Speichern" schreibt ohne Pfad-Dialog dorthin zurück;
  „Speichern unter…" bleibt daneben bestehen.
- **Sie weiß, ob sie ungesichert ist** → jede Aktion, die den Sitzungs-Stand verwirft (anderes Projekt
  öffnen, Fenster schließen), fragt vorher nach.

## 2. Lösung — **entschieden**

Der Sitzungs-Zustand ist genau ein Paar: **welche Datei bin ich** + **bin ich seit dem letzten
Schreiben verändert**. Beides bekommt **einen** Träger.

**Ort: der Kern** (`src/hexagon/services/`), framework-frei. Nicht verhandelbar aus drei Gründen, zwei
davon am Gate belegbar:

1. **`main.cpp` ist orakel-los per Konstruktion** — der blockierende Verify-Befund aus
   [slice-047](../done/slice-047-projekt-oeffnen.md) §7. „Bin ich verändert?" ist eine **Entscheidung**.
2. **`ui/command/` kann es nicht tragen:** der Träger muss den
   [`ModelChangedPort`](../../../../src/hexagon/ports/driven/model_changed_port.h) implementieren, und
   die Kante `ui_command → ports_driven` **existiert nicht** in [`.a-check.yml`](../../../../.a-check.yml)
   (nur `ui_view → ports_driven` und `services → ports_driven`). Sie zu ergänzen wäre eine
   Gate-Lockerung mit ADR-Pflicht ([§2.6](../../../../AGENTS.md)) — für einen Zustand, der ohnehin
   Qt-frei ist, ein schlechter Tausch.
3. **`ui/view/` wäre semantisch falsch:** die Sitzung ist keine Sicht.

**Form: ein `ProjectSession` im Kern, der selbst `ModelChangedPort`-Beobachter ist.**

- **Dirty entsteht aus Meldungen, nicht aus Aufrufen.** Genau das macht den Schutz
  **mutator-agnostisch** (§5): jede committete Mutation meldet über
  [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), also auch jede künftige, ohne dass ein
  neuer Mutator daran denken muss.
- **`ModelReplaced` setzt zurück, statt zu setzen.** Ein frisch geöffnetes Projekt ist **nicht**
  verändert — und weil `openProject` genau eine `ModelReplaced`-Meldung erzeugt (slice-047, belegt durch
  `…NotifiesExactlyOneFullRefresh`), fällt der Rücksetz-Fall des Öffnens **mit der Meldung zusammen**.
  Das ist der leicht zu übersehende Fall und hier strukturell erledigt.
- **Der Pfad kommt von den Handlern**, die ihn ohnehin haben (`openProject`/`saveProject`): erfolgreiches
  Öffnen und erfolgreiches Speichern melden ihn der Sitzung. **Nach**, nicht vor dem Schreiben (§9 R2).
- **Das Verdikt ist eine reine Abfrage** über dieses Paar — kein Qt, kein Dialog, kein Zustand außerhalb:
  `Proceed` (nichts zu verlieren) oder `AskFirst` (ungesichert). Damit ist der Kern des §6-Schnitts
  port-frei testbar.
- **„Speichern" nutzt den gemerkten Pfad und denselben Use-Case** — `saveProject`, kein zweiter
  Schreibpfad (§9 R1). Ohne gemerkten Pfad verhält es sich wie „Speichern unter…".

**Zwei- vs. dreiwertig, präzise:** das **Verdikt** ist zweiwertig (`Proceed`/`AskFirst`). Die **Antwort
des Benutzers** ist dreiwertig (speichern / verwerfen / abbrechen) und lebt im Composition-Root, der sie
in „Aktion ausführen" oder „Aktion **unterlassen**" übersetzt. Genau dieses Unterlassen ist §9 R3.

## 3. Anforderungs-Ebene — **entschieden** (Projektinhaber, 2026-07-26)

Die Ungesichert-Warnung hat **heute keine Anforderung**:
[`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) beschreibt das
Speichern selbst, [`LH-QA-005`](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery) den Absturz.
Damit stand dieselbe Frage an wie bei
[`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) in slice-047: **ohne
Kriterium gibt es kein Maß für „erfüllt"**.

> **Entschieden: AK-Ergänzung an [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)**
> (Muster [slice-048a](../done/slice-048a-drw-001-fangpunkte-ak-spec.md) / slice-047). Die Alternative
> „reine UX-Härtung ohne Lastenheft-Berührung" ist **verworfen**: der Punkt kam aus einer
> **Validation**, also aus der Bedarfs-Ebene — genau dort gehört er verankert. slice-047 hat
> vorgeführt, was das Auslassen kostet: die Traceability-Korrektur war für die AK-lose Anforderung
> nachträglich nicht belegbar.

**Warum der Lastenheft-Text hier steht und nicht schon drin ist.** Die Aufnahme ins Lastenheft ist
**Ausführung dieses Slice**, nicht seine Planung: sie braucht den Lifecycle-Schritt nach `in-progress/`
und das vorgeschaltete
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
([`AGENTS.md` §5](../../../../AGENTS.md)). Eine Spec-Änderung ohne laufenden Slice hätte keinen Anker.
Der Text ist darum hier **ausformuliert** — die Entscheidung ist gefallen, nur der Vollzug wartet.

**Vorlage (lösungsfrei nach [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei);
beim Vollzug Header-Version + Historie nachziehen, [MR-010](../../../../harness/conventions.md)):**

```markdown
- **Happy Path:** Given ein Projekt mit bekannter Projektdatei, when „Speichern",
  then wird ohne erneute Ziel-Abfrage in genau diese Datei geschrieben; ohne
  bekannte Datei wird einmalig nach dem Ziel gefragt und es ist danach bekannt.
- **Boundary:** Given ungesicherte Änderungen, when eine Aktion den Sitzungs-Stand
  verwerfen würde (anderes Projekt öffnen, Programm beenden), then wird der Benutzer
  vorher gefragt und kann speichern, verwerfen oder die Aktion abbrechen; ohne seine
  Entscheidung geht kein Stand verloren.
```

Bewusst **nicht** im Lastenheft-Text: wie der Zustand geführt wird, welches Ereignis ihn setzt oder
zurücksetzt, und wie die Rückfrage dargestellt wird — das ist Lösungsmechanik und gehört in
[`spec/spezifikation.md`](../../../../spec/spezifikation.md) bzw. in die ADRs. Das **§1-Mapping**
([`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b`) entsteht darum erst mit der Umsetzung, wenn die Mechanik steht — nicht vorab.

## 4. Bewusst NICHT Teil

- **GUI-Export** und **Zuletzt-geöffnet-Liste** — die beiden anderen Rest-Punkte der
  slice-047-Validation. Sie hängen **nicht** am Sitzungs-Zustand und würden den Schnitt sprengen;
  eigene Slices.
- **Projektversionierung** ([`LH-FA-BLD-004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung))
  und **Undo/Redo** ([`LH-QA-003`](../../../../spec/lastenheft.md#lh-qa-003--undoredo)) — „ungesichert"
  ist eine **Ja/Nein**-Eigenschaft der Sitzung, keine Historie. Die Abgrenzung ist im Lastenheft schon
  als offen vermerkt und wird hier **nicht** aufgelöst.
- **Auto-Save / Wiederherstellungs-Datei.** Anderer Mechanismus, anderes Risiko.

## 5. Wirkungs-Breite — **entschieden: jetzt bauen, nicht später**

Die **einzige** Modell-Mutation, die dem GUI-Benutzer heute offensteht, ist das Zeichnen von
Hilfslinien ([ADR-0019](../../adr/0019-drw-2d-canvas.md) v1 — so auch im
[slice-047-Verify](../../../reviews/2026-07-25-slice-047-verify.md) §3 vermerkt). Die
Ungesichert-Warnung schützt also **vorerst genau eine Art von Änderung**. Das ist wenig, und der Slice
darf seinen Wert nicht überzeichnen.

**Trotzdem jetzt — zwei belastbare Gründe, nicht ein Gefühl:**

1. **Der Schutz ist mutator-agnostisch, wenn er am
   [`ModelChangedPort`](../../../../src/hexagon/ports/driven/model_changed_port.h) hängt.** Er greift
   für jede **künftige** Mutation automatisch, ohne dass ein einziger neuer Mutator daran denken muss.
   Wird er erst nachgerüstet, wenn das interaktive Bauteil-Zeichnen existiert, ist die Frage nicht mehr
   „ein Beobachter", sondern „jeder Mutator einzeln nachgeprüft" — dieselbe Klasse Nacharbeit, die den
   [`next_*_id_`](../done/slice-047-projekt-oeffnen.md)-Reset in slice-047 teuer gemacht hat.
2. **Die Kosten sind heute am niedrigsten.** Ein Beobachter über einen Port mit **einem** Produzenten
   ist trivial zu belegen; mit zehn Produzenten ist er ein Regressions-Feld.

**Konsequenz für den Zuschnitt:** der Slice baut den Schutz **generisch** (Beobachter am Port), nicht
speziell für Hilfslinien. Ein hilfslinien-spezifisches Flag wäre die billigere, aber falsche Lösung —
sie müsste beim ersten weiteren Mutator wieder weg.

## 6. Orakel-Schnitt — **vorab entschieden** (nicht erst bei der Verifikation)

Die Rückfrage beim **Fenster-Schließen** hängt am Qt-Schließ-Ereignis und damit am modalen,
**sensorlosen** Teil ([slice-047](../done/slice-047-projekt-oeffnen.md) B5). slice-047 hat gezeigt, was
passiert, wenn man diesen Schnitt der Verifikation überlässt: ein blockierendes Finding und ein Umbau
nach der Implementierung. Deshalb steht er **hier**, vor dem Start:

**Orakel-Pflicht (testbar außerhalb des coverage-ausgenommenen `main`):**

| Zusicherung | Diskriminierende Gegenprobe |
|---|---|
| nach einer Mutation ist die Sitzung **ungesichert** | Beobachter abgemeldet ⇒ rot |
| nach erfolgreichem Speichern ist sie **sauber** | Rücksetzen entfernt ⇒ rot |
| nach dem Öffnen ist sie **sauber** (`ModelReplaced` setzt zurück) | Rücksetz-Fall vergessen ⇒ rot |
| nach **gescheitertem** Speichern bleibt sie **ungesichert** | Rücksetzen vor statt nach dem Schreiben ⇒ rot |
| „Speichern" schreibt in die **gemerkte** Datei (Round-Trip über das echte Repository) | Pfad nicht gemerkt ⇒ rot |
| die Frage **„darf dieser Stand verworfen werden?"** liefert ein Verdikt (verwerfen / fragen) | Verdikt-Bildung entfernt ⇒ rot |

Die letzte Zeile ist der Kern des Schnitts: **die Entscheidung ist eine reine Abfrage über den
Sitzungs-Zustand** und damit port-frei testbar. Was am Schließ-Ereignis hängt, ist nur noch *„Verdikt
holen → Dialog zeigen → Antwort zurückgeben"*.

**Benannte Grenze (bleibt ohne Sensor, bewusst):** der modale Dialog selbst und die Anbindung ans
Qt-Schließ-Ereignis. Bedingung dafür — und das ist die harte Auflage dieses Slice: **in der
Verdrahtung darf keine Entscheidung stehen.** Kein „wenn ungesichert dann…" im Ereignis-Handler; der
Handler fragt das Verdikt ab und führt aus. Wer diese Auflage bricht, reproduziert Finding B4.

## 7. Definition of Done

- [ ] **`ProjectSession` im Kern** (`src/hexagon/services/`, framework-frei, `ModelChangedPort`-Beobachter):
      `isDirty()` · `path()` (optional) · `markPersisted(path)` (Öffnen **und** Speichern) ·
      `verdictForDiscard()` → `Proceed`/`AskFirst`. `ModelReplaced` **setzt zurück**, jede andere Meldung
      setzt dirty.
- [ ] **Orakel: alle sechs Zeilen der §6-Tabelle** als Unit-/Adapter-Tests **außerhalb** des
      coverage-ausgenommenen `main`, **je einmal als diskriminierend belegt** (Gegenprobe rot, im
      Closure-Text protokolliert).
- [ ] **„Speichern" (neue Menü-Aktion)** nutzt `session.path()` + `services::saveProject`; ohne gemerkten
      Pfad fällt es auf die Ziel-Abfrage zurück. „Speichern unter…" bleibt unverändert und **setzt** den
      Pfad. **Kein** zweiter Schreibpfad am Use-Case vorbei (§9 R1).
- [ ] **Rückfrage vor Sitzungs-Verlust** an **beiden** Auslösern (Öffnen, Fenster schließen): der Handler
      holt das Verdikt und führt aus; bei `AskFirst` entscheidet der Benutzer speichern/verwerfen/**abbrechen**,
      und Abbrechen **unterlässt** die auslösende Aktion (§9 R3). **In der Verdrahtung steht keine
      Entscheidung** — die harte §6-Auflage.
- [ ] **Lastenheft:** die §3-Vorlage in
      [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) aufgenommen,
      Header-Version + [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md) nachgezogen
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).
- [ ] **Spezifikation §1** neuer Block [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b` mit der dann feststehenden Mechanik
      (Sitzungs-Zustand, Rücksetz-Regel, Verdikt) + Zeile in
      [`spezifikation-historie.md`](../../../../spec/spezifikation-historie.md).
- [ ] **Benutzerhandbuch** ([Abschnitt 4.3](../../../user/benutzerhandbuch.md)) nachgeführt — er benennt
      **heute beide Grenzen ausdrücklich als fehlend**; sie müssen mit der Lieferung verschwinden.
      Handbuch-Version + Änderungshistorie mitziehen. (**`docs/user/` steht in dieser DoD-Zeile** — die
      Lehre aus slice-047 V1.)
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **`make gates` grün**; `make schema-check` byte-unberührt (**keine** Schema-Änderung — der
      Sitzungs-Zustand ist **nicht** persistent).

## 8. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/services/project_session.{h,cpp}` | neu | der Sitzungs-Zustand + Verdikt (§2); `ModelChangedPort`-Beobachter, framework-frei |
| `src/hexagon/services/manage_project.{h,cpp}` | ändern | erfolgreiches Öffnen/Speichern meldet den Pfad an die Sitzung (**nach** dem Erfolg, R2) |
| `src/main.cpp` | ändern | Menü-Aktion **Speichern**; beide Verlust-Auslöser holen das Verdikt; Dialog + Antwort-Übersetzung (**keine** Entscheidung hier) |
| `tests/hexagon/test_project_session.cpp` | neu | die §6-Zeilen 1–4 + 6 (Zustand + Verdikt, port-frei) |
| `tests/adapters/test_project_open_handler.cpp` | ändern | §6-Zeile 5 (Round-Trip in die **gemerkte** Datei, echtes Repository) |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Aufnahme (§3-Vorlage) + Header-Version |
| `spec/spezifikation.md`, `spec/spezifikation-historie.md` | ändern | §1-Mechanik-Block + Provenance-Zeile |
| `docs/user/benutzerhandbuch.md` | ändern | die beiden „fehlt noch"-Hinweise in 4.3 auflösen |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |

**Nicht berührt:** `data-model.yaml`/`schema.sql` (kein persistenter Zustand), `.d-check.yml`/`.a-check.yml`
(keine Gate-Änderung — die Kern-Platzierung ist gerade der Weg **ohne** neue Kante), ADR-Index (keine neue
Grundsatz-Entscheidung).

## 9. Verbleibende Risiken

- **R1 — „Speichern" ohne Dialog ist ein neuer Datenverlust-Pfad.** Es überschreibt eine bestehende
  Datei **ohne Rückfrage** (genau das ist der Zweck). Die Atomarität aus
  [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) trägt das bereits
  — der Slice darf sie nicht umgehen (**kein** direkter Schreibpfad am Use-Case vorbei).
- **R2 — Reihenfolge beim Rücksetzen.** „Sauber" darf erst gelten, **nachdem** das Schreiben
  erfolgreich zurückgekehrt ist. Die Zeile 4 der Orakel-Tabelle ist genau dafür da; sie ist die einzige
  im Satz, die einen **Fehler**-Pfad prüft.
- **R3 — Abbrechen ist ein dritter Ausgang.** „Speichern / verwerfen / abbrechen" heißt: die
  auslösende Aktion (Öffnen, Schließen) muss **unterbleiben** können. Ein Zwei-Wege-Verdikt
  (ja/nein) reicht nicht — das fällt sonst erst im Benutzer-Test auf.

## 10. Trigger

- **Validations-Rest** aus der [slice-047](../done/slice-047-projekt-oeffnen.md)-Closure (2026-07-26,
  Rolle Projektinhaber). Kein Gate-Befund, kein Review-Finding — ein **Bedarfs**-Befund.

## 11. Closure-Trigger

- **Alle sechs Zeilen der §6-Orakel-Tabelle** grün **und** je einmal als diskriminierend belegt
  (Gegenprobe rot) — die Tabelle ist der Maßstab, an dem die Verifikation misst.
- **Lastenheft** um die §3-Vorlage ergänzt, Header-Version + Historie nachgezogen
  ([MR-010](../../../../harness/conventions.md)); §1-Mapping [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b` mit der dann
  feststehenden Mechanik.
- **Handbuch-Abschnitt 4.3** nachgeführt — er benennt beide Grenzen heute ausdrücklich als fehlend
  („kein Speichern auf die offene Datei", „keine Warnung vor ungesicherten Änderungen"); **die
  Doku-DoD-Zeile führt `docs/user/`** (Lehre aus slice-047 V1).
- `make gates` grün; Closure-Notiz mit der §6-Auflage („keine Entscheidung in der Verdrahtung")
  beantwortet.

## 12. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** mittel. **Phase-Reife:** die Persistenz-Mechanik ist reif (welle-1) und der
  Aufruf-Pfad seit slice-047 vorhanden — dieser Slice ergänzt **Sitzungs-Zustand**, keine neue Mechanik.
- **Risiko:** mittel — Datenverlust-nah (§9 R1/R2), und der schwerste Pfad liegt im sensorlosen
  Fenster-Ereignis. Letzteres ist durch den vorab entschiedenen §6-Schnitt entschärft, **nicht**
  beseitigt: die benannte Grenze bleibt.

## 13. Closure-Notiz

_(bei Ausführung auszufüllen)_
