---
id: slice-035
titel: ADR-Immutabilitäts-Gate — d-check-Modul `vcs` (AGENTS §2.5 computational)
status: done
welle: welle-5-erweiterung (Quergewerk harness-steering)
lastenheft_refs: []  # reines Prozess-/Gate-Steering, keine LH-Anforderung
adr_refs: []         # kein ADR (Verschärfung/Prozess-Gate, §2.6 n/a)
---

# Slice 035: ADR-Immutabilitäts-Gate — Modul `vcs`

**Status:** **done** (ausgeführt 2026-07-05; angelegt evidence-first — Fallout + Enforcement vor
dem Plan gemessen, Muster [slice-034](../done/slice-034-commits-traceability.md)).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
1 HIGH behoben** (H1 — zweite §2.5-Fundstelle in `docs/plan/adr/README.md`; + 1 MED / 3 LOW
eingearbeitet; [Report](../../../reviews/2026-07-05-slice-035-plan.md)).

**Welle:** welle-5-erweiterung — **Quergewerk `harness-steering`** (kein Wellen-Feature;
Muster [slice-033](../done/slice-033-dcheck-matrix-adr-slice.md)/[slice-034](../done/slice-034-commits-traceability.md)).
**DRW-Scope (032a/b/c) unberührt** — pausiert.

**Bezug:** die §2.5-Immutabilitäts-Regel steht an **zwei** normativen Stellen —
[AGENTS.md §2.5](../../../../AGENTS.md) **und** [`docs/plan/adr/README.md` §Konventionen](../../adr/README.md)
(„ADRs sind nach `Accepted` immutable … Korrekturen/Schärfungen entstehen als neue ADR mit
`Supersedes ADR-NN`"; die ADR-README rangiert Source-Precedence-**höher**) — heute **reine Konvention**
(**kein** `adr-check`-Skript im Repo; die Regel war nie maschinell erzwungen). Das d-check-Modul
`vcs` (v0.33, [MR-014](../../../../harness/conventions.md)/d-check.mk-Weg schon da) macht sie
**computational** (git-Diff-Immutabilität des ADR-**Core**; tool-native Ablösung des
`adr-check`-Musters, das a-check dogfoodet — Präzedenz wie [MR-015](../../../../harness/conventions.md)/commits).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-05.

**Schnitt-Herkunft:** Modul-Adoptions-Steering (d-check-Modul-Fahrplan, Roadmap §Harness-Gate-
Fahrplan). `vcs` ist der zweite „Regel→computational"-Gewinner nach `commits` (slice-034).
**Reine Doku-/Tooling-Entscheidung, kein Produktions-Code.**

---

## 1. Ziel

Der **Core** einer `Accepted`-ADR (Datei-Inhalt ohne die mutable `## Geschichte`-Provenance)
darf über eine Commit-Range nicht mehr verändert werden — die [AGENTS.md §2.5](../../../../AGENTS.md)-Regel
wird von Konvention zu **Gate**. Umsetzung über `make doc-immutable` (d-check-Modul `vcs`,
git-Range/`--staged`), als **CI-only-Sensor** (Muster `make doc-commits`/`schema-check`) —
**nicht** in `make gates` (git-abhängig, nicht-hermetisch). Der Status-Übergang
(`Accepted` → `Superseded by ADR-NNNN`) ist via `head-allow` **erlaubt** (Praxis-Nuance +
`matrix`-Kopplung → §6).

**Evidenz (vor dem Plan gemessen, vier Sonden):**
- **Positiv** — Core-Edit einer `Accepted`-ADR (`--staged`) → **`core-drift-vcs`**: Enforcement
  greift, das Format `immutable-when: '^\*\*Status:\*\* Accepted'` trifft b-cads ADR-Kopf (Zeile 3).
- **Negativ** — Edit im `## Geschichte`-Abschnitt (`--staged`) → **0**: `exclude-sections:
  [Geschichte]` hält die Provenance mutabel (kein Fehlalarm für Geschichts-Nachträge).
- **Historie-Disziplin** (`HEAD~120..HEAD`) → **0**: keine `Accepted`-ADR wurde je am Core
  editiert — §2.5 ist gelebt.
- **Reale Gate-Range** (`origin/main..HEAD`) → **0**.

## 2. Definition of Done

- [x] **`.d-check.yml` `vcs`-Block** (`paths: [docs/plan/adr/[0-9]*.md]`, `immutable-when:
      '^\*\*Status:\*\* Accepted'`, `status-line: '^\*\*Status:\*\*'`, `exclude-sections:
      [Geschichte]`, `head-allow: '^\*\*Status:\*\* (Accepted|Superseded by ADR-\d{4})'` — `\d{4}`
      deckungsgleich zu `commits`/`ids` [Review-L1]).
      **NICHT** in die `modules`-Liste (git-abhängig, opt-in via `--enable`/`doc-immutable`).
      *(Bereits als Messschritt vollzogen.)*
- [x] **CI-only-Sensor `make doc-immutable`** (d-check.mk, git-Range) in die **CI-Befehlsliste**
      aufgenommen (Muster `make doc-commits`/`schema-check`): CI ruft `make doc-immutable
      RANGE=<base>..<head>` (typ. `origin/main..HEAD`); `core-drift-vcs` bricht ab (Exit 1).
      **Kein `make gates`-Member** (nicht-hermetisch). *(Kein `.github/workflows/` — die
      CI-Befehlsliste ist dokumentarisch; real-verdrahtet mit der ersten CI-Datei.)*
- [x] **Optional: lokaler `pre-commit`-Hook** (dokumentiert, opt-in): `make doc-immutable
      STAGED=1` fängt eine Accepted-ADR-Core-Änderung **vor** dem Commit. Kein Gate-Pflichtteil.
- [x] **`harness/conventions.md` neuer Eintrag [MR-016](../../../../harness/conventions.md)**
      (Verschärfung/Prozess-Gate, kein [§2.6](../../../../AGENTS.md)-Lockerungsfall; Muster
      [MR-015](../../../../harness/conventions.md)): dokumentiert (1) die `vcs`-Adoption + Config,
      (2) den CI-only-Sensor-Status, (3) `head-allow` erlaubt den Status-Übergang (defensiv;
      Supersede-Praxis + `matrix`-Kopplung siehe §6) + `Geschichte` bleibt mutabel (Provenance),
      (4) tool-native Ablösung des `adr-check`-Musters (a-check-Präzedenz), (5) `immutable`-Alternative
      erwogen + verworfen (siehe §6).
- [x] **Gate-Bindung an BEIDEN §2.5-Fundstellen (Review-H1):** die Immutabilitäts-Regel steht in
      [AGENTS.md §2.5](../../../../AGENTS.md) **und** in [`docs/plan/adr/README.md` §Konventionen](../../adr/README.md)
      (Source-Precedence-höher; §Folgepflichten trägt eine weitere Immutabilitäts-Passage). **Beide**
      erhalten den Gate-Bindungs-Hinweis (`make doc-immutable`, [MR-016](../../../../harness/conventions.md));
      zugleich wird der divergente ADR-README-Wortlaut („**Schärfungen**") mit §2.5 („**Korrekturen**")
      **abgeglichen** (kein stiller Regel↔Regel-Drift). *Der erste Grep-Sweep (nur AGENTS/README/
      conventions) hatte `docs/plan/adr/README.md` übersehen — die slice-034-H1-Lehre gilt jetzt real
      angewandt (repo-weiter Sweep), nicht nur behauptet.*
- [x] **Gate-Doku:** neue Zeile in [`harness/README.md` §Sensors](../../../../harness/README.md)
      + [AGENTS.md §3](../../../../AGENTS.md) — `make doc-immutable` als **CI-only-Sensor**
      (Muster `doc-commits`), Bindung [AGENTS.md §2.5](../../../../AGENTS.md).
- [x] **Roadmap-Fahrplan-Nachzug:** die `vcs`-Zeile in [`roadmap.md`](../in-progress/roadmap.md)
      §Harness-Gate-Fahrplan von „nächster Kandidat" → **adoptiert (slice-035)** + Quergewerk-Zeile
      §Historische Trigger-Verschiebungen (Muster slice-034).
- [x] **`CHANGELOG.md`** (grobkörnig, [MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- [x] **`make gates` grün** (unberührt — `vcs` **nicht** in `gates`; `.d-check.yml`-`vcs`-Block
      additiv, `docs-check` fasst ihn nicht an — `make docs-check` grün **mit** dem Block belegt).
      `make doc-immutable RANGE=origin/main..HEAD` = 0 + Positiv-/Negativ-Test belegt.
- [x] **Nicht Teil:** die DRW-Arbeit; die weiteren Modul-Kandidaten `planning` (Lifecycle) und
      `tracked` — eigene Folge-Slices.

## 3. Plan (vor Ausführung)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `.d-check.yml` | ändern | `vcs`-Block; **nicht** in `modules` |
| CI-Befehlsliste (`harness/README.md` §Sensors, [AGENTS.md §3](../../../../AGENTS.md)) | ändern | `make doc-immutable` als CI-only-Sensor (Muster `doc-commits`) |
| `AGENTS.md` §2.5 **+** `docs/plan/adr/README.md` §Konventionen | ändern | **beide** Fundstellen: Gate-Bindungs-Hinweis (`make doc-immutable`) + Wortlaut-Abgleich Schärfungen/Korrekturen (Review-H1) |
| `harness/conventions.md` | ändern | neuer Eintrag **[MR-016](../../../../harness/conventions.md)** (vcs-Adoption) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Fahrplan-Zeile `vcs` → adoptiert; Quergewerk-Eintrag |
| `CHANGELOG.md` | ändern | `[Unreleased]`-Eintrag |
| `docs/reviews/{2026-07-05-slice-035-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review durchlaufen** (1 HIGH behoben + 1 MED/3 LOW eingearbeitet) → **startbar.**
  Das `vcs`-Modul + `doc-immutable`-Target sind mit slice-033 (d-check v0.37.1 / d-check.mk) real;
  Config evidenzbasiert validiert (Positiv/Negativ/Historie/Range).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, `make doc-immutable RANGE=origin/main..HEAD` = 0,
  Closure-Notiz → die verbleibenden Modul-Kandidaten (`planning`, `tracked`, `--trace`) werden startbar.

## 6. Risiken und offene Punkte

- **`vcs` (git) vs. `immutable` (Marker, hermetisch) — Entscheidung:** `immutable` wäre hermetisch
  (→ echter `make gates`-Member, stärker), verlangt aber einen **`<!-- immutable: sha256:… -->`-Marker
  pro Accepted-ADR** (heute 16 Marker; 0005/0018 = `Proposed` [Review-INFO]) + Nachpinnen bei jeder
  legitimen Geschichte-Änderung, und modelliert den `Accepted`→`Superseded`-Übergang nicht nativ. `vcs`
  **erkennt Accepted-ADRs automatisch** über die Status-Zeile (`immutable-when`), braucht **keine**
  Marker, und `head-allow` erlaubt den §2.5-Supersede-Status-Übergang; a-check dogfoodet `vcs`
  (direkte Präzedenz). **Gewählt: `vcs`** — der CI-only-Nachteil
  (nicht-hermetisch, wie commits) wiegt leichter als 18 Marker + Pflege.
- **Nicht-hermetisch → CI-only (kein `gates`-Member):** `vcs` braucht eine git-Range/`--staged`, die
  der hermetische `make gates`-Pfad nicht liefert. Verankerung in der **CI-Befehlsliste** (Muster
  `doc-commits`/`schema-check`). Da **kein `.github/workflows/`** existiert, ist die Verdrahtung
  dokumentarisch; real-scharf mit der ersten CI-Datei (benannt). Der `STAGED=1`-Pre-commit-Hook
  fängt lokal früher (opt-in).
- **`## Geschichte` bleibt mutabel (bewusst):** `exclude-sections: [Geschichte]` nimmt die
  Provenance-Rand-Tabelle aus dem Core — Geschichts-Nachträge an einer Accepted-ADR sind **kein**
  Verstoß (Negativ-Test 0). Das entspricht der b-cad-Konvention (Provenance ist die einzige mutable
  ADR-Zone; Voll-Historie-Sonde `root..HEAD` = 0 belegt: kein anderer Abschnitt je post-Accept mutiert).
  **[Review-L2]** `[Geschichte]` ist bewusst **eng** (ADR-Template = `## Geschichte`); ein künftiger ADR
  mit Provenance-Heading „Historie"/„Änderungshistorie" bräuchte die Liste erweitert (Muster `matrix`
  `[Historie, Geschichte, Änderungshistorie]`) — sonst würden legitime Nachträge eingefroren.
- **`head-allow`-Semantik + Praxis-Nuance (Review-M1):** erlaubt nur `Accepted` (unverändert) oder
  `Superseded by ADR-NNNN`; Rückbau auf `Proposed`/Freitext flaggt (korrekt — §2.5: kein Status-Rückbau).
  **Zwei ehrliche Einschränkungen:** (1) der `Superseded by`-Zweig ist **defensiv/ungetestet gegen reale
  Praxis** — b-cad führt Supersede-/Erfüllungsstatus im **mutablen [ADR-Index](../../adr/README.md)
  §Folgepflichten**, **nicht im Body**; ob die Body-Status-Zeile je auf „Superseded by…" umgeschrieben
  wird, ist offen. (2) Eine reale Supersession ist eine **gekoppelte Operation**: `matrix.status.forbidden:
  [superseded, deprecated]` (Pflicht-`docs-check`) flaggt **jeden Inbound-Link** auf ein superseded Ziel
  (`matrix-inactive`) → neue ADR + Index + **Inbound-Ref-Remediation** gehören zusammen, sonst wird ein
  anderes Gate rot. `vcs` (CI-only) segnet nur die Status-Zeile — **nicht** die ganze Mechanik.
- **Kein Scope-Creep:** `planning`/`tracked`/`--trace` und die DRW-Arbeit bleiben unberührt. `vcs`
  ändert **kein** Produktions-Code, **kein** Schema, **keine** LH-Anforderung.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Konventionen & Harness-Doku

- **Modus:** GF; **Dichte:** hoch — Heimat der `MR-NNN`, Verschärfungs-Disziplin
  ([AGENTS.md §2.6](../../../../AGENTS.md): kein ADR für ein *strengeres* Gate), §2.5-Immutabilität.
  **Phase-Reife:** Phase 4. **Risiko:** niedrig (0 Historie-Fallout, Enforcement empirisch belegt).

### Sub-Area: Build & Toolchain

- **Modus:** GF; **Dichte:** hoch — d-check.mk/`.d-check.yml`-Konvention ([MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-014](../../../../harness/conventions.md)),
  CI-only-Sensor-Muster (`doc-commits`/`schema-check`). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Ausgeführt 2026-07-05** — DoD vollständig; `make gates` grün, `make docs-check` = 0 **mit**
`vcs`-Block, `make doc-immutable RANGE=origin/main..HEAD` = 0 (kein Accepted-ADR-Core im Slice berührt).

**Beobachtbare Closure-Kriterien:**
- d-check-Modul `vcs` adoptiert (`.d-check.yml`-Block; `paths` ADR-Glob, `immutable-when` Accepted,
  `exclude-sections` Geschichte, `head-allow` Supersede) — **nicht** im `modules:`-Set → `make gates`/
  `make docs-check` hermetik-erhaltend unberührt.
- `make doc-immutable` als **CI-only-Sensor** dokumentiert ([`harness/README.md` §Sensors](../../../../harness/README.md)
  + [AGENTS.md §3](../../../../AGENTS.md), Muster `doc-commits`).
- **Beide** §2.5-Regel-Fundstellen ([AGENTS.md §2.5](../../../../AGENTS.md) **+**
  [`docs/plan/adr/README.md` §Konventionen](../../adr/README.md)) tragen den Gate-Bindungs-Hinweis;
  divergenter ADR-README-Wortlaut („Schärfungen") mit §2.5 („Korrekturen") abgeglichen;
  [MR-016](../../../../harness/conventions.md) verankert.

**Lerneintrag:** Der [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review fing **denselben H1-Fehler wie slice-034** — die §2.5-Regel hatte eine
zweite normative Heimat (`docs/plan/adr/README.md` §Konventionen), die mein Grep-Sweep (nur AGENTS/README/
conventions) übersah; sie rangiert sogar Source-Precedence-**höher**. Verschärfte Lehre: der
„alle-Fundstellen-Sweep" muss **repo-weit** sein (`grep -rl` ohne Datei-Whitelist) — die slice-034-Lehre
nannte eine zu enge Grep-Liste. Zweite Lehre: eine Status-Transition, die *ein* Gate (`vcs`) segnet, kann
ein *anderes* Gate (`matrix.status.forbidden`) verletzen — Gate-Wechselwirkungen mitdenken.

**Folge:** die verbleibenden Modul-Kandidaten `planning` (Lifecycle), `tracked` und `--trace` werden startbar.
