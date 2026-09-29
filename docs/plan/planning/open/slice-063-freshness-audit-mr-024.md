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
| `harness/conventions.md` Adaptions-Block: Eintrag [MR-024](../../../../harness/conventions.md) | neu (append-only) | Titel »Baseline-Bump v1.3.0 → v6.13.0 (vendored)«; **Pflichtglieder nach Baseline** (`grundlagen-harness-dateien.md` §Adaptions-Block): Datum · Geltungsbereich (gesamtes Repo) · **`Ersetzt-Baseline-Regel`** (modul-02 §Freshness-Audit — die dortige Update-Prozedur wird als b-cad-eigenes Bump-Verfahren adaptiert; ohne das Feld wäre der Eintrag Fork, keine Adaption) · **`Löst auf:` [MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)** + **`Ausgelöst durch Baseline-Stand: v6.13.0`** (Nachfolger-Semantik für die „gegenstandslos"-Klassifikation; je weiterer „gegenstandslos"/„widerspricht"-Fälle verankert die Tabelle sie je Zeile) · Adaption (Bump + künftige Bump-Prozedur) · **Audit-Tabelle 24 Zeilen** (je Ausgang + knappe Begründung) · Begründung · Auflösungs-Trigger (nächster Bump) |
| `harness/conventions.md` — **Form-Review** (Template-Abgleich) | prüfen/ändern | `harness/conventions.md` gegen `templates/harness/conventions.template.md` (Pflichtglieder: §Baseline-Drei-Felder, Adaptions-Block-Form, Zusatzklassen, Modus-Deklaration). Das `diff -r` der Templates ist beim Erst-Tag nicht ausführbar (kein `<alt>`-Verzeichnis) — Träger ist der Abgleich der **gefüllten** Artefakte gegen das Template; Befunde werden im selben Slice behoben oder als benannte Offene in [MR-024](../../../../harness/conventions.md) geführt |
| `harness/conventions.md` — **Stichprobe** (rotierend) | prüfen | der Bestands-Abschnitt dieses Audits: die [MR-000](../../../../harness/conventions.md)-Aussage („keine inhaltlichen Adaptionen") gegen modul-02 — [MR-000](../../../../harness/conventions.md) wird als Adoptions-Erklärung, nicht Adaption geprüft (deshalb zählt die Tabelle ihn mit, klassifiziert ihn aber über die Stichprobe, nicht über die 5 Ausgänge) |
| `Makefile` | ändern | slice-062er Gate-Aufruf `--verify v6.13.0` → `--verify` (ohne explizites Argument — die Tag-Auflösung aus der §Baseline-Stand-Zeile greift jetzt; DoD-Zeile „ohne explizites Tag-Argument") |
| `docs/plan/planning/open/{slice-062,slice-063,slice-064}*.md` | ändern | **Anker-Retrofit**: die Pläne referenzieren [MR-024](../../../../harness/conventions.md) bewusst ohne Anker — nach Implementation auf die Anker-Form nachziehen (L3) |
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
- **Lockerungs-Lens** (je Eintrag): war die Adaption eine **Lockerung** der
  damaligen Baseline und verschärft die neue Baseline an derselben Stelle, ist
  die richtige Antwort ein **Carveout**, keine stille Dauer-Adaption
  (betroffen sein können z. B. [MR-004](../../../../harness/conventions.md)/CHANGELOG als ausdrückliche
  Abweichung) — die Prüfung gehört in die Zeile.

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
| 1 | **Vollständige Klassifikation** — 24 Zeilen, jede mit Ausgang + Begründung; keine Lücke | Zählung im Closure-Text (24/24) | Ausgang „widerspricht" ohne einen der drei formkonformen Auflösungs-Wege (Übernahme als Rückbau · Adaption gilt weiter mit **benanntem Widerspruch** in der Zeile · Carveout bei unreifer Übernahme) ⇒ Review-Befund |
| 2 | **Tag-Pin maschinenlesbar** — die Stand-Zeile enthält `v6.13.0` in der Form, die `fetch-baseline-cache.sh` (slice-062) ohne explizites Argument auflösen kann | `fetch-baseline-cache.sh --verify` **ohne** Tag-Argument grün | Alte Prosa-Stand-Zeile ⇒ Skript-Exit ≠ 0 |
| 3 | **Keine tote Referenz mehr in conventions.md** — die Reste (AGENTS.md-Einleitung, harness/README-Guides-Zeile) sind bewusst slice-064 | `grep agents-regelwerk harness/conventions.md` → 0 Treffer | — |
| 4 | **Append-only-Disziplin** — die 24 Bestands-Einträge unverändert (diff-Leerheit), [MR-024](../../../../harness/conventions.md) rein hinzugekommen | `git diff` des Slice-Commits | Edit an Bestand-Einträgen ⇒ Review-Befund (Immutability-Disziplin) |
| 5 | **Gates grün** — insbesondere `ids` (Links auf den neuen Eintrag) und `anchors` | `make gates` | — |

## 5. Definition of Done

- [ ] [MR-024](../../../../harness/conventions.md) mit Audit-Tabelle (24 Zeilen, Ausgang + Begründung je Eintrag) im Adaptions-Block, append-only.
- [ ] §Baseline auf Tag-Pin umgestellt (§4-2 bestätigt); Vendor-Pfad + Adoption-Datum ergänzt.
- [ ] §Adoptierte Konventions-Quellen: tote URL ersetzt (§4-3 bestätigt).
- [ ] Geltungsänderungen (Spec-Straten, Regel 5, Inline-Form) als Prozess-Hinweise in [MR-024](../../../../harness/conventions.md) benannt — ohne Bestands-Rewrite.
- [ ] **Form-Review** vollzogen (conventions.md gegen `conventions.template.md`); Befunde behoben oder als benannte Offene in [MR-024](../../../../harness/conventions.md) geführt.
- [ ] **Stichprobe** vollzogen ([MR-000](../../../../harness/conventions.md)-Aussage gegen modul-02), Ergebnis im Closure-Text.
- [ ] **Anker-Retrofit** vollzogen: Pläne slice-062/063/064 referenzieren [MR-024](../../../../harness/conventions.md) mit Anker-Form.
- [ ] CHANGELOG [Unreleased]-Eintrag.
- [ ] `make gates` grün (inkl. slice-062er `baseline-verify`, jetzt ohne explizites Tag-Argument).
- [ ] [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report unter `docs/reviews/`.

## 6. Risiken

- **R1 — ein Audit-Ergebnis lautet „widerspricht".** Drei formkonforme
  Auflösungen: (a) Adaption gilt weiter, der Widerspruch wird **in der
  Zeile benannt** (kein Inhaber-Entscheid nötig); (b) Übernahme als Rückbau
  (neuer MR-Eintrag, append-only); (c) Carveout, falls die Übernahme unreif
  ist. **Nichts tun** — Widerspruch stillschweigend übernehmen — ist es
  nicht.
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

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-09-29)

Report: [`2026-09-29-slice-063-plan.md`](../../../reviews/2026-09-29-slice-063-plan.md) —
**2 HIGH / 2 MEDIUM / 3 LOW / 3 INFO, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor.

| # | Behandlung |
|---|---|
| **H1** (§4-3-Orakel unerfüllbar: grep über `harness/` trifft auch harness/README.md — ausdrücklich slice-064) | grep auf `harness/conventions.md` verengt, 064-Reste in der Zeile benannt. |
| **H2** ([MR-024](../../../../harness/conventions.md) ohne `Ersetzt-Baseline-Regel` = Fork-Klasse; „gegenstandslos"-Klassifikation ohne Nachfolger-Semantik) | Pflichtglieder ergänzt: `Ersetzt-Baseline-Regel` (modul-02 §Freshness-Audit), `Löst auf:` [MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht), `Ausgelöst durch Baseline-Stand: v6.13.0`; je weitere „gegenstandslos"/„widerspricht"-Fälle verankert die Tabelle je Zeile. |
| **M1** (Form-Review + Stichprobe angekündigt, nicht operationalisiert) | zwei §2-Zeilen (Form-Review = Abgleich der gefüllten Artefakte gegen `conventions.template.md`, da `diff -r` beim Erst-Tag nicht ausführbar; Stichprobe = [MR-000](../../../../harness/conventions.md)-Aussage gegen modul-02) + DoD-Zeilen. |
| **M2** („widerspricht"-Orakel verlangte Inhaber-Entscheid, wo die Quelle drei formkonforme Auflösungen kennt) | §4-1 + R1 auf die drei Auflösungs-Wege erweitert — (a) benannter Widerspruch ist formkonform ohne Entscheidung. |
| **L1** (slice-062 §3 schob die tote URL komplett auf 064) | in slice-062 nachgezogen (063 übernimmt conventions.md). |
| **L2** (Lockerungs-Lens fehlte) | Methodik-Bullet ergänzt (Carveout statt stiller Dauer-Adaption bei Lockerung + Baseline-Verschärfung). |
| **L3** (Anker-Retrofit ungescheduled) | §2-Zeile + DoD-Zeile. |

**Startbar:** nein — Zweit-Lauf nach slice-060-Muster erforderlich (H2
berührt die Normativität des neuen Eintrags; das verlangt unabhängige
Prüfung).
