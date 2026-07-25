# Validator-Skill — b-cad

* **Status:** Accepted (slice-047, 2026-07-25)
* **Version:** 1.0
* **Bezug:** Regelwerk Modul 8 (Agentenrollen — Rollen-Sequenz + Übergabe-Artefakte),
  Modul 11 §Abgrenzung Verification/Validation, [`spec/lastenheft.md`](../../spec/lastenheft.md)
  (Anforderungen + Abnahmekriterien)
* **Gilt für:** jeden Validator-Lauf in b-cad
* **Abgrenzung:** [`reviewer.md`](reviewer.md) (Plan + ADR) · [`verifier.md`](verifier.md) (DoD + Spec + Plan)

## Die Kernfrage

> Verification: „Bauen wir es **richtig**?" (gegen Plan/DoD) · **Validation: „Bauen wir das
> Richtige?" (gegen den realen Bedarf)**

Modul 8 nennt den gefährlichsten Fall ausdrücklich: **Verifikation grün, Validation rot** — dann
baut das Projekt *perfekt das Falsche*. Der umgekehrte Fall (Verifikation rot, Validation grün) ist
Prozess-Drift, auch wenn das Ergebnis zufällig passt.

## Wer diese Rolle spielt

In b-cad der **Projektinhaber**. Das ist regelwerk-konform — Rollen-Trennung ist **Kontext**-Trennung,
nicht Personen-Trennung (Modul 8). Dieser Skill beschreibt darum weniger einen Agenten-Lauf als die
**Übergabe**: was der Validator bekommen muss, um urteilen zu können, und was von ihm zurückkommt.

Ein AI-Lauf darf diese Rolle **nicht** stillschweigend mitspielen: „ist das, was der Benutzer
wirklich braucht?" ist genau die Frage, die ein Implementer-Kontext nicht beantworten kann, weil er
den Bedarf aus demselben Text ableitet, aus dem er gebaut hat.

## Übergabe-Artefakte (Modul 8 — ohne sie kein Rollenwechsel)

- **Verifier → Validator:** *Build-Artefakt + Slice-Resultat.* Konkret in b-cad: der Verify-Report
  (`docs/reviews/*-verify.md`) **und** ein real bedienbares Ergebnis — das gebaute Binary, ein
  erzeugtes Export-Artefakt, ein Beleg-Bild, oder die benannte Handlung, die der Benutzer jetzt tun
  kann und vorher nicht konnte.
- **Validator → Planner:** *Validierungsbeleg gegen realen Bedarf* — die Entscheidung plus ihre
  Begründung, die in die Closure-Notiz eingeht.

Ein Rollen-Sprung **ohne** diese Artefakte ist laut Modul 8 „der häufigste Pfad zu blinden Flecken".

## Was der Validator prüft

- Löst das Ergebnis die Anforderung **für einen Benutzer** — nicht nur für die DoD? Die Trennung ist
  in b-cad schon einmal teuer geworden: [`LH-FA-BLD-002`](../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[`003`](../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) galten als `done`, weil die Persistenz-
  **Mechanik** existierte; es gab aber keinen Aufruf-Pfad. Mechanik ≠ Anforderung.
- Ist der **Teilumfang** der richtige? Ein bewusst benannter Teilumfang ist zulässig — aber der
  Validator entscheidet, ob der *verbleibende* Rest den Nutzen trägt.
- Stimmt die Annahme über den Bedarf noch, die im Lastenheft steht? Fällt hier etwas auf, ist das
  ein **Lastenheft**-Signal (Planner/Architect), keine Implementierungs-Kritik.

## Was dieser Skill NICHT macht

- **Keine Qualitätsurteile** über den Diff (Reviewer) und **keine DoD-Prüfung** (Verifier) — kommt
  beides schon aus der vorherigen Übergabe.
- **Keine Reparatur** und keine Lösungsvorschläge auf Code-Ebene.
- **Kein stilles Nachbessern der Anforderung:** stellt sich heraus, dass das Lastenheft den Bedarf
  falsch fasst, ist das ein **Befund an den Planner**, keine Umdeutung im Nachhinein.

## Output

- `entscheidung`: **angenommen** | **angenommen mit benanntem Rest** | **abgelehnt**
- `begruendung`: woran der reale Bedarf gemessen wurde (Handlung, die jetzt möglich ist)
- `rest`/`folge`: was offen bleibt und wohin es geht (Folge-Slice, Lastenheft-Signal)

Die Entscheidung gehört in die **Closure-Notiz** des Slice — sie ist der Lerneintrag, mit dem der
Planner den Slice schließt (Modul 8: „Planner → Planner: Closure in `done/` + Lerneintrag").
