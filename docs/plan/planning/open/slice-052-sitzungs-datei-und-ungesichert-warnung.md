---
id: slice-052
titel: Sitzungs-Datei merken + Warnung vor ungesicherten Änderungen (GUI-Datenverlust-Schutz)
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md)]
---

# Slice 052: Sitzungs-Datei merken + Ungesichert-Warnung

**Status:** open — Scope-Reservierung aus der
[slice-047](../done/slice-047-projekt-oeffnen.md)-Validation. **Vier Fragen sind bereits entschieden**
(§3 Anforderungs-Ebene · §5 Wirkungs-Breite · §6 Orakel-Schnitt · §4 Abgrenzung); offen bleibt der
**Lösungsraum** (§2) und damit der Detail-Schnitt. Eigenes
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

## 2. Lösungsraum (offen — Entscheid beim Start; §3/§5/§6 sind bereits entschieden)

**Nicht** vorentschieden. Zwei Punkte sind aber schon jetzt belegbar und sollten den Schnitt leiten:

- **Der „verändert"-Zustand ist bereits beobachtbar.** Der
  [`ModelChangedPort`](../../../../src/hexagon/ports/driven/model_changed_port.h)
  ([ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)) meldet **jede** committete Mutation; ein
  Beobachter, der „seit dem letzten Schreiben kam mindestens eine Meldung" festhält, braucht **keine**
  neue Naht im Kern. Der `ModelReplaced`-Fall aus slice-047 muss den Zustand dabei **zurücksetzen**
  (ein frisch geöffnetes Projekt ist nicht verändert) — das ist der leicht zu übersehende Fall.
- **Wohin gehört der Zustand?** Genau die Frage, an der slice-047 sein blockierendes Verify-Finding
  hatte: was im coverage-ausgenommenen `main.cpp` landet, ist **orakel-los per Konstruktion**. „Welche
  Datei bin ich, bin ich verändert?" ist eine **Entscheidung**, keine Verdrahtung — sie gehört in eine
  testbare Naht (Muster: der `DrawingTargetSinks`-Rückweg aus
  [slice-047](../done/slice-047-projekt-oeffnen.md) §7). In `main` darf bleiben: Dialog, Meldung,
  Verdrahtung.

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

## 7. Verbleibende Risiken

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

## 8. Trigger

- **Validations-Rest** aus der [slice-047](../done/slice-047-projekt-oeffnen.md)-Closure (2026-07-26,
  Rolle Projektinhaber). Kein Gate-Befund, kein Review-Finding — ein **Bedarfs**-Befund.

## 9. Closure-Trigger

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

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** mittel. **Phase-Reife:** die Persistenz-Mechanik ist reif (welle-1) und der
  Aufruf-Pfad seit slice-047 vorhanden — dieser Slice ergänzt **Sitzungs-Zustand**, keine neue Mechanik.
- **Risiko:** mittel — Datenverlust-nah (§7 R1/R2), und der schwerste Pfad liegt im sensorlosen
  Fenster-Ereignis. Letzteres ist durch den vorab entschiedenen §6-Schnitt entschärft, **nicht**
  beseitigt: die benannte Grenze bleibt.

## 11. Closure-Notiz

_(bei Ausführung auszufüllen)_
