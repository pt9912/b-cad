---
id: slice-039b
titel: Akustik — b-cad-Spezifikation und ADR-Entscheidungen
status: open
welle: unzugeordnet (nach slice-039a)
lastenheft_refs: []  # Erst nach Closure von slice-039a auf dessen endgültige IDs umstellen.
adr_refs: []         # Neue ADR-IDs werden erst beim normativen Schreibakt vergeben.
---

# Slice 039b: Akustik — b-cad-Spezifikation und ADR-Entscheidungen

**Status:** open — technischer normativer Folgeslice, weder priorisiert noch
startfähig vor Closure von
[`slice-039a`](slice-039a-akustik-lastenheft-uebernahme.md).

**Welle:** unzugeordnet. Die spätere Einplanung folgt dem Lifecycle
`open → next → in-progress` und verdrängt die aktive Welle nicht.

**Annahmegrundlage:** Die selbsttragende fachliche Vollbaseline, der
10+4-Produktvertrag und das normative Übernahmeregister stehen vollständig in
[`slice-039a`](slice-039a-akustik-lastenheft-uebernahme.md#9-vollständiges-normatives-übernahmeregister).
Dieser Plan hängt nicht vom temporären Ablageort des früheren Arbeitsentwurfs
ab.

---

## 1. Ziel und Schnitt

Die von `slice-039a` abnahmebindend gemachten Produktanforderungen in
[`spec/spezifikation.md`](../../../../spec/spezifikation.md) technisch
präzisieren und notwendige Grundsatzentscheidungen in neuen ADRs treffen,
bevor Identity- oder Akustik-Produktionscode entsteht.

Dieser Slice entscheidet ausschließlich den b-cad-Anteil:

- fachliche Identität und Provenienz von Raum, raumseitiger Oberfläche und
  Patch einschließlich Eins-zu-eins-, Split-, Merge- und Stilllegungsfällen,
- eine gemeinsame geometrische Grenze mit einer oder zwei akustischen Seiten,
- Tür-, Fenster-, leere Öffnungs- und Portalverhalten einschließlich der
  blockierenden offenen Auswahlgrenze,
- akustische Materialien, Teilflächen, effektive Oberflächen und
  Validierungsinvarianten,
- die neutrale Naht zwischen Geometrieerzeugung, Hüllenprüfung und Export,
- Persistenz-/Migrationsfolgen für stabile Identitäten und Akustikdaten.

Nicht in diesem Slice normiert werden das gemeinsame Austauschschema, konkrete
Austauschfelder, Serialisierungstypen, Paketlayout oder Kompatibilitätsregeln.
Diese gehören ausschließlich in die gemeinsam gepflegte, produktneutrale
Formatspezifikation. b-cad darf danach nur eine konkrete Version referenzieren
und deren Anforderungen an seinem Adapter erfüllen.

## 2. Entscheidungs-Oracle

| Thema | In diesem Slice zu entscheidender Vertrag | Abgrenzung |
|---|---|---|
| Identity-Lifecycle | stabile fachliche Identität, Host-Provenienz, eindeutige Eins-zu-eins-Fortführung sowie sichtbare Split-/Merge-/Stilllegungsfolgen | keine prozesslokalen oder aus Darstellungsreihenfolgen abgeleiteten IDs als fachliche Identität |
| Entitätstrennung | Raum-, Oberflächen- und Patch-Identität bleiben getrennte Konzepte; Patch-Herkunft ersetzt keine Oberflächenidentität | keine stille Wiederverwendung einer Alt-ID für fachlich andere Entität |
| Mehrraumgrenze | eine geometrische Grenze kann eine oder zwei raumseitige akustische Belegungen tragen; Seite und Nachbarschaft sind eindeutig | keine deckungsgleichen Gegenflächen im Exportmodell |
| Einbauteile | Tür/Fenster schneiden die Wirtsfläche aus und füllen den Bereich als eigenes Element ohne Loch/Doppelbelegung | bewegliche Offen-/Geschlossen-Zustände bleiben ausgeschlossen |
| Leere Öffnung/Portal | eigenständiger persistenter Erzeugungspfad; Portal verbindet zwei Räume oder Raum/Außenbereich | kein künstlicher Abschluss; Portal zu nicht ausgewähltem Innenraum blockiert |
| Materialien | frequenzabhängige passive Kennwerte, Herkunft/Qualität, Referenzschutz; konstruktive und akustische Materialien getrennt | konkrete Feldnamen und Energieberechnung der gemeinsamen Formatspezifikation nicht vorwegnehmen |
| Patches | Innen-/Rand-/Öffnungs- und Überlappungsregeln; Material zwingend für Export, Rolle nur beschreibend | Rolle erzeugt keine eigene akustische Wirkung |
| Hüllenprüfung | Vollständigkeit, Orientierung, Mannigfaltigkeit, Überschneidung, Belegung und offene Übergänge mit adressierbaren Befunden | keine fälschlich positive Hülle bei offenen/ungültigen Grenzen |
| Exportauswahl | Räume/Raumgruppen, massive Auswahlgrenze, offene Portalgrenze und atomare Exportwirkung | kein Erfolg bei blockierendem Befund; vorhandener gültiger Export bleibt unverändert |
| Adapter-Naht | solverneutrale b-cad-Ausgabe hinter einem nach innen gerichteten Port; keine a-ray-, GPU- oder Solverabhängigkeit im Kern | gemeinsames Format bleibt externe normative Vertragsquelle |
| Persistenz | atomare Speicherung und migrationsfähiges Schema für die neuen fachlichen Entitäten | bestehende Projekte ohne Akustikdaten bleiben gültig |

## 3. Definition of Done

- [ ] **Voraussetzungen:** `slice-039a` ist abgeschlossen; seine endgültigen
      Rollen-, Ziel-, ACO- und QA-IDs ersetzen die leeren Frontmatter-Referenzen
      atomar. Die gemeinsame Formatspezifikation besitzt mindestens einen
      benannten Owner und einen geplanten Vertragsslice; sie muss für die
      internen b-cad-Entscheidungen noch nicht fertig sein.

- [ ] **Live-Audit:** aktuelle
      [`spec/spezifikation.md`](../../../../spec/spezifikation.md),
      [`spec/data-model.yaml`](../../../../spec/data-model.yaml),
      [`spec/architecture.md`](../../../../spec/architecture.md), bestehende
      ADRs und offene Pläne gegen das Oracle aus §2 prüfen. Bestehende
      `Accepted`-ADRs werden nicht geändert; neue ADR-IDs entstehen erst beim
      Schreibakt und werden im ADR-Index ergänzt.

- [ ] **Spezifikation:** alle §2-Themen mit überprüfbaren Invarianten,
      Vor-/Nachbedingungen und Fehlerfällen präzisieren. Die Spezifikation
      referenziert die endgültigen Lastenheft-IDs aufwärts, aber keine Slices.

- [ ] **Grundsatzentscheidungen:** neue ADRs nur für tatsächlich langfristige
      Alternativen und Konsequenzen, mindestens Identity-Fortführung,
      Mehrraum-/Portalmodell und Adapter-/Vertragsgrenze prüfen. ADRs verweisen
      auf Spec/Lastenheft, nicht auf diesen Slice.

- [ ] **Datenmodell-Folge:** fachliche Entitäten, Relationen, Constraints und
      Migrationsbedarf in `spec/data-model.yaml` nur dann ändern, wenn die
      technischen Entscheidungen dies bereits eindeutig tragen. In diesem Fall
      `make schema-check` ausführen; ansonsten einen benannten
      Identity-Foundation-Folgeslice mit vollständiger Schema-DoD anlegen.

- [ ] **Architektur-Sicht:** `spec/architecture.md` nur ändern, wenn eine neue
      stabile Komponenten-/Portgrenze entsteht. Der Text bleibt sprach- und
      meilensteinfrei und enthält keine ADR-Abwärtsreferenz; Provenienz steht
      ausschließlich in `## Geschichte`.

- [ ] **Gemeinsame Vertragsgrenze:** konkrete Austauschfelder, Energieformeln,
      Paket-/Schemaformate und Versionsregeln werden nicht in b-cad festgelegt.
      Der gemeinsame Vertragsslice erhält aus dem Oracle eine explizite
      Übergabeliste für Mehrraumgrenzen, Portale, Materialenergie, Identitäten,
      Provenienz und Kompatibilität.

- [ ] **Folgeplanung:** getrennte offene Pläne mindestens für Identity
      Foundation, Material, Boundary/Opening, Patch, Validierung, gemeinsamen
      Formatvertrag, Export und Interop anlegen. Jeder Plan nennt seine
      Lastenheft-/Spec-Bindung, Abhängigkeiten, engsten Sensoren und Risiken.

- [ ] **Unabhängiges Review:** vor Implementierungs-Start vollständiges
      [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review;
      zusätzlich unabhängiges Normativ-Diff-Review gegen §2 und die
      Vollbaseline aus `slice-039a`. HIGHs blockieren.

- [ ] **Sensoren:** `make docs-check`; bei Datenmodelländerung zusätzlich
      `make schema-check`; vor Handoff vollständig `make gates`.

## 4. Geplante Dateien

| Datei / Bereich | Behandlung |
|---|---|
| `spec/spezifikation.md` | technische Akustikverträge gemäß §2 |
| `docs/plan/adr/{neue-IDs}.md` | nur erforderliche neue Grundsatzentscheidungen |
| `docs/plan/adr/README.md` | Index für neue ADRs |
| `spec/data-model.yaml` | nur bei im Slice vollständig entschiedenem Schema; sonst Identity-Folgeslice |
| `spec/architecture.md` | nur bei neuer stabiler Sicht; keine ADR-Abwärtsreferenz |
| `docs/plan/planning/open/` | getrennte, benannte Folgepläne aus der DoD |
| `docs/reviews/{Datum}-slice-039b-plan.md` | unabhängiges Plan-Review |

## 5. Risiken und Closure

- **Einseitiger Formatvertrag:** technische Präzision kann versehentlich die
  gemeinsame Formatspezifikation vorwegnehmen. Jede Austauschfestlegung wird
  gegen §1 und §3 geprüft und andernfalls in den gemeinsamen Vertragsslice
  verschoben.
- **Identity zu spät:** Material-, Patch- oder Exportplanung darf keine
  Übergangslösung mit instabilen Kennungen etablieren. Identity Foundation
  bleibt deren Implementierungsvoraussetzung.
- **Fehlender Öffnungspfad:** Das bestehende Modell kennt keinen eigenständigen
  Erzeugungspfad für dauerhaft leere Öffnungen. Der Boundary-/Opening-Plan muss
  diesen Pfad ausdrücklich vom Tür-/Fensterzustand trennen.
- **Scope-Größe:** Falls das unabhängige Review die Entscheidungen nicht in
  einer Sitzung vollständig prüfen kann, vor Aktivierung in Identity/Boundary
  und Material/Validierung/Adapter teilen; die gemeinsame Vertragsgrenze bleibt
  in beiden Teilen identisch.

Closure erst bei vollständig entschiedener b-cad-Spezifikation, aktualisiertem
ADR-Index, benannten Folgeplänen und grünen erforderlichen Sensoren. Der Export
bleibt bis zu einer konkret versionierten gemeinsamen Formatspezifikation
blockiert.
