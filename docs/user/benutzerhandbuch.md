# Benutzerhandbuch: b-cad

Software-Version: 0.1.0
Handbuch-Version: 1.0
Stand: 2026-07-24

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
- **Hilfslinien** im Grundriss zeichnen,
- in Formate **exportieren** (IFC/DXF/STEP/STL/PDF/PNG),
- ein Projekt **speichern** und **öffnen**,
- Fremdformate **importieren** (IFC/DXF).

**In dieser Version noch NICHT möglich:**
- ein Gebäude **selbst planen** — Wände, Räume, Türen, Fenster, Treppen, Dächer
  interaktiv anlegen und bearbeiten. Es gibt dafür **weder** eine Bedienung in der
  Oberfläche **noch** eine Skript-/Definitionsdatei. Das interaktive Planen ist der
  **nächste große Ausbauschritt** (siehe FAQ, Abschnitt 8).

Wie ein Gebäude heute überhaupt in b-cad kommt, beschreibt **Abschnitt 3**.

### Zielgruppe dieses Handbuchs

Anwenderinnen und Anwender, die mit b-cad ein Gebäudemodell ansehen, zeichnen,
speichern und exportieren möchten. Technisches Vorwissen ist nicht nötig; für
den fortgeschrittenen Kommandozeilen-Betrieb (Abschnitt 4.4) hilft etwas
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

### 2.3 Grundlegende Bedienung

- **Ansicht wechseln:** Reiter **3D** oder **2D** anklicken.
- **3D drehen:** Im Reiter **3D** mit der Maus ziehen.
- **Hilfslinie zeichnen:** Im Reiter **2D** mit gedrückter linker Maustaste ziehen
  (siehe Abschnitt 4.2).

---

## 3. Ein Gebäude in b-cad bekommen und ansehen

Da Sie ein Gebäude in dieser Version **nicht selbst planen** (siehe Abschnitt 1),
gibt es **drei** Wege, mit einem Gebäude zu arbeiten:

1. **Das mitgelieferte Beispiel** — nach dem Start ist es sofort da (Abschnitt 2).
2. **Ein importiertes Gebäude** — aus einer IFC- oder DXF-Datei (Abschnitt 4.4).
3. **Ein geöffnetes Projekt** — eine zuvor gespeicherte `.bcad`-Datei (Abschnitt 4.4).

### Das geladene Gebäude ansehen

**Voraussetzung:** b-cad ist gestartet (Abschnitt 2.1).

**Vorgehen:**

1. Betrachten Sie im Reiter **3D** das Gebäude; ziehen Sie mit der Maus, um es zu
   drehen.
2. Wechseln Sie auf den Reiter **2D**, um den Grundriss (Draufsicht) zu sehen.

**Ergebnis:** Sie sehen dasselbe Gebäudemodell einmal räumlich (3D) und einmal als
maßstäblichen Grundriss (2D). Beide Ansichten stammen aus **einem** Modell.

> **Hinweis:** Das interaktive **Anlegen und Bearbeiten** von Wänden, Räumen,
> Türen, Fenstern, Treppen und Dächern ist in dieser Version noch nicht verfügbar.
> Sie können ein bereits bestehendes Gebäude ansehen, mit Hilfslinien versehen und
> exportieren — aber (außer Hilfslinien) noch nichts an der Bausubstanz ändern.

---

## 4. Aufgaben ausführen

### 4.1 Übersicht

| Aufgabe | Wo |
|---|---|
| Modell ansehen (3D/2D) | Oberfläche (Abschnitt 3) |
| Hilfslinie zeichnen | Oberfläche, Reiter 2D (4.2) |
| Modell exportieren (IFC/DXF/STEP/STL/PDF/PNG) | Kommandozeile (4.4) |
| Projekt speichern / öffnen | Kommandozeile (4.4) |
| Projekt importieren (IFC/DXF) | Kommandozeile (4.4) |
| Erweiterung (Plugin) laden | Kommandozeile (4.4) |

### 4.2 Eine Hilfslinie zeichnen

**Ziel:** Eine gerade Hilfslinie als Zeichenhilfe im Grundriss anlegen.

**Voraussetzung:** b-cad ist gestartet.

**Vorgehen:**

1. Wechseln Sie auf den Reiter **2D**.
2. Drücken Sie die **linke Maustaste** am gewünschten Startpunkt.
3. Ziehen Sie bei gedrückter Taste zum Endpunkt.
4. Lassen Sie die Maustaste los.

**Ergebnis:** Die Hilfslinie erscheint **sofort** im Grundriss. Sie gehört zum
Modell und erscheint auch im 2D-Export (DXF/PDF/PNG).

**Hinweise:**
- Ziehen Sie ohne echte Länge (Start = Ende), entsteht **keine** Hilfslinie.
- In dieser Version wird **frei** gezeichnet; Fangen, Raster und Winkel-Bindung
  sind noch nicht enthalten.

### 4.3 Welches Format wofür? (Kurzüberblick)

| Format | Zweck | Import | Export |
|---|---|---|---|
| **IFC** | Gebäudedaten-Austausch (BIM) | ✅ | ✅ |
| **DXF** | 2D-Grundriss-Austausch (CAD) | ✅ | ✅ |
| **STEP** | 3D-Volumenkörper (CAD/CAM) | — | ✅ |
| **STL** | 3D-Netz (z. B. 3D-Druck) | — | ✅ |
| **PDF** | maßstäblicher 2D-Plan (Vektor, zum Drucken) | — | ✅ |
| **PNG** | 2D-Plan als Bild (Raster) | — | ✅ |

Details und Grenzen: Abschnitt 6.

### 4.4 Kommandozeile (Export, Speichern, Öffnen, Import)

In dieser Version steuern Sie Export, Speichern, Öffnen und Import über
**Kommandozeilen-Optionen** des Programms `b-cad`. Rufen Sie es mit der
passenden Option und einem Ziel-/Quellpfad auf.

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
| `--open <pfad.bcad>` | Projekt öffnen (als Quelle für Export) |

Beispiel — speichern, dann öffnen und als PDF exportieren:
```
b-cad --save haus.bcad
b-cad --open haus.bcad --export-pdf haus.pdf
```

**Ergebnis:** `--save` schreibt die Projektdatei **atomar** — bei einem Fehler
(z. B. Medium voll) bleibt eine vorhandene Datei unverändert. `--open` lädt das
Projekt vollständig wieder (Modell, Geometrie, Materialzuordnungen). Ein Export
nach `--open` trägt den Datei**namen** als Herkunft.

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

**Lösung:** Prüfen Sie Pfad und Dateiname; öffnen Sie eine mit `--save` erstellte
`.bcad`-Datei.

### Import ergibt ein (fast) leeres Modell

**Ursache:** Die Quelldatei enthält keine von b-cad gelesenen Inhalte (z. B. eine
reine 3D-DXF, während b-cad 2D-Linien liest).

**Lösung:** Verwenden Sie eine Datei mit passendem Inhalt (z. B. einen
2D-Grundriss mit `LINE`-Elementen beim DXF-Import).

---

## 8. FAQ

**Kann ich in der Oberfläche Wände zeichnen?**
Noch nicht. In Version 0.1.0 zeigt die Oberfläche ein Beispielprojekt und erlaubt
das Zeichnen von Hilfslinien; das interaktive Bauteil-Zeichnen folgt in einer
späteren Version.

**Wie speichere/öffne ich über ein Menü?**
Ein „Datei"-Menü ist in dieser Version noch nicht enthalten — nutzen Sie
`--save`/`--open` auf der Kommandozeile (Abschnitt 4.4).

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
