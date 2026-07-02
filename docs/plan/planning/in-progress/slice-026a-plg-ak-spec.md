---
id: slice-026a
titel: Plugin-System — AK-Schärfung [LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg)..004 & Spec-Mapping (parametrisiert auf [ADR-0017](../../adr/0017-plugin-api-abi.md))
status: in-progress
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg), [LH-FA-PLG-002](../../../../spec/lastenheft.md#modul-plugin-system-plg), [LH-FA-PLG-003](../../../../spec/lastenheft.md#modul-plugin-system-plg), [LH-FA-PLG-004](../../../../spec/lastenheft.md#modul-plugin-system-plg)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0017](../../adr/0017-plugin-api-abi.md)]
---

# Slice 026a: Plugin-System — AK-Schärfung & Spec-Mapping

**Status:** in-progress — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review **0 HIGH / 1 MED / 3 LOW / 3 INFO**;
Start **nicht blockiert**, MED-1 (Ehrlichkeits-Klausel um Silent-Corruption-
Grenzfall vervollständigt) + LOW-1 (Wortlaut ohne „Initialisierung") + LOW-2
(ids-Familie um PLG) + LOW-3/INFO-1..3 (Ausführungs-/Closure-Hinweise)
eingearbeitet; beide benannten Entscheidungspunkte bestätigt (Sandbox-AK-
Fassung = legitime Reifung, keine stille Schwächung; ein Code/zwei Events
vorzugswürdig). [Report](../../../reviews/2026-07-02-slice-026a-plan.md).
**Reine Doku/Entscheidung, kein Code.**

**Welle:** welle-5-erweiterung (PLG-Strang, Entscheidungs-/Spec-Hälfte —
**M5-bindender Strang** der Welle; Muster slice-019a [IFC] / slice-020a
[STEP/STL] / slice-021a [DXF] / slice-025a [PDF/PNG]).

**Bezug:** [LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg) (Dynamische Plugins) + [LH-FA-PLG-002](../../../../spec/lastenheft.md#modul-plugin-system-plg)
(Plugin-API, stabiler Vertrag) + [LH-FA-PLG-003](../../../../spec/lastenheft.md#modul-plugin-system-plg) (Plugin-Lifecycle) +
[LH-FA-PLG-004](../../../../spec/lastenheft.md#modul-plugin-system-plg) (Plugin-Sandbox), bisher **Outline**-Stubs (Bullets ohne
per-ID-Anker); [OBJ-004](../../../../spec/lastenheft.md#3-projektziele) (Erweiterbarkeit — der M5-Trigger hängt an
diesem Projektziel, **kein** Plugin-ACC existiert oder entsteht; Review-INFO-2). **Parametrisiert auf [ADR-0017](../../adr/0017-plugin-api-abi.md) (Plugin-API-/ABI-Vertrag und
Sandbox-Modell, accepted 2026-07-02)** — das Backend ist **entschieden**
(`dlopen` + versionierter `extern "C"`-Handshake fail-closed + C++-Port-Facade
in-process; Plugin-Host als Driving Adapter; Plugins sehen nur model +
Driving-Ports, kein Beobachter-Zugang v1; Sandbox = Port-Vermittlung +
Fehler-Barriere mit ehrlich benannten Grenzen); dieser Slice braucht **keine
neue Grundsatz-ADR**, seine **Mapping-/Mechanik-Entscheidung lebt in
Lastenheft-AK + `spezifikation.md` §1/§4/§5/§6** (Muster 019a/020a/021a/025a).
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Schichtung; Plugin-Host = zweiter Driving Adapter),
[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) (Beobachter-Ausschluss v1), [ADR-0009](../../adr/0009-gui-framework-qt6.md) (Threading-Prämisse
Single-threaded am Qt-Event-Loop).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-02.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des PLG-Strangs (Muster
019a/020a/021a/025a — AK-Schärfung vor Impl). Die Plugin-AK (was heißt Laden/
Entladen zur Laufzeit, stabiler Vertrag, Lifecycle, Sandbox **benutzer-
beobachtbar**) brauchen **prüfbare AK + ein entschiedenes Mapping**, bevor
implementiert wird. **Reine Doku/Entscheidung, kein Code.**

---

## 1. Ziel

Die vier Plugin-Anforderungen [LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg)..004 bekommen
**lösungsfreie, benutzer-beobachtbare Akzeptanzkriterien** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ein
**entschiedenes Mapping** (`spezifikation.md` §1), bevor implementiert wird —
**innerhalb des von [ADR-0017](../../adr/0017-plugin-api-abi.md) entschiedenen Backends**. Zusätzlich wird die
**[ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflicht „AK-Schärfung + Spec-Nachzug" eingelöst**: die durch
den ADR-Accept konkretisierungs-bedürftigen Spec-Stellen — **§4**
[`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Präzisierung (Load-/Handshake-Ablehnung vs.
Laufzeit-Fehlverhalten), **§5** Plugin-Lifecycle-Span (die Span-Outline ist
ausdrücklich „ADR-Folge"), **§6** Plugin-API-Vertragszeile — werden nachgezogen.

**Plugins sind ein zweiter Driving-Weg in denselben Kern.** Anders als die
IO-Formate (Driven-Adapter, welle-4) steuert ein Plugin den Kern wie die GUI:
über Driving-Ports, mit derselben Validierung/Klemmung und derselben atomaren
Persistenz. Die fachliche Eigenheit, die die AK prägt: **Laden/Entladen zur
Laufzeit** (ohne Neustart), **Vertragsstabilität beobachtbar als Ablehnung
unpassender Plugins** (statt undefiniertem Verhalten) und **Sandbox als
beobachtbare Isolierung** fehlverhaltender Plugins bei unverändertem Modell.

**Reifephase-Teilumfang welle-5 (= [ADR-0017](../../adr/0017-plugin-api-abi.md)-Subset):** in-process
Shared-Library-Plugins ([REQ-TEC-008](../../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec)) mit Port-Vermittlung; **kein**
Beobachter-Zugang (pull-only in Hooks), **kein** Speicherschutz gegen
nicht-wohlgeformte Plugins (Grenze ehrlich benannt, Netz = Crash-Recovery
[LH-QA-005](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery)), **keine** UI-Erweiterungspunkte, **kein** Signier-/
Vertrauensmodell — alle als benannte Lücken/Re-Eval-Trigger in
[ADR-0017](../../adr/0017-plugin-api-abi.md) §Trigger.

## 2. Definition of Done

- [ ] **Lastenheft [LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg)..004 von Outline auf AK-Niveau**
      (**lösungsfrei, benutzer-beobachtbar** — **kein** `dlopen`/ABI-/Handshake-/
      vtable-/Thread-Vokabular im Lastenheft-Text, das gehört in §1;
      [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)); je Anforderung ein eigenes `####`-Heading mit **Inline-HTML-
      Anker** (Muster [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007); die bestehenden Modul-Anker-Links
      bleiben gültig, das Modul-Heading bleibt). Mindestens:
      **PLG-001 Dynamische Plugins — Happy:** Given ein laufendes b-cad und eine
      Plugin-Datei, when das Plugin geladen wird, then steht seine Funktion
      **ohne Neustart** zur Verfügung; when es entladen wird, then enden seine
      Wirkungen **ohne Neustart**, die Anwendung läuft weiter. **Boundary:**
      Given eine nicht ladbare Datei (kein Plugin/defekt), when geladen, then
      **sichtbare Ablehnung ohne Absturz**, das Modell bleibt unverändert.
      **PLG-002 Plugin-API — Happy:** Given ein zum laufenden b-cad **passendes**
      Plugin, when geladen, then wird es angenommen. **Negative:** Given ein
      Plugin mit **unpassendem Vertragsstand** (für eine andere b-cad-Version
      gebaut), when geladen, then wird es **vor jeder Wirkung abgelehnt**
      (sichtbare Meldung, **ohne jede (Teil-)Wirkung** — „Initialisierung" ist
      §1-Mechanik-Vokabular, Review-LOW-1; Modell unverändert) — statt
      undefiniert zu laufen; der Vertragsstand ist versioniert und die
      Ablehnung nennt ihn.
      **PLG-003 Lifecycle:** die Zustandsfolge Laden → Aktiv → Beenden →
      Entladen ist beobachtbar; **nach** dem Entladen hat das Plugin **keine**
      weitere Wirkung; ein Fehler in **jedem** Schritt führt dazu, dass das
      Plugin nicht (weiter) wirkt und die Anwendung weiterläuft.
      **PLG-004 Sandbox:** Given ein Plugin, das beim Arbeiten einen Fehler
      auslöst (**wohlgeformtes** Fehlverhalten, z. B. Ausnahme), when der Fehler
      auftritt, then wird das Plugin **isoliert/entladen** (sichtbare Meldung),
      das **Gebäudemodell bleibt unverändert**, die Anwendung läuft weiter;
      Modell-Änderungen durch Plugins unterliegen **denselben sichtbaren
      Prüf-/Klemm-Regeln wie manuelle Eingaben** (kein Nebeneingang).
      **Ehrlichkeits-Klausel (benutzer-beobachtbar, beide Grenzfälle —
      Review-MED-1):** der Schutz gilt für wohlgeformte Plugins; ein Plugin
      mit beliebigem Maschinencode kann die Anwendung **zum Absturz bringen**
      — dann gilt die Crash-Recovery-Zusage
      ([LH-QA-005](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery)): der letzte gespeicherte konsistente Stand bleibt
      ladbar — **oder Daten unbemerkt verfälschen, die ein nachfolgendes
      Speichern übernimmt; dagegen gibt es in dieser Ausbaustufe keinen
      Schutz** (Absturz erscheint nicht als einzige Folge — kein stilles
      Über-Versprechen des Recovery-Netzes). + Header-Nachzug +
      `lastenheft-historie.md` **0.1.13**
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie): Header-`Version:` == oberste Historie-Zeile).
- [ ] **`spec/spezifikation.md` §1 neuer Block [`LH-FA-PLG-001.a`](../../../../spec/lastenheft.md#modul-plugin-system-plg)** (Sammelblock —
      **deckt PLG-001..004**, Muster [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/[`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007)):
      das Plugin-Mapping innerhalb des [ADR-0017](../../adr/0017-plugin-api-abi.md)-Rahmens, als Mechanik
      **ohne ADR-Verweis im Körper** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):
      **Schicht** Plugin-Host als Driving Adapter (`adapters/plugin/`), lädt
      Shared Libraries ([REQ-TEC-008](../../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec)) über den System-Lader;
      **Vertrags-Handshake** versionierte Eintrittspunkte, **exakte**
      Versions-Gleichheit, fail-closed (Mismatch/fehlendes Symbol/nicht ladbar
      → Ablehnung **ohne** Initialisierung); **Lifecycle-Zustandsfolge**
      Entdeckt → Geladen → Handshake → Initialisiert → Aktiv → Beendet →
      Entladen, jeder Fehlerpfad endet in Isolieren/Entladen bei unverändertem
      Modell; **Port-Vermittlung** der Plugin-Kontext reicht ausschließlich
      Driving-Port-Referenzen (kein Driven-Port, kein Beobachter-Zugang —
      pull-only im Hook-Kontext; welches Port-Subset v1 erhält, fixiert der
      Impl-Slice dokumentiert); **Threading** Hooks laufen synchron im
      Hauptthread, Port-Aufrufe nur aus dem Hook-Kontext (Plugin-eigene Threads
      rufen keine Ports — Vertragspflicht, technisch nicht erzwingbar);
      **Fehler-Barriere** jeder Host→Plugin-Übergang ist ausnahme-gesichert;
      die Unload-Strategie im Fehlerpfad (Entladen vs. Isolieren ohne Entladen)
      dokumentiert der Impl-Slice. + `spezifikation-historie.md` +
      `**Letzte Änderung:**`-Header.
- [ ] **`spec/spezifikation.md` §4 + §5 + §6 nachgezogen ([ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflicht):**
      **§4** [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Zeile präzisieren — **Vorschlag: ein Code, zwei
      Log-Events** (Muster generische [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Wiederverwendung beim
      export-only-Lookup-Miss): Bedingung um **Load-/Handshake-Ablehnung**
      erweitert; Aktion „nicht geladen bzw. isoliert/entladen, Modell
      unverändert"; Log `event=plugin_rejected` (Laden/Handshake) bzw.
      `event=plugin_error` (Laufzeit-Fehlverhalten) — **kein** zweiter
      Fehler-Code ohne Not (Entscheidungspunkt fürs Review; Alternative:
      eigener Code für die Load-Ablehnung). **§5** eine neue Span-Zeile für den
      Plugin-Lifecycle (Name + Pflicht-Attribute — Plugin-Kennung, Operation
      laden/entladen/fehler, Ergebnis — entscheidet dieser Slice; §5-Vorspann
      „ADR-Folge" wird damit für PLG eingelöst; `requirement.id`-Konvention
      gilt). **§6** eine neue Vertragszeile **Plugin-API** (versionierter
      Vertrag, exakte Versions-Gleichheit beim Laden; Plugin-Host vermittelt
      Driving-Ports; Shared Libraries [REQ-TEC-008](../../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec); §1 [`LH-FA-PLG-001.a`](../../../../spec/lastenheft.md#modul-plugin-system-plg)).
      ADR-Index-Folgepflicht-Zeile ([ADR-0017](../../adr/0017-plugin-api-abi.md) „AK-Schärfung + Spec-Nachzug")
      abhaken (im Closure-Commit) — **dabei die getroffene
      Ein-Code/zwei-Events-Wahl festhalten** (Review-INFO-3: der
      [ADR-0017](../../adr/0017-plugin-api-abi.md)-§Entscheidung-4-Wortlaut `event=plugin_error` wird durch die
      ausdrückliche Delegation an diesen Slice präzisiert, nicht widersprochen).
- [ ] **`.d-check.yml` ids-Familie um PLG erweitert (Review-LOW-2):** der
      Fehler-Code-Regex `E-(IO|VAL|GEO)-…` deckt `E-PLG-001` **nicht** — dieser
      Slice macht den Code erstmals korpusweit prominent; die Familie wird um
      `PLG` erweitert, damit nackte Erwähnungen computational gefangen werden
      (**Verschärfung**, kein [AGENTS.md §2.6](../../../../AGENTS.md)-Lockerungsfall; Muster
      [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)).
- [ ] **`spec/architecture.md`:** voraussichtlich **unverändert** — Plugin-Host,
      `plugins/`-Baum und §5-Fehlermodell sind seit dem Bootstrap deklariert;
      **optional** eine `## Geschichte`-Provenance-Zeile „Plugin-Host/API →
      [ADR-0017](../../adr/0017-plugin-api-abi.md)" (exclude-section, Link erlaubt — Parität 025a) — sonst
      bewusst nicht geändert (Begründung in Closure).
- [ ] **Reine Doku/Entscheidung — kein Code, keine Tests, kein ADR, kein Schema**
      ([ADR-0017](../../adr/0017-plugin-api-abi.md) deckt das Backend; Mapping ist Spec-Entscheidung).
      `make gates` grün; `make schema-check` unberührt; Closure-Notiz mit
      Lerneintrag. **Nicht Teil:** der **PLG-Impl-Slice** (026b: Plugin-Host +
      Plugin-API-Header-Satz + Beispiel-/Test-Plugin im `plugins/`-Baum +
      Composition-Root + AK-Tests [werfendes Plugin, Vertrags-Mismatch,
      Load→Edit→Unload mit realer Bibliothek] + **arch-check-Regel P** +
      benannte Entscheidungen Symbol-Naht/Gate-Scope/`plugins/`-Unload-Strategie);
      ebenso **nicht Teil:** DRW-/UI-/Mehrsprachigkeits-Stränge der welle-5.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | [LH-FA-PLG-001](../../../../spec/lastenheft.md#modul-plugin-system-plg)..004 Outline → AK (lösungsfrei; per-ID-Heading + Inline-Anker, Modul-Heading bleibt); Header `Version:` → 0.1.13 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.13 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Invariante) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-PLG-001.a`](../../../../spec/lastenheft.md#modul-plugin-system-plg)-Sammelblock (Host/Handshake/Lifecycle/Port-Vermittlung/Threading/Fehler-Barriere); §4 [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Präzisierung; §5 Plugin-Lifecycle-Span-Zeile; §6 Plugin-API-Vertragszeile |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-026a) |
| `.d-check.yml` | ändern | ids-Fehler-Code-Familie um `PLG` erweitern (Review-LOW-2, Verschärfung) |
| `spec/architecture.md` | ggf. ändern | optional `## Geschichte`-Provenance „Plugin-Host/API → [ADR-0017](../../adr/0017-plugin-api-abi.md)"; sonst unverändert |
| `docs/plan/adr/README.md` | ändern (Closure) | [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflicht „AK-Schärfung + Spec-Nachzug" → erfüllt |
| `docs/reviews/{2026-07-02-slice-026a-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: reine Doku/Entscheidung; das
  Plugin-Backend ist mit [ADR-0017](../../adr/0017-plugin-api-abi.md) (accepted 2026-07-02) entschieden; die
  Driving-Ports, die der Plugin-Kontext vermitteln wird, sind real
  (`src/hexagon/ports/driving/`).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz mit Lerneintrag →
  der **PLG-Impl-Slice** (026b) wird startbar (mit eigenem
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review davor).

## 6. Risiken und offene Punkte

- **Lösungsfreiheit des Lastenhefts ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):** „Plugin wird ohne Neustart
  geladen/entladen", „unpassender Vertragsstand wird vor jeder Wirkung
  abgelehnt", „fehlverhaltendes Plugin wird isoliert, Modell unverändert" sind
  benutzer-beobachtbar; **`dlopen`/`dlsym`, `extern "C"`-Eintrittspunkte,
  ABI-/vtable-Mechanik, Handshake-Details, Threading-Modell** sind Mechanik und
  gehören in §1, **nicht** ins Lastenheft. Grenzfall „versionierter
  Vertragsstand": **dass** der Vertrag versioniert ist und Ablehnung ihn nennt,
  ist das **Was** (beobachtbar); **wie** die Version geprüft wird (exakte
  Gleichheit, Eintrittspunkt-Namen), ist das **Wie** (§1).
- **Sandbox-AK vs. Outline-Wortlaut (zentraler Review-Punkt, Muster
  025a-„[ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)-Erfüllbarkeit"):** der Outline-Stub sagt „Plugin darf das
  Modell nicht korruptieren". [ADR-0017](../../adr/0017-plugin-api-abi.md) hat ehrlich benannt, dass die
  stärkste Lesart **in-process prinzipiell nicht einlösbar** ist (kein
  Speicherschutz; Silent-Corruption-Pfad) und die AK auf **beobachtbares
  Fehlverhalten wohlgeformter Plugins** zu beziehen ist. Dieser Slice
  **bestätigt explizit**, dass die AK-Fassung (Isolierung + Modell unverändert
  + gleiche Prüf-/Klemm-Regeln + Ehrlichkeits-Klausel mit
  [LH-QA-005](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery)-Netz) den Vertragskern des Outline-Wortlauts **trägt**
  (Modell-Integrität gegenüber Plugin-Fehlverhalten über die vermittelten
  Wege) und ihn nicht still schwächt. Falls das Review die Fassung als
  **Vertrags-Schwächung** sieht → **HIGH** (dann Scope-Nachschärfung:
  Prozess-Isolation [ADR-0017](../../adr/0017-plugin-api-abi.md)-Option-P-Re-Eval vorziehen statt AK weichzeichnen).
- **[`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Präzisierung (benannter Entscheidungspunkt):** Vorschlag
  **ein Code, zwei Log-Events** (`plugin_rejected` Laden/Handshake vs.
  `plugin_error` Laufzeit) — Parallel-Muster: STEP/STL-/PDF/PNG-Import-Request
  nutzt die **generische** [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Zeile statt eines neuen Codes.
  Alternative (zweiter Code für Load-Ablehnung) ist benannt; das
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review prüft die Wahl gegen die §4-Konvention.
- **Anker-Hebung ohne Link-Bruch:** die PLG-IDs haben heute **keine**
  per-ID-Anker (Modul-Anker-Links im Repo-Korpus). Die Schärfung hebt sie auf
  `####`-Headings mit Inline-HTML-Ankern (Muster
  [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/018c); das Modul-Heading (und damit alle bestehenden
  `#modul-plugin-system-plg`-Links) **bleibt**. Nach der Hebung neue Verweise
  bevorzugt auf die per-ID-Anker; Alt-Links rotten nicht (docs-check prüft).
- **Header-Versions-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):** `**Version:**` der
  `lastenheft.md` auf **0.1.13** == jüngste `lastenheft-historie.md`-Zeile.
  **Ausführungs-Hinweis (Review-INFO-1):** die gelebte Tabellen-Ordnung hängt
  die jüngste Zeile am Tabellen-**Ende** an (nicht „oben") — die 0.1.13-Zeile
  folgt der gelebten Praxis; die Substanz der Invariante (Header == jüngste
  Zeile) ist davon unberührt.
- **Vorgefundene Header-Drift `spezifikation.md` (Review-LOW-3):** der
  `**Letzte Änderung:**`-Header steht auf 2026-06-18, die letzte inhaltliche
  Änderung war slice-025a (2026-07-01) — die dortige DoD-Zeile wurde faktisch
  nicht ausgeführt. Der 026a-Header-Nachzug **heilt** die Drift (→ 2026-07-02);
  im Closure als vorgefundene Drift benennen (zweiter Beleg: dieser Header hat
  keinen computational Sensor — Steering-Kandidat neben der
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)-Promotion).
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** die §1-/§4-/§5-/§6-Einarbeitung der
  frischen [ADR-0017](../../adr/0017-plugin-api-abi.md)-Entscheidung darf **keinen** ADR-Verweis im
  `spezifikation.md`-Körper hinterlassen (Provenance nur in
  `spezifikation-historie.md`) — vor dem Gate per `grep ADR-` selbst fangen
  (Lerneintrag 019a, Muster 020a/021a/025a).
- **Subset-Treue zu [ADR-0017](../../adr/0017-plugin-api-abi.md) (kein Über-Versprechen):** §1 behauptet nur,
  was [ADR-0017](../../adr/0017-plugin-api-abi.md) entschied (in-process, pull-only, kein Beobachter-Zugang,
  kein Speicherschutz, exakte Versions-Gleichheit). Beobachter-Zugang,
  UI-Erweiterungspunkte, Skript-Plugins, Signierung, Fremd-Toolchain-Binaries =
  benannte Lücken/Re-Eval ([ADR-0017](../../adr/0017-plugin-api-abi.md) §Trigger) — werden **nicht**
  spezifiziert.
- **Kein Scope-Creep in die Schwester-Stränge:** DRW/UI/Mehrsprachigkeit
  (welle-5) und [LH-QA-003](../../../../spec/lastenheft.md#lh-qa-003--undoredo) (Undo, unimplementiert —
  [ADR-0017](../../adr/0017-plugin-api-abi.md)-Review-HIGH-1-Lehre: nicht als Ist behaupten) bleiben
  unberührt.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; **Konventionen-Dichte:** hoch — AK-Format (Happy/Boundary/
  Negative), Reifephase-/Teilumfang-Klausel, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) (lösungsfrei),
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012 (Header == Historie), Fehler-Code-Konvention
  ([`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) besteht). **Phase-Reife:** PLG Phase 2 (Outline → AK).
  **Evidenz-/Diskrepanz-Risiko:** niedrig (reine Doku; [ADR-0017](../../adr/0017-plugin-api-abi.md) trägt die
  Backend-Entscheidung). **Reconciliation:** keiner; Folge-Slice = PLG-Impl
  (026b).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; **Dichte:** hoch ([ADR-0017](../../adr/0017-plugin-api-abi.md)-Leitplanke: kein neuer
  Grundsatz-ADR; Reifephase-Teilumfang wie die welle-4-Strang-Muster;
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor Impl). **Phase-Reife:** Phase 4. **Risiko:** niedrig.
