---
id: slice-027
titel: lint-Härtung — kuratierte clang-tidy-Familien-Erweiterung (cert/readability/performance/modernize/misc/cppcoreguidelines/portability), evidence-first
status: done
welle: harness-steering
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md), [ADR-0017](../../adr/0017-plugin-api-abi.md)]
---

# Slice 027: lint-Härtung — kuratierte clang-tidy-Familien-Erweiterung (evidence-first)

**Status:** **done** (2026-07-04) — Dry-Run vermessen (**1630 Befunde / 7 Familien**,
0 Parse-Fehler; Beleg [dryrun](../../../reviews/2026-07-04-slice-027-dryrun.md));
**8 Checks aktiviert + 10 Befunde gefixt** (verhaltensneutral), 24 Auslassungen
begründet, 0-Befund-Checks scharf (Falsifizierbarkeits-Invariante). `make gates`
grün, `make test` unverändert 228/228. — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
**0 HIGH / 3 MED / 4 LOW / 2 INFO**; Start **nicht blockiert**, alle Findings
vor Start eingearbeitet: MED-1 (Dry-Run-Regelkonformität auf die tragende
Begründung gestellt — Image-Pinnung statt falscher 025b/c-Präzedenz) +
MED-2 (Sub-Area-Deklaration der Fix-Pfade) + MED-3
(`--warnings-as-errors='*'`-Mechanik: keine Beobachtungsphase, Aktivierung
nur nach Fix) + LOW-1 (HeaderFilterRegex-Vorbehalt) + LOW-2
(Kommando-Protokoll im Beleg) + LOW-3 (kumulativer Fix-Budget-Escape ~60) +
LOW-4 (Beleg-Ablage als benannte Ausnahme) + INFO-2
(Falsifizierbarkeits-Invariante: 0-Befund-Checks sind aktiviert oder
begründet ausgelassen). Alle vier Entscheidungspunkte vom Review
**bestätigt** (kein ADR; Deckel vertretbar; docker-run-Analyse
regelkonform; Ein-Slice-Schnitt mit Escape).
[Report](../../../reviews/2026-07-03-slice-027-plan.md).

**Welle:** harness-steering (Quergewerk, Muster slice-018a–c/022 —
**welle-5-Scope unberührt**, kein Wellen-Feature; Roadmap erhält den
üblichen Quergewerk-Eintrag in „Historische Trigger-Verschiebungen").

**Bezug:** `make lint`-Gate (Vertrag: clang-tidy **0 Befunde** in `src/` +
`plugins/` + Suppression-Gate; Bindung
[ADR-0001](../../adr/0001-hexagonale-architektur.md) §Fitness,
[AGENTS.md §2.4](../../../../AGENTS.md)). Heutiger Check-Satz (seit
slice-002): `bugprone-*` + `clang-analyzer-*` +
`readability-function-cognitive-complexity`.
**Projektinhaber-Anstoß (2026-07-03, slice-026b-Closure §Steering):** die
Familien **`cert-*`, `readability-*`, `performance-*`, `modernize-*`,
`misc-*`, `cppcoreguidelines-*`, `portability-*`** prüfen und **kuratiert**
scharfschalten. [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)
(gepinnte Toolchain: clang-tidy/clang-21 — der Familien-Umfang ist
versionsgebunden), [ADR-0017](../../adr/0017-plugin-api-abi.md) (bekannter
Kollisionspunkt dlsym-Idiom im Plugin-Host).

**Kein ADR (begründet):** Aktivierung zusätzlicher Checks ist eine
**Verschärfung** — [AGENTS.md §2.6](../../../../AGENTS.md) verlangt ADRs nur
für Gate-**Lockerungen**; der Check-Satz ist Slice-Scope (Präzedenz:
slice-002 setzte den Initial-Satz ohne ADR; ids-Familie +PLG in 026a und
lint-Scope +`plugins/` in 026b je als „Verschärfung, kein §2.6-Fall").
`.clang-tidy` verlangt für zentrale Ausnahmen „ADR- **oder** Slice-ID" —
Slice-Deckung genügt.

**Autor:** Dietmar Burkard. **Datum:** 2026-07-03.

**Schnitt-Herkunft:** Quergewerk-Steering aus der 026b-Closure. **Ein**
Slice, weil Dry-Run und Aktivierung nur zusammen sinnvoll sind (die
Aktivierungsliste **ist** das Ergebnis des Dry-Runs) — mit hartem
**Umfangs-Deckel** (§1), damit der Schnitt sitzungstauglich bleibt.
**Nicht Teil:** Massen-Modernisierung des Bestands (Checks mit
Großbefund-Lage werden **nicht** aktiviert, sondern als benannte
Folge-Kandidaten dokumentiert), Schnittstellen-Änderungen an Ports
(z. B. Signatur-Umbauten wegen `performance-unnecessary-value-param`),
`readability-identifier-naming` (kein deklarierter Naming-Standard),
Änderungen an Schwelle/Scope bestehender Gates.

---

## 1. Ziel

Der `make lint`-Gate wird **evidence-first** gehärtet: erst ein
**Dry-Run-Bestandslauf** aller sieben Kandidaten-Familien über `src/` +
`plugins/` im gepinnten Container (Befund-Matrix Check → Anzahl als Beleg),
dann eine **kuratierte Aktivierungsliste** nach benannten Kriterien, Fixes
der Befunde der aktivierten Checks, und ein `.clang-tidy`, das jede bewusste
Auslassung begründet. Die 0-Befunde-Latte (rot/grün, keine Vorschläge)
bleibt unverändert; es wird nichts gelockert.

**Kurations-Kriterien (entschieden in diesem Plan):**

1. **Aktivieren**, wenn der Check im Bestand **0 Befunde** hat **oder**
   wenige, klar wertvolle Befunde (echte Fehler-/Klarheitsgewinne), die in
   diesem Slice gefixt werden.
2. **Umfangs-Deckel:** Checks mit **> 20 Befunden** werden in diesem Slice
   **nicht** aktiviert (kein Massen-Churn, keine Auto-Fix-Anwendung über
   den Bestand) — sie landen als benannte Folge-Kandidaten in der
   Closure-Notiz. **Kumulativer Escape (Review-LOW-3):** übersteigt die
   Summe der Fixes über alle Aktivierungs-Kandidaten **~60**, wird nach
   Wert priorisiert und der Rest als Folge-Kandidat benannt (Split statt
   Sitzungssprengung; die Zahlen sind Sitzungs-Heuristiken, keine
   hergeleiteten Grenzen).
3. **Alias-Deduplizierung:** viele `cert-*`-/`cppcoreguidelines-*`-Checks
   sind **Aliase** bereits aktiver `bugprone-*`-/`misc-*`-Checks
   (Doppel-Meldung desselben Befunds unter zwei Namen). Es wird der
   **Original-Check** aktiviert, Aliase bleiben aus (Begründung im
   `.clang-tidy`-Kommentar).
4. **Idiom-Kollisionen** werden nicht still weggefiltert: ein Check, der
   mit einem tragenden Idiom kollidiert (bekannt:
   `cppcoreguidelines-pro-type-reinterpret-cast` vs. das dlsym-Idiom des
   Plugin-Hosts; `cppcoreguidelines-owning-memory` vs. die
   Factory/Destroy-Paare der Plugin-ABI; `readability-magic-numbers` vs.
   Test-/Fixture-Literale), wird **entweder** mit zentraler, begründeter
   Ausnahme aktiviert (nur wenn `CheckOptions` das sauber tragen) **oder**
   begründet ausgelassen — Entscheidung je Check anhand der Matrix.
5. **Stil-Geschmacks-Checks** ohne Fehler-Fang-Wert und mit Format-Churn
   (`modernize-use-trailing-return-type`,
   `readability-braces-around-statements`-Familie,
   `readability-identifier-length` u. ä.) werden ausgelassen (benannt).

## 2. Definition of Done

- [ ] **Dry-Run-Bestandslauf (Beleg):** je Kandidaten-Familie ein
      clang-tidy-Lauf mit `-checks='-*,<familie>'` über die
      `compile_commands.json` des `bcad:build`-Images (gepinnte Toolchain,
      [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md); Aufruf im
      Container — kein Host-Tooling, [AGENTS.md §2.9](../../../../AGENTS.md)),
      Scope `src/` + `plugins/`. Ergebnis: **Befund-Matrix**
      (Check → Befundanzahl, je Familie) als Beleg-Report
      `docs/reviews/2026-07-04-slice-027-dryrun.md` — **inkl. der exakten
      `docker run`-Kommandos** (Reproduzierbarkeit, Review-LOW-2); die
      Ablage unter `docs/reviews/` ist eine **benannte Ausnahme**
      (Evidenz-Beleg statt Review — Review-LOW-4; Ort gewählt wegen der
      ids-Exempt-Klasse für Beleg-/Report-Prosa).
- [ ] **Kuratierte Aktivierungsliste** aus der Matrix nach den fünf
      §1-Kriterien; `.clang-tidy` erweitert. **Mechanik-Klarstellung
      (Review-MED-3):** die lint-Stage promotet per CLI
      `--warnings-as-errors='*'` **jeden** zu `Checks:` hinzugefügten Check
      **sofort** zum harten Fehler — es gibt keine „Beobachtungsphase";
      Aktivierung erfolgt daher erst **nach** dem Fix der Befunde. Der
      `WarningsAsErrors:`-Block wird als Hygiene synchron gehalten (nicht
      als Mechanismus). Je Familien-Block ein Kommentar mit Slice-Bezug;
      bewusste Auslassungen (Alias/Deckel/Idiom/Stil) als Kommentar
      begründet (kein stilles Auslassen). **Falsifizierbarkeits-Invariante
      (Review-INFO-2):** jeder Check mit **0 Matrix-Befunden** ist
      aktiviert, sofern kein Kriterium-3/4/5-Ausschluss (Alias/Idiom/Stil)
      greift — jede solche Auslassung ist begründet; die Aktivierungsliste
      ist gegen die Matrix prüfbar.
- [ ] **Befunde der aktivierten Checks gefixt** — Behandlung statt
      Suppression (kein NOLINT, [AGENTS.md §2.4](../../../../AGENTS.md);
      Muster 026b `bugprone-empty-catch`); `make lint` grün (0 Befunde).
- [ ] **Gate-Verträge unverändert wahr:** die lint-Zeile in
      [AGENTS.md §3](../../../../AGENTS.md) /
      [`harness/README.md`](../../../../harness/README.md) §Sensors nennt
      keine Check-Familien (nur „clang-tidy, 0 Befunde") — **kein**
      Doku-Nachzug nötig; Schwelle/Scope unverändert (Verschärfung, kein
      [AGENTS.md §2.6](../../../../AGENTS.md)-Fall, kein ADR, kein Carveout).
- [ ] **Roadmap:** Quergewerk-Eintrag in „Historische Trigger-Verschiebungen"
      (Muster slice-018/022; welle-5-Sequenz unberührt).
- [ ] `make gates` grün; **kein** Code-Verhalten geändert (nur Lint-Fixes —
      falls ein Fix Verhalten berühren würde, wird der Check stattdessen
      ausgelassen und benannt); `make test` 228/228 unverändert grün;
      CHANGELOG; Closure-Notiz mit Befund-Matrix-Referenz, Aktivierungs-/
      Auslassungs-Bilanz und Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/reviews/2026-07-04-slice-027-dryrun.md` | neu | Beleg: Befund-Matrix je Familie/Check (Dry-Run im gepinnten Container) |
| `.clang-tidy` | ändern | kuratierte Familien-Erweiterung (`Checks` + `WarningsAsErrors` synchron; begründete Auslassungen als Kommentar mit Slice-Bezug) |
| `src/**` · `plugins/**` (punktuell) | ändern | Fixes der Befunde aktivierter Checks (Behandlung, kein NOLINT; kein Verhaltens-Umbau) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Quergewerk-Eintrag „Historische Trigger-Verschiebungen" |
| `CHANGELOG.md` | ändern (Closure) | Unreleased-Eintrag slice-027 |
| `docs/reviews/2026-07-03-slice-027-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Startbar nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review;
  keine fachliche Vorbedingung (Quergewerk). Projektinhaber-Anstoß liegt vor
  (2026-07-03; Familien-Liste benannt).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz mit Befund-Matrix-Bilanz
  → welle-5-Arbeit läuft unverändert weiter (DRW/UI/Mehrsprachigkeit bzw.
  Welle-Closure-Entscheidung des Projektinhabers).

## 6. Risiken und offene Punkte

- **Unbekannte Befund-Lage (der Kern-Zweck des Dry-Runs):** die Matrix kann
  je Familie von 0 bis hunderten Befunden reichen. Der **Umfangs-Deckel**
  (§1-Kriterium 2) hält den Slice sitzungstauglich: Großbefund-Checks
  werden dokumentiert ausgelassen, nicht abgearbeitet. Damit ist die DoD
  auch bei ungünstiger Matrix erfüllbar (die Bilanz ist das Deliverable,
  nicht eine Mindest-Anzahl aktivierter Checks).
- **Alias-Doppelungen:** `cert-*`/`cppcoreguidelines-*` spiegeln viele
  `bugprone-*`-Checks als Aliase — ohne Deduplizierung meldete der Gate
  denselben Befund doppelt und die Familien-Zahl der Matrix überzeichnete.
  Kriterium 3 + Kommentar-Pflicht decken das.
- **Versions-Bindung der Matrix:** der Familien-Umfang gilt für die
  **gepinnte** clang-tidy-Version (clang-21,
  [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)); ein
  Toolchain-Hub kann neue Checks bringen — der Hub-Beschluss ist der
  benannte Prüfpunkt (kein eigener Mechanismus in diesem Slice).
- **Bekannte Idiom-Kollisionen (aus 026b):**
  `cppcoreguidelines-pro-type-reinterpret-cast` und
  `cppcoreguidelines-owning-memory` treffen den frisch gelieferten
  Plugin-Host (dlsym-Funktionszeiger-Casts; `new`/`delete` in den
  Plugin-Factory/Destroy-Paaren — dort **vertraglich** roh, ABI-Grenze).
  Erwartung: Auslassung mit Begründung; eine Aktivierung mit
  Options-Ausnahme nur, wenn sie den Vertragskern nicht durchlöchert.
- **Verhaltens-Risiko von Lint-Fixes:** Fixes bleiben mechanisch/lokal
  (z. B. `emplace_back`, `const&`, Include-Hygiene); jeder Fix läuft durch
  die volle Test-Suite (228/228 muss stehen). Ein Check, dessen Fix
  Verhalten oder eine Port-Signatur ändern würde, wird ausgelassen und
  benannt (§2-DoD-Zeile) — Schnittstellen-Umbau wäre ein eigener Slice.
- **Dry-Run-Mechanik ohne neues Make-Target (Review-MED-1 präzisiert):**
  der Bestandslauf ist eine **einmalige Analyse**, kein Sensor — er läuft
  als `docker run` gegen das gebaute `bcad:build`-Image. **Regelkonform,
  weil:** (a) clang-tidy ist **im Image gepinnt** — die
  [AGENTS.md §2.9](../../../../AGENTS.md)-Intention („nur container-gepinnte
  Werkzeuge, kein Host-Tooling") ist erfüllt; der PreToolUse-Guard erlaubt
  `docker run <image> <tool>` ausdrücklich; (b) es entsteht **kein**
  behaupteter Gate (kein Make-Target → `gate-consistency` unberührt).
  Die 025b/c-Reader-Belege sind **keine** Präzedenz hierfür (die liefen
  bewusst **außerhalb** des gepinnten Images als Reviewer-Cross-Check) —
  die Deckung trägt die Image-Pinnung selbst. Das **exakte
  `docker run`-Kommando wird im Beleg-Report mitprotokolliert**
  (Reproduzierbarkeit, Review-LOW-2). Laufzeit-Erwartung: 7 Voll-Korpus-
  Läufe (je Familie einer) — Zeit-, kein Korrektheits-Punkt (Review-INFO-1).
- **HeaderFilterRegex-Vorbehalt (Review-LOW-1):** `.clang-tidy` setzt
  `HeaderFilterRegex: '^src/'` — Header-Befunde werden nur unter `src/`
  gemeldet, `plugins/`-Header liegen außerhalb der Reichweite von Gate
  **und** Matrix. Da der Dry-Run dieselbe `.clang-tidy` erbt, ist die
  Matrix **gate-repräsentativ** (misst genau, was der Gate sähe); die
  Plugin-Header-Lücke wird als Matrix-Vorbehalt im Beleg benannt.
- **`misc-include-cleaner`/`misc-non-private-member-variables-in-classes`:**
  potenzielle Großbefund-/Fehlalarm-Kandidaten (Include-Churn; pure
  Werttypen des Domänen-Modells sind bewusst offene structs) — Erwartung:
  Auslassung per Kriterium 2/5 bzw. Options-Konfiguration; entscheidet die
  Matrix.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF (deklariert: `Makefile`, `tools/`, `.devcontainer/` —
  Konventions-Dichte hoch: Docker-only, gepinnte Toolchain, 0-Befunde-Gate).
  **Phase-Reife:** etabliert (Gate seit slice-002 real). **Evidenz-Risiko:**
  niedrig-mittel (Befund-Matrix unbekannt — genau dafür der Dry-Run).
  **Reconciliation:** keiner.

### Sub-Areas der Lint-Fixes (Review-MED-2): Hexagon-Kern · Adapter · Plugin-Host

- **Modus:** je GF (alle in
  [`harness/conventions.md`](../../../../harness/conventions.md)
  §Modus-Deklaration deklariert). Die Fixes sind **verhaltensneutrale
  Randänderungen** unter der harten §2-Grenze („kein Verhaltens-Umbau,
  sonst Check auslassen"); der **Plugin-Host** ist zusätzlich der benannte
  Idiom-Kollisions-Fokus (§6) — dort entscheidet die Matrix zwischen
  Options-Ausnahme und begründeter Auslassung, nie stiller Code-Umbau
  an der ABI-Grenze.

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; Quergewerk-Disziplin (Muster 018/022: Roadmap-Eintrag,
  kein Wellen-Feature, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) davor).
  **Risiko:** niedrig.
