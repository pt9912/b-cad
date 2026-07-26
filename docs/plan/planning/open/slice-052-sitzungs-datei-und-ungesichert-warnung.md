---
id: slice-052
titel: Sitzungs-Datei merken + Warnung vor ungesicherten Änderungen (GUI-Datenverlust-Schutz)
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen), [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 052: Sitzungs-Datei merken + Ungesichert-Warnung

**Status:** open — **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
2026-07-26 gelaufen: 2 HIGH / 7 MEDIUM / 5 LOW / 2 INFO → *nicht startbar***
([Report](../../../reviews/2026-07-26-slice-052-plan.md)). **Alle Findings eingearbeitet** (s. §13);
die beiden HIGH führten zu einem **Wechsel des Kern-Mechanismus** (§2) und zu einer korrigierten
Anforderungs-Lage (§3). **Ein zweites, unabhängiges
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start** — die Auflösung der HIGHs ist neu und ungeprüft.

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
- **Sie weiß, ob sie ungesichert ist** → **jede** Aktion, die den Sitzungs-Stand verwirft (neues Projekt,
  anderes Projekt öffnen, Fenster schließen), fragt vorher nach.

## 2. Lösung — **entschieden** (Projektinhaber 2026-07-26, nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-HIGH-1)

> **Der ursprüngliche Entwurf ist verworfen.** Er wollte den »verändert«-Zustand aus
> `ModelChangedPort`-Meldungen ableiten. Das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) hat belegt, dass das **für keine heute erreichbare
> Benutzer-Mutation** funktioniert: `addGuideLine`/`removeGuideLine`
> (`src/hexagon/services/structure_edit_service.cpp`:1347), alle Layer-Mutatoren und alle
> Material-Mutatoren melden **nichts** — „kein op" ist die ausdrückliche Entscheidung 2 der
> **`Accepted`**-[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md). Da Hilfslinien die **einzige**
> GUI-Mutation sind (§5), wäre die Warnung nie ausgelöst worden, **während alle Orakel-Zeilen grün
> stehen**. Genau dafür ist [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) da.

**Der Zustand kommt aus dem Vergleich, nicht aus Meldungen.**

Die Sitzung hält den **zuletzt geschriebenen Stand** und vergleicht:

```
markPersisted(pfad, building)    // nach erfolgreichem Öffnen UND nach erfolgreichem Speichern
isDirty(current) := current != last_persisted_
```

Warum das trägt, wo der Beobachter scheitert:

- **Wirklich mutator-agnostisch — strukturell, nicht per Konvention.** Es gibt keine Meldung, die ein
  Mutator vergessen könnte, und keine `op`-Liste, die gepflegt werden müsste. Jede heutige **und**
  jede künftige Mutation ist erfasst, weil der Vergleich am **Ergebnis** ansetzt.
- **Kein Vertrag wird berührt.** Kein neuer `op`, keine neue Kante, kein Port: die
  [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Entscheidung „kein op" bleibt gültig, statt
  umgangen oder per Folge-ADR aufgehoben zu werden. Damit entfällt auch die ganze
  `ui_command`-vs-Kern-Abwägung des Vorentwurfs ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MEDIUM-4): der Träger implementiert **keinen**
  Port mehr.
- **Semantisch genauer.** „Zurück-geändert auf den Dateistand" ist wieder **sauber** — beim
  Meldungs-Zähler wäre es fälschlich schmutzig geblieben.

**Ort: der Kern** (`src/hexagon/services/`), framework-frei. Jetzt aus einem **einfacheren** Grund als
im Vorentwurf: der Träger vergleicht `model::Building`-Werte und kennt sonst nichts — er ist Kern-Logik.
Und `main.cpp` scheidet weiter aus, weil dort Liegendes **orakel-los per Konstruktion** ist
([slice-047](../done/slice-047-projekt-oeffnen.md) §7 — der Plan liegt in `done/`).

**Preis, offen benannt:** `operator==` auf den Modell-Werttypen existiert **noch nicht** und muss
angelegt werden (mechanisch, ~10 Typen) — mit dem Risiko aus §9 R1. Dazu eine Modell-Kopie im
Speicher; der Vergleich läuft nur bei Verwerf-Aktionen, also selten.

**Zwei- vs. dreiwertig, präzise:** das **Verdikt** ist zweiwertig (`Proceed`/`AskFirst`). Die **Antwort
des Benutzers** ist dreiwertig (speichern / verwerfen / abbrechen). Damit die dreiwertige Antwort nicht
im orakel-losen `main` interpretiert wird, ist auch **ihre Auswertung** eine Kern-Funktion (§6, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
MEDIUM-3): sie bildet Antwort + Verdikt auf **„Aktion ausführen" / „Aktion unterlassen" / „vorher
speichern"** ab. `main` fragt, zeigt den Dialog, reicht die Antwort zurück und **führt aus** —
es **entscheidet** nichts.

**Ebenso die Ziel-Wahl beim Speichern** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-2): „bekannter Pfad → dorthin, sonst fragen" ist
eine Kern-Abfrage über die Sitzung, kein `if` im Menü-Handler.

## 3. Anforderungs-Ebene — **korrigiert** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-HIGH-2)

> **Die frühere Aussage „die Ungesichert-Warnung hat heute keine Anforderung" war falsch.**
> [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) trägt sie bereits auf
> **AK-Niveau**: „*Boundary: Given ein bereits geöffnetes, ungespeichertes Projekt, when „Neues Projekt",
> then Rückfrage „Änderungen verwerfen?" vor dem Anlegen.*" Die Anforderung existiert — sie ist nur
> **unerfüllt**, und zwar doppelt: es gibt weder die Rückfrage **noch** die Aktion „Neues Projekt".

Damit verschiebt sich die Arbeit an der Anforderungs-Ebene:

| Anforderung | Auslöser | Lage | Arbeit in diesem Slice |
|---|---|---|---|
| [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) | „Neues Projekt" | **AK existiert**, unerfüllt (Aktion fehlt ganz) | **erfüllen**, keine AK-Änderung |
| [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) | „Speichern" · „Programm beenden" | Happy-AK kennt kein „in die bekannte Datei"; kein Verwerf-Schutz beim Beenden | **AK ergänzen** (Vorlage unten) |
| [`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) | „Öffnen" | seit slice-047 AK-tragend, aber ohne Verwerf-Schutz | **AK ergänzen** (Vorlage unten) |

**Ein zweiter Befund fällt dabei ab, und er ist nicht klein:**
[`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) ist **nicht
benutzer-erfüllbar** — eine Aktion „Neues Projekt" existiert in `src/main.cpp` nicht. Das ist **exakt
dieselbe Klasse**, die slice-047 für BLD-002/003 aufgearbeitet hat: Mechanik ohne Aufruf-Pfad. Der
Projektinhaber hat entschieden, den Auslöser **mitzunehmen** statt ihn erneut zu vertagen (§4).

**Vorlage (lösungsfrei nach [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei);
beim Vollzug Header-Version + Historie nachziehen,
[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).**
Bewusst **nicht** dupliziert, was BLD-001 schon sagt — dort steht die Rückfrage bereits:

```markdown
LH-FA-BLD-002, zusätzlich:
- **Happy Path:** Given ein Projekt mit bekannter Projektdatei, when „Speichern",
  then wird ohne erneute Ziel-Abfrage in genau diese Datei geschrieben; ohne
  bekannte Datei wird einmalig nach dem Ziel gefragt und sie ist danach bekannt.
- **Boundary:** Given ungesicherte Änderungen, when „Programm beenden",
  then Rückfrage vor dem Beenden; der Benutzer kann speichern, verwerfen oder
  die Aktion abbrechen — ohne seine Entscheidung geht kein Stand verloren.

LH-FA-BLD-003, zusätzlich:
- **Boundary:** Given ungesicherte Änderungen, when „Öffnen",
  then Rückfrage vor dem Ersetzen des Arbeitsstands (speichern / verwerfen /
  abbrechen), analog LH-FA-BLD-001.
```

**Warum der Text hier steht und nicht schon im Lastenheft:** die Aufnahme ist **Ausführung** dieses
Slice, nicht seine Planung — sie braucht den Lifecycle-Schritt nach `in-progress/` und das
vorgeschaltete [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
([`AGENTS.md` §5](../../../../AGENTS.md)). Eine Spec-Änderung ohne laufenden Slice hätte keinen Anker.

Das **§1-Mapping** ([`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b`)
entsteht mit der Umsetzung, wenn die Mechanik steht — dort, und **nur** dort, lebt die Lösungsmechanik
(wie der Zustand geführt wird, wie verglichen wird, wie das Verdikt entsteht).

## 4. Bewusst NICHT Teil

**Neu dabei (war vorher nicht Teil):** die Aktion **„Neues Projekt"** — Projektinhaber-Entscheidung
2026-07-26 nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-HIGH-2. Sie nutzt dieselbe Verdikt-Maschinerie wie Öffnen/Schließen, schließt den
dritten von [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) geforderten
Auslöser und macht die Anforderung erstmals benutzer-erfüllbar.

Nicht Teil bleibt:

- **GUI-Export** und **Zuletzt-geöffnet-Liste** — die beiden anderen Rest-Punkte der
  slice-047-Validation. Sie hängen **nicht** am Sitzungs-Zustand; eigene Slices.
- **Projektversionierung** ([`LH-FA-BLD-004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung))
  und **Undo/Redo** ([`LH-QA-003`](../../../../spec/lastenheft.md#lh-qa-003--undoredo)) — „ungesichert"
  ist eine **Ja/Nein**-Eigenschaft der Sitzung, keine Historie.
- **Auto-Save / Wiederherstellungs-Datei** — dafür existiert eine **eigene** Anforderung
  ([`LH-QA-004`](../../../../spec/lastenheft.md#lh-qa-004--autosave), automatische Sicherung; [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  LOW-4). Anderer Mechanismus, anderes Risiko, eigener Slice.
- **Kein neuer `op`, keine [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Korrektur.** Der Vergleichs-Ansatz (§2) braucht beides nicht.

## 5. Wirkungs-Breite — **entschieden: jetzt bauen, nicht später**

Die **einzige** Modell-Mutation, die dem GUI-Benutzer heute offensteht, ist das Zeichnen von
Hilfslinien ([ADR-0019](../../adr/0019-drw-2d-canvas.md) v1 — so auch im
[slice-047-Verify](../../../reviews/2026-07-25-slice-047-verify.md) §3 vermerkt). Der Schutz greift
also **vorerst für eine Art von Änderung**. Der Slice darf seinen Wert nicht überzeichnen.

**Trotzdem jetzt — und nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-HIGH-1 mit anderem Argument als zuvor:**

1. **Der Vergleichs-Ansatz ist mutator-agnostisch, ohne dass jemand daran denken muss.** Er erfasst
   heutige und künftige Mutationen strukturell. Der frühere Beobachter-Ansatz hätte genau das
   *behauptet* und nicht geleistet — HIGH-1 ist der Beleg, wie leicht diese Klasse Zusage bricht.
2. **Die Kosten steigen mit dem Modell, nicht mit der Zeit.** `operator==` heute über ~10 Werttypen ist
   mechanisch; nach weiteren Bauteil-Familien ist es dieselbe Arbeit plus Nachrüst-Risiko (§9 R1).

**Konsequenz für den Zuschnitt:** generisch bauen, kein hilfslinien-spezifisches Flag.

## 6. Orakel-Schnitt — **vorab entschieden** (nicht erst bei der Verifikation)

slice-047 hat gezeigt, was passiert, wenn man diesen Schnitt der Verifikation überlässt: ein
blockierendes Finding und ein Umbau nach der Implementierung. Deshalb steht er **vor** dem Start —
erweitert um die vier Lücken, die [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) in der ersten Fassung gefunden hat.

**Orakel-Pflicht (testbar außerhalb des coverage-ausgenommenen `main`):**

| # | Zusicherung | Diskriminierende Gegenprobe | Herkunft |
|---|---|---|---|
| 1 | **Hilfslinie zeichnen ⇒ Sitzung ungesichert** (die Mutation, die **keinen** `op` meldet) | Vergleich durch einen Meldungs-Zähler ersetzt ⇒ rot | **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) HIGH-1** |
| 2 | eine **op-tragende** Mutation (Wand) ⇒ ungesichert | Vergleich entfernt ⇒ rot | — |
| 3 | nach erfolgreichem **Speichern** ⇒ sauber | Rücksetzen entfernt ⇒ rot | — |
| 4 | nach **Öffnen** ⇒ sauber | `markPersisted` im Öffnen-Pfad entfernt ⇒ rot | — |
| 5 | nach **gescheitertem** Speichern ⇒ **weiter** ungesichert | Rücksetzen vor statt nach dem Schreiben ⇒ rot | — |
| 6 | **zurück-geändert auf den Dateistand** ⇒ wieder sauber | Vergleich durch ein Einweg-Flag ersetzt ⇒ rot | §2 |
| 7 | **frische Sitzung ohne Benutzer-Änderung** ⇒ `Proceed` (keine Rückfrage beim ersten Schließen) | Anfangszustand „ungesichert" ⇒ rot | **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-1** |
| 8 | **Ziel-Wahl beim Speichern**: bekannter Pfad ⇒ „dorthin"; unbekannt ⇒ „fragen" | Zweig vertauscht ⇒ rot | **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-2** |
| 9 | „Speichern" schreibt in die **gemerkte** Datei (Round-Trip über das echte Repository) | Pfad nicht gemerkt ⇒ rot | — |
| 10 | **Verdikt** über den Sitzungs-Zustand: `Proceed` / `AskFirst` | Verdikt-Bildung entfernt ⇒ rot | — |
| 11 | **Antwort-Auswertung**: „abbrechen" ⇒ *Aktion unterlassen*; „verwerfen" ⇒ *ausführen*; „speichern" ⇒ *erst speichern, dann ausführen* | „abbrechen" führt aus ⇒ rot | **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-3** |

Zeile 1 ist die wichtigste des Slice: sie ist der **Regressions-Schutz gegen genau den Fehler**, den
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) gefunden hat. Zeile 11 zieht die dreiwertige Antwort aus dem orakel-losen `main` heraus (§2).

**Benannte Grenze (bleibt ohne Sensor, bewusst — und jetzt vollständig aufgezählt):** der modale
`QMessageBox`/`QFileDialog` selbst, die Anbindung ans **Qt-Schließ-Ereignis** und die Menü-Verdrahtung
der drei Auslöser. Bedingung — die harte Auflage dieses Slice: **in der Verdrahtung steht keine
Entscheidung.** Kein „wenn ungesichert dann…", kein „wenn Pfad bekannt dann…" im Handler; er fragt ab
und führt aus. Wer die Auflage bricht, reproduziert
[slice-047](../done/slice-047-projekt-oeffnen.md)-Finding B4.

## 7. Definition of Done

- [ ] **Gleichheit auf den Modell-Werttypen** (`src/hexagon/model/`): `operator==` über **alle** Felder
      der von `Building` gehaltenen Typen. **Vollständigkeits-Orakel** gegen §9 R1 (ein vergessenes Feld
      = stiller Datenverlust): je Typ ein Test, der **jedes** Feld einzeln verändert und Ungleichheit
      erwartet.
- [ ] **`ProjectSession` im Kern** (`src/hexagon/services/`, framework-frei, **kein** Port):
      `markPersisted(path, building)` · `path()` (optional) · `isDirty(current)` ·
      `verdictForDiscard(current)` → `Proceed`/`AskFirst` · `saveTarget()` → bekannter Pfad / Ziel-Abfrage ·
      Auswertung der dreiwertigen Antwort → ausführen / unterlassen / erst speichern.
- [ ] **Orakel: alle elf Zeilen der §6-Tabelle**, **je einmal als diskriminierend belegt** (Gegenprobe
      rot, im Closure-Text protokolliert). **Zeile 1 ist Pflicht** — sie ist der Regressions-Schutz gegen
      [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-HIGH-1.
- [ ] **Menü-Aktion „Speichern"** nutzt `saveTarget()` + `services::saveProject`; „Speichern unter…"
      bleibt und **setzt** den Pfad. **Kein** zweiter Schreibpfad am Use-Case vorbei (§9 R3).
- [ ] **Menü-Aktion „Neu"** ([`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)
      Happy Path): leeres Projekt mit genau **einem** Geschoss (Default-Höhe aus der Spezifikation),
      leerer Modellbaum — und die von der Boundary geforderte Rückfrage davor.
- [ ] **Rückfrage vor Sitzungs-Verlust an allen drei Auslösern** (Neu, Öffnen, Fenster schließen): der
      Handler holt Verdikt + Antwort-Auswertung und führt aus; **Abbrechen unterlässt** die auslösende
      Aktion. **In der Verdrahtung steht keine Entscheidung** (§6-Auflage).
- [ ] **Lastenheft:** die §3-Vorlagen in
      [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) und
      [`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) aufgenommen;
      **BLD-001 unverändert** (die AK existiert bereits). Header-Version +
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md) nachgezogen.
- [ ] **Spezifikation §1** neuer Block
      [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b` (Sitzungs-Zustand,
      Vergleichs-Semantik, Verdikt, Rücksetz-Regel) + Zeile in
      [`spezifikation-historie.md`](../../../../spec/spezifikation-historie.md).
- [ ] **`spec/architecture.md`** §2.1-Verzeichnisbaum um den neuen Kern-Service ergänzt ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
      MEDIUM-5 — der Baum zählt die Services heute **vollständig** auf; die Klasse F4 aus slice-047 nicht
      wiederholen).
- [ ] **Benutzerhandbuch** an **beiden** Stellen nachgeführt ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-6):
      [Abschnitt 4.3](../../../user/benutzerhandbuch.md) **und** die **FAQ** — beide behaupten heute, es
      gebe kein „Speichern" auf die offene Datei. Neu zu beschreiben: „Neu"/„Speichern" + die Rückfrage.
      Handbuch-Version + Änderungshistorie mitziehen. (**`docs/user/` steht in dieser DoD-Zeile** — die
      Lehre aus slice-047 V1.)
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) INFO-1);
      `make schema-check` byte-unberührt (**keine** Schema-Änderung — der Sitzungs-Zustand ist **nicht**
      persistent).

## 8. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/model/*.h` (Werttypen) | ändern | `operator==` über alle Felder — Grundlage des Vergleichs (§2) |
| `src/hexagon/services/project_session.{h,cpp}` | neu | Sitzungs-Zustand, Verdikt, Ziel-Wahl, Antwort-Auswertung (§2) |
| `src/hexagon/services/manage_project.{h,cpp}` | ändern | erfolgreiches Öffnen/Speichern meldet Pfad **+ Stand** an die Sitzung (**nach** dem Erfolg, §9 R2) |
| `src/main.cpp` | ändern | Menü **Neu**/**Speichern**; drei Verlust-Auslöser holen Verdikt + Auswertung; Dialog (**keine** Entscheidung hier) |
| `src/hexagon/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | beide Listen zählen Dateien **explizit** auf ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) LOW-3) |
| `tests/hexagon/test_model_equality.cpp` | neu | §7-Vollständigkeits-Orakel je Feld (§9 R1) |
| `tests/hexagon/test_project_session.cpp` | neu | §6-Zeilen 1–8, 10, 11 (Zustand, Verdikt, Ziel-Wahl, Antwort) |
| `tests/adapters/test_project_open_handler.cpp` | ändern | §6-Zeile 9 (Round-Trip in die **gemerkte** Datei, echtes Repository) |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Aufnahme BLD-002/003 + Header-Version |
| `spec/spezifikation.md`, `spec/spezifikation-historie.md` | ändern | §1-Mechanik-Block + Provenance-Zeile |
| `spec/architecture.md` | ändern | Kern-Service im §2.1-Baum ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-5) |
| `docs/user/benutzerhandbuch.md` | ändern | Abschnitt 4.3 **und** FAQ ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-6) |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |
| `docs/reviews/`-Report zum zweiten Plan-Review | neu | das **zweite** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) LOW-5) |

**Nicht berührt:** `data-model.yaml`/`schema.sql` (kein persistenter Zustand), `.d-check.yml`/`.a-check.yml`
(keine Gate-Änderung — der Vergleichs-Ansatz braucht **keine** neue Kante und **keinen** Port),
`docs/plan/adr/` (s. §10).

## 9. Verbleibende Risiken

- **R1 — unvollständige Gleichheit ist stiller Datenverlust.** Vergisst `operator==` ein Feld, gilt eine
  Änderung genau dieses Feldes als „nicht verändert" — die Warnung bleibt aus, der Stand geht verloren.
  Das ist **die** Achillesferse des gewählten Ansatzes und der Grund für das Feld-für-Feld-Orakel in §7.
  Ebenso bei **neuen** Feldern künftiger Slices: der Test muss mitwachsen.
- **R2 — Reihenfolge beim Rücksetzen.** „Sauber" darf erst gelten, **nachdem** das Schreiben erfolgreich
  zurückgekehrt ist (§6-Zeile 5 prüft genau das — die einzige Zeile auf einem Fehlerpfad).
- **R3 — „Speichern" ohne Dialog ist ein neuer Datenverlust-Pfad.** Es überschreibt eine bestehende
  Datei **ohne Rückfrage** (das ist der Zweck). Die Atomarität aus
  [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) trägt das bereits —
  **kein** direkter Schreibpfad am Use-Case vorbei.
- **R4 — Kosten des Vergleichs.** Eine Modell-Kopie im Speicher und ein O(n)-Vergleich; er läuft nur bei
  Verwerf-Aktionen. Bei sehr großen Modellen ist das ein benannter Re-Eval (z. B. Inhalts-Hash statt
  Vollvergleich), **kein** Thema dieses Schnitts.
- **R5 — „Neues Projekt" berührt den Demo-Aufbau.** Der Composition-Root baut heute beim Start ein
  Demo-Modell (`buildAcc001KernDemo`); „Neu" erzeugt ein **leeres** Projekt mit einem Geschoss. Die
  Wechselwirkung mit dem Startzustand (§6-Zeile 7) ist beim Schnitt zu prüfen.

## 10. ADR-Abwägung ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) MEDIUM-7 — nicht verneint, sondern begründet)

Das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) hat zu Recht gerügt, dass die Vorfassung eine Datenheimat-Entscheidung fällte und zugleich
ohne Abwägung erklärte, der ADR-Index bleibe unberührt —
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Entscheidung 1 hat dieselbe **Frageklasse**
ADR-pflichtig behandelt.

**Abwägung, am Kriterium statt am Gefühl.** [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) entschied die Heimat von Daten, die
**persistiert, exportiert und von mehreren Adaptern gesehen** werden — eine Grundsatzfrage mit
Fernwirkung auf Schema, Ports und Formate. Der Sitzungs-Zustand dieses Slice ist das Gegenteil:
**nicht persistiert** (kein Schema), **nicht exportiert**, **kein neuer Port**, **keine neue
Schicht-Kante**, **kein Framework** — er lebt und stirbt mit einer Programm-Sitzung. Er ändert keinen
öffentlichen Vertrag; das Verhalten, das ein Benutzer sieht, wird von Lastenheft-AK + §1-Block
normiert, nicht von einer Architektur-Entscheidung.

**Ergebnis: keine ADR** — aber als **abgewogen** protokolliert, nicht als Nebensache abgetan. Fällt das
zweite [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) anders aus, ist die ADR vor dem Start zu schreiben.

## 11. Trigger

- **Validations-Rest** aus der [slice-047](../done/slice-047-projekt-oeffnen.md)-Closure (2026-07-26,
  Rolle Projektinhaber). Kein Gate-Befund, kein Review-Finding — ein **Bedarfs**-Befund.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) 2026-07-26** (2 HIGH) → Mechanismus-Wechsel + Aufnahme des dritten Auslösers.

## 12. Closure-Trigger

- **Alle elf Zeilen der §6-Orakel-Tabelle** grün **und** je einmal als diskriminierend belegt
  (Gegenprobe rot) — die Tabelle ist der Maßstab, an dem die Verifikation misst.
- **Lastenheft** um die §3-Vorlagen ergänzt, Header-Version + Historie nachgezogen; §1-Mapping mit der
  dann feststehenden Mechanik.
- **Handbuch an beiden Stellen** nachgeführt (4.3 **und** FAQ).
- `make gates` grün; Closure-Notiz beantwortet die §6-Auflage („keine Entscheidung in der Verdrahtung")
  und protokolliert die Gegenproben.

## 13. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (2026-07-26)

Report: [`2026-07-26-slice-052-plan.md`](../../../reviews/2026-07-26-slice-052-plan.md) —
**2 HIGH / 7 MEDIUM / 5 LOW / 2 INFO, Verdikt „nicht startbar"**.

| # | Behandlung |
|---|---|
| **HIGH-1** | **Mechanismus gewechselt** (§2): Vergleich mit dem zuletzt geschriebenen Stand statt Meldungs-Beobachtung. Am Artefakt bestätigt: `structure_edit_service.cpp`:1147/:1194/:1283/:1311/:1347 melden nichts, [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §2 ist `Accepted`. **§6-Zeile 1** ist der Regressions-Schutz. |
| **HIGH-2** | **§3 korrigiert**: [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) trägt die Rückfrage bereits als AK. Der dritte Auslöser „Neues Projekt" ist **in den Scope genommen** (Projektinhaber-Entscheidung) — inkl. des Befunds, dass BLD-001 heute nicht benutzer-erfüllbar ist. |
| **MEDIUM-1** | §6-Zeile **7** (frische Sitzung ⇒ `Proceed`) + §9 R5 (Wechselwirkung mit dem Demo-Aufbau). |
| **MEDIUM-2** | §6-Zeile **8**: die Ziel-Wahl ist eine **Kern-Abfrage**, kein `if` im Menü-Handler — damit diskriminierend prüfbar. |
| **MEDIUM-3** | §6-Zeile **11**: die Auswertung der dreiwertigen Antwort ist Kern-Funktion; „abbrechen ⇒ Aktion unterlassen" hat ein Orakel. Die benannte Grenze ist jetzt **vollständig aufgezählt**. |
| **MEDIUM-4** | **gegenstandslos geworden**: der Träger implementiert keinen Port mehr, die `ui_command`-Abwägung entfällt. Der Hinweis auf das port-freie Muster ([ADR-0019](../../adr/0019-drw-2d-canvas.md) Option A) war berechtigt. |
| **MEDIUM-5** | `spec/architecture.md` ist in DoD **und** Datei-Plan aufgenommen. |
| **MEDIUM-6** | Handbuch-**FAQ** zusätzlich zu 4.3 in DoD und Datei-Plan. |
| **MEDIUM-7** | **§10** neu: ADR-Frage abgewogen statt verneint, mit Kriterium und Ergebnis. |
| **LOW-1** | Falsches `.a-check.yml`-Zitat entfernt (der Absatz ist mit MEDIUM-4 ohnehin entfallen). |
| **LOW-2** | `adr_refs` um [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) + [ADR-0019](../../adr/0019-drw-2d-canvas.md) ergänzt; `lastenheft_refs` um BLD-001. |
| **LOW-3** | Beide `CMakeLists.txt` im Datei-Plan. |
| **LOW-4** | [`LH-QA-004`](../../../../spec/lastenheft.md#lh-qa-004--autosave) bei der Auto-Save-Abgrenzung benannt. |
| **LOW-5** | Der [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report steht als Liefer-Artefakt im Datei-Plan. |
| **INFO-1** | Ruhe-Marker-Toggle in der DoD-Gates-Zeile vermerkt. |
| **INFO-2** | **gegenstandslos geworden**: die Rücksetzung hängt nicht mehr an der `ModelReplaced`-Meldung, sondern am Persistenz-Ereignis (`markPersisted`) — genau die Bindung, die INFO-2 als robuster benannt hat. |

**Lehre:** der Plan hatte den Orakel-Schnitt vorab entschieden — die richtige Lehre aus slice-047 — und
trotzdem an der entscheidenden Stelle nichts genützt: die sechs Orakel-Zeilen der Vorfassung wären
**grün** gewesen, während die Funktion für den realen Benutzerweg tot war. Ein Orakel prüft, was man
gebaut hat, nicht ob der Mechanismus den Weg trifft. Was hier geholfen hat, war die **unabhängige
Prüfung gegen eine `Accepted`-ADR, die der Autor nicht kannte**.

## 14. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** mittel. **Phase-Reife:** die Persistenz-Mechanik ist reif (welle-1) und der
  Aufruf-Pfad seit slice-047 vorhanden — dieser Slice ergänzt **Sitzungs-Zustand**, keine neue Mechanik.
- **Risiko:** mittel-hoch — Datenverlust-nah, und **R1** (unvollständige Gleichheit) wirkt **still**.
  Der schwerste Pfad liegt weiter im sensorlosen Fenster-Ereignis; durch den §6-Schnitt entschärft,
  **nicht** beseitigt.

## 15. Closure-Notiz

_(bei Ausführung auszufüllen)_
