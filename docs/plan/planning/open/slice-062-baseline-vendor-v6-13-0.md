---
id: slice-062
titel: Baseline-Bundle vendoren — ai-harness-course v6.13.0 nach `.harness/baseline/` + fetch/verify-Mechanik
status: open
welle: welle-7-regelwerk-migration
lastenheft_refs: []
adr_refs: [[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)]
---

# Slice 062: Baseline-Bundle vendoren + fetch/verify-Mechanik

**Status:** open — Detail-Schnitt vollzogen (2026-09-29). Erstes Slice der
neuen Welle `welle-7-regelwerk-migration` (Quergewerk `harness-steering`,
siehe Roadmap §Nächste Wellen). Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-09-29.

## Auslöser

Projektinhaber-Entscheid 2026-09-29: Umstellung des verbindlichen Regelwerks
auf **v6.13.0** mit **committet-vendored Baseline**. Zwei Treiber: (1) die
bisher in `AGENTS.md`/`harness/README.md`/`harness/conventions.md`
referenzierte `agents-regelwerk.md` liefert **404** — die Referenz-Form ist
tot; (2) das Release v6.13.0 ist ein **self-contained Baseline-Bundle**
(`regelwerk/` + `templates/` parallel), dessen Release-Text das
`.harness/baseline/<tag>/`-Layout selbst beschreibt. Blaupause der Mechanik:
das gleichnamige Skript im Schwester-Repo **u-boot** unter `tools/harness/`
dort (u-boot fährt das Muster produktiv, Stand v6.13.0), Pin-Muster
[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md).

## 1. Ziel

Die Baseline liegt **byte-exakt, git-getrackt und offline verifizierbar** im
Repo: `.harness/baseline/v6.13.0/{regelwerk,templates}/` plus
`SHA256SUMS`-Manifest. Ein `make baseline-verify`-Gate-Schritt bricht
fail-closed, wenn der vendored Baum manipuliert ist. Der Fetch-Mechanismus
(re-vendor / verify / freshness) ist als b-cad-eigenes Skript adoptiert —
mit den zwei dokumentierten u-boot-Schwächen gefixt.

## 2. Plan (vor Ausführung)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `.harness/baseline/v6.13.0/{regelwerk,templates}/` | neu (vendored, ~50 Dateien) | entpackt aus dem Release-Asset `lab-regelwerk.zip` des Tags v6.13.0; ZIP wird zuvor gegen die Release-`SHA256SUMS` verifiziert (`sha256sum --check`, belegt im Closure-Text); **keine** inhaltliche Bearbeitung |
| `.harness/baseline/v6.13.0/SHA256SUMS` | neu | Manifest über **beide** Bäume (regeneriert mit `find … -print0 | xargs -0 sha256sum`, Sortierung deterministisch), Grundlage des offline-Verify |
| `tools/harness/{fetch-baseline-cache.sh}` | neu | Adoption aus u-boot (208 Zeilen, Herkunfts-Vermerk im Header). Modi: re-vendor (Default, Netz, **außerhalb** der Gates), `--verify` (offline: `sha256sum -c` + Manifest-Deckung), `--check-freshness` (read-only Release-Listen-Abfrage, Exit 0 = Pin aktuell / 3 = Review-Bump fällig / 1 = Fehler; **kein** Auto-Update). Tag-Quelle ohne Argument: regex `vMAJOR.MINOR.PATCH` auf der Stand-Zeile in `harness/conventions.md` §Baseline (bis slice-063 steht dort noch kein Tag — bis dahin explicit-arg-Betrieb). **Fixes ggü. u-boot:** (1) Manifest-Regen/Check `-print0`/`-0`-sicher statt `xargs sha256sum`; (2) `--check-freshness` mit API-Pagination statt einzigem `per_page=100`-Page |
| `.devcontainer/Dockerfile` | ändern | neue Stage `baseline-verify`: `COPY . .` + `RUN tools/harness/fetch-baseline-cache.sh --verify v6.13.0` (hermetisch, `--network none`-fähig; Manifest-Check enthält Dateianzahl == Zeilenzahl). **Ausführungs-Klasse = Dockerfile-Stage** gemäß Makefile-Kopf („Jeder INTERNE Gate ist eine Dockerfile-Target-Stage"; Ausnahmen a-check/docs-check als Bind-Mount-Images) — das Fetch-Skript selbst bleibt Host-Werkzeug **außerhalb** der Gates (Muster `golden-regen`) |
| `Makefile` | ändern | Target `baseline-verify` → `$(GATE) --target baseline-verify .`; als **ersten** Schritt in `gates` aufnehmen. **Explizites Tag-Argument `v6.13.0`:** §Baseline trägt bis slice-063 keinen Tag — die Skript-Tag-Auflösung (Stand-Zeilen-Regex) würde sonst bei jedem Lauf Exit 1 liefern; slice-063 stellt auf Tag-Auflösung um |
| `AGENTS.md` §3-Gate-Tabelle + `harness/README.md` Gate-Tabelle | ändern | Pflicht: das `targets`-Modul ([MR-022](../../../../harness/conventions.md)) prüft **bidirektional** — jede reale Makefile-Regel muss in der `authority`-Tabelle stehen. Nur die Zeile für `baseline-verify` **plus die Rahmen-Einordnung** (Dockerfile-Stage), die die Tabellen ehrlich hält; inhaltliche Text-Änderungen an diesen Dateien sind **slice-064** |
| `.gitattributes` | ändern | Pin `.harness/baseline/** -text` — EOL-Drift-Schutz (Autocrlf/`text=auto` würde die byte-exakte Verifikation brechen); Präzedenz in dieser Datei (Text-Golden, dieselbe Begründung) |
| `.d-check.yml` | ändern (bedingt) | **Messung vorab**: scannt d-check versteckte Verzeichnisse (`.harness/`)? Trägt der Bundle-Korpus nackte `MR-<NNN>`-Tokens? Bei Befund: `scan.ignore`-Ventil für `.harness/baseline/**` — nur gemessen, nicht vorsorglich |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag (vendored Baseline + neues Gate) |

## 3. Bewusst NICHT Teil

- **§Baseline-Umstellung auf den Tag + Freshness-Audit + [MR-024](../../../../harness/conventions.md)** — slice-063
  (dort wird auch die Makefile-Zeile von `--verify v6.13.0` auf
  Tag-Auflösung umgestellt).
- **Tote `agents-regelwerk.md`-Referenzen reparieren** — slice-064 (der
  conventions.md-Anteil liegt bei slice-063; abgesehen von der
  targets-Pflicht-Zeile oben).
- **Inhaltliche Nutzung der vendored Templates** — b-cad bleibt bei seiner
  verkörperten Form; das `templates/`-Baum-Reisen ist reine
  Bundle-Fähigkeit (die `../templates/…`-Verweise des Regelwerks lösen
  netzlos auf).

## 4. Verifikations-Schnitt

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Byte-Identität zum Release-Asset** — jede vendored Datei stimmt mit dem (SHA256-geprüften) ZIP-Inhalt überein; das Manifest deckt beide Bäume ab | `sha256sum -c .harness/baseline/v6.13.0/SHA256SUMS` (offline) | Manipulation einer Datei ⇒ `make baseline-verify` rot (**gemessen**, nicht behauptet) |
| 2 | **Manifest-Deckung** — Dateianzahl == Manifest-Zeilen; keine Extra-Datei unterm Radar | `--verify`-Ausgabe im Closure-Text | Datei ergänzt, ohne Manifest-Regen ⇒ rot |
| 3 | **`make gates` bleibt hermetisch** — `baseline-verify` zieht kein Netz | Gates-Lauf; `--check-freshness` ist **nicht** Gates-Member | (Kriterium als Review-Befund benannt — eine messbare Gegenprobe erforderte die Netz-Operation im Gate, also genau den Mangel) |
| 4 | **Fresh-Clone-Bestand** — die vendored Dateien sind git-getrackt und auf dem frischen Klon da | manuelle Zählung `git ls-files .harness/baseline \| wc -l` im Closure-Text; **das `tracked`-Modul deckt `.harness/baseline/**` NICHT ab** (kein Doku-Link zeigt darauf — und es wird absichtlich keiner gesetzt) | Datei vergessen ⇒ Zähl-Abweichung im Closure-Text |
| 5 | **Makefile-Doku-Ehrlichkeit** — `baseline-verify` in AGENTS.md §3 + harness/README | `targets`-Modul (in `make gates` enthalten) | Tabellen-Zeile weggelassen ⇒ rot |

## 5. Definition of Done

- [ ] `.harness/baseline/v6.13.0/{regelwerk,templates}/` + `SHA256SUMS` committet; §4-1 und §4-2 im Closure-Text belegt.
- [ ] `tools/harness/{fetch-baseline-cache.sh}` mit den zwei u-boot-Fixes; `--verify` offline grün; `--check-freshness` einmal manuell gegen das Release-Liste-API gelaufen (Exit-Kodes dokumentiert).
- [ ] Dockerfile-Stage `baseline-verify` mit **explizitem Tag-Argument** `--verify v6.13.0`; `make baseline-verify` in `gates` als erster Schritt; `make gates` grün.
- [ ] `.gitattributes`-Pin `.harness/baseline/** -text` committet.
- [ ] d-check-Scan-Messung zum Bundle-Korpus im Closure-Text dokumentiert; `scan.ignore`-Ventil nur bei gemessenem Befund.
- [ ] §4-1-Gegenprobe (Manipulation ⇒ rot) einmal gemessen und im Closure-Text aufgeführt.
- [ ] AGENTS.md §3 + harness/README-Gate-Tabelle um die `baseline-verify`-Zeile (inkl. Dockerfile-Stage-Einordnung) ergänzt.
- [ ] CHANGELOG [Unreleased]-Eintrag.
- [ ] [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report unter `docs/reviews/`.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-09-29)

Report: [`2026-09-29-slice-062-plan.md`](../../../reviews/2026-09-29-slice-062-plan.md) —
**2 HIGH / 3 MEDIUM / 4 LOW / 3 INFO, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor.

| # | Behandlung |
|---|---|
| **HIGH-1** (Gate-Aufruf ohne Tag-Argument wäre permanent rot — §Baseline trägt bis slice-063 keinen Tag; Skript löst den Tag in allen Modi vor dem Dispatch auf) | Dockerfile-Stage und Makefile-Zeile nennen jetzt **explizit `--verify v6.13.0`**; slice-063 übernimmt die Umstellung auf Tag-Auflösung (§2-Zeile dort). |
| **HIGH-2** (Ausführungs-Klasse unbestimmt — „nur die Zeile" hätte den targets-Honesty-Vertrag gebrochen) | Entscheidung: **Dockerfile-Stage** (gemäß Makefile-Kopf); Fetch-Skript = Host-Werkzeug außerhalb der Gates (Muster `golden-regen`); AGENTS/README-Ergänzung um die Rahmen-Einordnung erweitert. |
| **MED-1** (EOL-Drift bricht byte-exakte Verifikation) | `.gitattributes`-Pin `.harness/baseline/** -text` als §2-Zeile + DoD (Präzedenz Text-Golden). |
| **MED-2** (§4-4 behauptete `tracked`-Sensor-Deckung, die es für `.harness/` nicht gibt) | Zeile ehrlich auf die manuelle `git ls-files`-Zählung reduziert; kein Probe-Link. |
| **MED-3** (d-check-Scan des Bundle-Korpus unausgesprochen) | Messung vorab als §2-Zeile + DoD; `scan.ignore`-Ventil nur bei gemessenem Befund. |
| **LOW-1** (Größen-Angabe) | 54 Dateien, ~512 KiB (gemessen). |
| **LOW-2/3** (Pagination = Robustheit, kein Lückenschluss; xargs-Fix vorsorglich) | Fixes bleiben, Begründung im Plan ist robustheitsbezogen — keine Textänderung nötig. |
| **LOW-4** (§4-3-Gegenprobe hypothetisch) | als Review-Kriterium benannt statt als laufende Gegenprobe. |

**Startbar:** nein — die HIGH-Fixes berühren Mechanik und Honesty-Rahmen;
Zweit-Lauf erforderlich (slice-060-Muster).

## 6. Risiken

- **R1 — Netz-Stelle im Setup.** Der re-vendor selbst braucht Netz (curl des
  Assets); das ist bewusst **außerhalb** der Gates (Freshness-Audit-Verfahren
  des Regelwerks: Netz-Operation, dann netzlose Prüfung). Für diese Migration
  ist das Asset bereits lokal verifiziert — der Slice selbst kommt ohne Netz
  aus.
- **R2 — u-boot-Fixes unbewiesen.** Die `-print0`-Fixes und die Pagination
  sind Code-Adaption, nicht 1:1-Kopie — §4-2 und ein manueller
  `--check-freshness`-Lauf (R2-Spinne: Exit 3 bei neuem Release) sind die
  Belege, nicht das u-boot-Vertrauen.
- **R3 — Repo-Größe.** 54 Textdateien (~512 KiB, gemessen) kommen in git;
  gegen den Vorteil netzloser, auditierbarer Baseline-Referenz bewusst in
  Kauf genommen (User-Entscheid). Der `.gitattributes`-Pin (§2) hält
  byte-exakte Verifikation über EOL-Einstellungen hinweg.

## 7. Trigger

- Projektinhaber-Entscheid 2026-09-29 (welle-7-Öffnung); 404-Befund
  `agents-regelwerk.md`; Release-Layout v6.13.0.

## 8. Closure-Trigger

- §4-1..§4-5 grün (§4-1 mit gemessener Gegenprobe); `make gates` grün; DoD
  abgearbeitet. Closure-Notiz.
- Danach startet slice-063 (Freshness-Audit + Tag-Pin), das die
  `targets`-Honesty-Form von slice-062 unangetastet lässt.

## 9. Sub-Area-Modus-Begründung

### Sub-Area: Harness-Tooling (vendored Baseline)

- **Modus:** BF (bestehendes Repo, Doku führt); **Dichte:** niedrig —
  mechanische Adoption mit zwei lokalen Fixes, keine Spec-Straten berührt.
- **Risiko:** niedrig — rein additiv (ein Skript, ein Target, vendored
  Texte); das einzige Gate-Verhalten ändert sich um einen fail-closed
  Offline-Schritt.
- **Warum kein Split:** Skript + Bundle + Target tragen sich gegenseitig
  (ohne Bundle verifiziert nichts, ohne Target prüft nichts).

## 10. Closure-Notiz

_(bei Ausführung auszufüllen)_
