---
id: slice-051
titel: Review-Artefakt-Pflicht — eine [`MR-006`](../../../../harness/conventions.md)/[`MR-009`](../../../../harness/conventions.md)-Behauptung ohne Report ist ein Befund
status: open
welle: welle-5-erweiterung
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md)]
---

# Slice 051: Review-Artefakt-Pflicht (harness-steering)

**Status:** open — **Skelett** (Scope-Reservierung nach
[MR-020](../../../../harness/conventions.md); Detail-Schnitt + eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
**beim Start**, nicht jetzt).

**Welle:** welle-5-erweiterung (Quergewerk / harness-steering).
**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-25.

## Auslöser (Befund aus slice-047, 2026-07-25)

Beim Abschluss von slice-047 fiel auf: der Plan behauptet im Kopf zwei
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Ergebnisse
(„047a 0 HIGH", „047b 3 HIGH / 2 MED"), aber **kein Report existiert**. Die Prüfung des Gesamtbestands
ergab eine **Lücke von vier aufeinanderfolgenden Slices**:

| Slice | [`MR-006`](../../../../harness/conventions.md) im Plan behauptet | Report in `docs/reviews/` |
|---|---|---|
| slice-044 | ja | vorhanden |
| **slice-045** | ja („zwei unabhängige [`MR-006`](../../../../harness/conventions.md), 0 HIGH") | **fehlt** |
| **slice-046a** | ja („1 HIGH → Option A") | **fehlt** |
| **slice-046b** | ja („0 HIGH") | **fehlt** |
| **slice-047** | ja (047a + 047b) | **fehlt** |
| slice-048a | ja | vorhanden |

Davor lückenlos von slice-011 bis slice-044, danach wieder ab slice-048a — **101 Reports**, ein Loch
von vier.

**Das ist ein Regelwerks-Verstoß, keine Stil-Frage.** Regelwerk **Modul 10** (Review Harness), §Reviewer
berichtet auch, was er nicht gefunden hat, wörtlich: »abgelegt wird **ein Report pro Lauf** unter
`docs/reviews/`, Folgeläufe als neue Datei statt Überschreibung«.

**Zwei weitere Modul-10-Lücken, im selben Zug erhoben:**

1. **Reviewer-Skill-Datei fehlte** — Modul 10: »Jeder Reviewer-Agent braucht eine Skill-Datei in
   `.harness/` mit ‚worauf achtest du in diesem Repo'«; ohne sie driftet das Verhalten zwischen
   Sessions. **In slice-047 bereits geschlossen:** `.harness/skills/reviewer.md` (Version 1.0).
2. **Negativbefund-Zeilen** (»geprüft, ohne Befund« je betrachtetem Bereich) tragen nur **13 von 101**
   Reports. Modul 10 nennt sie ausdrücklich als den Teil, »den ein Reviewer-Agent am ehesten weglässt,
   weil ihn niemand einfordert« — ohne sie ist ein grüner Report nicht auditierbar (»nichts gefunden«
   und »nicht angesehen« sehen identisch aus). Die neue Skill-Datei fordert sie ein; der **Alt-Bestand**
   bleibt, wie er ist.

## 1. Ziel

Die Report-Pflicht **computational** machen, statt sie der Disziplin zu überlassen — Muster der
bisherigen Steering-Slices (033–037: erst Regel, dann Gate). Eine Review-Ergebnis-Behauptung in einem
Slice-Plan ohne zugehöriges Report-Artefakt soll **rot** sein.

## 2. Lösungsraum (offen — Entscheid beim Start)

Bewusst **nicht** vorentschieden; drei Kandidaten, in aufsteigendem Aufwand:

- **(a) Link-Pflicht + `links`-Modul.** Konvention: jeder Plan **verlinkt** seinen Report als
  Markdown-Link; ein fehlendes Ziel meldet das bereits aktive `links`-Modul als `target-missing`.
  **Billigster Weg** (kein neues Werkzeug), aber er erzwingt nur die *Auflösbarkeit* eines vorhandenen
  Links — ein Plan **ohne** Link bleibt still. Deckt also den Fall nicht, der hier real eingetreten ist.
- **(b) Lokales Gate-Skript.** Prüft: Plan behauptet ein Review-Ergebnis ⇒ es existiert
  `docs/reviews/*-<slice-id>-*.md`. Fängt genau den realen Fall. **Gegenläufig zur slice-050-Richtung**
  (dort sind zwei lokale Gate-Skripte gerade retired worden) — braucht also eine Begründung, warum hier
  wieder eines entsteht.
- **(c) d-check-CR.** Ein Modul/Feature „Behauptung ⇒ Artefakt" im gepinnten Werkzeug. Präzedenz: der
  **d-check-Pilot-CR** (a-check-Lastenheft 0.14.0) und die `targets`-Ablösung. Längster Weg, aber der
  einzige, der die Regel für **alle** Konsumenten trägt und keinen lokalen Sonderweg schafft.

**Vorabklärung beim Start (Handbuch-Pflicht, nicht Inferenz):** ob d-check ≥ v0.51.1 dafür bereits ein
Modul trägt — die `d-check`-Fähigkeiten sind **am gepinnten Handbuch** zu prüfen
(`d-check`-Repo `docs/user/benutzerhandbuch.md`), nicht aus den Konfig-Kommentaren zu schließen
(Lehre aus slice-050).

## 3. Bewusst NICHT Teil

- **Keine rückwirkenden Reports für 045/046a/046b/047.** Ein Report dokumentiert einen **beobachteten**
  Lauf; einen nachträglich zu schreiben, der ein nicht beobachtetes Review beschreibt, wäre eine
  Fälschung der Audit-Spur. Die vier Lücken werden als **benannte Alt-Last** geführt (Closure-Notiz
  slice-047), nicht geheilt.
- **Kein Nachrüsten der Negativbefund-Zeilen im Alt-Bestand** (88 Reports) — die Skill-Datei greift ab
  jetzt; rückwirkend wäre es dieselbe Fälschungs-Klasse.
- **Keine neue MR**, solange der Lösungsweg offen ist: hier wird nichts von der Baseline **abgewichen**
  — im Gegenteil, die Baseline (Modul 10) wird wieder eingehalten. Eine MR entsteht nur, falls der
  gewählte Weg eine b-cad-spezifische Adaption erzwingt.

## 4. Trigger

- Befund beim slice-047-Abschluss (2026-07-25), Projektinhaber-Auftrag „betriebsregelwerk-konform
  weiterarbeiten". Belegt am **gepinnten Regelwerk** (Release-ZIP `lab-regelwerk.zip`, Modul 10), nicht
  aus Inferenz.

## 5. Closure-Trigger

- Gate real + Negativprobe (ein Plan mit MR-Behauptung ohne Report ⇒ rot), `make gates` grün,
  Closure-Notiz.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — Regel-Reichweite.** Nicht jede MR-Nennung ist eine Ergebnis-**Behauptung**: Pläne
  verlinken [`MR-006`](../../../../harness/conventions.md) auch nur als Konventions-Referenz (slice-050 nennt es 15×). Ein naives „Plan enthält
  eine MR-Kennung ⇒ Report nötig" wäre ein Falsch-Positiv-Automat. Die auslösende **Form** muss präzise
  gefasst werden (z. B. nur die Kopf-Zeile „**[`MR-006`](../../../../harness/conventions.md)-Review** … HIGH/MED").
- **Rest-Risiko #2 — Slice-Suffixe.** Reports heißen `…-slice-042a-plan.md`, Pläne tragen Buchstaben-
  Suffixe; ein Umbrella-Plan (047) deckt mehrere Läufe (047a/047b). Die Zuordnung Plan↔Report ist
  **nicht** 1:1 — dieselbe Suffix-Blindheit, an der `--trace` in slice-038 scheiterte.
- **Rest-Risiko #3 — Richtungs-Konflikt mit slice-050.** Option (b) brächte ein lokales Gate-Skript
  zurück, das slice-050 gerade abgeräumt hat. Wer (b) wählt, begründet es explizit.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Harness-Steering / Gate-Konfiguration

- **Modus:** GF; **Dichte:** mittel. **Phase-Reife:** Gate-Infrastruktur reif. **Risiko:** niedrig
  (Doku-/Gate-Ebene, kein Produktionscode) — der Aufwand steckt in der **Regel-Fassung** (Rest-Risiko #1/#2),
  nicht in der Mechanik.

## 8. Closure-Notiz

_(bei Ausführung auszufüllen)_
