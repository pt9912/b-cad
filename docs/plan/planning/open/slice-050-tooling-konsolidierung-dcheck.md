---
id: slice-050
titel: Tooling-Konsolidierung — gate-consistency.sh → d-check `targets`-Modul, idlink.py → `d-check --repair`; arch-check.sh bleibt
status: open
welle: welle-5-erweiterung
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md)]
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
- **`tools/arch-check.sh`** → **bleibt** (P1 `dlopen`-**Aufruf** / P2 Quote-Angle-Include-Granularität; d-check
  hat **kein** Code-/C++-Struktur-Modul, a-check prüft nur Kanten — beidseitig blind).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

**Bewusst NICHT Teil (benannte Grenzen):**

- **`arch-check.sh` bleibt unangetastet** (Residual-Rest P1/P2). Ein etwaiger Fold in a-check wäre ein eigener,
  a-check-seitiger Re-Eval — nicht dieser Schnitt.
- **Kein Verhaltens-Verlust an der Referenz-Integrität** — die `ids`/`matrix`/… Prüfungen bleiben; nur die
  **Erzeugungs-** und **Doku↔Makefile-**Seite wandert von lokalen Skripten auf d-check-Bordmittel.

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
- [ ] **Retire `idlink.py`:** `tools/idlink.py` löschen; Referenzen nachziehen (**Tombstone-Purge s. Teil C**).

### C — Gemeinsam

- [ ] **Link-/Tombstone-Purge VOR dem Löschen (HIGH-3 — sonst `docs-check` RED):** die gelöschten Skripte sind
      referenziert → `target-missing` (Modul `links`) + `codepath-missing` (Modul `codepaths`, roots enthält
      `harness`). Konkret: **`harness/conventions.md` Z. 444 + Z. 478** halten Markdown-Links auf
      `tools/gate-consistency.sh` (Z. 478 = der Include-Awareness-Bullet, beschreibt nur das retirte Tool →
      **löschen**; Z. 444 umtexten). Für verbleibende **historische Inline-Code-Nennungen** (Closure-/Alt-Pläne,
      dieser Plan) `.d-check.yml` **`codepaths.ignore-refs`** um `tools/gate-consistency.sh` + `tools/idlink.py`
      erweitern (**Muster slice-033**-Tombstone). Plan-/Closure-Doku ohne Inline-Code-Pfad auf die gelöschten
      Dateien halten.
- [ ] **`README.md` (Root) Z. 143 (LOW-1):** die `tools/`-Prosa nennt `gate-consistency` (kein Link → nicht
      gate-brechend, aber stale) → nachziehen.
- [ ] **Prosa-`make X`-Ehrlichkeit (LOW-2):** bestätigen, dass **keine** Honesty-`make X`-Behauptung
      **ausschließlich** in Prosa lebt (die `targets`-Tabellen-Prüfung sähe sie nicht); der `harness/README.md`-
      Prosa-Bezug (Z. 21–23) trägt kein exklusives `make X`-Versprechen.
- [ ] **`arch-check.sh` unangetastet** (+ AGENTS.md-Zeile 153 bleibt).
- [ ] **`make gates` grün** nach dem Umbau (`targets` 0 Befunde über die dann bereinigte/exemptierte Menge);
      **Negativprobe** (wie bei den Alt-Skripten üblich): eine erfundene `make phantom`-Tabellenzeile bzw. eine
      undokumentierte Regel muss `gate-phantom`/`gate-undocumented` werfen (Wirksamkeits-Beleg vor dem Retire).
- [ ] **Doku:** `harness/README.md`/AGENTS.md »Modul 13«-Bezüge (gate-consistency) auf `targets` umstellen;
      CHANGELOG; **neue MR oder Ergänzung zu [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)**
      (das trug schon die arch-check→a-check-Migration; die gate-consistency→targets-Ablösung ist dieselbe
      Klasse → Ergänzung naheliegend, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) bestätigt).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.d-check.yml` | ändern | `targets`-Block (`makefiles` ×3, `doc-tables` **[AGENTS.md, harness/README.md]**, `authority`, `exempt-targets`) + `modules:` += `targets`; `codepaths.ignore-refs` += die 2 Alt-Skripte (Tombstone); Kommentar Z. 69 (idlink) korrigieren |
| `Makefile` | ändern | `gate-consistency` aus `gates:` + Target + `.PHONY` entfernen |
| `Dockerfile` | ändern | Stage `gate-consistency` entfernen (Stage `arch-check` bleibt) |
| `tools/gate-consistency.sh` | **löschen** | → Modul `targets` |
| `tools/idlink.py` | **löschen** (link-only, MED-1) | → `make doc-repair` |
| `AGENTS.md` | ändern | Tabellenzeile `gate-consistency` raus; **Geplant-Tabelle `make X` entschärfen (HIGH-2)**; Utility-Targets dokumentieren/exempt (gate-undocumented); idlink→doc-repair |
| `harness/README.md` | ändern | §Sensors-`make X`-Tabelle (jetzt `doc-tables`-Quelle); Modul-13-Bezüge; idlink→doc-repair |
| `harness/conventions.md` | ändern | **Z. 444/478 gate-consistency.sh-Links purgen (HIGH-3)**; [MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)-Ergänzung (oder neue MR) |
| `README.md` (Root) | ändern | Z. 143 `gate-consistency`-Prosa nachziehen (LOW-1) |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag |
| `docs/reviews/2026-07-24-slice-050-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Projektinhaber-Tooling-Audit 2026-07-24; Fähigkeiten am **gepinnten Handbuch** belegt. Nach eigenem
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  (0 HIGH) → startbar.

## 5. Closure-Trigger

- DoD A+B+C vollständig, `make gates` grün, Negativprobe wirksam, Closure-Notiz.

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
- **Reihenfolge:** A (targets) und B (repair) sind unabhängig; könnten getrennte Commits im selben Slice sein.
  arch-check.sh unberührt.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Gate-Konfiguration / Tooling

- **Modus:** GF; **Dichte:** hoch (Modul-Aktivierung + Retire + Negativprobe + Baseline-getriebene
  exempt-Liste). **Phase-Reife:** Gate-Infrastruktur reif (d-check.mk trägt `doc-targets`/`doc-repair` schon).
  **Risiko:** niedrig-mittel (mechanisch; Baseline + Negativprobe als Netz).

## 8. Closure-Notiz

_(bei Ausführung auszufüllen)_
