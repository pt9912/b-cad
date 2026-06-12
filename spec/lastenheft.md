# Lastenheft — b-cad

**Version:** 0.1.2
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
  Projekt anlegen, then Fehler-Code `E-IO-001`, kein leerer
  Projektzustand mit verlorenem Vorgänger.

**Out-of-Scope:** Projektvorlagen-Galerie (spätere Welle).

#### LH-FA-BLD-002 — Projekt speichern

**Beschreibung:** Das Projekt wird **persistent und atomar** in eine
Projektdatei (SQLite, siehe ADR-0003) gespeichert.

**Akzeptanzkriterien:**

- **Happy Path:** Given geändertes Projekt, when „Speichern", then
  Projektdatei enthält den vollständigen Modellstand; erneutes Laden
  ergibt ein identisches Modell.
- **Boundary:** Given Absturz während des Schreibens, when Neustart,
  then der letzte konsistente Stand ist intakt (kein halb geschriebenes
  Projekt — vgl. LH-QA-005).
- **Negative:** Given Zielmedium voll, when „Speichern", then
  Fehler-Code `E-IO-002`, vorheriger Dateistand unverändert.

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
  Punkt gesetzt, then Fehler-Code `E-GEO-001`.

#### LH-FA-WAL-002 — Wandstärke definieren

**Beschreibung:** Wandstärke ist parametrisch im Bereich **50 mm bis
1000 mm**.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Wand, when Stärke = 240 mm, then Geometrie
  und 3D-Körper aktualisieren sich sofort (vgl. LH-FA-D3-002).
- **Boundary:** Given Stärke = 50 mm oder 1000 mm, then akzeptiert;
  Given 49 mm oder 1001 mm, then auf Grenzwert geklemmt + Hinweis,
  Fehler-Code `E-VAL-001`.

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

- **LH-FA-DOR-001 — Tür platzieren.**
- **LH-FA-DOR-002 — Tür verschieben.**
- **LH-FA-DOR-003 — Türparameter bearbeiten** (Breite, Höhe, Anschlag).
- **LH-FA-DOR-004 — Wandöffnung automatisch erzeugen** (Tür schneidet
  Wand, vgl. LH-FA-WIN-005).

### Modul Fenster (`WIN`)

- **LH-FA-WIN-001 — Fenster platzieren.**
- **LH-FA-WIN-002 — Fenster verschieben.**
- **LH-FA-WIN-003 — Fensterparameter bearbeiten.**
- **LH-FA-WIN-004 — Brüstungshöhe definieren.**
- **LH-FA-WIN-005 — Wandöffnung automatisch erzeugen.**

### Modul Treppen (`STR`)

- **LH-FA-STR-001 — Treppe erzeugen.**
- **LH-FA-STR-002 — Stufenanzahl definieren.**
- **LH-FA-STR-003 — Laufbreite definieren.**
- **LH-FA-STR-004 — Treppengeländer erzeugen.**

### Modul Dach (`ROF`)

- **LH-FA-ROF-001 — Satteldach.**
- **LH-FA-ROF-002 — Walmdach.**
- **LH-FA-ROF-003 — Pultdach.**
- **LH-FA-ROF-004 — Dachneigung definieren.**
- **LH-FA-ROF-005 — Dachüberstand definieren.**

### Modul Decken (`SLB`)

- **LH-FA-SLB-001 — Decke erzeugen.**
- **LH-FA-SLB-002 — Deckendicke definieren.**
- **LH-FA-SLB-003 — Deckenausschnitte.**

### Modul Fundament (`FND`)

- **LH-FA-FND-001 — Fundament erzeugen.**
- **LH-FA-FND-002 — Fundamenttiefe definieren.**
- **LH-FA-FND-003 — Bodenplatte erzeugen.**

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
  Eingabe, vgl. `E-VAL-001`), when die Ablehnung erfolgt, then ändert
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

- **LH-FA-MAT-001 — Materialien verwalten.**
- **LH-FA-MAT-002 — Materialbibliothek.**
- **LH-FA-MAT-003 — Materialzuweisung.**
- **LH-FA-MAT-004 — Texturen.**
- **LH-FA-MAT-005 — U-Wert** (bauphysikalischer Kennwert).
- **LH-FA-MAT-006 — Kostenkennwerte.**

### Modul Auswertungen (`EVL`)

- **LH-FA-EVL-001 — Flächenberechnung.**
- **LH-FA-EVL-002 — Volumenberechnung.**
- **LH-FA-EVL-003 — Wohnflächenberechnung.**
- **LH-FA-EVL-004 — Materiallisten.**
- **LH-FA-EVL-005 — Türlisten.**
- **LH-FA-EVL-006 — Fensterlisten.**

### Modul Import / Export (`IO`)

#### LH-FA-IO-001 — IFC-Import

**Beschreibung:** Import eines IFC-Modells in das b-cad-Gebäudemodell.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine valide IFC-Datei mit Wänden/Geschossen,
  when importiert, then entsprechende b-cad-Bauteile entstehen; Anzahl
  Geschosse und Wände stimmt mit der Quelle überein.
- **Negative:** Given eine nicht-IFC-Datei, when importiert, then
  Fehler-Code `E-IO-003`, kein Teil-Import.

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

| Version | Datum | Änderung | Verweis |
|---|---|---|---|
| 0.1.0 | 2026-06-08 | Initiale Outline-Fassung aus Domänen-Vorlage; ID-Schema `LH-FA-<BEREICH>-<NNN>` etabliert | Greenfield-Bootstrap (Kurs-Modul 2) |
| 0.1.1 | 2026-06-11 | LH-FA-D3-002 von Outline auf AK-Niveau geschärft (Reifephase-Klausel §1/§4); lösungsfreie, benutzer-beobachtbare Formulierung; irreführender Querverweis „vgl. LH-QA-001" entfernt | slice-010a |
| 0.1.2 | 2026-06-12 | LH-FA-WAL-006 Teilumfang „Eckenschluss endpunkt-verbundener Wände" von Outline auf AK-Niveau geschärft (Vollumfang bleibt offen); Auslöser: ACC-002-Abnahme-Befund | slice-012 |
