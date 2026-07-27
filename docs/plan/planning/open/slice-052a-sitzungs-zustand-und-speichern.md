---
id: slice-052a
titel: Sitzungs-Zustand + „Speichern" + Ungesichert-Rückfrage bei Öffnen/Beenden
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 052a: Sitzungs-Zustand + „Speichern" + Ungesichert-Rückfrage

**Status:** open — **drei [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe
gefahren, alle „nicht startbar"** ([Lauf 1](../../../reviews/2026-07-26-slice-052-plan.md): 2 HIGH /
7 MED / 5 LOW / 2 INFO · [Lauf 2](../../../reviews/2026-07-26-slice-052-plan-2.md): 1 HIGH / 11 MED /
4 LOW / 2 INFO · [Lauf 3](../../../reviews/2026-07-26-slice-052a-plan-3.md): 1 HIGH / 3 MED / 4 LOW /
2 INFO). Lauf 1 + 2 galten dem **ungeteilten** slice-052; Lauf 2 führte zum **Split**
([`slice-052b`](slice-052b-neues-projekt.md)), Lauf 3 zum **Struktur-Vorläufer**
[`slice-053`](../done/slice-053-fenster-als-adapter.md). Alle Findings sind eingearbeitet (§13).

**Abhängigkeit: [`slice-053`](../done/slice-053-fenster-als-adapter.md) muss zuerst laufen** — ohne das
Hauptfenster als Adapter-Klasse ist die Schließ-Rückfrage nicht orakel-fähig (Lauf-3-HIGH-1). **Ein
vierter [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
vor dem Start**, sobald 053 geliefert ist.

**Welle:** welle-5-erweiterung. **Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser (Validations-Rest slice-047, 2026-07-26)

Der Projektinhaber hat slice-047 mit **„angenommen mit benanntem Rest"** validiert
([`validator.md`](../../../../.harness/skills/validator.md)). Zwei der vier Rest-Punkte hängen an
**derselben fehlenden Sache** — die Sitzung hat keinen Zustand „welche Datei bin ich, und bin ich seit
dem letzten Schreiben verändert worden?":

1. **Kein „Speichern" auf die offene Datei.** Es gibt nur **Speichern unter…** — jeder Speichervorgang
   fragt erneut nach einem Pfad.
2. **Keine Warnung vor ungesicherten Änderungen.** Wer ein anderes Projekt öffnet oder das Fenster
   schließt, verliert seine Änderungen **still**.

**Warum das ein Datenverlust-Thema ist:** [`harness/README.md`](../../../../harness/README.md)
§Safety nennt Datenverlust am Gebäudemodell den **schärfsten Fehlerfall**. Die bisherige Absicherung
([`LH-QA-005`](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery) Crash-Recovery,
[`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) Atomarität) schützt
gegen Verlust **beim Schreiben**. Hier geht es um den Verlust **ohne** Schreiben — eine Lücke, die
erst entstand, als slice-047 dem Benutzer überhaupt eine Sitzung gab, die er verlieren kann.

## 1. Ziel

- **Die Sitzung weiß, welche Datei sie ist** → „Speichern" schreibt ohne Pfad-Dialog dorthin zurück;
  „Speichern unter…" bleibt daneben bestehen.
- **Die Sitzung weiß, ob sie ungesichert ist** → **Öffnen** und **Fenster schließen** fragen vorher
  nach. (Der dritte Auslöser „Neues Projekt" liegt in [`slice-052b`](slice-052b-neues-projekt.md).)

## 2. Lösung — **entschieden** (Projektinhaber 2026-07-26, nach Lauf-1-HIGH-1)

> **Der erste Entwurf ist verworfen.** Er wollte den »verändert«-Zustand aus
> `ModelChangedPort`-Meldungen ableiten. Lauf 1 hat belegt, dass das **für keine heute erreichbare
> Benutzer-Mutation** funktioniert: `addGuideLine`
> (`src/hexagon/services/structure_edit_service.cpp`:1347), `removeGuideLine` (:1350–1359), alle
> Layer-Mutatoren (:1283, :1311) und alle Material-Mutatoren (:1147, :1194) melden **nichts** — „kein
> op" ist die ausdrückliche Entscheidung 2 der **`Accepted`**-[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md).
> Da Hilfslinien die **einzige** GUI-Mutation sind (§5), wäre die Warnung nie ausgelöst worden,
> **während alle Orakel-Zeilen grün stehen**.

**Der Zustand kommt aus dem Vergleich, nicht aus Meldungen.**

```
ProjectSession(building)            // Baseline bei Sitzungs-Beginn — im Kern, nicht in main (§6-Zeile 8)
markPersisted(pfad, building)       // nach erfolgreichem Öffnen UND nach erfolgreichem Speichern
isDirty(current) := current != last_persisted_
```

Was der Vergleich leistet — und was **nicht** (Lauf-2-MEDIUM-1, präzisiert statt wiederholt):

- **Gegenüber neuen Mutatoren ist er strukturell.** Es gibt keine Meldung, die ein Mutator vergessen
  könnte, und keine `op`-Liste, die gepflegt werden müsste. Jeder heutige und jeder künftige
  **Schreibweg** ist erfasst, weil der Vergleich am **Ergebnis** ansetzt.
- **Gegenüber neuen Feldern ebenfalls — aber nur in **einer** Implementierungs-Form.** Lauf-3-MEDIUM-1
  hat zu Recht gerügt, dass die Vorfassung „strukturell" behauptete und zwei Absätze später eine
  Konvention beschrieb. Die Auflösung ist eine **Festlegung, die der Plan bisher schuldig blieb**: die
  Vergleiche sind **`= default`** (C++20 — `CMakeLists.txt`:5 setzt `CXX_STANDARD 20`). Ein
  `auto operator==(const T&) const = default;` vergleicht **alle** Member, die der Typ hat — auch die,
  die ein künftiger Slice hinzufügt, **ohne** dass jemand den Operator anfasst. Damit ist auch die
  Feld-Dimension strukturell. **Handgeschriebene Vergleiche sind verboten**; genau sie wären der stille
  Datenverlust aus §9 R1, und genau dagegen steht die Gegenprobe der §6-Zeile 3.
- **Semantisch genauer als ein Flag.** „Zurück-geändert auf den Dateistand" ist wieder **sauber**.
- **Kein Vertrag wird berührt.** Kein neuer `op`, keine neue Schicht-Kante, kein Port: die
  [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Entscheidung bleibt gültig, statt umgangen oder per
  Folge-ADR aufgehoben zu werden.

**Ort: der Kern** (`src/hexagon/services/`), framework-frei — der Träger vergleicht
`model::Building`-Werte und kennt sonst nichts. `main.cpp` scheidet aus, weil dort Liegendes
**orakel-los per Konstruktion** ist ([slice-047](../done/slice-047-projekt-oeffnen.md) §7).

**Preis, beziffert (Lauf-2-LOW-2):** `operator==` auf **13** Struct-Typen — `Building` + die neun von
ihm gehaltenen Element-Typen + die verschachtelten Koordinaten-Träger `Point2D`, `Segment`, `Footprint`
(Lauf-2-MEDIUM-3: **die sind der eigentliche Ort des Risikos**, weil die Element-Typen ihre
Punkt-Felder als Ganzes ersetzen). Dazu eine Modell-Kopie im Speicher; der Vergleich läuft nur bei
Verwerf-Aktionen.

**Drei Entscheidungen liegen im Kern, nicht in der Verdrahtung** (Lauf-1-MEDIUM-2/-3, Lauf-2-MEDIUM-6a):

| Frage | Kern-Abfrage | Warum nicht in `main` |
|---|---|---|
| Darf dieser Stand verworfen werden? | `verdictForDiscard(current)` → `Proceed`/`AskFirst` | sonst orakel-los |
| Wohin schreibt „Speichern"? | `saveTarget()` → bekannter Pfad / Ziel-Abfrage | sonst bliebe Zeile 9 grün, während der Handler doch fragt |
| Was folgt aus der Antwort? | Antwort (speichern/verwerfen/abbrechen) + Verdikt → ausführen / **unterlassen** / erst speichern | „abbrechen ⇒ unterlassen" ist die schärfste Zusage des Slice |

`main` fragt ab, zeigt den Dialog, reicht die Antwort zurück und **führt aus** — es **entscheidet**
nichts.

## 3. Anforderungs-Ebene — **korrigiert** (Lauf-1-HIGH-2)

> **Die frühere Aussage „die Ungesichert-Warnung hat heute keine Anforderung" war falsch.**
> [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) trägt sie bereits auf
> **AK-Niveau** — allerdings für den Auslöser „Neues Projekt", der in
> [`slice-052b`](slice-052b-neues-projekt.md) liegt.

Für **diesen** Slice sind zwei AK zu ergänzen (lösungsfrei nach
[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei);
beim Vollzug Header-Version + Historie nachziehen,
[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)):

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
  then Rückfrage vor dem Ersetzen des Arbeitsstands; der Benutzer kann
  speichern, verwerfen oder die Aktion abbrechen.
```

**Zuordnung der Auslöser, begründet** (Lauf-3-LOW-2): „Speichern" gehört zu **BLD-002**, „Öffnen" zu
**BLD-003** — beides unmittelbar. Der Auslöser **„Programm beenden"** schärft streng genommen keine der
beiden Anforderungen; er hängt an BLD-002, weil die Rückfrage dort **das Speichern** anbietet und der
Verlust, den sie verhindert, der Verlust eines **ungespeicherten** Stands ist. Die Alternative wäre eine
eigene Anforderung „Sitzung beenden", die es nicht gibt und die dieser Slice nicht erfindet.

**Beide Vorlagen sind dreiwertig ausformuliert** (Lauf-2-MEDIUM-8): der frühere Rückverweis „analog
[LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)" behauptete eine Deckung, die der dortige **zweiwertige** AK-Text („Rückfrage
‚Änderungen verwerfen?'") nicht hergibt. Für „Neu" löst das
[`slice-052b`](slice-052b-neues-projekt.md).

**Warum der Text hier steht und nicht schon im Lastenheft:** die Aufnahme ist **Ausführung** dieses
Slice, nicht seine Planung — sie braucht den Lifecycle-Schritt nach `in-progress/` und das
vorgeschaltete [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start).

## 4. Bewusst NICHT Teil

- **„Neues Projekt"** → [`slice-052b`](slice-052b-neues-projekt.md). Der Split ist die Antwort auf
  Lauf-2-HIGH-1 und -MEDIUM-11: der Auslöser bringt eigene Zustands-, Anforderungs- und Doku-Arbeit mit
  (Reset der gemerkten Datei, Negative-AK, fehlende Zeichen-Ebene) und hat in einem Schnitt mit dem
  Sitzungs-Zustand nachweislich Löcher hinterlassen.
- **GUI-Export** und **Zuletzt-geöffnet-Liste** — die beiden anderen Rest-Punkte der
  slice-047-Validation; sie hängen nicht am Sitzungs-Zustand.
- **Projektversionierung** ([`LH-FA-BLD-004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung))
  und **Undo/Redo** ([`LH-QA-003`](../../../../spec/lastenheft.md#lh-qa-003--undoredo)) — „ungesichert"
  ist eine **Ja/Nein**-Eigenschaft, keine Historie.
- **Auto-Save** — eigene Anforderung
  ([`LH-QA-004`](../../../../spec/lastenheft.md#lh-qa-004--autosave)), eigener Mechanismus, eigener Slice.
- **Kein neuer `op`, keine [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Korrektur.**

## 5. Wirkungs-Breite — **entschieden: jetzt bauen, nicht später**

Die **einzige** Modell-Mutation, die dem GUI-Benutzer heute offensteht, ist das Zeichnen von
Hilfslinien ([ADR-0019](../../adr/0019-drw-2d-canvas.md) v1). Der Schutz greift also vorerst für **eine**
Art von Änderung — das ist wenig, und der Slice darf seinen Wert nicht überzeichnen.

**Trotzdem jetzt:** der Vergleich erfasst jeden künftigen **Schreibweg** ohne Zutun (§2), und die
Kosten wachsen mit dem Modell, nicht mit der Zeit — `operator==` über 13 Typen ist heute mechanisch,
nach weiteren Bauteil-Familien dieselbe Arbeit plus Nachrüst-Risiko (§9 R1).

## 6. Orakel-Schnitt — **vorab entschieden**

slice-047 hat gezeigt, was passiert, wenn man diesen Schnitt der Verifikation überlässt. Deshalb steht
er **vor** dem Start — erweitert um die Lücken beider [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe.

| # | Zusicherung | Diskriminierende Gegenprobe | Herkunft |
|---|---|---|---|
| 1 | **Hilfslinie zeichnen ⇒ Sitzung ungesichert** (die Mutation, die **keinen** `op` meldet) | Vergleich durch einen Meldungs-Beobachter **ersetzt** ⇒ rot | **Lauf 1 HIGH-1** |
| 2 | eine **op-tragende** Mutation (Wand) ⇒ ungesichert | Vergleich entfernt ⇒ rot | — |
| 3 | **Feld-Vollständigkeit der Gleichheit**: je Struct-Typ **jedes** Feld einzeln verändert ⇒ ungleich — **inkl. `Point2D`/`Segment`/`Footprint`** | ein Feld aus einem `operator==` entfernt (z. B. `Point2D::y_mm`) ⇒ rot | **Lauf 2 MEDIUM-2/-3** |
| 4 | nach erfolgreichem **Speichern** ⇒ sauber | Rücksetzen entfernt ⇒ rot | — |
| 5 | nach **Öffnen** ⇒ sauber | `markPersisted` im Öffnen-Pfad entfernt ⇒ rot | — |
| 6 | nach **gescheitertem** Speichern ⇒ **weiter** ungesichert | Rücksetzen vor statt nach dem Schreiben ⇒ rot | — |
| 7 | **zurück-geändert auf den Dateistand** ⇒ wieder sauber | Vergleich durch ein Einweg-Flag ersetzt ⇒ rot | §2 |
| 8 | **frisch konstruierte Sitzung** (Baseline = Stand bei Sitzungs-Beginn) ⇒ `Proceed` | Baseline leer statt Start-Stand ⇒ rot | **Lauf 1 MED-1 / Lauf 2 MED-4** |
| 9 | **Ziel-Wahl beim Speichern**: bekannter Pfad ⇒ „dorthin"; unbekannt ⇒ „fragen" | Zweige vertauscht ⇒ rot | **Lauf 1 MEDIUM-2** |
| 10 | „Speichern" schreibt in die **gemerkte** Datei (Round-Trip über das echte Repository) | Pfad nicht gemerkt ⇒ rot | — |
| 11 | **Verdikt**: `Proceed` / `AskFirst` | Verdikt-Bildung entfernt ⇒ rot | — |
| 12 | **Antwort-Auswertung**: „abbrechen" ⇒ *unterlassen*; „verwerfen" ⇒ *ausführen*; „speichern" ⇒ *erst speichern, dann ausführen* | „abbrechen" führt aus ⇒ rot | **Lauf 1 MEDIUM-3** |
| 12a | **„speichern" im Verwerf-Fluss scheitert** (Zielmedium/Recht) ⇒ die auslösende Aktion wird **unterlassen**, der Stand bleibt ungesichert | Fehler geschluckt und trotzdem ausgeführt ⇒ rot | **Lauf 3 MEDIUM-2a** |
| 13 | **Fenster-Schließen mit ungesichertem Stand** ⇒ Rückfrage; „abbrechen" ⇒ Fenster bleibt offen (headless über `QCloseEvent` + `QApplication::sendEvent`) — **setzt [`slice-053`](../done/slice-053-fenster-als-adapter.md) voraus** | Schließ-Anbindung entfernt ⇒ rot | **Lauf 2 MEDIUM-10 / Lauf 3 HIGH-1** |
| 14 | **Baseline-Übergabe**: die Sitzung wird mit dem Stand **nach** dem Start-Aufbau konstruiert | Konstruktion mit leerem `Building` ⇒ rot (Zeile 8 allein bliebe grün) | **Lauf 3 MEDIUM-2b** |

**Zeile 13 war in Lauf 3 der blockierende HIGH — und ist der Grund für
[`slice-053`](../done/slice-053-fenster-als-adapter.md):** unter dem Datei-Plan der Vorfassung wäre sie **nicht
herstellbar** gewesen, weil `main.cpp` in **kein** Testbinary gelinkt ist (`src/CMakeLists.txt`:6 vs.
`tests/CMakeLists.txt`) und die zitierte Präzedenz **Adapter-Klassen** prüft. Das Ereignis ist erst
prüfbar, wenn das Fenster eine Adapter-Klasse **ist** — deshalb der Vorläufer, nicht ein weiterer
Anlauf hier. **Die Korrektur der Grenze bleibt richtig:** die Vorfassung erklärte das
Qt-Schließ-Ereignis für sensorlos. Das Repo stellt Qt-Ereignisse aber längst headless zu
(`tests/adapters/test_canvas_widget.cpp`:108–116, `tests/adapters/test_viewer_widget.cpp`:55–67;
[ADR-0009](../../adr/0009-gui-framework-qt6.md) (f)). Die Sensorlosigkeit wäre eine Eigenschaft der
**Verortung** gewesen, nicht des Ereignisses — also eine Entscheidung, keine Naturkonstante.

**Benannte Grenze (bleibt ohne Sensor, bewusst — vollständige Aufzählung):** **nur** die modalen
Dialoge selbst (`QMessageBox`, `QFileDialog`) — sie blockieren im Test und werden über eine
injizierbare Naht gerufen ([`slice-053`](../done/slice-053-fenster-als-adapter.md) §3).

**Nicht** mehr in der Grenze: das **Schließ-Ereignis** (Zeile 13) und die **Menü-Verdrahtung** — beides
wird mit [`slice-053`](../done/slice-053-fenster-als-adapter.md) prüfbar (dessen §3-Zeile 4 belegt, dass eine
Menü-Aktion programmatisch auslösbar ist). Damit fällt auch Lauf-3-MEDIUM-3: die DoD-Zusage „‚Speichern'
**nutzt** `saveTarget()`" lag in der Vorfassung vollständig in der sensorlosen Zone — Zeile 9 prüfte die
Kern-Abfrage, nicht ihre Verwendung. Der Nachweis der **Verwendung** hängt damit ebenfalls an 053.

Die harte Auflage bleibt: **in der Verdrahtung steht keine Entscheidung.**

## 7. Definition of Done

- [ ] **Gleichheit auf den Modell-Werttypen** (`src/hexagon/model/`): `auto operator==(const T&) const
      = default;` auf **allen 13** Struct-Typen — `Building`, die neun Element-Typen **und** die
      verschachtelten `Point2D`/`Segment`/`Footprint` (Lauf-2-MEDIUM-3). **`= default` ist Pflicht, kein
      Stil:** nur der compiler-generierte Vergleich nimmt künftige Felder automatisch auf
      (Lauf-3-MEDIUM-1). Orakel: §6-Zeile 3.
- [ ] **`ProjectSession` im Kern** (`src/hexagon/services/`, framework-frei, **kein** Port):
      Konstruktion mit **Start-Baseline** · `markPersisted(path, building)` · `path()` ·
      `isDirty(current)` · `verdictForDiscard(current)` · `saveTarget()` · Antwort-Auswertung.
- [ ] **Orakel: alle 15 Zeilen der §6-Tabelle** (1–12, 12a, 13, 14), **je einmal als diskriminierend belegt** (Gegenprobe
      rot, im Closure-Text protokolliert). **Zeile 1 ist Pflicht** — Regressions-Schutz gegen
      Lauf-1-HIGH-1; **Zeile 3** ist die einzige Absicherung gegen §9 R1.
- [ ] **Menü-Aktion „Speichern"** nutzt `saveTarget()` + `services::saveProject`; „Speichern unter…"
      bleibt und **setzt** den Pfad. **Kein** zweiter Schreibpfad am Use-Case vorbei (§9 R3).
- [ ] **Rückfrage vor Sitzungs-Verlust** an **beiden** Auslösern dieses Slice (Öffnen, Fenster
      schließen); **Abbrechen unterlässt** die auslösende Aktion. Der Schließ-Weg wird über die
      [`slice-053`](../done/slice-053-fenster-als-adapter.md)-Naht geprüft (§6-Zeile 13).
- [ ] **Lastenheft:** die §3-Vorlagen in
      [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) und
      [`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden); Header-Version +
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md) nachgezogen.
- [ ] **Spezifikation §1** neuer Block für **beide** geschärften Anforderungen —
      [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.b` **und** die
      neue [`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)-Boundary
      (Lauf-3-LOW-1: der bestehende Block ist ein **kombinierter** `002.a / 003.a`; ein reiner
      `002.b`-Block ließe die neue BLD-003-AK spezifikations-seitig unverankert). Inhalt:
      Sitzungs-Zustand, Vergleichs-Semantik (`= default`), Baseline, Verdikt, Rücksetz-Regel. + Zeile in
      [`spezifikation-historie.md`](../../../../spec/spezifikation-historie.md).
- [ ] **`spec/architecture.md`** §2.1-Verzeichnisbaum um den neuen Kern-Service ergänzt (Lauf-1
      MEDIUM-5 — der Baum zählt die Services heute **vollständig** auf).
- [ ] **Benutzerhandbuch** an **vier** Stellen (Lauf-2-MEDIUM-9, soweit dieser Slice sie berührt):
      **4.3**, **FAQ**, **2.3 Grundlegende Bedienung** und die **4.1-Aufgaben-Tabelle** — alle vier
      beschreiben heute einen Stand ohne „Speichern" und ohne Rückfrage. Handbuch-Version +
      Änderungshistorie mitziehen. (**`docs/user/` steht in dieser DoD-Zeile** — Lehre aus slice-047 V1.)
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`, Lauf-1-INFO-1/Lauf-2-INFO-2);
      `make schema-check` byte-unberührt (**kein** persistenter Zustand).

## 8. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/model/*.h` (13 Struct-Typen) | ändern | `operator==` über alle Felder, inkl. `Point2D`/`Segment`/`Footprint` (§2) |
| `src/hexagon/services/project_session.{h,cpp}` | neu | Baseline, Verdikt, Ziel-Wahl, Antwort-Auswertung (§2) |
| `src/hexagon/services/manage_project.{h,cpp}` | ändern | erfolgreiches Öffnen/Speichern meldet Pfad **+ Stand** (**nach** dem Erfolg, §9 R2) |
| `src/adapters/ui/view/main_window.*` (aus [`slice-053`](../done/slice-053-fenster-als-adapter.md)) | ändern | Menü **Speichern**; Öffnen + **Schließen** holen Verdikt/Auswertung — hier, weil nur hier prüfbar (Lauf-3-HIGH-1) |
| `src/main.cpp` | ändern | Verdrahtung: Sitzung konstruieren (Baseline, §6-Zeile 14), Dialoge stellen (**keine** Entscheidung) |
| `src/hexagon/CMakeLists.txt`, `tests/CMakeLists.txt` | ändern | beide Listen zählen Dateien **explizit** auf (Lauf-1-LOW-3) |
| `tests/hexagon/test_model_equality.cpp` | neu | §6-Zeile 3 (Feld-für-Feld über alle 13 Typen) |
| `tests/hexagon/test_project_session.cpp` | neu | §6-Zeilen 1, 2, 4–9, 11, 12 |
| `tests/adapters/test_project_open_handler.cpp` | ändern | §6-Zeile 10 (Round-Trip in die gemerkte Datei) |
| `tests/adapters/test_main_window.cpp` (aus [`slice-053`](../done/slice-053-fenster-als-adapter.md)) | ändern | §6-Zeilen 13 + 14 (`QCloseEvent` headless + Baseline-Übergabe) |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Aufnahme BLD-002/003 + Header-Version |
| `spec/spezifikation.md`, `spec/spezifikation-historie.md` | ändern | §1-Mechanik-Block + Provenance-Zeile |
| `spec/architecture.md` | ändern | Kern-Service im §2.1-Baum |
| `docs/user/benutzerhandbuch.md` | ändern | 4.3, FAQ, 2.3, 4.1 |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |
| `docs/reviews/`-Report zum dritten Plan-Review | neu | das dritte [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start (Lauf-1-LOW-5) |

**Nicht berührt:** `data-model.yaml`/`schema.sql` (kein persistenter Zustand), `.d-check.yml`/`.a-check.yml`
(keine neue Kante, kein Port), `docs/plan/adr/` (§10).

## 9. Verbleibende Risiken

- **R1 — unvollständige Gleichheit ist stiller Datenverlust.** Vergisst `operator==` ein Feld, gilt
  dessen Änderung als „nicht verändert". **Die** Achillesferse des Ansatzes; Gegenmaßnahme ist
  §6-**Zeile 3**. Für **neue** Felder künftiger Slices bleibt es eine **Konvention** (der Test muss
  mitwachsen) — das ist die ehrliche Grenze der „strukturell"-Aussage in §2 (Lauf-2-MEDIUM-1). Ein
  Gate, das ein `operator==` gegen die Feldliste eines Structs prüft, existiert nicht; ob eines
  entstehen soll, ist ein **benannter Re-Eval**, nicht Teil dieses Schnitts.
- **R2 — Reihenfolge beim Rücksetzen.** „Sauber" gilt erst, **nachdem** das Schreiben erfolgreich
  zurückgekehrt ist (§6-Zeile 6 — die einzige Zeile auf einem Fehlerpfad).
- **R3 — „Speichern" ohne Dialog ist ein neuer Datenverlust-Pfad.** Es überschreibt die gemerkte Datei
  **ohne Rückfrage** (das ist der Zweck). Die Atomarität aus
  [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) trägt das bereits —
  **kein** Schreibpfad am Use-Case vorbei. *(Der gefährlichste Fall dieser Klasse — „Speichern" nach
  „Neu" schreibt in die alte Datei — liegt in [`slice-052b`](slice-052b-neues-projekt.md); dieser Slice
  kennt keine Aktion, die den Pfad stehen lässt und den Stand ersetzt.)*
- **R4 — Kosten des Vergleichs.** Modell-Kopie + O(n)-Vergleich, nur bei Verwerf-Aktionen. Bei sehr
  großen Modellen ein benannter Re-Eval (Inhalts-Hash), **kein** Thema dieses Schnitts.
- **R5 — Start-Baseline vs. Demo-Aufbau.** Der Composition-Root baut beim Start ein Demo-Modell
  (`main.cpp`:445) **und** legt ggf. eine Ebene „Canvas" an (:459–460) — ein Stand, der nie in einer
  Datei stand. Die Baseline wird deshalb **bei Sitzungs-Beginn nach diesem Aufbau** gesetzt und ist
  Konstruktions-Parameter des Kern-Objekts (§2), damit die Festlegung nicht im orakel-losen `main`
  landet (§6-Zeile 8). Lauf-2-MEDIUM-4 hat beide Start-Mutationen benannt.

## 10. ADR-Abwägung (Lauf-1-MEDIUM-7 — abgewogen, nicht verneint)

[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Entscheidung 1 behandelte dieselbe **Frageklasse**
(Datenheimat) ADR-pflichtig. Dort ging es um Daten, die **persistiert, exportiert und von mehreren
Adaptern gesehen** werden — mit Fernwirkung auf Schema, Ports und Formate. Der Sitzungs-Zustand ist das
Gegenteil: **nicht persistiert**, **nicht exportiert**, **kein neuer Port**, **keine neue
Schicht-Kante**, **kein Framework**; er lebt und stirbt mit einer Programm-Sitzung und ändert keinen
öffentlichen Vertrag. Das benutzer-sichtbare Verhalten normieren Lastenheft-AK + §1-Block.

**Ergebnis: keine ADR** — als Abwägung protokolliert. Fällt der dritte
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
anders aus, ist sie vor dem Start zu schreiben.

## 11. Trigger

- **Validations-Rest** aus der [slice-047](../done/slice-047-projekt-oeffnen.md)-Closure (2026-07-26,
  Rolle Projektinhaber) — ein **Bedarfs**-Befund, kein Gate-Befund.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) Lauf 1** (2 HIGH) → Mechanismus-Wechsel. **Lauf 2** (1 HIGH, MEDIUM-11) → **Split**.

## 12. Closure-Trigger

- **Alle 15 Zeilen der §6-Tabelle** grün **und** je einmal als diskriminierend belegt (Gegenprobe rot).
- **Lastenheft** um die §3-Vorlagen ergänzt, Header-Version + Historie nachgezogen; §1-Mapping mit der
  dann feststehenden Mechanik.
- **Handbuch an allen vier Stellen** nachgeführt.
- `make gates` grün; Closure-Notiz beantwortet die §6-Auflage („keine Entscheidung in der Verdrahtung")
  und protokolliert die Gegenproben.

## 13. MR-006-Einarbeitung (zwei Läufe, 2026-07-26)

**Lauf 1** ([Report](../../../reviews/2026-07-26-slice-052-plan.md)) — 2 HIGH / 7 MED / 5 LOW / 2 INFO:
HIGH-1 → Mechanismus-Wechsel (§2). HIGH-2 → Anforderungs-Lage korrigiert (§3). MED-1 → §6-Zeile 8.
MED-2 → §6-Zeile 9. MED-3 → §6-Zeile 12. MED-4 + INFO-2 → mit dem Wechsel **gegenstandslos** (kein Port,
keine Meldungs-Bindung). MED-5 → `architecture.md` in DoD + Datei-Plan. MED-6 → Handbuch-FAQ.
MED-7 → §10. LOW-1..5 + INFO-1 → eingearbeitet.

**Lauf 2** ([Report](../../../reviews/2026-07-26-slice-052-plan-2.md)) — 1 HIGH / 11 MED / 4 LOW / 2 INFO:

| # | Behandlung |
|---|---|
| **HIGH-1** (nach „Neu" schreibt „Speichern" in die alte Datei) | **an der Wurzel: Split.** Dieser Slice kennt keine Aktion, die den Modellstand ersetzt und den Pfad stehen lässt. Reset-Semantik + Orakel liegen in [`slice-052b`](slice-052b-neues-projekt.md). |
| **MEDIUM-1** (Struktur vs. Konvention) | §2 unterscheidet jetzt **neue Mutatoren** (strukturell erfasst) von **neuen Feldern** (Konvention); §9 R1 benennt die Grenze und den Re-Eval statt sie zu überzeichnen. |
| **MEDIUM-2** (Feld-Orakel außerhalb des Maßstabs) | als **§6-Zeile 3** in die Tabelle gehoben, die §12 zum Maßstab erklärt. |
| **MEDIUM-3** (verschachtelte Werttypen) | §2 + §7 nennen `Point2D`/`Segment`/`Footprint` **namentlich**; die Gegenprobe der Zeile 3 nennt `Point2D::y_mm` als Beispiel. |
| **MEDIUM-4** (Anfangszustand) | **Baseline ist Konstruktions-Parameter** des Kern-Objekts (§2), Orakel §6-Zeile 8, Risiko §9 R5 — inkl. der zweiten Start-Mutation `addLayer`. |
| **MEDIUM-5** (B4-Klasse bei „Neu") | → [`slice-052b`](slice-052b-neues-projekt.md). |
| **MEDIUM-6a** (Speichern-Verdrahtung nicht in der Grenze) | die Grenze ist neu gefasst und zählt die Menü-Verdrahtung **inkl. Speichern** auf; Zeilen 9 + 10 decken die Entscheidung. **MEDIUM-6b** (Neu-Erzeugung) → 052b. |
| **MEDIUM-7** (Negative-AK [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) | → [`slice-052b`](slice-052b-neues-projekt.md). |
| **MEDIUM-8** (zweiwertige AK als Referenz) | beide §3-Vorlagen sind **dreiwertig ausformuliert**; der Rückverweis „analog BLD-001" ist entfernt. |
| **MEDIUM-9** (Handbuch-Stellen) | DoD nennt **vier** Stellen für diesen Slice (4.3, FAQ, 2.3, 4.1); die „Neu"-Stellen (§1 „Heute möglich", §3 „drei Wege") trägt 052b. |
| **MEDIUM-10** (Schließ-Ereignis testbar) | **§6-Zeile 13** neu, headless über `QCloseEvent`; die Grenze führt das Ereignis nicht mehr. |
| **MEDIUM-11** (Sizing) | **Split** in 052a/052b; §14 neu bewertet. |
| **LOW-1** | `removeGuideLine` mit **eigener** Fundstelle (:1350–1359) belegt. |
| **LOW-2** | **13** Struct-Typen statt „~10". |
| **LOW-3** (§1-Anker für „Neu") | → [`slice-052b`](slice-052b-neues-projekt.md). |
| **LOW-4** (§14-Widerspruch) | §14 neu formuliert. |
| **INFO-1** (Gegenprobe braucht Ersatz-Implementierung) | in §6-Zeile 1 als **Ersatz** statt Entfernung ausgeschrieben. |
| **INFO-2** (Ruhe-Marker) | in der DoD-Gates-Zeile. |

**Lauf 3** ([Report](../../../reviews/2026-07-26-slice-052a-plan-3.md)) — 1 HIGH / 3 MED / 4 LOW / 2 INFO.
Der Lauf bestätigte Split (sauber, keine Zusage verloren) und eigenständige Lieferbarkeit von 052a:

| # | Behandlung |
|---|---|
| **HIGH-1** (Orakel-Zeile 13 unter dem eigenen Datei-Plan nicht herstellbar) | **Vorläufer-Slice [`slice-053`](../done/slice-053-fenster-als-adapter.md)**: das Hauptfenster wird eine Adapter-Klasse, erst dann ist das Schließ-Ereignis prüfbar. 052a hängt davon ab. Projektinhaber-Entscheidung 2026-07-26. |
| **MEDIUM-1** (Struktur/Konvention beruht auf ungenannter Implementierungs-Form) | **`= default` festgelegt** (C++20, `CMakeLists.txt`:5): der compiler-generierte Vergleich nimmt künftige Felder automatisch auf, handgeschriebene Operatoren sind **verboten**. Damit ist auch die Feld-Dimension strukturell — §2 und §9 R1 sind widerspruchsfrei. |
| **MEDIUM-2a** (Fehlerweg „Speichern im Verwerf-Fluss scheitert") | **§6-Zeile 12a** neu. |
| **MEDIUM-2b** (Baseline-Übergabe in `main`) | **§6-Zeile 14** neu. |
| **MEDIUM-3** (Nutzung von `saveTarget()` in der sensorlosen Zone) | mit 053 prüfbar; die Grenze führt die Menü-Verdrahtung nicht mehr. |
| **LOW-1** | §1-Block deckt **beide** Anforderungen (der bestehende ist ein kombinierter `002.a/003.a`). |
| **LOW-2** | §3 begründet die Auslöser-Zuordnung, inkl. „Programm beenden". |
| **LOW-3** | Sizing: die Tabelle wuchs auf 15 Zeilen; §14 bleibt „hoch", ein weiterer Split ist laut Lauf 3 **nicht** angezeigt. |
| **LOW-4** | `adr_refs` ohne [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) (kommt im Körper nicht mehr vor). |
| **INFO-1** | Ruhe-Marker in der DoD-Gates-Zeile. |
| **INFO-2** (Pflege-Signal: dritte Wiederholung) | **beantwortet durch [`slice-053`](../done/slice-053-fenster-als-adapter.md)** — die Klasse „Entscheidung im Kern geprüft, Nutzung im Handler nicht" bekommt einen strukturellen Ort statt einer vierten Einzelfall-Behandlung. |

**Lehre aus drei Läufen:** Der Plan hatte den Orakel-Schnitt **vorab** entschieden — die richtige Lehre
aus slice-047 — und beide HIGHs lagen trotzdem **außerhalb** dessen, was ein Orakel je gezeigt hätte:
Lauf-1-HIGH-1 hätte grüne Orakel bei toter Funktion ergeben, Lauf-2-HIGH-1 war ein Zustands-Loch, das
erst durch eine **Scope-Erweiterung** entstand, und Lauf-3-HIGH-1 war ein **versprochenes Orakel, das
der eigene Datei-Plan nicht hergab** — ein Sensor, den man zusagt, ohne den Code dorthin zu bewegen, wo
er messbar ist. Ein Orakel prüft, was man gebaut hat; ob der
Mechanismus den realen Weg trifft und ob ein gewachsener Scope bis in alle Ecken durchgezogen wurde,
prüft nur ein unabhängiger Leser. **Und:** Scope-Wachstum ist nicht additiv — der dritte Auslöser hat
nicht ein Feature, sondern eine Zustands-, eine Anforderungs- und eine Doku-Dimension mitgebracht.

## 14. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Sitzung / Persistenz-Bedienung

- **Modus:** GF; **Dichte:** **hoch** (13 `operator==` + Feld-Orakel, ein neuer Kern-Service mit sechs
  Verantwortlichkeiten, 15 Orakel-Zeilen **je mit roter Gegenprobe**, AK-Ergänzung an zwei
  Anforderungen, fünf Spec-/Doku-Dateien). Die frühere Einstufung „mittel / keine neue Mechanik" war
  falsch (Lauf-2-MEDIUM-11/LOW-4): der Sitzungs-Zustand **ist** neue Mechanik.
- **Phase-Reife:** Persistenz-Mechanik reif (welle-1), Aufruf-Pfad seit slice-047 vorhanden.
- **Risiko:** mittel-hoch — Datenverlust-nah, und **R1** wirkt **still**.

## 15. Closure-Notiz

_(bei Ausführung auszufüllen)_
