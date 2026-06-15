---
id: slice-018c
titel: Per-ID-Anker-Präzision — HTML-Anker für 17 Bullet-Sub-IDs (LOW-3-Auflösung)
status: done
welle: harness-steering
lastenheft_refs: []
adr_refs: []
---

# Slice 018c: Per-ID-Anker-Präzision (Inline-HTML-Anker, v0.9.0)

**Status:** done (2026-06-15). Folge-Slice von slice-018b (done) — löst dessen
**akzeptierten LOW-3** (Geschwister-Heading-Anker) auf. **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen**
([Report](../../../reviews/2026-06-15-slice-018c-plan.md) — 3 HIGH + 2 MED
**eingearbeitet**: Mengen korrigiert, Über-Reichweite selbst-verifiziert,
`sect`-Regex-Kollision behandelt). DoD offen bis `make gates` grün.

**Welle:** harness-steering (Quergewerk; setzt 018b fort). Bindung über [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
(Referenz-Richtung) + [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check) (docs-check via d-check) + [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths) (Referenz-
Integritäts-Gate) statt LH/ADR — Harness-Sensor-Slice. **[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) n/a** (keine
AK-/Inhalts-Änderung — reine Anker-Form).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-15.

**Bezug:** slice-018b verlinkte 17 Bullet-Sub-IDs (D3/IO/ROM/WAL) auf das
**Heading des ersten Requirements ihrer Sektion** (Weg-B-Kapitel-Semantik) —
gate-gültig, aber der Anker zeigt aufs Geschwister-Requirement (LOW-3, akzeptiert).
d-check **v0.9.0** (bereits gepinnt) erkennt **Inline-HTML-Anker**
(DC-FA-ANCH-001) → die Präzision ist jetzt auflösbar.

---

## 1. Discovery-Evidenz (Source v0.9.0 + Probe, 2026-06-15)

- **17 betroffene Sub-IDs**, alle in `spec/lastenheft.md` als **Bullets** (kein
  eigenes Heading) unter dem ersten Requirement ihrer Sektion: `D3-003..006`
  (erben [`#### LH-FA-D3-002`](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung), Z. 562-565), `IO-002..008` (erben [`#### LH-FA-IO-001`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import),
  Z. 707-713), `ROM-002..005` (erben [`#### LH-FA-ROM-001`](../../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen), Z. 198-201),
  `WAL-004/005` (erben [`#### LH-FA-WAL-003`](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), Z. 145-146).
- **Mengen sauber getrennt (H1, korrigiert):** die 17 IDs haben **26 rohe
  Text-Vorkommen** im Live-Korpus — das ist **nicht** die Repoint-Zahl. Davon:
  **19 in `lastenheft.md` selbst** (17 Definitions-Bullets + 2 ACC-Tabellen-Zellen,
  alle `in_target` → idlink fasst sie nicht an), **1 Log-Prosa**
  (`docs/reviews/…-018b-code-review.md`, nur Backtick), und **6 echte
  Cross-File-`[ID](…)`-Links**, die das Geschwister-Heading tragen und repointet
  werden: `adr/0002`, `adr/0007`, `adr/0009`, `adr/README.md`,
  `architecture.md` (2×). Unabhängig re-derived (Befehl im Report).
- **v0.9.0 `anchors` (`anchors.go` `htmlAnchors`/`AnchorSet`, DC-FA-ANCH-001.b):**
  `id="…"` an **beliebigem** Tag **plus** `name="…"` an `<a>`; **wörtlich**
  (case-sensitiv) verglichen; mit den Heading-Slugs zur `AnchorSet` vereinigt.
  Erkennung auf **vorverarbeiteten** Zeilen → Anker in Fences/Inline-Code zählen
  **nicht**. Leeres `id=""` erzeugt bewusst keinen Anker. (Alle vier Punkte gegen
  `anchors_test.go` im Review bestätigt.)
- **GitHub-Parität:** d-checks eigene slice-022 (DC-FA-ANCH-001) hält fest, dass
  GitHub `<a name>` (klassisch) und `id=` an beliebigem Element (modern) als
  Sprungziele rendert; d-check modelliert genau das. *(Die `user-content-`-Präfix-
  Mechanik — GitHub matcht `#frag` gegen den entpräfixten DOM-`id` — ist reales
  GitHub-Verhalten/Eigen-Wissen, nicht slice-022 zugeschrieben; M2.)* → die
  präzisen Anker navigieren real, nicht nur Gate-grün.
- **d-check-Pin:** bereits **v0.9.0** (`sha256:5bccf9…`) seit slice-018b; HEAD des
  Source-Repos `v0.9.0-2` = 2 reine Docs-Commits über dem Tag, kein Code-Drift →
  **kein Bump nötig**.

## 2. Entschiedene Strategie

- **Anker-Form:** je Sub-ID ein **leeres** `<a id="<id-lowercase>"></a>` direkt nach
  dem Bullet-Marker, **vor** dem Fett-Span — die reale Bullet-Form bleibt unangetastet
  (M1):

  ```
  - <a id="lh-fa-io-002"></a>**LH-FA-IO-002 — IFC-Export.**
  ```

  Im Preview unsichtbar; **kein Tabellen-/Struktur-Umbau** (deckt die frühere
  Vorgabe „Tabellen nicht umbauen" — hier auf Bullet-Ebene ohne Form-Verlust).
- **Anker-Wert = ID kleingeschrieben** (`lh-fa-io-002`): deterministisch aus der ID
  berechenbar, case-konsistent mit der Heading-Slug-Konvention (v0.9.0 vergleicht
  HTML-Anker **wörtlich**).
- **idlink lernt HTML-Anker (H3-fest):** `build_map` scannt die **Definitionsdateien**
  (fence-aware) nach `<a id="…">`/`<a name="…">`; ein Anker-Wert, dessen `.upper()`
  eine Familien-ID ist, wird als **präziser Anker** dieser ID gesetzt — **höchste
  Präzedenz** (expliziter Anker > eigenes Heading > Kapitel). Dieser Scan ist die
  **eigenständige Quelle der Wahrheit** und damit unabhängig davon, dass der
  bisherige `sect`-Definitions-Regex die Zeile nach dem vorangestellten
  `<a …></a>` **nicht mehr** matcht; der `sect`-Regex wird zusätzlich um ein
  optionales führendes `<a …></a>` gehärtet (Regression-Schutz).
- **Bestehende Links normalisieren (self-healing):** idlink repointet existierende
  `[<ID>](…)`-Links (Linktext == **genau eine** Familien-ID, **kein Bild-Link**,
  nicht `inTarget`) auf das **kanonische** `link_for`-Ziel, wenn es abweicht —
  idempotent (schreibt nur bei `ziel != kanonisch`). **Erwarteter Diff
  selbst-verifiziert (H1/H2):** **6** Sub-ID-Cross-File-Links (→ Per-ID-Anker)
  **+ 1** vorbestehender datei-only-Link in `AGENTS.md:196` ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) → präziser
  `#mr-009…`-Anker, incidenteller Self-Heal). Die im Review zunächst genannten 5
  Über-Reichweiten waren **bereits kanonisch** (4 schon mit präzisem Anker) — real
  ist es **1**; alle übrigen ~500 ID-Links bleiben unberührt.
- **Scope:** nur `lastenheft.md` erhält Anker (alle 17 dort). Korpus-Repoint über
  idlink; `done-archive/` bleibt via `corpus()`-Filter ausgenommen — eingefrorene
  Pläne behalten ihren Kapitel-Anker (akzeptiert, Referenz-Integrität slice-018a).
  `done/` ist **in-scope** (daher der [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Self-Heal-Pfad bewusst eingerechnet).

## 3. Definition of Done

- [x] **17 `<a id>`-Anker** in `lastenheft.md` an den Sub-ID-Bullets (D3/IO/ROM/WAL,
      Z. 145-146/198-201/562-565/707-713); Preview-unsichtbar; außerhalb Code/Fence.
- [x] **`idlink.build_map`** liest `<a id/name>` (fence-aware) und bevorzugt den
      präzisen Anker; `sect`-Regex gegen führendes `<a …></a>` gehärtet.
- [x] **idlink Link-Normalisierung:** existierende ID-Links auf kanonisches
      Ziel/Anker gezogen. **Soll-Diff: 7 Repoints** (6 Sub-ID + 1 [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)), vor Apply
      per `--dry-run`/`git diff` bestätigt; kein weiterer Link berührt.
- [x] **`anchors`-Modul grün:** alle 6 Sub-ID-Fragmente lösen auf die neuen
      HTML-Anker (v0.9.0); kein `anchor-missing`.
- [x] **`make gates` grün** (docs-check 0 Befunde, alle Module); idlink **idempotent**
      (2× Re-Run = 0). arch-check/lint/test/coverage unberührt.
- [x] **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)); Closure-Notiz mit Lerneintrag.

## 4. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | 17 `<a id>`-Anker an Sub-ID-Bullets |
| `tools/{idlink.py}` | ändern | HTML-Anker lernen (+`sect`-Härtung) + Link-Normalisierung |
| `docs/plan/adr/{0002,0007,0009,README}`, `spec/architecture.md`, `AGENTS.md` | ändern (generiert) | 7 Repoints |
| `docs/reviews/{2026-06-15-slice-018c-plan.md}` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |
| `CHANGELOG.md` | ändern | Eintrag (backticked) |

## 5. Trigger

- slice-018b done; LOW-3 als Folge-Punkt notiert; d-check **v0.9.0 gepinnt** →
  Inline-HTML-Anker verfügbar; Projektinhaber-Freigabe (slice-018c statt Direkt-Commit).

## 6. Closure-Trigger

- DoD vollständig, `make gates` grün. **Stehende Konvention danach:** Sub-IDs ohne
  eigenes Heading erhalten bei Bedarf ein `<a id="<id>">`; idlink bevorzugt es
  automatisch und hält Alt-Links kanonisch (Normalisierung).

## 7. Risiken und offene Punkte

- **GitHub-`user-content-`-Präfix (geprüft, retired):** GitHub präfixt `id` im DOM
  mit `user-content-`, matcht `#frag` aber gegen den entpräfixten Wert → Navigation
  funktioniert. (Eigen-Wissen; nicht slice-022 zugeschrieben — M2.)
- **Wörtlicher (case-sensitiver) Vergleich:** Anker-Wert == Fragment, exakt — beide
  all-lowercase, aus derselben `id.lower()`-Quelle. (Review: `#Übersicht` ≠
  `id="übersicht"` im d-check-Test — Mitigation nötig und erfüllt.)
- **`sect`-Regex-Kollision (H3):** der vorangestellte `<a …></a>` bricht den alten
  Definitions-Detektor. Behandelt: HTML-Scan als eigenständige Quelle **plus**
  `sect`-Regex-Härtung — sonst Fallback auf ankerlosen Datei-Link (Regression ggü. 018b).
- **Link-Normalisierung Reichweite:** self-healing über den ganzen Live-Korpus; der
  Soll-Diff (7) ist enumeriert + vor Apply via `--dry-run`/`git diff` zu bestätigen.
  Bild-Links `![..](..)` explizit ausgeschlossen, nur `tgt != canon` schreibt (idempotent).
- **Anker in Code/Fence zählt nicht** (v0.9.0): vor `**LH-FA-…**` (Fett, kein Code)
  platzieren. Negativ-Probe: Anker im Code → `anchor-missing`.
- **`spans`/andere Module** könnten rohe `<a>`-Tags beanstanden. Mitigation:
  `make gates` als Orakel.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Harness/Doku-Gates (`spec/lastenheft.md`-Anker, `tools/idlink.py`)

- **Modus:** GF. Reine Referenz-/Anker-Form, **non-normativ** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) n/a — keine
  AK-/Inhalts-Änderung); Diff maschinell generiert (idlink) + d-check-orakel-geprüft.
  Risiko niedrig (17 Anker + 7 Repoints, alle gate-validiert, Soll-Diff enumeriert).

## 9. Closure-Notiz

**Closure 2026-06-15.** `make gates` grün; `docs-check` **0 Befunde** (95 Dateien,
alle Module); Tests 122/122. MR-006-Plan-Review (3 HIGH + 2 MED) **vor**
Implementierung eingearbeitet.

**Geliefert:**
- **17 Inline-HTML-Anker** (`<a id="lh-fa-…">`) an den Bullet-Sub-IDs in
  `lastenheft.md` (D3/IO/ROM/WAL) — Preview-unsichtbar, von d-check v0.9.0 als
  `AnchorSet` erkannt.
- **`tools/idlink.py`** erweitert: (a) `build_map` liest Inline-HTML-Anker
  (`<a id/name>`, fence-aware) und bevorzugt sie als Per-ID-Anker (höchste
  Präzedenz); `sect`-Regex gegen führendes `<a …></a>` gehärtet (H3). (b)
  **Link-Normalisierung**: bestehende `[<ID>](…)`-Links werden auf das kanonische
  Ziel gezogen (idempotent, self-healing).
- **7 Repoints** (selbst-verifizierter Soll-Diff): 6 Sub-ID-Cross-File-Links
  (adr/0002·0007·0009·README, architecture×2) auf den Per-ID-Anker + 1
  incidenteller Self-Heal (`AGENTS.md:196`, MR-009 datei-only → präzis).

**Negativ-Beleg (DoD):** ohne die Anker meldet `docs-check` exakt **6
anchor-missing** (io-002/003/005×2, d3-004, rom-002); mit Anker 0 — beweist, dass
die Präzision an den HTML-Ankern hängt.

**Lerneintrag:** Bracket-in-Inline-Code in der Plan-Prosa (Code-Span mit `[…]`-
Klammer + Familien-ID) ließ idlinks Code-Span-Wrapping einen **verschachtelten
Link** erzeugen → Nicht-Idempotenz. Behoben durch Prosa-Bereinigung (kein `[…]`,
keine nackte Familien-ID in Inline-Code); Normalisierung danach idempotent (2× = 0).
*(Verallgemeinerung: verschachtelte/mehrzeilige Markdown-Konstrukte bleiben idlinks
Schwachstelle — vgl. 018b MED-1.)*

**Stehende Konvention:** Sub-IDs ohne eigenes Heading bekommen bei Bedarf ein
`<a id="<id>">`; idlink bevorzugt den expliziten Anker und hält Alt-Links kanonisch.
