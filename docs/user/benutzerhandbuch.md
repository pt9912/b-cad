# Benutzerhandbuch: b-cad

Software-Version: 0.1.0
Handbuch-Version: 1.6
Stand: 2026-07-28

---

## 1. Einleitung

### Zweck der Software

**b-cad** ist eine Desktop-Anwendung zur **Erstellung, Bearbeitung, Analyse und
Visualisierung von Wohngebäuden** — Einfamilien- und Mehrfamilienhäuser, Anbauten,
Garagen, Nebengebäude. Gebäude werden **parametrisch** modelliert (Geschosse,
Wände, Türen, Fenster, Räume, Treppen, Dächer, Decken); der **2D-Grundriss** und
die **3D-Ansicht** leiten sich aus **einem durchgängigen Datenmodell** ab. Fertige
Modelle lassen sich in gängige Austauschformate exportieren (IFC, DXF, STEP, STL,
PDF, PNG) und als Projekt speichern. Zielgruppe des Programms: **private Bauherren
und professionelle Planer**.

### Was diese Version (0.1.0) leistet — und was noch nicht

Der oben genannte Zweck ist das **Ziel** des Produkts. Version 0.1.0 ist ein
**früher Stand**. Ehrlich eingeordnet:

**Heute möglich:**
- ein Gebäude **ansehen** — das mitgelieferte Beispiel, ein importiertes oder ein
  geöffnetes Projekt (3D-Ansicht + 2D-Grundriss),
- **Wände** im Grundriss zeichnen — Werkzeug **Wand** wählen und ziehen; die Wand
  erscheint sofort in 2D **und** 3D, rastet auf vorhandene Eckpunkte ein und
  überlebt Speichern/Öffnen und Export,
- **Hilfslinien** im Grundriss zeichnen — mit **Einrasten** auf vorhandene
  Eckpunkte, **sichtbar markiert, bevor Sie klicken**,
- in Formate **exportieren** (IFC/DXF/STEP/STL/PDF/PNG),
- ein Projekt **speichern** und **öffnen** (Menü **Datei** oder Kommandozeile),
- ein **neues, leeres Projekt anlegen** (Menü **Datei → Neu**) — mit einem
  Geschoss und einer Zeichen-Ebene, sofort bezeichenbar,
- Fremdformate **importieren** (IFC/DXF).

**In dieser Version noch NICHT möglich:**
- ein Gebäude **vollständig selbst planen**. Die **Wand** ist das erste Bauteil,
  das Sie in der Oberfläche selbst anlegen können (siehe oben) — **Räume, Türen,
  Fenster, Treppen und Dächer** nicht: für sie gibt es **weder** eine Bedienung in
  der Oberfläche **noch** eine Skript-/Definitionsdatei.
- eine gezeichnete Wand **wieder entfernen** oder eine Aktion **rückgängig
  machen**. Ein begonnener Zug lässt sich abbrechen (Escape), eine **fertige** Wand
  bleibt. Ungesicherte Änderungen sind durch die Rückfrage geschützt
  (Abschnitt 4.3).
- eine gezeichnete Wand **auswählen** und ihre Stärke oder Höhe ändern — sie
  entsteht mit den Standardwerten. Das folgt im nächsten Ausbauschritt.

Wie ein Gebäude heute überhaupt in b-cad kommt, beschreibt **Abschnitt 3**.

### Zielgruppe dieses Handbuchs

Anwenderinnen und Anwender, die mit b-cad ein Gebäudemodell ansehen, zeichnen,
speichern und exportieren möchten. Technisches Vorwissen ist nicht nötig; für
den fortgeschrittenen Kommandozeilen-Betrieb (Abschnitt 4.5) hilft etwas
Erfahrung mit einem Terminal.

### Voraussetzungen

b-cad läuft in einer vorbereiteten Container-Umgebung. Sie benötigen:

- **Docker** und **GNU make** auf Ihrem Rechner.
- Für die grafische Oberfläche eine **grafische Sitzung** (unter Linux mit
  X11/XWayland; eine Grafikkarte wird genutzt, sonst wird per Software gerendert).

Ein lokales Qt- oder CAD-SDK ist **nicht** erforderlich — alles Nötige ist im
Container enthalten.

---

## 2. Erste Schritte

### 2.1 b-cad starten

**Ziel:** Die grafische Oberfläche öffnen.

**Voraussetzung:** Docker und make sind installiert; Sie sind in einer grafischen
Sitzung angemeldet.

**Vorgehen:**

1. Öffnen Sie ein Terminal im Projektverzeichnis von b-cad.
2. Erlauben Sie dem Container einmalig den Zugriff auf Ihre Anzeige:
   ```
   xhost +local:
   ```
3. Starten Sie das Programm:
   ```
   make run
   ```

**Ergebnis:** Das b-cad-Fenster öffnet sich und zeigt das Beispielprojekt.

**Hinweis:** Erscheint kein Fenster, prüfen Sie Schritt 2 (`xhost +local:`). Nach
der Arbeit können Sie den Anzeige-Zugriff mit `xhost -local:` wieder entziehen.

### 2.2 Überblick über die Oberfläche

Das Fenster hat oben zwei Reiter:

- **3D** — die räumliche Ansicht des Gebäudes. Sie können das Modell mit der Maus
  drehen (Orbit) und so von allen Seiten betrachten.
- **2D** — die maßstäbliche Grundriss-Ansicht (Draufsicht) mit den Wand-Achsen und
  den Hilfslinien.

Sie wechseln die Ansicht durch Klick auf den jeweiligen Reiter.

In der **Menüleiste** liegen zwei Menüs: **Datei** (Abschnitt 4.3) und
**Werkzeug** — dort wählen Sie, was ein Zug im Reiter **2D** erzeugt:
**Hilfslinie** (Voreinstellung) oder **Wand**. Das gewählte Werkzeug ist im Menü
**markiert**.

Am **unteren Fensterrand** liegt eine **Hinweis-Zeile**. Dort erscheint ein kurzer
Text, wenn eine Eingabe **nicht** zum gewünschten Ergebnis geführt hat — sie
unterbricht Ihre Arbeit nicht und muss nicht weggeklickt werden.

### 2.3 Grundlegende Bedienung

- **Ansicht wechseln:** Reiter **3D** oder **2D** anklicken.
- **3D drehen:** Im Reiter **3D** mit der Maus ziehen.
- **Werkzeug wählen:** Menü **Werkzeug** → **Hilfslinie** (Voreinstellung) oder
  **Wand**. Der Zug ist derselbe — das Werkzeug entscheidet, was entsteht.
- **Hilfslinie oder Wand zeichnen:** Im Reiter **2D** mit gedrückter linker
  Maustaste ziehen (siehe Abschnitt 4.2). Nah an einem vorhandenen Eckpunkt
  **rastet** der Punkt exakt auf ihn ein.
- **Zug abbrechen:** **Escape** drücken, während Sie ziehen — es entsteht nichts.
- **Neues Projekt:** Menü **Datei** → **Neu** (siehe Abschnitt 4.3).
- **Projekt speichern/öffnen:** Menü **Datei** → **Speichern**,
  **Speichern unter…** bzw. **Öffnen…** (siehe Abschnitt 4.3).
- **Ungesicherte Änderungen:** Wenn Sie ein Projekt öffnen oder b-cad beenden,
  ohne Ihre Änderungen gespeichert zu haben, fragt b-cad nach — Sie können
  **speichern**, **verwerfen** oder die Aktion **abbrechen** (Abschnitt 4.3).

---

## 3. Ein Gebäude in b-cad bekommen und ansehen

Ein Gebäude **von Grund auf** selbst zu planen, geht in dieser Version noch nicht
— **Wände** können Sie selbst zeichnen (Abschnitt 4.2), die übrigen Bauteile nicht
(siehe Abschnitt 1). Es gibt **vier** Wege, mit einem Gebäude zu arbeiten:

1. **Das mitgelieferte Beispiel** — nach dem Start ist es sofort da (Abschnitt 2).
2. **Ein importiertes Gebäude** — aus einer IFC- oder DXF-Datei (Abschnitt 4.5).
3. **Ein geöffnetes Projekt** — eine zuvor gespeicherte `.bcad`-Datei; über
   **Datei → Öffnen…** in der Oberfläche (Abschnitt 4.3).
4. **Ein neues, leeres Projekt** — über **Datei → Neu** (Abschnitt 4.3). Es
   enthält ein Geschoss und eine Zeichen-Ebene; darin können Sie **Wände** und
   Hilfslinien zeichnen (Abschnitt 4.2), die übrigen Bauteile noch nicht.

### Das geladene Gebäude ansehen

**Voraussetzung:** b-cad ist gestartet (Abschnitt 2.1).

**Vorgehen:**

1. Betrachten Sie im Reiter **3D** das Gebäude; ziehen Sie mit der Maus, um es zu
   drehen.
2. Wechseln Sie auf den Reiter **2D**, um den Grundriss (Draufsicht) zu sehen.

**Ergebnis:** Sie sehen dasselbe Gebäudemodell einmal räumlich (3D) und einmal als
maßstäblichen Grundriss (2D). Beide Ansichten stammen aus **einem** Modell.

> **Hinweis:** **Wände** können Sie in dieser Version selbst anlegen
> (Abschnitt 4.2) — **Räume, Türen, Fenster, Treppen und Dächer** nicht, und
> **bearbeiten** (Stärke/Höhe ändern, entfernen) lässt sich auch eine gezeichnete
> Wand noch nicht. Sie können also ein bestehendes Gebäude ansehen, um Wände und
> Hilfslinien **ergänzen** und exportieren.

---

## 4. Aufgaben ausführen

### 4.1 Übersicht

| Aufgabe | Wo |
|---|---|
| Modell ansehen (3D/2D) | Oberfläche (Abschnitt 3) |
| **Wand zeichnen** (Werkzeug **Wand**, mit Einrasten) | Oberfläche, Reiter 2D (4.2) |
| Hilfslinie zeichnen (Einrasten mit sichtbarer Markierung) | Oberfläche, Reiter 2D (4.2) |
| Neues Projekt anlegen | Oberfläche, Menü **Datei → Neu** (4.3) |
| Projekt speichern / öffnen | Oberfläche, Menü **Datei** (4.3) |
| Ungesicherte Änderungen sichern | Rückfrage beim Öffnen/Beenden (4.3) |
| Modell exportieren (IFC/DXF/STEP/STL/PDF/PNG) | Kommandozeile (4.5) |
| Projekt importieren (IFC/DXF) | Kommandozeile (4.5) |
| Erweiterung (Plugin) laden | Kommandozeile (4.5) |

### 4.2 Eine Hilfslinie oder eine Wand zeichnen

**Ziel:** Eine gerade Hilfslinie als Zeichenhilfe **oder eine Wand** im Grundriss
anlegen. Die Geste ist **dieselbe** — das gewählte **Werkzeug** entscheidet, was
entsteht.

**Voraussetzung:** b-cad ist gestartet.

**Vorgehen:**

1. Wechseln Sie auf den Reiter **2D**.
2. Drücken Sie die **linke Maustaste** am gewünschten Startpunkt.
3. Ziehen Sie bei gedrückter Taste zum Endpunkt.
4. Lassen Sie die Maustaste los.

**Ergebnis:** Die Hilfslinie erscheint **sofort** im Grundriss. Sie gehört zum
Modell und erscheint auch im 2D-Export (DXF/PDF/PNG).

**Einrasten auf vorhandene Punkte (Fangen).** Führen Sie den Mauszeiger beim
Setzen von Anfang **oder** Ende nah an einen vorhandenen **Eckpunkt** — den
Endpunkt einer Wandachse oder einer bereits gezeichneten Hilfslinie —, dann
**rastet** der Punkt auf ihn ein: die Hilfslinie beginnt bzw. endet **exakt**
dort und nicht „ungefähr dort". So setzen Sie zwei Hilfslinien millimetergenau
aneinander, ohne die Ansicht vergrößern zu müssen. Der gefangene Wert bleibt
auch nach **Speichern und Öffnen** und im **Export** exakt derselbe.

**Hinweise:**
- Ziehen Sie ohne echte Länge (Start = Ende), entsteht **keine** Hilfslinie. Das
  gilt auch, wenn Anfang und Ende auf **denselben** Punkt einrasten.
- Sind Sie weiter als etwa eine Fingerbreite (rund 12 Bildschirmpunkte) von jedem
  Eckpunkt entfernt, wird **frei** gezeichnet — der Endpunkt ist dann genau die
  angeklickte Stelle.
- Eingerastet wird nur auf **sichtbare** Punkte im Sinne der Ebenen: liegt eine
  Hilfslinie auf einer ausgeblendeten Ebene, ist sie nicht fangbar.
- **Sie sehen vorher, worauf eingerastet wird:** sobald der Mauszeiger nah genug an
  einem Eckpunkt ist, erscheint dort eine Markierung — **bevor** Sie klicken. Sie
  gilt für Anfang **und** Ende: auch während Sie ziehen, zeigt sie das Ziel des
  Endpunkts an. Ist kein Punkt in Reichweite, verschwindet sie wieder.
- Gefangen wird auf Eckpunkte **aller Geschosse**, nicht nur des angezeigten. Ein
  Punkt kann also einrasten, ohne dass an dieser Stelle eine Linie gezeichnet ist —
  **die Markierung macht genau das sichtbar**, statt es zu verbergen.
- Verlassen Sie die Zeichenfläche oder ändern Sie die Ansicht (Zoomen,
  Fenstergröße, anderes Geschoss), verschwindet die Markierung. Sie erscheint mit
  der nächsten Mausbewegung wieder — **am Einrasten selbst ändert das nichts**.
- **Raster**, **Winkel-Bindung** und weitere Fang-Arten (Schnittpunkt, Mitte,
  Lot) sind noch nicht enthalten.

#### Eine Wand zeichnen

**Voraussetzung:** b-cad ist gestartet und ein Projekt ist geladen (das
mitgelieferte Beispiel genügt).

**Vorgehen:**

1. Wählen Sie **Werkzeug → Wand**. Die Auswahl bleibt, bis Sie sie ändern.
2. Wechseln Sie auf den Reiter **2D**.
3. Ziehen Sie mit gedrückter linker Maustaste von der Anfangs- zur Endposition.

**Ergebnis:** Es entsteht **eine Wand** mit Standard-Stärke und -Höhe. Sie
erscheint **sofort** im Grundriss **und** in der 3D-Ansicht, und sie überlebt
Speichern/Öffnen sowie den Export.

**Zwei Wände zu einer Ecke verbinden.** Setzen Sie den Punkt der zweiten Wand in
**Fang-Nähe** des Endpunkts der ersten (dasselbe Einrasten wie oben, Anfang **wie**
Ende). Dann teilen beide Wände **exakt** denselben Punkt, und die Ecke wird
geschlossen dargestellt — in 2D und in 3D. **Ohne** Einrasten gelingt das
praktisch nicht: die Toleranz liegt bei einem Zehntel Millimeter, ein
Bildschirmpunkt trägt bei normaler Ansicht rund zwei Zentimeter.

**Hinweise:**
- **Eine Wand je Zug.** Ein mehrpunktiger Wandzug in **einem** Zug ist noch nicht
  enthalten; mehrere Wände entstehen durch mehrere Züge, deren Endpunkte
  einrasten.
- **Ziehen ohne Länge** (Start = Ende) erzeugt **keine** Wand; in der Hinweis-Zeile
  am unteren Fensterrand steht dann „Keine Wand angelegt."
- **Abbrechen:** Solange Sie ziehen, brechen **Escape** oder ein Klick in ein
  anderes Fenster den Zug ab — es entsteht keine Wand.
- **Nicht rücknehmbar:** Eine **fertige** Wand können Sie in dieser Version nicht
  entfernen und nicht rückgängig machen. Speichern Sie vorher, wenn Sie zum alten
  Stand zurück wollen.
- **Stärke und Höhe** übernimmt b-cad aus den Standardwerten (die Höhe ist die
  Geschosshöhe); sie lassen sich in dieser Version **nachträglich nicht** ändern.
- Die Wand entsteht immer im **angezeigten** Geschoss.

### 4.3 Ein Projekt speichern und öffnen (Menü **Datei**)

**Ziel:** Den aktuellen Modellstand als Projektdatei (`.bcad`) sichern und ein
zuvor gespeichertes Projekt wieder zum Arbeitsstand machen.

**Voraussetzung:** b-cad ist gestartet (Abschnitt 2.1).

#### Speichern

1. Öffnen Sie das Menü **Datei** und wählen Sie **Speichern**.
2. Ist bereits eine Projektdatei bekannt — weil Sie das Projekt geöffnet oder
   schon einmal gespeichert haben —, wird **ohne weitere Rückfrage genau
   dorthin** geschrieben.
3. Ist noch keine bekannt, fragt b-cad **einmalig** nach Ordner und Dateinamen;
   die Endung `.bcad` wird ergänzt, wenn Sie keine angeben. Ab dann ist die
   Datei bekannt.

Mit **Speichern unter…** wählen Sie das Ziel **immer** selbst; die gewählte
Datei ist danach die bekannte.

**Ergebnis:** Die Projektdatei enthält den vollständigen Modellstand. Sie wird
**atomar** geschrieben: Schlägt das Schreiben fehl (z. B. Medium voll oder kein
Schreibrecht), bleibt eine **vorhandene** Datei unverändert — es entsteht **kein**
halb geschriebenes Projekt. Ein Fehler wird als Meldung angezeigt.

#### Ein neues Projekt anlegen

1. Öffnen Sie das Menü **Datei** und wählen Sie **Neu**.
2. Haben Sie ungesicherte Änderungen, erscheint zuerst die Rückfrage (siehe
   unten).

**Ergebnis:** Sie arbeiten in einem neuen Projekt mit **einem Geschoss** und
**einer Zeichen-Ebene** — Hilfslinien lassen sich sofort zeichnen. Es ist noch
**keine Datei zugeordnet**: das nächste **Speichern** fragt nach dem Ziel und
überschreibt **nicht** das zuvor geöffnete Projekt.

#### Ungesicherte Änderungen — die Rückfrage

b-cad vergleicht Ihren aktuellen Stand mit dem zuletzt gespeicherten. Weicht er
ab, erscheint vor jeder Aktion, die ihn verwerfen würde (**Neu**, **Öffnen**,
**b-cad beenden**), eine Rückfrage mit drei Möglichkeiten:

| Antwort | Was passiert |
|---|---|
| **Speichern** | Ihr Stand wird zuerst gesichert, danach läuft die Aktion. |
| **Verwerfen** | Die Aktion läuft, Ihre Änderungen gehen verloren. |
| **Abbrechen** | **Es passiert nichts** — die Aktion unterbleibt, Ihr Stand bleibt. |

Schlägt das Speichern dabei fehl oder brechen Sie die Ziel-Auswahl ab, wird die
auslösende Aktion **ebenfalls nicht** ausgeführt: ohne Ihre ausdrückliche
Entscheidung geht kein Stand verloren.

Nehmen Sie eine Änderung wieder zurück, sodass Ihr Stand dem gespeicherten
entspricht, gilt er wieder als gesichert — dann fragt b-cad auch nicht nach.

#### Öffnen

1. Öffnen Sie das Menü **Datei** und wählen Sie **Öffnen…**.
2. Haben Sie ungesicherte Änderungen, erscheint zuerst die Rückfrage (siehe
   oben).
3. Wählen Sie eine `.bcad`-Datei und bestätigen Sie.

**Ergebnis:** Der geladene Stand **ersetzt** Ihren bisherigen Arbeitsstand.
3D-Ansicht und Grundriss zeigen danach das geöffnete Projekt, der Fenstertitel
nennt den Dateinamen, und Sie können am geladenen Projekt weiterarbeiten, ohne
dass Vorhandenes überschrieben wird.

**Hinweise:**
- **Öffnen ist lesend:** Es wird **nichts** ergänzt. Speichern Sie unmittelbar
  nach dem Öffnen, entsteht derselbe Inhalt.
- Enthält das geöffnete Projekt **keine Zeichen-Ebene**, erscheint ein Hinweis:
  Hilfslinien lassen sich erst zeichnen, wenn eine Ebene vorhanden ist — b-cad
  legt sie **nicht** von sich aus an (das würde die Datei verändern).
- Lässt sich eine Datei nicht öffnen (nicht vorhanden, beschädigt, oder ein
  Bauteil ist nicht darstellbar), wird sie **als Ganzes** abgelehnt: Sie
  erhalten eine Meldung, und Ihr bisheriger Stand bleibt unverändert.
- Ein **Speichern** auf die zuletzt geöffnete Datei (ohne erneute Pfad-Auswahl)
  gibt es in dieser Version noch nicht; **Speichern unter…** fragt jedes Mal
  nach dem Ziel. Es gibt auch **keine** Warnung vor ungesicherten Änderungen —
  speichern Sie, bevor Sie ein anderes Projekt öffnen.

### 4.4 Welches Format wofür? (Kurzüberblick)

| Format | Zweck | Import | Export |
|---|---|---|---|
| **IFC** | Gebäudedaten-Austausch (BIM) | ✅ | ✅ |
| **DXF** | 2D-Grundriss-Austausch (CAD) | ✅ | ✅ |
| **STEP** | 3D-Volumenkörper (CAD/CAM) | — | ✅ |
| **STL** | 3D-Netz (z. B. 3D-Druck) | — | ✅ |
| **PDF** | maßstäblicher 2D-Plan (Vektor, zum Drucken) | — | ✅ |
| **PNG** | 2D-Plan als Bild (Raster) | — | ✅ |

Details und Grenzen: Abschnitt 6.

### 4.5 Kommandozeile (Export, Import, Speichern/Öffnen ohne Oberfläche)

**Export** und **Import** steuern Sie in dieser Version ausschließlich über
**Kommandozeilen-Optionen** des Programms `b-cad`; **Speichern** und **Öffnen**
gibt es hier **zusätzlich** zum Menü **Datei** (Abschnitt 4.3) — skriptbar, ohne
Oberfläche. Rufen Sie `b-cad` mit der passenden Option und einem Ziel-/Quellpfad
auf.

#### Modell exportieren

**Vorgehen:** Rufen Sie `b-cad` mit der Export-Option Ihres Formats und einem
Zielpfad auf. Verfügbare Optionen:

| Option | Ergebnis |
|---|---|
| `--export-ifc <pfad>` | IFC-Datei |
| `--export-dxf <pfad>` | DXF-Datei (2D-Grundriss) |
| `--export-step <pfad>` | STEP-Datei (3D) |
| `--export-stl <pfad>` | STL-Datei (3D) |
| `--export-pdf <pfad>` | PDF-Plan (maßstäblich) |
| `--export-png <pfad>` | PNG-Bild (Grundriss) |

Beispiel:
```
b-cad --export-pdf plan.pdf
```

**Ergebnis:** Die Zieldatei wird vollständig geschrieben. Schlägt das Schreiben
fehl (z. B. kein Schreibrecht), bleibt **kein** halb geschriebenes Ergebnis
zurück, und Sie erhalten eine Fehlermeldung (Abschnitt 7).

**Hinweis — Herkunft im Export:** PDF und PNG zeigen unten eine dezente Zeile mit
**Version, Quelle und Datum**; STEP und STL tragen dieselbe Herkunft im
Datei-Kopf. So erkennen Sie später, aus welchem Stand ein Export stammt, und
Exporte verschiedener Stände sind unterscheidbar. Die **Quelle** ist der Name
der geöffneten Projektdatei (siehe „Projekt öffnen"); ohne geöffnetes Projekt
bleibt sie leer.

#### Projekt speichern und öffnen

**Ziel:** Den aktuellen Modellstand als Projektdatei sichern und später wieder
laden.

| Option | Ergebnis |
|---|---|
| `--save <pfad.bcad>` | Projekt speichern (atomar) |
| `--open <pfad.bcad>` | Projekt öffnen (als **Quelle für Export**, siehe Hinweis) |

Beispiel — speichern, dann öffnen und als PDF exportieren:
```
b-cad --save haus.bcad
b-cad --open haus.bcad --export-pdf haus.pdf
```

**Ergebnis:** `--save` schreibt die Projektdatei **atomar** — bei einem Fehler
(z. B. Medium voll) bleibt eine vorhandene Datei unverändert. `--open` lädt das
Projekt vollständig wieder (Modell, Geometrie, Materialzuordnungen). Ein Export
nach `--open` trägt den Datei**namen** als Herkunft.

> **Wichtiger Unterschied zum Menü Datei.** Ein Aufruf mit `--open` startet
> **keine** Oberfläche: das Programm lädt das Projekt, führt die angegebenen
> Aufgaben (Speichern/Export) aus und endet. Der geladene Stand wird hier also
> **nicht** zu einem bearbeitbaren Arbeitsstand — dafür ist **Datei → Öffnen…**
> in der Oberfläche da (Abschnitt 4.3). Auf der Kommandozeile ist `--open` eine
> **Export-Quelle**, im Menü ist Öffnen ein **Sitzungs-Wechsel**.

**Hinweis:** Öffnen einer nicht vorhandenen oder beschädigten Datei bricht mit
einer Fehlermeldung ab (kein Absturz).

#### Projekt importieren (IFC / DXF)

| Option | Ergebnis |
|---|---|
| `--import-ifc <pfad>` | IFC-Datei einlesen |
| `--import-dxf <pfad>` | DXF-Datei einlesen |

**Hinweis:** b-cad liest ein definiertes **Teil-Set** dieser Formate (z. B. beim
DXF ebene `LINE`-Grundrisse; 3D-Inhalte werden übersprungen). Eine Datei, die
nichts Passendes enthält, wird abgelehnt oder ergibt ein leeres Modell.

#### Erweiterung (Plugin) laden

| Option | Ergebnis |
|---|---|
| `--plugin <pfad>` | Erweiterung beim Start laden |

**Hinweis:** Ein abgelehntes oder fehlerhaftes Plugin führt **nicht** zum Absturz
und ändert das Modell nicht; Annahme wie Ablehnung werden gemeldet.

---

## 5. Einstellungen und Rollen

b-cad kennt in dieser Version **keine Benutzerkonten, Rollen oder Rechte** — alle
Funktionen stehen jeder anwendenden Person offen. Es gibt auch keine gesonderte
Einstellungsmaske; das Verhalten ist fest vorgegeben (z. B. PDF-Maßstab 1:100).

---

## 6. Import und Export im Detail

- **IFC** und **DXF** unterstützen **Import und Export**; STEP, STL, PDF und PNG
  sind **Export-only** (ein Import-Versuch wird abgelehnt).
- **PDF** ist ein **maßstäblicher Vektor-Plan** (fester Maßstab 1:100, zum
  Drucken geeignet); **PNG** ist derselbe Grundriss als **Rasterbild**.
- **STEP** enthält die Bauteile als 3D-Volumenkörper; **STL** als 3D-Dreiecksnetz
  (z. B. für den 3D-Druck).
- Alle Exporte werden **atomar** geschrieben (vollständige Datei oder gar keine
  Änderung am Zielpfad).
- **Grenze:** b-cads Import/Export deckt bewusst ein Teil-Set der Formate ab; sehr
  spezielle Konstrukte fremder Programme werden ggf. übersprungen.

---

## 7. Fehlerbehebung

### Das Fenster öffnet sich nicht (`make run`)

**Ursache:** Der Container darf nicht auf Ihre Anzeige zugreifen.

**Lösung:**
1. Führen Sie `xhost +local:` aus.
2. Starten Sie `make run` erneut.

### „…-Export fehlgeschlagen" / „Speichern fehlgeschlagen"

**Ursache:** Der Zielpfad ist nicht beschreibbar (fehlendes Schreibrecht, nicht
vorhandenes Verzeichnis) oder das Medium ist voll.

**Lösung:**
1. Wählen Sie ein Verzeichnis, in dem Sie Schreibrechte haben.
2. Prüfen Sie den freien Speicherplatz.
3. Starten Sie den Befehl erneut.

**Gut zu wissen:** Bei einem Schreibfehler bleibt eine bereits vorhandene Datei
**unverändert** — es entsteht nie ein halb geschriebenes Ergebnis.

### „Öffnen fehlgeschlagen"

**Ursache:** Die Projektdatei existiert nicht, ist beschädigt oder wurde nicht
von b-cad geschrieben.

**Lösung:** Prüfen Sie Pfad und Dateiname; öffnen Sie eine von b-cad erzeugte
`.bcad`-Datei (**Datei → Speichern unter…** oder `--save`).

**Gut zu wissen:** Eine Datei, die sich nicht vollständig laden lässt, wird **als
Ganzes** abgelehnt — Ihr bisheriger Arbeitsstand bleibt unverändert; es entsteht
nie ein halb geladenes Projekt.

### Import ergibt ein (fast) leeres Modell

**Ursache:** Die Quelldatei enthält keine von b-cad gelesenen Inhalte (z. B. eine
reine 3D-DXF, während b-cad 2D-Linien liest).

**Lösung:** Verwenden Sie eine Datei mit passendem Inhalt (z. B. einen
2D-Grundriss mit `LINE`-Elementen beim DXF-Import).

---

## 8. FAQ

**Kann ich in der Oberfläche Wände zeichnen?**
Ja — seit Handbuch-Version 1.6. Wählen Sie **Werkzeug → Wand** und ziehen Sie im
Reiter **2D** (Abschnitt 4.2). Es entsteht **eine** Wand je Zug, mit
Standard-Stärke und -Höhe; sie rastet auf vorhandene Eckpunkte ein und ist damit
zu geschlossenen Ecken verbindbar. Was noch **nicht** geht: die Wand wieder
**entfernen**, ihre Stärke/Höhe **nachträglich ändern** und andere Bauteile
(Räume, Türen, Fenster, Treppen, Dächer) interaktiv anlegen.

**Wie speichere/öffne ich über ein Menü?**
Über das Menü **Datei** in der Oberfläche: **Speichern**, **Speichern unter…**
und **Öffnen…** (Abschnitt 4.3). **Speichern** schreibt ohne erneute
Pfad-Auswahl in die bekannte Projektdatei; **Speichern unter…** fragt immer nach
dem Ziel. Skriptbar bleibt der Weg über `--save`/`--open` (Abschnitt 4.5).

**Warnt mich b-cad, bevor ungesicherte Änderungen verloren gehen?**
Ja. Beim **Öffnen** eines anderen Projekts und beim **Beenden** fragt b-cad
nach, wenn Ihr Stand vom zuletzt gespeicherten abweicht — mit **Speichern**,
**Verwerfen** und **Abbrechen** (Abschnitt 4.3). Abbrechen lässt alles, wie es
ist.

**Woran erkenne ich, aus welchem Stand ein Export stammt?**
An der Herkunfts-Angabe (Version, Quelle, Datum) — sichtbar in PDF/PNG und im
Kopf von STEP/STL.

**Welche Formate kann ich importieren?**
IFC und DXF. STEP, STL, PDF und PNG sind reine Ausgabe-Formate.

---

## 9. Glossar

| Begriff | Bedeutung |
|---|---|
| **Gebäudemodell** | Das durchgängige Datenmodell, aus dem 2D- und 3D-Ansicht abgeleitet werden. |
| **Grundriss** | Die maßstäbliche Draufsicht (2D-Ansicht). |
| **Hilfslinie** | Eine gerade Zeichenhilfe im Grundriss (keine Wand). |
| **Projektdatei (`.bcad`)** | Die gespeicherte Datei mit dem vollständigen Modellstand. |
| **Export** | Ausgabe des Modells in ein Austauschformat (IFC/DXF/STEP/STL/PDF/PNG). |
| **Plugin** | Eine ladbare Erweiterung. |
| **Herkunft** | Version, Quelle und Datum, die einem Export mitgegeben werden. |

---

## 10. Anhang

### 10.1 Kommandozeilen-Optionen (Übersicht)

| Option | Wirkung |
|---|---|
| `--export-ifc/-dxf/-step/-stl/-pdf/-png <pfad>` | Modell exportieren |
| `--import-ifc/-dxf <pfad>` | Projekt importieren |
| `--save <pfad.bcad>` | Projekt speichern |
| `--open <pfad.bcad>` | Projekt öffnen |
| `--plugin <pfad>` | Erweiterung laden |

### 10.2 Feste Werte

- PDF-Maßstab: **1:100**.
- Seitengröße PDF: A4.

### 10.3 Lizenz / Weitergabe

Achten Sie beim Weitergeben importierter Fremd-Dateien auf deren Lizenzbedingungen.

---

## 11. Änderungshistorie

| Handbuch-Version | Software-Version | Stand | Änderung |
|---|---|---|---|
| 1.0 | 0.1.0 | 2026-07-24 | Erstfassung: Start, Ansichten, Hilfslinie, Export/Speichern/Öffnen/Import über die Kommandozeile, Fehlerbehebung. |
| 1.6 | 0.1.0 | 2026-07-29 | **Wände zeichnen** aufgenommen — das erste Bauteil, das in der Oberfläche selbst entsteht: neues **Werkzeug**-Menü (Hilfslinie/Wand, Voreinstellung Hilfslinie), neue **Hinweis-Zeile** am unteren Fensterrand, neuer Unterabschnitt „Eine Wand zeichnen" in 4.2 (samt Eckenschluss über das Einrasten und der Begründung, warum es ohne Einrasten praktisch nicht gelingt) und die vier benannten Grenzen: **eine** Wand je Zug, **kein** Entfernen/Rückgängig, **keine** nachträgliche Änderung von Stärke/Höhe, Zeichnen nur im angezeigten Geschoss. §1 ist **präzisiert statt gestrichen** („ein Gebäude **vollständig** selbst planen" — Wände ja, übrige Bauteile nein), ebenso §2.2/§2.3, der Kopf von §3, der §3-Hinweis, die 4.1-Aufgaben-Tabelle und die FAQ-Antwort „Kann ich in der Oberfläche Wände zeichnen?", die bis hierher **verneinte**. |
| 1.5 | 0.1.0 | 2026-07-28 | **Anzeige des Fang-Ziels** aufgenommen: eine Markierung zeigt **vor dem Klick**, auf welchen Eckpunkt eingerastet würde — für Anfang **und** Ende, auch während des Ziehens. Damit ist die in 1.4 benannte Grenze „Eine Anzeige, **worauf** gerade eingerastet wird, gibt es in dieser Version noch nicht" **aufgehoben und ersetzt**; zugleich ist der bisher nur als Einschränkung beschriebene Geschoss-übergreifende Fang jetzt als **Nutzen** formuliert (die Markierung macht sichtbar, was sonst als Sprung erschien). Neu benannt: die Markierung verschwindet beim Verlassen der Zeichenfläche und bei Ansichts-Änderungen und kehrt mit der nächsten Mausbewegung zurück — **am Einrasten selbst ändert das nichts**. §1 „Heute möglich" und die 4.1-Aufgaben-Tabelle nachgezogen. |
| 1.4 | 0.1.0 | 2026-07-28 | **Einrasten (Fangen)** beim Zeichnen von Hilfslinien aufgenommen: neuer Absatz in 4.2 (Anfang **wie** Ende rasten exakt auf Endpunkte von Wandachsen und Hilfslinien ein; der gefangene Wert überlebt Speichern/Öffnen und Export unverändert) samt der vier benannten Grenzen — freies Zeichnen außerhalb von rund 12 Bildschirmpunkten, Entartung durch beidseitiges Einrasten, keine Fangbarkeit auf ausgeblendeten Ebenen, **keine Anzeige** des Fang-Ziels und Fangen über **alle** Geschosse. §1 „Heute möglich", §2.3 und die 4.1-Aufgaben-Tabelle nachgezogen; der frühere Satz „In dieser Version wird **frei** gezeichnet; Fangen … sind noch nicht enthalten" ist damit überholt und ersetzt. |
| 1.3 | 0.1.0 | 2026-07-27 | **Datei → Neu** aufgenommen: eigener Unterabschnitt in 4.3 (Inhalt des neuen Projekts — ein Geschoss, eine Zeichen-Ebene, sofort bezeichenbar — und die Zusage, dass **keine Datei zugeordnet** ist, das nächste Speichern also nach dem Ziel fragt statt das zuvor geöffnete Projekt zu überschreiben); „Neu" als dritter Auslöser der Rückfrage ergänzt; §1 „Heute möglich", §2.3, die 4.1-Aufgaben-Tabelle und die Wege-Zählung in §3 (**drei → vier**) nachgezogen. |
| 1.2 | 0.1.0 | 2026-07-27 | **Speichern** (auf die bekannte Projektdatei, ohne erneute Ziel-Abfrage) und die **Rückfrage vor ungesicherten Änderungen** aufgenommen: neuer Unterabschnitt in 4.3 mit der dreiwertigen Antwort (speichern/verwerfen/**abbrechen ⇒ es passiert nichts**), Ergänzung in 2.3, neue Zeile in der 4.1-Aufgaben-Tabelle und zwei FAQ-Einträge — die frühere FAQ-Aussage „Ein ‚Speichern' auf die zuletzt geöffnete Datei gibt es noch nicht" ist damit überholt und ersetzt. |
| 1.1 | 0.1.0 | 2026-07-26 | Menü **Datei** (Speichern unter…/Öffnen…) als neuer Abschnitt 4.3 aufgenommen — das Handbuch beschrieb Speichern/Öffnen bis dahin als reine Kommandozeilen-Aufgabe und die FAQ verneinte ein Datei-Menü. Der Unterschied der beiden Wege ist jetzt benannt: im Menü ist Öffnen ein **Sitzungs-Wechsel**, auf der Kommandozeile ist `--open` eine **Export-Quelle**. Frühere 4.3/4.4 zu 4.4/4.5 verschoben. |
