# Status

## 


---

## Tagesabschluss 2026-07-27 — Stand + nächste-Sitzung-Zeiger

`make gates` **EXIT=0** (docs-check **0 Befunde / 263 Dateien** · a-check 0 · arch-check ok ·
**352/352** Tests · Coverage 91,8 %), `make io-smoke` EXIT=0, `make schema-check` EXIT=0,
`make acc-002-beleg` EXIT=0. **19 Commits, alle auf `origin/main`.** `in-progress/` trägt **keinen**
Slice — der Ruhe-Sentinel steht.

### Was heute abgeschlossen wurde

| | |
|---|---|
| **Vierer-Kette komplett** | [054](../done/slice-054-manage-project-port.md) (`ManageProjectPort`) → [053](../done/slice-053-fenster-als-adapter.md) (Fenster + Handler als Adapter) → [052a](../done/slice-052a-sitzungs-zustand-und-speichern.md) (Sitzungs-Zustand, „Speichern", Rückfrage) → [052b](../done/slice-052b-neues-projekt.md) („Neues Projekt"). **[`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)/002/003 erstmals in der Oberfläche erfüllbar.** Lastenheft 0.1.17 → **0.1.19**, Handbuch 1.1 → **1.3** |
| **welle-5 geschlossen** | [`done/welle-5-results.md`](../done/welle-5-results.md), **M5 gebucht** (war seit dem 2026-07-03 inhaltlich erfüllt, die Roadmap sagte „offen") |
| **welle-6 geschnitten** | `welle-6-interaktiv-planen`, Meilenstein **M6**; Ziel [OBJ-004→OBJ-001](../../../../spec/lastenheft.md#3-projektziele), Trigger: **eine Wand ist im 2D-Canvas zeichenbar und parametrisch änderbar** |
| **Erster welle-6-Plan** | [`slice-048b`](slice-048b-drw-001-fangpunkte-impl.md) (Fangpunkte), Review durch, **startbar** |

**Elf [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe
an einem Tag, 22 HIGH — jeder Lauf fand etwas Echtes, keine Zeile Code ist deswegen gefallen.**

### ▶ Nächste Sitzung: hier ansetzen

**[`slice-048b`](slice-048b-drw-001-fangpunkte-impl.md) ist startbar** — aber **R6 zuerst**:

> Vor dem ersten Commit ist zu verifizieren, dass die `guide_lines`-Round-Trip-Orakel
> ([032b](../done/slice-032b-drw-impl.md)) und die 2D-Export-Orakel
> ([032c](../done/slice-032c-drw-export.md)) die Koordinaten **wertgleich** prüfen — nicht nur die
> Anzahl. Ergeben sie das nicht, gehört Orakel-Zeile 10 als eigener Test ergänzt. **Nicht in den
> Vollzug verschieben.**

Danach: `snap.{h,cpp}` (reine Funktion, ohne Geschoss-Parameter), Aufruf an den zwei
`screenToModel`-Stellen **mit `pull_()` an Ort und Stelle**, zehn Orakel-Zeilen mit Gegenprobe.

**Danach offen in welle-6:** der eigentliche Wellen-Kern — **Wand zeichnen im Canvas** — hat noch
keinen Plan. Und die **Fang-Anzeige** (R5: der Fang reicht über das dargestellte Geschoss hinaus,
also sollte sichtbar sein, worauf gerastet wurde).

### Die drei Lehren des Tages

1. **Ein Default-Argument kann ein Orakel aushebeln.** `sinks = {}` hätte einen vergesslichen
   Aufrufer still durchgelassen; der Verlust wäre unter **allen** Orakeln grün geblieben. Wo ein
   Parameter verhaltenstragend ist, ist der **fehlende** Default der schärfste Sensor.
2. **„Geprüft" heißt nicht „am richtigen Ort geprüft".** Dreimal blieb eine Gegenprobe grün, weil ein
   Orakel die falsche Komponente prüfte (052a, 052b, und im 048b-Plan wäre es fast ein viertes Mal
   passiert — MEDIUM-2). **Regel-Kandidat, 3×-Regel erfüllt**, s. u.
3. **Ein Wellen-Label ohne Abschluss-Kriterium hört auf, etwas zu bedeuten — und niemand merkt es.**
   welle-5 lief 24 Tage über ihren erfüllten Meilenstein. welle-6 trägt deshalb einen **beobachtbaren
   Trigger** im Wellen-Block, und die Closure wird fällig, sobald er erfüllt ist.

### Offene Entscheidungen für den Projektinhaber

- **Regel-Kandidat als MR aufnehmen?** *Eine Orakel-Zeile benennt nicht nur die Zusicherung, sondern
  die **Komponente**, an der sie diskriminiert; Zusagen über das Zusammenspiel zweier Komponenten
  werden am **Zusammenspiel** belegt.* Dreimal belegt (053 · 052a · 052b), Sensor wäre die
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse
  „reale Sensor-Deckung" — computational nicht prüfbar, wie [MR-020](../../../../harness/conventions.md).
  **Das ist eine Regelwerk-Entscheidung, nicht meine.**
- **M6 als Meilenstein** habe ich angelegt, weil die Meilenstein-Tabelle eine `Welle(n)`-Spalte führt
  und welle-6 sonst ein Loch hinterlassen hätte. Falls du die Welle ohne Meilenstein willst: eine Zeile.
- **`slice-044b` und `slice-051`** tragen weiter `welle: welle-5-erweiterung` (Provenance; keiner ist
  welle-6-bindend). Bewusst nicht umgehängt.

### Weiter offen, ohne Wellen-Bindung

[`slice-051`](../open/slice-051-review-artefakt-pflicht.md) (Review-Artefakt-Pflicht als Gate — die
Alt-Last aus 045/046a/046b/047 ist damit **nicht** geheilt, nur benannt) ·
[`slice-044b`](../open/slice-044b-golden-import-fremd.md) ·
[`slice-006`](../open/slice-006-drittanbieter-attribution.md) · `slice-039a/b` · `slice-040a`.

---

## 2026-07-27 (5) — welle-5 geschlossen, **welle-6 geschnitten**

Der Planungs-Punkt „viele Slices, keine Wellen" vom 2026-07-26 ist entschieden und vollzogen
(Projektinhaber-Wahl: **welle-5 schließen und eine neue Welle schneiden**).

### Was gebucht wurde

- **[`done/welle-5-results.md`](../done/welle-5-results.md)** geschrieben (6 Abschnitte, Muster
  welle-1..4). **M5 „Erweiterbar" ist erreicht** — inhaltlich seit dem 2026-07-03 mit dem
  Plugin-Strang, festgestellt in [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md):15; die
  Roadmap-Zeile stand bis heute gegen diese `Accepted`-ADR auf „offen".
- **Der Befund steht im Closure-Dokument selbst** (§0), nicht nur hier: die Welle lief **24 Tage über
  ihren erfüllten Meilenstein hinaus** und sammelte am Ende **31 Slice-Pläne** unter einem Label.
  §2 gliedert den Umfang deshalb nach **Strängen** (Plugin · DRW · Export · GUI-Bedienbarkeit ·
  harness-steering), nicht als eine Wellen-Leistung — das ist die ehrliche Beschreibung dessen, was
  passiert ist. Carveout-Audit: keine aktiven; die Welle hat **verschärft**, nicht gelockert.

### welle-6-interaktiv-planen — mit Trigger, nicht nur mit Thema

**Ziel:** [OBJ-001](../../../../spec/lastenheft.md#3-projektziele) — der Benutzer legt **Bauteile in
der Oberfläche** an und bearbeitet sie. **Trigger:** eine **Wand** ist im 2D-Canvas **zeichenbar und
parametrisch änderbar**, ohne Kommandozeile. Neuer Meilenstein **M6 „Interaktiv planbar"**.

**Die Lehre aus welle-5 steht im Wellen-Block selbst:** diese Welle ist zu schließen, **sobald der
Trigger erfüllt ist — nicht, wenn die Arbeit ausgeht**. Was daneben läuft (harness-steering,
Alt-Bestand), ist ausdrücklich **nicht** wellen-bindend und verlängert sie nicht.

**Warum dieses Ziel:** das Benutzerhandbuch führt „ein Gebäude selbst planen" bis heute unter „In
dieser Version noch **NICHT** möglich" und nennt es „den nächsten großen Ausbauschritt". Es ist der
größte Abstand zwischen Produkt-Zweck und Ist-Stand — und die Welle baut direkt auf dem auf, was
gerade fertig wurde: dem interaktiven Canvas (`slice-043`) und der GUI-Kette, die dem Fenster
erstmals Sensoren gegeben hat.

### Eine Kleinigkeit, bewusst nicht still geregelt

[`slice-044b`](../open/slice-044b-golden-import-fremd.md) und
[`slice-051`](../open/slice-051-review-artefakt-pflicht.md) tragen weiter
`welle: welle-5-erweiterung` im Frontmatter, obwohl die Welle zu ist. Ich habe das **nicht**
umgeschrieben: beide wurden während welle-5 geschnitten (Provenance), und keiner von beiden ist
welle-6-bindend — sie in die neue Welle zu ziehen wäre genau die Sammelbecken-Bewegung, die dieser
Schritt beendet. Die welle-5-Closure führt sie in §6 als „übernommen, nicht erledigt".
**Falls du das Frontmatter-Feld lieber nachgezogen hättest: sag Bescheid, es sind zwei Zeilen.**

### Startbare Fäden in welle-6

- **`slice-048b`** (DRW-001-Implementierung, Fang-Punkte) — Plan noch nicht in `open/`; gehört
  inhaltlich hierher, weil Fangen die erste Voraussetzung für präzises Zeichnen ist.
- Der eigentliche Wellen-Kern — **Wand zeichnen im Canvas** — hat noch keinen Plan.

**Nicht wellen-bindend, weiter offen:** `slice-051` (Review-Artefakt-Pflicht), `slice-044b`,
`slice-006`, `039a/b`, `040a`.

---

## 2026-07-27 (4) — slice-052b geschlossen: **die Vierer-Kette ist komplett**

`make gates` **EXIT=0** (docs-check 0 Befunde / 260 Dateien · a-check 0 · arch-check ok ·
**352/352** Tests · Coverage 91,8 %), `make io-smoke` EXIT=0, `make schema-check` EXIT=0.
`in-progress/` ist leer, der Ruhe-Sentinel steht.

**[`slice-052b`](../done/slice-052b-neues-projekt.md) done** (`7674344` Review-Lauf-2-Einarbeitung →
Start → Implementierung → Closure). „Datei → Neu" legt ein Projekt mit **einem Geschoss und einer
Zeichen-Ebene** an, setzt den Sitzungs-Zustand zurück und läuft hinter derselben Rückfrage wie Öffnen
und Beenden. **Lastenheft 0.1.19**, **Benutzerhandbuch 1.3**.

### Damit ist der Validations-Rest von slice-047 abgearbeitet

**054 → 053 → 052a → 052b**, alle vier `done`. Zwischenstand der Kette:

| Slice | Was | Review-Läufe |
|---|---|---|
| **054** | `ManageProjectPort` — die Wurzel der GUI-Findings-Klasse | 1 (2 HIGH) |
| **053** | Fenster + Menü-Handler als Adapter | 2 (2 + 1 HIGH) |
| **052a** | Sitzungs-Zustand, „Speichern", Rückfrage | 4 (8 HIGH gesamt) |
| **052b** | „Neues Projekt" | 2 (3 + 3 HIGH) |

**Jeder** Lauf hat etwas Echtes gefunden. Die letzten drei Läufe fanden jeweils dasselbe Muster:
**ein Plan, der vor seinen Vorläufern geschrieben wurde, bucht auf Artefakten, die es so nicht gibt.**

### Der Regel-Kandidat ist reif — dritte Wiederholung

052a: eine Gegenprobe blieb grün, weil der Test **die falsche Komponente** prüfte (Reihenfolge zwischen
zwei Komponenten, belegt an einer einzelnen). 052b-Lauf-2-HIGH-3 fand **wörtlich dieselbe Bauart**:
Orakel-Zeilen auf `test_project_session.cpp` gebucht, obwohl die Kette seit 052a im
`ProjectMenuHandler` liegt. Davor 053: „eingearbeitet ≠ entschieden".

> **Vorschlag für eine MR** (3×-Regel erfüllt): *Eine Orakel-Zeile benennt nicht nur die Zusicherung,
> sondern die **Komponente**, an der sie diskriminiert. Zusagen über das Zusammenspiel zweier
> Komponenten werden am **Zusammenspiel** belegt, nicht an einer von beiden.* Sensor: die
> [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse
> „reale Sensor-Deckung" — computational (noch) nicht prüfbar, wie [MR-020](../../../../harness/conventions.md).

**Nicht** von mir angelegt — das ist eine Regelwerk-Entscheidung des Projektinhabers.

### Was jetzt offen ist

- **VORRANGIG: „viele Slices, keine Wellen"** (2026-07-26, unten) — welle-5 ist ein Label ohne
  Artefakt, der M5-Trigger laut [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) erfüllt, die
  Roadmap sagt „offen". Die Kette, die den Punkt verdrängt hat, ist jetzt abgeräumt.
- **`slice-048b`** (DRW-001-Impl, Fang-Punkte) — Plan liegt weiter nicht in `open/`.
- [`slice-051`](../open/slice-051-review-artefakt-pflicht.md) — Review-Artefakt-Pflicht als Gate.
- [`slice-006`](../open/slice-006-drittanbieter-attribution.md), 039a/b, 040a, 044b — Alt-Bestand.

---

## 2026-07-27 (3) — slice-052a geschlossen: die Kette ist bis auf 052b durch

`make gates` **EXIT=0** (docs-check 0 Befunde / 259 Dateien · a-check 0 · arch-check ok ·
**341/341** Tests · Coverage 91,7 %), `make io-smoke` EXIT=0, `make schema-check` EXIT=0.
`in-progress/` ist leer, der Ruhe-Sentinel steht.

**[`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md) done** (`f11d76d`
Review-Lauf-4-Einarbeitung → `cd428cd` Start → `acbeddb` Implementierung → Closure). Neuer
`ProjectSessionPort` + `ProjectSessionService`, `= default`-Gleichheit auf allen 13 Modell-Werttypen,
„Speichern" auf die bekannte Datei, dreiwertige Rückfrage vor Öffnen und Beenden. **Lastenheft
0.1.18**, **Benutzerhandbuch 1.2**, Spezifikations-Sammelblock `BLD-002.b`/`003.b`.

**Die Lehre des Tages steckt in einer Gegenprobe, die NICHTS rot machte.** Orakel-Zeile 6
(„gescheitertes Speichern lässt ungesichert") war belegt — dachte ich. Die Gegenprobe
„`markPersisted` **vor** statt nach dem Schreiben" ließ **alle 339 Tests grün**: der Test prüfte den
Sitzungs-Service **isoliert**, während die Zusage an der **Reihenfolge zwischen zwei Komponenten**
hängt (`ManageProjectService` → `ProjectSessionService`). Behoben durch zwei Tests, die den
Fehlschlag durch den **echten** Use-Case führen; danach ist die Gegenprobe rot.

> **Regel-Kandidat:** eine Zusage über eine **Reihenfolge zwischen zwei Komponenten** kann kein Test
> einer einzelnen Komponente belegen — auch wenn er exakt die richtige Eigenschaft prüft. Der
> Orakel-Schnitt hatte die Zeile korrekt benannt; sie lag nur am falschen **Ort**. Das ist die zweite
> Wiederholung dieser Bauart (053: „eingearbeitet ≠ entschieden"; hier: „geprüft ≠ am richtigen Ort
> geprüft"). Beim dritten Mal gehört daraus eine MR.

**Zweite Lehre — beim Bauen entschieden, im Plan abweichend protokolliert:** die Verwerf-Komposition
(Verdikt → fragen → auswerten → ggf. speichern) sollte laut Plan in `main.cpp` liegen. Genau dort
wäre sie wieder orakel-los gewesen. Sie ist als `ProjectMenuHandler::mayDiscard(ask, ask_target)` in
den geprüften Adapter gewandert; `main.cpp` reicht nur die zwei **Dialoge** herein.

**Weiter in der Kette:** ▶ **[`slice-052b`](../done/slice-052b-neues-projekt.md)** („Neues Projekt",
macht [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) erstmals
benutzer-erfüllbar) — **zweiter
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
offen**, und er muss dieselbe Frage stellen wie die Läufe zu 053 und 052a: bucht 052b auf dem, was
054/053/052a **wirklich** geliefert haben? Bekannt ist bereits, dass 052b noch auf
`src/adapters/ui/view/main_window.*` als zu ändernde Datei zeigt; dazu kommen jetzt der
`ProjectSessionPort` (Reset der Baseline bei „Neu") und die `FileActions`-Erweiterung.

**Unberührt geblieben:** der Planungs-Punkt „viele Slices, keine Wellen" (unten, 2026-07-26).

---

## 2026-07-27 (2) — slice-053 geschlossen: das Fenster hat einen Sensor

`make gates` **EXIT=0** (docs-check 0 Befunde / 258 Dateien · a-check 0 · arch-check ok ·
**301/301** Tests · Coverage 91,5 %), `make io-smoke` EXIT=0, **`make acc-002-beleg` EXIT=0**
(1276×753, 9 Wand-Netze). `in-progress/` ist wieder leer, der Ruhe-Sentinel steht.

**[`slice-053`](../done/slice-053-fenster-als-adapter.md) done** (`77f7455` Review-Lauf-2-Einarbeitung
→ `826b3e5` Start → `6c44d73` Implementierung → Closure). `MainWindow` (`ui/view/`, port-frei) +
`ProjectMenuHandler` (`ui/command/`, am 054-Port). Damit existiert die **erste reale
`ui_command`-Datei am Port** — den architektonischen Beleg, den 054 offen ließ, führt jetzt `a-check`
an einem Artefakt.

**Der zweite Review-Lauf hat eine stille Falle gefangen, die ich selbst gelegt hatte.** Der 054-Port
trug `sinks = {}`. Ein Handler, der die Senken vergisst, hätte kompiliert und eine gültige
`Resolution` geliefert — der Verlust der Zeichen-Ziel-Neuauflösung (**slice-047-Verify-B4, exakt
derselbe Fehler**) wäre unter **allen** zugesagten Orakeln grün geblieben. Der Default ist gefallen;
gemessen ist das Weglassen jetzt ein **Compile-Fehler**. Weil der Compiler das Durchreichen *leerer*
Senken nicht sieht, kam Orakel-Zeile 6 dazu.

**Netz-Lehren:**

1. **Ein Default-Argument kann ein Orakel aushebeln.** `= {}` an einem Port-Parameter macht das
   Vergessen unsichtbar. Wo ein Parameter *verhaltenstragend* ist, ist der fehlende Default der
   billigste und schärfste Sensor, den es gibt — er kostet nichts und wirkt vor jedem Test.
2. **„Eingearbeitet" ist nicht „entschieden".** Lauf-1-MEDIUM-4 galt als erledigt, hatte die
   Entscheidung aber nur in den Vollzug verschoben („Die Antworten stehen im Plan-Vollzug"). Der
   zweite Lauf hat das gefangen. **Eine Finding-Behandlung, die auf später zeigt, ist keine.**
3. **Ein Plan, der vor seinem Vorläufer geschrieben wurde, muss gegen das Gelieferte geprüft werden**
   — nicht gegen das Erwartete. Genau dafür war Lauf 2 da, und er hat sich gelohnt.

**Weiter in der Kette:** ▶ **[`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md)**
(Sitzungs-Zustand + „Speichern" + Rückfrage) — **vierter
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
offen**. Er muss dieselbe Frage stellen wie Lauf 2 zu 053: bucht 052a auf dem, was 053 **wirklich**
geliefert hat? Konkret nachzuziehen (bewusst nicht auf Vorrat erledigt): 052a verweist noch auf
`src/adapters/ui/view/main_window.*` als zu ändernde Datei und auf den verworfenen
`sendEvent`-Mechanismus; die `CloseGuard`-Naht heißt jetzt so und ist unbesetzt. Danach 052b.

**Unberührt geblieben:** der Planungs-Punkt „viele Slices, keine Wellen" (unten, 2026-07-26).

---

## 2026-07-27 — slice-054 geschlossen (die Wurzel der GUI-Findings-Klasse)

`make gates` **EXIT=0** (docs-check 0 Befunde / 257 Dateien · a-check 0 · arch-check ok ·
**291/291** Tests · Coverage 91,5 %), `make io-smoke` EXIT=0, `make schema-check` EXIT=0.
`in-progress/` trägt **keinen** Slice mehr — der Ruhe-Sentinel steht wieder.

**[`slice-054`](../done/slice-054-manage-project-port.md) done** (`8546bac` Review-Einarbeitung →
`4f9aeeb` Start → `3ee4dd2` Implementierung → Closure): der `ManageProjectPort` ist realisiert. Die
vierfach wiederholte Findings-Klasse (047-B4 · 052 · 052a · 053) ist damit an der **Wurzel** erledigt,
nicht zum fünften Mal einzeln behandelt.

**Der [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
hat den Slice gerettet, nicht nur kommentiert** ([Report](../../../reviews/2026-07-27-slice-054-plan.md),
2 HIGH · 5 MED · 4 LOW · 2 INFO, Verdikt „nicht startbar"): der Plan hätte einen Port-Header mit
`ProjectRepositoryPort`- und `StructureEditService`-Parametern gebaut — `ports_driving` darf nur `model`
importieren. Die Gegenprobe der Closure hat es dann **gemessen**:
`manage_project_port.h:7: wrong-direction: ports_driving -> ports_driven`. Ohne das Review wäre der
Slice mit rotem `make a-check` gegen die Wand gelaufen.

**Netz-Lehren:**

1. **Ein Driving Port zwingt die Infrastruktur in den Konstruktor.** Das ist keine Stilfrage — es ist
   die einzige gate-legale Form, und **genau sie** macht den Vertrag adapter-tauglich. Wer die
   Abhängigkeiten in der Signatur lässt, baut einen Port, den kein Adapter rufen darf.
2. **Der stärkere Invarianz-Beleg ist Nicht-Änderung.** Die fünf Bestands-Testdateien blieben
   **byte-unverändert** (der `using`-Alias trägt die Wortgleichheit). Ein Test, der im selben Slice
   mitumgebaut wird, kann seine eigene Regression nicht mehr zeigen.
3. **Reviewer-Befunde sind Hypothesen, keine Urteile.** LOW-2 („Klammerform `.{h}` uneinheitlich") war
   **falsch** — gemessen: der explizite Pfad meldet `codepath-missing`, weil `codepaths.roots` `src`
   enthält. Die Klammer ist das Ventil für geplante Dateien. Widerlegt statt eingearbeitet.

**Weiter in der Kette:** ▶ **[`slice-053`](../done/slice-053-fenster-als-adapter.md)** (Hauptfenster als
testbarer Adapter) — **zweiter [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
noch offen**; er muss zusätzlich prüfen, ob 053 auf dem Port aufsetzt, den 054 **tatsächlich**
geliefert hat (Sinks als Methoden-Parameter, `save` ohne `Building`). Danach 052a → 052b.

**Unberührt geblieben:** der Planungs-Punkt „viele Slices, keine Wellen" (unten, 2026-07-26) — er
wartet weiter auf die Projektinhaber-Entscheidung.

---

## Tagesabschluss 2026-07-26 — Stand + nächste-Sitzung-Zeiger

`make gates` **EXIT=0** (docs-check **0 Befunde / 256 Dateien** · a-check 0 · arch-check ok ·
**285/285** Tests · Coverage 91,4 %), `make io-smoke` EXIT=0, `make schema-check` ok.
Alles committet; **lokal auf `main`** (14 Commits seit `2570c99`), **nicht gepusht**.
`in-progress/` trägt **keinen** Slice — der Ruhe-Sentinel steht.

### Zuletzt geschlossene Slices

*(Slice-Ebene gehört hierher, nicht in den `## Aktuelle Welle`-Block der Roadmap — die führt die
**Wellen**-Sequenz. Die Wellen-Closure selbst steht am Wellen-Ende in `done/welle-N-results.md`.)*

### Was heute abgeschlossen wurde

- **slice-047 geschlossen** (`a708d1b` Findings → `a3406ed` Closure). Verify-Verdikt „nicht fertig"
  abgearbeitet: die Neu-Auflösung des Zeichen-Ziels wanderte aus dem orakel-losen `main.cpp` in den
  Use-Case, die vier ungedeckten Id-Zähler bekamen ein Orakel, **Lastenheft 0.1.17**
  ([`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) Outline → AK) und
  **Benutzerhandbuch 1.1**. **Validation (Projektinhaber): angenommen mit benanntem Rest.**
- **slice-049 geschlossen** (`ec89ed0` Purge+Härtung in **einem** Commit → `ed663ab` Closure):
  56 »welle-N«-Fundstellen aus den drei Spec-Straten, Regel computational über die neue
  `matrix`-Klasse `temporal` = **[MR-023](../../../../harness/conventions.md)**. Belegt: 56 Befunde
  vorher, 0 nachher.

### Offener Planungs-Punkt (Projektinhaber-Befund 2026-07-26): „viele Slices, keine Wellen"

**Belegt, nicht gemeint:**

- **welle-5 existiert als drei Zeilen** in [`roadmap.md`](roadmap.md) (Welle-ID im
  `## Aktuelle Welle`-Block · Meilenstein-Tabellen-Zeile · Mermaid-Knoten) plus dem `welle:`-Feld in
  ~19 Slice-Frontmattern. **Keine** Scope-Definition, **keine** Slice-Liste, **kein**
  Abschluss-Kriterium außer dem M5-Trigger. welle-1..4 haben je ein `done/welle-N-results.md`; für
  welle-5 gibt es kein Artefakt.
- **Der M5-Trigger ist erfüllt, die Tabelle sagt „offen".**
  [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) (Accepted, immutabel) stellt fest: „M5
  „Erweiterbar" ([OBJ-004](../../../../spec/lastenheft.md#3-projektziele)) ist mit dem Plugin-Strang
  ([ADR-0017](../../adr/0017-plugin-api-abi.md)) **inhaltlich geliefert**".
  [`roadmap.md`](roadmap.md) führt M5 dagegen als **offen**. Doku-Unwahrheit derselben Klasse wie
  slice-047-F4/V1.
- **Zwölf Slices seit dem 2026-07-02** (043–054) unter einem Label, thematisch unverbunden:
  2D-Canvas · Golden-Files · Export-Provenance · Projekt-Persistenz im GUI · vier
  harness-steering-Quergewerke · die GUI-Architektur-Kette. Eine Welle hat ein Ziel und ein Ende;
  das hier ist ein Sammelbecken.

**Zu entscheiden (Projektinhaber, nicht der Agent):**

1. **welle-5 schließen** — M5 buchen, `done/welle-5-results.md` schreiben, die Folge-Arbeit in eine
   **neu benannte** Welle mit eigenem Ziel und Trigger schneiden.
2. **Die Wellen-Fiktion beenden** — die Roadmap sagt ehrlich, dass seit M4/M5 **strang-basiert**
   gearbeitet wird (DRW · GUI-Bedienbarkeit · harness-steering), und die Wellen-Sektion beschreibt nur
   noch die **Historie**. Dann fällt auch das `welle:`-Frontmatter-Feld zur Disposition.
3. **Bewusst so lassen** — dann muss mindestens die M5-Zeile korrigiert werden, sonst bleibt die
   Roadmap gegen eine `Accepted`-ADR unwahr.

**Nicht heute entschieden** — Tagesabschluss. Der Punkt hat Vorrang vor dem Start eines weiteren
Slice, weil er die Buchführung betrifft, unter der jeder Slice läuft.

### Startbare Fäden (Projektinhaber-Wahl)

- **`slice-048b`** — DRW-001-Impl (Fang-Punkte); laut slice-049-Sequenz an der Reihe, **Plan liegt
  noch nicht in `open/`**.
- [`slice-051`](../open/slice-051-review-artefakt-pflicht.md) — Review-Artefakt-Pflicht
  (Gate-Nachzug der [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report-Alt-Last).
- die **Vierer-Kette** unten (aus der slice-047-Validation).

### Was offen in `open/` liegt — und in welcher Reihenfolge

**Sequenz: [`slice-054`](../done/slice-054-manage-project-port.md) →
[`slice-053`](../done/slice-053-fenster-als-adapter.md) →
[`slice-052a`](../done/slice-052a-sitzungs-zustand-und-speichern.md) →
[`slice-052b`](../done/slice-052b-neues-projekt.md).**

Alle vier stammen aus dem **Validations-Rest von slice-047** („Speichern" auf die offene Datei +
Warnung vor ungesicherten Änderungen). Sie sind das Ergebnis von **sechs**
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufen
mit **acht HIGHs** — jeder Lauf hat etwas Echtes gefunden, **keine Zeile Code ist gefallen**.

| Slice | Was | Stand |
|---|---|---|
| **054** | `ManageProjectPort` realisieren (die in `architecture.md`:76 seit dem Bootstrap deklarierte Ziel-Form) | Plan fertig, **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) offen** |
| **053** | Hauptfenster als testbarer Adapter (`ui/view/` port-frei + Handler in `ui/command/`) | ein [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) durch (2 HIGH eingearbeitet), **zweiter Lauf offen** |
| **052a** | Sitzungs-Zustand über **Vergleich** + „Speichern" + Rückfrage bei Öffnen/Beenden | drei [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) durch, **vierter offen** (nach 053) |
| **052b** | „Neues Projekt" — macht [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen) erstmals benutzer-erfüllbar | ein [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) durch, **zweiter offen** |

### Der Befund, der die Kette erklärt (nicht wieder herleiten)

`.a-check.yml` erlaubt **keinem** Adapter, `hexagon/services/` zu rufen (`ui_view` → `model`/`ports_driven`,
`ui_command` → `model`/`ui_view`/`ports_driving`). Die Projekt-Use-Cases sind aber **Services ohne
Port** — also darf sie nur `main.cpp` rufen, und dort ist alles orakel-los. **Deshalb** landete seit
slice-047 in jedem GUI-Slice dieselbe Klasse Finding (047-B4 · 052-Lauf-1-MED-2/3 ·
052a-Lauf-3-HIGH-1 · 053-HIGH-1). Kein Disziplin-Problem — ein **fehlender Port**.
`spec/architecture.md`:76 nennt die Auflösung seit dem Bootstrap; **slice-054 vollzieht sie.**

### Drei Netz-Lehren von heute

1. **`main.cpp` ist orakel-los per Konstruktion** — in kein Testbinary gelinkt
   (`src/CMakeLists.txt`:6), coverage-ausgenommen, `io-smoke` kehrt vor dem GUI-Aufbau zurück. Die
   Frage ist nie „wie teste ich `main`?", sondern „gehört dieser Schritt dorthin?".
2. **Eine Prüfung findet nur, was ihr Maßstab enthält.** Plan-Review, Code-Review und Verify haben
   alle das falsche Benutzerhandbuch übersehen, weil die DoD-Doku-Zeile nur `CHANGELOG` + `spec/`
   nannte. **Regel: ein Slice, der eine Anforderung benutzer-erfüllbar macht, führt `docs/user/` in
   seiner Doku-DoD-Zeile.** Gefunden hat es der Projektinhaber, nicht der Prozess.
3. **Ein vorab entschiedener Orakel-Schnitt schützt nicht vor allem.** Zwei der acht HIGHs lagen
   **außerhalb** dessen, was ein Orakel je gezeigt hätte: eine tote Funktion mit grünen Orakeln, und
   ein versprochenes Orakel, das der eigene Datei-Plan nicht hergab.

### Offene Prozess-Punkte

- **`git add -A` vermeiden**, Pfade einzeln stagen (Herkunft: `17da627` nahm Fremd-Änderungen mit).
- **Rollen-Skills:** `.harness/skills/` trägt reviewer · verifier · validator (v1.0);
  **Planner · Architect · Implementation fehlen** — nimmt `slice-051` sie auf? Frage dahinter:
  **Delta vs. Duplikat** zu AGENTS.md. Kein `make verify`-Target.
- **Alt-Last unbelegbar:** für slice-045/046a/046b/047 existiert **kein** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report, obwohl die
  Plan-Köpfe Ergebnisse behaupten. Nachträgliche Reports wären Fälschung → Gate-Nachzug in
  [`slice-051`](../open/slice-051-review-artefakt-pflicht.md).

---

## Tagesabschluss 2026-07-25 — Stand + nächste-Sitzung-Zeiger

`make gates` **EXIT=0** (docs-check 0 Befunde / 246 Dateien · a-check 0 · arch-check ok ·
**280/280** Tests · Coverage 91,4 %). Alles committet; **lokal auf `main`, Push nach eigenem Ermessen.**



### Werkzeug-Lehren von heute (nicht wieder herleiten)

- Der **wirksame `A_CHECK_IMAGE`-Pin** steht im `Makefile`, **nicht** in `a-check.mk`: die Zuweisung
  steht **vor** dem `include`, dessen `?=` ist ein No-op. `make -n a-check` zeigt die Wahrheit.
- **`constructs.adapter` ist ein Teilstring-Vergleich** → Zone **mit Schrägstrich** notieren.
- **`make doc-repair | git apply` scheitert** (kontextlose Hunks) → `git apply --unidiff-zero`; der Patch
  ist **sichtprüfpflichtig** (verlinkt aufs Verzeichnis, repo-fremde Kennungen falsch).
- **`ignore-refs` ist seit d-check v0.49.0 querschnittlich** (`links`/`anchors`/`codepaths`) — damit sind
  Referenzen aus **unveränderlichen** MR/ADR tombstone-bar, ohne sie zu editieren.
- **Regelwerk-Bezug:** gepinnte Release-ZIP `lab-regelwerk.zip` (v1.3.0) ziehen und selbst lesen. Die
  Migration auf **v3.5.2** steht aus; bis dahin ist die committet-vendored Baseline nach u-boot-Muster
  **zurückgestellt** (Projektinhaber 2026-07-25).
  
  
### Was heute fertig wurde

- **[`slice-050`](../done/slice-050-tooling-konsolidierung-dcheck.md) GESCHLOSSEN** — Tooling-Konsolidierung.
  `gate-consistency.sh` + `idlink.py` retired (d-check-Modul `targets` bzw. `make doc-repair`);
  **arch-check-Regel P1** → a-check **v0.16.0 `constructs`**. `tools/arch-check.sh` **lebt weiter** mit
  Regel **P2** (geschlossene Import-Allowlist, kanten-basiert nicht abbildbar). Zwei neue MRs
  ([MR-021](../../../../harness/conventions.md) P1, [MR-022](../../../../harness/conventions.md) targets/repair);
  **keine** Gate-Lockerung ⇒ kein ADR.
- **slice-047b implementiert** + Code-Review vollständig eingearbeitet (s. o.).

### Wo morgen angesetzt wird — Reihenfolge zwingend

1. **Verify-Findings einarbeiten** — Report liegt:
   [`2026-07-25-slice-047-verify.md`](../../../reviews/2026-07-25-slice-047-verify.md).
   **Verdikt: nicht fertig.** 11 DoD-Zeilen — **8 bestätigt, 0 widerlegt, 3 unbelegt**. Die Zeilen
   sind also nicht falsch, aber drei tragen kein Orakel. Offen:
   - **B4 (unbelegt):** `reresolve_after_open()` in `main.cpp` wird von **keinem** Sensor ausgeführt
     (main.cpp ist in kein Testbinary gelinkt, coverage-ausgenommen, `io-smoke` kehrt vor dem
     GUI-Aufbau zurück). Gegenprobe des Verifiers: Aufruf entfernt → **280/280 grün**. Der
     Adapter-Test ruft die Setter selbst nach, prüft also nicht den Produktions-Aufruf.
   - **B5 (unbelegt):** das Datei-Menü selbst hat keinen Sensor (modaler Dialog, bewusst).
   - **B2 (teil-ungedeckt):** die Zusage „**jeden** Id-Zähler" ist nur für 5 von 9 diskriminierend —
     Resets für opening/roof/slab/stair entfernt → 280/280 grün.
   - **C1 (erledigt 2026-07-25):** die Spec normierte die Export-Quelle als „via `--open`/**GUI**
     geöffnetes Projekt" — einen GUI-Export gibt es **nicht**. Stammte aus `70a30f1`, korrigiert.
   - **Anforderungs-Ebene:** trägt für [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern).
     Für [`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) ist die Korrektur
     **formal unbelegbar**, solange die Anforderung im Lastenheft **Outline ohne AK** ist — „benutzer-
     erfüllbar?" hat dort kein Maß. **Entscheidung nötig:** AK-Schärfung nachziehen (Muster slice-048a,
     [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) oder
     die Grenze im Closure-Text benennen.
2. **Validation** (Rolle: **Projektinhaber**, [`.harness/skills/validator.md`](../../../../.harness/skills/validator.md)).
   Übergabe-Artefakt nach Modul 8: Build-Ergebnis + Slice-Resultat. Konkret die Frage, die weder
   Review noch Verify beantworten: *ist Projekt-Öffnen so, wie es jetzt ist, für einen Benutzer
   brauchbar?* Die Entscheidung geht in die Closure-Notiz.
3. **Closure slice-047:** §8-Notiz, `git mv` `in-progress/` → `done/` **mit** Ruhe-Sentinel-Toggle im
   **selben** Commit ([MR-017](../../../../harness/conventions.md), [AGENTS §2.8](../../../../AGENTS.md)),
   Roadmap nachziehen. Achtung: der Sentinel muss beim Schließen **zurück** in den
   `## Aktuelle Welle`-Block (heute wurde er beim Öffnen entfernt).

### Danach frei wählbar

- **[`slice-049`](../done/slice-049-spec-straten-prozess-rein-welle.md)** (welle-Purge + `matrix`-Härtung) —
  berührt `.d-check.yml`, das slice-050 heute umgebaut hat: **Konflikte prüfen**.
- **[`slice-051`](../open/slice-051-review-artefakt-pflicht.md)** (neu, Skelett) — Review-Artefakt-Pflicht
  computational. **Offene Frage vom Projektinhaber:** ob dieser Slice auch die **fehlenden Rollen-Skills**
  (Planner/Architect/Implementation) aufnimmt oder ob dafür ein eigener Slice geschnitten wird.
- **slice-048b** (DRW-001-Impl) — der eigentliche Feature-Fortschritt der DRW-Kampagne.

### ⚠ Offener Klärungspunkt: Fremd-Änderungen in `17da627`

Der Commit `17da627` (Titel: Code-Review-Findings) enthält Änderungen, die **nicht** zum Slice gehören
und **nicht** vom Implementer stammen: `roadmap.md` wurde von 372 auf 94 Zeilen gekürzt und der Inhalt in
zwei **neue, von nirgends verlinkte** Dateien ausgelagert — `status.md` und **`d-ckeck.md`**
(Tippfehler im Dateinamen). Ursache: ein `git add -A` des Implementers hat parallel im Arbeitsbaum
entstandene Fremd-Änderungen mitgenommen; die Commit-Message beschreibt sie nicht.

**Zu klären (Projektinhaber):** stammen `status.md`/`d-ckeck.md` von dir, und sollen sie bleiben? Falls
ja: Tippfehler im Dateinamen korrigieren und beide verlinken (sonst sind es tote Dokumente — kein Gate
fängt eine unverlinkte, aber existierende Datei). Falls nein: Revert des Doku-Teils von `17da627`.
**Lehre für den Implementer:** kein `git add -A`, wenn der Arbeitsbaum fremde Änderungen tragen kann —
Pfade einzeln stagen.

### Prozess-Befunde von heute (Alt-Last, bewusst NICHT geheilt)

- **Fehlende Plan-Review-Reports** für slice-045/046a/046b/047: die Pläne behaupten
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Ergebnisse,
  ohne dass ein Artefakt unter `docs/reviews/` liegt. Regelwerk **Modul 10** verlangt „ein Report **pro
  Lauf**". Rückwirkende Reports wären eine **Fälschung der Audit-Spur** — sie werden nicht geschrieben;
  der Gate-Nachzug liegt in slice-051.
- **Negativbefund-Zeilen** („geprüft, ohne Befund") tragen nur **13 von 101** Alt-Reports. Die neuen
  Skills fordern sie ein; der Alt-Bestand bleibt, wie er ist (dieselbe Fälschungs-Grenze).
- **Rollen-Skills** nach Modul 8 (»eine Person darf alle Rollen spielen — aber mit unterschiedlichen
  Skill-Dateien«): **Reviewer · Verifier · Validator** liegen jetzt unter `.harness/skills/`;
  **Planner · Architect · Implementation** fehlen noch. Deren Inhalte überschneiden sich stark mit
  `AGENTS.md` (das laut Modul 9 ohnehin in **jeden** Lauf-Kontext gehört) — die zu entscheidende Frage
  ist **Delta vs. Duplikat**, nicht ob.
- **Kein `make verify`-Target.** Modul 11 zeigt das Muster (DoD-Aussage → Operationalisierung →
  `verify-*`-Sub-Target, bewusst **nicht** in `make gates`). Die DoD-Prüfung ist derzeit rein
  inferentiell — laut Modul 11 „die am wenigsten ausgereifte" Schicht.


### ▶ Tagesabschluss 2026-07-24 — Stand & nächste Sitzung

**Heute geliefert (alles auf `main`, `make gates` grün, Arbeitsbaum sauber):**

- **`slice-047a` done** (`70a30f1`) — Projekt-Persistenz-CLI (`--save`/`--open`) + echte Export-Quelle.
- **Benutzerhandbuch v0.1.0** (`f202249`).
- **`slice-048a` done** (`855fe7a`) — DRW-001 Fangpunkte **AK-Schärfung** (Lastenheft 0.1.16, spez. §1); erste
  Anforderung der **DRW-Aids-Kampagne** (alle offenen `DRW-*` durcharbeiten: 001→002→003→004→007).
- **`slice-049` GEPLANT** (`145ae20`) — **Spec-Straten prozess-rein**: »welle-N«-Purge (3 Straten, 58 Stellen) +
  `matrix`-Gate-Härtung (`temporal`-Klasse `token '[Ww]elle-\d'`; Mechanik fixture-bestätigt).
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) **0 HIGH**.
- **[`slice-050`](../done/slice-050-tooling-konsolidierung-dcheck.md) done** (`4146cda`→`9e52f02`) —
  **Tooling-Konsolidierung**: `gate-consistency.sh` → d-check-Modul `targets` (bidirektional, also
  **strenger** als das abgeloeste Skript); `idlink.py` → `make doc-repair`; **arch-check-Regel P1** → a-check
  **v0.16.0 `constructs`** (Roh-Text-Monopol — ein Kanten-Pruefer gatet erstmals einen **Aufruf**).
  **`arch-check.sh` lebt weiter** mit Regel **P2**: die geschlossene Import-Allowlist ist *deny-by-default*
  und kanten-basiert nicht abbildbar (ein Ziel ohne Schicht bleibt bei a-check **unbeurteilt**).
  Zwei neue MRs: [MR-021](../../../../harness/conventions.md) (P1) + [MR-022](../../../../harness/conventions.md)
  (`targets`/`--repair`); **keine** Gate-Lockerung ⇒ kein ADR.
  Zwei [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Reviews
  (Ur-Plan 3 HIGH; Teil-D-Nachtrag 3 HIGH) — der Nachtrag hat den Schnitt von „arch-check ganz retiren" auf
  „nur P1" **verkleinert**.

**Kontext-Erkenntnis (Tooling-Audit):** d-check (gepinnt v0.51.1) ist fähiger als die Konfig-Kommentare
vermuten lassen — Fähigkeiten IMMER am gepinnten Handbuch (`d-check`-Repo `docs/user/benutzerhandbuch.md`)
prüfen, nicht inferieren.

**▶ NÄCHSTE SITZUNG — zwei startbare Fäden (freie Wahl, je eigenes Impl-`git mv` open→in-progress):**

1. **`slice-049`** ausführen (welle-Purge + `matrix`-Härtung) — Purge + `.d-check.yml`-`temporal`-Klasse + neue
   MR im **selben Commit** (Regel darf nicht vor der Bereinigung grün sein).
2. **`slice-048b`** (DRW-001-**Impl**: Fangen im Canvas — `PlanView`-Fang-Punkte + Bildschirm-Schwellwert +
   Maus-Auswahl + `QMouseEvent`-AK; [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) einschlägig), dann DRW-002/003/004/007.

*Empfehlung:* 049 ist ein kleiner, in sich geschlossener Gate-/Hygiene-Slice (berührt `.d-check.yml`, das
slice-050 gerade angefasst hat — Konflikte prüfen); 048b ist der eigentliche Feature-Fortschritt der
DRW-Kampagne. Reihenfolge nach Priorität des Projektinhabers.

**Aus slice-050 mitzunehmen (Werkzeug-Lehren):** der wirksame `A_CHECK_IMAGE`-Pin steht im `Makefile`, nicht
in `a-check.mk` (dessen `?=` ist nach der Include-Reihenfolge ein No-op); `make doc-repair | git apply`
scheitert an kontextlosen Hunks → `git apply --unidiff-zero`, und der Patch ist **sichtprüfpflichtig**;
`ignore-refs` ist seit d-check v0.49.0 querschnittlich und damit das Mittel, Referenzen aus
**unveränderlichen** MR/ADR zu tombstonen statt sie zu editieren.

---

**[`slice-048a`](../done/slice-048a-drw-001-fangpunkte-ak-spec.md) done** (2026-07-24): **DRW-001 Fangpunkte —
AK-Schärfung** (DRW-Aids-Strang, erste offene DRW-Anforderung der Kampagne). Lastenheft **0.1.16**
([`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) Outline → AK: die gezeichnete Position rastet
**exakt** auf einen **sichtbaren** Endpunkt [Wand-Achse/Hilfslinie] ein, sobald der Cursor nahe genug ist —
interaktiv beobachtbar seit dem Canvas [slice-043]; Boundary außerhalb → frei, Anfang=Ende → bestehende
Ablehnung; Negative kein sichtbarer Fang-Punkt) + spez. §1 [`LH-FA-DRW-001.a`](../../../../spec/lastenheft.md#lh-fa-drw-001) (Fang-Aid = **UI-Zustand**,
Fang-Punkte aus der **bestehenden** `PlanView` → **kein** neues Schema/`op`/Port/Kante). Parametrisiert auf
[ADR-0019](../../adr/0019-drw-2d-canvas.md) Entscheidung 6 (**kein** neuer Grundsatz-ADR);
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
**0 HIGH / 2 MED / 3 LOW / 2 INFO** ([Report](../../../reviews/2026-07-24-slice-048a-plan.md), MED/LOW
eingearbeitet). `make gates` grün, `schema-check` byte-unberührt. **▶ Folge:** slice-048b (DRW-001-**Impl**:
Fang-Punkte aus `PlanView` + Bildschirm-Schwellwert + Maus-Auswahl + `QMouseEvent`-AK), danach die Kampagne
DRW-002 Raster → 003 Winkel → 004 Bemaßung → 007 Gruppen. **Quergewerk eingereiht (Projektinhaber 2026-07-24):**
slice-049 **welle-Purge + `matrix`-Härtung** — Prozess-Vokabular »welle-N« aus den drei Spec-Straten
(`lastenheft`/`spezifikation`/`architecture`, 58 Fundstellen) entfernen **und** das `matrix`-Gate so schärfen,
dass »welle-\d« in Spec-Straten künftig als Fehler erkannt wird (heutige Lücke: `matrix` kennt nur
`slice-\d{3}`).

**[`slice-046a`](../done/slice-046a-export-provenance.md) done** (2026-07-24): **Export-Herkunft** (Datum/Version
injizierbar + sichtbar im PDF-Footer, in STEP/STL-Header) — Exporte verschiedener Stände sind nun **unterscheidbar**
(SOURCE_DATE_EPOCH-Muster: Writer clock-frei, Root injiziert echt, Golden fix). **[`slice-045`](../done/slice-045-pdf-info-metadaten.md)**
(statische PDF/PNG-Metadaten) darin **gefaltet-retired**. Zwei unabhängige [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
(046: 1 HIGH → Option A). **[`slice-046b`](../done/slice-046b-png-titelblock.md) done** (2026-07-24): **sichtbarer
PNG-Titelblock** (self-rolled 5×7-Font + `tEXt`) — Herkunft jetzt sichtbar in **PDF *und* PNG**; visuell verifiziert.
Damit ist der Export-Provenance-Strang für **PDF/PNG/STEP/STL** komplett. **Offen (Deferral,
[MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)):** IFC-`FILE_NAME`/DXF-Provenance-Nachzug;
**echte „Quelle"** via [`slice-047`](../done/slice-047-projekt-oeffnen.md) („Projekt öffnen"). `make gates` grün
(266 Tests). **Prozess-Fix:** `make golden-regen` mount-frei (`tar`-Stream → Golden gehören dem User, nicht root).

**[`slice-044a`](../done/slice-044a-golden-export-infra.md) done** (2026-07-24): Byte-Golden aller 6 Export-Formate
+ STEP-Header-Fix (byte-deterministisch), [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
0 HIGH + [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH,
`make gates` grün (262 Tests). Folge-Naht [`slice-044b`](../open/slice-044b-golden-import-fremd.md)
(Import-Golden-fremd) offen.

**Der DRW-Interaktiv-Strang ist v1 FERTIG** — [`slice-043`](../done/slice-043-drw-canvas-impl.md) **done**
(2026-07-23; Schritt 5 der Roadmap, der ursprüngliche Endpunkt): interaktiver 2D-Zeichen-Canvas
([ADR-0019](../../adr/0019-drw-2d-canvas.md)). Neues 2D-`view/`-`CanvasWidget` (`QWidget`/`QPainter`) + reiner
`ViewTransform`-`screenToModel`-Werttyp + Read/Schreib **port-frei** über `ui/command/` (Option A: `std::function`-
Verdrahtung, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-HIGH-1
gelöst → **kein** `ui_view → ports_driving`, `.a-check.yml` unverändert) + `QTabWidget`-3D/2D-Umschaltung +
Headless-`QMouseEvent`-AK. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
1 HIGH (gelöst) + [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
0 HIGH; 256 Tests, Coverage 91,2 %. **Benannte [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Re-Eval-Trigger**
(Fang/Raster/Winkel, interaktive Geschoss-/Ebenen-Auswahl, Bauteile zeichnen, Selektion/Picking/Bemaßung, DRW-`op`)
= **eigene spätere Slices** (kein Skelett — im ADR + ADR-Index verankert).

**Die [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Export-Refactor-Familie
ist KOMPLETT** (`slice-042a…042d` **done** 2026-07-23; das reservierte `slice-042e` in die 042d-Closure
**gefaltet + retired**, da kein struktureller Diff verblieb):
- **042a** Kern-Naht (`DerivedGeometry`-Vertrag + `StepBox`/`translateMeshZ`→`model/`).
- **042b** 2D-Projektion in den Kern + `PlanViewPort` — **DRW-Canvas architektonisch entsperrt**.
- **042c** STEP/STL-Body-Migration; `geometry → services_geo`-Kante weg.
- **042d** Persistenz-`rise` kern-geliefert (`PersistedDerivations`); `persistence → services_geo`-Kante weg.

**Ergebnis (maschinell belegt):** **alle** Adapter→Kern-Kanten sind weg — `services_geo` trägt nur noch
`services → services_geo` (`make a-check` = 0 Befunde); `architecture.md` §2-Tabelle + §1-Diagramm doku-wahr.
Je Slice [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
+ [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) **0 HIGH**;
verhaltens-invariant (254 Tests, Coverage 91,5 %).

**Schritt 5 (Canvas-Impl) = der ursprüngliche Roadmap-Endpunkt** — **erreicht** (s. o.,
[`slice-043`](../done/slice-043-drw-canvas-impl.md) done).

---

### ▶ Golden files: [`slice-044a`](../done/slice-044a-golden-export-infra.md) **done** · [`slice-044b`](../open/slice-044b-golden-import-fremd.md) **offen** (045/046a s. oben)

**Split ausgeführt** (Reviewer-Empfehlung, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
0 HIGH, [Report](../../../reviews/2026-07-23-slice-044-plan.md)). Ziel: byte-genaue Golden files als Netz gegen
**Encoder-Drift bei unveränderter Semantik** (komplementär zu den vorhandenen Decode-Orakeln).

- **044a — done 2026-07-24:** Export-Golden **alle 6** (byte-exakt: IFC/DXF/PDF/PNG; **STEP** via Adapter-
  `FILE_NAME`-Header-Fix [OCC `APIHeaderSection_MakeHeader`+`Apply`, byte-verifiziert]; **STL** OCC-versions-gebunden,
  Caveat) + `BCAD_TEST_GOLDEN_DIR`-Compile-Def + `.gitattributes` (binär) + `make golden-regen`/`golden-check` (Muster
  `schema-regen`/`schema-check`, CI-only) + dedizierter `golden_gen` mit geteilter `goldenModel()`-TU. **262 Tests**
  (+6 `GoldenExport.*`), `golden-check` grün. [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  0 HIGH (STEP-Fix header-only, DATA unberührt; Determinismus empirisch bestätigt).
- **044b — offen (Skelett in `open/`):** Import-Golden-**fremd** (nur IFC+DXF haben Import): IFC aus [buildingSMART](https://github.com/buildingSMART/Sample-Test-Files)
  (CC-BY-4.0), DXF aus [ezdxf](https://github.com/mozman/ezdxf)/[ixmilia](https://github.com/ixmilia/dxf) (MIT).
  **MED-1 blockiert 044b-Start:** b-cads IFC-Import **wirft [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)**
  (Ganzdatei-Ablehnung), wenn *einer* Wand die 'Axis'-Polyline **oder** Spatial-Containment fehlt → Fixture je Wand
  **kuratieren** (ggf. minimale konforme IFC erzeugen); DXF **2D-`LINE`** (nicht 3D — b-cad ist 2D-only).
- **045 → done, gefaltet in [`slice-046a`](../done/slice-046a-export-provenance.md)** (statische PDF-`/Info`/PNG-`tEXt`
  gingen in die injizierbare Export-Herkunft auf — s. den 046a-Block oben).

**Nächste Aktion:** `slice-045` schließen (Closure/Commit) **oder** `slice-044b` (Fremd-Import-Golden) starten —
044b-[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) beim
Start. Reihenfolge = Projektinhaber-Wahl. — Alternativ ein anderer Strang aus dem freien Menü unten.

`slice-041a` **done** (2026-07-23): die **DRW-Canvas-Grundsatz-ADRs sind Accepted** — [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)
(driven Adapter serialisieren, der Kern liefert abgeleitete Geometrie als `DerivedGeometry`-Bündel; alle
`adapter→services_geo`-`.a-check.yml`-Kanten entfallen; `architecture.md` §2/§1 werden wahr) und
[ADR-0019](../../adr/0019-drw-2d-canvas.md) (2D-Canvas + 2D-Lese-Naht `PlanViewPort`, **auf
[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) neu geschnitten** →
**keine** `io→services_geo`-Kante), je unabhängiges Text-Review 0 HIGH.
[LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) interaktive Erzeugungs-AK (Lastenheft 0.1.15)
+ Spec §1/§6 + architecture §1.1 nachgezogen. **DRW-Fundament 032a→b→c komplett** (durabel + sichtbar).

**Nächste Schritte** (je eigenes [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
davor; Reihenfolge = **Projektinhaber-Wahl**, keine offene Pflicht mehr). Der **M5-Trigger**
([OBJ-004](../../../../spec/lastenheft.md#3-projektziele) »Erweiterung durch Plugins«) ist mit slice-026b
**geliefert**; die [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Refactor-
Familie (042a–d) + der **DRW-Interaktiv-Strang v1** (slice-043) sind **done** (s. o.). Verbleibender
welle-5-Inhalt (ohne Meilenstein-Bindung, frei wählbar):
- **DRW-Zeichen-Aids + weitere Canvas-Funktionen** — [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Re-Eval-Trigger:
  Fang/Raster/Winkel ([LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)..003, je
  AK-Schärfung [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) zuerst),
  interaktive Geschoss-/Ebenen-Auswahl, Bauteile zeichnen, Selektion/Picking/Bemaßung, DRW-`op`.
- **UI-Themes/Docking** ([LH-FA-UI-001](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui)..003-Teilumfang).
- **Mehrsprachigkeit** ([LH-QA-006](../../../../spec/lastenheft.md#lh-qa-006--mehrsprachigkeit)).
- **oder** welle-5-Closure + Meilenstein-**M5**-Buchung (Verifikation + Carveout-Audit + `done/welle-5-results.md`).

**Commits lokal — Push nur auf explizites Wort.**

**Vorgänger-Trigger (beide erfüllt):** welle-4-austausch done (2026-07-01,
[`../done/welle-4-results.md`](../done/welle-4-results.md)) + Plugin-API-/ABI-ADR accepted
([ADR-0017](../../adr/0017-plugin-api-abi.md), 2026-07-02 — unabhängiges Text-Review
1 HIGH/4 MED/2 LOW/3 INFO + Projektinhaber-Durchsicht 2 LOW/2 INFO, alle eingearbeitet).

**Welle-Ziel:** b-cad wird **erweiterbar** ([OBJ-004](../../../../spec/lastenheft.md#3-projektziele), Meilenstein M5):
ein **Plugin-System** (`PLG`, [LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg)..004) hinter dem
**Plugin-Host als Driving Adapter** ([ADR-0017](../../adr/0017-plugin-api-abi.md): `dlopen` +
versionierter `extern "C"`-Handshake fail-closed + C++-Port-Facade, Plugins sehen nur
model + Driving-Ports, Sandbox = Port-Vermittlung + Fehler-Barriere, arch-check-**Regel P**).
Dazu die aus welle-3 zurückgestellten **2D-Zeichen-Werkzeuge `DRW`**
([LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)..007), **UI-Themes/Docking**
([LH-FA-UI-001](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui)..005-Teilumfang) und **Mehrsprachigkeit**
([LH-QA-006](../../../../spec/lastenheft.md#lh-qa-006--mehrsprachigkeit)). **M5-bindend ist allein der PLG-Strang**
([OBJ-004](../../../../spec/lastenheft.md#3-projektziele) »Erweiterung durch Plugins«); DRW/UI/Mehrsprachigkeit sind
Wellen-Inhalt ohne Meilenstein-Bindung — bei Umfangs-Druck entscheidet der Projektinhaber
über Nachschnitt (Modul-5-Sizing), nicht der Kalender.

**Closure-Trigger** (deliverable-granular; konkrete Slices emergieren mit
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review):
- ✓ **Plugin-API-/ABI-ADR** ([ADR-0017](../../adr/0017-plugin-api-abi.md)) accepted (zwei
  unabhängige Review-Runden, keine offenen HIGH/MED) — der Wellen-Trigger.
- **PLG-Schärfung + Impl** ([ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflichten):
  [LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg)..004-AK (lösungsfrei,
  [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei); Sandbox-AK auf beobachtbares
  Fehlverhalten wohlgeformter Plugins bezogen) + Spec-§4/§5/§6-Nachzug; Plugin-Host +
  Plugin-API + Beispiel-/Test-Plugin (`plugins/`-Baum) + AK-Tests (werfendes Plugin,
  ABI-Mismatch, Load→Edit→Unload mit realer `.so`) + arch-check-**Regel P**; benannte
  Impl-Entscheidungen mit Beleg (Symbol-Naht, Gate-Scope `plugins/`, Unload-Strategie)
  → [OBJ-004](../../../../spec/lastenheft.md#3-projektziele) erfüllt = **M5-Trigger**.
- **DRW-Strang:** 2D-Zeichen-Werkzeuge ([LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)..007,
  aus welle-3 zurückgestellt) — Scope-Schnitt je [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start);
  je gelieferter Familie Outline → AK.
- **UI-Strang:** dunkles/helles Theme + Docking-Teilumfang
  ([LH-FA-UI-001](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui)..003).
- **Mehrsprachigkeit:** [LH-QA-006](../../../../spec/lastenheft.md#lh-qa-006--mehrsprachigkeit)
  (Deutsch/Englisch, UI-Strings vollständig aus Ressourcen).
- Unabhängige Welle-Verifikation + Carveout-Audit + `done/welle-5-results.md`;
  [OBJ-004](../../../../spec/lastenheft.md#3-projektziele) erfüllt → **Meilenstein M5**.

**Fortschritt (eingefrorener Snapshot 2026-07-05 — der AKTUELLE Stand steht oben in §Aktuelle Welle;
seither geliefert: DRW-032b/c, die d-check-Gate-Slices 033–038, slice-041a, die
[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Familie 042a–d und der
2D-Canvas slice-043 — Details in `done/` + `CHANGELOG.md`):**
- ✓ **slice-032a — DRW-Strang eröffnet, Fundament AK-geschärft** (2026-07-05;
  [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) Accepted, Text-Review 0 HIGH): 2D-Zeichen-Daten
  (Hilfslinien + Ebenen) als **Kern-Werttypen** auf `Building`, neuer **`EditDrawingPort`** (Driving),
  Layer-Sichtbarkeit = **Export-Filter**, Beobachtbarkeit über Persistenz + 2D-Export (**kein Canvas**
  v1, deferiert). [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)/006 Outline → AK
  (Lastenheft 0.1.14) + Spec §1/§2.2/§4/§6-Mapping + Subset-Grenze um Hilfslinien;
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  **0 HIGH** (MED-1 Beobachtbarkeit tragfähig ohne Canvas). Folge: **slice-032b** (Impl: Kern + Port +
  Service + `guide_lines`-Persistenz) → **slice-032c** (DXF/PDF/PNG-Export-Sichtbarkeit).
- ✓ **slice-026b — Plugin-System lauffähig** (2026-07-03; Host/API/Beispiel-Plugin/Regel P):
  Plugin-Host als Driving Adapter (`src/adapters/plugin/`, dlfcn-Monopol) mit
  fail-closed-Handshake, 7-Stufen-Lifecycle und Fehler-Barriere
  ([`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
  `plugin_rejected`/`plugin_error`); Plugin-API `src/plugin_api/` (Port-Subset v1 =
  `EditStructurePort`+`EvaluatePort`, invalidierbarer Kontext); `plugins/`-Baum
  (Beispiel + 4 Fixtures, MODULE ohne Kern-Linkage); **Symbol-Naht = `ENABLE_EXPORTS`**
  (Beleg: Exception aus realer `.so` im Host gefangen); `--plugin`-CLI;
  **arch-check-Regel P** + lint-Scope `plugins/`. 8 AK-Tests mit realer `.so`;
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  0 HIGH + unabhängiges Code-Review **0 HIGH** (3 MED/2 LOW vor Closure behoben);
  gates grün (228/228, 90,3 %). **Beide [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflichten
  erfüllt — der [OBJ-004](../../../../spec/lastenheft.md#3-projektziele)/M5-Pfad ist frei**
  (M5-Buchung = Projektinhaber-Entscheidung bei der Welle-Closure; benannte Lücke:
  GUI-Plugin-Verwaltung → UI-Strang).
- ✓ **slice-026a — PLG-AK-Schärfung + Spec-Mapping** (2026-07-03, reine Doku/Entscheidung):
  [LH-FA-PLG-001](../../../../spec/lastenheft.md#lh-fa-plg-001)..004 von Outline auf AK
  (Lastenheft **0.1.13**, lösungsfrei/benutzer-beobachtbar, per-ID-Inline-Anker; Sandbox-AK
  auf **wohlgeformtes** Fehlverhalten bezogen + Ehrlichkeits-Klausel mit beiden Grenzfällen
  Absturz→Crash-Recovery/[LH-QA-005](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery)
  **oder** Silent-Corruption ohne Schutz) + spez. §1
  [`LH-FA-PLG-001.a`](../../../../spec/lastenheft.md#lh-fa-plg-001)-Sammelblock
  (Host/Handshake exakt-fail-closed/Lifecycle/Port-Vermittlung pull-only/Threading/
  Fehler-Barriere), §4 [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
  = **ein Code, zwei Log-Events** (`plugin_rejected`/`plugin_error`), §5 Span
  `bcad.plugin.lifecycle`, §6 Plugin-API-Vertragszeile; `.d-check.yml`-ids-Familie um PLG
  (Verschärfung). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  **0 HIGH** + unabhängige read-only-Diff-Durchsicht **0 HIGH/MED/LOW**; `make gates` grün.
  Erste [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflicht erfüllt →
  **slice-026b (PLG-Impl) startbar** (eigenes
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) davor).
- ✓ **[ADR-0017](../../adr/0017-plugin-api-abi.md) „Plugin-API-/ABI-Vertrag und Sandbox-Modell" accepted** —
  `dlopen`/`dlsym`/`dlclose` (glibc, **keine neue Dependency**, kein `QPluginLoader` — Regel E)
  + versionierter `extern "C"`-Handshake fail-closed + C++-Port-Facade **in-process** unter
  gepinnter Toolchain; Plugins sehen nur model + Driving-Ports (kein Beobachter-Zugang v1);
  Sandbox = Port-Vermittlung + Fehler-Barriere mit ehrlich benannten Grenzen (kein
  Speicherschutz, Silent-Corruption-Pfad, Threading-Vertrag); Symbol-Naht = benannte
  Impl-Entscheidung (statisches Kern-Dazulinken verboten). Unabhängiges Text-Review
  (**1 HIGH** — nicht existierender Undo-Stack als Ist behauptet, behoben — + 4 MED + 2 LOW
  + 3 INFO) + Projektinhaber-Durchsicht (2 LOW + 2 INFO), alle eingearbeitet; Folgepflichten
  im [ADR-Index](../../adr/README.md). **Welle-Trigger erfüllt, Welle gestartet.**