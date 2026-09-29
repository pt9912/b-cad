---
id: slice-064
titel: Pointer-Artefakte auf v6.13.0 umstellen — AGENTS.md-Lesediscipline + harness/README-Guides + Arbeitskonfiguration
status: open
welle: welle-7-regelwerk-migration
lastenheft_refs: []
adr_refs: []
---

# Slice 064: Pointer-Artefakte + Arbeitskonfiguration

**Status:** open — Detail-Schnitt vollzogen (2026-09-29). Drittes Slice der
`welle-7-regelwerk-migration`; läuft nach slice-062 (vendored Baseline
existiert) und slice-063 (§Baseline trägt den Tag). Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-09-29.

## Auslöser

Der Regelwerk-Einstieg aus [AGENTS.md](../../../../AGENTS.md) (Einleitung)
verweist auf das
`agents-regelwerk.md`-Destillat („einmal pro Session lesen") — die Raw-URL
liefert **404**, und v6.13.0 ändert die Einstieg-Form: **die verkörperte Form
des Adopters (AGENTS.md, Konventionen, ausgefüllte Artefakte) führt; das
Regelwerk wird pro Entscheidung nachgeschlagen** — Einstieg ist der Index
`regelwerk/README.md` des vendored Bundles, geladen wird nur der benötigte
Abschnitt. Nicht das ganze Regelwerk im Kontext halten.

## 1. Ziel

Alle Regelwerk-Referenzen der Pointer-Artefakte zeigen auf die **vendored
Baseline v6.13.0** (netzlos, git-getrackt) statt auf eine tote Raw-URL und
ein per-Session-/tmp-Verfahren. Die Lesediscipline in `AGENTS.md` beschreibt
das Nachschlage-Modell. `grep agents-regelwerk` über die normativen
Dokumente → **0 Treffer**.

## 2. Plan (vor Ausführung)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `AGENTS.md` (Einleitung) | ändern | Absatz „Betriebsregelwerk in Agenten-Kurzform" neu: Quelle = `.harness/baseline/v6.13.0/regelwerk/` (Inline-Code; git-getrackt), Einstieg `regelwerk/README.md` als nachschlagbare Vertiefung, Version-Pin v6.13.0 (§Baseline `harness/conventions.md`); Lesediscipline: verkörperte Form führt, **pro Entscheidung nur den benötigten Abschnitt** laden; nachschlagen erfordert kein „einmal pro Session"-Volllesen |
| `AGENTS.md` §3-Gate-Tabelle | prüfen/ändern | nur falls slice-062s `baseline-verify`-Zeile hieran rückt (soll sie nicht — separater Edit) |
| `harness/README.md` Guides-Tabelle | ändern | tote `agents-regelwerk.md`-Zeile **ersetzen** durch zwei vendored Zeilen (Muster des README-Templates): `.harness/baseline/v6.13.0/regelwerk/` (Betriebsregelwerk, nach Module) + `.harness/baseline/v6.13.0/templates/` (Ziel-Form, parallel — die `../templates/…`-Verweise des Regelwerks lösen netzlos auf); Stand-Verweis auf conventions.md §Baseline bleibt |
| `.claude/settings.local.json` | ändern (lokal) | curl-Allowlist um das Release-Asset-Muster des Tags v6.13.0 ergänzen — der `/tmp`-Fall dient künftig **nur** dem re-vendor (slice-062-Skript), nicht mehr dem Session-Lesen |
| `CHANGELOG.md` | — | kein Eintrag: benutzer-sichtbar ändert sich nichts; die Migration als Ganze ist in slice-062/063 verzeichnet |

## 3. Bewusst NICHT Teil

- **§Baseline / [MR-024](../../../../harness/conventions.md) / Audit** — slice-063.
- **Umstrukturierung der Pointer-Artefakte auf die neue Ziel-Form**
  (§1 „Was diese Datei ist", §Leseordnung als eigene Pflichtsektion,
  `harness/sensors/<target>.md`-Split) — die Voll-Form-Adaption ist ein
  möglicher **Folge-Slice**; die Form-Review-Erkenntnisse aus slice-063
  (templates-diff) entscheiden über Anlass und Schnitt. Hier nur die
  referenzielle Reparatur + Lesediscipline.
- **Inhaltliche Änderungen an `docs/user/`** — das Handbuch beschreibt das
  Produkt, nicht den Harness.

## 4. Verifikations-Schnitt

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Keine tote Regelwerk-Referenz** — `grep -rn agents-regelwerk AGENTS.md harness/ docs/ spec/` → 0 Treffer | grep im Closure-Text | Alte Zeile steht gelassen ⇒ rot |
| 2 | **Alle neuen Referenzen resolven und sind getrackt** — die vendored Pfade existieren (slice-062) und sind git-getrackt | `tracked`-Modul (`make gates`) | Referenz auf ungetrackten Pfad ⇒ rot |
| 3 | **Session-Lesen kommt ohne Netz aus** — die Lesediscipline benennt den vendored Pfad als Quelle | Lesen von `.harness/baseline/v6.13.0/regelwerk/README.md` in einer frischen Session (belegt im Closure-Text) | — |
| 4 | **Gates grün** — insbesondere `links`/`anchors`/`targets` | `make gates` | — |

## 5. Definition of Done

- [ ] `AGENTS.md`-Einleitung: Lesediscipline + vendored Quelle + Version-Pin (§4-3 belegt).
- [ ] `harness/README.md`-Guides-Tabelle: zwei vendored Zeilen, tote Zeile entfernt.
- [ ] `grep -rn agents-regelwerk AGENTS.md harness/ docs/ spec/` → 0 Treffer (§4-1 belegt).
- [ ] `.claude/settings.local.json`-Allowlist ergänzt.
- [ ] `make gates` grün.
- [ ] [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report unter `docs/reviews/`.

## 6. Risiken

- **R1 — doppelter Stand.** Nach diesem Slice nennen AGENTS.md (v6.13.0) und
  §Baseline (v6.13.0 ab slice-063) dieselbe Version — die Pin-Ehrlichkeit
  lebt in §Baseline; AGENTS.md nennt sie nur referenziell. Doppel-Pflege
  vermeiden: AGENTS.md wiederholt **keine** Version-Zeile, die §Baseline
  widersprechen könnte (Form: „adoptierter Stand siehe conventions.md
  §Baseline").
- **R2 — Gewohnheits-Bruch im Session-Ablauf.** „Einmal pro Session
  vollständig lesen" wird zur Nachschlage-Disziplin — Gefahr, dass regulative
  Abschnitte (z. B. Modul 9/10) künftig gar nicht mehr gelesen werden.
  Gegenmittel: die Index-Funktion der `regelwerk/README.md` wird im AGENTS.md
  Text explizit benannt; der
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Reviewer
  prüft, ob die
  Nachschlage-Formulierung die Review-/Verifikations-Pflichten schwächt.
- **R3 — `.claude/settings.local.json` ist lokal.** Die Ergänzung gilt nur
  für diese Arbeitskopie; für Re-vendor auf anderen Maschinen dokumentiert
  slice-062s Skript-Header den Asset-URL.

## 7. Trigger

- welle-7 (Projektinhaber-Entscheid 2026-09-29); 404-Befund
  `agents-regelwerk.md`; Einstieg-Form des Regelwerks v6.13.0.

## 8. Closure-Trigger

- §4-1..§4-4 grün; DoD abgearbeitet. Closure-Notiz — **und damit ist die
  welle-7 closure-fähig** (alle drei Slices geliefert).

## 9. Sub-Area-Modus-Begründung

### Sub-Area: Pointer-Artefakte (AGENTS.md / harness/README.md)

- **Modus:** BF; **Dichte:** niedrig — referenzielle Reparatur an zwei
  Dateien plus lokale Konfiguration.
- **Risiko:** niedrig — kein Code, kein Spec-Stratum; das Risiko steckt in
  R2 (Verhaltens-Wortlaut), nicht in der Mechanik.
- **Warum kein Split:** AGENTS.md-Text und README-Tabelle referenzieren
  dieselbe Quelle — getrennt wären zwei Halbwahrheiten committet.

## 10. Closure-Notiz

_(bei Ausführung auszufüllen)_
