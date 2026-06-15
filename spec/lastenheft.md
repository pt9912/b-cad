# Lastenheft — b-cad

**Version:** 0.1.7
**Status:** Draft
**Autor:** Dietmar Burkard, **Datum:** 2026-06-08

**Reifephase (Bootstrap):** Outline (Phase 2). Top-Level und ID-Schema
stehen; Akzeptanzkriterien sind für die Welle-1-Anforderungen
ausformuliert, für spätere Module zunächst als Beschreibung skizziert
und werden pro Slice geschärft.

## 1. Zweck und Geltungsbereich

b-cad ist eine **Desktop-Anwendung zur Erstellung, Bearbeitung, Analyse
und Visualisierung von Wohngebäuden** — Einfamilien- und
Mehrfamilienhäuser, Anbauten, Garagen und Nebengebäude. Gebäude werden
**parametrisch** modelliert; 2D- und 3D-Darstellung leiten sich aus
**einem durchgängigen Datenmodell** ab.

Die Software richtet sich sowohl an private Bauherren (geführte,
CAD-kenntnisfreie Bedienung) als auch an professionelle Planer
(vollständige Planung mit offenen Austauschformaten).

Nicht hier (gehört in [`spezifikation.md`](spezifikation.md) /
[`architecture.md`](architecture.md) und die ADRs): *wie* das System
gebaut wird — Geometrie-Kern, GUI-Framework, Persistenz, Build.

## 2. Stakeholder und Rollen

| ID | Rolle | Erwartung |
|---|---|---|
| ROLE-001 | Bauherr | Kann Gebäude erstellen und visualisieren — ohne tiefe CAD-Kenntnisse. |
| ROLE-002 | Architekt / Planer | Kann vollständige Planungen durchführen und offen exportieren. |
| ROLE-003 | Administrator | Kann Bibliotheken (Material, Bauteile) und Systemeinstellungen verwalten. |

## 3. Projektziele

| ID | Ziel | Beschreibung |
|---|---|---|
| OBJ-001 | Einfache Gebäudeplanung | Gebäude ohne tiefe CAD-Kenntnisse modellierbar. |
| OBJ-002 | Parametrische Modellierung | Alle Bauteile parametrisch bearbeitbar. |
| OBJ-003 | Durchgängiges Gebäudemodell | 2D- und 3D-Darstellung aus demselben Datenmodell. |
| OBJ-004 | Erweiterbarkeit | Erweiterung durch Plugins. |
| OBJ-005 | Offene Austauschformate | Import/Export offener Formate (IFC, DXF, STEP, STL). |

## 4. Funktionale Anforderungen

ID-Schema: `LH-FA-<BEREICH>-<NNN>` (siehe
[`harness/conventions.md` MR-002](../harness/conventions.md#mr-002--id-schema-für-b-cad)).
Vollständig ausformulierte Akzeptanzkriterien (Happy / Boundary /
Negative) tragen die Welle-1-Anforderungen; die übrigen sind als
Outline geführt und werden im jeweiligen Slice auf Akzeptanz-Niveau
geschärft.

### Modul Gebäude / Projekt (`BLD`)

#### LH-FA-BLD-001 — Projekt anlegen

**Beschreibung:** Der Benutzer erzeugt ein neues, leeres Gebäudeprojekt.

**Akzeptanzkriterien:**

- **Happy Path:** Given keine offene Datei, when „Neues Projekt", then
  ein leeres Projekt mit genau einem Geschoss (EG, Default-Höhe aus
  [`spezifikation.md`](spezifikation.md)) und einem leeren Modellbaum.
- **Boundary:** Given ein bereits geöffnetes, ungespeichertes Projekt,
  when „Neues Projekt", then Rückfrage „Änderungen verwerfen?" vor dem
  Anlegen.
- **Negative:** Given kein Schreibrecht im Default-Projektpfad, when
  Projekt anlegen, then Fehler-Code [`E-IO-001`](spezifikation.md#4-fehler-codes-und-logging-felder), kein leerer
  Projektzustand mit verlorenem Vorgänger.

**Out-of-Scope:** Projektvorlagen-Galerie (spätere Welle).

#### LH-FA-BLD-002 — Projekt speichern

**Beschreibung:** Das Projekt wird **persistent und atomar** in eine
Projektdatei (SQLite) gespeichert.

**Akzeptanzkriterien:**

- **Happy Path:** Given geändertes Projekt, when „Speichern", then
  Projektdatei enthält den vollständigen Modellstand; erneutes Laden
  ergibt ein identisches Modell.
- **Boundary:** Given Absturz während des Schreibens, when Neustart,
  then der letzte konsistente Stand ist intakt (kein halb geschriebenes
  Projekt — vgl. LH-QA-005).
- **Negative:** Given Zielmedium voll, when „Speichern", then
  Fehler-Code [`E-IO-002`](spezifikation.md#4-fehler-codes-und-logging-felder), vorheriger Dateistand unverändert.

#### LH-FA-BLD-003 — Projekt laden

Projekt kann erneut geöffnet werden; Modellbaum, Geometrie und
Materialzuordnungen werden vollständig wiederhergestellt.

#### LH-FA-BLD-004 — Projektversionierung

Änderungshistorie des Projekts wird gespeichert (Wiederherstellung
früherer Stände). *Outline — Abgrenzung zu Undo/Redo (LH-QA-003) im
Slice klären.*

### Modul Geschosse (`FLR`)

- **LH-FA-FLR-001 — Geschoss anlegen.**
- **LH-FA-FLR-002 — Geschoss löschen.**
- **LH-FA-FLR-003 — Geschoss kopieren** (inkl. aller Bauteile).
- **LH-FA-FLR-004 — Geschosshöhe definieren.**
- **LH-FA-FLR-005 — Geschossreihenfolge ändern.**

### Modul Wände (`WAL`)

#### LH-FA-WAL-001 — Wand zeichnen

**Beschreibung:** Wände werden durch Linienzüge erzeugt; aufeinander
folgende Segmente bilden zusammenhängende Wandzüge.

**Akzeptanzkriterien:**

- **Happy Path:** Given aktives Geschoss, when ein Linienzug mit ≥ 2
  Punkten gezeichnet wird, then je Segment eine Wand mit Default-Stärke
  und -Höhe; verbundene Endpunkte werden geometrisch verbunden.
- **Boundary:** Given zwei Punkte mit Abstand 0, when Segment beendet,
  then keine Null-Längen-Wand (verworfen, Hinweis).
- **Negative:** Given Eingabe außerhalb des Zeichenbereichs, when
  Punkt gesetzt, then Fehler-Code [`E-GEO-001`](spezifikation.md#4-fehler-codes-und-logging-felder).

#### LH-FA-WAL-002 — Wandstärke definieren

**Beschreibung:** Wandstärke ist parametrisch im Bereich **50 mm bis
1000 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Wand, when Stärke = 240 mm, then Geometrie
  und 3D-Körper aktualisieren sich sofort (vgl. LH-FA-D3-002).
- **Boundary:** Given Stärke = 50 mm oder 1000 mm, then akzeptiert;
  Given 49 mm oder 1001 mm, then auf Grenzwert geklemmt + Hinweis,
  Fehler-Code [`E-VAL-001`](spezifikation.md#4-fehler-codes-und-logging-felder).

#### LH-FA-WAL-003 — Wandhöhe definieren

Parametrisch im Bereich **500 mm bis 10000 mm** (Grenzwert-Verhalten
analog LH-FA-WAL-002).

- **LH-FA-WAL-004 — Wand verschieben.**
- **LH-FA-WAL-005 — Wand teilen.**

#### LH-FA-WAL-006 — Wand verbinden

**Beschreibung:** Wände, die sich berühren, verbinden sich zu einem
geschlossenen Wandkörper.

**Teilumfang Eckenschluss (geschärft 2026-06-12, slice-012; Auslöser:
ACC-002-Abnahme-Befund):** Gilt für Wände, die sich an einem
**gemeinsamen Endpunkt** im **selben Geschoss** treffen — genau zwei
Wände am Punkt.

**Akzeptanzkriterien (Teilumfang):**

- **Happy Path:** Given zwei Wände gleicher Höhe mit gemeinsamem
  Endpunkt im Winkel, then zeigt die Darstellung (2D und 3D) eine
  **geschlossene Ecke ohne Kerbe oder Loch** — der äußere Eckbereich
  ist körperlich gefüllt.
- **Boundary:** Given eine kollineare Fortsetzung gleicher Stärke,
  then ein glatter Übergang; ungleicher Stärke, then ein stumpfer
  Stoß ohne Loch. Given ein sehr spitzer Winkel, then ragt die
  Eck-Geometrie **höchstens um die größere der beiden Wandstärken
  über den gemeinsamen Endpunkt hinaus** — sonst enden beide Wände
  stumpf. Given ungleiche Wandhöhen, then ist die Ecke bis zur
  niedrigeren Wandhöhe geschlossen, darüber endet die höhere Wand
  stumpf.
- **Negative/Abgrenzung:** Given drei oder mehr Wände am selben
  Punkt, oder eine Berührung ohne gemeinsamen Endpunkt (T-Stoß,
  Kreuzung), then bleiben die Enden unverändert stumpf. Der
  **Vollumfang** (Schnittpunkte als Verbindungs-Knoten) ist
  ausdrücklich **nicht** Teil dieser Schärfung und bleibt offen.

#### LH-FA-WAL-007 — Wandtyp wählen

Wandtyp aus {Innenwand, Außenwand, Tragwand}; Typ steuert
Default-Material und Auswertungs-Kategorie.

### Modul Räume (`ROM`)

#### LH-FA-ROM-001 — Raum automatisch erkennen

**Beschreibung:** Geschlossene Wandzüge bilden automatisch Räume.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein geschlossener Wandzug, when er geschlossen
  wird, then genau ein Raum mit korrektem Polygon entsteht.
- **Boundary:** Given verschachtelte geschlossene Wandzüge, then innerer
  und äußerer Raum werden getrennt erkannt (keine Doppelzählung der
  Fläche).
- **Negative:** Given ein offener Wandzug, then kein Raum; kein Fehler.

- **LH-FA-ROM-002 — Raumfläche berechnen.**
- **LH-FA-ROM-003 — Raumvolumen berechnen.**
- **LH-FA-ROM-004 — Raumbezeichnung.**
- **LH-FA-ROM-005 — Raumkategorien** {Wohnen, Schlafen, Küche, Bad,
  Technik, Sonstige}.

### Modul Türen (`DOR`)

Türen sind **wand-gehostete Bauteile**: eine platzierte Tür sitzt in
einer Wirtswand und bricht diese an ihrer Stelle durch (Öffnung). Die
folgenden Anforderungen wurden 2026-06-13 (slice-013a) von Outline auf
Akzeptanz-Niveau geschärft (Reifephase-Klausel).

#### LH-FA-DOR-001 — Tür platzieren

**Beschreibung:** Eine Tür wird an einer Wand platziert; ihre Lage ist
parametrisch entlang der Wand.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Wand, when eine Tür an einer Position
  entlang der Wand platziert wird, then sitzt die Tür in der Wand und
  die Wand ist an dieser Stelle durchbrochen (in der 3D-Darstellung
  sichtbar, vgl. LH-FA-DOR-004), ohne Benutzer-Refresh.
- **Boundary:** Given eine Position, bei der die Tür sonst über ein
  Wandende hinausragen würde, then wird die Position so begrenzt, dass
  die Tür vollständig in der Wand liegt; passt selbst die schmalste Tür
  nicht in die Wand, then wird die Platzierung abgelehnt (kein
  Durchbruch außerhalb der Wand).
- **Negative:** Given eine Platzierung ohne Wirtswand, then wird sie
  abgelehnt; es entsteht keine verwaiste Tür.

#### LH-FA-DOR-002 — Tür verschieben

**Akzeptanzkriterien:**

- **Happy Path:** Given eine platzierte Tür, when sie entlang ihrer
  Wand verschoben wird, then folgt der Wand-Durchbruch der neuen
  Position (alte Stelle wieder geschlossen), ohne Benutzer-Refresh.
- **Boundary:** Verschieben über ein Wandende hinaus wird auf den
  gültigen Bereich begrenzt (Verhalten wie LH-FA-DOR-001 Boundary).
- **Negative:** Eine abgelehnte/ungültige Verschiebung lässt die Tür an
  ihrer bisherigen Position; die Darstellung bleibt unverändert.

#### LH-FA-DOR-003 — Türparameter bearbeiten

**Beschreibung:** Breite, Höhe und **Anschlag** (Öffnungsseite/-richtung)
sind parametrisch. Breite **600 mm bis 2000 mm**, Höhe **1800 mm bis
2500 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Tür, when Breite/Höhe geändert werden, then
  passt sich der Wand-Durchbruch sofort an; when der Anschlag geändert
  wird, then ist die geänderte Öffnungsseite benutzer-beobachtbar.
- **Boundary:** Given Breite oder Höhe am Grenzwert, then akzeptiert;
  außerhalb des Bereichs, then auf den nächsten Grenzwert geklemmt +
  Hinweis.

#### LH-FA-DOR-004 — Wandöffnung automatisch erzeugen

**Beschreibung:** Eine Tür erzeugt **automatisch** eine Öffnung in ihrer
Wirtswand — ohne separaten Benutzer-Schritt (vgl. LH-FA-WIN-005).

**Akzeptanzkriterien:**

- **Happy Path:** Given eine an einer Wand platzierte Tür, then ist die
  Wand an der Tür-Position über die Türbreite und von der Standfläche
  bis zur Türhöhe durchbrochen, in 2D- und 3D-Darstellung beobachtbar;
  das verbleibende Wandvolumen ist um das Öffnungsvolumen verringert.
- **Boundary:** Given eine Tür höher als die Wand, then reicht der
  Durchbruch höchstens bis zur Wandhöhe (kein Durchbruch über die Wand
  hinaus).
- **Negative:** Given eine entfernte Tür, then ist die Wand an der
  vormaligen Stelle wieder geschlossen.

### Modul Fenster (`WIN`)

Fenster sind **wand-gehostete Bauteile** wie Türen (LH-FA-DOR-*),
zusätzlich mit einer Brüstungshöhe. Geschärft 2026-06-13 (slice-013a).

#### LH-FA-WIN-001 — Fenster platzieren

**Akzeptanzkriterien:** wie LH-FA-DOR-001 (Happy/Boundary/Negative),
für ein Fenster — eine platzierte Fenster-Öffnung sitzt in der Wand und
bricht sie durch (vgl. LH-FA-WIN-005); ohne Wirtswand abgelehnt.

#### LH-FA-WIN-002 — Fenster verschieben

**Akzeptanzkriterien:** wie LH-FA-DOR-002, für ein Fenster.

#### LH-FA-WIN-003 — Fensterparameter bearbeiten

**Beschreibung:** Breite und Höhe parametrisch. Breite **300 mm bis
3000 mm**, Höhe **300 mm bis 2500 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Fenster, when Breite/Höhe geändert werden,
  then passt sich die Wandöffnung sofort an.
- **Boundary:** Grenzwert-Verhalten wie LH-FA-DOR-003 (akzeptiert am
  Grenzwert; außerhalb geklemmt + Hinweis).

#### LH-FA-WIN-004 — Brüstungshöhe definieren

**Beschreibung:** Die Brüstungshöhe ist der Abstand der Fenster-Unterkante
über der Geschoss-Standfläche, parametrisch **0 mm bis 2000 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Fenster, when die Brüstungshöhe gesetzt wird,
  then beginnt die Wandöffnung erst oberhalb der Brüstung — die Wand
  bleibt unterhalb des Fensters geschlossen (beobachtbar in 3D).
- **Boundary:** Given Brüstung + Fensterhöhe größer als die Wandhöhe,
  then reicht die Öffnung höchstens bis zur Wandhöhe; given Brüstung am
  Grenzwert, then akzeptiert, außerhalb geklemmt + Hinweis.

#### LH-FA-WIN-005 — Wandöffnung automatisch erzeugen

**Akzeptanzkriterien:** wie LH-FA-DOR-004, für ein Fenster — die
Öffnung wird automatisch erzeugt, beginnt oberhalb der Brüstung
(LH-FA-WIN-004) und reicht über die Fensterhöhe; bei entferntem Fenster
ist die Wand wieder geschlossen.

### Modul Treppen (`STR`)

Geschärft 2026-06-14 (slice-016a) von Outline auf AK-Niveau
(Reifephase-Klausel). **Teilumfang welle-2: gerade einläufige Treppe** —
eine gerade Treppe, die zwei Geschosse verbindet. Podest-, U-/L-förmige
und Wendeltreppen bleiben ausdrücklich **offen** (späterer Vollumfang).
b-cad ist **keine Statik** (§6): Stufen-/Breiten-Bereiche sind
parametrische Komfort-Vorgaben, keine erzwungene Baurecht-Prüfung.

#### LH-FA-STR-001 — Treppe erzeugen

**Beschreibung:** Eine gerade Treppe verbindet ein unteres mit einem
oberen Geschoss; sie steigt vom Startpunkt als Stufenfolge auf.

**Akzeptanzkriterien:**

- **Happy Path:** Given zwei übereinanderliegende Geschosse, when eine
  gerade Treppe an einer Position erzeugt wird, then zeigt die
  3D-Darstellung eine Stufenfolge, die von der unteren zur oberen
  Geschossebene aufsteigt.
- **Boundary:** Stufenanzahl/Laufbreite am Grenzwert akzeptiert (Bereiche
  siehe LH-FA-STR-002/003).
- **Negative:** Given keine gültige Zwei-Geschoss-Spanne (nur ein
  Geschoss, oder Start- = Zielgeschoss), then entsteht keine Treppe; kein
  Fehler/Absturz.

#### LH-FA-STR-002 — Stufenanzahl definieren

**Beschreibung:** Die Stufenanzahl ist parametrisch im Bereich
**2 bis 30**.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Treppe, when die Stufenanzahl geändert wird,
  then passt sich die Zahl der sichtbaren Stufen an; die Treppe verbindet
  weiterhin beide Geschossebenen.
- **Boundary:** Stufenanzahl am Grenzwert akzeptiert; außerhalb → auf den
  Grenzwert geklemmt + Hinweis.

#### LH-FA-STR-003 — Laufbreite definieren

**Beschreibung:** Die Laufbreite ist parametrisch im Bereich
**800 mm bis 2000 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Treppe, when die Laufbreite geändert wird,
  then wird die Treppe sichtbar breiter bzw. schmaler.
- **Boundary:** Laufbreite am Grenzwert akzeptiert; außerhalb → geklemmt +
  Hinweis.

#### LH-FA-STR-004 — Treppengeländer

**Beschreibung:** Eine gerade Treppe trägt ein Geländer (Handlauf) entlang
des Laufs. **Teilumfang welle-2:** das Geländer ist **immer sichtbar**;
eine An/Aus-Schaltung oder Seitenwahl bleibt offen.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Treppe, then ist entlang des Laufs ein
  Geländer auf Handlaufhöhe sichtbar (in der 3D-Darstellung), das der
  Stufenfolge folgt.
- **Negative:** Given eine entfernte Treppe, then verschwindet auch ihr
  Geländer (kein verwaistes Geländer).

### Modul Dach (`ROF`)

Geschärft 2026-06-13 (slice-014a) von Outline auf AK-Niveau
(Reifephase-Klausel). **Teilumfang welle-2: rechteckiger
Dach-Grundriss** — Sattel-, Walm- und Pultdach über einem rechteckigen
Grundriss. Komplexe (L-/U-förmige) Polygon-Grundrisse bleiben
ausdrücklich **offen** (späterer Vollumfang).

#### LH-FA-ROF-001 — Satteldach

**Beschreibung:** Ein Satteldach über dem rechteckigen Grundriss hat
zwei zueinander geneigte Dachflächen, die sich an einem First treffen.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein rechteckiger Grundriss, when ein Satteldach
  mit Neigung und Überstand erzeugt wird, then zeigt die 3D-Darstellung
  zwei geneigte Flächen, die sich an einem First über der Mitte treffen;
  das Dach kragt um den Überstand über den Grundriss hinaus.
- **Boundary:** Neigung/Überstand am Grenzwert akzeptiert (Bereiche
  siehe LH-FA-ROF-004/005).
- **Negative:** Given ein nicht-rechteckiger oder degenerierter
  Grundriss, then entsteht kein Dach; kein Fehler/Absturz.

#### LH-FA-ROF-002 — Walmdach

**Beschreibung:** Ein Walmdach ist an allen vier Seiten geneigt; der
First ist kürzer als der Grundriss (an den Giebelseiten abgewalmt).

**Akzeptanzkriterien:**

- **Happy Path:** Given ein rechteckiger Grundriss, when ein Walmdach
  erzeugt wird, then sind alle vier Dachflächen geneigt und der First
  ist kürzer als die längere Grundriss-Seite (beobachtbar in 3D).
- **Boundary/Negative:** wie LH-FA-ROF-001.

#### LH-FA-ROF-003 — Pultdach

**Beschreibung:** Ein Pultdach hat eine einzige, zu einer Seite geneigte
Dachfläche.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein rechteckiger Grundriss, when ein Pultdach
  erzeugt wird, then zeigt die Darstellung eine einzige geneigte Fläche
  (von der hohen Traufe zur niedrigen).
- **Boundary/Negative:** wie LH-FA-ROF-001.

#### LH-FA-ROF-004 — Dachneigung definieren

**Beschreibung:** Die Dachneigung ist parametrisch im Bereich
**5° bis 60°**.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Dach, when die Neigung erhöht wird, then
  steigt der First sichtbar (steileres Dach).
- **Boundary:** Given Neigung am Grenzwert (5°/60°), then akzeptiert;
  außerhalb → auf den Grenzwert geklemmt + Hinweis.

#### LH-FA-ROF-005 — Dachüberstand definieren

**Beschreibung:** Der Dachüberstand (Auskragung über den Grundriss) ist
parametrisch im Bereich **0 mm bis 1500 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Dach, when der Überstand gesetzt wird, then
  kragt die Dachkante um diesen Betrag über den Grundriss hinaus.
- **Boundary:** Given Überstand am Grenzwert, then akzeptiert; außerhalb
  → geklemmt + Hinweis.

### Modul Decken (`SLB`)

Decken sind **horizontale Platten** über einem Grundriss. Geschärft
2026-06-13 (slice-015a) von Outline auf AK-Niveau (Reifephase-Klausel).

#### LH-FA-SLB-001 — Decke erzeugen

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Grundriss-Umriss, when eine Decke erzeugt
  wird, then erscheint eine horizontale Platte über dem Umriss in der
  3D-Darstellung.
- **Negative:** Given ein degenerierter/leerer Grundriss, then entsteht
  keine Decke; kein Fehler/Absturz.

#### LH-FA-SLB-002 — Deckendicke definieren

**Beschreibung:** Die Deckendicke ist parametrisch im Bereich
**100 mm bis 500 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Decke, when die Dicke geändert wird, then
  passt sich die Platten-Stärke sichtbar an.
- **Boundary:** Dicke am Grenzwert akzeptiert; außerhalb → geklemmt +
  Hinweis.

#### LH-FA-SLB-003 — Deckenausschnitte

**Beschreibung:** Eine Decke kann eine Aussparung erhalten (z. B. für
eine Treppe oder einen Schacht).

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Decke, when ein Ausschnitt gesetzt wird,
  then ist die Decke an dieser Stelle durchbrochen (als Loch sichtbar);
  das verbleibende Decken-Volumen ist um das Ausschnitt-Volumen
  verringert.
- **Boundary:** Ein Ausschnitt, der über den Decken-Umriss hinausragt,
  wird auf den Umriss begrenzt (kein Loch außerhalb der Platte).

### Modul Fundament (`FND`)

Fundament und Bodenplatte sind **horizontale Platten** am
Gebäude-Aufstand. Geschärft 2026-06-13 (slice-015a).

#### LH-FA-FND-001 — Fundament erzeugen

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Grundriss-Umriss, when ein Fundament erzeugt
  wird, then erscheint eine Platte am Aufstand des Gebäudes (in 3D
  sichtbar, unterhalb des untersten Geschosses).
- **Negative:** wie LH-FA-SLB-001 (degeneriert → kein Fundament).

#### LH-FA-FND-002 — Fundamenttiefe definieren

**Beschreibung:** Die Fundamenttiefe (Stärke der Fundament-Platte) ist
parametrisch im Bereich **200 mm bis 2000 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Fundament, when die Tiefe geändert wird,
  then passt sich die Platten-Stärke sichtbar an.
- **Boundary:** am Grenzwert akzeptiert; außerhalb → geklemmt + Hinweis.

#### LH-FA-FND-003 — Bodenplatte erzeugen

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Grundriss-Umriss, when eine Bodenplatte
  erzeugt wird, then erscheint eine Fundament-Platte auf Geländehöhe
  (Oberkante auf Höhe 0).
- **Negative:** wie LH-FA-SLB-001.

### Modul 3D-Modellierung (`D3`)

- **LH-FA-D3-001 — Automatische Extrusion** (2D-Elemente → 3D-Körper).

#### LH-FA-D3-002 — Echtzeitaktualisierung

**Beschreibung:** Änderungen am Gebäudemodell (Bauteil anlegen,
Parameter ändern) sind **sofort** in der 3D-Darstellung sichtbar —
ohne expliziten Aktualisierungs-Schritt des Benutzers.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Gebäude mit 3D-Darstellung und eine Wand,
  when der Benutzer die Wandstärke ändert, then zeigt die
  3D-Darstellung den geänderten Wandkörper, ohne dass der Benutzer
  einen Aktualisierungs-/Refresh-Schritt ausführt.
- **Boundary:** Given eine Parameteränderung, die auf einen Grenzwert
  geklemmt wird (vgl. LH-FA-WAL-002), when die Klemmung greift, then
  zeigt die 3D-Darstellung den geklemmten — tatsächlich übernommenen —
  Stand.
- **Negative:** Given eine abgelehnte Parameteränderung (ungültige
  Eingabe, vgl. [`E-VAL-001`](spezifikation.md#4-fehler-codes-und-logging-felder)), when die Ablehnung erfolgt, then ändert
  sich die 3D-Darstellung nicht.

**Out-of-Scope (LH-FA-D3-002):** Latenz-/Performance-Budget (bei
Bedarf eigene `LH-QA`-Anforderung); gleichzeitige Aktualisierung
mehrerer unabhängiger Ansichten (LH-FA-UI-004).

- **LH-FA-D3-003 — Perspektivansicht.**
- **LH-FA-D3-004 — Orthogonale Ansichten.**
- **LH-FA-D3-005 — Schnittebenen.**
- **LH-FA-D3-006 — Explosionsansicht.**

### Modul Zeichnungsfunktionen (`DRW`)

- **LH-FA-DRW-001 — Fangpunkte.**
- **LH-FA-DRW-002 — Raster.**
- **LH-FA-DRW-003 — Winkelvorgaben.**
- **LH-FA-DRW-004 — Bemaßung.**
- **LH-FA-DRW-005 — Hilfslinien.**
- **LH-FA-DRW-006 — Layer.**
- **LH-FA-DRW-007 — Gruppen.**

### Modul Materialsystem (`MAT`)

Geschärft 2026-06-14 (slice-017a) von Outline auf AK-Niveau (Reifephase-
Klausel). Material ist eine **Eigenschaft**, die Bauteilen zugewiesen wird und
in die Auswertungen (EVL) einfließt — **keine** Geometrie. **Teilumfang:**
MAT-004 (Texturen) ist darstellungs-nah und bleibt **offen** (gehört zur Sicht,
nicht zu M3).

#### LH-FA-MAT-001 — Materialien verwalten

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Projekt, when ein Material mit Name und Kategorie
  angelegt (bzw. geändert/entfernt) wird, then steht es zur Zuweisung bereit
  (bzw. ist geändert/entfernt).
- **Negative:** Given ein Material ohne Name, then wird es abgelehnt.

#### LH-FA-MAT-002 — Materialbibliothek

**Akzeptanzkriterien:**

- **Happy Path:** Given das Projekt, when die Materialbibliothek geöffnet wird,
  then zeigt sie die verfügbaren Materialien (Name, Kategorie), aus denen
  ausgewählt werden kann.

#### LH-FA-MAT-003 — Materialzuweisung

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Bauteil (Wand/Dach/Decke) und ein Material, when das
  Material dem Bauteil zugewiesen wird, then ist das Bauteil dem Material
  zugeordnet und seine Kennwerte (U-Wert/Kosten) fließen in die Auswertungen ein.
- **Negative:** Given ein Bauteil ohne zugewiesenes Material, then gilt „kein
  Material"; es entsteht kein Fehler.

#### LH-FA-MAT-005 — U-Wert

**Beschreibung:** Ein Material trägt einen **U-Wert** (Wärmedurchgangs-
koeffizient, bauphysikalischer Kennwert).

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Material, when sein U-Wert gesetzt wird, then ist er
  am Material hinterlegt und über die zugewiesenen Bauteile abrufbar.

#### LH-FA-MAT-006 — Kostenkennwerte

**Beschreibung:** Ein Material trägt **Kostenkennwerte** (Kosten je m² bzw.
je m³).

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Material mit Kostenkennwerten, when eine
  Kosten-Auswertung erfolgt, then fließen die Kennwerte je zugewiesenem Bauteil
  (Fläche/Volumen) in die Summe ein.

### Modul Auswertungen (`EVL`)

Geschärft 2026-06-14 (slice-017a) von Outline auf AK-Niveau (Reifephase-
Klausel). Auswertungen sind eine **reine Ableitung aus dem committeten Modell**
(read-only) — sie zeigen Werte, sie verändern das Modell nicht.

#### LH-FA-EVL-001 — Flächenberechnung

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Geschoss mit erkannten Räumen, when die Fläche
  ausgewertet wird, then zeigt die Auswertung die **Netto-Grundfläche** je Raum
  (und die Summe je Geschoss) in m².
- **Boundary:** Given ein Geschoss ohne geschlossene Räume, then ist die
  ausgewiesene Fläche 0 (kein Fehler).

#### LH-FA-EVL-002 — Volumenberechnung

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Gebäude mit Bauteilen, when das Volumen ausgewertet
  wird, then zeigt die Auswertung das **Netto-Volumen** (Bauteil-Volumen
  abzüglich der Öffnungen) in m³.
- **Boundary:** leeres Modell → Volumen 0.

#### LH-FA-EVL-003 — Wohnflächenberechnung

**Beschreibung:** Die Wohnfläche wird aus den Raum-Netto-Flächen abgeleitet.
**Teilumfang welle-3:** Wohnfläche = Summe der Netto-Grundflächen;
**Anrechnungsfaktoren** (Dachschrägen, Balkone, …) bleiben **offen**.

**Akzeptanzkriterien:**

- **Happy Path:** Given die erkannten Räume, when die Wohnfläche ausgewertet
  wird, then zeigt sie die Summe der Raum-Netto-Flächen in m².

#### LH-FA-EVL-004 — Materiallisten

**Akzeptanzkriterien:**

- **Happy Path:** Given Bauteile mit zugewiesenem Material, when die
  Materialliste ausgewertet wird, then zeigt sie je Material die **Menge**
  (Fläche bzw. Volumen) der zugeordneten Bauteile.
- **Boundary:** Bauteile ohne Material werden nicht material-gruppiert; kein
  Fehler.

#### LH-FA-EVL-005 — Türlisten

**Akzeptanzkriterien:**

- **Happy Path:** Given platzierte Türen, when die Türliste ausgewertet wird,
  then zeigt sie die Türen mit Anzahl und Maßen (Breite/Höhe).

#### LH-FA-EVL-006 — Fensterlisten

**Akzeptanzkriterien:**

- **Happy Path:** Given platzierte Fenster, when die Fensterliste ausgewertet
  wird, then zeigt sie die Fenster mit Anzahl und Maßen (Breite/Höhe/Brüstung).

### Modul Import / Export (`IO`)

#### LH-FA-IO-001 — IFC-Import

**Beschreibung:** Import eines IFC-Modells in das b-cad-Gebäudemodell.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine valide IFC-Datei mit Wänden/Geschossen,
  when importiert, then entsprechende b-cad-Bauteile entstehen; Anzahl
  Geschosse und Wände stimmt mit der Quelle überein.
- **Negative:** Given eine nicht-IFC-Datei, when importiert, then
  Fehler-Code [`E-IO-003`](spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Import.

- **LH-FA-IO-002 — IFC-Export.**
- **LH-FA-IO-003 — DXF-Import.**
- **LH-FA-IO-004 — DXF-Export.**
- **LH-FA-IO-005 — STEP-Export.**
- **LH-FA-IO-006 — STL-Export.**
- **LH-FA-IO-007 — PDF-Export** (maßstäblicher Plan, vgl. ACC-004).
- **LH-FA-IO-008 — PNG-Export.**

### Modul Benutzeroberfläche (`UI`)

- **LH-FA-UI-001 — Docking-Fenster.**
- **LH-FA-UI-002 — Dunkles Theme.**
- **LH-FA-UI-003 — Helles Theme.**
- **LH-FA-UI-004 — Mehrmonitorbetrieb.**
- **LH-FA-UI-005 — Anpassbare Werkzeugleisten.**

### Modul Plugin-System (`PLG`)

- **LH-FA-PLG-001 — Dynamische Plugins** (Laden/Entladen zur Laufzeit).
- **LH-FA-PLG-002 — Plugin-API** (stabiler Vertrag, vgl. ADR-Folge).
- **LH-FA-PLG-003 — Plugin-Lifecycle.**
- **LH-FA-PLG-004 — Plugin-Sandbox** (Plugin darf das Modell nicht
  korruptieren).

## 5. Nichtfunktionale Anforderungen

### LH-QA-001 — Performance Projektöffnung

- **Anforderung:** Projektöffnung eines Standardprojekts < 3 Sekunden.
- **Messmethode:** Zeitmessung über das Referenz-Projekt aus ACC-001;
  Messprotokoll in [`spezifikation.md`](spezifikation.md).

### LH-QA-002 — Speicherverbrauch

- **Anforderung:** < 2 GB RAM bei Standardprojekten.
- **Messmethode:** Resident-Set-Size-Messung auf dem Referenzprojekt.

### LH-QA-003 — Undo/Redo

- **Anforderung:** mindestens 1000 Schritte.
- **Messmethode:** Test, der 1000 Operationen ausführt und vollständig
  zurücknimmt; Modell danach bit-identisch zum Ausgangszustand.

### LH-QA-004 — Autosave

- **Anforderung:** automatische Sicherung alle 300 Sekunden.
- **Messmethode:** Test mit simulierter Zeit; Autosave-Artefakt nach
  Intervall vorhanden.

### LH-QA-005 — Crash-Recovery

- **Anforderung:** Wiederherstellung nach Absturz ohne Verlust des
  letzten konsistenten Stands.
- **Messmethode:** simulierter Absturz (`kill -9`) zwischen
  Schreibphasen; nach Neustart letzter konsistenter Stand ladbar
  (vgl. LH-FA-BLD-002 Boundary).

### LH-QA-006 — Mehrsprachigkeit

- **Anforderung:** Deutsch und Englisch.
- **Messmethode:** UI-Strings vollständig aus Ressourcen; keine
  hartkodierten anzeigbaren Strings (Linter-Regel, geplant).

## 6. Globale Out-of-Scope-Punkte

- Mehrbenutzer-/Kollaborations-Betrieb am selben Projekt gleichzeitig.
- Statik-/Tragwerksberechnung (Tragwand ist Klassifikation, keine
  Bemessung).
- Cloud-Speicherung und Web-Client.
- Gebäudetechnik-Fachplanung (HLS, Elektro) über die Geometrie hinaus.
- Bauantrags-/Genehmigungs-Workflow.

## 7. Abnahmekriterien

| ID | Kriterium | Bezug |
|---|---|---|
| ACC-001 | Ein Einfamilienhaus mit ≥ 2 Geschossen, 10 Räumen, 15 Fenstern, 12 Türen und 1 Dach kann vollständig erstellt werden. | BLD, FLR, WAL, ROM, DOR, WIN, ROF |
| ACC-002 | Das Gebäude wird automatisch als 3D-Modell dargestellt. | LH-FA-D3-001, LH-FA-D3-002 |
| ACC-003 | Export nach IFC erfolgreich. | LH-FA-IO-002 |
| ACC-004 | Export eines maßstäblichen PDF-Plans erfolgreich. | LH-FA-IO-007 |
| ACC-005 | Projekt kann gespeichert und wieder geladen werden. | LH-FA-BLD-002, LH-FA-BLD-003 |

## 8. Glossar

| Begriff | Bedeutung im Lastenheft |
|---|---|
| Gebäudemodell | durchgängiges parametrisches Datenmodell, Quelle für 2D und 3D. |
| Wandzug | zusammenhängende Folge von Wandsegmenten. |
| Raum | aus geschlossenem Wandzug abgeleitetes Flächen-/Volumen-Element. |
| Standardprojekt | Referenzprojekt gemäß ACC-001 (Messbasis für LH-QA-001/002). |
| Bauteil | parametrisches Modell-Element (Wand, Tür, Fenster, Decke, …). |

## 9. Historie

Ausgelagert nach [`lastenheft-historie.md`](lastenheft-historie.md)
(slice-018a / [MR-011](../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths) — Provenance-Datei außerhalb der `matrix`-Spec-Straten).
