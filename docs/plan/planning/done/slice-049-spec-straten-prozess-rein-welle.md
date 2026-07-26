---
id: slice-049
titel: Spec-Straten prozess-/zeit-rein — »welle-N«-Purge (lastenheft/spezifikation/architecture) + `matrix`-Gate-Härtung
status: done
welle: welle-5-erweiterung
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md)]
---

# Slice 049: Spec-Straten prozess-/zeit-rein — »welle-N«-Purge + matrix-Gate-Härtung

**Status:** **done** (Closure 2026-07-26, s. §8).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review**
2026-07-24 (Reviewer ≠ Autor): **0 HIGH / 3 MED / 2 LOW / 1 INFO → startbar**
([Report](../../../reviews/2026-07-24-slice-049-plan.md); MED-1 [`Welle-1`/`Welle-1v`-Operationalisierungs-Paar
als load-bearing, gezielte Kern-/Viewer-Umschrift], MED-2 [Historie-Provenance-Asymmetrie **strukturell**
begründen], MED-3 [neue-MR-Forward-Reference als `ids`-Falle → Nummer-frei formuliert], LOW-1/LOW-2 eingearbeitet).

**Welle:** welle-5-erweiterung (Quergewerk / **harness-steering + Doku-Hygiene** — Muster
[slice-036](slice-036-planning-lifecycle.md)-artige Gate-Adoption: eine gelebte, aber **nicht
computational** durchgesetzte Stratifizierungs-Regel wird maschinell + der Ist-Verstoß bereinigt).

**Auslöser (Projektinhaber, 2026-07-24):** Prozess-/Planungs-Vokabular **»welle-N«** (Projekt-Abwicklungs-
Wellen — die **zeitliche/Planungs-Schicht**) leckt in die drei **Spec-Straten**
(`spec/lastenheft.md`, `spec/spezifikation.md`, `spec/architecture.md`) — **58 Fundstellen**. Das
verletzt die **Stratifizierungs-/Referenz-Richtungs-Disziplin**
([MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)/[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):
Spec-Straten beschreiben das **Was/Wie** (fortschreibbar, zeit-los), **nicht** die Projekt-Zeitachse.
`spec/architecture.md` **Zeile 6 formuliert das Prinzip sogar selbst** (»…keine **Wellen**, Slices,
Commit-Hashes oder Closure-Daten«) — und **Zeile 93 verletzt es** (»welle-3 zurückgestellt«). **Kritisch:**
die »scharf« geschalteten Module `matrix`/`ids` **fangen das nicht** — echte **Regel-Lücke** (s. §1).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

**Bewusst NICHT Teil (benannte Grenzen):**

- **ADRs / Planning-Slices / Roadmap / Historie-Dateien** — dort ist »welle-N« **legitim** (die Roadmap
  **ist** die zeitliche Schicht; ADRs tragen historische »welle-N-Trigger«). Nur die **drei Spec-Straten**
  werden gereinigt und geprüft. Eine spätere `adr → temporal`-Ausweitung ist ein **benannter Re-Eval**
  (ADR-Körper tragen z. T. legitime historische Wellen-Trigger → eigene Abwägung), **nicht** dieser Schnitt.
- **Kein inhaltlicher Bedeutungs-Wandel** — die Reifephase-/Teilumfang-**Aussagen bleiben**; nur der
  Wellen-**Tag** wird durch prozess-freie Formulierung ersetzt (»Reifephase-Teilumfang«/»aktuelle
  Ausbaustufe«/Lastenheft-Versions-Bezug/»benannte Lücke«). **Keine** Anforderungs-Änderung → **kein**
  Lastenheft-Versions-Bump nötig (rein editorial; s. §6 Rest-Risiko).

---

## 1. Ziel

Die drei Spec-Straten werden **prozess-/zeit-rein**: **kein** Wellen-Identifikator »welle-N« mehr (außer in
den ausgenommenen Provenance-Abschnitten `Historie`/`Geschichte`). Zugleich wird die Regel **computational**
— `matrix` erkennt künftig `welle-\d` in Spec-Straten als **`matrix-forbidden`**, analog zur
`slice-\d{3}`-Token-Verschärfung (2026-07-05).

**Warum die heutigen Gates es nicht fangen (verifiziert in `.d-check.yml`):**
`matrix` verbietet in Spec-Straten nur (a) Markdown-**Links** abwärts auf `adr`/`slice` und (b) den bare
Token **`slice-\d{3}`** (auf der `slice`-Klasse). `welle-5` matcht `slice-\d{3}` **nicht** und ist kein Link.
`ids` erzwingt Link-Pflicht nur für die 7 ID-Familien — »welle« ist keine. → **Regel-Lücke, keine
Fehlkonfiguration.**

**Empirisch bestätigte Gate-Mechanik (Fixture-Test 2026-07-24, gepinnte d-check v0.51.1):** eine neue
`matrix`-Klasse **`temporal`** (paths = die Roadmap, wo »welle-N« legitim wohnt) mit **`token: '[Ww]elle-\d'`**
+ Regel `{from: spec-straten, to: temporal, allow: false}` meldet `welle-4`/`Welle-1`/`welle-1v` in einem
Spec-Stratum als **`matrix-forbidden`** und respektiert `exclude-sections: [Historie, Geschichte, …]`
(der `## Geschichte`-Abschnitt bleibt ungeflaggt). `[Ww]` deckt Groß-/Kleinschreibung.

## 2. Definition of Done

- [x] **`.d-check.yml` `matrix` gehärtet:** neue Klasse `temporal` (paths = `docs/plan/planning/in-progress/roadmap.md`,
      `token: '[Ww]elle-\d'`) + Regel `{from: spec-straten, to: temporal, allow: false}`; Kommentar-Block
      (Normativität: [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths),
      Muster der bestehenden `slice`-Token-Verschärfung). **Baseline-Beleg:** vor der Bereinigung schlägt die
      Regel an (Fundstellen-Liste = Arbeitsvorrat); nach der Bereinigung **0 Befunde**.
- [x] **`spec/lastenheft.md` purge (18):** Wellen-Tags prozess-frei ersetzen — »Teilumfang (welle-N):« →
      »Teilumfang:« bzw. »Reifephase-Teilumfang:«; »Welle-1-Anforderungen« → »die bereits AK-geschärften
      Anforderungen«; »(spätere Welle)« → »(spätere Ausbaustufe)«. **Bedeutung unverändert.**
- [x] **`spec/spezifikation.md` purge (38):** analog — »Welle-1-Einschränkung/Näherung« →
      »Reifephase-Einschränkung/Näherung«; »Teilumfang welle-N« → »Teilumfang« (der oft **schon danebenstehende
      Lastenheft-Versions-Bezug** [»Lastenheft 0.1.4«] trägt die Provenance prozess-frei); **load-bearing
      Zeiger sorgfältig, KEINE generische Umschrift:**
      (a) **`Welle-1-Operationalisierung`/`Welle-1v-Operationalisierung` (Z. 382/388, MED-1)** — das `v`-Suffix
      kodiert die **Kern-vs-Viewer**-Unterscheidung ([ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Erfüllung); das generische »Reifephase-«-Muster
      würde beide **kollabieren**. Gezielt: Z. 382 → »**Kern-Operationalisierung von „sichtbar"**«, Z. 388 →
      »**Viewer-Operationalisierung von „sichtbar"**«;
      (b) `welle-1v-viewer` (Roadmap-Strang-Name) → »der Viewer-Strang«;
      (c) durchgestrichener »ADR in welle-4-austausch« (Z. 1161) → »ADR entschieden«.
      **Kein Blind-`sed`-Replace — je Zeile einzeln.**
- [x] **`spec/architecture.md` purge (1):** Z. 93 »welle-3 zurückgestellt« → »in dieser Ausbaustufe
      zurückgestellt«. **Z. 6 (Prinzip-Satz »keine Wellen…«) bleibt** — er ist die Regel-Aussage und matcht
      `[Ww]elle-\d` ohnehin nicht (kein Digit).
- [x] **Nicht-`-\d`-Prozess-Reste (Gate-blind, dennoch bereinigen):** »spätere Welle« (lastenheft Z. 73),
      »Wellen«-Prosa außerhalb des Prinzip-Satzes — manuell prozess-frei; im Plan-Review benennen (die Gate-
      Regel deckt nur den **Identifikator** `welle-\d`, nicht jede Prosa; bewusste, benannte Grenze).
- [x] **`make gates` grün** (`matrix` 0 Befunde über die 3 Straten; `schema-check` byte-unberührt; **keine**
      Code-/Schema-Berührung). Selbst-Gegenprobe: `grep -niE '[Ww]elle-[0-9]' spec/{lastenheft,spezifikation,architecture}.md`
      = 0 außerhalb `Geschichte`/`Historie`.
- [x] **Neue MR-Konvention »Spec-Straten sind prozess-/zeit-rein«** (Entscheidung MED-3 festgenagelt:
      **eigene neue MR**, distinktes Prinzip wie [MR-014](../../../../harness/conventions.md) adr↛slice — **nicht**
      nur [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)-Ergänzung; **Reifephase-Reflexions-Pflicht**: Wellen-/Slice-/Datums-Marker gehören in
      Planning/Roadmap, nie in die Straten; Sensor = das `matrix`-`temporal`-Gate). **Nummer + Anker erst BEI
      Ausführung vergeben** und dann **alle** Vorkommen verlinkt (kein bare `MR-\d{3}`-Token im Plan/Slice-File
      → sonst `ids`-`id-unlinked` bzw. `anchor-missing`; MED-3). Header/Datum der Konvention nach
      [MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Muster.
- [x] **Provenance — bewusst asymmetrisch (MED-2, strukturell begründet):** **`spec/spezifikation-historie.md`**
      (eine **datums-indizierte** freie Provenance-Zeile → editorial-Eintrag zulässig **ohne** Versions-Bump)
      **und** **`architecture.md` `## Geschichte`** (freier Provenance-Abschnitt) bekommen je eine Zeile;
      **`spec/lastenheft-historie.md` bekommt KEINE** — sie ist **versions-indiziert** (`| Version | Datum | … |`),
      ein Eintrag erzwänge einen `**Version:**`-Bump, den der rein editorial Purge nicht rechtfertigt (und die
      [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)-
      Invariante würde ihn verlangen). Die Lastenheft-Provenance trägt statt dessen **CHANGELOG + slice-049 +
      Commit**. Die Asymmetrie ist damit **nicht willkürlich**, sondern folgt der Struktur der zwei Historien.
- [x] **CHANGELOG** [Unreleased]-Eintrag (deckt die Lastenheft-Provenance mit ab). **Kein**
      Lastenheft-`**Version:**`-Bump (rein editorial — s. §6 Rest-Risiko #3).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.d-check.yml` | ändern | `matrix.classes` += `temporal` (roadmap, `token '[Ww]elle-\d'`) + `rules` += `{spec-straten→temporal: false}` |
| `spec/lastenheft.md` | ändern | 18 Wellen-Tags → prozess-frei (Bedeutung unverändert) |
| `spec/spezifikation.md` | ändern | 38 Wellen-Tags → prozess-frei (load-bearing Zeiger sorgfältig) |
| `spec/architecture.md` | ändern | Z. 93 Wellen-Tag → prozess-frei (Z. 6 Prinzip bleibt) |
| `spec/spezifikation-historie.md` | ändern | datums-indizierte Provenance-Zeile slice-049 (editorial) |
| `harness/conventions.md` | ändern | **neue MR** »Spec-Straten prozess-/zeit-rein« (Nummer bei Ausführung; Muster [MR-014](../../../../harness/conventions.md)) |
| `spec/lastenheft-historie.md` | **unberührt** | versions-indiziert → kein Eintrag ohne Versions-Bump (MED-2) |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |
| `docs/reviews/2026-07-24-slice-049-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Projektinhaber-Beschluss 2026-07-24 (»welle hat in den Spec-Dokumenten nichts verloren« + »das Gate
  erkennt es nicht«). Gate-Mechanik empirisch bestätigt (§1). Nach eigenem
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  (0 HIGH) → **startbar**. **Sequenz:** VOR slice-048b (DRW-001-Impl).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (`matrix` 0 über die Straten), Selbst-`grep` sauber, Closure-Notiz.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — Purge + Gate müssen ZUSAMMEN landen (Reihenfolge):** die neue `matrix`-Regel macht
  **jede** verbliebene `welle-\d`-Fundstelle zum Fehler → die Regel darf **nicht** vor der Bereinigung grün
  sein. In **einem** Slice/Arbeitsbaum kein Problem (Gate prüft den End-Zustand); beim Commit die Härtung +
  Purge **im selben Commit** (kein Zwischenzustand mit rotem Gate).
- **Rest-Risiko #2 — Bedeutungs-Erhalt (zentraler Review-Punkt):** die Umformulierung darf die
  **Reifephase-/Teilumfang-Aussage nicht verwässern** (z. B. »Teilumfang welle-2: gerade einläufige Treppe«
  → »Teilumfang: gerade einläufige Treppe« — der Vollumfang-offen-Sinn bleibt). Mitigation **je Datei
  unterschiedlich (LOW-1):** in **`spezifikation.md`** tragen die daneben stehenden **Lastenheft-Versions-Bezüge**
  (»Lastenheft 0.1.6«) die Stufungs-Provenance prozess-frei weiter; in **`lastenheft.md`** steht **kein** solcher
  Bezug neben den `Teilumfang (welle-N)`-Tags — dort hält »Teilumfang:« **allein** die »bewusst partiell«-Semantik,
  und die Stufungs-Zuordnung wandert (per Prinzip) in die Roadmap/Historie. Je Zeile einzeln prüfen, **kein**
  globaler `sed`-Replace.
- **Rest-Risiko #3 — Editorial ⇒ Lastenheft-Version?** Der Purge ändert **keine Anforderung** (nur Prozess-
  Vokabular). Vorschlag: **kein** `**Version:**`-Bump (Muster: reine Referenz-/Formulierungs-Korrektur), die
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)-
  Invariante (Header == jüngste Historie-Zeile) bleibt **unberührt** (kein neuer Historie-Eintrag). **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  bestätigt/verwirft** (Alternative: patch-artiger Eintrag ohne AK-Wirkung).
- **Rest-Risiko #4 — Gate-Token-Reichweite:** `[Ww]elle-\d` fängt den **Identifikator**, nicht Prosa wie
  »spätere Welle«/»Wellen«. Bewusste, benannte Grenze (die Prosa-Reste einmalig manuell; der Prinzip-Satz
  arch Z. 6 bleibt). Ein breiteres `[Ww]elle` würde den Prinzip-Satz fälschlich flaggen → verworfen (oder
  erforderte dessen Umformulierung — Overkill).
- **`temporal`-Klasse-Semantik (LOW-2 — rein deklarativ):** `paths: [roadmap]` deklariert `temporal` nur als
  »reale« Klasse, in der `[Ww]elle-\d` legitim wohnt; die Token-Verbots-Mechanik feuert auf den **`from`**-
  Dokumenten (spec-straten) via Regel, **unabhängig** von den `to`-`paths` → die Roadmap wird dadurch **nicht**
  zusätzlich geprüft (es gibt **keine** `{from: temporal, …}`-Regel). Im `.d-check.yml`-Kommentarblock explizit
  klarstellen (kein Folge-Autor soll eine Roadmap-Prüfung hineinlesen). Regel greift **nur** `from: spec-straten`
  → Slices/ADRs/Historie unberührt. Fixture-verifiziert.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Gate-Konfiguration (`.d-check.yml`)

- **Modus:** GF; **Dichte:** hoch (Token-Klasse + Regel; Baseline-Beleg vor/nach; Fixture-verifiziert).
  **Phase-Reife:** Referenz-Integritäts-Gate reif (matrix/slice-Token-Präzedenz). **Risiko:** niedrig
  (additive Regel; Gegenprobe grün/rot vorhanden).

### Sub-Area: Spec-Purge (3 Straten)

- **Modus:** GF; **Dichte:** hoch (bedeutungs-erhaltende Umformulierung, load-bearing Zeiger, kein
  Blind-Replace, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-
  Geist: das *Was* bleibt, der Prozess-Tag geht). **Phase-Reife:** Doku-Hygiene. **Risiko:** niedrig-mittel
  (57 Einzelstellen; Selbst-`grep` als Netz).

## 8. Closure-Notiz

**Ausgeführt 2026-07-26.** Sensor-Läufe (selbst gefahren, nur `make`-Targets):

| Lauf | Exit | Kennzahlen |
|---|---|---|
| `make gates` | **0** | `docs-check`/d-check **248 Dateien, 0 Befunde** · `a-check` 0 · `arch-check` ok · `lint` 0 + „suppression-gate ok" · `test` **285/285** · `coverage-gate` **91,4 %** |
| `make schema-check` | **0** | „schema.sql == d-migrate(data-model.yaml)" — **keine** Schema-/Code-Berührung |
| **Baseline-Beleg** (neue Regel gegen den **unbereinigten** Stand) | **rot** | **56** `matrix-forbidden` — lastenheft **17** · spezifikation **38** · architecture **1**. Die Fundstellen-Liste **war** der Arbeitsvorrat. |
| **Nach-Messung** | **0** | dieselbe Regel, bereinigter Stand: 0 Befunde |
| Selbst-Gegenprobe | — | `grep -niE '[Ww]elle-[0-9]'` über die drei Straten = **0** |

**Ein Plan-Befund, der erst in der Ausführung sichtbar wurde (Abweichung, begründet):**
Der Plan (§1 + LOW-2) sah die `temporal`-Klasse mit `paths: [roadmap.md]` vor. Gemessen: damit meldet die
Regel **57** statt 56 Befunde — der Zusatzbefund ist der **legitime Markdown-Link**
`spec/architecture.md` → Roadmap in genau dem Hard-Rule-Satz, der auf die zeitliche Schicht **verweisen
muss** („die zeitliche Schicht lebt in …"). Mit leeren `paths` bleibt die Token-Wirkung vollständig und
der Link heil. **Umgesetzt: `paths: []`** — die Klasse ist eine reine Token-Trägerin; die Deklaration, *wo*
»welle-N« legitim wohnt, steht als Prosa im Konfigurations-Kommentar und in
[MR-023](../../../../harness/conventions.md#mr-023--spec-straten-sind-prozess-zeit-rein-d-check-matrix-klasse-temporal),
nicht als `paths`-Eintrag. Die LOW-2-Aussage des Reviews („rein deklarativ, die Roadmap wird nicht
zusätzlich geprüft") war für die **Token**-Mechanik richtig und für die **Link**-Mechanik falsch.

**Zweiter Plan-Befund:** die DoD verlangte eine Provenance-Zeile in `architecture.md ## Geschichte` als
„freiem Provenance-Abschnitt" (MED-2). Der Abschnitt ist aber eine **ADR-typisierte Tabelle**
(`Architektur-Aspekt | Prägende ADR`) — ein Purge-Eintrag hat dort keine Spalte. Umgesetzt als
**Prosa-Zeile in der Präambel** desselben Abschnitts (gate-seitig unbedenklich: `Geschichte` steht in
`matrix.exclude-sections`, der Slice-Zeiger trägt zusätzlich den `status-provenance`-Marker). Die
Tabellen-Semantik bleibt unangetastet.

**Geliefert:**

- **56 Fundstellen bereinigt, keine Aussage-Änderung.** Statt der Welle wird die **Eigenschaft** benannt:
  „Teilumfang", „Reifephase-Einschränkung/-Näherung", „in dieser Ausbaustufe", „benannte Lücke".
- **Die zwei load-bearing Zeiger gezielt statt generisch** (MED-1): `Welle-1`/`Welle-1v`-Operationalisierung
  von „sichtbar" → **Kern**- bzw. **Viewer**-Operationalisierung. Das generische »Reifephase-«-Muster hätte
  beide kollabiert und die Kern-vs-Viewer-Unterscheidung
  ([ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)) zerstört — der Grund, warum hier **kein**
  `sed` lief, sondern jede Zeile einzeln.
- **Gate-Härtung** (`temporal`-Klasse + Regel) — die Regel ist damit **computational**, nicht mehr nur in
  `architecture.md` behauptet. **Verschärfung, kein Carveout** ([§2.6](../../../../AGENTS.md) n/a).
- **[MR-023](../../../../harness/conventions.md#mr-023--spec-straten-sind-prozess-zeit-rein-d-check-matrix-klasse-temporal)**
  aufgenommen (Nummer + Anker erst hier vergeben, MED-3) und in **beiden** Honesty-Dokumenten
  ([`AGENTS.md` §3](../../../../AGENTS.md), [`harness/README.md`](../../../../harness/README.md)) als
  `docs-check`-Vertragsteil geführt.
- **Provenance bewusst asymmetrisch** (MED-2): `spezifikation-historie.md` (datums-indiziert) + die
  `architecture.md`-Präambel bekommen eine Zeile; **`lastenheft-historie.md` bleibt unberührt** — sie ist
  **versions-indiziert**, ein Eintrag erzwänge über
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)
  einen `Version:`-Bump, den ein rein editorialer Purge nicht rechtfertigt. Lastenheft-Provenance trägt
  CHANGELOG + Commit. **Kein** Versions-Bump (Rest-Risiko #3 damit entschieden: bestätigt).
- **Purge + Härtung in EINEM Commit** (Rest-Risiko #1) — kein Zwischenzustand mit rotem Gate.

**Lerneintrag:** Eine Regel, die ein Dokument **über sich selbst** aufschreibt, ist keine Regel — sie ist
eine Absichtserklärung. `spec/architecture.md` trug den Satz „keine Wellen, Slices, Commit-Hashes oder
Closure-Daten" in seiner Hard-Rule und verletzte ihn in derselben Datei; über alle drei Straten hinweg
56-fach. Erst der Sensor macht daraus eine Invariante. Zweite Lehre, aus der `paths`-Abweichung: ein
Gate-Mechanismus mit **zwei** Erkennungswegen (Token **und** Link) trifft mit einer Konfiguration beide —
wer nur den einen im Blick hat, baut sich ein Falsch-Positiv auf einer legitimen Referenz. Das fällt nur
auf, wenn man die Baseline **wirklich** fährt statt sie zu plausibilisieren.

**Folge:** Die `adr → temporal`-Ausweitung bleibt ein **benannter Re-Eval** (ADR-Körper tragen z. T.
legitime historische Wellen-Trigger — eigene Abwägung, nicht dieser Schnitt). Nächster Faden laut
Plan-Sequenz: `slice-048b` (DRW-001-Impl).
