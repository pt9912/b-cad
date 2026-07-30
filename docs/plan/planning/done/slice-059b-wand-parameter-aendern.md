---
id: slice-059b
titel: Wand parametrisch ändern — Eigenschaften-Bereich, Klemmung, Ablehnung; der Abschluss-Trigger ([LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003)
status: done
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung), [LH-FA-UI-001](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 059b: Wand parametrisch ändern

**Status:** done (2026-07-29). Entstanden aus der Teilung von slice-059 (§11) und
danach **eigenständig** geprüft: der eigene
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
meldet **0 HIGH** (§11a). Die Auswahl-Hälfte ist [`slice-059a`](../done/slice-059a-wand-auswaehlen.md).

**Welle:** welle-6-interaktiv-planen — **hier hängt der Abschluss-Trigger.** Mit diesem Slice ist er
erfüllt: „eine Wand ist im 2D-Canvas zeichenbar **und parametrisch änderbar**, ohne Kommandozeile."
**Danach ist die Welle zu schließen, nicht weiterzufüllen.**

**Setzt voraus:** [`slice-059a`](../done/slice-059a-wand-auswaehlen.md) (Auswahl + Melde-Naht — ohne sie hat
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
Vergleichsregel der Orakel lautet deshalb: **Text mit DERSELBEN Umwandlung zurück-lesen wie die
Senke** (§2.2) und den Zahlwert vergleichen. **Mit einer anderen** wäre ein Feld-Inhalt „50,0" im
Test grün und im Produkt eine **Ablehnung** — das Orakel prüfte dann eine andere Software als die
ausgelieferte. Die Anzeige-**Form** ist keine Gestaltungs-Zusage, aber sie ist **eingeschränkt**:
sie muss von derselben Umwandlung lesbar sein (§2.2, §4-13).

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

**Und die UMWANDLUNG ist damit selbst eine Bauvorschrift, nicht eine Implementierungs-Freiheit.** An
ihr hängen drei Zusagen, gemessen:

| Eingabe | `QString::toDouble` | `std::stod` | `std::from_chars` (volle Konsumption) |
|---|---|---|---|
| `abc` / leer | `ok = false`, 0 | **wirft** | Fehler |
| `50 mm` | `ok = false`, 0 | **50** (Rest ignoriert) | Fehler (Rest) |
| `50,0` | `ok = false`, 0 | **50** (Komma trennt) | Fehler (Rest) |
| `inf` / `nan` | `ok = true` | durchgereicht | durchgereicht |

**Entschieden: `std::string_view` + `std::from_chars` mit VOLLER Konsumption.** Drei Gründe: (1) die
Senke bleibt **Qt-frei** — heute inkludiert **kein** `ui/command/`-Objekt Qt, und ein `QString`-
Eingang wäre das erste (gate-frei, gemessen, aber eine Eigenschaft, die man nicht beiläufig ändert);
(2) `from_chars` **wirft nicht** und ist **locale-frei** — `std::stod` täte beides nicht; (3) die
nicht-endlichen Werte (`inf`/`nan`) laufen weiterhin **in den Kern** und werden **dort** abgelehnt,
die Zweiteilung der Ablehnungs-AK bleibt also erhalten.

**Schließbedingung, ohne die das Produkt einen Fehler hätte, den kein Orakel sieht:** die
**Anzeige-Form muss von derselben Umwandlung wieder gelesen werden**. §2.1 gab „50,0" und „50 mm"
frei — beide sind für jede der drei Umwandlungen **kein** vollständiger Zahlwert. Ein Benutzer, der
ein zurückgeschriebenes Feld nur mit Enter bestätigt, bekäme eine **Ablehnung für einen Wert, den er
nie geändert hat**. Die Anzeige-Form ist damit **eingeschränkt** (schlichte Dezimalzahl, kein
Tausender-Trenner, keine Einheit im Feld) und bekommt eine eigene Orakel-Zeile (§4-13).

### 2.3 Das Fenster bekommt WERTE, keine fertigen Texte — sonst hat der abnahmebindende Konjunkt keinen Sensor

[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) bindet ab: „der
**tatsächlich übernommene Wert** wird dem Nutzer **genannt**". Das ist eine Zusage über den **Wert**
im Text, nicht über Wortlaut.

**Der Weg des Vorgängers trägt sie nicht.** In [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md)
setzt der Composition-Root die Hinweis-Texte zusammen und reicht sie als fertige Zeichenkette ans
Fenster; ein Fenster-Orakel belegt dann nur, dass **ein gereichter Text** erscheint. Die
**Zusammensetzung** — die Stelle, an der der übernommene Wert in den Text kommt — liegt in
`src/main.cpp` und ist **per Konstruktion orakel-los**.

**Für slice-058 war das richtig:** dessen drei Hinweise sind **konstante** Sätze ohne Wert. **Hier
ist es falsch**, weil genau der Wert die Abnahme trägt.

**Entschieden: die Fenster-Naht nimmt `(Ausgang, übernommener Wert)`**, nicht `QString`. Das Fenster
setzt daraus den Hinweis **und** schreibt den Wert ins Feld zurück. Damit sind **beide** Zusagen am
Fenster-Surrogat messbar: dass der Hinweis den **Wert** trägt (§4-5) und dass das **Feld** ihn zeigt
(§4-6). Der **Wortlaut** bleibt Fenster-Sache und ist **keine** Zusage — geprüft wird, dass der Wert
darin vorkommt.

**Die Abweichung von der slice-053-Grenze („Meldungstexte bleiben im Composition-Root") ist benannt
und begründet:** dort ging es um **Dialog**-Texte; hier geht es um eine wert-tragende Statuszeile.
Der Unterschied ist nicht Geschmack, sondern Messbarkeit — ein Text ohne Wert darf im Root entstehen,
ein Text **mit** abnahmebindendem Wert nicht.

**Und der Rückweg ins Feld liegt aus demselben Grund im Fenster** (§4-6): läge er im Root, wäre die
Gegenprobe „Rückweg entfernt" im Test **nicht herstellbar**.

### 2.4 Wo die „sofort"-Zusage beobachtbar ist — und mit welcher Größe

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
> die Hervorhebung ([`slice-059a`](../done/slice-059a-wand-auswaehlen.md) §2.3) — jetzt der Netz-Zähler.
> **Die Frage ist nie „welches Instrument habe ich?", sondern „welche Größe ändert sich beim
> Fehler?"**

## 3. Bewusst NICHT Teil

- **Die Auswahl selbst** — das ist [`slice-059a`](../done/slice-059a-wand-auswaehlen.md).
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
| 3 | **Stärke ändern wirkt** — das Modell trägt den übernommenen Wert | Fenster (Eingabe) → Senke → Dienst | Übernahme-Aufruf entfernt ⇒ rot. **Der Feld-Konjunkt gehört NICHT hierher:** im Happy-Fall ist der übernommene Wert **identisch mit der Eingabe**, ein entfernter Rückweg ließe das Feld unverändert richtig aussehen. Diskriminierend wird er erst bei der Klemmung — dafür gibt es §4-6 |
| 4 | **Die 3D-Sicht folgt** ([LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)) — **von der Bedienung ausgelöst**, nicht am Dienst | Fenster → Senke → Dienst → `ViewerScene` am **selben** Dienst. **Beobachtete Größe: der Netz-INHALT der geänderten Wand** — **nicht** die Netz-**Anzahl** (bleibt konstant, §2.4) und **nicht** `effectiveUpdates()` allein: der Zähler bewegt sich gemessen **auch bei einer wirkungslosen Setzung** (derselbe Wert erneut ⇒ 2 → 3) und belegt damit „es kam etwas an", nicht „die Darstellung zeigt den neuen Stand" | Meldekette unterbrochen ⇒ rot. Mit der Anzahl als Größe wäre die Zeile in **beiden** Armen grün |
| 5 | **Klemmung ist sichtbar und BENANNT** — Eingabe 49 ⇒ Modell trägt **50**, und der **angezeigte Hinweis enthält den Wert 50** | **Zwei Orte, beide gebucht** (§2.3): Senken-Test (Ausgang **und** übernommener Wert) **und** `MainWindow`-Surrogat — dort wird die Naht mit `(Clamped, 50)` gerufen und der **erscheinende Text** auf den Wert geprüft | Wert nicht in den Text übernommen ⇒ rot. **Abnahmebindend** ist die **Nennung des Werts**, nicht der Wortlaut. **Vorbedingung: das Eingabe-Widget klemmt NICHT selbst** — sonst ist die Zeile **unerreichbar** statt rot |
| 6 | **Nach der Klemmung zeigt das FELD den übernommenen Wert**, nicht die Eingabe | `MainWindow`-Surrogat (Feld mit der **Senken-Umwandlung** zurück-gelesen, §2.1) | Rückweg entfernt ⇒ das Feld zeigt 49 ⇒ rot. **Bauvorschrift** aus [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E5. **Der Rückweg liegt im FENSTER** (§2.3) — läge er im Composition-Root, wäre diese Gegenprobe im Test nicht herstellbar |
| 7 | **Ablehnung, nicht-endlich** — `inf`/`nan` erreichen den Kern und werden **abgelehnt**: Modell unverändert, Hinweis | Senken-Test | Ablehnungs-Weg entfernt ⇒ rot |
| 8 | **Ablehnung, nicht-numerisch** — „abc", leer, **und die Rest-Fälle „50 mm"/„50,0"** erreichen den Kern **nie** und werden **in der Senke** abgelehnt: kein Port-Aufruf, Modell unverändert, Hinweis | **Senken-Test** (dort fällt die Entscheidung, §2.2) | Volle-Konsumption-Prüfung entfernt ⇒ „50 mm" wird als **50** übernommen ⇒ rot; Umwandlungs-Prüfung ganz entfernt ⇒ `0` wird übernommen und **geklemmt** (auf **50** bei der Stärke, auf **500** bei der Höhe — gemessen) ⇒ rot. **Beides ist eine stille Falsch-Übernahme statt einer Ablehnung** |
| 9 | **Höhe gilt gleichlautend** ([LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren)) — Happy, Klemmung (499 ⇒ 500), Ablehnung; **eigene** Zeile mit **eigenem** Senken-Test | Senken-Test **und** `MainWindow`-Surrogat (wie §4-3, 5, 7) | Höhen-Weg auf den Stärke-Mutator verdrahtet ⇒ rot (die Verwechslung wäre sonst still, weil beide `ParamResult` liefern) |
| 10 | **Kein Wurf verlässt den Ereignis-Pfad** — bei **veralteter** Wand-Id gibt es einen **Hinweis**, keine Ausnahme (`setWallThickness` wirft bei unbekannter Id) | **`ui/command/`-Parameter-Senke** (dort liegt die Barriere und dort liegt der Test) | `try`/`catch` entfernt ⇒ rot |
| 13 | **Die Anzeige-Form ist von der Senken-Umwandlung lesbar** — der Rundlauf schließt: was das Feld zeigt, nimmt die Senke **ohne Ablehnung** wieder an (§2.2-Schließbedingung) | `MainWindow`-Surrogat + Senken-Test: Feld-Inhalt nach einer Übernahme **unverändert** erneut abschicken | Anzeige mit Komma/Einheit/Tausender-Trenner ⇒ die unveränderte Eingabe wird **abgelehnt** ⇒ rot. **Ohne diese Zeile hätte das Produkt einen Fehler, den kein anderes Orakel sieht:** Enter auf einem nie geänderten Feld ergäbe eine Ablehnung |
| 11 | **Der Zeichen- und Auswahl-Pfad bleibt unverändert** | Bestands-Orakel (inkl. [`slice-059a`](../done/slice-059a-wand-auswaehlen.md)) | (Regressions-Netz) |
| 12 | **Die geänderte Stärke überlebt Speichern/Laden und Export** | **Bestands-Netz** (`make io-smoke`, Persistenz-Runden-Orakel): eine über den Bereich geänderte Wand ist dieselbe wie eine über den Dienst geänderte, und §4-3 belegt, dass die Bedienung den Dienst erreicht | (Netz — **kein** eigener Sensor, benannte Entscheidung) |

**Elf Zeilen tragen einen eigenen Sensor** (1–10 und 13), zwei sind **Netz** (11, 12).

**Was ausdrücklich KEIN Orakel bekommt** (§2.3): die 2D-Sichtbarkeit einer Stärken-Änderung — der
Canvas zeichnet Achsen, eine Stärken-Änderung bewegt keine. Und **keine Zusage** ist die Anzeige-Form
der Zahlen (§2.1).

## 5. Definition of Done

- [ ] **`src/adapters/ui/command/`-Parameter-Lese-Quelle** (neu, Bauform `plan_view_plan_source.h`):
      kapselt `PlanViewPort::wallParams`. **Kein** neuer Port — die Naht existiert seit slice-057.
- [ ] **`src/adapters/ui/command/`-Parameter-Senke** (neu): nimmt den **Text** als
      `std::string_view` und wandelt mit `std::from_chars` **bei voller Konsumption** um (§2.2 —
      **Qt-frei**, wurf-frei, locale-frei), ruft `setWallThickness`/`setWallHeight`, **fängt** die
      Würfe, meldet **einen** Ausgang **mit übernommenem Wert**. Orakel §4-5, 7, 8, 9, 10, 13.
- [ ] **`src/adapters/ui/view/main_window.{h,cpp}`**: **nicht-modaler Eigenschaften-Bereich** — zwei
      Felder, Übernahme bei Abschluss der Eingabe, „keine Auswahl"-Zustand. **Die Naht nimmt
      `(Ausgang, übernommener Wert)`, keinen fertigen Text** (§2.3): das Fenster setzt den Hinweis
      zusammen **und** schreibt den Wert ins Feld zurück. Orakel §4-1, 2, 5 (Fenster-Hälfte), 6, 13.
- [ ] **`src/main.cpp`**: Verdrahtung — Auswahl-Meldung (aus
      [`slice-059a`](../done/slice-059a-wand-auswaehlen.md)) → Lese-Quelle → Anzeige; Feld → Senke → **Ausgang
      als Wert** ans Fenster. **Der Root reicht Werte durch, er setzt hier keine Texte zusammen**
      (§2.3 — anders als in [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md), und aus
      einem benannten Grund).
- [ ] **Tests**: neuer Test der Parameter-Senke (§4-5, 7, 8, 9, 10) · `test_main_window.cpp`
      (§4-1, 2, 6) · **ein neuer Test, der Fenster, Senke und Viewer-Surrogat an denselben Dienst
      hängt** (§4-3, 4, **und die Fenster-Hälfte von §4-9**) — eigene Datei, damit die Kette einen
      Ort hat.
- [ ] **Orakel §4-1 bis §4-10 und §4-13 je mit roter Gegenprobe** im Closure-Text, **einzeln**
      gemessen — **elf Zeilen mit eigenem Sensor**; §4-11 und §4-12 als **Netz** benannt.
      **Vier Zeilen tragen ausgeschriebene Vorbedingungen bzw. Bauvorschriften:** §4-3 (der
      Feld-Konjunkt gehört nicht hierher), §4-4 (die Größe ist der Netz-**Inhalt**), §4-5 (das Feld
      klemmt nicht selbst) und §4-6 (der Rückweg liegt im Fenster).
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
      die **Geometrie-Korrektheit gegen die Spezifikation** im vollen Umfang der Konvention
      (mindestens Orientierung/Winding, Bündigkeit/Spaltfreiheit, Höhen-/Maß-Exaktheit, Totalität bei
      Degeneration) — **im interaktiven Pfad** und mit den drei von
      [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen ausdrücklich genannten
      Schwerpunkten: **Eckenschluss**
      ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)),
      **Nachbar-Rebuild**, **Raum-Neuerkennung**. **Die drei sind Schwerpunkte, nicht die Grenze** —
      die Konvention rangiert höher als ihre Aufzählung hier. **HIGHs blockieren die Closure.** Es
      hängt hier, weil mit diesem Slice der letzte interaktive Mutator entsteht.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/adapters/ui/command/`-Parameter-Lese-Quelle `.{h}` | neu | Kapselung von `wallParams` |
| `src/adapters/ui/command/`-Parameter-Senke `.{h}` | neu | Text-Eingang, Barriere, übernommener Wert |
| `src/adapters/ui/view/main_window.{h,cpp}` | ändern | Eigenschaften-Bereich + Rückweg |
| `src/main.cpp` | ändern | Verdrahtung + Hinweis-Texte |
| `tests/adapters/`-Test der Parameter-Senke `.{cpp}` | neu | §4-5, 7, 8, 9, 10 |
| `tests/adapters/`-Test der Kette Fenster→Senke→Dienst→Viewer `.{cpp}` | neu | §4-3, 4, 9 (Fenster-Hälfte) |
| `tests/adapters/test_main_window.cpp` | ändern | §4-1, 2, 5 (Fenster-Hälfte), 6, 13 |
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
- **Danach die Welle**, und zwar in **drei** Handgriffen (am Bestand geprüft, nicht „M6 buchen"):
  eine **Closure-Datei** `welle-6-results.md` neben den Vorgängern in
  [`done/`](../done/) (alle sechs bisherigen Wellen haben eine), der **Meilenstein-Status** M6 in der Roadmap-Tabelle (heute `offen`) und der
  **Welle-Block** samt Ruhe-Marker.
  **[MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)
  steht der Closure nicht im Weg** — im Folgepflicht-Block sind nur die **zwei**
  [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)-Zeilen `offen`, und **beide bucht dieser Slice**;
  die [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Zeile „Raster/Winkel" trägt eine **ausdrückliche
  Deferral-Entscheidung**. Das steht hier, damit die Closure nicht an einer vermeintlich offenen
  Zeile hängen bleibt.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (Fenster + Kommando-Schicht)

- **Modus:** GF; **Dichte:** mittel-groß — eine Lese-Quelle, eine Senke mit Text-Eingang und
  Barriere, der **erste zustandstragende** Eingabe-Bereich des Fensters samt Rückweg, dreizehn
  Orakel-Zeilen (elf mit eigenem Sensor).
- **Risiko:** **hoch** — hier entsteht der einzige Weg, auf dem eine Benutzer-Eingabe das
  Gebäudemodell **verändert**, und drei der vier Ausgänge sind Fehl-Ausgänge.
- **Warum diese Hälfte den Wellen-Abschluss trägt:** der Trigger verlangt „parametrisch änderbar" —
  das ist genau dieser Slice.

## 11. Herkunft dieses Plans

Entstanden aus der **Teilung** von slice-059 im ersten
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
([`2026-07-29-slice-059-plan.md`](../../../reviews/2026-07-29-slice-059-plan.md), MEDIUM-6). **Neun**
Befunde des Laufs betreffen **diese** Hälfte und sind hier eingearbeitet (drei MEDIUM, sechs
LOW-Positionen):

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

## 11a. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (eigener erster Lauf, 2026-07-29)

Report: [`2026-07-29-slice-059b-plan.md`](../../../reviews/2026-07-29-slice-059b-plan.md) —
**0 HIGH / 3 MEDIUM / 6 LOW / 4 INFO + 17 Negativbefunde, „STARTBAR"**. Unabhängiger Reviewer,
verschieden von Autor und den Reviewern des ungeteilten Plans und von
[`slice-059a`](../done/slice-059a-wand-auswaehlen.md).

**Die neue Entscheidung trägt** — beide Begründungszweige am Artefakt bestätigt (`src/main.cpp` ist
in keinem Testziel; das Fenster wäre eine zweite Ausgangs-Autorität), und die Gegenprobe von §4-8 ist
**exakt** wie behauptet (`setWallThickness(…, 0.0)` ⇒ `Clamped`, `applied = 50`; Höhe: 500).
**Die eigentliche Kehrseite lag woanders:**

| # | Behandlung |
|---|---|
| **MEDIUM-1** (§4-5 buchte **zwei** Orte, aber nur einer misst: die abnahmebindende **Nennung** des übernommenen Werts endet an der Senke — das Fenster-Orakel belegt nur, dass ein **gereichter** Text erscheint, und die **Zusammensetzung** liegt im orakel-losen Composition-Root) | **§2.3 ist neu und entscheidet die Naht: das Fenster bekommt `(Ausgang, Wert)`, keinen fertigen Text.** Damit ist die Nennung am Fenster-Surrogat messbar. **Die Abweichung von slice-058 ist benannt und begründet:** dessen Hinweise sind **konstante** Sätze ohne Wert, hier trägt der Wert die Abnahme — ein Text ohne Wert darf im Root entstehen, ein Text **mit** abnahmebindendem Wert nicht. |
| **MEDIUM-2** (§4-6 hat nur dann einen Sensor, wenn der **Rückweg im Fenster** liegt; zwei benachbarte DoD-Zeilen sagten Verschiedenes) | **Derselbe Schnitt** (§2.3): der Rückweg liegt im Fenster, sonst ist „Rückweg entfernt" im Test nicht herstellbar. Die zwei DoD-Zeilen sind vereinheitlicht. |
| **MEDIUM-3** (§2.2 entschied die **Naht**, aber nicht die **Umwandlung** — und an ihr hängen drei Zusagen: `std::stod("abc")` **wirft** statt still 0 zu liefern; `toDouble("50,0")`/`"50 mm"` ⇒ `ok = false`, `stod("50,0")` ⇒ 50) | **Die Umwandlung ist jetzt Bauvorschrift:** `std::string_view` + `std::from_chars` bei **voller Konsumption** — Qt-frei, wurf-frei, locale-frei, und `inf`/`nan` laufen weiterhin in den Kern. **Dazu die Schließbedingung samt eigener Orakel-Zeile (§4-13): die Anzeige-Form muss von derselben Umwandlung wieder lesbar sein.** Ohne sie hätte das Produkt einen Fehler, den **kein** Orakel sieht: Enter auf einem nie geänderten Feld ergäbe eine Ablehnung. |
| **LOW-1** (die Einarbeitungs-Zählung in §11 stimmte nicht) | **Neun** statt vier. |
| **LOW-2** (§4-3 trug einen Konjunkt, der im `Accepted`-Fall nicht diskriminiert — der übernommene Wert ist dort **identisch** mit der Eingabe) | Der Feld-Konjunkt ist aus §4-3 **entfernt** und dort ausdrücklich als **kein Beleg** benannt; er lebt in §4-6, wo er misst. |
| **LOW-3** (`effectiveUpdates()` bewegt sich gemessen **auch bei einer wirkungslosen** Setzung: 2 → 3) | **§4-4 verlangt jetzt den Netz-INHALT**, nicht den Zähler — der belegt „es kam etwas an", nicht „die Darstellung zeigt den neuen Stand". **Das ist die sechste Wiederholung der Instrument-Klasse in diesem Strang**, und diesmal war das ERSATZ-Instrument das ungenaue. |
| **LOW-4** (die [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-DoD-Zeile **verengte** die höherrangige Quelle auf drei Punkte) | Die Zeile nennt jetzt den **vollen** Konventions-Umfang und führt die drei ADR-Punkte als **Schwerpunkte, nicht als Grenze**. |
| **LOW-5** (der Wellen-Abschluss braucht mehr als „M6 buchen", und die [MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)-Lage gehört dazu) | §9 nennt die **drei** Handgriffe und stellt fest, dass der Folgepflicht-Block der Closure **nicht** im Weg steht. |
| **LOW-6 (b)** (Front-Matter-Asymmetrie: [LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen) gelistet, [LH-FA-UI-001](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) nicht, obwohl §3 beide in derselben Rolle zitiert) | **Ergänzt.** |
| **LOW-6 (a)** (fünf Rückverweise in [`slice-058`](../done/slice-058-wand-zeichnen-im-canvas.md) zeigen seit der Teilung auf die **falsche** Hälfte — darunter die **einzigen zwei Wegweiser** zur [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Pflicht) | **Alle fünf korrigiert.** **Neunte Zählung derselben Bauart** in diesem Strang — und die erste, die nicht durch Umformulieren, sondern durch **Teilen** entstanden ist. |

**Positiv bestätigt (17 Negativbefunde):** die **Teilung ist sauber** — keine Zusage heimatlos (17
Zeilen des ungeteilten Plans gegen 10 + 12 abgeglichen), und `slice-059b` ist technisch sogar **ohne**
`slice-059a` baubar; die Abhängigkeit ist **Produkt-Erreichbarkeit**, und der Plan sagt das richtig.
Gemessen bestätigt: Netz-**Anzahl** bei Parameter-Änderung konstant (1 → 1), `effectiveUpdates()` bei
**Ablehnung** unbewegt (1 → 1). Ein Qt-Include in `ui/command/` wäre **gate-frei** — der Plan
entscheidet sich trotzdem dagegen (§2.2).

**Startbar: JA** — 0 HIGH nach der vorab festgelegten Abbruchregel. Die drei MEDIUM und sechs LOW
sind eingearbeitet; sie betreffen **Nahtschnitt und Vorbedingungen**, nicht die Bauart.

## 12. Closure-Notiz

**Vollzogen 2026-07-29.** `make gates` grün, **414 Tests** (399 vor dem Slice, **+15** neu),
Zeilen-Coverage 92,0 %; `make io-smoke` grün, `make acc-002-beleg` grün (das Bild **nicht**
committet — Abnahme-Artefakt von
[`slice-012`](../done-archive/slice-012-eckenschluss-wal006-teil.md)). Kein
Kern-/Persistenz-/Export-/Schema-Diff, **`.a-check.yml` byte-unberührt** (`a-check` = 0 Befunde,
**keine neue Kante**: die Senke lebt in `ui/command/` an der vorhandenen
`ui_command → ports_driving`-Kante, und sie ist **Qt-frei** geblieben).

### Die Orakel-Zeilen, je mit EINZELN gemessener roter Gegenprobe

| Zeile | Sonde (Mutation) | gefallen |
|---|---|---|
| §4-1 | Anzeige-Aufruf entfernt | `EigenschaftenBereichZeigtDieParameter` + 2 |
| §4-2 | Leeren des Bereichs entfernt | `OhneAuswahlStehenKeineAltenWerteDa` |
| §4-3 | Übernahme-Aufruf entfernt | `UebernahmeImFensterErreichtModellUnd3D` |
| §4-4 | die 3D-Szene sieht `WallThicknessChanged` nicht | **eigene Sonde**: drei `ViewerSceneAk`-Zeilen — §4-3 und §4-4 teilen einen Test, aber **nicht** dieselbe Sonde |
| §4-5 | Klemm-Hinweis **ohne** den übernommenen Wert | `KlemmungWirdMitDemWertGenannt` |
| §4-6 | Rückweg ins Feld entfernt | `NachDerKlemmungZeigtDasFeldDenUebernommenenWert` |
| §4-7 | Ablehnungs-Weg entfernt (`Rejected` → `Accepted`) | `NichtEndlichWirdVomKernAbgelehnt` |
| §4-8 (a) | Volle-Konsumption-Prüfung entfernt | `DieUmwandlungVerlangtVolleKonsumption` + `NichtNumerischWirdInDerSenkeAbgelehnt` |
| §4-8 (b) | Umwandlungs-Prüfung **ganz** entfernt (`0` wird übernommen) | `NichtNumerischWirdInDerSenkeAbgelehnt` |
| §4-9 | Höhen-Weg auf den Stärke-Mutator verdrahtet | **beide** Höhen-Zeilen (Senke **und** Fenster-Kette) |
| §4-10 | `try`/`catch` der Barriere entfernt | `VeralteteWandIdWirdGefangenUndGemeldet` + 1 |
| §4-13 (a) | Anzeige-Form **mit Einheit** („240.0 mm") | 4 Fenster-Zeilen |
| §4-13 (b) | Anzeige-Form **mit Komma** („240,0") | dieselben 4 |

**§4-11 und §4-12 sind Netz:** der Zeichen- und Auswahl-Pfad lief unverändert grün (399 → 414 Tests,
**keine** Bestands-Zeile gefallen); Persistenz/Export prüfen `make io-smoke` und die Runden-Orakel.

**§4-13 ist die Zeile, die sich am meisten gelohnt hat.** Beide Sonden — Einheit und Komma — lassen
**vier** Fenster-Zeilen fallen. Ohne sie hätte das Produkt einen Fehler gehabt, den kein anderes
Orakel sieht: **Enter auf einem nie geänderten Feld ergäbe eine Ablehnung**, weil die Anzeige-Form
von der Eingabe-Umwandlung nicht gelesen werden kann. Der Befund stammt aus dem Plan-Review, und er
war beim Schreiben des Plans **nicht** offensichtlich.

### Was der Vollzug gegenüber dem Plan geändert hat

1. **Die Umwandlung ist eine eigene, öffentliche Naht** (`WallParamSink::parse`). Der Plan verlangte,
   dass Anzeige und Eingabe **dieselbe** Umwandlung benutzen; das Fenster kann die Senke aber nicht
   einbinden (`ui_view → ui_command` ist keine deklarierte Kante). Die zwei Seiten sind deshalb
   **nur über ein Orakel** gekoppelt (§4-13) — und das steht als Kommentar an **beiden** Enden.
2. **Der Bestands-Test `IsConstructibleHeadlessWithCentralWidget` wurde angepasst**, weil das Fenster
   das gereichte Widget nicht mehr direkt als Zentral-Widget setzt, sondern in eine Spalte mit dem
   Eigenschaften-Bereich legt (E4: fester Bereich, keine Andock-Verwaltung). Die Zusage war nie „es
   **ist** das Zentral-Widget", sondern „das Fenster **übernimmt** es" — jetzt geprüft über
   `central->window() == &window`, was beim Fallenlassen ebenso rot wird.
3. **`main` hat zwei Helfer bekommen** (`shownOutcome`, `makeParamActions`) und **die zwei
   Import-Blöcke sind gefaltet** (`runImportIfRequested`, Muster `runExportIfRequested`). Auslöser
   war der Lint-Gate (kognitive Komplexität 33 > 20); die Faltung entfernt eine **Zeichen-für-Zeichen
   -Dopplung**, die mit jedem Format weitergewachsen wäre — `make io-smoke` belegt beide Importe
   danach unverändert.
4. **Ein `bool editing_thickness` im Composition-Root** merkt sich, welches Feld die Meldung
   ausgelöst hat. Das ist die kleinste Naht, die den Rückweg ins **richtige** Feld führt, ohne dem
   Fenster einen zweiten Zustand zu geben.

### Reichweite und benannte Grenzen

- **Die Herkunft der angezeigten Werte hat keinen Sensor** (§4-1): eine Implementierung, die sie aus
  dem `Building` zöge, lieferte dieselben Zahlen, und `a-check` sieht es nicht (`ui_view → model` ist
  erlaubt). Die schmale Abfrage ist **Bauvorschrift** (E15), kein Beleg — so gebaut, so benannt.
- **Der Composition-Root bleibt orakel-los.** Deshalb liegt die Umwandlung in der Senke und der
  Rückweg im Fenster; beide Entscheidungen sind genau aus diesem Grund gefallen (§2.2/§2.3).
- **Die 2D-Sichtbarkeit einer Stärken-Änderung bekommt kein Orakel** und ist keine: der Canvas
  zeichnet Achsen, eine Stärken-Änderung bewegt keine.
- **Lastenheft und Spezifikation am Artefakt geprüft, nicht pauschal verneint:**
  [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) trägt seit
  [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md) den Block „Interaktive Änderung" mit
  allen vier Konjunkten (Happy · Klemmung sichtbar · Ablehnung sichtbar · Boundary (Auswahl)); alle
  vier haben jetzt einen Sensor (§4-3/§4-9 · §4-5/§4-6 · §4-7/§4-8 · §4-2). **Kein Spec-Diff nötig**
  — diesmal belegt.

### Wellen-Stand

**Der Abschluss-Trigger von welle-6 ist erfüllt:** eine Wand ist im 2D-Canvas **zeichenbar** und
**parametrisch änderbar**, ohne Kommandozeile. **Vor der Closure steht das
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
des Bauteil-Strangs** (Eckenschluss, Nachbar-Rebuild, Raum-Neuerkennung im interaktiven Pfad) —
HIGHs blockieren sie. Danach die drei Handgriffe aus §9: Closure-Datei, Meilenstein-Status M6,
Welle-Block.
