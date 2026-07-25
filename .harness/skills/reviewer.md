# Reviewer-Skill — b-cad

* **Status:** Accepted (slice-047, 2026-07-25)
* **Version:** 1.0
* **Bezug:** Regelwerk Modul 10 (Review Harness), [`AGENTS.md`](../../AGENTS.md) §2 (Hard Rules),
  [`harness/conventions.md`](../../harness/conventions.md)
  [MR-006](../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  (Plan-Review) und [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  (Code-Review)
* **Gilt für:** jeden unabhängigen Reviewer-Lauf in b-cad (Reviewer ≠ Autor)

Modul 10: „Jeder Reviewer-Agent braucht eine Skill-Datei mit ‚worauf achtest du in diesem Repo'" —
ohne sie driftet das Verhalten zwischen Sessions (gleiche Eingabe, andere Findings/Kategorien).

## Review-Arten (Modul 10 — was wogegen)

| Art | Gegenstand | Wann | b-cad-Anker |
|---|---|---|---|
| **Plan-Review** | der Slice-Plan gegen Spec + Accepted-ADRs | **vor** der Implementierung, kein Diff | [MR-006](../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) |
| **Design-Review** | Lösungs-Schnitt gegen die Architektur (Layer, Schnittstellen, ADR-Verträglichkeit) | vor dem Festzurren der Details | ADR-Text-Reviews |
| **Code-Review** | der fertige Diff gegen Plan + Konventionen | nach der Implementierung | [MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) |

Die Art gehört in den Report-Kopf. Ein Plan-Review, das einen Diff bewertet, ist keins.

## Kontext-Eingang (Pflicht)

Vor dem Lesen des Gegenstands zu beschaffen — ohne diesen Block sieht der Reviewer den Code,
aber nicht die Verträge, gegen die er prüft:

- der **Gegenstand** selbst (Diff der Range bzw. die Plan-Datei)
- [`spec/lastenheft.md`](../../spec/lastenheft.md) für jede referenzierte `LH-FA-*`/`LH-QA-*`/`ACC-*`-Kennung
- [`spec/spezifikation.md`](../../spec/spezifikation.md) für `REQ-TEC-*`/`E-*`-Kennungen und §1-Blöcke
- jede **ADR**, deren Id im Plan, im Diff oder in der Commit-Message vorkommt (Status beachten:
  `Accepted` ⇒ Core immutabel, [`AGENTS.md`](../../AGENTS.md) §2.5)
- [`AGENTS.md`](../../AGENTS.md) §2 (Hard Rules) + §3 (Quality Gates)
- die **letzten Reports zum selben Modul** in [`docs/reviews/`](../../docs/reviews/)
- bei Werkzeug-Behauptungen: das **gepinnte Handbuch** des Werkzeugs, nie die Inferenz
  (d-check/a-check liegen als Nachbar-Repos; die gepinnte Version steht in `d-check.mk`/`Makefile`)

## Klassifikation (b-cad-konkret)

**HIGH** — blockiert Merge/Start; eines der folgenden:
- Verstoß gegen eine **Hard Rule** ([`AGENTS.md`](../../AGENTS.md) §2): hexagonale Richtung (§2.1),
  nicht-atomare Persistenz (§2.2), Host-Toolchain statt `make` (§2.3/§2.9), Inline-Suppression (§2.4),
  Edit am Core einer `Accepted`-ADR (§2.5), Gate-Lockerung ohne ADR (§2.6)
- **ADR-Verstoß** (Layer-Grenze, Port-Richtung, Backend-Wahl)
- Korrektheitsfehler im **kritischen Pfad**: Persistenz/Crash-Recovery, Geometrie-Transaktionalität,
  Id-Vergabe, Fehler-Barriere des Plugin-Hosts
- eine **behauptete** Sensor-Deckung, die real nicht existiert (Honesty, §3) — inkl. „`make X` grün"
  ohne Lauf
- eine Quellen-Behauptung (Zitat, Zeilennummer, Handbuch-Fähigkeit), die der Quelle **widerspricht**

**MEDIUM** — vor Merge zu klären:
- unklare Fehlerbehandlung am Rand des Spec-Bereichs (`E-*`-Codes)
- fehlender **Negativtest** bei neuem öffentlichem Vertrag
- ein Test, dessen Zusicherung **nicht diskriminiert** (wäre auch ohne die Änderung grün)
- Wiederholung eines Musters, das schon zweimal LOW war
- Doku-Drift zwischen Plan/Code/AGENTS.md, die keinen Gate bricht

**LOW** — stilistisch/redaktionell ohne semantische Wirkung: Tippfehler, stale Zeilennummer,
unbenutzter Include, inkonsistente Benennung.

**INFO** — Hinweis ohne erwartete Aktion (z. B. „hierfür existiert bereits ein Gate, das du nicht
kennst"; „gehört zur Verifier-Rolle").

## Was dieser Skill NICHT macht

- **Keine Lösungsvorschläge** („schreib das so") — der Reviewer kategorisiert, der Implementer entscheidet.
  Ein Finding beschreibt den **beobachtbaren Befund**, nicht die Reparatur.
- Kein Refactoring-Vorschlag, der über den Gegenstand hinausgeht.
- **Keine Verifikation gegen die DoD** — das ist Verifier-Aufgabe (Modul 11).
- Keine Validierung gegen reale Bedürfnisse — Validator-Aufgabe.
- Keine Repo-Mutation außer der Report-Datei.

Fällt etwas aus diesen Kategorien auf: **INFO**-Finding mit Verweis auf die zuständige Rolle.

## Output-Schema

Jedes Finding trägt:

- `kategorie`: HIGH | MEDIUM | LOW | INFO
- `quelle`: ADR-Id, LH-/REQ-/E-Kennung, Hard-Rule-Nummer, MR-Id oder „Maintainability"
- `pfad`: `Datei:Zeile`
- `befund`: 1–2 Sätze, **beobachtbar**, ohne Lösungsvorschlag
- `verifizierbar`: ja/nein — gibt es einen Gate-/Test-Lauf, der den Befund bestätigen würde?
  (wenn ja: welchen)

**Pflicht am Ende: Negativbefunde.** Je betrachtetem Bereich eine Zeile „geprüft, ohne Befund".
Grund (Modul 10): „keine Findings in X" und „X nicht angesehen" sehen sonst identisch aus — beides
eine leere Liste. Die Negativbefund-Zeilen machen die **Abdeckung** des Laufs sichtbar und sind der
Teil, den ein Reviewer am ehesten weglässt, weil ihn niemand einfordert.

**Report-Gerüst:** Kopf-Metadaten (Review-Art · Gegenstand · Skill-Version · Modell · Eingangs-Kontext)
→ Findings nach Output-Schema (HIGH zuerst) → Negativbefunde → Kategorie-Summary → **Verdikt**.
Ablage: **ein Report pro Lauf** unter [`docs/reviews/`](../../docs/reviews/), Dateiname
`YYYY-MM-DD-slice-NNN[x]-{plan,code-review}.md`; Folgeläufe als **neue Datei**, nie Überschreibung.

## Pflege

Bei **dreimaligem** Auftreten desselben Findings:
- ist die Kategorie noch richtig? → Klassifikation hier schärfen
- gibt es einen ADR-/AGENTS.md-Eintrag, der es verhindert hätte? → Folge-ADR oder AGENTS.md-Update
- gibt es eine Fitness Function, die es prüfen würde? → Gate ergänzen (Modul 13,
  [`AGENTS.md`](../../AGENTS.md) §3)

Diese Datei wird **versioniert, nicht überschrieben**: die Versions-Zeile im Kopf hochziehen und die
Änderung begründen (Muster ADR-Hard-Rule, Modul 4).
