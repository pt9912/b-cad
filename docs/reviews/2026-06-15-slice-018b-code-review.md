# Code-Review — slice-018b (Voll-Korpus-`ids`, Implementierung)

**Datum:** 2026-06-15. **Reviewer:** unabhängiger Agent (kein Autoren-Kontext),
verifiziert gegen reale Quellen (Read/Grep/Bash, d-check-Source im Schwester-Repo,
read-only Probe-Läufe). **Gegenstand:** Implementierung von
[slice-018b](../plan/planning/done-archive/slice-018b-voll-korpus-ids.md) —
`tools/idlink.py`, `.d-check.yml`, generierter Diff über ~53 Doku-Dateien
(`git diff 5fd01c5..HEAD`). Baum am Ende sauber.

## Verdikt: HIGH-Findings vorhanden: nein (0 HIGH) — 1 MEDIUM + LOW, **alle eingearbeitet**

`make docs-check` grün (92 Dateien, 0 Befunde, selbst ausgeführt). Mechanik
(Slugify-Port, Cross-Stratum, Family-Target-Filter, Anker-Auflösung) korrekt; der
ausgelieferte Diff ist semantisch sauber.

## MEDIUM (eingearbeitet)

- **MED-1 — Skript nicht idempotent (CHANGELOG:67, mehrzeiliger Code-Span).** Der
  Inline-Code-Span `make versions` läuft über CHANGELOG:66→67; auf der isolierten
  Zeile 67 ist die Backtick-Zahl ungerade, die zeilenbasierte Span-Erkennung paart
  verschoben → die `ADR-0004`-ID gilt als nackt → Re-Run backtickt sie erneut
  (Doppel-Backtick-Artefakt).
  Der ausgelieferte Stand war korrekt (manuell gefixt), aber ein künftiger Re-Apply
  hätte erneut korrumpiert; das Gate fängt es nicht (CHANGELOG ist `exempt-paths`).
  **Eingearbeitet:** `tools/idlink.py` Inline-Code-Erkennung auf **dokument-weit**
  (Carry-over über Zeilengrenzen) umgestellt → idempotent verifiziert (2× Re-Run =
  0 Änderungen). Zusätzlich **d-check-Pin auf v0.9.0** (dokument-weites Stripping;
  Wurzel der Klasse).

## LOW (eingearbeitet / akzeptiert)

- **LOW-1 — toter Code** (`LOG_RE`/`is_log_file`, ungenutzt + fehlerhafte Präzedenz).
  **Eingearbeitet:** entfernt.
- **LOW-2 — Closure-Notiz „8 Module"** statt 7 (`.d-check.yml` führt 7 Module).
  **Eingearbeitet:** korrigiert.
- **LOW-3 — Geschwister-Heading-Anker bei Bullet-Sub-IDs** (17 Fälle, z.B.
  `LH-FA-WAL-004/005` → `#lh-fa-wal-003-…`): die Definition liegt physisch in der
  Sektion (Weg-B-Kapitel-Semantik), der Anker zeigt aber auf das Heading eines
  Geschwister-Requirements. Gate-gültig, nur Präzisions-Imprecision. **Akzeptiert**
  (Weg B); mit dem v0.9.0-HTML-Anker-Support künftig optional auflösbar.
- **LOW-4 — `LH-QA-007`-Datei-Fallback** (undefinierte ID → Link auf `lastenheft.md`
  ohne Anker, 6×). Gate-gültig, semantisch „ins Leere". **Akzeptiert** als
  Folge-Punkt (Spec-Lücke; slice-006-Kontext).

## INFO (Negativbefunde / verifiziert ohne Befund)

- **Slugify-Port korrekt:** `tools/idlink.py` 1:1 zu d-check `anchors.go` (Bracket-
  Matching, `` ` ``/`*`-Strip, `_` bleibt, Space→`-`, Duplikat-Suffix `-N`). Alle
  verankerten IDs lösen auf reale Heading-Slugs (eigener `HeadingSlugs`-Nachbau → 0
  anchor-missing), konsistent mit grünem `anchors`-Modul.
- **Cross-Stratum korrekt:** `E-*` in `lastenheft.md`/`architecture.md` → `spezifikation.md`;
  `OBJ`/`ACC` aus Fundstellen → `lastenheft.md`; Self-Target (`E-VAL-001` in
  `spezifikation.md`) korrekt `inTarget`-frei.
- **Definitions-Anker, nicht Referenz:** `OBJ`→§3, `ACC`→§7, `REQ-TEC`→§9, `LH-FA-WAL-006`→eigenes `####` — keine ID auf einer reinen Referenz-Sektion.
- **Mehrzeilen-Mangling systematisch:** nur CHANGELOG:67 (MED-1); alle anderen Zeilen
  mit ungerader Backtick-Zahl korrekt; keine fälschliche Verlinkung von `EVL-001..006`
  (matcht `LH-FA-`-Regex korrekt nicht).
- **Keine fälschlich verlinkten Schema-Beispiele:** conventions.md `MR-002`-Tabelle —
  Real-IDs verlinkt, Platzhalter (`LH-FA-<BEREICH>`) bleiben nackt, `MR-002` Self-Target.
- **Apply-Robustheit:** rechts→links + Überlappungs-Guard korrekt (Mehr-ID-Zeilen,
  gemischt Code+nackt, Bereiche).
- **DoD §3:** alle Haken berechtigt (Gate grün belegt).

## Fazit

**slice-018b ist closure-reif — kein HIGH.** Die Mechanik ist korrekt und vom Gate als
Orakel bestätigt. MED-1 (Idempotenz) war die einzige reproduzierbare Schwäche —
behoben (dokument-weite Erkennung + v0.9.0-Pin). LOW-1/2 eingearbeitet, LOW-3/4
akzeptiert + als Folge-Punkte notiert.
