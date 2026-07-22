---
id: slice-039a
titel: Akustik-CR 001 — Annahmebaseline und normative Lastenheft-Übernahme
status: open
welle: unzugeordnet (Akustik-Folgewelle; fachlich angenommen, noch nicht priorisiert)
lastenheft_refs: []  # Anforderungen entstehen erst in diesem Slice; endgültige IDs werden beim Schreibakt vergeben.
adr_refs: []         # Grundsatz-/Mechanik-ADRs folgen im b-cad-Spec-/ADR-Slice.
---

# Slice 039a: Akustik-CR 001 — Annahmebaseline und normative Übernahme

**Status:** open — fachlich autorisiert, aber nicht priorisiert. Vor einem
Implementierungs-Start ist das unabhängige
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
einzuarbeiten; HIGH-Findings blockieren.

**Welle:** unzugeordnet. Der fachlich angenommene Change Request begründet eine
Akustik-Folgewelle, zieht aber keine Priorisierung gegenüber der aktiven
[`welle-5-erweiterung`](../in-progress/roadmap.md#aktuelle-welle) nach sich.

**Annahmegrundlage:** Der Projektinhaber hat am 2026-07-22 die in diesem Plan
zusammengefasste Produkterweiterung fachlich angenommen. Der vorausgegangene
Arbeitsentwurf lag nur temporär außerhalb dieses Repositories; er ist weder
verlinkbare Quelle noch dauerhafte Abhängigkeit dieses Plans. Die Annahme ist
noch nicht normativ; Rang 1 wird erst durch diesen Slice in
[`spec/lastenheft.md`](../../../../spec/lastenheft.md) hergestellt.

**Autor:** Codex im Auftrag des Projektinhabers. **Datum:** 2026-07-22.

**Schnitt-Herkunft:** dauerhafte Annahmebaseline und erster normativer Schritt
der angenommenen Umsetzungsfolge. Dieser Slice bewahrt den vollständig
angenommenen Inhalt im Repo und übernimmt davon ausschließlich das
Produktversprechen: Scope, Rolle, Ziel, lösungsfreie Anforderungen und
Qualitätsanforderungen sowie die formale ACO-Bereichsdeklaration. Technische
Identity-, Split-/Merge-, Grenzflächen-, Portal-, Energie-, Schema- und
Austauschformatmechanik wird hier vollständig inventarisiert, aber erst in
einem eigenen b-cad-Spec-/ADR-Slice normativ entschieden. Kein Produktionscode,
kein Persistenzschema und kein Austauschformat werden hier festgelegt.

---

## 1. Ziel

Den fachlich angenommenen Akustik-CR in den abnahmebindenden b-cad-Vertrag
übernehmen, ohne seine technische Mechanik in das Lastenheft zu ziehen. Danach
sind der eng begrenzte Akustik-Scope, die neue Rolle und das Projektziel, zehn
funktionale ACO-Anforderungen und vier Qualitätsanforderungen mit endgültigen,
kollisionsfreien IDs kanonisch definiert. Der technische Folgeslice kann diese
Anforderungen anschließend in Spezifikation und ADRs präzisieren.

### 1.1 Fachlich angenommene Leitplanken

Diese Leitplanken dokumentieren die Annahme selbsttragend, ohne den temporären
Arbeitsentwurf als Quelle vorauszusetzen:

1. Das Akustik-Fachmodul darf Innenräume außerhalb des
   Wohngebäudeschwerpunkts unterstützen, verspricht aber keine vollständige
   Nichtwohngebäudeplanung.
2. Raumseitige akustische Oberflächen sind eigenständige fachliche Entitäten;
   konstruktive und akustische Materialien bleiben getrennt.
3. b-cad verwaltet weder Schallquellen und Empfänger noch Simulationen oder
   Ergebnisse. a-ray ist primärer, aber nicht exklusiver Exportverbraucher;
   Ergebnisimport benötigt eine eigene spätere Produktentscheidung.
4. Der Austauschvertrag soll ausschließlich in einer gemeinsam gepflegten,
   produktneutralen und eigenständig versionierten Formatspezifikation
   normativ werden. Kein beteiligtes Produkt darf ihn einseitig festlegen.
5. Für den Benutzer muss eine gemeinsame Grenze je angrenzendem Raum getrennt
   akustisch belegbar sein, ohne im Export eine geometrische Doppelbelegung zu
   erzeugen.
6. Eine leere Öffnung ist eigenständig anlegbar, bearbeitbar, löschbar und
   persistent. Sie bildet ein Portal; bewegliche Tür- und Fensterzustände sind
   nicht Teil der angenommenen Erweiterung.
7. Ein Portal zu einem nicht ausgewählten Nachbarraum blockiert den Export,
   bis dieser Nachbarraum ausgewählt ist. Künstliche Abschlussbedingungen sind
   nicht Teil der Erweiterung.
8. Raum-, Oberflächen- und Patch-Zuordnungen müssen über Speichern, Laden und
   fachlich unveränderte Neuerkennung stabil bleiben; unsichere Zuordnungen
   dürfen nicht still übernommen werden.
9. Frequenzkennlinien bleiben erweiterbar und werden nicht auf eine feste
   Oktavbandliste beschränkt. Absorption, Transmission, Reflexion und Streuung
   müssen gemeinsam eine passive Energiebilanz ohne Energieerzeugung bilden.
10. Die normative und technische Übernahme erfolgt in getrennten
    Spezifikations- und Implementierungsslices; die fachliche Annahme allein
    autorisiert noch keine Implementierung. Repräsentation gemeinsamer
    Grenzen sowie Identity-, Split-/Merge- und Persistenzmechanik bleiben dem
    b-cad-Spec-/ADR-Slice vorbehalten.

## 2. Selbsttragende Vertragsmatrix für die Übernahme

Die Matrix ist der vollständige Inhalts-Oracle des Lastenheft-Slices. Die beim
Schreibakt vergebenen IDs ersetzen die Platzhalter; der beobachtbare Vertrag
darf dabei weder entfallen noch um technische Mechanik erweitert werden.

| ACO | Beschreibung und positiver Vertrag | Grenzen und negative Verträge |
|---|---|---|
| 01 — persistente fachliche Identitäten | Räume, raumseitige Oberflächen und benutzerdefinierte Patches bleiben bei unverändertem Projekt über Speichern, Laden und erneute Raumerkennung mit ihren Zuweisungen identisch. | Bei Teilung/Vereinigung bleiben unveränderte Entitäten referenzierbar und nur tatsächlich betroffene Zuweisungen werden sichtbar zur Klärung markiert. Eine unsichere Altzuordnung wird nie still auf eine andere Entität übertragen. |
| 02 — akustische Materialien | Akustische Materialien lassen sich mit frei erweiterbaren, mindestens oktavbandfähigen Frequenzstützstellen und Herkunfts-/Qualitätsangaben anlegen, bearbeiten, kopieren, löschen und verlustfrei speichern/laden. | Frequenzen sind positiv und streng aufsteigend; Einzelkoeffizienten liegen einschließlich der Randwerte in `0..1`, sofern die gemeinsame passive Energiebilanz erfüllt bleibt. Ungültiges Speichern verändert den Bestand nicht. Zugewiesene Materialien sind bis zur expliziten Neu- oder Nichtbelegung aller Referenzen löschgeschützt. |
| 03 — raumseitige Begrenzungsflächen | Boden, Decke, Wände sowie vorhandene Türen, Fenster und Öffnungen sind je erkanntem Raum einzeln auswählbar und eindeutig zugeordnet; beide Seiten einer gemeinsamen Wand können verschieden belegt werden. | Einbauteile ersetzen ihren Ausschnitt ohne Loch oder Doppelbelegung. Offene oder ungültige Räume werden nicht als geschlossen ausgewiesen; die Ursache bleibt sichtbar. |
| 04 — leere Öffnung | Eine dauerhaft freie Wandöffnung zwischen Räumen oder zum Außenbereich lässt sich anlegen, in Lage/Abmessung bearbeiten, löschen und speichern/laden. Änderungen aktualisieren die sichtbare offene Verbindung; Löschen schließt die Wand wieder. | Ungültige Lage oder Abmessung wird ohne Modelländerung abgelehnt. Außenverbindungen werden als offen ausgewiesen. Bewegliche Offen-/Geschlossen-Zustände von Türen und Fenstern sind ausgeschlossen. |
| 05 — Materialzuweisung | Ein akustisches Material lässt sich einer auswählbaren Raumseite zuweisen; Anzeige und Zuordnung bleiben nach Speichern/Laden erhalten. | Die Änderung einer Wandseite lässt die Gegenseite unverändert. Nicht existente oder keinem Raum zugeordnete Oberflächen werden ohne verwaiste Referenz abgelehnt. |
| 06 — akustische Teilfläche | Eine vollständig innerhalb ihrer Wirtsfläche liegende polygonale Teilfläche überschreibt dort nach Materialzuweisung die Grundbelegung; eine optionale Rolle ist nur beschreibend. | Randberührung ist zulässig, Innenüberschneidung zwischen Patches sowie mit Tür-, Fenster- oder leeren Öffnungen nicht. Ungültige Geometrie wird ohne Änderung abgelehnt. Ein unbelegter Patch darf gespeichert werden, blockiert aber den Export. |
| 07 — Raumhüllenprüfung | Eine geschlossene, vollständig belegte Raumhülle wird als gültig gemeldet; explizite Öffnungen und transmissive Elemente werden als solche ausgewiesen. | Fehlende/unbelegte Flächen, nicht mannigfaltige Kanten, inkonsistente Orientierung, Überschneidungen und Doppelbelegung verhindern ein positives Gesamtergebnis und werden betroffenen Oberflächen zugeordnet. Fehler, Warnungen und Hinweise bleiben unterscheidbar. |
| 08 — solverneutraler Szenenexport | Ein unabhängiger Importer kann aus einem erfolgreichen, versionierten Export Raumhülle, räumlichen Bezug, stabile fachliche Beziehungen, Materialeigenschaften und Provenienz ohne interne b-cad-Daten rekonstruieren. | Mehrraumgrenzen und verschieden belegte Seiten bleiben ohne deckungsgleiche Gegenflächen unterscheidbar; Einbauteile und leere Öffnungen bleiben korrekt. Bei ungültigem Modell oder unbeschreibbarem Ziel entsteht kein als erfolgreich gemeldeter Teil-Export, und ein vorhandener gültiger Export bleibt unverändert. |
| 09 — Exportauswahl | Einzelne Räume und zusammenhängende Raumgruppen sind auswählbar; der Export enthält nur deren Hüllen und benötigte Referenzdaten. Übergänge innerhalb einer ausgewählten Gruppe bleiben erhalten. | An massiven Auswahlgrenzen bleibt die nicht exportierte Nachbarschaft erkennbar. Ein Portal zu einem nicht ausgewählten Nachbarraum blockiert bis zu dessen Auswahl. Eine Auswahl ohne gültigen Raum wird mit Ursache abgelehnt. |
| 10 — Exportvorprüfung | Vor dem Export werden Umfang, Oberflächen/Patches, leere Öffnungen an Auswahlgrenzen, fehlende Belegungen, Grenzfehler, Doppel-/Mehrdeutigkeiten und verwendete Formatversion zusammengefasst. Ein fehlerfreies Modell ist exportierbar. | Dokumentierte Warnungen erfordern sichtbare Bestätigung. Jeder blockierende Befund verhindert die Erfolgsmeldung als gültiges Szenenpaket. Die Prüfung bleibt produktneutral und darf keine Zustimmung eines konkreten Verbrauchers voraussetzen. |

Bei unverändertem ID-Bestand sind `ROLE-004`,
[`OBJ-006`](../../../../spec/lastenheft.md),
[`LH-FA-ACO-001`](../../../../spec/lastenheft.md) bis
[`LH-FA-ACO-010`](../../../../spec/lastenheft.md) sowie
[`LH-QA-008`](../../../../spec/lastenheft.md) bis
[`LH-QA-011`](../../../../spec/lastenheft.md) die geplanten Kandidaten.
[`LH-QA-007`](../../../../spec/lastenheft.md) bleibt für die in
[`slice-006`](slice-006-drittanbieter-attribution.md) und der vorgeschlagenen
[`ADR-0005`](../../adr/0005-drittanbieter-lizenz-attribution.md) koordinierte
Attributionspflicht frei. Dies ist keine Reservierung: Der Live-Audit kann die
Kandidaten vor dem atomaren Schreibakt nur auf einen dann freien,
konfliktfreien Satz verschieben.

| QA-Kandidat | Beobachtbares Kriterium | Abnahme-/Messmethode |
|---|---|---|
| [LH-QA-008](../../../../spec/lastenheft.md) — deterministische Oberflächenidentität | Ein unverändertes Projekt behält fachliche Raum-, Oberflächen- und Patch-Kennungen samt Zuweisungen; lokale Änderungen betreffen nur die tatsächlich geänderten Entitäten, unsichere Altbelegungen werden sichtbar. | Referenzprojekt speichern, laden, erneut erkennen und nach reinem Darstellungswechsel vergleichen; anschließend je eine lokale Teilungs-/Vereinigungsänderung prüfen und Kennungs-/Zuweisungsdiff fachlich bewerten. |
| [LH-QA-009](../../../../spec/lastenheft.md) — reproduzierbarer Export | Identischer Projektstand, Auswahl und Formatversion ergeben semantisch denselben Export; nichtfachliche Metadaten wie Zeitstempel ändern die fachliche Gleichheit nicht. | Zwei unabhängige Exporte derselben Eingabe durch einen formatkonformen Vergleich normalisieren und fachlich vergleichen; anschließend Auswahl oder Formatversion gezielt ändern und einen sichtbaren Unterschied nachweisen. |
| [LH-QA-010](../../../../spec/lastenheft.md) — rückwärtskompatibler Projektbestand | Projekte ohne Akustikdaten bleiben ladbar, bearbeitbar und speicherbar; die Erweiterung führt für sie keine Pflichtdaten ein. | Ein vor Einführung der Funktion erzeugtes Referenzprojekt öffnen, eine bestehende Standardänderung vornehmen, speichern und erneut laden; sämtliche bisherigen Inhalte und Standardfunktionen prüfen. |
| [LH-QA-011](../../../../spec/lastenheft.md) — optionale Fachfunktion | Normaler Start, Grundmodellierung und bestehende Standardexporte funktionieren ohne a-ray, CUDA, OptiX oder GPU. | Anwendung in einer Umgebung ohne diese Komponenten starten und einen repräsentativen Grundmodellierungs- und Standardexport-Ablauf erfolgreich ausführen. |

## 3. Definition of Done

- [ ] **Live-ID-Audit vor dem Schreibakt:** Rollen-, Ziel-, Funktions- und
      Qualitäts-IDs gegen das dann aktuelle
      [`spec/lastenheft.md`](../../../../spec/lastenheft.md) sowie alle Pläne in
      `open/`, `next/` und `in-progress/` sowie vorgeschlagene ADR-Folgepflichten
      prüfen. Plan-IDs sind keine normative
      Reservierung; Konflikte werden vor der Vergabe sichtbar aufgelöst. Der
      koordinierte Kandidat
      [`LH-QA-007`](../../../../spec/lastenheft.md) für Attribution bleibt bei
      unverändertem Stand frei; die Akustik-QA verwenden dann
      [`LH-QA-008`](../../../../spec/lastenheft.md) bis
      [`LH-QA-011`](../../../../spec/lastenheft.md). Endgültige IDs entstehen
      atomar mit dem Lastenhefttext.

- [ ] **Scope, Rolle und Ziel normativ übernehmen:**
      [`spec/lastenheft.md`](../../../../spec/lastenheft.md) §1 bleibt auf
      Wohngebäude fokussiert und erlaubt dem Akustik-Fachmodul ausschließlich
      für seinen Zweck auch Innenräume außerhalb dieses Schwerpunkts; kein
      Versprechen vollständiger Nichtwohngebäudeplanung. §2 erhält die Rolle
      Akustikplaner, §3 das fachplanungsfähige Oberflächenmodell — jeweils mit
      der beim Live-Audit vergebenen endgültigen ID.

- [ ] **Produktgrenze und Nichtumfang normativ sichtbar machen:** Die
      beobachtbare Verantwortungsgrenze aus §1.1 und §9 wird im Lastenheft
      festgehalten. Insbesondere verwaltet b-cad keine Schallquellen, Empfänger,
      Solver-/GPU-Konfiguration, Simulationsergebnisse, daraus berechneten
      akustischen Raumkennwerte oder Auralisation. a-ray bleibt primärer, aber
      nicht exklusiver
      Exportverbraucher. Ergebnisimport, bewegliche Tür-/Fensterzustände,
      künstliche Portalabschlüsse und vollständige Nichtwohngebäudeplanung
      bleiben ausdrücklich außerhalb dieser Produkterweiterung.

- [ ] **ACO-Familie als Produktvertrag:** neues Lastenheft-Modul mit zehn
      fortlaufenden funktionalen Anforderungen (`LH-FA-ACO-<NNN>`, endgültige
      Nummern beim Schreibakt) gemäß §2:
      1. persistente fachliche Identitäten,
      2. akustische Materialien,
      3. raumseitige Begrenzungsflächen,
      4. leere Öffnungen,
      5. Materialzuweisung je Raumseite,
      6. akustische Teilflächen,
      7. Hüllenvalidierung,
      8. solverneutraler Szenenexport,
      9. Auswahl des Exportumfangs,
      10. Exportvorprüfung.
      Die Akzeptanzkriterien übernehmen die vollständigen
      Happy-/Boundary-/Negative-Verträge aus §2 einschließlich Referenzschutz,
      Portal-Auswahlgrenze, Patch-Materialpflicht und atomarem Fehlerverhalten.

- [ ] **Lösungsfreiheit des Lastenhefts:** die ACO-AK bleiben
      benutzer-beobachtbar gemäß
      [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei).
      Keine Feldnamen, Formeln, Mesh-Zuordnungen, Statuswerte, Persistenztabellen,
      Portsignaturen, Bibliotheks- oder Containerformate. Zulässig sind das
      beobachtbare Was, vertragliche Wertebereiche und Invarianten wie
      „keine Energieerzeugung“, „keine stille Fehlzuordnung“, „keine
      Doppelbelegung“ und „offene Auswahlgrenze blockiert Export“.

- [ ] **Qualitätsanforderungen normativ übernehmen:** deterministische
      Oberflächenidentität, reproduzierbarer Export, Rückwärtskompatibilität
      bestehender Projekte und optionale Fachfunktion. Die endgültigen
      `LH-QA-<NNN>`-IDs werden erst nach dem Live-Audit als zusammenhängender,
      freier Block vergeben; insbesondere wird keine im offenen Planning
      genannte Nummer still überschrieben.

- [ ] **Begriffe nachziehen:** Glossar um akustisches Material, raumseitige
      Oberfläche, Patch/Teilfläche, leere Öffnung/Portal und gemeinsame
      geometrische Grenze ergänzen — fachlich und technikneutral. Bestehende
      Begriffe „Material“, „Raum“, „Tür“, „Fenster“ und „Öffnung“ dürfen nicht
      widersprüchlich umgedeutet werden.

- [ ] **ACO im ID-Schema deklarieren, ohne
      [MR-002](../../../../harness/conventions.md#mr-002--id-schema-für-b-cad)
      umzuschreiben:**
      [`harness/conventions.md`](../../../../harness/conventions.md) erhält einen
      neuen, chronologisch nächsten MR-Eintrag, der das Bereichskürzel `ACO`
      ergänzt und [MR-002](../../../../harness/conventions.md#mr-002--id-schema-für-b-cad)
      ausschließlich erweitert. Die bestehenden immutable MR-Einträge bleiben
      inhaltlich unverändert. `.d-check.yml` braucht voraussichtlich keine
      Änderung, weil dessen funktionale ID-Regel Bereichskürzel generisch
      akzeptiert; das wird im Slice verifiziert und nur bei realem Fallout
      korrigiert.

- [ ] **Version und Provenance atomar nachziehen:** Header-Version des
      Lastenhefts und jüngste Zeile in
      [`spec/lastenheft-historie.md`](../../../../spec/lastenheft-historie.md)
      werden auf dieselbe, beim Ausführungszeitpunkt nächste freie Version
      gesetzt ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/
      [MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)).
      Die Historie nennt die fachliche Annahme vom 2026-07-22 als Herkunft,
      ohne einen externen Hostpfad oder eine nicht getrackte Linkquelle in den
      Doku-Korpus einzuführen.

- [ ] **Strata-Grenze sichtbar halten:** `spec/spezifikation.md`,
      `spec/architecture.md`, `spec/data-model.yaml`, ADRs und Produktionscode
      bleiben in diesem Slice unverändert. Offene technische Entscheidungen
      werden nicht als entschieden behauptet. Der nachfolgende
      b-cad-Spec-/ADR-Slice beginnt erst nach Abschluss dieses Vertrags-Slices.

- [ ] **Semantische Closure-Evidenz:** Der Closure-Abschnitt enthält eine
      Matrix der tatsächlich vergebenen Rollen-, Ziel-, ACO- und QA-IDs gegen
      sämtliche Zeilen aus §2 sowie eine manuelle Prüfliste für Scope,
      Anforderungsanzahl, Glossarverträglichkeit,
      [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Lösungsfreiheit
      und die
      [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Versionsinvariante.
      Ein unabhängiges Text-/Diff-Review prüft den normativen Entwurf gegen
      §1.1, §2, das vollständige Routing in §9 und diese Evidenz;
      HIGH-Findings blockieren die Closure.

- [ ] **Sensoren:** `make docs-check` als engster Sensor; danach `make gates`
      vollständig grün. `make schema-check` ist nicht erforderlich, solange
      `spec/data-model.yaml` und `schema.sql` unberührt bleiben. Kein
      Erfolgshinweis ohne den vollständigen Gate-Lauf.

## 4. Plan (vor Ausführung)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | Scope, Rolle, Ziel, ACO-Modul mit zehn funktionalen Anforderungen, vier QA-Anforderungen, Glossar; endgültige IDs erst beim Schreibakt; Header-Version nachziehen |
| `spec/lastenheft-historie.md` | ändern | neue oberste Provenance-Zeile, versionsgleich zum Lastenheft-Header |
| `harness/conventions.md` | ändern | neuer MR-Eintrag: ACO ergänzt das Bereichsschema; [MR-002](../../../../harness/conventions.md#mr-002--id-schema-für-b-cad) bleibt immutable |
| `.d-check.yml` | voraussichtlich unverändert | generische Bereichsregex gegen ACO verifizieren; Änderung nur bei reproduziertem Fallout |
| `docs/reviews/2026-07-22-slice-039a-plan.md` | neu | unabhängiger [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review-Report |
| `docs/reviews/{Ausführungsdatum}-slice-039a-normative-diff.md` | neu | unabhängiges Text-/Diff-Review gegen §1.1, §2, §9 und die Closure-Evidenz |
| `docs/plan/planning/open/slice-039b-akustik-spezifikation-und-adrs.md` | neu | ein selbsttragender Folgeslice übernimmt das gesamte deferierte technische Routing aus §9 |

## 5. Trigger und Abhängigkeiten

- **Erfüllt:** Die in §1.1 festgehaltene Produkterweiterung ist seit
  2026-07-22 durch den Projektinhaber fachlich angenommen.
- **Vor Aktivierung:** Wellen-/Priorisierungsentscheidung des Projektinhabers;
  `open` bedeutet keine Umsetzungszusage
  ([Planning-Lifecycle](../README.md#lifecycle-bedeutungen)). Die Einplanung
  weist eine Welle zu und bewegt den Slice per reinem `git mv` von `open/` nach
  `next/`. Der spätere Implementierungs-Start ist ein eigener reiner
  `git mv`-Schritt von `next/` nach `in-progress/`; nur dabei wird — sofern es
  weiterhin der erste aktive Slice ist — der Ruhemarker in der Roadmap im
  selben Move-Commit entfernt.
- **Vor Implementierungs-Start:** unabhängiges
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review,
  alle HIGHs behoben.
- **Kein technischer Vorläufer:** Der Slice ist reine Vertragsübernahme. Seine
  Closure ist Vorbedingung für den b-cad-Spec-/ADR-Slice.

## 6. Closure-Trigger und Folgefolge

- DoD vollständig; endgültige IDs kollisionsfrei; Lastenheft-Header == jüngste
  Historie-Version; `make gates` grün; Closure-Notiz ergänzt.
- Danach wird der **b-cad-Spec-/ADR-Slice** plan- und startbar: technische
  Identity-/Split-/Merge-Regeln, gemeinsame Grenzflächen und Portale,
  Energie-/Toleranzregeln sowie die neutrale Naht zwischen Geometrieerzeugung
  und Export. Er darf den gemeinsamen produktneutralen Austauschvertrag nicht
  einseitig festlegen.
- Erst nach diesem technischen Entscheidungs-Slice folgt die
  Identity-Foundation-Implementierung; der Export bleibt zusätzlich von einer
  konkret versionierten gemeinsamen Austauschformatspezifikation abhängig.

## 7. Risiken und offene Punkte

- **ID-Kollision / Parallelplanung:** [`slice-006`](slice-006-drittanbieter-attribution.md)
  und die vorgeschlagene
  [`ADR-0005`](../../adr/0005-drittanbieter-lizenz-attribution.md) koordinieren
  [`LH-QA-007`](../../../../spec/lastenheft.md), ohne dass diese ID bereits
  normativ ist. Mitigation: bei unverändertem Stand Akustik-QA
  [`LH-QA-008`](../../../../spec/lastenheft.md) bis
  [`LH-QA-011`](../../../../spec/lastenheft.md); Live-Audit und endgültige
  Vergabe erst im Lastenheft-Schreibakt.
- **Vertragsgröße:** zehn funktionale plus vier Qualitätsanforderungen sind ein
  großer reiner Doku-Schnitt. Vor Aktivierung wird anhand der vollständigen
  Matrix in §2 verbindlich geprüft, ob ein unabhängiger Reviewer den Diff in
  einer Sitzung vollständig abgleichen kann. Andernfalls wird in mindestens
  „Scope/Identity/Material/Oberflächen“ und „Validierung/Export/QA“ geteilt;
  jeder Teilschnitt bleibt kanonisch selbsttragend, und es gibt keine
  Teilübernahme mit halb gültigen Querverweisen.
- **Lösungsmechanik schwappt zurück:** Die angenommene Erweiterung umfasst auch
  technische Leitplanken für Folgeslices. Beim Transfer in das Lastenheft
  werden ausschließlich benutzer-beobachtbare Produktanforderungen formuliert;
  Feldnamen, Formeln und Datenstrukturen gehen in den Folgeslice.
- **Externe Vertragsquelle noch nicht versioniert:** Die gemeinsame
  produktneutrale Austauschformatspezifikation existiert noch nicht als
  referenzierbarer b-cad-Vertrag. Der Lastenhefttext darf deshalb nur
  Produktanforderungen an Erzeugung und Beobachtbarkeit festlegen.
- **Aktive Welle bleibt maßgeblich:** Die Annahme darf die noch offene
  `welle-5-erweiterung` oder deren DRW-Folgearbeit nicht still verdrängen.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Produktvertrag / Lastenheft

- **Modus:** GF für das neue ACO-Modul, Brownfield für §1–§3 und das Glossar.
- **Konventionen-Dichte:** hoch — Source Precedence,
  [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Lösungsfreiheit,
  ID-Schema,
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Versionsinvariante
  und bestehende Begriffsbedeutungen.
- **Phase-Reife:** ACO Phase 0 → Phase 2/AK; bestehendes Lastenheft bleibt Draft
  mit inkrementell geschärften Modulen.
- **Evidenz-/Diskrepanz-Risiko:** mittel — die selbsttragend dokumentierte
  fachliche Annahme trifft auf einen bereits dichten kanonischen Vertrag und
  eine konkurrierende offene QA-Planung.
- **Reconciliation-Aufwand:** Live-ID-Audit, terminologische Einpassung und
  strikte Trennung von Produktversprechen und technischer Mechanik.

### Sub-Area: Harness-ID-Schema

- **Modus:** Brownfield.
- **Konventionen-Dichte:** hoch —
  [MR-002](../../../../harness/conventions.md#mr-002--id-schema-für-b-cad) ist
  etabliert und als MR-Eintrag immutable; Erweiterungen erfolgen additiv durch
  einen neuen MR.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig, sofern ACO additiv bleibt und die
  generische d-check-Regex ohne Konfigurationsänderung greift.
- **Reconciliation-Aufwand:** ein additiver MR-Eintrag; keine Umschreibung der
  Historie.

## 9. Vollständiges normatives Übernahmeregister

Die fachlich angenommene Vollbaseline steht in Anhang A. Dieses Register ordnet
jeden ihrer Abschnitte einem dauerhaften Zielstratum zu. „Später“ bedeutet
nicht „verworfen“; der jeweilige Folgeslice muss den bezeichneten Inhalt aus
Anhang A vollständig abgleichen und seine Abweichungen begründen. Das gesamte
deferierte technische Routing übernimmt zunächst der selbsttragende Folgeplan
[`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md).

| Baseline-Abschnitt | Inhalt | Normatives Ziel / Behandlung |
|---|---|---|
| 1–2 | Problem, Nutzen, Systemgrenze | Produktkontext für Lastenheft; technische Motivation für [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md) |
| 3.1 | eng begrenzte Scope-Erweiterung | dieser Slice: `spec/lastenheft.md` §1 |
| 3.2 | Verantwortung b-cad / a-ray | beobachtbare Produktgrenze in diesem Slice; technische Port-/Komponentengrenzen erst in [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md) |
| 4 | Ziele und Nutzen | Rolle, Ziel und ACO-Modul dieses Slice; keine Solverfunktion in b-cad |
| 5.1–5.3 | Rolle, Ziel, ACO-Kürzel | dieser Slice: Lastenheft plus additiver MR-Eintrag |
| 5.4 | zehn funktionale Verträge | dieser Slice lösungsfrei gemäß §2; sämtliche Mechanik aus Beschreibung/AK wird in [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md) nachgeführt |
| 6 | vier Qualitätsverträge | dieser Slice mit den kollisionsfreien Kandidaten aus §2 und messbaren AK |
| 7.1–7.3 | fachliches Modell, Identität, Mehrraumgrenzen/Öffnungen | [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md); danach Identity-Foundation und Boundary-/Opening-Implementierung |
| 7.4–7.7 | Materialien, Patches, Hülle, Exportauswahl | [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md); danach getrennte Material-, Patch- und Validierungsslices |
| 8 | Architektur- und Integrationsgrenzen | [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md) und erforderliche neue ADRs; Architektur-Sicht nur bei tatsächlich neuer stabiler Komponentengrenze und ohne Abwärtsreferenzen |
| 9.1–9.5 | Produktneutraler Austauschvertrag, Felder, Mehrraum-/Portal- und Energiesemantik, Versionierung | in [`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md) ausschließlich an einen gemeinsamen Vertragsslice routen; nicht einseitig in b-cad normieren |
| 10 | Nichtumfang | Ausschlüsse im Lastenheft sichtbar halten; neue Funktionen nur über neue Produktentscheidung |
| 11 | Risiken R-1 bis R-8 | in die Risikoregister der jeweiligen Folgepläne übernehmen und bei Closure einzeln disponieren |
| 12 | Reihenfolge der Umsetzung | verbindliche Planungsabhängigkeiten: Lastenheft → b-cad Spec/ADR → Identity Foundation; gemeinsamer Vertrag vor Export; weitere Fachslices gemäß Anhang A |
| 13–14 | fachliche Annahme und Entscheidungsprotokoll | dauerhaft durch §1.1, §2, dieses Register und Anhang A belegt; kein Rückverweis auf den temporären Ablageort |

Vor Closure dieses Slice wird der konkret benannte
[`slice-039b`](slice-039b-akustik-spezifikation-und-adrs.md) als
selbsttragender offener Folgeplan angelegt und aus jeder deferierten
Tabellenzeile heraus verlinkt. Er
übernimmt das vollständige technische Routing und plant seinerseits die
getrennten Implementierungs- und gemeinsamen Vertragsslices. Dadurch bleibt
kein angenommener Inhalt lediglich im Anhang liegen, ohne den Dateiscope dieses
Vertrags-Slices um eine unbestimmte Menge von Plänen zu erweitern.

## 10. Anhang A — fachlich angenommene Vollbaseline

Der folgende Snapshot ist die vollständige, am 2026-07-22 fachlich angenommene
Arbeitsfassung. Er ist innerhalb dieses Repo-Dokuments eingebettet und benötigt
weder einen Link noch den Fortbestand des temporären Ablageorts. Der Snapshot
ist noch keine normative Spezifikation; bei der Übernahme gelten die
Zielstrata und Prüfpflichten aus §2 und §9. Die äußere Vierer-Fence bewahrt den
Markdown-Inhalt vollständig; lediglich die nichtsemantischen nachgestellten
Leerzeichen der Metadatenzeilen wurden entfernt. Die vorläufigen IDs werden
dadurch nicht als bereits kanonische Repo-Referenzen ausgegeben.

````markdown
# Change Request 001 — Akustisches Oberflächen- und Austauschmodell

**Produkt:** b-cad
**Status:** Fachlich angenommen
**Art:** Scope- und Funktionsumfangserweiterung
**Datum:** 2026-07-21
**Letzte Überarbeitung:** 2026-07-22
**Fachlich angenommen:** 2026-07-22 durch den Projektinhaber
**Primärer Verbraucher:** a-ray
**Normative Wirkung:** Keine. Sie entsteht erst durch die Übernahme in die
normativen Spezifikationen.

## 1. Zusammenfassung

b-cad soll um ein akustisches Oberflächen- und Austauschmodell erweitert werden.

Der Benutzer soll raumseitige Begrenzungsflächen eines Gebäudemodells einzeln
akustisch beschreiben, mit frequenzabhängigen Materialkennwerten belegen,
Teilflächen wie Absorber oder Diffusoren modellieren, die daraus entstehende
Raumhülle validieren und als versioniertes, solverneutrales Szenenpaket
exportieren können.

Die eigentliche Akustikberechnung wird nicht in b-cad implementiert. Sie erfolgt
im eigenständigen Werkzeug **a-ray**. Quellen, Empfänger, Solverparameter,
Raytracing, Path Tracing, GPU-Beschleunigung, Impulsantworten und akustische
Kennwerte liegen vollständig im Verantwortungsbereich von a-ray.

Der Austauschvertrag darf keine Implementierungsdetails von a-ray, NVIDIA
OptiX, CUDA oder eines bestimmten Simulationsverfahrens voraussetzen.

Die alleinige normative Vertragsquelle für das Austauschformat ist eine
gemeinsam gepflegte, produktneutrale Austauschformatspezifikation. b-cad und
a-ray referenzieren jeweils eine konkrete Version dieser Spezifikation. Weder
das b-cad- noch das a-ray-Lastenheft darf Geometrieformat, Semantik oder
Kompatibilitätsregeln einseitig festlegen.

## 2. Anlass und Problemstellung

Das bisherige b-cad-Modell beschreibt Bauteile und deren konstruktive
Materialien. Für geometrische Raumakustik reicht diese Sicht nicht aus:

1. Akustisch wirksam ist die dem Raum zugewandte Oberfläche, nicht automatisch
   das Kernmaterial eines Bauteils.
2. Die beiden Seiten derselben Wand können unterschiedliche akustische
   Belegungen besitzen.
3. Eine Oberfläche kann in Teilflächen mit verschiedenen Eigenschaften
   unterteilt sein.
4. Absorption und Streuung sind frequenzabhängig.
5. Ein externer Solver benötigt eine geschlossene, triangulierte und
   semantisch referenzierbare Raumhülle.
6. Instabile Raum- oder Flächenkennungen würden persistierte Belegungen und
   externe Ergebnisse unzuverlässig machen.

b-cad besitzt bereits parametrische Bauteilgeometrie, Räume, Materialverwaltung,
Persistenz, 2D-/3D-Darstellung und Exportadapter. Diese Grundlagen sollen
weiterverwendet werden, ohne den CAD-Kern mit Simulations- oder
GPU-Abhängigkeiten zu belasten.

## 3. Beantragte Produktentscheidung

### 3.1 Geltungsbereich

Der allgemeine Zweck von b-cad und der bestehende Schwerpunkt auf
Wohngebäuden bleiben unverändert. Das Akustik-Fachmodul darf auch Innenräume
außerhalb dieses Schwerpunkts für externe Akustikberechnungen vorbereiten.
Damit ist kein Anspruch auf vollständige Planung beliebiger Nichtwohngebäude
oder Gebäudetypen verbunden.

Empfohlene Änderung in `spec/lastenheft.md`, Abschnitt 1:

> b-cad ist eine Desktop-Anwendung zur Erstellung, Bearbeitung, Analyse und
> Visualisierung parametrischer Wohngebäude. Fachmodule können für ihren
> jeweiligen Zweck auch Innenräume außerhalb dieses Schwerpunkts unterstützen,
> ohne damit vollständige Planung beliebiger Nichtwohngebäude zu versprechen.

### 3.2 Systemgrenze zwischen b-cad und a-ray

Die Produktverantwortung wird wie folgt getrennt:

| Verantwortungsbereich | b-cad | a-ray |
|---|:---:|:---:|
| Parametrische Gebäudegeometrie | Ja | Nein |
| Raum- und Bauteilmodell | Ja | Nein |
| Raumseitige akustische Oberflächen | Ja | Import |
| Akustische Materialkennlinien | Ja | Import und Auswertung |
| Oberflächen-Patches | Ja | Import |
| Validierung der geometrischen Raumhülle | Ja | zusätzliche Importvalidierung |
| Export des akustischen Szenenpakets | Ja | Import |
| Quellen und Empfänger | Nein | Ja |
| Simulationsparameter | Nein | Ja |
| CPU-/GPU-Raytracing | Nein | Ja |
| OptiX/CUDA | Nein | optional |
| Energy-Time-Curves und Impulsantworten | Nein | Ja |
| Akustische Kennwerte | Nein | Ja |
| Auralisation | Nein | später optional |

## 4. Ziele und Nutzen

Der Change Request soll ermöglichen:

- raumseitige Oberflächen getrennt von Bauteilkernen zu beschreiben,
- unterschiedliche Belegungen auf beiden Seiten einer Wand zu führen,
- akustische Teilflächen auf Wänden, Böden und Decken zu modellieren,
- frequenzabhängige Absorptions-, Streu- und optionale
  Transmissionskennwerte zu verwalten,
- Materialdaten mit Herkunfts- und Qualitätsinformationen zu versehen,
- eine geschlossene und validierbare akustische Raumhülle abzuleiten,
- stabile Raum-, Oberflächen- und Patch-Kennungen bereitzustellen,
- eine triangulierte, solverneutrale Szene an a-ray zu übergeben,
- b-cad und a-ray unabhängig entwickeln, testen und versionieren zu können.

## 5. Vorgeschlagene Lastenheft-Erweiterung

**ID-Status:** Alle in diesem Abschnitt genannten `ROLE`-, `OBJ`-, `LH-FA`-
und `LH-QA`-IDs sind vorläufige Bezeichner dieses CR. Die endgültigen IDs
werden erst bei der normativen Übernahme anhand des dann gültigen b-cad-Schemas
vergeben; interne Verweise sind dabei atomar anzupassen.

### 5.1 Neue Rolle

| ID | Rolle | Erwartung |
|---|---|---|
| `ROLE-004` | Akustikplaner | Kann raumseitige Oberflächen akustisch klassifizieren, prüfen und als Berechnungsmodell für externe Werkzeuge bereitstellen. |

### 5.2 Neues Projektziel

| ID | Ziel | Beschreibung |
|---|---|---|
| `OBJ-006` | Fachplanungsfähiges Oberflächenmodell | Raumbegrenzende Flächen und Teilflächen können unabhängig vom Bauteilkern mit fachbezogenen Eigenschaften belegt, validiert und offen exportiert werden. |

### 5.3 Neues Bereichskürzel

Für die neuen funktionalen Anforderungen wird das Bereichskürzel `ACO`
(**Acoustics**) beantragt. Bei Annahme ist das ID-Schema in
`harness/conventions.md` entsprechend zu ergänzen.

### 5.4 Funktionale Anforderungen

#### LH-FA-ACO-001 — Persistente fachliche Identitäten bereitstellen

**Beschreibung:** b-cad verwaltet persistente Kennungen für Räume,
raumseitige Oberflächen und vom Benutzer angelegte Patches. Diese Kennungen
sind Projektbestand und bleiben bei fachlich unveränderten Entitäten über
Speichern, Laden und erneute Raumerkennung stabil.

Diese Anforderung ist Foundation-Voraussetzung für Materialzuweisungen,
Patches, Export und den Abgleich externer Ergebnisse. Sie muss vor der ersten
Materialzuweisung umgesetzt sein.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein unverändertes Projekt mit Räumen, Oberflächen und
  Patches, when es gespeichert, geladen und erneut erkannt wird, then bleiben
  alle fachlichen Kennungen und Zuweisungen identisch.
- **Boundary:** Given eine Oberfläche wird geteilt oder mehrere Oberflächen
  werden vereinigt, then bleiben unveränderte Entitäten stabil referenzierbar
  und tatsächlich betroffene Zuweisungen werden sichtbar zur Klärung markiert.
- **Negative:** Given b-cad kann eine alte Entität nicht sicher eins zu eins
  zuordnen, then wird deren Kennung und Belegung nicht still auf eine andere
  Entität übertragen.

#### LH-FA-ACO-002 — Akustisches Material verwalten

**Beschreibung:** Der Benutzer kann akustische Materialien mit
frequenzabhängigen Oberflächeneigenschaften anlegen, bearbeiten, kopieren und
löschen.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein Projekt, when ein akustisches Material mit gültigen
  Frequenzstützstellen und Koeffizienten gespeichert wird, then steht es für
  Oberflächenzuweisungen zur Verfügung und bleibt nach Speichern/Laden
  vollständig erhalten.
- **Boundary:** Given ein einzelner Koeffizient mit dem Wert `0` oder `1`, then
  wird er akzeptiert, sofern alle kombinierten Invarianten erfüllt sind.
- **Negative:** Given ein Koeffizient außerhalb von `0..1`, eine nicht positive
  Frequenz, nicht streng aufsteigende Frequenzstützstellen oder eine Verletzung
  der passiven Energiebilanz, when gespeichert wird, then wird die Eingabe
  sichtbar abgelehnt; es entsteht kein teilweise gespeicherter Datensatz.
- **Referenzschutz:** Given ein akustisches Material ist mindestens einer
  Oberfläche zugewiesen, when der Benutzer es löschen will, then wird die
  Löschung abgelehnt und die betroffenen Zuweisungen werden angezeigt. Erst
  nach expliziter Neu- oder Nichtbelegung aller Referenzen ist die Löschung
  möglich.

**Mindestdaten je Frequenzstützstelle:**

- Frequenz in Hertz,
- Absorptionsgrad,
- Streugrad,
- optionaler Transmissionsgrad.

**Zusätzliche Metadaten:**

- Bezeichnung,
- Hersteller oder Quelle,
- Referenz oder Datenblatt,
- Mess- oder Schätzverfahren,
- optional Gültigkeits- oder Qualitätsangabe.

b-cad muss mindestens Oktavbanddaten unterstützen. Das Datenmodell und das
Austauschformat dürfen jedoch nicht auf eine feste Liste von Frequenzen
beschränkt sein.

Die Kombination der Kennwerte muss eine passive Energiebilanz ohne
Energieerzeugung beschreiben. Der Streugrad wirkt nur auf reflektierte Energie.

#### LH-FA-ACO-003 — Raumseitige Begrenzungsflächen bereitstellen

**Beschreibung:** Für einen erkannten Raum stellt b-cad die akustisch wirksamen
raumseitigen Begrenzungsflächen von Wänden, Boden, Decke sowie vorhandenen Türen,
Fenstern und Öffnungen einzeln bereit.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein geschlossener Raum, when dessen akustische
  Oberflächen angezeigt werden, then sind Boden, Decke und jede raumseitige
  Wandfläche einzeln auswählbar und dem Raum eindeutig zugeordnet.
- **Boundary:** Given zwei Räume an derselben Wand, then besitzt jeder Raum eine
  eindeutig referenzierbare raumseitige Oberfläche; beide Seiten können
  unterschiedliche akustische Belegungen tragen, ohne im Export geometrisch
  doppelt belegt zu werden.
- **Einbauteil:** Given eine Tür oder ein Fenster in einer Wirtsfläche, then
  schneidet dessen Aussparung die Wirtsfläche exakt aus und eine eigene
  Einbauteiloberfläche füllt diesen Bereich ohne unbeabsichtigtes Loch oder
  Doppelbelegung. Kein Bereich ist zugleich Wirts- und Einbauteiloberfläche.
- **Negative:** Given ein offener oder geometrisch ungültiger Raum, then wird
  keine fälschlich geschlossene akustische Raumhülle ausgewiesen; die Ursache
  wird sichtbar gemeldet.

#### LH-FA-ACO-004 — Leere Öffnung verwalten

**Beschreibung:** Der Benutzer kann in einer Wand eine dauerhaft freie
Öffnung ohne Tür- oder Fensterelement anlegen, bearbeiten und löschen. Sie
bildet eine offene Verbindung zwischen zwei Innenräumen oder zwischen einem
Innenraum und dem Außenbereich.

**Akzeptanzkriterien:**

- **Happy Path:** Given zwei aneinandergrenzende Räume, when der Benutzer eine
  gültige leere Öffnung in ihrer gemeinsamen Wand anlegt, then ist der Bereich
  in beiden Räumen sichtbar offen und bleibt nach Speichern/Laden erhalten.
- **Bearbeiten:** Given eine bestehende leere Öffnung, when Lage oder
  Abmessungen gültig geändert werden, then werden Wanddurchbruch und offene
  Verbindung aktualisiert und bleiben nach Speichern/Laden erhalten.
- **Boundary:** Given eine leere Öffnung grenzt an den Außenbereich, then wird
  sie als offene Außenverbindung erkannt und bei der Hüllenprüfung entsprechend
  ausgewiesen.
- **Löschen:** Given eine bestehende leere Öffnung, when der Benutzer sie
  löscht, then ist die Wand an dieser Stelle wieder geschlossen.
- **Negative:** Given Lage oder Abmessungen der leeren Öffnung sind für die
  Wirtswand ungültig, when sie angelegt oder bearbeitet wird, then wird die
  Änderung sichtbar abgelehnt und das bestehende Modell bleibt unverändert.

Eine geöffnete Tür oder ein geöffnetes Fenster ist in diesem CR kein Ersatz
für die dauerhaft freie Öffnung; bewegliche Offen-/Geschlossen-Zustände sind
nicht Bestandteil dieses CR.

#### LH-FA-ACO-005 — Akustisches Material einer Oberfläche zuweisen

**Beschreibung:** Der Benutzer weist einer raumseitigen Begrenzungsfläche ein
akustisches Material zu.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine auswählbare Raumoberfläche und ein akustisches
  Material, when die Zuweisung erfolgt, then zeigt die Oberfläche die neue
  Belegung und die Zuweisung bleibt nach Speichern/Laden erhalten.
- **Boundary:** Given eine Wand zwischen zwei Räumen, when nur die Seite eines
  Raums geändert wird, then bleibt die gegenüberliegende Seite unverändert.
- **Negative:** Given eine nicht mehr existierende oder keinem Raum
  zugeordnete Oberfläche, then wird die Zuweisung abgelehnt; keine verwaiste
  Referenz wird gespeichert.

#### LH-FA-ACO-006 — Akustische Teilfläche anlegen

**Beschreibung:** Auf einer raumseitigen Begrenzungsfläche kann der Benutzer
polygonale Teilflächen anlegen, ihnen ein akustisches Material zuweisen und sie
optional mit einer beschreibenden Rolle wie Absorber, Reflektor, Diffusor oder
Vorhang kennzeichnen. Die Rolle ersetzt keine Materialzuweisung und besitzt
keine eigene akustische Wirkung.

**Akzeptanzkriterien:**

- **Happy Path:** Given eine Wandinnenfläche, when eine vollständig innerhalb
  liegende Teilfläche angelegt und belegt wird, then überschreibt sie in ihrem
  Bereich die akustische Belegung der Grundfläche.
- **Boundary:** Given eine Teilfläche berührt den Rand der Wirtsfläche, then
  bleibt sie gültig, sofern ihr Inneres die Wirtsfläche nicht überschreitet.
- **Überlappungsregel:** Die Innenbereiche zweier Patches auf derselben
  Wirtsfläche dürfen sich nicht schneiden oder ineinander liegen. Gemeinsame
  Randsegmente und Randpunkte sind zulässig.
- **Öffnungsregel:** Das Innere eines Patches darf das Innere einer Tür-,
  Fenster- oder leeren Öffnung nicht schneiden. Eine Berührung ihrer Kontur
  ist zulässig. Ein Patch verändert niemals die Funktion der Öffnung.
- **Unbelegt:** Given ein Patch besitzt vorübergehend kein akustisches
  Material, then kann das Projekt gespeichert werden, aber die Exportprüfung
  meldet einen blockierenden Fehler.
- **Negative:** Given eine selbstschneidende, außerhalb liegende oder
  nach den beiden vorstehenden Regeln überlappende Teilfläche, then wird sie
  abgelehnt; das bestehende Oberflächenmodell bleibt unverändert.

#### LH-FA-ACO-007 — Akustische Raumhülle validieren

**Beschreibung:** Der Benutzer kann prüfen, ob die aus raumseitigen
Oberflächen gebildete akustische Raumhülle vollständig und konsistent ist.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein vollständig geschlossener Raum mit vollständiger
  Materialbelegung, when die Prüfung ausgeführt wird, then wird die Hülle als
  gültig gemeldet.
- **Boundary:** Given explizite Öffnungen oder transmissive Elemente, then
  werden sie als solche ausgewiesen und nicht still als geschlossene,
  reflektierende Fläche behandelt.
- **Negative:** Given fehlende Flächen, nicht mannigfaltige Kanten,
  inkonsistente Orientierung, Überschneidungen oder unbelegte Bereiche, then
  nennt die Prüfung die betroffenen Oberflächen sichtbar und liefert kein
  positives Gesamtergebnis.

Die Validierung muss zwischen Fehlern, Warnungen und Hinweisen unterscheiden.

#### LH-FA-ACO-008 — Solverneutrales akustisches Szenenpaket exportieren

**Beschreibung:** Der Benutzer kann ein ausgewähltes akustisches Raummodell als
versioniertes, dokumentiertes und solverneutrales Szenenpaket exportieren. Der
primäre Verbraucher ist a-ray.

**Pflichtinhalt des Exports:**

- vollständige und eindeutig orientierte Raumgeometrie,
- Einheit und räumlicher Bezug der Geometrie,
- stabile Identitäten und eindeutige Beziehungen von Räumen, Oberflächen,
  Teilflächen und Öffnungen,
- akustische Materialeigenschaften und beschreibende Oberflächenrollen,
- Herkunfts-, Projekt- und Versionsinformationen.

Die konkrete Repräsentation dieser Inhalte legt ausschließlich die gemeinsame
Austauschformatspezifikation fest.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein gültiges akustisches Raummodell, when exportiert
  wird, then kann ein unabhängiger Importer die vollständige Raumhülle,
  Oberflächenidentitäten und Materialzuweisungen ohne Zugriff auf interne
  b-cad-Datenstrukturen rekonstruieren.
- **Boundary:** Given mehrere Räume oder unterschiedliche Belegungen derselben
  gemeinsamen Grenze, then kann ein unabhängiger Importer beide raumseitigen
  Belegungen eindeutig unterscheiden, ohne deckungsgleiche, entgegengesetzt
  orientierte Flächen zu erhalten.
- **Öffnungen:** Given eine Tür oder ein Fenster, then ersetzt dessen
  Einbauteiloberfläche die ausgeschnittene Wirtsfläche ohne Loch oder
  Doppelbelegung. Given eine leere Öffnung, then bleibt die offene Verbindung
  zu den angrenzenden Räumen oder zum Außenbereich eindeutig erhalten.
- **Negative:** Given ein ungültiges Modell oder ein nicht beschreibbarer
  Zielpfad, then wird kein verwendbarer Teil-Export als erfolgreich gemeldet.
  Ein bereits vorhandener gültiger Export bleibt unverändert.

**Kopplungsregel:** Das Paket bleibt solverneutral und legt keine internen
Implementierungsformate von b-cad oder a-ray offen.

**Hinweis zur Konsumierung:** Oberflächenrollen (`LH-FA-ACO-006`) werden als
beschreibende Herkunftsmerkmale exportiert und tragen keine eigene
akustische Wirkung. Das Verhalten ergibt sich aus den Materialkennwerten gemäß
`LH-FA-ACO-002`. Aus Patches abgeleitete Bereiche bleiben als eigenständige,
stabil referenzierbare Oberflächen erkennbar.

#### LH-FA-ACO-009 — Exportumfang auswählen

**Beschreibung:** Der Benutzer kann festlegen, welche Räume oder
zusammenhängenden Raumgruppen in das akustische Szenenpaket aufgenommen werden.

**Akzeptanzkriterien:**

- **Happy Path:** Given mehrere erkannte Räume, when genau ein Raum ausgewählt
  wird, then enthält der Export nur dessen Raumhülle und direkt benötigte
  Referenzdaten.
- **Boundary:** Given eine zusammenhängende Raumgruppe mit expliziten
  Öffnungen, then bleiben die Übergänge und Raumzuordnungen erhalten.
- **Auswahlgrenze:** Given eine massive Grenzfläche zu einem nicht ausgewählten
  Nachbarraum, then wird nur die Seite des ausgewählten Raums exportiert und
  die nicht exportierte Nachbarschaft bleibt eindeutig erkennbar.
- **Offene Auswahlgrenze:** Given eine leere Öffnung verbindet einen
  ausgewählten mit einem nicht ausgewählten Raum, then ist der Export
  blockiert, bis der Nachbarraum in die Auswahl aufgenommen wird.
- **Negative:** Given eine Auswahl ohne gültigen Raum, then wird der Export
  abgelehnt und die Ursache angezeigt.

#### LH-FA-ACO-010 — Export vorab prüfen

**Beschreibung:** Vor dem Export zeigt b-cad eine zusammengefasste
Exportprüfung an.

**Die Prüfung enthält mindestens:**

- Anzahl Räume,
- Anzahl Oberflächen und Patches,
- Anzahl leerer Öffnungen und davon an der Auswahlgrenze endende Öffnungen,
- fehlende Materialzuweisungen,
- unvollständige oder widersprüchliche Raumgrenzen,
- geometrische Doppelbelegungen und mehrdeutige Zuordnungen,
- verwendete Formatversion.

**Akzeptanzkriterien:**

- **Happy Path:** Given ein gültiges Modell, then meldet die Prüfung keine
  blockierenden Fehler und der Export kann ausgeführt werden.
- **Boundary:** Given ausschließlich dokumentierte Warnungen, then kann der
  Benutzer den Export nach sichtbarer Bestätigung fortsetzen.
- **Negative:** Given ein blockierender Fehler, then ist der Export nicht als
  gültiges akustisches Szenenpaket gemäß gemeinsamer
  Austauschformatspezifikation möglich.

## 6. Qualitätsanforderungen

Die folgenden IDs sind wie in Abschnitt 5 beschrieben vorläufig. Aus dem
vorliegenden CR-Bestand wird keine anderweitige Reservierung von `LH-QA-007`
abgeleitet.

### LH-QA-007 — Deterministische Oberflächenidentität

Bei unveränderter Gebäudegeometrie müssen Speichern/Laden, erneute
Raumerkennung und reine Darstellungswechsel dieselben fachlichen Raum-,
Oberflächen- und Patch-Kennungen sowie Materialzuweisungen ergeben.

Änderungen an einer Wand, Öffnung, Decke oder einem Raum dürfen nicht unbemerkt
zu falsch zugeordneten akustischen Belegungen führen.

**Akzeptanzkriterien:**

- **Happy Path:** Unverändertes Projekt speichern, laden und erneut auswerten
  ergibt identische Oberflächen- und Patch-Zuordnungen.
- **Boundary:** Eine lokale Geometrieänderung ersetzt oder markiert nur
  tatsächlich betroffene Oberflächen.
- **Negative:** Kann eine alte Belegung nicht sicher zugeordnet werden, wird sie
  nicht still auf eine andere Fläche übertragen; der Konflikt wird sichtbar.

### LH-QA-008 — Reproduzierbarer Export

Bei identischem Projektstand, identischer Auswahl und identischer
Exportformatversion muss der fachliche Inhalt des Exports reproduzierbar sein.

Nicht fachlich relevante Unterschiede wie Zeitstempel dürfen die semantische
Gleichheit nicht verändern.

### LH-QA-009 — Rückwärtskompatibler Projektbestand

Projekte ohne Akustikdaten müssen weiterhin geladen, bearbeitet und gespeichert
werden können. Das Akustikmodul darf für bestehende b-cad-Projekte keine
Pflichtdaten einführen.

### LH-QA-010 — Optionale Fachfunktion

Akustische Funktionen dürfen den normalen b-cad-Start, die Grundmodellierung
und Standardexporte nicht von a-ray, CUDA, OptiX oder einer GPU abhängig machen.

## 7. Fachliche Regeln für die technische Spezifikation

Bei Annahme sind in `spec/spezifikation.md` mindestens folgende Regeln zu
entscheiden:

1. unterstützte Frequenzstützstellen und Interpolationsarten,
2. Wertebereiche und Energiebilanz von Absorption, Streuung, Transmission und
   Reflexion,
3. Behandlung fehlender Werte und Extrapolation,
4. Priorität und flächige Partitionierung von Wirtsflächen, Einbauteilen,
   leeren Öffnungen und Patches,
5. persistente Raum-, Oberflächen- und Patch-Identitäten einschließlich
   Eins-zu-eins-, Split-, Merge- und Stilllegungsregeln,
6. fachliche Trennung von Patch, wirksamer Oberfläche und Host-Provenance,
7. geometrische Grenzflächen mit ein- oder beidseitigen akustischen
   Belegungen und Raum-zu-Raum-Portale,
8. Innen-/Außenseitenkonvention,
9. Orientierung der Flächennormalen,
10. geometrische Toleranzen,
11. Regeln für geschlossene Hüllen, explizite Öffnungen und Auswahlgrenzen,
12. Triangulierungsregeln und zulässige Mesh-Qualität,
13. Längen-, Winkel- und Frequenzeinheiten,
14. atomare Exportsemantik,
15. Datenschutz- und Lizenzhinweise für eingebettete Materialdaten.

### 7.1 Vorgaben für die b-cad-Spezifikation

Die technische b-cad-Spezifikation muss die folgenden Mechanismen festlegen:

- Laufzeitbezogene Raumkennungen und die Reihenfolge einer Raumerkennung sind
  keine persistente fachliche Identität.
- Bei einer nachweisbaren Eins-zu-eins-Fortschreibung bleibt die fachliche
  Kennung erhalten. Unsichere geometrische Ähnlichkeit genügt dafür nicht.
- Bei einem Split werden der Vorgänger stillgelegt und neue Nachfolger mit
  nachvollziehbarer Abstammung erzeugt. Bei einem Merge werden alle Vorgänger
  stillgelegt und eine neue Entität mit nachvollziehbarer Abstammung erzeugt.
- Zuweisungen werden bei Split oder Merge nicht automatisch auf einen
  beliebigen Nachfolger übertragen. Konfliktfreie Vorschläge werden erst nach
  Bestätigung wirksam.
- Ein Patch und die daraus abgeleitete wirksame akustische Oberfläche sind
  verschiedene persistente Entitäten. Die Beziehung zur Wirtsfläche bleibt
  nachvollziehbar.
- Die leere Öffnung ist ein persistentes Wandobjekt mit eigenem Lebenszyklus.
  Ihre Geometrie wird in Raumableitung, Persistenz und Validierung wie die
  Aussparung eines Einbauteils, aber ohne eingesetzte Abschlussfläche behandelt.
- Geometrische Toleranzen für Randberührung, Überlappung, Aussparung,
  Flächenvergleich und Eins-zu-eins-Zuordnung werden zentral festgelegt und in
  Bearbeitung, Validierung und Exportvorbereitung identisch angewendet.

Format, physische Verpackung, Schemas, Kompatibilitätsregeln sowie das
Verhalten bei unbekannten Feldern und neueren Versionen werden nicht in der
b-cad-Spezifikation, sondern ausschließlich in der gemeinsamen
produktneutralen Austauschformatspezifikation normativ festgelegt.

Diese Mechanik gehört nicht in das lösungsfreie Lastenheft.

## 8. Auswirkungen auf Datenmodell und Architektur

Der CR erfordert voraussichtlich neue fachliche Entitäten und Relationen:

- akustisches Material,
- Frequenzstützstelle und Koeffizienten,
- Materialherkunft,
- raumseitige Oberfläche,
- Raum-/Bauteilseite,
- akustische Teilfläche,
- leere Öffnung,
- Oberflächenrolle,
- Validierungsbefund,
- Exportprofil,
- persistente fachliche Kennung,
- geometrische Grenzfläche und deren Seitenzuordnungen,
- Portal und Auswahlgrenze.

Betroffene Bereiche:

| Bereich | Erwartete Änderung |
|---|---|
| `spec/data-model.yaml` | neue Tabellen, Relationen, persistente IDs sowie Split-/Merge-Provenance |
| Domänenmodell | Materialien, Oberflächen, Seiten und Teilflächen |
| Raumableitung | stabile Zuordnung von Raum und Begrenzungsflächen; keine Verwendung laufzeitbezogener `RoomId` als fachliche Identität |
| Geometrieadapter | triangulierbare, neutral beschriebene Innenflächen |
| Persistenz | atomarer Roundtrip aller akustischen Daten |
| UI/Viewer | Flächenselektion, Belegung, Patch-Editor und Validierung |
| Exportadapter | versioniertes akustisches Szenenpaket |
| Tests | Roundtrip, Identität, Mesh-Topologie und Exportvertrag |

Die fachliche Trennung muss erhalten bleiben:

> Ein CAD-Bauteil erzeugt oder hostet akustische Oberflächen. Ein
> CAD-Bauteilmaterial ist nicht automatisch ein akustisches Oberflächenmaterial.

Der Exportadapter darf keine OCC-, Qt-, CUDA-, OptiX- oder a-ray-internen
Datentypen in den Austauschvertrag übernehmen.

## 9. Austauschvertrag mit a-ray

### 9.1 Grundsatz

Der Vertrag zwischen b-cad und a-ray ist dateibasiert und solverneutral.

Die **gemeinsame produktneutrale Austauschformatspezifikation** ist die
alleinige normative Vertragsquelle. Sie wird unabhängig von den
Produktspezifikationen versioniert und legt mindestens physische Verpackung,
Schemas, Geometriecontainer, Einheiten, Identitäten, Grenzflächen-/
Seitensemantik, Portale, Materialenergiebilanz, Validierung und
Kompatibilitätsregeln fest. b-cad deklariert beim Export die verwendete
Formatversion; a-ray deklariert und prüft seine unterstützten Formatversionen.

Die Produktlastenhefte dürfen Anforderungen an Erzeugung beziehungsweise
Konsumierung stellen, aber keine abweichende Formatsemantik definieren.
Unabhängig von seinem Status bindet das a-ray-ADR
`docs/plan/adr/0002-geometrieformat.md` nur die a-ray-Implementierung und ist
keine Autorität für den gemeinsamen Vertrag oder den b-cad-Export.

Ein Paket besitzt logisch mindestens folgende Bestandteile; die Darstellung
ist ausdrücklich keine Festlegung von Dateinamen oder Containerformaten:

```text
Szenenpaket
├── Manifest und Version
├── triangulierte Geometrie
├── Räume, Grenzflächen, Seiten und Portale
├── akustische Oberflächen und Materialien
└── Provenance und Metadaten
```

Vor Implementierung des Export-Slices müssen b-cad und a-ray dieselbe konkrete
Version der gemeinsamen Austauschformatspezifikation referenzieren. Die Wahl
zwischen glTF/GLB und einem anderen Geometriecontainer erfolgt ausschließlich
dort und nicht durch eine einseitige Produktentscheidung.

### 9.2 Geometrischer Mehrraumvertrag

Die folgenden Feldnamen und Zuordnungen sind Anforderungen an die noch zu
verabschiedende gemeinsame Austauschformatspezifikation. Sie werden nicht in
das b-cad-Lastenheft übernommen.

Eine gemeinsame massive Grenze zwischen zwei Räumen wird geometrisch genau
einmal unter einer `boundary_id` gespeichert. Jedes Mesh-Primitiv verweist auf
genau eine `boundary_id`. Die Grenze besitzt bis zu zwei Seitenzuordnungen;
jede verweist auf genau eine `room_id`, genau eine akustische `surface_id` und
die Orientierung zum zugeordneten Raum. Unterschiedliche Materialien beider
Wandseiten erzeugen daher keine deckungsgleichen Dreiecke.

Ein vom Benutzer angelegter Patch wird mit stabiler `patch_id` ausgetauscht.
Jede aus ihm abgeleitete wirksame Oberfläche besitzt eine eigene `surface_id`
und verweist über `host_patch_id` auf ihre Herkunft. Geometrie und akustische
Belegung referenzieren die `surface_id`; `patch_id` und `host_patch_id` dienen
der Provenance und besitzen keine eigene Solversemantik.

Neu entstandene Räume, Oberflächen oder Patches dürfen bei Split oder Merge
die stabilen Vorgängerkennungen in `derived_from` aufführen. Das Feld beschreibt
ausschließlich Abstammung und bewirkt keine automatische Übernahme externer
Zuordnungen.

Türen und Fenster schneiden ihre Wirtsgrenze aus und ersetzen die Aussparung
durch eine eigene Grenzfläche mit eigenen Seitenzuordnungen. Eine leere Öffnung
schneidet die Wirtsgrenze aus, enthält keine verdeckende Geometrie und wird als
Portal mit stabiler `portal_id` und den angrenzenden `room_id`-Werten
beschrieben. Ein Portal zum Außenbereich besitzt genau einen angrenzenden Raum
und kennzeichnet die Gegenseite als `outside`.

Eine massive Grenze zu einem nicht exportierten Nachbarraum darf mit nur der
exportierten Raumseite und dem Status `neighbor_not_exported` enthalten sein.
Ein Portal zu einem nicht exportierten Nachbarraum ist kein vollständiges
Simulationsmodell und blockiert den Export, bis der Nachbarraum ausgewählt ist.
Künstliche Abschlüsse oder Randbedingungen sind nicht Bestandteil dieses CR
und bedürfen eines separaten Change Requests.

### 9.3 Energetischer Materialvertrag

Die gemeinsame Austauschformatspezifikation legt je Frequenzstützstelle fest:

- Fehlt der optionale Transmissionsgrad `τ(f)`, gilt `τ(f) = 0`.
- Für Absorption `α(f)` und Transmission `τ(f)` gilt
  `0 ≤ α(f) ≤ 1`, `0 ≤ τ(f) ≤ 1` und `α(f) + τ(f) ≤ 1`.
- Der reflektierte Anteil ist `ρ(f) = 1 - α(f) - τ(f)`.
- Für den Streugrad gilt `0 ≤ s(f) ≤ 1`. Er teilt ausschließlich den
  reflektierten Anteil in `(1-s(f))·ρ(f)` spiegelnde und `s(f)·ρ(f)` diffuse
  Energie.

Damit sind insbesondere einzelne Randwerte `0` oder `1` nur zulässig, wenn
alle kombinierten Invarianten erfüllt bleiben.

### 9.4 Nicht im b-cad-Export enthalten

Das b-cad-Szenenpaket enthält ausdrücklich nicht:

- Schallquellen,
- Empfänger oder Mikrofone,
- Richtcharakteristiken,
- Solverparameter,
- Rayanzahl oder Reflexionsordnung,
- GPU- oder OptiX-Konfiguration,
- Energy-Time-Curves,
- Raumimpulsantworten,
- Auralisationsdaten,
- berechnete Kennwerte.

Diese Daten werden in a-ray angelegt oder verwaltet.

### 9.5 Versionierung

Das Szenenpaket muss mindestens enthalten:

- Formatname,
- Major-/Minor-Version,
- eindeutige Kennung der gemeinsamen Austauschformatspezifikation,
- Generatorname und Generatorversion,
- eindeutige Szenenkennung,
- optionalen fachlichen Inhalts-Hash.

a-ray muss ausschließlich anhand der gemeinsamen Austauschformatspezifikation
und der deklarierten Formatversion entscheiden können, ob es ein Paket
vollständig unterstützt, eingeschränkt importiert oder ablehnt.

## 10. Nicht Bestandteil dieses CR

- Schallquellen, Empfänger und Richtcharakteristiken,
- Simulationseinstellungen,
- Ray Tracing, Beam Tracing, Path Tracing oder Image-Source-Berechnung,
- Lösung der Wellengleichung, FEM, BEM oder FDTD,
- NVIDIA OptiX, CUDA oder andere GPU-Backends,
- Berechnung von Energy-Time-Curves oder Impulsantworten,
- Kennwerte wie `T20`, `T30`, `EDT`, `C80`, `D50`, `STI`, `G` oder `LF`,
- Auralisation,
- Import oder Visualisierung von a-ray-Ergebnissen,
- automatische Optimierung von Materialien oder Geometrie,
- verlässliche Konzertsaal-Prognose oder bauakustische Zertifizierung.

Diese Punkte gehören in das Lastenheft von a-ray oder in spätere,
eigenständige b-cad-Change-Requests.

## 11. Risiken

### R-1 — Verdeckte Scope-Ausweitung

Eine pauschale Erweiterung auf „Gebäude und Innenräume“ könnte als Versprechen
vollständiger Nichtwohngebäudeplanung missverstanden werden. Die Erweiterung
bleibt deshalb auf den Zweck des Akustik-Fachmoduls begrenzt.

### R-2 — Instabile Raum- oder Flächenkennungen

Instabile Kennungen können persistierte Materialbelegungen und externe
Referenzen falsch zuordnen. Eine rein laufzeitbezogene Raumkennung oder ein ADR
ohne persistentes Datenmodell erfüllt die Foundation-Anforderung nicht.

### R-3 — Vermischung von Bauteil- und Oberflächenmaterial

Konstruktive und akustische Materialien müssen getrennte fachliche Konzepte
bleiben.

### R-4 — Scheingenauigkeit

Tabellierte Materialwerte sind modell- und messabhängig. Herkunft und
Datenqualität müssen nachvollziehbar sein.

### R-5 — Zu frühe Solverkopplung

Direkte Abhängigkeiten von a-ray, OptiX oder CUDA würden den b-cad-Kern
unnötig binden und die Testbarkeit verschlechtern.

### R-6 — Unzureichende Mesh-Topologie

Optisch plausible CAD-Geometrie kann für Raytracing ungeeignet sein.
Nicht mannigfaltige Kanten, doppelte Flächen oder falsche Normalen müssen vor
dem Export erkannt werden. Insbesondere dürfen zwei akustisch verschieden
belegte Seiten einer gemeinsamen Wand nicht als deckungsgleiche Dreiecksflächen
exportiert werden.

### R-7 — Versionsdrift des Austauschformats

Ohne eigenständige Schemas und Kompatibilitätsregeln könnten b-cad und a-ray
nur gemeinsam veröffentlicht werden. Abweichende Entscheidungen in einem der
beiden Produktlastenhefte würden zudem zwei konkurrierende Vertragsquellen
erzeugen.

### R-8 — Fehlender Erzeugungspfad für leere Öffnungen

Das bestehende Öffnungsmodell (`src/hexagon/model/opening.h`) kennt Türen und
Fenster; deren Entfernung schließt die Wand wieder. Ohne das mit diesem CR
beantragte eigenständige Wandobjekt „leere Öffnung“ wären offene
Raumverbindungen weder durch den Benutzer erzeugbar noch die zugehörigen
Abnahmekriterien umsetzbar.

## 12. Umsetzungsvorschlag nach Annahme

Der CR soll nicht in einem einzelnen Implementierungsslice umgesetzt werden.

1. **Lastenheft-Slice:** Scope, Rolle, Ziel und Anforderungen mit endgültigen
   IDs normativ übernehmen.
2. **b-cad-Spec-/ADR-Slice:** Vor der Implementierung Identität,
   Eins-zu-eins-, Split-/Merge- und Stilllegungsregeln,
   Grenzflächen-/Seitenmodell, leere Öffnungen und Portale sowie die neutrale
   Naht zwischen Geometrieerzeugung und Export in `spec/spezifikation.md` und
   erforderlichen ADRs entscheiden.
3. **Identity-Foundation-Slice:** persistente Raum-, Oberflächen- und
   Patch-Identitäten samt Eins-zu-eins-, Split-, Merge-, Stilllegungs- und
   Provenance-Regeln im Datenmodell und in der Persistenz implementieren.
4. **Gemeinsamer Vertragsslice:** produktneutrale
   Austauschformatspezifikation einschließlich Mehrraumgrenzen, Portalen,
   Energiesemantik, Schemas und Versionierung gemeinsam mit a-ray festlegen.
5. **Material-Slice:** akustische Materialien und Frequenzkennlinien.
6. **Boundary-/Opening-Slice:** raumseitige Basisflächen, gemeinsame
   geometrische Grenzflächen sowie Anlegen, Bearbeiten, Löschen und Persistieren
   leerer Öffnungen und deren Portale.
7. **Patch-Slice:** Teilflächen, effektive Oberflächen und Prioritätsregeln.
8. **Validierungs-Slice:** Hüllen-, Normalen-, Auswahlgrenzen- und
   Materialprüfung.
9. **Export-Slice:** Export gemäß einer konkret referenzierten Version der
   gemeinsamen Austauschformatspezifikation.
10. **Interop-Slice:** Referenzpakete und gegenseitige Vertragstests mit a-ray.

Der Identity-Foundation-Slice beginnt erst nach Abschluss des
b-cad-Spec-/ADR-Slices. Die Implementierungsslices 5 bis 10 dürfen die Identity
Foundation nicht durch prozesslokale Kennungen ersetzen. Der Export-Slice
beginnt erst, wenn der gemeinsame Vertrag in einer konkreten Version vorliegt.

## 13. Abnahme des Change Requests

Der Projektinhaber hat am 2026-07-22 folgende Entscheidungen fachlich
bestätigt:

1. Das Akustik-Fachmodul darf Innenräume außerhalb des bisherigen
   Wohngebäudeschwerpunkts unterstützen, ohne vollständige
   Nichtwohngebäudeplanung zu versprechen.
2. Akustische Oberflächen sind eigenständige fachliche Entitäten.
3. Konstruktive und akustische Materialien bleiben getrennt.
4. b-cad verwaltet keine Quellen, Empfänger oder Simulationen.
5. a-ray ist primärer, aber nicht exklusiver Verbraucher des Exports.
6. Die gemeinsam gepflegte, produktneutrale Austauschformatspezifikation ist
   die einzige normative Vertragsquelle und wird eigenständig versioniert.
7. Gemeinsame Grenzen werden einmal geometrisch und mit ein- oder beidseitigen
   akustischen Oberflächen beschrieben; leere Öffnungen sind Portale.
8. Die leere Öffnung ist ein eigenständig anleg-, bearbeit- und löschbares
   Wandobjekt; bewegliche Tür-/Fensterzustände bleiben außerhalb dieses CR.
9. Persistente Identitäten samt Split-/Merge-Regeln bilden einen eigenen,
   vorgelagerten Foundation-Slice.
10. Die Umsetzung erfolgt in mehreren Spezifikations- und Implementierungsslices.

## 14. Entscheidungsprotokoll

| Entscheidung | Status |
|---|---|
| Akustik-Fachmodul für Innenräume außerhalb des Wohngebäudeschwerpunkts öffnen | Angenommen |
| Akustisches Oberflächenmodell aufnehmen | Angenommen |
| Quellen und Empfänger ausschließlich in a-ray verwalten | Angenommen |
| a-ray als primären externen Verbraucher benennen | Angenommen |
| Gemeinsame produktneutrale Formatspezifikation als alleinige Vertragsquelle festlegen | Angenommen |
| Gemeinsame Grenzfläche mit raumseitigen Seitenbelegungen verwenden | Angenommen |
| Leere Öffnung als eigenständiges Wandobjekt aufnehmen | Angenommen |
| Portal zu nicht ausgewähltem Raum bis zur Auswahl des Nachbarraums blockieren | Angenommen |
| Persistente Identitäten als Foundation-Slice vorziehen | Angenommen |
| Frequenzkennlinien nicht auf feste Oktavbänder begrenzen | Angenommen |
| Energiesemantik für Absorption, Transmission, Reflexion und Streuung festlegen | Angenommen |
| Ergebnisimport aus diesem CR ausschließen; separater CR erforderlich | Angenommen |
````

## 11. Closure-Notiz

<!-- Erst nach Ausführung füllen: endgültige IDs, Versionsstand, Review-/Gate-Belege und Lerneintrag. -->
