---
id: slice-033
titel: Doku-Referenz-Gate — d-check v0.37.1 & Matrix-Verschärfung adr→slice (no-downward, SDP)
status: done
welle: welle-5-erweiterung (Quergewerk harness-steering)
lastenheft_refs: []  # reines Werkzeug-/Gate-Steering, keine LH-Anforderung
adr_refs: [[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)]  # Digest-/Dependency-Pinning-Prinzip (Pin-Hebung)
---

# Slice 033: Doku-Referenz-Gate — d-check v0.37.1 & Matrix `adr→slice`

**Status:** done (2026-07-05, **evidence-first** — Fallout vor dem
Plan gemessen, Muster [slice-027](../done/slice-027-lint-haertung.md)).
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
**0 HIGH** (3 MED eingearbeitet); `make gates` grün (docs-check über d-check.mk),
Closure-Notiz §8.

**Welle:** welle-5-erweiterung — **Quergewerk `harness-steering`** (kein
Wellen-Feature; Muster [slice-018a](../done-archive/slice-018a-dcheck-gate-mechanik-done-archive.md)
/ [slice-030](../done/slice-030-a-check-gate.md)). **DRW-Scope
(slice-032a/b/c) unberührt** — pausiert, WIP beiseite.

**Bezug:** Referenz-Integritäts-Gate `make docs-check` (d-check, [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths));
[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) (Digest-Pinning-Prinzip — die
Pin-Hebung ist ein bewusster Commit). Auslöser: der DRW-Grundsatz-ADR-Entwurf
referenzierte Folge-Slices im Körper — gegen **Stable Dependencies** (stabile ADR ↮
volatile Slice) und gegen b-cads eigene [ADR-Index](../../adr/README.md)-Konvention
(»Folgepflicht-Status im mutablen Index, **nicht** im ADR-Body«). Das Schwester-Tool
**a-check** fährt die Verschärfung bereits (dessen `.d-check.yml`: `adr→slice` +
`token` + Grandfathering).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-05.

**Schnitt-Herkunft:** Werkzeug-/Gate-Steering, aus dem DRW-ADR-Entwurf emergiert. Ein
Gate-Verschärfung + Pin-Hebung ist **kein** Wellen-Feature und gehört in ein eigenes
Quergewerk (Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[slice-030](../done/slice-030-a-check-gate.md)),
getrennt von der DRW-Feature-Arbeit. **Reine Doku-/Tooling-Entscheidung, kein
Produktions-Code.**

---

## 1. Ziel

Die **Referenz-Richtungs-Disziplin** (Stable Dependencies Principle / no-downward)
wird um die Kante **`adr → slice`** computational geschlossen: ein ADR-Körper darf
keine Slice-Kennung als Vorwärts-/Rückwärts-Abhängigkeit nennen (Folge-Slice-Nummern
leben im mutablen [ADR-Index](../../adr/README.md), Präzedenz-Verweise per ADR). Dazu
wird der `make docs-check`-Pin von **d-check v0.11.0 auf v0.37.1** gehoben (die
`token`/`exempt-paths`/`status-provenance`-Features gibt es ab v0.31). Die Kante
`spec-straten → slice` (in b-cad seit [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)
als Link-Form gesetzt) wird durch den geteilten `token` mitgeschärft (Klartext-Token,
nicht nur Links).

**Evidenz (vor dem Plan gemessen, Muster [slice-027](../done/slice-027-lint-haertung.md)):**
- **Pin-Sprung v0.11.0 → v0.37.1, Config unverändert: `docs-check` = 0 Befunde**
  (175 Dateien). Die neuen Module (`tracked`/`planning`/`commits`/`vcs`) sind opt-in
  und **nicht** in b-cads `modules:`-Liste → inert. `codepaths`-Verhalten unverändert.
  **Der 26-Minor-Sprung ist auf b-cads Doku fallout-frei.**
- **Matrix-Regel `adr→slice` + `token: slice-\d{3}` + Grandfathering(0001–0017):
  30 Befunde**, **alle** `matrix-forbidden` Slice-Tokens in den **Spec-Straten**
  (lastenheft 11 · spezifikation 18 · architecture 1). **0** ADR-Befunde — das
  Grandfathering trägt (0013–0017 mit 3–17 Slice-Tokens je ADR sind exempt).

Diese 30 Spec-Slice-Tokens sind **vorbestehende** Reifephase-/Verifikations-Provenance
(»Geschärft 2026-06-13 (slice-013a)«, »der slice-012-Teilumfang behandelt …«) — durch
die Token-Schärfung erstmals sichtbar. Sie werden **remediert**, nicht toleriert.

## 2. Definition of Done

- [x] **docs-check auf `d-check.mk` umgestellt** (v0.37.1, `DCHECK_DIGEST`
      `sha256:3bbdb19b…`, Bind-Mount `docker run --network none -v $(CURDIR):/repo:ro`;
      Muster a-check.mk/slice-030). `include d-check.mk` (erzeugt von
      `d-check --print-mk`), `docs-check` = Alias-Prerequisite auf `doc-check`,
      gate-consistency-Awareness. Die `tools/Dockerfile`-FROM-Stage ist **entfernt**
      (codepaths-`ignore-refs`-Tombstone für die eingefrorene slice-004-Referenz).
      **Evidenz:** Pin-Sprung-Fallout = 0 (v0.37.1, Config unverändert gemessen).
- [x] **`.d-check.yml` Matrix verschärft:** `token: 'slice-\d{3}'` auf der `slice`-Klasse
      (Klartext-Token-Erkennung), neue Regel `{from: adr, to: slice, allow: false}`,
      **Grandfathering** `exempt-paths: ["docs/plan/adr/000[1-9]-*.md",
      "docs/plan/adr/001[0-7]-*.md"]` (die immutablen Accepted-ADRs 0001–0017;
      [AGENTS.md §2.5](../../../../AGENTS.md)). Ab dem **DRW-Grundsatz-ADR** gilt die Disziplin
      (der DRW-ADR ist bereits no-downward geschrieben). *(Bereits als Messschritt
      vollzogen.)*
- [x] **Spec-Remediation der 30 Befunde** — die vorbestehenden Slice-Provenance-Tokens
      werden als **Verifikations-Zeiger** deklariert bzw. entfernt:
      **(a) `lastenheft.md` + `spezifikation.md` (29):** Zeilen-Marker
      `<!-- d-check:status-provenance -->` an die jeweilige Zeile (das
      handbuch-sanktionierte Mittel für »Verifikations-Zeiger, keine
      Entscheidungsgrundlage«). Der **Wortlaut bleibt unverändert** (unsichtbarer
      HTML-Kommentar; **kein** Eingriff in abnahmebindenden Text — wichtig für Rang-1).
      **(b) `architecture.md` (1, Zeile 87):** die Slice-Kennung wird **entfernt**
      (nicht markiert) — `architecture.md` ist per [AGENTS.md §2.7](../../../../AGENTS.md)
      **slice-frei** (»keine Slices«); der Requirement-Verweis
      [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden).a bleibt.
      **Kein** Auslagern nach `*-historie.md` (Marker erhält die inline-Traceability
      Slice→Requirement; risikoärmer als Vertragstext-Umbau).
- [x] **`harness/conventions.md` neuer Eintrag [MR-014](../../../../harness/conventions.md)** (Verschärfung, kein
      [AGENTS.md §2.6](../../../../AGENTS.md)-Lockerungsfall — die Regel wird **strenger**;
      Muster [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-013](../../../../harness/conventions.md#mr-013--arch-check-via-a-check)):
      dokumentiert (1) die docs-check-Umstellung auf `d-check.mk` + Pin-Sprung v0.11.0→v0.37.1 (Live-Pin in `d-check.mk`),
      (2) die neue Matrix-Kante `adr→slice` + `token` + Grandfathering, (3) den
      `status-provenance`-Marker als sanktioniertes Ausnahme-Mittel für Spec-Straten,
      (4) den `docs-check`-Vertrag »kein Slice-Token im ADR-Körper (ab 0018) / in den
      Spec-Straten (außer als deklarierter Verifikations-Zeiger)«. Auflösungs-Trigger
      permanent.
- [x] **Gate-Doku-Nachzug:** der `docs-check`-Vertrag in
      [`harness/README.md` §Sensors](../../../../harness/README.md) und
      [`AGENTS.md` §3](../../../../AGENTS.md) um die `adr→slice`-Kante ergänzen
      (Referenz-Richtung: Spec/ADR ↮ Slice). `.d-check.yml`-Kopfkommentar aktualisiert.
- [x] **`CHANGELOG.md`** (grobkörnig, [MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)):
      ein Eintrag unter `[Unreleased]` (d-check-Hebung + Matrix-Verschärfung).
- [x] **`make gates` grün** (docs-check 0 Befunde nach Remediation; die übrigen Gates
      unberührt — reine Doku-/Tooling-Änderung). `make schema-check`/`io-smoke`
      unberührt.
- [x] **Nicht Teil:** die DRW-Arbeit (slice-032a AK-Schärfung neu fassen, DRW-Grundsatz-ADR
      zurückführen + Index-Zeile, 032b/032c) — folgt **nach** dieser Steering-Closure
      unter dem v0.37.1-Gate; der ADR-Index-Grandfather-Rand (0001–0017) und der
      `entity_layers`-/DRW-Scope bleiben unberührt.

## 3. Plan (vor Ausführung)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `d-check.mk` (neu) · `tools/Dockerfile` (entfernt) · `Makefile` · `tools/gate-consistency.sh` · `tools/README.md` | neu/entfernt/ändern | docs-check auf d-check.mk umgestellt (Bind-Mount, `DCHECK_DIGEST` v0.37.1; Muster a-check.mk); `include d-check.mk`, `docs-check`→`doc-check`-Alias; `.d-check.yml` codepaths-`ignore-refs`-Tombstone |
| `.d-check.yml` | ändern | Matrix: `token: slice-\d{3}` + Regel `adr→slice` + Grandfathering `exempt-paths` 0001–0017 + Kopfkommentar |
| `spec/lastenheft.md` | ändern | 11 Zeilen `<!-- d-check:status-provenance -->` (Wortlaut unverändert) |
| `spec/spezifikation.md` | ändern | 18 Zeilen `<!-- d-check:status-provenance -->` (Wortlaut unverändert) |
| `spec/architecture.md` | ändern | Zeile 87: Slice-Kennung entfernen ([AGENTS.md §2.7](../../../../AGENTS.md) slice-frei) |
| `harness/conventions.md` | ändern | neuer Eintrag **[MR-014](../../../../harness/conventions.md)** (Verschärfung) |
| `harness/README.md` | ändern | §Sensors `docs-check`-Vertrag um `adr→slice`-Kante |
| `AGENTS.md` | ändern | §3 `docs-check`-Vertrag um `adr→slice`-Kante |
| `CHANGELOG.md` | ändern | `[Unreleased]`-Eintrag (grobkörnig) |
| `docs/reviews/{2026-07-05-slice-033-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review.**
  Der v0.37.1-Digest ist aufgelöst (a-check fährt ihn bereits), Pin-Sprung- und
  Regel-Fallout sind gemessen.

## 5. Closure-Trigger

- DoD vollständig, `make docs-check`/`make gates` grün, Closure-Notiz → die pausierte
  **DRW-Arbeit** (slice-032a neu fassen, DRW-Grundsatz-ADR zurück, 032b/032c) wird unter dem
  v0.37.1-Gate fortgesetzt.

## 6. Risiken und offene Punkte

- **Rang-1-Lastenheft-Berührung (benannt, minimiert):** 11 der Marker sitzen im
  `lastenheft.md` (abnahmebindend, Rang 1). **Mitigation:** der Marker ist ein
  **unsichtbarer HTML-Kommentar** — der abnahmebindende **Wortlaut ändert sich
  nicht**. Nur die computational Referenz-Klasse wird deklariert. Keine AK-, Werte-
  oder Semantik-Änderung.
- **Marker vs. Auslagern (Entscheidung):** die 29 Spec-Slice-Refs sind echte
  Reifephase-/Verifikations-Provenance (»in slice-NNNa geschärft«). Der
  `status-provenance`-Marker ist das **handbuch-sanktionierte** Mittel dafür und
  **erhält** die inline-Traceability Slice→Requirement; ein Auslagern nach
  `*-historie.md` (wie [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)
  für ADR-Refs) würde diese Traceability aus dem Spec-Text reißen und mehr
  Vertragstext anfassen — **verworfen zugunsten der Marker** (niedrigeres Risiko).
- **Geteilter `token` (kein Per-Regel-Scope):** der `token` sitzt auf der `slice`-Klasse
  und armiert **beide** Regeln (`spec→slice` **und** `adr→slice`). Eine
  Zwei-Klassen-Trennung (adr-Regel mit Token, spec-Regel ohne) wäre möglich, würde
  aber `spec→slice` unter der Token-Disziplin **schwächen** und von der
  a-check-Kanonik abweichen — **verworfen** zugunsten der vollen Disziplin (Spec **und**
  ADR slice-frei, außer deklarierte Verifikations-Zeiger).
- **Grandfathering-Rand exakt (0001–0017):** `exempt-paths` nimmt genau die immutablen
  Accepted-ADRs aus; der **DRW-Grundsatz-ADR** (0018, in Arbeit) ist **nicht** exempt und bereits
  no-downward geschrieben (verifiziert: 0 Slice-Tokens). Neue ADRs ab 0018 tragen die
  Disziplin; ein legitimer Verifikations-Zeiger in einem neuen ADR wäre per
  Zeilen-Marker zu deklarieren.
- **Reproduzierbarkeit ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)):** der
  Live-Pin bleibt digest-genau in `d-check.mk` (`DCHECK_DIGEST`); die Hebung ist ein bewusster
  Commit mit Begründung (Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/slice-030).
- **Kein Scope-Creep in DRW:** die DRW-Feature-Arbeit bleibt pausiert; dieser Slice
  ändert **kein** Produktions-Code, **kein** Schema, **keine** LH-Anforderung.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF; **Dichte:** hoch — Digest-Pinning-Konvention ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md),
  Live-Pin in `d-check.mk`), `docs-check`-Vertrag ([MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)),
  Reproduzierbarkeits-Inventur. **Phase-Reife:** Phase 4. **Risiko:** niedrig (0
  Versions-Fallout gemessen).

### Sub-Area: Konventionen & Harness-Doku

- **Modus:** GF; **Dichte:** hoch — Heimat der `MR-NNN`, Verschärfungs-Disziplin
  ([AGENTS.md §2.6](../../../../AGENTS.md): Verschärfung ≠ Lockerung, kein ADR),
  Referenz-Richtungs-Doktrin. **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-07-05):**

- **docs-check auf `d-check.mk` umgestellt** (v0.37.1, `DCHECK_DIGEST` `sha256:3bbdb19b…`,
  Bind-Mount `--network none`; Muster a-check.mk/slice-030) — `include d-check.mk`,
  `docs-check`→`doc-check`-Alias, gate-consistency-Awareness, `tools/Dockerfile` entfernt
  (codepaths-`ignore-refs`-Tombstone). **Versions-Sprung-Fallout = 0** gemessen (Hebung von
  v0.11.0, Config unverändert) — die neuen Module (`tracked`/`planning`/`commits`/`vcs`)
  opt-in/inert, `codepaths` unverändert.
- **Matrix `adr → slice`** aktiv (`.d-check.yml`: `token: slice-\d{3}` +
  `{from: adr, to: slice, allow: false}` + Grandfathering `exempt-paths` 0001–0017). Der
  DRW-Grundsatz-ADR (0018, im Scratchpad) ist bereits no-downward (0 Slice-Tokens, geprüft).
- **30 vorbestehende Spec-Straten-Slice-Tokens remediert** ([MED-2](../../../reviews/2026-07-05-slice-033-plan.md)-Triage):
  **12** `<!-- d-check:status-provenance -->`-Marker für reine Reifephase-Provenance
  (Wortlaut **unverändert** — auch im Rang-1-Lastenheft), **16** substanzielle Fachverweise
  **selbsttragend umformuliert** (Slice-Zeiger raus, Fachinhalt bleibt), `architecture.md`
  slice-frei ([AGENTS.md §2.7](../../../../AGENTS.md), Entfernung statt Marker).
- **[MR-014](../../../../harness/conventions.md)** angelegt (Lineage auf
  [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths));
  Gate-Doku ([`harness/README.md` §Sensors](../../../../harness/README.md) + [AGENTS.md §3](../../../../AGENTS.md) +
  `.d-check.yml`-Kopf) + CHANGELOG ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review
  0 HIGH** (3 MED eingearbeitet: Zählung 10/19; Marker-auf-Spec empirisch belegt; substanzielle
  Verweise umformuliert). `make docs-check` **0 Befunde** (176 Dateien); `make gates` grün.

**Lerneintrag:**

- **Evidence-first (Muster [slice-027](../done/slice-027-lint-haertung.md)):** den
  26-Minor-d-check-Sprung **isoliert** gemessen (0 Fallout) **vor** der Regel-Aktivierung —
  trennt Pin-Risiko sauber vom Regel-Fallout; die Regel-Aktivierung ergab exakt die vorhergesagten
  30 Spec-Befunde, 0 ADR (Grandfathering trägt).
- **`status-provenance` wirkt auf Spec-Straten, nicht nur `adr`** — empirisch belegt (Marker auf
  einer Lastenheft-Zeile klärte den Befund), gegen die a-check-Kommentar-Formulierung »im
  ADR-Körper« geprüft (Review-MED-3).
- **Marker unterdrückt, Umformulierung löst (Review-MED-2):** reine Provenance markiert (Wortlaut
  erhalten, wichtig für Rang-1), substanzielle Fachverweise umformuliert (echte SDP-Auflösung) —
  die Sonderbehandlung `architecture.md` (Entfernung) folgt [AGENTS.md §2.7](../../../../AGENTS.md).
- **born-in-slice-Referenzen:** die im Slice **selbst** entstehende [MR-014](../../../../harness/conventions.md)
  auf die conventions.md-**Datei** verlinkt (kein Anker-Rätselraten vor Existenz); die im
  Scratchpad liegende ADR (0018) generisch als »DRW-Grundsatz-ADR« referenziert (kein toter Link).

**Restrisiko / Nachfolge:** die pausierte **DRW-Arbeit** wird unter dem v0.37.1-Gate fortgesetzt —
der DRW-Grundsatz-ADR (0018) aus dem Scratchpad zurück + [ADR-Index](../../adr/README.md)-Zeile,
slice-032a als reine AK-Schärfung neu fassen (alle IDs verlinkt), dann 032b/032c. Die
borderline-substanziellen Spec-Marker sind ein benannter Re-Eval für eine spätere Spec-Bereinigung
(Auslagern der Reifephase-Provenance nach `*-historie.md`, Muster
[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)).
