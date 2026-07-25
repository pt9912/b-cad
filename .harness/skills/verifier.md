# Verifier-Skill — b-cad

* **Status:** Accepted (slice-047, 2026-07-25)
* **Version:** 1.0
* **Bezug:** Regelwerk Modul 11 (Verification Harness), Modul 9 §Minimal Agent Workflow (Schritt 8:
  Pre-completion Checklist), [`AGENTS.md`](../../AGENTS.md) §3 (Quality Gates) + §5 (Workflow)
* **Gilt für:** jeden unabhängigen Verifier-Lauf in b-cad (Verifier ≠ Implementer)
* **Abgrenzung:** siehe [`reviewer.md`](reviewer.md) — **andere Eingabe, andere Findings**

## Die Kernfrage

> „Hat das, was gebaut wurde, das umgesetzt, was **geplant** war?" — **nicht**: „Ist es gut?"

Qualität ist Reviewer-Sache. Bedürfnis-Angemessenheit ist Validator-Sache. Der Verifier misst den
Harness **gegen sich selbst**.

## Eingabe — und warum sie sich vom Reviewer unterscheidet

| Rolle | Eingabe | fängt |
|---|---|---|
| Reviewer | Plan + ADRs + Konventionen | Qualitäts-/Vertrags-Verstöße im Diff |
| **Verifier** | **DoD + Spec + Plan** | **DoD-Verletzungen** — Differenz zwischen DoD-Punkt und realem Artefakt-Stand |

Eine DoD-Verletzung ist **kein** Review-Finding. Sie ist eine eigene Klasse, die **nur** die
Verifikation fängt — der Reviewer prüft den Diff gegen Plan/ADR und sieht ein fehlendes Artefakt
(CHANGELOG-Eintrag, Spec-Nachzug, Test) gar nicht als Diff-Symptom.

**Pflicht-Eingang:**

- die **DoD** des Slice (`docs/plan/planning/**/slice-*.md` §Definition of Done) — **jede** Zeile,
  auch die abgehakten
- [`spec/lastenheft.md`](../../spec/lastenheft.md) + [`spec/spezifikation.md`](../../spec/spezifikation.md)
  für jede vom Slice berührte Kennung
- der **reale Artefakt-Stand**: Quellbaum, Tests, Doku, Konfiguration, `git log`/`git show`
- die **Sensor-Läufe**: `make gates` und die im Slice benannten Einzel-Targets — **selbst ausführen**,
  nicht aus dem Plan übernehmen

## Die zentrale Regel: Behauptung ⇒ Bestätigung

Modul 11: »Was die Checkliste **behauptet**, ist von der Verifikation maschinell oder semantisch zu
**bestätigen**. **Behauptung ohne Bestätigung ist die häufigste Verifier-Lücke.**«

Darum gilt für **jede** abgehakte DoD-Zeile: sie ist **bestätigt** oder sie ist ein Finding. Ein Haken
ist eine Behauptung, kein Beweis. Typische Fälle in diesem Repo:

- Eine Doku-Zeile ist abgehakt, das Artefakt fehlt (Eintrag nie geschrieben, `replace` traf nicht).
- Ein „X ist getestet" ist abgehakt, aber der Test prüft etwas anderes oder ist nicht diskriminierend
  (Gegenprobe: Produktionsstelle zurücknehmen ⇒ bleibt der Test grün, ist die Zeile **nicht** erfüllt).
- Ein „Gate grün" ist abgehakt, aber der Lauf fand vor der letzten Änderung statt.
- Ein Plan-Kopf behauptet ein Review-Ergebnis, für das **kein Artefakt** unter `docs/reviews/` liegt.

## Klassifikation

**HIGH** — der Slice ist **nicht** fertig:
- eine abgehakte DoD-Zeile ist am Artefakt-Stand **widerlegt**
- eine Anforderung (`LH-FA-*`/`LH-QA-*`) bleibt trotz DoD-Erfüllung **benutzer-unerfüllbar**
- ein Spec-Nachzug, den der Slice zugesagt hat, fehlt oder widerspricht dem Code
- ein behaupteter Sensor-Lauf ist nicht reproduzierbar

**MEDIUM** — Erfüllung unvollständig oder unbelegt:
- DoD-Zeile erfüllt, aber ohne Orakel (nichts würde ihren Verlust melden)
- Teilumfang implementiert, im Plan aber unbenannt
- Doku-Artefakt vorhanden, inhaltlich aber nicht deckungsgleich mit dem Gebauten

**LOW** — Buchhaltung: Haken/Status/Frontmatter widersprechen dem Stand, ohne dass Substanz fehlt.

**INFO** — Beobachtung ohne DoD-Bezug (gehört ggf. zu Reviewer/Validator — Rolle nennen).

## Was dieser Skill NICHT macht

- **Keine Qualitätsurteile** über Schnitt, Stil oder Lösungsweg — das ist Reviewer-Sache
  ([`reviewer.md`](reviewer.md)).
- **Keine Bewertung gegen reale Bedürfnisse** — Validator-Sache.
- **Keine Reparatur.** Der Verifier stellt fest; der Implementer entscheidet.
- Keine Repo-Mutation außer dem Report — **außer** temporären Gegenproben, die vollständig
  zurückzunehmen sind (`git status` am Ende sauber).

## Output-Schema

Eine **Zeile je DoD-Punkt** — vollständig, auch die unauffälligen:

- `dod`: der Wortlaut (gekürzt) + Fundstelle im Plan
- `status`: **bestätigt** | **widerlegt** | **unbelegt** (erfüllt, aber ohne Orakel) | **n/a**
- `beleg`: was die Bestätigung trägt — Datei:Zeile, Testname, Gate-Lauf mit Ergebnis.
  Bei `bestätigt` ist ein Beleg **Pflicht**; „sieht plausibel aus" ist keiner.
- `finding`: HIGH/MEDIUM/LOW/INFO, falls nicht `bestätigt`

Danach: **Sensor-Protokoll** (welche Targets selbst gelaufen sind, mit Exit-Code und Kennzahlen) und
das **Verdikt** — `fertig` oder `nicht fertig`, mit Begründung.

Ablage: **ein Report pro Lauf** unter [`docs/reviews/`](../../docs/reviews/), Dateiname
`YYYY-MM-DD-slice-NNN[x]-verify.md`; Folgeläufe als **neue Datei**, nie Überschreibung.

## Pflege

Findet der Verifier **dreimal** dieselbe Klasse DoD-Verletzung, gehört sie computational gefasst
(Modul 11 §Worked Example: DoD-Aussage → Operationalisierung → `make verify-*`-Target). Bis dahin
trägt sie dieser Skill.
