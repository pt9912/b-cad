---
id: slice-050
titel: Tooling-Konsolidierung — gate-consistency.sh → d-check `targets`-Modul, idlink.py → `d-check --repair`, arch-check-Regel P1 → a-check `constructs` (v0.16.0); P2 bleibt
status: open
welle: welle-5-erweiterung
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0017](../../adr/0017-plugin-api-abi.md)]
---

# Slice 050: Tooling-Konsolidierung — Alt-Skripte → d-check-Bordmittel

**Status:** open.
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review**
2026-07-24 (Reviewer ≠ Autor): **3 HIGH / 2 MED / 2 LOW / 1 INFO → nach Einarbeitung startbar**
([Report](../../../reviews/2026-07-24-slice-050-plan.md)). **Eingearbeitet:** HIGH-1 (`doc-tables` **+
harness/README.md** — sonst schrumpft der Honesty-Korpus), HIGH-2 (AGENTS.md-**Geplant-Tabelle** wirft
`gate-phantom`; `targets` kennt **keine** Sektions-Marker → Geplant-`make X` entschärfen), HIGH-3
(`codepaths.ignore-refs`-Tombstone + `conventions.md`-Link-Purge vor dem Löschen, Muster slice-033), MED-1
(idlink.py ist **link-only** verifiziert → `--repair` deckt **vollständig**, kein Anker-Rest), MED-2 (exakte
`exempt-targets`-Liste), LOW-1/LOW-2.

> **Amendment 2026-07-25 (Projektinhaber): Teil D — arch-check-**Regel P1** wandert zu a-check.**
> Der im Ur-Plan als „eigener, a-check-seitiger Re-Eval" benannte Trigger ist **eingetreten**:
> **a-check v0.16.0** (2026-07-25) bringt den Optionalblock **`constructs`** / die Regel
> **`construct-leak`** (a-check-Handbuch 1.33, dort ADR-seitig als Roh-Text-Monopol gebucht) — ein Monopol, das
> ausdrücklich für das `dlopen`-**Aufruf**-Monopol gebaut ist. **`tools/arch-check.sh` bleibt bestehen** und
> hält weiter **Regel P2** (die geschlossene Import-Allowlist, die a-check strukturell nicht sieht).
> **[`MR-006`](../../../../harness/conventions.md)-Nachtrag durchgeführt** 2026-07-25 (3 HIGH/5 MED/5 LOW/7 INFO,
> [Report](../../../reviews/2026-07-25-slice-050-teil-d-plan.md)); die HIGH-Funde sind eingearbeitet — HIGH-2 hat
> den Schnitt von „ganz retiren" auf „nur P1" verkleinert, HIGH-1 den Pin-Ort korrigiert, HIGH-3 ist damit
> gegenstandslos.

**Welle:** welle-5-erweiterung (Quergewerk / **harness-steering + Tooling-Hygiene**).

**Auslöser (Projektinhaber, 2026-07-24, Tooling-Audit):** Nach der d-check/a-check-Migration halten zwei
lokale `tools/`-Skripte nur noch Funktionen, die die **gepinnte d-check v0.51.1** (Handbuch 1.42) inzwischen
selbst trägt. Belegt am **autoritativen d-check-Handbuch** (gepinnte v0.51.1, im d-check-Repo unter docs/user/; §4.10 /
§6 / Änderungshistorie), nicht aus Inferenz:

- **`tools/gate-consistency.sh`** (Doku↔Makefile-Ehrlichkeit) → **Modul `targets`**. Handbuch-Änderungshistorie
  1.22 (d-check v0.38.0) wörtlich: »Neues opt-in-Modul `targets` … **Löst den Doku-↔-Makefile-Kern des
  `gate-consistency.sh`-Meta-Gates ab**« (`gate-phantom` + `gate-undocumented`, hermetisch, kein git).
- **`tools/idlink.py`** (ID-Link-Generierung) → **`d-check --repair`** (§4.10: `id-unlinked` → Markdown-Link als
  `git apply`-Patch). **`d-check.mk` liefert `doc-repair` bereits als fertiges Target.**
- **`tools/arch-check.sh`** → **bleibt, schrumpft auf P2** (Amendment 2026-07-25, s. Teil D). Der Ur-Stand
  dieser Zeile lautete „bleibt (P1 `dlopen`-Aufruf / P2 Quote-Angle-Granularität — a-check prüft nur Kanten)";
  die P1-Hälfte davon galt bis **a-check v0.15.0**. Mit **v0.16.0** trägt a-check `constructs`/`construct-leak`
  (Roh-Text-Monopol, scan-weit) → **P1 wandert**. **P2 bleibt lokal**, weil es eine *geschlossene Allowlist*
  ist, die ein kanten-basierter Prüfer strukturell nicht abbildet ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Nachtrag HIGH-2, fixture-belegt).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

**Bewusst NICHT Teil (benannte Grenzen):**

- **`arch-check.sh` wird nicht gelöscht** (Amendment 2026-07-25): nur **P1** wandert; **P2 bleibt** dort.
  Ein etwaiger P2-Fold bliebe ein eigener, a-check-seitiger Re-Eval (CR-Kandidat: geschlossene Allowlist je
  Schicht / inverse Zone) — nicht dieser Schnitt.
- **Keine Gate-Lockerung, also keine ADR** — [AGENTS.md §2.6](../../../../AGENTS.md) verlangt eine ADR nur für
  **Lockerungen**; hier bleibt jede Regel erhalten, es wechselt allein der Durchsetzungs-Mechanismus für P1.
  Lage identisch zu [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check), das aus genau
  diesem Grund ohne ADR auskam.
- **Kein Verhaltens-Verlust an der Referenz-Integrität** — die `ids`/`matrix`/… Prüfungen bleiben; nur die
  **Erzeugungs-** und **Doku↔Makefile-**Seite wandert von lokalen Skripten auf d-check-Bordmittel.
- **Kein Fold von `suppression-gate.sh`/`license-check`** — andere Werkzeug-Klasse, kein Trigger.
- **[ADR-0017](../../adr/0017-plugin-api-abi.md) wird NICHT editiert** — die ADR ist `Accepted`, ihr Core ist per
  [AGENTS.md §2.5](../../../../AGENTS.md) + [MR-016](../../../../harness/conventions.md) (`make doc-immutable`)
  unveränderlich. Die Sensor-Umbindung für P1 lebt in der **neuen
  [`MR-021`](../../../../harness/conventions.md)** (Muster [`MR-013`](../../../../harness/conventions.md)).

---

## 1. Ziel

Zwei Alt-Skripte + ihre Makefile-Targets + Dockerfile-Stages retiren und durch die **bereits im gepinnten
d-check-Image vorhandenen** Bordmittel ersetzen — **verhaltens-erhaltend bzw. -verschärfend**, ein Werkzeug-Satz
weniger zu pflegen, keine Drift-Replikation mehr (idlink.py musste d-checks `Slugify` nachbauen).

**Ausgangslage (verifiziert):** `d-check.mk` (via `--print-mk`) liefert bereits die Targets `doc-targets` und
`doc-repair`; die `.a-check.yml`/`.d-check.yml`-Gates laufen digest-gepinnt. b-cad führt seine `make`-Targets als
**Tabelle in `AGENTS.md`** (Z. 150–174) → das `targets`-Modul (`doc-tables`/`authority`) ist ein **tragfähiger**
Ersatz für den Prosa-freien Kern von `gate-consistency.sh`.

## 2. Definition of Done

### A — `gate-consistency.sh` → Modul `targets`

- [ ] **`.d-check.yml` `targets`-Block** ergänzen:
      `makefiles: [Makefile, a-check.mk, d-check.mk]` (**alle drei** — die realen Regeln von `make a-check`/
      `docs-check` leben in den Includes; sonst `gate-phantom`-Falschbefund),
      **`doc-tables: [AGENTS.md, harness/README.md]`** (**HIGH-1** — `gate-consistency.sh` scannt **beide**
      Honesty-Dokumente [Skript Z. 30]; `harness/README.md` §Sensors trägt eine **zweite** `make X`-Tabelle;
      nur AGENTS.md ⇒ Korpus schrumpft, ein Phantom in README bliebe ungefangen),
      `authority: AGENTS.md`, `exempt-targets: [...]` (s. u.). `targets` in die `modules:`-Liste (hermetisch,
      kein git → zulässig im Default-Set neben `planning`/`tracked`).
- [ ] **`gate-phantom` aus »Geplant«-Tabellen entschärfen (HIGH-2):** `targets` hat **keine** Sektions-Marker-
      Awareness (anders als `gate-consistency.sh`s `awk`-`/Geplant/`-Unterdrückung) → es liest **jede**
      Tabellenzeile. Die **AGENTS.md §3-»Geplant«-Tabelle** (`make coverage-gate-critical`/`make ci`/
      `make fullbuild` — **nicht** real) würde **`gate-phantom ×3`** werfen, und `exempt-targets` hilft
      **nicht** (das adressiert nur `gate-undocumented`). Auflösung: diese Geplant-`make X`-Zellen aus der
      **Tabellen**-Form nehmen (Prosa/Code-Fence/ohne `make`-Präfix) — ebenso etwaige Prosa-`make X` in
      `harness/README.md` (Z. 28–29) und die **Negativ-Beispiel-Tabelle** AGENTS.md Z. 127/128
      (`make <target>`-Platzhalter). **Baseline-Lauf VOR Retire** belegt die vollständige Phantom-Menge.
- [ ] **`gate-undocumented`-`exempt-targets` (exakte Namen, MED-2):** `targets` prüft **bidirektional** (jede
      reale Regel muss in AGENTS.md stehen) — strenger als das unidirektionale `gate-consistency.sh` (**Gewinn**).
      Die realen, **un**dokumentierten Utility-Regeln sind exakt: `help`, `dev-image`, `record-gates`,
      `versions`, `schema-regen`, `a-check-graph`, sowie aus `d-check.mk`: `doc-check`, `doc-doctor`,
      `doc-trace`, `doc-complete`, `doc-planning`, `doc-tracked`, `doc-targets`, `doc-help` (`doc-repair` **wird**
      dokumentiert, s. Teil B). Je Fall **dokumentieren** (in die AGENTS.md-Tabelle) **oder** `exempt-targets`
      (exakte Regelnamen, kein Glob — Handbuch §5). Baseline bestätigt die endgültige Liste.
- [ ] **Retire `gate-consistency`:** aus der `gates:`-Zeile im `Makefile` entfernen (`targets` läuft nun in
      `docs-check`; alternativ `doc-targets` explizit in `gates` — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) wählt),
      das `gate-consistency:`-Target + die **Dockerfile-Stage `gate-consistency`** entfernen,
      `tools/gate-consistency.sh` löschen, `.PHONY`-Liste + AGENTS.md-Tabellenzeile (Z. 151) nachziehen
      (**Link-/Tombstone-Purge s. Teil C**).

### B — `idlink.py` → `d-check --repair` (`make doc-repair`)

- [ ] **Fix-Weg dokumentieren:** `make doc-repair` (bereits vorhanden, aus `d-check.mk`) ist der neue Weg,
      `id-unlinked`-Prosa-Befunde als Patch zu beheben (`--repair | git apply`). AGENTS.md/README-Verweise auf
      `idlink.py` → auf `make doc-repair` umstellen; `.d-check.yml`-Kommentar Z. 69 (»Anker/Links generiert von
      tools/idlink.py, vom Gate validiert«) korrigieren.
- [ ] **idlink.py ist link-only (MED-1, am Quelltext verifiziert — kein Anker-Rest):** `idlink.py` **schreibt
      nie** `<a id=…>` (Headings verbatim durchgereicht); die `HTML_ID`/`HTML_NAME`-Regex **lesen** bestehende
      Anker nur als Link-**Ziel**-Präzedenz (`build_map`). Es ist **rein** ein Referenz-Link-Generator → von
      `--repair`/`id-unlinked` (Handbuch §4.10: nackte Kennung → Markdown-Link, nur nackte Prosa)
      **vollständig** gedeckt. Die slice-018c-per-ID-Anker existieren im Repo, wurden aber **nicht** von
      idlink.py erzeugt (manuell/Inline-HTML). **Kein** manueller Anker-Schritt, **kein** Mini-Helfer nötig.
      (Log-Datei-Backtick-Normalisierung ist nicht gate-relevant — Log-Dateien sind `ids`-`exempt-paths`.)
- [ ] **B-Befund 2026-07-25 (am eigenen Lauf belegt, in die Doku aufnehmen): `git apply` braucht
      `--unidiff-zero`.** Das Handbuch (§4.10) nennt schlicht `git apply fix.patch`; real erzeugt `--repair`
      **kontextlose** Hunks (`@@ -7,1 +7,1 @@`), die `git apply` **per Default ablehnt**
      (»Anwendung des Patches fehlgeschlagen«). Verifiziert beim Reparieren **dieses** Plans (44 `id-unlinked`):
      `git apply` scheitert, `git apply --unidiff-zero` wendet an. Der in AGENTS.md/README zu dokumentierende
      Weg lautet daher `make doc-repair > fix.patch && git apply --unidiff-zero fix.patch`.
- [ ] **B-Befund 2026-07-25 (Qualitäts-Grenze von `--repair`, kein Blocker): Link-Ziel ist das Verzeichnis.**
      `--repair` ersetzt eine nackte Kennung durch `[`ADR-NNNN`](../../adr)` — einen **Verzeichnis**-Link, nicht
      den Datei-/Anker-Link auf die Definition (der Handbuch-`fixCandidate` sagt das selbst: »Anker ggf.
      ergänzen«). Zwei Folgen, beide beim Reparieren dieses Plans real aufgetreten: (a) die Links sind schwächer
      als die im Repo übliche Datei+Anker-Form und wollen nachgeschärft werden; (b) eine **repo-fremde** Kennung
      (hier a-checks eigene ADR-Nummern, in b-cad-Prosa zitiert) wird auf **b-cads** `docs/plan/adr/` gelinkt —
      **sachlich falsch**. Konsequenz für die Doku: `--repair`-Patches sind **sichtprüfpflichtig** (Handbuch:
      »Lesen Sie den Patch vor dem Anwenden«), und fremde Kennungen gehören nicht als nackte ID in b-cad-Prosa.
      Das ist **kein** Regress gegenüber `idlink.py` (MED-1 bleibt gültig), aber der Fix-Weg ist nicht
      „blind anwendbar".
- [ ] **Retire `idlink.py`:** `tools/idlink.py` löschen; Referenzen nachziehen (**Tombstone-Purge s. Teil C**).

### D — Regel **P1** → a-check `constructs`; `arch-check.sh` schrumpft auf **P2** (Amendment 2026-07-25)

**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Nachtrag 2026-07-25** (Reviewer ≠ Autor, Delta-Review Teil D): **3 HIGH / 5 MED / 5 LOW / 7 INFO**
([Report](../../../reviews/2026-07-25-slice-050-teil-d-plan.md)). **HIGH-2 hat den Schnitt geändert:** die
erste Amendment-Fassung wollte `arch-check.sh` **ganz** retiren und den P2-Verzicht per neuer ADR unter
[AGENTS.md §2.6](../../../../AGENTS.md) buchen. Der Reviewer wies fixture-belegt nach, dass P2 im
[ADR-0017](../../adr/0017-plugin-api-abi.md)-Wortlaut eine **geschlossene Allowlist** ist
(„inkludieren **nur** … — keine …"), von der a-check nur die **Verbots**-Hälfte + die auflösbaren Kanten trägt;
der Verzicht wäre also deutlich größer gewesen als die zwei benannten Exoten.
**Projektinhaber-Entscheid 2026-07-25 (zweite Runde): P2 bleibt als schlanker Wächter.** Damit wird **nichts**
gelockert → **keine ADR nötig** (§2.6 n/a, Lage exakt wie bei
[MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)), und HIGH-3 (D8 ↔ Teil C) sowie der
Tombstone-Bedarf für `tools/arch-check.sh` **entfallen ersatzlos** — die Datei bleibt bestehen.

**Fähigkeits-Beleg (am a-check-Handbuch 1.33–1.35 + empirisch an v0.16.0, vom Reviewer unabhängig reproduziert):**

| [ADR-0017](../../adr/0017-plugin-api-abi.md)-Regel P, Teilaussage | heute | nach Teil D | Beleg |
|---|---|---|---|
| **P1** `dlopen`/`dlsym`/`dlclose`-**Aufruf** nur in `src/adapters/plugin/` | `arch-check.sh` | **`construct-leak`** | Fixture: fremder Adapter · Nachbar `plugin_helper/` · `plugins/`-Baum · `main.cpp` (`composition_root: forbid`) je 1 Befund; Host still |
| P2 `src/adapters/`-Header aus `plugins/` | `arch-check.sh` + a-check | unverändert **beide** | Fixture: `lateral-adapter`/`wrong-direction`, quote- **und** angle-Form |
| P2 Qt- / OCC- / SQLite- / `dlfcn.h`-Header aus `plugins/` | a-check (`tech`) | unverändert a-check | Fixture, 5 `tech-leak`, beide Formen |
| P2 Kern-Umgehung (`hexagon/services/`), Richtungs-Bruch (`ports/driven`) | `edges`/`direction` | unverändert a-check | Fixture: `wrong-direction`, `port-direction-mismatch` |
| **P2 geschlossene Allowlist** (plugin-lokal/relativ, repo-intern-schichtlos, repo-extern) | **`arch-check.sh`** | **bleibt `arch-check.sh`** | Reviewer-Fixture: a-check meldet dort **0 Befunde** (Ziel ohne Schicht ⇒ unbeurteilt) |

**Konsequenz:** ausschließlich **P1** wandert. `arch-check.sh` behält **P2**; die Prüf-Logik halbiert sich (P1-Block entfällt), die Datei bleibt
ähnlich lang, weil die Begründung „warum P2 lokal bleibt" als Kopf-Kommentar hineinwandert (75 → 71 Zeilen);
sein Kopf-Kommentar wird ehrlich (P-Rest = **nur** noch die Import-Allowlist). Gate-Stärke **unverändert**.

- [x] **D1 — Pin-Bump auf v0.16.0 `@sha256:aef28cfe25bb054b1b0eb28420222a45b9f6ce9425b7ffd0f55e6ae56f295b56`**
      (Quelle: a-check-Release-Register `version.md#aktuell`, v0.16.0 vom 2026-07-25).
      **HIGH-1 (verifiziert):** der wirksame Pin steht in **`Makefile` Z. 45** (`24939a6b…`, v0.13.0), **nicht** in
      `a-check.mk` Z. 5 (`203df7ab…`) — dessen `?=` ist ein **No-op**, weil der `Makefile` die Variable **vor**
      `include a-check.mk` (Z. 46) setzt. `make -n a-check` belegt es. Ohne diesen Bump bricht der real gepinnte
      Prüfer am D2-Block strikt ab (`field constructs not found`, **Exit 2** ⇒ `make gates` rot).
      **Beide** Stellen auf denselben Digest ziehen (sonst bleibt eine irreführende Divergenz stehen).
      **Selbst-Pin-Lag beachten** ([`MR-013`](../../../../harness/conventions.md)): `--print-mk` des v0.16.0-Images druckt noch v0.15.0 — der Digest
      kommt aus dem Register, nicht aus `--print-mk`.
      Vorprobe belegt: Repo + **unveränderter** Config gegen v0.16.0 = **0 Befunde**, kein Regress aus a-checks
      Ziel-Glob-/Abdeckungs-Änderungen (Handbuch 1.34/1.35), **kein** Abdeckungs-Hinweis.
- [x] **D2 — `.a-check.yml` `constructs`-Block** (P1, byte-genau die verifizierte Fassung):
      `{pattern: '\bdl(m?open|sym|close)\s*\(', match: regex, adapter: src/adapters/plugin/,
      composition_root: forbid}`. **MED-1 (verifiziert): der Schrägstrich ist Pflicht** — `adapter` ist ein
      **Teilstring**-Vergleich auf dem Pfad (Handbuch §4: »`adapters/config` matcht auch
      `adapters/configurator`«); ohne ihn entkäme ein `dlopen` in einem Nachbar-Verzeichnis
      einem Geschwister-Verzeichnis, dessen Name mit demselben Präfix beginnt (Fixture: ein erfundenes
      *plugin_helper*-Verzeichnis neben dem Plugin-Host). Fixture-Gegenüberstellung: ohne Schrägstrich 2 Befunde, mit Schrägstrich 3
      (der Nachbar wird gefangen), Host in beiden Fällen still.
      Vorprobe: Repo + diesem Block = **0 Befunde**.
- [x] **D3 — `arch-check.sh` auf P2 schrumpfen** (**nicht** löschen): den P1-Block (Z. 36–47) entfernen;
      Kopf-Kommentar (Z. 6–30) auf den verbliebenen P-Rest umschreiben — P1 liegt ab jetzt bei a-check
      (`constructs`), hier bleibt **nur** die geschlossene Import-Allowlist für `plugins/` + `src/plugin_api/`
      inkl. Angle-Verbot; die Erfolgsmeldung (Z. 73) entsprechend. `Makefile`-Target, `.PHONY`,
      `.devcontainer/Dockerfile`-Stage `arch-check` und die `gates:`-Zeile bleiben **unverändert**.
- [x] **D4 — Kommentar-Differenz ehrlich benennen (MED-2, Korrektur):** `arch-check.sh` grept roh, `constructs`
      strippt C-Kommentare (Handbuch §4). Die frühere Fassung nannte das eine „Verschärfung der Ehrlichkeit" und
      belegte sie mit `src/adapters/plugin/plugin_host.h:13` — **falsch**: diese Zeile liegt **in** der erlaubten
      Zone und war nie ein Falsch-Positiv. Real ist es ein **Deckungs-Verlust** (ein `dlopen(` in einem
      Kommentar außerhalb der Zone meldet nicht mehr) — vertretbar, weil ein Kommentar kein Aufruf ist, aber in
      [`MR-021`](../../../../harness/conventions.md) als **Verlust** zu benennen, nicht als Gewinn.
- [x] **D5 — neue [MR-021](../../../../harness/conventions.md)** „Regel P1 via a-check `constructs`" mit
      Lineage-Pointer auf [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)
      (**kein In-Place-Edit** an [`MR-013`](../../../../harness/conventions.md) — Unveränderlichkeit nach Aufnahme, Muster [`MR-003`](../../../../harness/conventions.md)→[`MR-007`](../../../../harness/conventions.md),
      [`MR-010`](../../../../harness/conventions.md)→[`MR-012`](../../../../harness/conventions.md)). Inhalt: Sensor-Umbindung **nur für P1**, die Fähigkeits-Tabelle oben, die
      Kommentar-Differenz (D4), der Teilstring-Fallstrick (D2). **Kein ADR** — es wird nichts gelockert
      (§2.6 n/a), Lage identisch zu [`MR-013`](../../../../harness/conventions.md).
- [x] **D6 — Sensor-Bindungen der ADRs (MED-3):** **15** ADRs binden Fitness-Functions an `make arch-check`;
      da das Target **bleibt** und nur P1 abwandert, bleiben die Bindungen gültig. Zu prüfen und ggf. per
      [`MR-021`](../../../../harness/conventions.md) zu erfassen sind genau die Stellen, die **P1** dem `arch-check` zuschreiben — insbesondere die
      [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflicht-Zeile im ADR-Index (`docs/plan/adr/README.md`, „arch-check-Regel P … erfüllt durch
      slice-026b"): sie ist als **historische** Erfüllungs-Aussage weiterhin wahr, bekommt aber einen
      Provenance-Zeiger auf [`MR-021`](../../../../harness/conventions.md). **Kein** ADR-Edit (§2.5).
- [x] **D7 — Negativprobe D (Wirksamkeits-Beleg):** ein `::dlopen(`-Aufruf außerhalb `src/adapters/plugin/`
      (fremder Adapter · `plugins/`-Baum · `main.cpp`) muss `construct-leak` werfen — **und** eine
      P2-Verletzung (Quote-Include außerhalb der Allowlist) muss weiterhin `arch-check` rot machen; danach
      beide zurücknehmen. Muster: die `arch-check`-Gegenprobe aus slice-026b (CHANGELOG Z. 301).
- [x] **D8 — Doku-Nachzug Teil D** (Umfang gegenüber der ersten Fassung **kleiner**, weil das Target bleibt):
      `AGENTS.md` §3 — `make a-check`-Zeile um `constructs`/P1 erweitern, `make arch-check`-Zeile (Z. 153) auf
      **P2-only** umschreiben; `harness/README.md` Z. 73 (a-check) + Z. 74 (arch-check).
      **MED-4:** fünf **In-Code**-Kommentare nennen `arch-check` — `src/adapters/plugin/plugin_host.h:5` und
      `plugin_host.cpp:2` (beide „arch-check Regel P1" → a-check `constructs`), `src/adapters/CMakeLists.txt:80`
      (P1-Bezug), `src/plugin_api/CMakeLists.txt:5` und `plugins/CMakeLists.txt:10` (P2 — bleibt wahr, nur der
      P1-Halbsatz zieht nach). **HIGH-3 entfallen:** `harness/conventions.md` Z. 442/455/468 ([`MR-013`](../../../../harness/conventions.md)) bleiben
      **unangetastet und wahr**, da `tools/arch-check.sh` existiert. `README.md` Z. 24/143,
      `spec/architecture.md` Z. 175 und `spec/spezifikation.md` Z. 579/638/677 sind **Teil-A/C**-Nachzüge
      (gate-consistency/Regel-Namen), nicht Teil D.

### C — Gemeinsam

- [ ] **Link-/Tombstone-Purge VOR dem Löschen (HIGH-3 — sonst `docs-check` RED):** alle drei gelöschten Skripte
      sind referenziert → `target-missing` (Modul `links`) + `codepath-missing` (Modul `codepaths`, `roots`
      enthält `harness`). **Amendment 2026-07-25 — HIGH-3 wird ANDERS aufgelöst als im Ur-Plan:** der Ur-Plan
      wollte `harness/conventions.md` Z. 444 **umtexten** und Z. 478 **löschen** — beide Zeilen liegen **innerhalb
      von [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)**, und die
      MR-Unveränderlichkeit nach Aufnahme ist etablierte Praxis ([`MR-012`](../../../../harness/conventions.md) zu [`MR-010`](../../../../harness/conventions.md) wörtlich: »nach Aufnahme
      inhaltlich unveränderlich«, Nachzug per **neuem** Eintrag; Lineage [`MR-003`](../../../../harness/conventions.md)→[`MR-007`](../../../../harness/conventions.md), [`MR-010`](../../../../harness/conventions.md)→[`MR-012`](../../../../harness/conventions.md)).
      In-Place-Edit an [`MR-013`](../../../../harness/conventions.md) unterliefe genau das. **Auflösung stattdessen: Tombstone, [`MR-013`](../../../../harness/conventions.md) bleibt
      unangetastet.** Der Ur-Plan konnte das nicht sehen, weil er die **modul-lokale** Alias-Form
      `codepaths.ignore-refs` annahm (deckt nur Inline-Code-Pfade, **nicht** Markdown-Links).
      **Am gepinnten d-check-Handbuch belegt (v0.51.1, Handbuch 1.38 / d-check v0.49.0, §5):** `ignore-refs` ist
      seit v0.49.0 eine **querschnittliche Top-Level-Liste**, die `links`, `anchors` **und** `codepaths`
      gemeinsam honorieren — je Eintrag `refs` (Pflicht, Globs auf den **aufgelösten** Zielpfad), `in`
      (optionaler Quell-Skopus) und `keep`. Das Handbuch nennt als typischen Fall wörtlich den **Tombstone**:
      »ein bewusst entfernter Pfad, den immutable/historische Doku (z. B. eine akzeptierte ADR) noch zitiert …
      ohne dass man die immutable Doku editieren dürfte«. Konkret:
      - `.d-check.yml`: die bestehende `codepaths.ignore-refs`-**Alias**-Liste auf die **Top-Level**-Form heben
        (Alias bleibt gültig, aber deckt `links` nicht) und um `tools/gate-consistency.sh` + `tools/idlink.py`
        erweitern. **`tools/arch-check.sh` NICHT** — die Datei bleibt bestehen (Amendment: nur P1 wandert), ein
        Tombstone dafür wäre ein toter Eintrag. Kommentar nach Muster slice-033/slice-042b (Anlass + §2.6 n/a).
      - Damit gedeckt **ohne** Fremd-Edit: `harness/conventions.md` Z. 444/478 ([`MR-013`](../../../../harness/conventions.md), Markdown-Links auf
        `gate-consistency.sh`), `docs/plan/planning/done*/**` (eingefrorene Closure-Doku) und dieser Plan.
        **Z. 442 bleibt scharf und wahr** (Link auf `tools/arch-check.sh` — die Datei existiert weiter).
      - **LOW-2 ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Nachtrag):** die Fundstellen-Menge **maschinell** erheben (`grep`-Sweep über
        Markdown-Links **und** Inline-Code), nicht aus dieser Aufzählung ableiten — der Reviewer fand mit
        [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) Z. 80 mindestens eine im Ur-Plan ungenannte Stelle.
      - **LOW-3 ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Nachtrag):** die Hebung Alias → Top-Level weitet die **drei bestehenden** Tombstones
        (`tools/Dockerfile`, `plan_geometry.{h,cpp}`) still von `codepaths` auf `links`+`anchors` aus. Das ist
        eine Ausweitung ohne eigenen Anlass → entweder bewusst mitbuchen (Kommentar) oder die Altbestände als
        `codepaths`-skopierte Einträge belassen und nur die neuen querschnittlich führen.
      - **Gate-Wirksamkeit bleibt:** ohne passenden Eintrag meldet ein fehlendes Ziel weiter — nichts
        verschwindet still (Handbuch §5). **Keine** Schwellen-Lockerung → §2.6 n/a.
- [ ] **`README.md` (Root) Z. 143 (LOW-1):** die `tools/`-Prosa nennt `gate-consistency` (kein Link → nicht
      gate-brechend, aber stale) → nachziehen. `arch-check` bleibt dort korrekt stehen.
- [ ] **Prosa-`make X`-Ehrlichkeit (LOW-2):** bestätigen, dass **keine** Honesty-`make X`-Behauptung
      **ausschließlich** in Prosa lebt (die `targets`-Tabellen-Prüfung sähe sie nicht); der `harness/README.md`-
      Prosa-Bezug (Z. 21–23) trägt kein exklusives `make X`-Versprechen.
- [ ] **`arch-check.sh` bleibt als Target** — AGENTS.md-Zeile 153 bleibt, wird aber in **D8** auf **P2-only**
      umgeschrieben. `arch-check` ist real **und** dokumentiert, also in **keiner** Reihenfolge ein
      `exempt-targets`-Kandidat (**LOW-1 des Nachtrags**: die frühere Behauptung „Reihenfolge D → A ist
      zwingend" war überzogen). Einzige reale Kopplung: die D8-Umschreibung derselben Zeile, die Teil A als
      `doc-tables`-Quelle liest — **eine Reihenfolge-Empfehlung, kein Zwang**.
- [ ] **`make gates` grün** nach dem Umbau (`targets` 0 Befunde über die dann bereinigte/exemptierte Menge);
      **Negativprobe** (wie bei den Alt-Skripten üblich): eine erfundene `make phantom`-Tabellenzeile bzw. eine
      undokumentierte Regel muss `gate-phantom`/`gate-undocumented` werfen (Wirksamkeits-Beleg vor dem Retire).
- [ ] **Doku:** `harness/README.md`/AGENTS.md »Modul 13«-Bezüge (gate-consistency) auf `targets` umstellen;
      CHANGELOG; **neue MR** (statt Ergänzung zu
      [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)). **Amendment 2026-07-25:** der
      Ur-Plan ließ „Ergänzung **oder** neue MR" offen und nannte die Ergänzung „naheliegend" — das ist mit der
      MR-Unveränderlichkeits-Praxis (s. Tombstone-Auflösung oben) **nicht** vereinbar; es wird **eine** neue
      **[`MR-021`](../../../../harness/conventions.md)**, die A+B (targets/repair) **und** D (P1→`constructs`) trägt, mit Lineage-Pointer auf
      [`MR-013`](../../../../harness/conventions.md) (Muster [`MR-003`](../../../../harness/conventions.md)→[`MR-007`](../../../../harness/conventions.md), [`MR-010`](../../../../harness/conventions.md)→[`MR-012`](../../../../harness/conventions.md)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.d-check.yml` | ändern | `targets`-Block (`makefiles` ×3, `doc-tables` **[AGENTS.md, harness/README.md]**, `authority`, `exempt-targets`) + `modules:` += `targets`; **`ignore-refs` auf Top-Level heben** (querschnittlich `links`+`codepaths`, d-check ≥ v0.49.0) += die **2** gelöschten Alt-Skripte (Tombstone; **nicht** arch-check.sh); Kommentar Z. 69 (idlink) korrigieren |
| `Makefile` | ändern | `gate-consistency` aus `gates:` + Target + `.PHONY` entfernen (**`arch-check` bleibt**); **D1** — `A_CHECK_IMAGE` Z. 45 auf v0.16.0 `@sha256:aef28cfe…` (der **wirksame** Pin, HIGH-1) |
| `.devcontainer/Dockerfile` | ändern | Stage `gate-consistency` entfernen (**Stage `arch-check` bleibt**) |
| `a-check.mk` | ändern | **D1** — Z. 5 auf denselben v0.16.0-Digest ziehen (inertes `?=`, aber sonst irreführend) |
| `.a-check.yml` | ändern | **D2** — `constructs`-Block (`dlopen`-Aufruf-Monopol, Zone **mit Schrägstrich**, `composition_root: forbid`) |
| `tools/gate-consistency.sh` | **löschen** | → Modul `targets` |
| `tools/idlink.py` | **löschen** (link-only, MED-1) | → `make doc-repair` |
| `tools/arch-check.sh` | **ändern** (Teil D, **nicht** löschen) | P1-Block raus (→ a-check `constructs`); **P2 bleibt** (geschlossene Allowlist, kanten-basiert nicht abbildbar); Kopf-Kommentar + Erfolgsmeldung ehrlich |
| `src/adapters/plugin/plugin_host.{h,cpp}`, `src/adapters/CMakeLists.txt` | ändern | **MED-4** — In-Code-Kommentare „arch-check Regel P1" → a-check `constructs` |
| `src/plugin_api/CMakeLists.txt`, `plugins/CMakeLists.txt` | ändern | **MED-4** — P2-Bezug bleibt wahr, nur der P1-Halbsatz zieht nach |
| `AGENTS.md` | ändern | Tabellenzeile `gate-consistency` raus; `make arch-check`-Zeile (Z. 153) auf **P2-only**; `make a-check`-Zeile um `constructs`/P1; **Geplant-Tabelle `make X` entschärfen (HIGH-2 des Ur-Reviews)**; Utility-Targets dokumentieren/exempt; idlink→doc-repair |
| `harness/README.md` | ändern | §Sensors-`make X`-Tabelle (jetzt `doc-tables`-Quelle); Z. 73 (a-check) + Z. 74 (arch-check → P2-only); Modul-13-Bezüge; idlink→doc-repair |
| `harness/conventions.md` | ändern | **neue [MR-021](../../../../harness/conventions.md)** (Lineage → [`MR-013`](../../../../harness/conventions.md)). **[`MR-013`](../../../../harness/conventions.md) bleibt unangetastet** — Z. 442 (arch-check-Link) bleibt ohnehin wahr |
| `docs/plan/adr/README.md` | ändern | **D6** — Provenance-Zeiger auf [`MR-021`](../../../../harness/conventions.md) an der [ADR-0017](../../adr/0017-plugin-api-abi.md)-Regel-P-Folgepflicht-Zeile (kein ADR-Edit, §2.5) |
| `README.md` (Root) | ändern | Z. 143 `gate-consistency`-Prosa nachziehen (LOW-1) |
| `spec/architecture.md` | ändern (**optional**) | Z. 175 nennt `make arch-check` als **»geplant«** — falsch, seit slice-026b real. **Vorgefundene** Staleness, nicht von Teil D verursacht; Hygiene-Nachzug, streichbar |
| `spec/spezifikation.md` | ändern (**optional**) | Z. 579/638/677 schreiben Regel A/B/C dem `arch-check` zu — die tragen seit slice-030/[`MR-013`](../../../../harness/conventions.md) a-check. **Vorgefundene** Staleness; Hygiene-Nachzug, streichbar |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |
| `docs/reviews/2026-07-24-slice-050-plan.md` | vorhanden | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report (A/B/C) |
| [`docs/reviews/2026-07-25-slice-050-teil-d-plan.md`](../../../reviews/2026-07-25-slice-050-teil-d-plan.md) | vorhanden | **[`MR-006`](../../../../harness/conventions.md)-Nachtrag** auf das Delta Teil D (Reviewer ≠ Autor): 3 HIGH/5 MED/5 LOW/7 INFO, HIGH eingearbeitet |

## 4. Trigger

- Projektinhaber-Tooling-Audit 2026-07-24; Fähigkeiten am **gepinnten Handbuch** belegt. Nach eigenem
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  (0 HIGH) → **A/B/C startbar**.
- **Teil D (Amendment 2026-07-25):** Release **a-check v0.16.0** (2026-07-25) mit `constructs`/`construct-leak`
  — der im Ur-Plan benannte „eigener, a-check-seitiger Re-Eval"-Trigger. **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Nachtrag durchgeführt**
  (3 HIGH eingearbeitet, Schnitt auf **P1-only** verkleinert) → **D startbar**.

## 5. Closure-Trigger

- DoD A+B+C+**D** vollständig, `make gates` grün, **beide** Negativproben wirksam (A: `gate-phantom`/
  `gate-undocumented`; D: `construct-leak` **und** weiterhin rotes `arch-check` bei P2-Verstoß), Closure-Notiz.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — `gate-undocumented`-Bidirektionalität (Purge-Aufwand):** `targets` verlangt, dass **jede**
  reale Regel in `AGENTS.md` steht. Die realen Regeln (`Makefile`+`a-check.mk`+`d-check.mk`) umfassen Utilities
  (`help`, `record-gates`, `dev-image`, `a-check-graph`, `versions`, die `doc-*`-Distributoren) — die
  entweder dokumentiert oder via `exempt-targets` (**exakte** Regelnamen, kein Glob) ausgenommen werden. **Baseline
  zuerst laufen**, dann Liste festlegen. Kein HIGH (mechanisch), aber die Hauptarbeit.
- **Rest-Risiko #2 — `AGENTS.md`-Tabellen-Grammatik (HIGH-Kandidat):** `doc-tables: [AGENTS.md]` liest **alle**
  `make X` aus **allen** Tabellen. AGENTS.md enthält eine **Negativ-Beispiel-Tabelle** (Z. 127/128, u. a.
  `make <target>`-Platzhalter und `make docs-check / make gates (real)`). Prüfen, ob das `targets`-Modul den
  Platzhalter/Slash/„(real)" korrekt ignoriert oder ob diese Zeilen `gate-phantom` (Ziel `<target>`) werfen →
  ggf. Zeilen umformen oder Tabelle vom Scan trennen. **Fixture-/Baseline-Beleg vor Retire.**
- **Rest-Risiko #3 — ~~idlink-Anker-Funktion~~ ERLEDIGT (MED-1):** die Sorge um eine Definitions-Anker-Hebung
  war ein **Phantom** — `idlink.py` ist am Quelltext **link-only** (schreibt nie `<a id=…>`); `--repair` deckt
  es **vollständig**. Kein Blocker fürs Löschen.
- **Rest-Risiko #4 — Baseline-Pflicht (aus HIGH-2/MED-2):** vor jedem Retire **einmal** `targets` fahren und die
  **volle** `gate-phantom`- (Geplant-/Negativ-Tabellen) + `gate-undocumented`-Menge (Utility-Regeln) einsehen;
  erst danach Doku entschärfen / `exempt-targets` fixieren. Mechanisch, aber die Hauptarbeit; die
  **Negativprobe** (erfundene Regel/Zeile) belegt die Wirksamkeit vor dem Löschen der Alt-Skripte.
- **Rest-Risiko #5 (Teil D) — ~~Gate-Blind-Fleck~~ ENTFALLEN ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Nachtrag HIGH-2):** die erste
  Amendment-Fassung wollte P2 aufgeben und den Verzicht per ADR buchen. Der Reviewer wies nach, dass P2 im
  [ADR-0017](../../adr/0017-plugin-api-abi.md)-Wortlaut eine **geschlossene Allowlist** ist und der Verzicht
  damit deutlich größer gewesen wäre als beschrieben (zusätzlich ungefangen: plugin-lokale/relative Includes
  und repo-interne, schichtlose Ziele — a-check meldet dort 0 Befunde, weil ein Ziel ohne Schicht **unbeurteilt**
  bleibt). Projektinhaber-Entscheid: **P2 bleibt in `arch-check.sh`.** Damit **keine** Lockerung, **keine** ADR,
  kein Blind-Fleck. Offener Re-Eval-Trigger für später: eine a-check-CR (geschlossene Allowlist je Schicht /
  inverse Zone) — Präzedenz der d-check-Pilot-CR, a-check-Lastenheft 0.14.0.
- **Rest-Risiko #6 (Teil D) — Pin-Bump-Kopplung (verschärft durch HIGH-1):** `constructs` setzt **v0.16.0**
  voraus, und der **wirksame** Pin steht im `Makefile` Z. 45, nicht in `a-check.mk` (dessen `?=` ist ein No-op).
  Wird nur `a-check.mk` gebumpt, läuft weiter v0.13.0 und der D2-Block bricht mit **Exit 2** ab
  (`field constructs not found`) ⇒ `make gates` rot. Der Pin-Bump ist ein bewusster Commit
  ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Prinzip, [`MR-013`](../../../../harness/conventions.md)-Praxis).
  **Vorprobe gefahren:** Repo gegen v0.16.0 mit **unveränderter** Config = 0 Befunde, mit `constructs`-Block
  ebenfalls 0.
- **Rest-Risiko #7 (Teil D, MED-5-Erbe) — Fitness-Bindungen breiter als [ADR-0017](../../adr/0017-plugin-api-abi.md):** **15** ADRs binden
  Fitness-Functions an `make arch-check`. Da das Target **bleibt**, bleiben sie gültig; nachzuziehen sind nur
  die Stellen, die ausdrücklich **P1** dem `arch-check` zuschreiben (D6). Ohne den Amendment-Rückbau wäre das
  ein 15-fach-Nachzug gewesen.
- **Reihenfolge:** **D vor A empfohlen** (nicht zwingend — LOW-1): D8 schreibt die AGENTS.md-`arch-check`-Zeile
  um, die Teil A als `doc-tables`-Quelle liest; andersherum entstünde nur eine zweite Durchsicht, kein Fehler.
  B (repair) ist von beiden unabhängig. Vorschlag: **drei Commits** — D, A, B/C.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Gate-Konfiguration / Tooling

- **Modus:** GF; **Dichte:** hoch (Modul-Aktivierung + **zwei** Retires + ein Skript-Rückbau + zwei
  Negativproben + Baseline-getriebene exempt-Liste + Pin-Bump). **Phase-Reife:** Gate-Infrastruktur reif
  (d-check.mk trägt `doc-targets`/`doc-repair` schon; a-check v0.16.0 trägt `constructs`). **Risiko:**
  niedrig-mittel (mechanisch; Baseline + Negativproben + gefahrene Vorprobe als Netz). **Keine §2.6-Buchung** —
  es wird nichts gelockert (Amendment nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Nachtrag HIGH-2).

## 8. Closure-Notiz

_(bei Ausführung auszufüllen)_
