---
id: slice-063
titel: Freshness-Audit über die Adaptions-Liste gegen v6.13.0 + Baseline-Bump-Eintrag [MR-024](../../../../harness/conventions.md) + Tag-Pin in §Baseline
status: open
welle: welle-7-regelwerk-migration
lastenheft_refs: []
adr_refs: []
---

# Slice 063: Freshness-Audit + Baseline-Bump-Eintrag + Tag-Pin

**Status:** open — Detail-Schnitt vollzogen (2026-09-29). Kern-Slice der
`welle-7-regelwerk-migration`. Läuft nach slice-062 (liest die Baseline aus
`.harness/baseline/v6.13.0/regelwerk/`, netzlos). Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-09-29.

## Auslöser

welle-7 (Projektinhaber-Entscheid 2026-09-29: Umstellung auf v6.13.0 **mit**
Freshness-Audit). Das Verfahren schreibt das Update-Verfahren bei
Tag-Wechsel vor: Review **durch die Adaptions-Liste** mit fünf Ausgängen je
`MR-<NNN>`-Eintrag, Form-Review, Rückbau append-only, Stichprobe gegen den
Bestand. b-cad hat heute **keine** Version-Referenz (§Baseline sagt nur
„Template-Set 2026-06") und keine Bump-Prozedur — beides entsteht hier.

## 1. Ziel

`harness/conventions.md` §Baseline trägt einen **Tag-Pin** (`v6.13.0`) statt
eines Monatsstands; jeder der 24 Adaptions-Einträge ist gegen v6.13.0
klassifiziert — mit den fünf Ausgängen: **gegenstandslos / bleibt gültig /
teilweise überholt / Bezug entfallen / widerspricht**. Der
Baseline-Bump-Eintrag **[MR-024](../../../../harness/conventions.md)**
(append-only, Ablösungs-Präzedenz
[MR-003](../../../../harness/conventions.md#mr-003--docs-check-als-vendored-doku-sensor)→[MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)
/
[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)→[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie))
verankert die Klassifikationstabelle und die künftige Bump-Prozedur. Die
tote Raw-URL in §Adoptierte Konventions-Quellen ist ersetzt.

## 2. Plan (vor Ausführung)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `harness/conventions.md` §Baseline | ändern | **Stand: `v6.13.0`** (VERSION, kein Datum — sonst findet ein Versions-Sensor keine Version und bricht fail-closed ab), Zusatz „Kurs-Welle 153 · 2026-09-28" (aus `regelwerk/README.md` Stand-Zeile), Vendor-Pfad `.harness/baseline/v6.13.0/`, Datum der Adoption 2026-09-29 mit Erstadoption-Verweis (2026-06-08) |
| `harness/conventions.md` §Adoptierte Konventions-Quellen | ändern | tote `agents-regelwerk.md`-Raw-URL **ersetzen** durch: gepinnte Release-ZIP-URL (`…/releases/download/v6.13.0/lab-regelwerk.zip`) als externe Nennung + den vendored Pfad als Lesequelle (Inline-Code); Derivat-Hinweis (bei Konflikt gilt das Lehrmaterial) bleibt |
| `harness/conventions.md` Adaptions-Block: Eintrag [MR-024](../../../../harness/conventions.md) | neu (append-only) | `MR-024 — Baseline-Bump v1.3.0 → v6.13.0 (vendored)` mit: Datum, Geltungsbereich (gesamtes Repo), Adaption (Bump + künftige Bump-Prozedur), **Audit-Tabelle 24 Zeilen** (je Ausgang + knappe Begründung), Begründung, Auflösungs-Trigger (nächster Bump) |
| `CHANGELOG.md` | ändern | [Unreleased]-Eintrag (Regelwerk-Migration, Audit-Ergebnis-Zusammenfassung) |

**Audit-Methodik** (pro MR-Eintrag): betreffende Regelwerks-Abschnitte in
`.harness/baseline/v6.13.0/regelwerk/` lesen (Index: `regelwerk/README.md`),
neu gegen den Wortlaut des MR-Eintrags halten, Ausgang vergeben. Bekannte
Befunde aus der Vorbereitung (vom Audit je zu bestätigen):

- **[MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)
  → gegenstandslos**: b-cad deklarierte drei Spec-Ränge als Abweichung
  („Kurs-Default zwei"); v6.13.0 übernimmt exakt diese Form
  (`spec/spezifikation.md` = Rang 2 der 9-rangigen Source Precedence).
- **[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371)
  /
  [MR-023](../../../../harness/conventions.md#mr-023--spec-straten-sind-prozess-zeit-rein-d-check-matrix-klasse-temporal)
  → bleiben gültig** (deckungsgleich mit verschärfter Referenz-Richtung bzw.
  zeitlicher Reinheit der Spec-Straten; v6.13.0 verschärft Regel 5 noch
  einmal — „kein Spec-Dokument nennt eine ADR oder einen Slice, auch nicht
  in seiner Historie" — b-cads `matrix`-Gate deckt das Link- und Token-Modus
  ab, die Historie-Klausel wird im Audit bewertet).
- **Geltungsänderung „alle drei Spec-Straten obligatorisch"** wirkt nur auf
  **neue** Slices/Instanzen — bestehende Artefakte werden nicht rückwirkend
  umgeschrieben (Regelwerks-Regel: „Neue Instanzen folgen der neuen Form,
  bestehende werden nicht rückwirkend umgeschrieben"). Das Ergebnis steht in
  der [MR-024](../../../../harness/conventions.md)-Tabelle als
  Prozess-Hinweis, nicht als Nacharbeit-Auftrag.
- **Inline-MR-Form bleibt**: „Ein Eintrag je Datei" ist Default, Inline ist
  explizit erlaubte Wahl — b-cads 24 Inline-Einträge migrieren **nicht**; als
  Befund in [MR-024](../../../../harness/conventions.md) benannt.

## 3. Bewusst NICHT Teil

- **AGENTS.md / harness/README.md** — slice-064 (Pointer-Artefakte); die
  Einzige Ausnahme bleibt die targets-Pflicht-Zeile aus slice-062.
- **Rückwirkende Umschrift bestehender Slices/Specs** auf die neue
  Template-Form (siehe oben).
- **Neue Gates** für MR-Format oder Baseline-Pin — erst denken, dann
  mechanisieren (Muster: Regeln erst evidenzbasiert, dann computational);
  eigener Folge-Slice, falls sich ein Kandidat zeigt.

## 4. Verifikations-Schnitt

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Vollständige Klassifikation** — 24 Zeilen, jede mit Ausgang + Begründung; keine Lücke | Zählung im Closure-Text (24/24) | Ausgang „widerspricht" ohne Übernahme-Entscheid oder Carveout-Verweis ⇒ Review-Befund |
| 2 | **Tag-Pin maschinenlesbar** — die Stand-Zeile enthält `v6.13.0` in der Form, die `fetch-baseline-cache.sh` (slice-062) ohne explizites Argument auflösen kann | `fetch-baseline-cache.sh --verify` **ohne** Tag-Argument grün | Alte Prosa-Stand-Zeile ⇒ Skript-Exit ≠ 0 |
| 3 | **Keine tote Referenz mehr in conventions.md** | `grep agents-regelwerk harness/` → 0 Treffer | — |
| 4 | **Append-only-Disziplin** — die 24 Bestands-Einträge unverändert (diff-Leerheit), [MR-024](../../../../harness/conventions.md) rein hinzugekommen | `git diff` des Slice-Commits | Edit an Bestand-Einträgen ⇒ Review-Befund (Immutability-Disziplin) |
| 5 | **Gates grün** — insbesondere `ids` (Links auf den neuen Eintrag) und `anchors` | `make gates` | — |

## 5. Definition of Done

- [ ] [MR-024](../../../../harness/conventions.md) mit Audit-Tabelle (24 Zeilen, Ausgang + Begründung je Eintrag) im Adaptions-Block, append-only.
- [ ] §Baseline auf Tag-Pin umgestellt (§4-2 bestätigt); Vendor-Pfad + Adoption-Datum ergänzt.
- [ ] §Adoptierte Konventions-Quellen: tote URL ersetzt (§4-3 bestätigt).
- [ ] Geltungsänderungen (Spec-Straten, Regel 5, Inline-Form) als Prozess-Hinweise in [MR-024](../../../../harness/conventions.md) benannt — ohne Bestands-Rewrite.
- [ ] CHANGELOG [Unreleased]-Eintrag.
- [ ] `make gates` grün (inkl. slice-062er `baseline-verify`, jetzt ohne explizites Tag-Argument).
- [ ] [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report unter `docs/reviews/`.

## 6. Risiken

- **R1 — ein Audit-Ergebnis lautet „widerspricht".** Dann Entscheidung des
  Projektinhabers: Regelwerk-Regel übernehmen (MR-Eintrag append-only
  ablösen) oder Carveout deklarieren. Beides ist Form-konform; **nichts tun**
  ist es nicht.
- **R2 — Anchor-Brüche.** Der Anker des neuen Eintrags existiert erst mit der
  Implementation; die Slice-Pläne (062–064) referenzieren ihn deshalb
  bewusst **ohne Anker** — die Anker-Form wird bei der Implementation
  nachgetragen.
- **R3 — Audit-Tiefe.** 24 Einträge gegen 26 Regelwerks-Dateien sind ein
  Lesen-Auftrag mit Urteils-Anspruch; Gefahr ist oberflächliche
  Klassifikation. Gegenmittel: je MR die betroffene Regelwerks-Stelle im
  Audit-Text zitieren (Kurs-Welle-Zitier-Form: Tag + Pfad + §Abschnitt in
  Inline-Code statt Link — „der Sprung löscht den alten Pin").

## 7. Trigger

- welle-7 (Projektinhaber-Entscheid 2026-09-29); Freshness-Audit-Verfahren
  des Regelwerks v6.13.0 (Modul 2); 404-Befund `agents-regelwerk.md`.

## 8. Closure-Trigger

- §4-1..§4-5 grün; DoD abgearbeitet; [MR-024](../../../../harness/conventions.md)
  verankert. Closure-Notiz.
- Danach startet slice-064 (Pointer-Artefakte + Arbeitskonfiguration).

## 9. Sub-Area-Modus-Begründung

### Sub-Area: harness/conventions.md (Adaptions-Block)

- **Modus:** BF; **Dichte:** mittel — 24 klassifizierte Einträge gegen 26
  Regelwerks-Dateien, append-only-Ergänzung, keine Struktur-Umstellung.
- **Risiko:** mittel — konventionell-normativer Text mit Tragweite für alle
  Folgeslices; kein Code, kein Spec-Stratum.
- **Warum kein Split:** die Klassifikation ist **ein** Denkvorgang über
  eine Liste — teilen hieße, 24 Einträge an zwei Autoren-Kontexte zu
  verteilen.

## 10. Closure-Notiz

_(bei Ausführung auszufüllen)_
