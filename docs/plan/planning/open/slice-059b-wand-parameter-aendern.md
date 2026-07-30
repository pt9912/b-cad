---
id: slice-059b
titel: Wand parametrisch ändern — Eigenschaften-Bereich, Klemmung, Ablehnung; der Abschluss-Trigger ([LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003)
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 059b: Wand parametrisch ändern

**Status:** open — **Detail-Schnitt vollzogen** (2026-07-29), entstanden aus der Teilung von
slice-059 im ersten
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
(§11). Die Auswahl-Hälfte ist [`slice-059a`](slice-059a-wand-auswaehlen.md).

**Welle:** welle-6-interaktiv-planen — **hier hängt der Abschluss-Trigger.** Mit diesem Slice ist er
erfüllt: „eine Wand ist im 2D-Canvas zeichenbar **und parametrisch änderbar**, ohne Kommandozeile."
**Danach ist die Welle zu schließen, nicht weiterzufüllen.**

**Setzt voraus:** [`slice-059a`](slice-059a-wand-auswaehlen.md) (Auswahl + Melde-Naht — ohne sie hat
der Eigenschaften-Bereich kein Subjekt) und [`slice-057`](../done/slice-057-lese-naht-bauteil-identitaet.md)
(die schmale Parameter-Abfrage `wallParams`).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-29.

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md), **Entscheidungen 4, 5, 13, 14, 15**. Der Kern
klemmt Stärke und Höhe seit welle-1; über die Oberfläche ist diese Parametrik **nicht erreichbar**.

## 1. Ziel

Ein nicht-modaler **Eigenschaften-Bereich** zeigt **Stärke** und **Höhe** der ausgewählten Wand und
nimmt neue Werte an: der **übernommene** Wert ist ablesbar, eine **Klemmung** wird dem Benutzer
**mit dem tatsächlich übernommenen Wert genannt**, eine **Ablehnung** lässt das Modell unverändert
und ist sichtbar, die **3D-Darstellung folgt ohne Zutun**, und **kein Wurf verlässt den
Ereignis-Pfad**.

## 2. Die drei Entwurfs-Fragen dieses Slice

### 2.1 Das Eingabe-Feld darf NICHT selbst klemmen — sonst sind zwei Akzeptanzkriterien unerreichbar

[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) verlangt
interaktiv **beides**: „Klemmung sichtbar" (Eingabe außerhalb des Bereichs ⇒ geklemmt **und der
tatsächlich übernommene Wert wird dem Nutzer genannt**) und „Ablehnung sichtbar" (ungültige Eingabe
⇒ Modell unverändert + Hinweis). Daraus folgt die Wahl des Widgets — zwingend:

| Variante | Konsequenz |
|---|---|
| Zahlen-Drehfeld mit Modell-Bereich 50–1000 | **Beide Negativ-AK werden unerreichbar.** Die Ziffernfolge „49" ist zwar eintippbar (`validate` ⇒ *Intermediate*), aber der **Abschluss** klemmt sie weg (`interpretText()` ⇒ 50) — der **Kern** sieht die 49 nie, klemmt also nie, und der Hinweis erscheint nie. „inf" wäre bereits als Eingabe **Invalid**. Das ist genau das Dritte, das [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E8 verbietet: „die AK-Zeile stehen lassen und nichts dazu bauen" |
| Zahlen-Drehfeld mit weiterem Bereich | erreichbar, aber es gäbe **zwei** Klemm-Autoritäten; welche gegriffen hat, wäre nicht ablesbar |
| **Textfeld + Übernahme** (gewählt) | der **Kern** ist die **einzige** Klemm-Autorität; beide Negativ-AK sind über die Oberfläche erreichbar |

**Übernommen wird bei Abschluss der Eingabe, nicht je Tastendruck.** Ein je Tastendruck übernehmender
Pfad mutierte beim Tippen von „240" zuerst auf 2 (⇒ geklemmt auf 50), dann 24 (⇒ 50), dann 240 — drei
Modell-Mutationen, drei Geometrie-Neubauten, drei Raum-Neuerkennungen und **zwei falsche
Klemm-Hinweise** für **eine** Eingabe. [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E5 sagt „eine
Klemmung beim Tippen darf den **Zeichen**fluss nicht unterbrechen" — das ist erfüllt, weil der
Hinweis **nicht-modal** ist, nicht weil jeder Tastendruck wirkt.

**Der Preis der Entscheidung, benannt:** das Anzeige-Surrogat wird eine **Zeichenkette**. Die
Vergleichsregel der Orakel lautet deshalb: **Text zurück-parsen und den Zahlwert vergleichen** — die
Anzeige-**Form** („50" vs. „50,0" vs. „50 mm") ist **keine** Zusage dieses Slice.

**Nach der Übernahme zeigt das Feld den ÜBERNOMMENEN Wert**, nicht die Eingabe. Zwei Ebenen, die man
nicht verwechseln darf: **abnahmebindend** ist der Lastenheft-Konjunkt „der tatsächlich übernommene
Wert wird dem Nutzer **genannt**" (Hinweis-Text); dass zusätzlich **das Feld** ihn zeigt, steht in
[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E5 und ist eine **Bauvorschrift**. Beides wird
umgesetzt, beides bekommt eine Zeile — aber nur eines ist Abnahme.

### 2.2 Die Senke nimmt den TEXT entgegen — sonst hat die halbe Ablehnungs-AK keinen Sensor

Der Kern lehnt **nicht-endliche** Werte ab (`Rejected`, Modell unverändert). Eine **nicht-numerische**
Eingabe erreicht ihn dagegen **nie** — gemessen: `toDouble("abc")` liefert `ok == false`, während
`toDouble("inf")` `ok == true` liefert und `inf` durchreicht. Die Umwandlung Text → Zahl **ist damit
eine Ausgangs-Entscheidung**, und die Frage ist nur, **wo** sie fällt:

| Ort der Umwandlung | Konsequenz |
|---|---|
| Composition-Root | **orakel-los** — `src/main.cpp` ist in kein Testbinary gelinkt (in [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) §12 ausdrücklich festgestellt) |
| Fenster | eine **zweite Ausgangs-Autorität** neben der Senke — genau die Doppel-Autorität, die §2.1 fürs Klemmen zu Recht ablehnt |
| **Senke** (gewählt) | **eine** Ausgangs-Autorität, und die nicht-numerische Hälfte ist dort prüfbar |

**Entschieden: die Parameter-Senke nimmt den Text entgegen** und liefert **einen** Ausgang für alle
Fälle — nicht-numerisch, nicht-endlich (`Rejected`), geklemmt (`Clamped` **mit** übernommenem Wert),
angenommen (`Accepted`), Wurf (unbekannte Id). **Das ist der Unterschied zur Schwester-Senke aus
[`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md)**, die ein `Point2D`-Paar nimmt: dort gab
es keine Textform, hier ist sie der Anfang des Ausgangs.

### 2.3 Wo die „sofort"-Zusage beobachtbar ist — und mit welcher Größe

[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) sagt „Geometrie
und 3D-Körper aktualisieren sich **sofort**". Auf der **2D-Fläche hat das kein Korrelat**: der Canvas
zeichnet Wand-**Achsen**, und eine Stärken-Änderung bewegt keine Achse; auch die Bounding-Box bleibt
gleich, also die Abbildung. **Eine Tinten-Sonde am Canvas wäre dafür nicht diskriminierend** — sie
rührte sich nicht, egal ob die Änderung ankam.

**Beobachtbar ist die Zusage an drei Stellen** ([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E4):
am **übernommenen Wert** im Bereich, am **3D-Körper** und im **Export**.

**Und der 3D-Beleg braucht die richtige Größe.** Der Viewer-Surrogat hält seine Netze in einer Abbildung
je `WallId`; eine Parameter-Änderung **ersetzt** ein Netz, sie fügt keines hinzu — die **Anzahl**
(`wallMeshes().size()`) bleibt **konstant**. Genau diese Anzahl ist aber das Orakel des Vorbilds aus
[`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) §4-10. **Wer es kopiert, schreibt eine
Zeile, die in beiden Armen grün ist.** Bewegt wird `effectiveUpdates()` (bzw. der **Inhalt** des
Netzes der geänderten Wand) — das ist die zu beobachtende Größe, und sie steht in der Zeile.

> **Dieselbe Klasse zum fünften Mal in diesem Strang:** ein Instrument, das anderswo getragen hat,
> trägt hier nicht. Erst der Pull-Zähler, dann die Tinten-Sonde am Fenster, dann die Tinten-Sonde für
> die Hervorhebung ([`slice-059a`](slice-059a-wand-auswaehlen.md) §2.3) — jetzt der Netz-Zähler.
> **Die Frage ist nie „welches Instrument habe ich?", sondern „welche Größe ändert sich beim
> Fehler?"**

## 3. Bewusst NICHT Teil

- **Die Auswahl selbst** — das ist [`slice-059a`](slice-059a-wand-auswaehlen.md).
- **Wandtyp und Material.** E13: [LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen)
  ist reine Outline ohne AK; beides interaktiv zu bedienen hieße, **fremde Anforderungen in einem
  UI-Strang auf AK-Niveau zu schärfen**.
- **Wand verschieben/teilen** ([LH-FA-WAL-004](../../../../spec/lastenheft.md#lh-fa-wal-004)/005,
  Outline), **Entfernen und Rückgängigmachen** (E12, benannte Grenze im Lastenheft).
- **Andocken/Anordnen des Bereichs.** E4: ein **fester** Bereich, keine Docking-Verwaltung
  ([LH-FA-UI-001](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) bleibt Outline).
- **Eine Anzeige-Form-Zusage** (Nachkommastellen, Einheit) — §2.1, benannte Grenze.
- **Jede Änderung an Kern, Persistenz, Export, Schema.** Die Mutatoren und die Lese-Naht
  **existieren**.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Der Bereich zeigt die Parameter der gewählten Wand** — die Werte kommen über die **schmale Abfrage** (slice-057) | `MainWindow`-Surrogat: Feld-Text **zurück-geparst** == `wallParams` (§2.1) | Anzeige-Aufruf entfernt ⇒ rot. **Kein Sensor für die Herkunft:** eine Implementierung, die die Werte aus dem `Building` zöge, lieferte **dieselben** Inhalte, und `make a-check` sieht es nicht (`ui_view → model` ist erlaubt) — die Naht-Treue ist eine **Bauvorschrift** (E15), kein Beleg |
| 2 | **Keine Auswahl ⇒ nichts zu ändern** — der Bereich sagt „keine Auswahl" und trägt **keine** Werte einer zuvor gewählten Wand; **auch nach** Öffnen/Anlegen | `MainWindow`-Surrogat, im Anschluss an eine gefallene Auswahl | Leeren entfernt ⇒ die alten Werte stehen weiter da ⇒ rot. **Abnahmebindend** ([LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) Boundary (Auswahl)) |
| 3 | **Stärke ändern wirkt** — das Modell trägt den übernommenen Wert, und das Feld zeigt ihn | Fenster (Eingabe) → Senke → Dienst | Übernahme-Aufruf entfernt ⇒ rot |
| 4 | **Die 3D-Sicht folgt** ([LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)) — **von der Bedienung ausgelöst**, nicht am Dienst | Fenster → Senke → Dienst → `ViewerScene` am **selben** Dienst. **Beobachtete Größe: `effectiveUpdates()`** (bzw. der Netz-**Inhalt** der Wand) — **nicht** die Netz-**Anzahl**: sie bleibt bei einer Parameter-Änderung **konstant** (§2.3) | Meldekette unterbrochen ⇒ rot. Mit der Anzahl als Größe wäre die Zeile in **beiden** Armen grün |
| 5 | **Klemmung ist sichtbar und BENANNT** — Eingabe 49 ⇒ Modell trägt **50**, der Hinweis **nennt 50** | Senken-Test (Ausgang + übernommener Wert) **und** `MainWindow`-Surrogat (Hinweis-Text) | Hinweis ohne Wert ⇒ rot. **Abnahmebindend** ist die **Nennung** (§2.1). **Vorbedingung: das Eingabe-Widget klemmt NICHT selbst** — sonst ist die Zeile **unerreichbar** statt rot |
| 6 | **Nach der Klemmung zeigt das FELD den übernommenen Wert**, nicht die Eingabe | `MainWindow`-Surrogat (Feld zurück-geparst) | Rückweg entfernt ⇒ das Feld zeigt 49 ⇒ rot. **Bauvorschrift** aus [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E5 — eigene Zeile, weil sie einen eigenen Weg hat (R2) |
| 7 | **Ablehnung, nicht-endlich** — `inf`/`nan` erreichen den Kern und werden **abgelehnt**: Modell unverändert, Hinweis | Senken-Test | Ablehnungs-Weg entfernt ⇒ rot |
| 8 | **Ablehnung, nicht-numerisch** — „abc"/leer erreichen den Kern **nie** und werden **in der Senke** abgelehnt: kein Port-Aufruf, Modell unverändert, Hinweis | **Senken-Test** (dort fällt die Entscheidung, §2.2) | Text-Prüfung entfernt ⇒ `0` wird übernommen und auf 50 geklemmt ⇒ rot (**stille Falsch-Übernahme** statt Ablehnung) |
| 9 | **Höhe gilt gleichlautend** ([LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren)) — Happy, Klemmung (499 ⇒ 500), Ablehnung; **eigene** Zeile mit **eigenem** Senken-Test | Senken-Test **und** `MainWindow`-Surrogat (wie §4-3, 5, 7) | Höhen-Weg auf den Stärke-Mutator verdrahtet ⇒ rot (die Verwechslung wäre sonst still, weil beide `ParamResult` liefern) |
| 10 | **Kein Wurf verlässt den Ereignis-Pfad** — bei **veralteter** Wand-Id gibt es einen **Hinweis**, keine Ausnahme (`setWallThickness` wirft bei unbekannter Id) | **`ui/command/`-Parameter-Senke** (dort liegt die Barriere und dort liegt der Test) | `try`/`catch` entfernt ⇒ rot |
| 11 | **Der Zeichen- und Auswahl-Pfad bleibt unverändert** | Bestands-Orakel (inkl. [`slice-059a`](slice-059a-wand-auswaehlen.md)) | (Regressions-Netz) |
| 12 | **Die geänderte Stärke überlebt Speichern/Laden und Export** | **Bestands-Netz** (`make io-smoke`, Persistenz-Runden-Orakel): eine über den Bereich geänderte Wand ist dieselbe wie eine über den Dienst geänderte, und §4-3 belegt, dass die Bedienung den Dienst erreicht | (Netz — **kein** eigener Sensor, benannte Entscheidung) |

**Zehn Zeilen tragen einen eigenen Sensor** (1–10), zwei sind **Netz** (11, 12).

**Was ausdrücklich KEIN Orakel bekommt** (§2.3): die 2D-Sichtbarkeit einer Stärken-Änderung — der
Canvas zeichnet Achsen, eine Stärken-Änderung bewegt keine. Und **keine Zusage** ist die Anzeige-Form
der Zahlen (§2.1).

## 5. Definition of Done

- [ ] **`src/adapters/ui/command/`-Parameter-Lese-Quelle** (neu, Bauform `plan_view_plan_source.h`):
      kapselt `PlanViewPort::wallParams`. **Kein** neuer Port — die Naht existiert seit slice-057.
- [ ] **`src/adapters/ui/command/`-Parameter-Senke** (neu): nimmt den **Text** (§2.2),
      ruft `setWallThickness`/`setWallHeight`, **fängt** die Würfe, meldet **einen** Ausgang **mit
      übernommenem Wert**. Orakel §4-5, 7, 8, 9, 10.
- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: **nicht-modaler Eigenschaften-Bereich** — zwei
      Felder, Übernahme bei Abschluss der Eingabe, „keine Auswahl"-Zustand, Rückweg für den
      übernommenen Wert. Orakel §4-1, 2, 6.
- [ ] **`src/main.cpp`**: Verdrahtung — Auswahl-Meldung (aus
      [`slice-059a`](slice-059a-wand-auswaehlen.md)) → Lese-Quelle → Anzeige; Feld → Senke → Hinweis
      **und** Anzeige-Nachzug. **Die Texte bleiben hier.**
- [ ] **Tests**: neuer Test der Parameter-Senke (§4-5, 7, 8, 9, 10) · `test_main_window.cpp`
      (§4-1, 2, 6) · **ein neuer Test, der Fenster, Senke und Viewer-Surrogat an denselben Dienst
      hängt** (§4-3, 4, **und die Fenster-Hälfte von §4-9**) — eigene Datei, damit die Kette einen
      Ort hat.
- [ ] **Orakel §4-1 bis §4-10 je mit roter Gegenprobe** im Closure-Text, **einzeln** gemessen; §4-11
      und §4-12 als **Netz** benannt. **Zwei Zeilen tragen eine ausgeschriebene Vorbedingung**
      (§4-5: das Feld klemmt nicht selbst; §4-4: die beobachtete Größe ist nicht die Anzahl).
- [ ] **`make a-check` grün** — **mit** ausgeschriebener Aussage, ob eine neue Kante entstanden ist.
      **Kein** Kern-/Persistenz-/Export-/Schema-Diff, am `git diff --stat` belegt.
- [ ] **Benutzerhandbuch** — **gesucht, nicht aufgezählt**: §1 („Stärke/Höhe nachträglich nicht
      änderbar" stammt aus slice-058 und ist zu **ersetzen**), §2.2/§2.3 (Eigenschaften-Bereich), die
      4.1-Tabelle, ein 4.2-Unterabschnitt, die FAQ. Plus Version + Änderungshistorie.
- [ ] **[ADR-Index](../../adr/README.md)**: die Folgepflichtzeilen „Auswahl-/Änderungs-Slice" **und**
      „[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
      vor der Welle-Closure" auf **erfüllt**.
- [ ] **Lastenheft/Spezifikation: am Artefakt prüfen, ob etwas fehlt** — [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md)
      hat geliefert. **Nicht pauschal verneinen:** im 057-Review war genau diese Pauschal-Verneinung
      falsch.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**;
      **`make acc-002-beleg` grün**. **Das erzeugte Bild wird NICHT committet** (Abnahme-Runde von
      [`slice-012`](../done-archive/slice-012-eckenschluss-wal006-teil.md); Lehre slice-058).
- [ ] **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
      des Bauteil-Strangs** — **vor** der Welle-Closure, unabhängiger Reviewer:
      **Eckenschluss** ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)),
      **Nachbar-Rebuild** und **Raum-Neuerkennung** im interaktiven Pfad. **HIGHs blockieren die
      Closure.** Es hängt hier, weil mit diesem Slice der letzte interaktive Mutator entsteht.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/command/`-Parameter-Lese-Quelle `.{h}` | neu | Kapselung von `wallParams` |
| `src/adapters/ui/command/`-Parameter-Senke `.{h}` | neu | Text-Eingang, Barriere, übernommener Wert |
| `src/adapters/ui/view/main_window.{h,cpp}` | ändern | Eigenschaften-Bereich + Rückweg |
| `src/main.cpp` | ändern | Verdrahtung + Hinweis-Texte |
| `tests/adapters/`-Test der Parameter-Senke `.{cpp}` | neu | §4-5, 7, 8, 9, 10 |
| `tests/adapters/`-Test der Kette Fenster→Senke→Dienst→Viewer `.{cpp}` | neu | §4-3, 4, 9 (Fenster-Hälfte) |
| `tests/adapters/test_main_window.cpp` | ändern | §4-1, 2, 6 |
| `tests/CMakeLists.txt` | ändern | **zwei** neue Testdateien |
| `docs/user/benutzerhandbuch.md` | ändern | **gesucht** + Version |
| `docs/plan/adr/README.md` | ändern | **zwei** Folgepflichtzeilen |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Reports | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) vor der Closure |

**Nicht berührt** (`spec/**` unter Vorbehalt der DoD-Prüfzeile): `src/hexagon/**`,
`src/adapters/io/**`, `src/adapters/persistence/**`, `spec/**`, `data-model.yaml`/`schema.sql`,
`docs/plan/adr/0021-wand-im-2d-canvas.md`.

## 7. Risiken

- **R1 — die Klemm-Autorität** (§2.1). Ein selbst klemmendes Eingabe-Feld macht zwei abnahmebindende
  AK **unerreichbar** statt rot. Vor dem Bau von §4-5 ist am Artefakt zu prüfen, dass eine Eingabe
  von 49 den **Kern** erreicht.
- **R2 — der Rückweg ist die neue Naht.** Alle bisherigen Fenster-Nähte **lösen aus**
  (`Action`, `CloseGuard`, `ToolActions`); dies ist die erste, die einen Wert **annimmt und
  zurückbekommt**. „Das Feld zeigt den übernommenen Wert" verlangt, dass das Fenster **nach** der
  Übernahme neu gesetzt wird — deshalb hat §4-6 eine **eigene** Zeile und nicht nur einen Konjunkt.
- **R3 — `wallParams` ist total, der Mutator wirft.** Dieselbe Wand-Id liefert lesend `nullopt` und
  schreibend eine Ausnahme. Wer den Lese-Weg als Vorbild für den Schreib-Weg nimmt, baut die fehlende
  Barriere ein — **dieselbe Falle wie in slice-058**, eine Ebene weiter.
- **R4 — das Instrument des Vorgängers ist blind** (§2.3): die Netz-**Anzahl** bewegt sich bei einer
  Parameter-Änderung nicht. Wer §4-10 aus slice-058 kopiert, schreibt eine beidseitig grüne Zeile.
- **R5 — zwei Mutatoren, ein Muster.** Stärke und Höhe liefern **beide** `ParamResult`; eine
  Verdrahtungs-Verwechslung wäre still. Deshalb prüft §4-9 die Höhe **einzeln** und nicht „analog".
- **R6 — das Handbuch wird an den Stellen unwahr, die slice-058 GERADE geschrieben hat.** Dort steht
  „Stärke/Höhe nachträglich nicht änderbar" als **benannte Grenze**. Zu **ersetzen**, nicht zu
  ergänzen.
- **R7 — die Welle schließt hier.** Nach der Closure dieses Slice ist der Trigger erfüllt; die Welle
  ist zu **schließen**, sobald das
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  durch ist — **nicht**, wenn die Arbeit ausgeht (Lehre der welle-5-Closure: 24 Tage Überlauf).

## 8. Trigger

- [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen, Folgepflicht
  „Auswahl-/Änderungs-Slice" (**zweite Hälfte**) **und**
  „[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  vor der Welle-Closure", samt den Zeilen im [ADR-Index](../../adr/README.md).

## 9. Closure-Trigger

- §4-1 bis §4-10 grün + **je einzeln** diskriminierend belegt; §4-11/§4-12 als Netz grün;
  `make gates`, `make io-smoke` und `make acc-002-beleg` grün; kein Kern-/Schema-/Export-Diff belegt;
  Handbuch und ADR-Index nachgezogen; Closure-Notiz.
- **Danach die Welle:** der Abschluss-Trigger ist erfüllt — liegt die
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Freigabe
  vor, ist **zu schließen** (M6 buchen).

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (Fenster + Kommando-Schicht)

- **Modus:** GF; **Dichte:** mittel-groß — eine Lese-Quelle, eine Senke mit Text-Eingang und
  Barriere, der **erste zustandstragende** Eingabe-Bereich des Fensters samt Rückweg, zwölf
  Orakel-Zeilen (zehn mit eigenem Sensor).
- **Risiko:** **hoch** — hier entsteht der einzige Weg, auf dem eine Benutzer-Eingabe das
  Gebäudemodell **verändert**, und drei der vier Ausgänge sind Fehl-Ausgänge.
- **Warum diese Hälfte den Wellen-Abschluss trägt:** der Trigger verlangt „parametrisch änderbar" —
  das ist genau dieser Slice.

## 11. Herkunft dieses Plans

Entstanden aus der **Teilung** von slice-059 im ersten
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
([`2026-07-29-slice-059-plan.md`](../../../reviews/2026-07-29-slice-059-plan.md), MEDIUM-6). Vier
Befunde des Laufs betreffen **diese** Hälfte und sind hier eingearbeitet:

| # | Behandlung |
|---|---|
| **MEDIUM-1** (die **nicht-numerische** Hälfte der Ablehnungs-AK hatte keinen Ort: gemessen erreicht `abc` den Kern **nie**, `inf` schon — im Composition-Root wäre die Entscheidung orakel-los, im Fenster entstünde eine zweite Ausgangs-Autorität) | **§2.2 entscheidet die Naht: die Senke nimmt den TEXT.** Eine Ausgangs-Autorität, und §4-8 hat einen Ort. Die Gegenprobe ist scharf: ohne Text-Prüfung wird `0` übernommen und auf 50 **geklemmt** — eine stille Falsch-Übernahme statt einer Ablehnung. |
| **MEDIUM-2** (das Viewer-Instrument aus slice-058 ist für eine Parameter-Änderung **strukturell blind**: die Netz-**Anzahl** bleibt konstant) | **§4-4 nennt die beobachtete Größe** (`effectiveUpdates()` bzw. Netz-Inhalt) — und §2.3 schreibt die Lehre aus, weil es die **fünfte** Wiederholung derselben Klasse in diesem Strang ist. |
| **MEDIUM-4** (§6 hatte **keinen Ort** für den Ketten-Test, und die CMake-Zeile schloss ihn aus) | **Eigene §6-Zeile + eigene Testdatei**; die CMake-Zeile nennt **zwei** neue Dateien. Dieselbe Bauart war in slice-058 §11a schon einmal Befund. |
| **LOW-1** (§2.3 schrieb einen **ADR**-Satz dem **Lastenheft** als „wörtlichen AK-Konjunkt" zu; „Eingabefeld" kommt in beiden Spec-Straten nicht vor) | **§2.1 trennt jetzt die Ebenen:** abnahmebindend ist „der übernommene Wert wird **genannt**", die Feld-Anzeige ist **Bauvorschrift** (E5). Beide bekommen eine eigene Orakel-Zeile (§4-5, §4-6). |
| **LOW-2** („49 ist nicht eintippbar" war ungenau — es ist eintippbar, der **Abschluss** klemmt) | Formulierung in §2.1 korrigiert; die **Folgerung** war richtig und trägt. |
| **LOW-3** (§4-8-alt: „aus der schmalen Abfrage, nicht aus dem Domänen-Objekt" hat **keinen** Sensor) | **In §4-1 benannt**: die Naht-Treue ist eine **Bauvorschrift**, kein Beleg — `make a-check` sieht sie nicht, weil `ui_view → model` erlaubt ist. |
| **LOW-4** (das Surrogat wird eine **Zeichenkette**, ihr Format war nirgends festgelegt) | **Vergleichsregel in §2.1**: Text zurück-parsen, **Zahlwert** vergleichen; die Anzeige-Form ist ausdrücklich **keine** Zusage. |
| **LOW-5 (a)** (§4-14-alt sagte „wie §4-10..12", war aber nur im Fenster-Test verortet) | §4-9 nennt **beide** Orte: eigener Senken-Test **und** Fenster-Hälfte im Ketten-Test. |
| **LOW-5 (c)** ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) fehlte im Front-Matter, obwohl die [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Zeile darauf zielt) | **Im Front-Matter ergänzt.** |

**Startbar:** **nein** — dieser Plan hat **noch kein eigenes**
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start).
Er entstand als Hälfte eines geprüften Plans, aber die Teilung selbst ist ungeprüft, und §2.2 (die
Text-Naht) ist eine **neue** Entscheidung, die kein Reviewer gesehen hat.

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
