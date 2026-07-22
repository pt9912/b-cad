---
id: slice-040a
titel: CR 002 — GLB-/CSV-Import: Annahmebaseline und Lastenheft-Übernahme
status: open
welle: unzugeordnet (Akustik-/Import-Folgewelle; Annahmeentscheidung offen)
lastenheft_refs: []  # Anforderungen entstehen erst nach fachlicher Annahme und Live-ID-Audit.
adr_refs: []         # Technische Entscheidungen folgen in einem eigenen Spec-/ADR-Slice.
---

# Slice 040a: CR 002 — GLB-/CSV-Import und Acoustic Bridge

**Status:** open — fachlich reviewt und annahmereif, aber noch nicht durch den
Projektinhaber angenommen oder priorisiert. Die Annahme ist ein expliziter
Aktivierungstrigger und wird nicht aus der Plananlage abgeleitet.

**Welle:** unzugeordnet. Der Plan begründet eine spätere Akustik-/Importwelle,
verdrängt aber weder die aktive Welle noch die offenen
[`slice-039a`](slice-039a-akustik-lastenheft-uebernahme.md) und
[`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md).

**Annahmegrundlage:** Der am 2026-07-22 fachlich ohne offene HIGH-/MEDIUM-
Findings geprüfte CR 002 wird in Anhang A vollständig und selbsttragend
bewahrt. Sein vorausgegangener Arbeitsentwurf liegt nur temporär außerhalb des
Repositories; er ist weder verlinkbare Quelle noch dauerhafte Abhängigkeit.

**Schnitt:** Dieser Slice übernimmt nach ausdrücklicher Annahme ausschließlich
den Produktvertrag in das Lastenheft: Ziele, lösungsfreie Anforderungen,
Qualitätsanforderungen, Begriffe, Nichtumfang und die additiven ID-Bereiche.
Parser-, Feld-, Datenmodell-, Port-, Budget- und Persistenzmechanik wird nur
inventarisiert und in einen eigenen CR-002-Spec-/ADR-Folgeslice geroutet. Kein
Produktionscode und kein gemeinsamer b-cad–a-ray-Exportvertrag entstehen hier.

---

## 1. Ziel

Nach fachlicher Annahme den GLB-Oberflächenimport, den CSV-Import akustischer
Materialkennlinien und die manuelle Acoustic Bridge kollisionsfrei in den
abnahmebindenden b-cad-Produktvertrag übernehmen. Die Übernahme bleibt
unabhängig vom temporären CR-Ablageort und respektiert die CR-001-Reihenfolge:

1. [`slice-039a`](slice-039a-akustik-lastenheft-uebernahme.md) liefert die
   endgültigen ACO-/QA-/Ziel-IDs und den Produktvertrag.
2. [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md) entscheidet
   anschließend Spezifikation und ADRs, aber erzeugt keinen Produktionscode.
3. Die Acoustic Bridge startet erst nach implementierter CR-001-Identity-
   Foundation, implementiertem Akustikmaterialmodell und implementiertem
   Boundary-/Opening-/Portalmodell.

## 2. Selbsttragender Lastenheft-Oracle

Die folgenden 29 Funktionsverträge und acht Qualitätsverträge sind vollständig
zu übernehmen. Anhang A enthält ihre ausformulierten Akzeptanzkriterien; beim
normativen Schreiben dürfen sie nur redaktionell und zur Lösungsfreiheit
angepasst, nicht inhaltlich verkürzt werden.

### 2.1 GLB-Oberflächenimport (SMI)

| Nr. | Produktvertrag | Unverzichtbare Grenzen und Fehlerwirkung |
|---|---|---|
| SMI-01 | Lokale GLB-Datei als dauerhaftes nichtparametrisches Oberflächenmodell importieren. | Beschädigte oder nicht unterstützte Datei wird mit Ursache und ohne Teilbestand abgelehnt; verwendete Szene ist sichtbar. |
| SMI-02 | Initial ausschließlich GLB-Einzeldateien unterstützen. | `.gltf`, externe Ressourcen und Netzwerkbezüge liegen außerhalb des Umfangs und werden nicht geladen. |
| SMI-03 | Quellhierarchie und räumliche Anordnung nachvollziehbar erhalten. | Spiegelung/Skalierung diagnostizieren; gleicher bestätigter Import ergibt dieselbe Anordnung. |
| SMI-04 | Gültige dreiecksbasierte Oberflächen unabhängig von kompakter oder ausgeschriebener Quellrepräsentation übernehmen. | Nichtdreiecks- oder ungeeignete Inhalte werden nicht still übernommen; Umfang verworfener Inhalte wird berichtet. |
| SMI-05 | Wiederholte Quellgeometrie je räumlichem Vorkommen auswählbar und unterscheidbar halten. | Speichern/Laden und spätere Klassifikation bewahren die Vorkommensunterscheidung. |
| SMI-06 | Größe und Orientierung vor Importabschluss anzeigen und bestätigen. | Kein stiller Maßstabs-/Achsenfehler; unplausible Ausdehnung warnt sichtbar. |
| SMI-07 | Visuelle Quellmaterialien als Herkunfts-/Orientierungsmerkmale erhalten. | Keine automatische akustische Interpretation; fehlende Referenz bleibt sichtbarer Zustand. |
| SMI-08 | Flächenorientierung, Normalen und Doppelseitigkeit diagnostizieren. | Abgeleitete Normalen bleiben getrennte reproduzierbare Diagnosedaten; deaktivierte Ableitung blockiert betroffene akustische Klassifikation, verändert aber die Quelle nicht; keine allgemeine Mesh-Reparatur. |
| SMI-09 | Importmodelle, Gruppen und räumliche Vorkommen in Baum und 3D auswählen, sperren und ein-/ausblenden. | Auswahl zeigt Herkunft, visuelles Material, geometrischen Umfang und Anordnung; große Modelle bleiben filterbar. |
| SMI-10 | Importmodell und fachliche Referenzen persistent speichern. | Roundtrip bewahrt Geometrie, Hierarchie und Kennungen; eingebettete Geometrie bleibt auch ohne spätere Quelldatei verfügbar. |
| SMI-11 | Geänderte GLB-Quelle mit Vorschau sicher neu einlesen. | Sichere Matches dürfen Zuordnungen behalten; Konflikte bleiben sichtbar und blockieren Exportprüfung; Bestätigung aktualisiert atomar, Abbruch/Fehler ändert nichts. |
| SMI-12 | Strukturierten, fachlich vergleichbaren Importbericht erzeugen. | Quelle, Version, Zeitpunkt, Umfang, Reparaturen/Verwerfungen sowie adressierbare Warnungen und Fehler bleiben nachvollziehbar. |
| SMI-13 | Importiertes Modell kontrolliert entfernen. | Abhängigkeiten werden vorab angezeigt; Abbruch oder vollständige Kaskade sind explizit, atomar und ohne verwaiste Referenzen; eigenständige Akustikmaterialien bleiben erhalten. |

### 2.2 CSV-Akustikmaterialimport (AMI)

| Nr. | Produktvertrag | Unverzichtbare Grenzen und Fehlerwirkung |
|---|---|---|
| AMI-01 | Lokale CSV-Datei unverändert als korrigierbare Vorschau anzeigen. | Tabellen-/Zahleninterpretation und fehlerhafte Datensätze sind vor Übernahme sichtbar; nicht unterstützte Dateien werden begründet abgelehnt. |
| AMI-02 | Spalten auf Materialname, Frequenzkennwerte, Streuung, Transmission und Quellenmetadaten abbilden. | Originalbezeichnungen bleiben sichtbar; Mehrdeutigkeit verlangt Benutzerentscheidung. |
| AMI-03 | Frei definierte passive Frequenzkennlinien importieren. | Positive eindeutige Frequenzen; kombinierte Werte erzeugen keine Energie; Streuung wirkt nur auf Reflexion; blockierender Fehler lässt gesamten Materialbestand unverändert. |
| AMI-04 | Einzelnen Streukoeffizienten sichtbar als konstante Kennlinie übernehmen. | Keine vorgetäuschte frequenzabhängige Messung. |
| AMI-05 | Allgemeine, Hersteller-, Literatur-, Schätz-, Kalibrier- und Projektgeltung unterscheiden. | Unklare Geltung muss bestätigt werden; projektspezifische Daten gelangen nicht automatisch in globale Bibliothek. |
| AMI-06 | Originale und optional strukturierte Quellen-/Bibliografieangaben erhalten. | Fehlende Quelle warnt; Quelldatensatz und -zeile bleiben nachvollziehbar. |
| AMI-07 | Namenskonflikte ausdrücklich durch Ersetzen, konfliktfreies Zusammenführen, Umbenennen oder Überspringen lösen. | Keine stille Überschreibung; jede Abweichung wird vorab aufgelöst; Fehler/Abbruch verändern den gesamten Bestand nicht. |
| AMI-08 | Wiederverwendbares, versioniertes CSV-Importprofil speichern. | Fehlende erwartete Inhalte werden vor Import gemeldet; tatsächlich verwendete Einstellungen bleiben protokolliert. |
| AMI-09 | Materialimport vollständig oder gar nicht übernehmen. | Blockierender Fehler oder Abbruch bewahrt den vorherigen Materialbestand. |
| AMI-10 | Strukturiertes Importprotokoll erzeugen. | Alle Konfliktoperationen, Warnungen, Fehler, Quellen und der atomare Ausgang bleiben nachvollziehbar. |

### 2.3 Manuelle Acoustic Bridge und Mapping (MAP)

| Nr. | Produktvertrag | Unverzichtbare Grenzen und Fehlerwirkung |
|---|---|---|
| MAP-01 | Importbereiche manuell als Raumseite, gemeinsame Grenzseite, Einbauteil oder leere Öffnung klassifizieren. | CR-001-Identitäten und -Entitäten werden wiederverwendet. Quellbereich, Host-Provenienz, Oberfläche und Grenze bleiben getrennt. Gemeinsame Grenzen werden nicht doppelt belegt. Leere Öffnung entsteht nur aus geschlossener gültiger Kontur/Polygon, eindeutiger Wirtsgrenze und angrenzenden Räumen; sie ist dieselbe persistente, bearbeit-/löschbare CR-001-Entität. Ungültige oder mehrdeutige Klassifikation ändert nichts. |
| MAP-02 | Visuelles Quellmaterial bestätigt auf bestehendes Akustikmaterial abbilden. | Betroffene Bereiche/Vorkommen werden vorab angezeigt; visuelles Material bleibt Herkunft; Zuordnung erzeugt allein keine Raumseite. |
| MAP-03 | Namenbasierte Zuordnungsvorschläge erzeugen. | Vorschlag ist nie Bestätigung; unscharfe oder mehrdeutige Treffer bleiben kenntlich und erfordern Entscheidung. |
| MAP-04 | Bestätigte Regeln als projektbezogenes Mappingprofil speichern. | Geltungsbereich/Priorität sichtbar; Konflikte verlangen Entscheidung; Anwendung bleibt bis Bestätigung Vorschau. |
| MAP-05 | Unklassifizierte und klassifizierte, aber unbelegte Flächen getrennt anzeigen. | Anzahl/Fläche und Viewer-Auswahl je Zustand; Pflichtlücken blockieren die akustische Exportprüfung. |
| MAP-06 | Klassifikationen und Materialzuordnungen nach Neuimport prüfen. | Nur sichere Matches bleiben; keine stille Übertragung; ungelöste Konflikte blockieren und folgen der atomaren Neuimportwirkung. |

### 2.4 Qualitätsverträge

Bei unverändertem ID-Bestand sind
[`OBJ-007`](../../../../spec/lastenheft.md) und
[`OBJ-008`](../../../../spec/lastenheft.md),
[`LH-FA-SMI-001`](../../../../spec/lastenheft.md) bis
[`LH-FA-SMI-013`](../../../../spec/lastenheft.md),
[`LH-FA-AMI-001`](../../../../spec/lastenheft.md) bis
[`LH-FA-AMI-010`](../../../../spec/lastenheft.md),
[`LH-FA-MAP-001`](../../../../spec/lastenheft.md) bis
[`LH-FA-MAP-006`](../../../../spec/lastenheft.md) sowie
[`LH-QA-012`](../../../../spec/lastenheft.md) bis
[`LH-QA-019`](../../../../spec/lastenheft.md) lediglich Kandidaten. Der
Live-Audit vergibt beim Schreibakt die dann freien endgültigen IDs.

| QA-Kandidat | Beobachtbarer Vertrag / Messung |
|---|---|
| [LH-QA-012](../../../../spec/lastenheft.md) — Determinismus | Identische Quelle, Importversion und Profil ergeben ein fachlich identisches Modell; Referenzimporte semantisch vergleichen. |
| [LH-QA-013](../../../../spec/lastenheft.md) — Provenienz | Modell und Materialdaten sind auf Datei, Prüfsumme, Importversion und Vorgang zurückführbar; Nachweis im gespeicherten Projekt und Bericht. |
| [LH-QA-014](../../../../spec/lastenheft.md) — keine stille Parametrisierung | Importierte Meshes bleiben nichtparametrisch; eine spätere Rekonstruktion ist ein eigener sichtbarer Vorgang. |
| [LH-QA-015](../../../../spec/lastenheft.md) — große Datei | Auf dem spezifizierten Referenzsystem: Vorschau für 1 Mio. Dreiecke/10.000 Szenenelemente/1.000 Vorkommen ≤ 10 s, Import ≤ 60 s, zusätzlicher Speicher ≤ 3 GiB; Fortschritt und Abbruch sichtbar. |
| [LH-QA-016](../../../../spec/lastenheft.md) — sichere Verarbeitung | Gültige Eingaben innerhalb der festgelegten Budgets werden verarbeitet; Überschreitungen, Überläufe und strukturelle Angriffe werden ohne Absturz oder Bestandsänderung abgelehnt. |
| [LH-QA-017](../../../../spec/lastenheft.md) — Regression | Bestehende IFC-, DXF- und Projektimporte bleiben unverändert und unabhängig von optionalen Konvertern. |
| [LH-QA-018](../../../../spec/lastenheft.md) — Plattform | Importfunktion arbeitet unter Linux ohne SketchUp oder proprietäres SketchUp-SDK. |
| [LH-QA-019](../../../../spec/lastenheft.md) — Rechte/Provenienz | Herkunft und Lizenzinformationen sind speicherbar; Besitz einer Datei wird nicht als Weitergaberecht ausgegeben. |

### 2.5 Vollständige fachliche Annahmematrix

Diese zwölf Zeilen sind die vollständige Annahmeentscheidung. Der eingebettete
Arbeitsstand enthält in §17 nur elf, nicht 1:1 zu dessen §16 passende
Protokollzeilen; dieser redaktionelle Mangel wird nicht in den aktiven Plan
übernommen. Bei Annahme wechseln alle zwölf Statuswerte atomar von `Offen` auf
`Angenommen`.

| Nr. | Entscheidung | Status |
|---:|---|---|
| 1 | GLB als dauerhaftes nichtparametrisches Oberflächenmodell importieren | Offen |
| 2 | `.gltf` mit externen Ressourcen aus dem initialen Umfang ausschließen | Offen |
| 3 | Importierte Meshes nicht automatisch in parametrische b-cad-Bauteile umwandeln | Offen |
| 4 | Direkten SKP-Import aus b-cad ausschließen | Offen |
| 5 | CSV als initiales Tabellenformat für akustische Kennwerte unterstützen | Offen |
| 6 | XLS/XLSX initial nur nach externer kontrollierter CSV-Konvertierung verwenden | Offen |
| 7 | Visuelle und akustische Materialien als getrennte Konzepte behandeln | Offen |
| 8 | Acoustic Bridge nur manuell und erst nach normativem `slice-039b` sowie implementierten CR-001-Identity-, Material- und Boundary-/Opening-/Portal-Slices einführen | Offen |
| 9 | Materialzuordnungen nur über eine bestätigte Mappingebene wirksam machen | Offen |
| 10 | Projektbezogene und kalibrierte Daten dauerhaft entsprechend kennzeichnen | Offen |
| 11 | GLB/CSV ausschließlich als b-cad-Eingaben behandeln und kein b-cad–a-ray-Exportformat voraussetzen oder festlegen | Offen |
| 12 | Öffentliche Regressionen nur mit synthetischen Fixtures; reale Bayreuth-Daten ausschließlich optional nach Lizenz-, Weitergabe- und Provenienzprüfung | Offen |

## 3. Definition of Done

- [ ] **Fachliche Annahme belegen:** Der Projektinhaber bestätigt alle zwölf
      Entscheidungen aus §2.5. Erst dann wechseln die Statuswerte atomar auf
      `Angenommen`, das Annahmedatum wird ergänzt und die normative Übernahme
      autorisiert.

- [ ] **CR-001-Voraussetzung:**
      [`slice-039a`](slice-039a-akustik-lastenheft-uebernahme.md) ist
      abgeschlossen. Seine endgültigen Ziel-, ACO- und QA-IDs ersetzen alle
      vorläufigen CR-002-Bezüge. [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md)
      muss für diesen reinen Lastenheft-Slice noch nicht abgeschlossen sein,
      bleibt aber Voraussetzung des technischen Folgeslice.

- [ ] **Live-ID-Audit:** Rollen, Ziele, Funktions- und Qualitäts-IDs gegen das
      dann aktuelle `spec/lastenheft.md`, alle offenen/aktiven Pläne und
      vorgeschlagene ADR-Folgepflichten prüfen. Kein Plan reserviert IDs.
      Kandidaten aus §2.4 werden nur übernommen, wenn sie beim atomaren
      Schreibakt konfliktfrei sind.

- [ ] **Lastenheft normativ übernehmen:** zwei Ziele, 13 SMI-, zehn AMI-, sechs
      MAP- und acht QA-Verträge aus §2 einschließlich Nichtumfang und
      b-cad–a-ray-Systemgrenze einfügen. GLB/CSV bleiben ausschließlich
      Eingabeformate; `.gltf`, SKP-/XLSX-Parser, automatische Parametrisierung,
      allgemeine Mesh-Reparatur, Simulation und Exportformatfestlegung bleiben
      ausgeschlossen.

- [ ] **Lösungsfreiheit:** Die Übernahme folgt
      [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei).
      GLB/CSV als Produktformate, beobachtbare Grenzwerte und atomare
      Fehlerwirkung bleiben zulässig; Chunk-/Accessor-/Feldnamen,
      Statusvokabular, Portsignaturen, Datenentitäten, Formeln und
      Persistenzmechanik verbleiben im technischen Routing aus §4.

- [ ] **Bereichskürzel additiv deklarieren:** `SMI`, `AMI` und `MAP` werden in
      `harness/conventions.md` durch einen neuen chronologisch nächsten
      MR-Eintrag ergänzt. Der bestehende
      [MR-002](../../../../harness/conventions.md#mr-002--id-schema-für-b-cad)
      bleibt unverändert. `.d-check.yml` wird nur bei reproduziertem Fallout
      angepasst.

- [ ] **Begriffe und bestehende Domänen:** Glossar ergänzt importiertes
      Oberflächenmodell, Quellbereich, räumliches Vorkommen, visuelles Material,
      Importprofil, Mappingprofil und Host-Provenienz. Akustikmaterial,
      Oberfläche, Grenze, leere Öffnung und Portal werden nicht neu oder
      widersprüchlich definiert, sondern aus CR 001 wiederverwendet.

- [ ] **Version und Historie:** Lastenheft-Header und jüngste erste Datenzeile
      in `spec/lastenheft-historie.md` erhalten dieselbe nächste freie Version
      gemäß
      [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie).

- [ ] **Technischen Folgeslice anlegen:** Ein selbsttragender offener
      CR-002-Spec-/ADR-Plan übernimmt das vollständige Routing aus §4. Er hängt
      normativ von `slice-039b` ab, dupliziert keine CR-001-Domänen und plant
      GLB-/CSV-Import, Mapping und Bridge in getrennten Implementierungsslices.

- [ ] **Semantische Evidenz:** Closure enthält eine Matrix der endgültigen IDs
      gegen alle 29+8 Zeilen aus §2. Ein unabhängiges Text-/Diff-Review führt
      eine vollständige 37-zeilige Pass/Fail-Checkliste ohne Stichproben und
      prüft zusätzlich Ziele, Glossar, Nichtumfang und Historie gegen §2, §4
      und Anhang A; HIGH-Findings blockieren.

- [ ] **Sensoren:** `make docs-check`, danach vollständig `make gates`.
      `make schema-check` ist nur nötig, falls entgegen dem Schnitt ein
      Datenmodell geändert wird; in diesem Fall ist zunächst der Slice-Scope zu
      korrigieren.

## 4. Technisches Übernahmeregister

| CR-Abschnitt | Zielstratum / Folgeverantwortung |
|---|---|
| 1–5 | Produktkontext, Ziele und vorläufige ID-Familien dieses Slice |
| 6–9 | lösungsfreie Übernahme gemäß §2; technische Details werden herausgelöst |
| 10.1 | CR-002-Spezifikation: GLB-Version, Primitive, Accessors, Transformationen, Normalen, Einbettung und atomare Grenzen |
| 10.2 | CR-002-Spezifikation: CSV-Dialekt, Feldmuster, Energiebilanz, leere Werte, Konflikte, Profilversionierung |
| 10.3 | nach normativem `slice-039b`: Importidentität/Host-Provenienz, manuelle Bridge und Wiederverwendung der implementierten CR-001-Identity-, Material- und Boundary-Domänen |
| 10.4 | CR-002-Spezifikation: Datei-, Element-, Bild-/Textur-, Geometrie-, Vorschau- und Gesamtbudgets sowie Performance-Messmethode |
| 11 | CR-002-ADRs und technische Spezifikation; Ports bleiben Vorschläge bis zur Entscheidung, keine glTF-/Qt-/Solvertypen im Kern |
| 12 | öffentliche synthetische Fixtures; realer Bayreuth-Fall nur optional nach Lizenz-/Provenienzfreigabe |
| 13–14 | Nichtumfang und Risiken in Lastenheft beziehungsweise Folgepläne übernehmen |
| 15 | Reihenfolge: Lastenheft → CR-002 Spec/ADR und CR-001-Gates → Inspect/Domain/Viewer/CSV/Mapping/Reimport/Bridge/Fixtures/optionale CLI |
| 16–17 | fachliche Annahme ausschließlich über die vollständige 12-zeilige Matrix in §2.5; der unvollständige Snapshot-Stand aus §17 wird nicht normativ übernommen |

## 5. Geplante Dateien

| Datei / Komponente | Behandlung |
|---|---|
| `spec/lastenheft.md` | Ziele, SMI-/AMI-/MAP-Module, QA, Produktgrenzen, Nichtumfang und Glossar |
| `spec/lastenheft-historie.md` | neue jüngste Provenienzzeile, versionsgleich zum Header |
| `harness/conventions.md` | neuer additiver MR-Eintrag für SMI/AMI/MAP |
| `.d-check.yml` | voraussichtlich unverändert; generische Bereichsregel verifizieren |
| `docs/plan/planning/open/` | ein benannter selbsttragender CR-002-Spec-/ADR-Folgeplan |
| `docs/reviews/2026-07-22-slice-040a-plan.md` | unabhängiger [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review |
| `docs/reviews/{Ausführungsdatum}-slice-040a-normative-diff.md` | unabhängiges Normativ-Diff-Review vor Closure |

## 6. Trigger, Lifecycle und Risiken

- **Annahmetrigger offen:** Plananlage ist keine Produktannahme. Ohne explizite
  Bestätigung bleiben CR, Slice und normative Quellen unverändert.
- **Einplanung:** Nach Annahme und Priorisierung reiner `git mv` von `open/`
  nach `next/`; Implementierungs-Start später als eigener reiner `git mv` nach
  `in-progress/`. Nur beim ersten aktiven Slice wird der Roadmap-Ruhemarker im
  selben Move-Commit entfernt.
- **CR-001-Reihenfolge:** CR 002 erweitert vorhandene Akustikdomänen. Er darf
  Identity, Material, Grenze, Öffnung oder Portal weder parallel spezifizieren
  noch implementieren.
- **Vertragsdrift:** GLB/CSV sind ausschließlich Eingaben. Kein CR-002-Text darf
  das gemeinsame Exportformat festlegen oder voraussetzen.
- **Verbindliche Scope-Entscheidung:** Die 29+8-Verträge werden wegen ihrer
  gemeinsamen ID-, Provenienz-, Atomaritäts- und Bridge-Bezüge in einem
  normativen Schreibakt übernommen. Voraussetzung ist ein unabhängiger
  Reviewlauf mit der vollständigen 37-zeiligen Pass/Fail-Matrix aus der DoD;
  Stichproben sind unzulässig. Steht diese Review-Kapazität vor Aktivierung
  nicht bereit, bleibt dieser Slice blockiert und muss vor jeder normativen
  Änderung durch selbsttragende SMI- und AMI/MAP-Teilslices ersetzt werden.
- **Rechte/Fixtures:** Reale Bayreuth-Daten bleiben außerhalb öffentlicher
  Artefakte, solange Lizenz, Weitergabe und Konverter-Provenienz nicht belegt
  sind. Öffentliche Gates verwenden synthetische Fixtures.
- **Ressourcenvertrag:** Konkrete Budgets sind Produktgrenzen, ihre technische
  Buchführung und Allokationsstrategie bleiben dem Spec-/ADR-Folgeslice
  vorbehalten.

## 7. Sub-Area-Modus

### Produktvertrag / Lastenheft

- **Modus:** GF für SMI/AMI/MAP, Brownfield für Ziele, QA, Glossar und
  bestehende Akustikbegriffe.
- **Konventionen-Dichte:** hoch — Source Precedence, CR-001-Abhängigkeiten,
  ID-Live-Audit,
  [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Lösungsfreiheit
  und
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Versionierung.
- **Evidenzrisiko:** hoch — großer externer Arbeitsentwurf, viele vorläufige IDs
  und eine manuelle Bridge in noch nicht implementierte CR-001-Domänen.
- **Reconciliation:** vollständiger 29+8-Diff, terminologische Einpassung und
  Trennung von Produktgrenze und Mechanik.

### Technisches Routing

- **Modus:** Brownfield für Hexagon/Persistenz/Viewer, GF für GLB-/CSV-Adapter
  und Import-Provenienz.
- **Konventionen-Dichte:** hoch — Kernreinheit, atomare Persistenz,
  Docker-only, keine parallelen Akustikdomänen und kein einseitiger
  Exportvertrag.
- **Reconciliation:** eigener Spec-/ADR-Slice nach CR-001-Normativik; getrennte
  Implementierungspläne und öffentliche synthetische Fixtures.

## 8. Anhang A — vollständig geprüfte CR-002-Baseline

Der folgende Snapshot bewahrt die am 2026-07-22 geprüfte Arbeitsfassung
vollständig innerhalb dieses Repo-Dokuments. Er benötigt weder einen Link noch
den Fortbestand des temporären Ablageorts und besitzt bis zur ausdrücklichen
Annahme keine normative Wirkung. Die äußere Vierer-Fence verhindert, dass
vorläufige IDs, technische Feldnamen oder Portvorschläge als aktive kanonische
Repo-Referenzen erscheinen. Nichtsemantische nachgestellte Leerzeichen werden
beim Einbetten entfernt.

````markdown
# Change Request 002 — Import externer Oberflächenmodelle und akustischer Materialtabellen

**Produkt:** b-cad
**Status:** Offen
**Art:** Funktionsumfangserweiterung
**Datum:** 2026-07-21
**Letzte Überarbeitung:** 2026-07-22
**Abhängigkeit:** Change Request 001 — Akustisches Oberflächen- und Austauschmodell
**Bridge-Voraussetzungen:** normativ abgeschlossener Slice 039b; implementierte CR-001-Slices für Identity Foundation, akustisches Materialmodell sowie Boundary/Opening/Portal
**Referenzfall, keine Produktfunktion:** Bayreuther Festspielhaus
**Normative Wirkung:** Keine, bis der Change Request angenommen und in die normativen Spezifikationen übernommen wurde.

## 1. Zusammenfassung

b-cad soll externe triangulierte Oberflächenmodelle im Format **GLB (glTF 2.0)**
sowie akustische Materialkennlinien aus **CSV-Dateien** importieren können.

Der GLB-Import dient insbesondere dazu, bestehende nichtparametrische
Gebäude- und Saalmodelle, beispielsweise aus SketchUp konvertierte Modelle, als
persistente Referenz- und Oberflächengeometrie in b-cad zu übernehmen.

Der CSV-Import dient dazu, projektspezifische akustische Material- oder
Flächengruppenparameter mit frequenzabhängigen Absorptionswerten,
Streukoeffizienten und Quellenangaben einzulesen. Ein konkreter Referenzfall ist
die Materialtabelle des Bayreuther Festspielhauses.

Die importierte GLB-Geometrie wird **nicht automatisch in parametrische
b-cad-Wände, Decken, Dächer oder Öffnungen umgewandelt**. Sie wird als eigenes,
nichtparametrisches Oberflächenmodell verwaltet. Erst ein ausdrücklicher,
manueller Klassifikationsvorgang ordnet ausgewählte Quellflächen den
akustischen Raumseiten, gemeinsamen Grenzen oder leeren Öffnungen gemäß Change
Request 001 zu.

Die importierten CSV-Daten werden **nicht blind als universelle
Materialbibliothek** behandelt. Projektspezifische oder kalibrierte Datensätze
müssen als solche gekennzeichnet bleiben.

## 2. Anlass und Problemstellung

Bestehende Konzertsaal- und Forschungsmodelle liegen häufig nicht als IFC oder
parametrisches CAD-Modell vor. Typische Ausgangsformate sind SketchUp-Dateien,
polygonale Exporte und Tabellen mit Simulationsparametern.

Unter Linux kann ein SketchUp-Modell außerhalb von b-cad nach GLB konvertiert
werden:

```text
SKP
  ↓ externer Konverter
GLB
  ↓ b-cad
importiertes Oberflächenmodell
  ↓ akustische Klassifikation und Materialzuweisung
b-cad-Akustikmodell
```

Ein späterer Export des b-cad-Akustikmodells ist nicht Bestandteil dieses CR.
Er richtet sich ausschließlich nach der erst noch gemeinsam zu
verabschiedenden produktneutralen Austauschformatspezifikation aus CR 001.

Für den Referenzfall Bayreuther Festspielhaus liegen zusätzlich tabellarische
akustische Daten vor. Eine typische Tabelle enthält:

- Material- oder Flächengruppenname,
- Absorptionsgrade bei 125, 250, 500, 1000, 2000 und 4000 Hz,
- einen Streukoeffizienten,
- eine bibliografische Quelle.

Ohne einen kontrollierten GLB- und CSV-Import müssten Geometrie, Gruppen,
Materialnamen und Kennwerte manuell nachgebildet werden. Das ist fehleranfällig,
schlecht reproduzierbar und für Vergleichs- oder Validierungsmodelle
ungeeignet.

## 3. Beantragte Produktentscheidung

### 3.1 Eigenständiges nichtparametrisches Oberflächenmodell

b-cad erhält ein dauerhaftes fachliches Konzept für importierte
Oberflächenmodelle.

Ein importiertes Modell:

- ist kein parametrisches Gebäudemodell,
- erhält nachvollziehbare Quellgruppen, räumliche Vorkommen und
  Oberflächenbereiche,
- behält seine Quellstruktur soweit möglich,
- kann angezeigt, ausgewählt, ein- und ausgeblendet werden,
- kann durch einen ausdrücklichen Benutzervorgang akustisch klassifiziert und
  belegt werden,
- kann aus einer aktualisierten Quelldatei neu importiert werden,
- bleibt vom ursprünglichen Quelldokument unterscheidbar.

### 3.2 Getrennte Importpfade

Die bestehenden Importpfade für parametrische Gebäude werden nicht
zweckentfremdet.

```text
Parametrischer Gebäudeimport
└── IFC/DXF/... → parametrisches b-cad-Gebäudemodell

Oberflächenimport
└── GLB → importiertes Oberflächenmodell

Materialtabellenimport
└── CSV → akustische Material- oder Flächengruppendaten
```

### 3.3 Systemgrenze zu SketchUp

Direkter `.skp`-Import ist nicht Bestandteil dieses CR.

Die Konvertierung von SKP nach GLB erfolgt durch ein externes Werkzeug. b-cad
muss weder SketchUp noch das proprietäre SketchUp SDK voraussetzen.

### 3.4 Systemgrenze zu a-ray

Dieser CR setzt keinen bestehenden b-cad–a-ray-Austauschvertrag voraus und
trifft keine Entscheidung über dessen Container- oder Geometrieformat.

GLB und CSV sind ausschließlich **Eingabeformate für b-cad**. Sie werden durch
diesen CR weder zu Exportformaten noch zum fachlichen b-cad–a-ray-Vertrag. Ob
die gemeinsame produktneutrale Austauschformatspezifikation aus CR 001 später
GLB, einen anderen Container oder eine andere Repräsentation verwendet, ist
eine unabhängige Entscheidung.

## 4. Ziele und Nutzen

Der Change Request soll ermöglichen:

- bestehende polygonale Saal- und Gebäudemodelle unter Linux zu übernehmen,
- Szenenhierarchien, Instanzen und Materialnamen nachvollziehbar zu erhalten,
- visuelle und akustische Materialien sauber zu trennen,
- projektspezifische Materialtabellen reproduzierbar einzulesen,
- Geometriegruppen mit Materialtabellen kontrolliert zu verknüpfen,
- Importfehler und Datenverluste sichtbar zu machen,
- importierte Modelle für akustische Oberflächen nach CR 001 zu verwenden,
- die allgemeinen Import- und Mappingfunktionen an einem optionalen
  Bayreuth-Referenzfall zu prüfen,
- den b-cad-Kern frei von SketchUp-, CUDA- und Solverabhängigkeiten zu halten.

## 5. Vorgeschlagene Lastenheft-Erweiterung

**ID-Status:** Alle in diesem CR genannten `OBJ`-, `LH-FA`- und `LH-QA`-IDs
sind vorläufig. Vor der normativen Übernahme ist ein Live-Audit gegen den dann
gültigen Stand von `spec/lastenheft.md` und `harness/conventions.md`
durchzuführen; endgültige IDs und alle internen Verweise werden dabei atomar
angepasst. Die Nummerierung setzt weder eine unveränderte noch eine bereits
abgeschlossene Übernahme von CR 001 voraus.

### 5.1 Neue Projektziele

| ID | Ziel | Beschreibung |
|---|---|---|
| `OBJ-007` | Offener Oberflächenimport | Externe triangulierte Szenen können als dauerhaftes, nichtparametrisches Oberflächenmodell übernommen werden. |
| `OBJ-008` | Reproduzierbarer Materialdatenimport | Akustische Kennwerttabellen können validiert, nachvollziehbar und wiederholbar importiert werden. |

### 5.2 Neue Bereichskürzel

Für die funktionalen Anforderungen werden folgende Bereichskürzel beantragt:

- `SMI` — Surface Model Import
- `AMI` — Acoustic Material Import
- `MAP` — Mapping imported geometry to acoustic data

Bei Annahme sind die Kürzel im ID-Schema in `harness/conventions.md` durch einen
neuen additiven `MR`-Eintrag einzuführen. Der bestehende `MR-002` wird nicht
umgeschrieben.

## 6. Funktionale Anforderungen — GLB-Oberflächenimport

### LH-FA-SMI-001 — GLB-Datei importieren

**Beschreibung:** Der Benutzer kann eine lokale `.glb`-Datei als
nichtparametrisches Oberflächenmodell importieren.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine gültige glTF-2.0-GLB-Datei, when der Benutzer sie
  importiert, then werden Geometrie, räumliche Anordnung, erkennbare
  Quellstruktur, Namen und visuelle Materialbezüge nachvollziehbar übernommen.
- **Boundary:** Given eine Datei enthält mehrere auswählbare Szenen, then wird
  die verwendete Szene vor Abschluss des Imports eindeutig angezeigt oder vom
  Benutzer ausgewählt.
- **Negative:** Given eine beschädigte oder nicht unterstützte GLB-Datei, then
  wird der Import mit konkreter Ursache und ohne teilweise persistiertes Modell
  abgelehnt.

### LH-FA-SMI-002 — Initialen Importumfang auf GLB begrenzen

**Beschreibung:** Im initialen Umfang importiert b-cad GLB-Einzeldateien.
`.gltf`-Dateien mit externen Ressourcen gehören nicht zum Produktversprechen
dieses CR.

**Akzeptanzkriterien:**

- **Happy Path:** Given der Benutzer wählt eine `.glb`-Datei, then steht der
  Oberflächenimport zur Verfügung.
- **Negative:** Given der Benutzer wählt eine `.gltf`-Datei, then wird sie als
  im initialen Umfang nicht unterstützt gemeldet; externe oder über Netzwerk
  referenzierte Ressourcen werden nicht geladen.

### LH-FA-SMI-003 — Quellhierarchie und räumliche Anordnung erhalten

**Beschreibung:** Quellhierarchie und räumliche Anordnung werden nachvollziehbar
übernommen.

**Akzeptanzkriterien:**

- die räumliche Anordnung entspricht der ausgewählten Quellszene,
- gespiegelte oder skalierte Vorkommen werden korrekt dargestellt und
  diagnostiziert,
- die Quellhierarchie und ihre Namen bleiben nachvollziehbar,
- wiederholter Import derselben Datei erzeugt dieselbe Anordnung.

### LH-FA-SMI-004 — Dreiecksbasierte Oberflächen übernehmen

**Beschreibung:** Dreiecksbasierte Mesh-Primitives werden als
importierte Oberflächengeometrie übernommen.

**Akzeptanzkriterien:**

- gültige dreiecksbasierte Oberflächen werden unabhängig von ihrer kompakten
  oder ausgeschriebenen Quellrepräsentation übernommen,
- fehlende Orientierungshilfen werden diagnostiziert; optional daraus
  abgeleitete Orientierungsdaten verändern die Quellgeometrie nicht,
- ungeeignete oder nicht dreiecksbasierte Inhalte werden nicht still als
  Oberflächen übernommen,
- der Importbericht nennt Art und Umfang übernommener und verworfener Inhalte.

### LH-FA-SMI-005 — Wiederholte Quellgeometrie unterscheidbar erhalten

**Beschreibung:** Mehrfach vorkommende Quellgeometrie bleibt in jedem
räumlichen Vorkommen einzeln nachvollziehbar.

**Akzeptanzkriterien:**

- jedes sichtbare Vorkommen bleibt einzeln auswählbar und auf seine Quelle
  zurückführbar,
- gleichartige Vorkommen behalten ihre jeweilige räumliche Anordnung,
- eine spätere akustische Klassifikation kann Vorkommen eindeutig
  unterscheiden,
- Speichern/Laden verändert diese Unterscheidung nicht.

### LH-FA-SMI-006 — Größe und Orientierung kontrollieren

**Beschreibung:** b-cad übernimmt Größe und Orientierung eines GLB-Modells
kontrolliert in das Projekt.

**Akzeptanzkriterien:**

- Abmessungen und Orientierung werden vor Abschluss des Imports angezeigt,
- es entsteht kein stiller Maßstabs- oder Achsenfehler,
- unplausible Ausdehnungen lösen eine Warnung und eine sichtbare Bestätigung
  aus,
- derselbe bestätigte Import erzeugt reproduzierbar dieselbe Lage und Größe.

### LH-FA-SMI-007 — Visuelle Materialien übernehmen

**Beschreibung:** Namen und grundlegende Referenzen visueller GLB-Materialien
werden übernommen.

**Akzeptanzkriterien:**

- Name und Zuordnung des visuellen Quellmaterials bleiben nachvollziehbar,
- Farbe und Texturreferenz dürfen zur Orientierung übernommen werden,
- visuelle Materialien werden nicht automatisch zu akustischen Materialien,
- fehlende Materialreferenzen werden als eigener Zustand behandelt.

### LH-FA-SMI-008 — Flächenorientierung diagnostizieren

**Beschreibung:** b-cad prüft Flächenorientierung, Normalen und
Doppelseitigkeit.

**Akzeptanzkriterien:**

- inkonsistente Normalen werden sichtbar gemeldet,
- gespiegelte Instanzen werden bei der Orientierung berücksichtigt,
- eine visuell doppelseitige Fläche wird nicht still als zwei physische
  akustische Flächen interpretiert,
- bei aktivierter Ableitung werden fehlende Normalen reproduzierbar als
  getrennte Diagnosedaten erzeugt; Quellgeometrie und Quellattribute bleiben
  unverändert,
- bei deaktivierter Ableitung bleiben fehlende Normalen als ungelöste Diagnose
  sichtbar; betroffene Bereiche können als Referenzgeometrie gespeichert
  werden, blockieren aber ihre akustische Klassifikation und Exportvalidierung,
- allgemeine Mesh-Reparaturen werden durch diese Diagnose nicht ausgeführt.

### LH-FA-SMI-009 — Importiertes Modell anzeigen und auswählen

**Beschreibung:** Der Benutzer kann importierte Modelle, ihre Gruppen und
räumlichen Vorkommen in Baum- und 3D-Ansicht auswählen.

**Akzeptanzkriterien:**

- Importmodelle können ein-/ausgeblendet und gesperrt werden,
- Auswahl in der Baumansicht hebt die Geometrie hervor,
- Auswahl im Viewer zeigt Quellname, visuelles Material, geometrischen Umfang
  und räumliche Anordnung,
- große Modelle bleiben über Gruppen und Filter bedienbar.

### LH-FA-SMI-010 — Importmodell persistent speichern

**Beschreibung:** Das importierte Oberflächenmodell und seine fachlichen
Referenzen bleiben nach Speichern/Laden erhalten.

**Akzeptanzkriterien:**

- der Roundtrip verändert Geometrie, Hierarchie und Kennungen nicht,
- Herkunft und Importvorgang bleiben nachvollziehbar,
- das Projekt bleibt auch verfügbar, wenn die externe Quelldatei später fehlt,
  sofern die Geometrie eingebettet gespeichert wurde.

### LH-FA-SMI-011 — Quelldatei neu einlesen

**Beschreibung:** Der Benutzer kann ein Importmodell aus einer geänderten
GLB-Datei aktualisieren.

**Akzeptanzkriterien:**

- vor der Bestätigung werden unveränderte, geänderte, neue und entfallene
  Quellbereiche samt betroffenen akustischen Zuordnungen angezeigt,
- sicher wiedererkannte Bereiche können ihre Zuordnungen behalten;
  mehrdeutige oder geänderte Bereiche werden als ungelöste Konflikte markiert,
- der Benutzer kann den Neuimport abbrechen oder den vollständig angezeigten
  neuen Stand bestätigen,
- bei Bestätigung werden Importmodell, sichere Zuordnungen und Konfliktstatus
  atomar aktualisiert; ungelöste Konflikte blockieren die akustische
  Exportprüfung,
- bei Abbruch oder Fehler bleiben Importmodell und abhängige Zuordnungen
  vollständig unverändert.

### LH-FA-SMI-012 — Importbericht erzeugen

**Beschreibung:** Jeder Import erzeugt einen strukturierten Bericht.

**Akzeptanzkriterien:**

- Quelle, Importversion und Zeitpunkt bleiben nachvollziehbar,
- Umfang, Ausdehnung und visuelle Materialgruppen des übernommenen Modells
  werden zusammengefasst,
- verworfene oder reparierte Inhalte sowie Warnungen und Fehler werden mit
  ihrer betroffenen Quellstelle aufgeführt,
- ein wiederholter Import mit denselben Einstellungen erzeugt einen fachlich
  vergleichbaren Bericht.

### LH-FA-SMI-013 — Importiertes Modell entfernen

**Beschreibung:** Ein importiertes Oberflächenmodell kann kontrolliert aus dem
Projekt entfernt werden.

**Akzeptanzkriterien:**

- abhängige akustische Oberflächen und Materialabbildungen werden vor dem
  Entfernen angezeigt,
- ohne Abhängigkeiten kann das Modell unmittelbar entfernt werden,
- bei Abhängigkeiten kann der Benutzer abbrechen oder das Importmodell
  gemeinsam mit allen daraus abgeleiteten akustischen Oberflächen und
  Geometriezuordnungen entfernen; eigenständige akustische Materialien bleiben
  erhalten,
- die gewählte Entfernung erfolgt atomar; bei Fehler bleiben Modell und
  Abhängigkeiten unverändert,
- es entstehen keine verwaisten Referenzen.

## 7. Funktionale Anforderungen — CSV-Akustikmaterialimport

### LH-FA-AMI-001 — CSV-Datei auswählen und Vorschau anzeigen

**Beschreibung:** Der Benutzer kann eine lokale CSV-Datei auswählen und vor dem
Import eine tabellarische Vorschau anzeigen.

**Akzeptanzkriterien:**

- eine unterstützte CSV-Datei wird ohne Veränderung der Quelldatei lesbar
  dargestellt,
- erkannte Tabellenstruktur und Zahleninterpretation werden vor dem Import
  angezeigt und können bei Mehrdeutigkeit korrigiert werden,
- fehlerhafte Zeilen und nicht interpretierbare Werte sind in der Vorschau
  auffindbar,
- eine nicht unterstützte Datei wird mit konkreter Ursache abgelehnt,
- die Quelldatei wird durch die Vorschau nicht verändert.

### LH-FA-AMI-002 — Spalten auf fachliche Felder abbilden

**Beschreibung:** Der Benutzer kann CSV-Spalten auf Materialname,
Frequenzkennwerte, Streuung, Transmission und Quellenmetadaten abbilden.

**Akzeptanzkriterien:**

- erkannte Materialbezeichnungen, Frequenzkennwerte und Quellenangaben werden
  vor der Übernahme fachlich benannt angezeigt,
- Originalbezeichnungen bleiben zur Herkunftsklärung sichtbar,
- nicht erkannte Inhalte können bewusst ausgelassen oder als Zusatzinformation
  erhalten werden,
- mehrdeutige Zuordnungen verlangen eine Benutzerentscheidung.

### LH-FA-AMI-003 — Frequenzkennlinien importieren

**Beschreibung:** Absorptions-, Streu- und optionale Transmissionskennwerte
werden als frei definierte Frequenzstützstellen importiert.

**Akzeptanzkriterien:**

- Frequenzen müssen positiv, je Kennlinie eindeutig und streng aufsteigend sein,
- einzelne Kennwerte und ihre Kombination müssen eine passive Energiebilanz
  ohne Energieerzeugung erfüllen; Streuung wirkt nur auf den reflektierten
  Anteil,
- fehlende optionale Werte werden in der Vorschau mit ihrer fachlichen Wirkung
  angezeigt,
- ungültige Werte und kombinierte Verletzungen werden datensatzgenau gemeldet,
- ein energetisch oder strukturell fehlerhafter Datensatz wird nicht teilweise
  als gültiges Material gespeichert; enthält der Importvorgang einen
  blockierenden Fehler, bleibt der gesamte Materialbestand unverändert.

### LH-FA-AMI-004 — Einzelnen Streukoeffizienten behandeln

**Beschreibung:** Enthält eine Tabelle nur einen frequenzunabhängigen
Streukoeffizienten, kann er als konstante Streukennlinie übernommen werden.

**Akzeptanzkriterien:**

- die Replikation auf vorhandene Frequenzstützstellen ist in der Vorschau
  sichtbar,
- das Material wird als „konstante Streuung aus Einzelwert“ gekennzeichnet,
- es wird nicht vorgetäuscht, dass frequenzabhängige Streumessungen vorliegen.

### LH-FA-AMI-005 — Projektspezifischen Geltungsbereich kennzeichnen

**Beschreibung:** Ein importierter Datensatz kann als allgemein,
herstellerspezifisch, literaturbasiert, geschätzt, kalibriert oder
projektspezifisch klassifiziert werden.

**Akzeptanzkriterien:**

- der Benutzer muss für Tabellen ohne eindeutige Metadaten einen Geltungsbereich
  bestätigen,
- projektspezifische oder für eine Simulation kalibrierte Daten können als
  solche gekennzeichnet werden,
- projektspezifische Daten werden nicht automatisch in eine globale
  Materialbibliothek verschoben.

### LH-FA-AMI-006 — Quellenangaben erhalten

**Beschreibung:** Bibliografie-, Quellen- und Referenzangaben werden
unverändert und strukturiert übernommen.

**Akzeptanzkriterien:**

- der Originaltext bleibt erhalten,
- optional können Autor, Jahr, Titel und Referenz getrennt erfasst werden,
- fehlende Quellenangaben lösen eine Warnung, aber nicht zwingend einen
  Importfehler aus,
- Quelle und Quellzeile bleiben am Material nachvollziehbar.

### LH-FA-AMI-007 — Doppelte Materialnamen behandeln

**Beschreibung:** Bei identischen oder normalisiert identischen Namen muss der
Benutzer zwischen Ersetzen, Zusammenführen, Umbenennen und Überspringen wählen
können.

**Akzeptanzkriterien:**

- bestehende Materialien werden nicht still überschrieben,
- Unterschiede der Kennlinien werden in einer Vorschau angezeigt,
- **Ersetzen:** Der bestehende Materialdatensatz behält seine fachliche
  Identität und erhält vollständig die bestätigten neuen Werte; bestehende
  Oberflächenzuweisungen bleiben mit ihm verbunden.
- **Zusammenführen:** Nur konfliktfreie Ergänzungen werden vorgeschlagen.
  Abweichende Werte für denselben Kennwert müssen vor der Bestätigung einzeln
  aufgelöst werden.
- **Umbenennen:** Ein neuer, eigenständiger Materialdatensatz wird angelegt;
  das bestehende Material bleibt unverändert.
- **Überspringen:** Weder Material noch bestehende Zuweisungen werden geändert.
- Die Entscheidungen für alle Namenskonflikte werden vor der Übernahme
  zusammengefasst; Fehler oder Abbruch lassen den gesamten Materialbestand
  unverändert.

### LH-FA-AMI-008 — CSV-Importprofil speichern

**Beschreibung:** Spaltenabbildung, Trennzeichen, Dezimalformat und
Klassifikationsregeln können als benanntes Importprofil gespeichert werden.

**Akzeptanzkriterien:**

- ein Profil kann erneut auf gleichartige Tabellen angewendet werden,
- Profilversion und verwendete Einstellungen werden im Importprotokoll
  dokumentiert,
- fehlende erwartete Spalten werden vor dem Import gemeldet.

### LH-FA-AMI-009 — Materialimport atomar ausführen

**Beschreibung:** Der Materialimport wird vollständig oder gar nicht
übernommen.

**Akzeptanzkriterien:**

- blockierende Fehler verhindern jede Änderung,
- bei Abbruch bleibt der vorherige Materialbestand unverändert,
- erfolgreiche Importe können über einen zusammengehörigen Vorgang
  nachvollzogen werden.

### LH-FA-AMI-010 — Importprotokoll erzeugen

**Beschreibung:** Jeder CSV-Import erzeugt ein strukturiertes Protokoll.

**Akzeptanzkriterien:**

- Quelle, Importprofil und Importvorgang bleiben nachvollziehbar,
- übernommene, ersetzte, zusammengeführte, umbenannte, übersprungene und
  fehlerhafte Datensätze werden zusammengefasst,
- Warnungen und Fehler lassen sich zum betroffenen Quelldatensatz
  zurückverfolgen,
- das Protokoll zeigt, ob der Vorgang vollständig übernommen oder ohne
  Bestandsänderung abgebrochen wurde.

## 8. Funktionale Anforderungen — Geometrie-/Materialabbildung

Die Acoustic Bridge dieses Abschnitts setzt vier getrennte Vorleistungen
voraus: den normativ abgeschlossenen Spec-/ADR-Slice 039b ohne Produktionscode,
die implementierte CR-001-Identity-Foundation, das implementierte akustische
Materialmodell sowie das implementierte Boundary-/Opening-/Portalmodell. Ein
importiertes Quellobjekt wird niemals allein durch einen Materialnamen oder den
Import selbst zu einer akustischen Raumseite.

### LH-FA-MAP-001 — Importierte Geometrie akustisch klassifizieren

**Beschreibung:** Der Benutzer kann ausgewählte Bereiche eines importierten
Oberflächenmodells manuell als raumseitige Oberfläche, Seite einer gemeinsamen
Raumgrenze, Einbauteiloberfläche oder leere Öffnung klassifizieren und den
betroffenen Räumen zuordnen.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein importierter Flächenbereich und ein erkannter Raum,
  when der Benutzer den Bereich als Raumseite klassifiziert, then entsteht eine
  stabil referenzierbare akustische Oberfläche mit sichtbarem Rückverweis auf
  die importierte Quelle.
- **Gemeinsame Grenze:** Given ein Flächenbereich trennt zwei Räume, when beide
  Raumseiten klassifiziert werden, then bleiben ihre akustischen Belegungen
  unabhängig, ohne eine geometrische Doppelbelegung zu erzeugen.
- **Leere Öffnung:** Given eine geschlossene Randkontur eines offenen
  Meshbereichs oder ein geschlossenes eingezeichnetes Öffnungspolygon, when
  der Benutzer eine eindeutige Wirtsgrenze und die angrenzenden Räume zuordnet,
  then entsteht dieselbe persistente, bearbeit- und löschbare leere Öffnung wie
  nach CR 001.
- **Boundary:** Given dieselbe importierte Quelle wird in mehreren räumlichen
  Vorkommen verwendet, then kann jedes Vorkommen getrennt klassifiziert werden.
- **Negative:** Given eine Klassifikation ist geometrisch oder räumlich
  mehrdeutig, then wird sie nicht gespeichert; die Ursache und betroffenen
  Bereiche werden angezeigt.
- **Ungültige Öffnung:** Given Kontur oder Polygon ist nicht geschlossen,
  selbstüberschneidend, keiner eindeutigen Wirtsgrenze zuordenbar oder würde
  eine vorhandene Fläche beziehungsweise Öffnung doppelt belegen, then wird die
  Klassifikation ohne Bestandsänderung abgelehnt.

Quellbereich, akustische Oberfläche und gemeinsame Raumgrenze bleiben getrennte
fachliche Identitäten. Eine unvollständige Klassifikation darf als
Zwischenstand gespeichert werden, blockiert aber die akustische
Exportvalidierung nach CR 001.

### LH-FA-MAP-002 — Visuelles Material auf akustisches Material abbilden

**Beschreibung:** Der Benutzer kann ein importiertes visuelles GLB-Material
einem akustischen Material zuordnen.

**Akzeptanzkriterien:**

- alle betroffenen Quellbereiche und räumlichen Vorkommen werden vor der
  Zuweisung angezeigt,
- eine Zuweisung kann projektweit oder selektiv gelten,
- das visuelle Material bleibt als Herkunftsmerkmal erhalten,
- die akustische Zuweisung bleibt nach Speichern/Laden erhalten.

### LH-FA-MAP-003 — Namenbasierten Zuordnungsvorschlag erzeugen

**Beschreibung:** b-cad kann anhand normalisierter Namen Vorschläge zwischen
GLB-Materialien, Knoten oder Gruppen und importierten CSV-Materialien erzeugen.

**Akzeptanzkriterien:**

- Vorschläge werden nicht automatisch als bestätigt behandelt,
- Groß-/Kleinschreibung, Leerzeichen und einfache Sonderzeichen dürfen
  normalisiert werden,
- unscharfe Ähnlichkeit muss als solche gekennzeichnet sein,
- mehrere mögliche Treffer verlangen eine Benutzerentscheidung.

### LH-FA-MAP-004 — Regelbasierte Zuordnung speichern

**Beschreibung:** Der Benutzer kann Zuordnungsregeln als projektbezogenes
Mapping-Profil speichern.

**Akzeptanzkriterien:**

- ein Profil kann aus bestätigten Zuordnungen erzeugt, benannt und erneut
  angewendet werden,
- der Geltungsbereich jeder Regel und ihre Priorität werden vor der Anwendung
  verständlich angezeigt,
- mehrere passende oder widersprüchliche Regeln verlangen eine
  Benutzerentscheidung,
- die Anwendung eines Profils erzeugt zunächst eine Vorschau und verändert
  ohne Bestätigung keine Zuordnung,
- Profil und bestätigte Zuordnungen bleiben nach Speichern/Laden erhalten.

### LH-FA-MAP-005 — Unklassifizierte oder unbelegte Flächen anzeigen

**Beschreibung:** b-cad zeigt getrennt an, welche importierten Bereiche noch
nicht akustisch klassifiziert und welche klassifizierten Oberflächen noch ohne
akustische Materialzuweisung sind.

**Akzeptanzkriterien:**

- Anzahl und Fläche werden je Zustand zusammengefasst,
- betroffene Elemente sind im Viewer auswählbar,
- nach CR 001 kann ein vollständiger Akustikexport blockiert werden, solange
  Pflichtflächen unbelegt sind.

### LH-FA-MAP-006 — Zuordnungen bei Neuimport prüfen

**Beschreibung:** Nach Aktualisierung eines GLB-Modells werden bestehende
Geometrie-/Materialzuordnungen erneut geprüft.

**Akzeptanzkriterien:**

- sicher wiedererkannte Elemente behalten die Zuordnung,
- umbenannte oder strukturell geänderte Elemente werden als Konflikt angezeigt,
- keine Zuordnung wird still auf einen anderen Quellbereich übertragen,
- ungelöste Klassifikations- oder Materialkonflikte blockieren die akustische
  Exportvalidierung,
- Bestätigung, Abbruch und Fehlerwirkung folgen der atomaren
  Neuimportsemantik aus `LH-FA-SMI-011`.

## 9. Qualitätsanforderungen

### LH-QA-012 — Importdeterminismus

Bei identischer Quelldatei, identischer Importerversion und identischem
Importprofil muss ein fachlich identisches Importmodell entstehen.

### LH-QA-013 — Quellnachvollziehbarkeit

Jedes importierte Modell und jeder importierte Materialdatensatz muss auf
Quelldatei, Prüfsumme, Importer-Version und Importvorgang zurückgeführt werden
können.

### LH-QA-014 — Keine automatische Parametrisierung

b-cad darf importierte Meshes nicht still als parametrische Wände, Decken oder
Räume ausgeben. Eine spätere manuelle oder halbautomatische Rekonstruktion muss
als eigener Vorgang erkennbar sein.

### LH-QA-015 — Große Dateien

Auf dem in der technischen Spezifikation festgelegten Referenzsystem muss die
Vorschau eines Referenzmodells mit einer Million Dreiecken, 10.000
Szenenelementen und 1.000 wiederverwendeten Vorkommen innerhalb von 10 Sekunden
erscheinen. Der bestätigte Import muss innerhalb von 60 Sekunden abgeschlossen
sein und darf höchstens 3 GiB zusätzlichen Arbeitsspeicher belegen. Fortschritt
und Abbruchmöglichkeit bleiben währenddessen sichtbar.

### LH-QA-016 — Sichere Dateiverarbeitung

Parserfehler, übergroße Allokationen, zyklische oder extrem tiefe
Szenenhierarchien und ungültige Geometriebezüge dürfen nicht zum
unkontrollierten Absturz führen. Eingaben innerhalb der in Abschnitt 10.4
festgelegten Ressourcenlimits werden verarbeitet; Überschreitungen werden vor
einer unkontrollierten Ressourcenbelegung mit konkreter Ursache und ohne
Bestandsänderung abgelehnt.

### LH-QA-017 — Unveränderte Standardfunktionen

Der GLB-/CSV-Import darf bestehende IFC-, DXF- oder b-cad-Projektimporte nicht
verändern oder von optionalen Konverterwerkzeugen abhängig machen.

### LH-QA-018 — Plattformneutralität

Der Importkern muss unter Linux ohne SketchUp-Installation und ohne
proprietäres SketchUp SDK funktionieren.

### LH-QA-019 — Lizenz- und Herkunftstransparenz

b-cad muss Herkunfts- und Lizenzinformationen importierter Modelle und
Materialdaten speichern können. Das Vorhandensein einer Datei bedeutet nicht
automatisch, dass sie frei weitergegeben werden darf.

## 10. Fachliche Regeln für die technische Spezifikation

Bei Annahme sind mindestens folgende Punkte technisch festzulegen:

### 10.1 GLB

1. Initial wird ausschließlich GLB auf Basis von glTF 2.0 unterstützt.
   `.gltf` sowie externe oder Netzwerkressourcen werden vor dem Import
   abgelehnt.
2. Zulässige GLB-Chunks, glTF-Versionen und Extensions sowie der Umgang mit
   erforderlichen, aber nicht unterstützten Extensions werden festgelegt.
3. Für dreiecksbasierte Geometrie sind Positionsdaten Pflicht; Indizes sind
   optional. Eine nicht indizierte Dreiecksliste wird in aufeinanderfolgenden
   Dreiergruppen gelesen und ist nur bei einer durch drei teilbaren Vertexzahl
   gültig.
4. Initial wird nur der glTF-Primitivmodus `TRIANGLES` übernommen.
   `TRIANGLE_STRIP`, `TRIANGLE_FAN`, Punkte und Linien werden als nicht
   unterstützt protokolliert und verworfen. Enthält die gewählte Szene danach
   keine übernehmbare Dreiecksgeometrie, wird der gesamte Import abgelehnt.
5. Unterstützte Accessor-, Komponenten- und Indextypen, Sparse Accessors,
   Bereichsprüfungen und Ausrichtung werden festgelegt.
6. Knoten-, Primitive-, Vorkommens- und Quellkennungen sowie deren
   Matchingstrategie beim Neuimport werden festgelegt.
7. Transformation von Achsen, Händigkeit und Einheiten sowie die Behandlung
   negativer Skalierung und des Winding werden festgelegt. Vorhandene Normalen
   werden validiert. Fehlen sie, darf aus Dreiecksorientierung und
   Welttransformation deterministisch eine separate Diagnosenormale abgeleitet
   werden. Diese Ableitung mutiert weder Quellpositionen noch Quellattribute und
   führt kein Welding, Glätten, Schließen oder anderes Mesh-Repair aus. Ist die
   Ableitung deaktiviert, bleibt der Orientierungszustand `unresolved`; der
   Import als Referenzgeometrie ist zulässig, Acoustic Bridge und
   Exportvalidierung bleiben für die betroffenen Bereiche bis zur expliziten
   Klärung blockiert.
8. Textur- und Bildspeicherung sowie Einbettung der importierten Geometrie in
   den Projektbestand werden festgelegt.
9. Vorschau, Import, Neuimport und Entfernung verwenden atomare Commit-Grenzen;
   ein Parser- oder Persistenzfehler verändert den Projektbestand nicht.

### 10.2 CSV

1. Unterstützt werden UTF-8 mit oder ohne BOM sowie Komma, Semikolon und
   Tabulator als Trennzeichen. Dezimalpunkt und Dezimalkomma werden in der
   Vorschau erkannt und können manuell korrigiert werden.
2. Spaltennamen werden normalisiert, ihre Originalschreibweise bleibt erhalten.
   Zulässige Schreibweisen für Materialname, Frequenz, Absorption, Streuung,
   Transmission und Quellenangaben werden einschließlich Mustern wie
   `alpha125`, `a125`, `scattering`, `transmission` und `source` festgelegt.
3. Frequenzen sind positiv und innerhalb einer Kennlinie eindeutig. Original-
   und normalisierte Werte sowie Rundungsregeln werden gespeichert.
4. Fehlt der optionale Transmissionsgrad `τ(f)`, gilt `τ(f) = 0`. Für
   Absorption `α(f)` und Transmission gilt `0 ≤ α(f) ≤ 1`,
   `0 ≤ τ(f) ≤ 1` und `α(f) + τ(f) ≤ 1`.
5. Der reflektierte Anteil ist `ρ(f) = 1 - α(f) - τ(f)`. Der Streugrad
   `0 ≤ s(f) ≤ 1` teilt ausschließlich `ρ(f)` in
   `(1-s(f))·ρ(f)` spiegelnde und `s(f)·ρ(f)` diffuse Energie.
6. Behandlung sonstiger leerer Zellen, Interpolation und Extrapolation werden
   festgelegt und in der Vorschau ausgewiesen.
7. Ersetzen, Zusammenführen, Umbenennen und Überspringen werden gemäß
   `LH-FA-AMI-007` als explizite Konfliktoperationen modelliert.
8. Herkunfts- und Geltungsbereichsmodell, Profilformat, Versionierung sowie die
   atomare Importsemantik werden festgelegt.

### 10.3 Mapping

1. Slice 039b liefert ausschließlich die normative b-cad-Spezifikation und die
   ADRs; sein Abschluss bedeutet nicht, dass CR-001-Produktionscode existiert.
   Die Acoustic Bridge darf erst implementiert werden, wenn Slice 039b normativ
   abgeschlossen und die daraus geplanten CR-001-Slices für Identity Foundation,
   akustisches Materialmodell sowie Boundary/Opening/Portal implementiert sind.
2. Quell-Primitive und Vorkommen besitzen Importidentitäten und
   Host-Provenance. Diese Kennungen sind weder `surface_id` noch Kennung einer
   gemeinsamen Grenzfläche.
3. Die manuelle Klassifikation erzeugt oder referenziert eigenständige
   akustische `surface_id`-Werte und ordnet sie einer Raumseite zu. Zwei Seiten
   einer gemeinsamen Grenze referenzieren eine gemeinsame geometrische Grenze,
   ohne Quell-Primitives zu duplizieren.
4. Leere Öffnungen werden aus einer geschlossenen Randkontur eines offenen
   Meshbereichs oder einem geschlossenen, nicht selbstüberschneidenden
   Öffnungspolygon erzeugt. Wirtsgrenze und angrenzende Räume müssen eindeutig
   sein; die Fläche darf keine bestehende Oberfläche oder Öffnung doppelt
   belegen. Das Ergebnis ist dieselbe persistente, bearbeit- und löschbare
   Öffnungsentität und Portalbeziehung wie nach CR 001. Ein visuelles Loch oder
   ein Materialname allein erzeugt kein Portal.
5. Priorität expliziter Quell-, Material-, Knoten- und Namensregeln,
   Normalisierung und Umgang mit Mehrdeutigkeiten werden festgelegt.
6. Persistenz stabiler Zuordnungen, Verhalten bei Geometrie-Neuimport und
   Protokollierung manueller Bestätigungen werden festgelegt.
7. Bei bestätigtem Neuimport werden Importmodell, Host-Provenance, sicher
   erkannte Zuordnungen und Konflikte in einer Transaktion aktualisiert.

### 10.4 Ressourcen- und Leistungsgrenzen

1. Der GLB-Import akzeptiert Quelldateien bis 2 GiB, höchstens 8 GiB
   dekodierte Geometrie-Bufferdaten, eine Szenentiefe bis 256 und höchstens eine
   Million Szenenelemente.
2. Zusätzlich gelten höchstens 100 Millionen Vertices, 300 Millionen Indizes
   und 100 Millionen Dreiecke je Importvorgang.
3. Dekodierte Bilder und Texturen sind zusammen auf 4 GiB, einzeln auf 1 GiB
   und je Bild auf 32.768 × 32.768 Pixel begrenzt. Die Prüfung verwendet die
   dekodierte Pixelgröße, nicht nur die komprimierte Dateigröße.
4. Für jeden GLB-Import gilt ein gemeinsames logisches Importbudget von 12 GiB.
   Darauf werden Quellbytes, dekodierte Buffer und Bilder, abgeleitete Normalen,
   Vorschaugeometrie, Indexdaten sowie sonstige temporäre oder persistierbare
   Ableitungen vor ihrer Erzeugung angerechnet. Gemeinsam genutzte unveränderte
   Daten werden innerhalb desselben Vorgangs nur einmal gezählt.
5. Der CSV-Import akzeptiert Dateien bis 1 GiB und eine Million Datensätze;
   Vorschau, normalisierte Werte und temporäre Konfliktdaten werden auf dasselbe
   gemeinsame Prinzip eines vorab geprüften Importbudgets verpflichtet. Dessen
   CSV-Grenze beträgt 4 GiB.
6. Vor jeder größenabhängigen Allokation werden deklarierte und tatsächliche
   Größen, Elementprodukte und kumulierte Budgets gegen diese Grenzen und gegen
   arithmetischen Überlauf geprüft.
7. Überschreitungen werden vor dem Commit mit konkreter Ursache abgelehnt;
   temporäre Daten werden verworfen und der Projektbestand bleibt unverändert.
8. Referenzhardware, Messbeginn und Messende für `LH-QA-015` werden in der
   Spezifikation reproduzierbar dokumentiert.

## 11. Auswirkungen auf Datenmodell und Architektur

Voraussichtlich werden neue Entitäten benötigt:

- `ImportedSurfaceModel`,
- `ImportedScene`,
- `ImportedNode`,
- `ImportedMesh`,
- `ImportedPrimitive`,
- `ImportedInstance`,
- `ImportedVisualMaterial`,
- `ImportSource`,
- `ImportReport`,
- `AcousticMaterialImportBatch`,
- `AcousticMaterialSourceRow`,
- `MaterialMappingProfile`,
- `GeometryMaterialBinding`,
- `AcousticSurfaceClassification`,
- `ImportedGeometryHostProvenance`.

Vorgeschlagene neutrale Schnittstellen:

```cpp
class SurfaceModelImporterPort {
public:
    virtual ~SurfaceModelImporterPort() = default;

    virtual SurfaceImportPreview inspect(
        const std::filesystem::path& source) const = 0;

    virtual ImportedSurfaceModel importModel(
        const std::filesystem::path& source,
        const SurfaceImportOptions& options) const = 0;
};
```

```cpp
class AcousticMaterialTableImporterPort {
public:
    virtual ~AcousticMaterialTableImporterPort() = default;

    virtual MaterialTablePreview inspect(
        const std::filesystem::path& source,
        const MaterialTableImportOptions& options) const = 0;

    virtual AcousticMaterialImportBatch importTable(
        const std::filesystem::path& source,
        const MaterialTableImportOptions& options) const = 0;
};
```

Die Ports dürfen keine SketchUp-, glTF-Bibliotheks-, Qt- oder
Solver-spezifischen Datentypen nach außen geben.

Betroffene Bereiche:

| Bereich | Erwartete Änderung |
|---|---|
| `spec/data-model.yaml` | Importmodelle, Quellmetadaten, Mappings und Batch-Protokolle |
| Domänenmodell | nichtparametrische Szenen- und Meshobjekte |
| Importadapter | GLB- und CSV-Adapter |
| Persistenz | eingebettete oder referenzierte Importdaten |
| Viewer | Hierarchie, Instanzen, Materialfarben und Diagnosen |
| Akustikmodul | manuell klassifizierte Importbereiche als Host-Provenance eigenständiger akustischer Oberflächen |
| Validierung | Mesh-, Normalen-, Einheiten- und Materialprüfungen |
| Tests | Referenz-GLBs, CSV-Varianten, Roundtrip und Neuimport |
| CLI | optionaler headless Import und Bericht |

## 12. Referenz- und Regressionstests

Das Bayreuther Festspielhaus ist ein möglicher Referenzfall zur Abnahme der
allgemeinen Import-, Klassifikations- und Mappinganforderungen, aber keine
eigene Produktfunktion.

Der zu prüfende Referenzworkflow endet im Rahmen dieses CR beim
b-cad-Akustikmodell:

```text
Bayreuth.skp
    ↓ externer Linux-Konverter
Bayreuth.glb
    ↓ GLB-Import
b-cad ImportedSurfaceModel

Materials.xls
    ↓ kontrollierte Konvertierung nach CSV
Materials.csv
    ↓ CSV-Import
projektspezifische Bayreuth-Akustikmaterialien

GLB-Namen / Gruppen / Materialien
    ↓ bestätigtes Mapping
akustisch klassifizierte und belegte Oberflächen
    ↓ Hüllen- und Identitätsprüfung nach CR 001
b-cad-Akustikmodell
```

Ein Exportformat oder Export zu a-ray wird durch diesen Workflow weder
vorausgesetzt noch festgelegt.

Die öffentlichen automatisierten Tests verwenden synthetische, frei
weitergebbare GLB- und CSV-Fixtures. Sie decken mindestens Gruppen,
Vorkommen, visuelle Materialien, Raumseitenklassifikation, gemeinsame Grenzen,
leere Öffnungen, Materialkonflikte, passive Energiebilanz und Neuimport ab.
Eine synthetische Materialtabelle kann beispielsweise folgende logische
Struktur besitzen:

| Materialname | α125 | α250 | α500 | α1000 | α2000 | α4000 | s | Bibliografie |
|---|---:|---:|---:|---:|---:|---:|---:|---|
| SyntheticWood | 0.18 | 0.12 | 0.10 | 0.09 | 0.08 | 0.07 | 0.05 | Synthetic fixture |
| SyntheticAudience | 0.40 | 0.59 | 0.67 | 0.70 | 0.72 | 0.76 | 0.70 | Synthetic fixture |

Reale Bayreuth-Dateien dürfen erst in ein Repository oder verteiltes
Testpaket aufgenommen werden, wenn Lizenz, Weitergaberecht und zulässiger
Nutzungsumfang dokumentiert sind. Die Konverter-Provenance umfasst mindestens
Quellprüfsumme, Werkzeug, Werkzeugversion, Einstellungen, Ausführungsplattform
und Ergebnisprüfsumme. Fehlt eine dieser Freigaben, bleibt der reale Datensatz
in einem optionalen privaten Referenzpaket; die öffentlichen Regressionstests
verwenden ausschließlich synthetische Fixtures.

## 13. Nicht Bestandteil dieses CR

- direkter SketchUp-`.skp`-Import,
- `.gltf` mit externen Ressourcen im initialen Umfang,
- Einbettung eines SketchUp-Konverters in b-cad,
- automatische Rekonstruktion parametrischer b-cad-Bauteile,
- vollautomatische Raum- oder Wandklassifikation,
- allgemeine automatische Mesh-Reparatur,
- akustische Simulation,
- Quellen und Empfänger,
- Festlegung, Änderung oder Annahme eines b-cad–a-ray-Austauschformats,
- XLS-/XLSX-Parser im initialen Umfang,
- globale Veröffentlichung fremder Materialdatensätze,
- Interpretation visueller Texturen als akustische Kennwerte,
- garantierte verlustfreie Übernahme jeder glTF-Extension.

Ein späterer XLSX-Import kann ein eigener CR oder eine Erweiterung des
CSV-Importers sein. Für den initialen Umfang wird eine kontrollierte
XLS/XLSX-nach-CSV-Konvertierung vorausgesetzt.

## 14. Risiken

### R-1 — Semantikverlust bei SKP→GLB

Der externe Konverter kann Gruppen, Instanzen, Materialseiten, Namen oder
Normalen verändern. b-cad kann nur die GLB-Datei validieren, nicht automatisch
die Gleichheit mit dem ursprünglichen SKP garantieren.

### R-2 — Verwechslung visueller und akustischer Materialien

Ein GLB-Materialname ist zunächst nur ein visuelles oder organisatorisches
Merkmal. Eine akustische Zuweisung muss explizit erfolgen.

### R-3 — Instabile externe Namen

Namen können beim Konvertieren verändert oder dupliziert werden. Dauerhafte
Zuordnungen dürfen nicht ausschließlich auf Anzeigenamen beruhen.

### R-4 — Große und komplexe Modelle

Ungefilterte Konzertsaalmodelle können sehr viele Instanzen, Dreiecke und
Texturen enthalten.

### R-5 — Fehlerhafte Oberflächentopologie

Visuell plausible Modelle können offene Kanten, doppelte Flächen, umgedrehte
Normalen und degenerierte Dreiecke enthalten.

### R-6 — Locale-Fehler in CSV

Dezimalkomma und Semikolon beziehungsweise Dezimalpunkt und Komma können zu
falsch interpretierten Werten führen.

### R-7 — Scheingenauigkeit der Materialdaten

Kalibrierte Simulationsparameter sind nicht automatisch direkt gemessene,
allgemein übertragbare Materialkennwerte.

### R-8 — Versionsdrift

Änderungen am externen Konverter oder am GLB-Generator können
Szenenhierarchien und Mappingregeln beeinflussen.

### R-9 — Lizenzverletzungen

Öffentlich herunterladbare Modelle oder Tabellen können Einschränkungen für
Weitergabe und kommerzielle Nutzung besitzen.

## 15. Umsetzungsvorschlag nach Annahme

1. **Lastenheft-/ID-Slice:** Live-Audit ausführen, endgültige IDs vergeben und
   die Bereichskürzel durch einen neuen additiven MR-Eintrag einführen.
2. **CR-002-Spec-Slice:** Importgrenzen, Energiebilanz, Ressourcenlimits,
   Konfliktwirkung, Klassifikation und atomare Operationen spezifizieren.
3. **CR-002-ADR-Slice:** Lebenszyklus importierter Modelle, Einbettung, stabile
   Importidentitäten und neutrale Bridge-Naht entscheiden.
4. **CR-001-Normative-Gate:** Slice 039b ist mit b-cad-Spezifikation und ADRs
   normativ abgeschlossen. Dieses Gate enthält keinen Produktionscode.
5. **CR-001-Identity-Prerequisite:** Der aus Slice 039b geplante
   Identity-Foundation-Slice ist implementiert und abgenommen.
6. **CR-001-Material-Prerequisite:** Das aus Slice 039b geplante akustische
   Materialmodell ist implementiert und abgenommen; CR 002 legt keine zweite
   Materialdomäne an.
7. **CR-001-Boundary-Prerequisite:** Das aus Slice 039b geplante
   Boundary-/Opening-/Portalmodell ist implementiert und abgenommen; CR 002 legt
   keine parallelen Grenz- oder Portalentitäten an.
8. **GLB-Inspect-Slice:** Parser, Vorschau und Importbericht.
9. **GLB-Domain-Slice:** Persistentes `ImportedSurfaceModel`.
10. **Viewer-Slice:** Hierarchie, Vorkommen und Materialanzeige.
11. **CSV-Inspect-Slice:** Tabellenstruktur-, Spalten- und Werteerkennung.
12. **CSV-Domain-Slice:** Importiert validierte Kennwerte in das vorhandene
    CR-001-Materialmodell; Energiebilanz und Herkunftsdaten bleiben erhalten.
13. **Mapping-Slice:** visuelle zu akustischen Materialien und Mappingprofile
    auf Basis der vorhandenen CR-001-Materialidentitäten.
14. **Reimport-/Removal-Slice:** Matching sowie atomare Konflikt-, Neuimport-
    und Löschwirkung.
15. **Acoustic-Bridge-Slice:** manuelle Klassifikation mit getrennter
    Quell-Provenance und akustischer Identität. Der Slice startet erst, wenn die
    Gates 4 bis 7 erfüllt sind.
16. **Fixture-/Reference-Slice:** synthetische öffentliche Fixtures sowie
    optionaler privater Bayreuth-Referenztest nach Lizenz- und
    Provenanceprüfung.
17. **CLI-Slice:** optionaler Import und maschinenlesbarer Bericht ohne GUI.

**Definition of Done für die Implementierungsslices:** GLB- und CSV-Importer,
Neuimport, Entfernung, Mapping und Acoustic Bridge sind ohne GUI gegen
synthetische Referenzdateien automatisiert testbar. Die Tests decken atomare
Fehlerwirkung, Ressourcenlimits, Performanceziel, passive Energiebilanz,
Identität und Konfliktfälle ab. Diese Testarchitektur ist keine
Lastenheftanforderung.

## 16. Abnahme des Change Requests

Der CR gilt als fachlich angenommen, wenn der Projektinhaber bestätigt:

1. GLB wird als nichtparametrisches Oberflächenmodell importiert.
2. `.gltf` mit externen Ressourcen bleibt außerhalb des initialen Umfangs.
3. Importierte Meshes werden nicht automatisch in b-cad-Bauteile umgewandelt.
4. Direkter SKP-Import bleibt außerhalb von b-cad.
5. CSV wird als initiales Tabellenformat für akustische Kennwerte unterstützt.
6. XLS/XLSX muss zunächst extern nach CSV konvertiert werden.
7. Visuelle und akustische Materialien bleiben getrennte Konzepte.
8. Die Acoustic Bridge erfordert eine ausdrückliche manuelle Klassifikation.
   Sie setzt den normativ abgeschlossenen, produktionscodefreien Slice 039b und
   die implementierten CR-001-Slices für Identity Foundation, akustisches
   Materialmodell sowie Boundary/Opening/Portal voraus.
9. Materialzuordnungen benötigen eine bestätigte Mappingebene.
10. Projektspezifische und kalibrierte Daten bleiben entsprechend gekennzeichnet.
11. GLB und CSV sind ausschließlich b-cad-Eingabeformate; dieser CR setzt kein
    b-cad–a-ray-Exportformat voraus und legt keines fest.
12. Öffentliche Regressionstests verwenden synthetische Fixtures; ein realer
    Bayreuth-Datensatz wird nur bei dokumentierter Lizenz, Weitergaberecht und
    Konverter-Provenance als optionales Referenzpaket verwendet.

## 17. Entscheidungsprotokoll

| Entscheidung | Status |
|---|---|
| Eigenständiges `ImportedSurfaceModel` einführen | Vorgeschlagen |
| GLB als initiales Oberflächen-Importformat | Vorgeschlagen |
| `.gltf` mit externen Ressourcen im initialen Umfang ausschließen | Vorgeschlagen |
| Direkten SKP-Import ausschließen | Vorgeschlagen |
| CSV als initiales Materialtabellenformat | Vorgeschlagen |
| XLS/XLSX extern nach CSV konvertieren | Vorgeschlagen |
| GLB-Materialien nicht automatisch akustisch interpretieren | Vorgeschlagen |
| Manuelle Acoustic Bridge erst nach normativem Slice 039b und implementierten CR-001-Identity-, Material- und Boundary/Opening/Portal-Slices einführen | Vorgeschlagen |
| Mappingprofile persistent speichern | Vorgeschlagen |
| Öffentliche synthetische Fixtures; Bayreuth nur optional nach Rechteprüfung | Vorgeschlagen |
| GLB/CSV ausschließlich als Eingabeformate behandeln und kein Exportformat voraussetzen | Vorgeschlagen |
````

## 9. Closure-Notiz

<!-- Nach Ausführung: Annahmedatum, endgültige IDs, Versionsstand, Folgeplan, Review- und Gate-Belege. -->
