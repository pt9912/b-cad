# Code-Review slice-015c — Decken/Fundament-Persistenz

**Datum:** 2026-06-14. **Reviewer:** unabhängiger Agent (Reviewer ≠ Autor),
Modul 11. **Gegenstand:** uncommitteter Diff von slice-015c
([Plan](../plan/planning/done/slice-015c-decken-fundament-persistenz.md)).

## Verdikt: HIGH-Findings vorhanden: nein — Closure hält

Die Hochrisiko-Pfade (verschachtelter `polygon_json`-Parser, `%.17g`-Puffer,
`stod`-Totalität, FK-/Transaktions-Reihenfolge, Mapper-Totalität, Feld-
Vollständigkeit) wurden isoliert nachgebaut und Zeichen für Zeichen
durchgespielt. Die in 013b/014b/015b durch grüne Gates gerutschte Klasse von
HIGH-Korrektheitsfehlern liegt hier **nicht** vor. Es blieben drei MEDIUM
(Robustheit/Test) und zwei LOW — alle drei MEDIUM **eingearbeitet**.

## Findings + Auflösung

### MED-1 — `std::stod` parst Müll-Präfixe still (Totalitäts-Lücke) → **gefixt**
`parseRing` nutzte `std::stod(token)`, das nur den führenden numerischen Präfix
liest und den Rest **ohne Fehler** verwirft (`"1.5x"` → `1.5`, `"0x10"` → `16`).
Der Code-Kommentar und das CHANGELOG behaupten Totalität (`E-IO` bei jedem
Format-Fehler) — gegenüber Fremd-/teilkorruptem DB-Inhalt war das nicht erfüllt.
**Fix:** `std::stod(token, &consumed)` + Prüfung `consumed == token.size()`
(sonst `E-IO`); leeres Token wirft jetzt ebenfalls (LOW-1 mit erledigt). Die
behauptete Totalität ist nun real.

### MED-2 — kein Negativ-Parse-Test für malformed `polygon_json` → **gefixt**
Die fünf Wurf-Pfade (ungerade Wertzahl, unbalanciert, kein Grundriss-Ring,
`stod`-Fehler, unbekannter `slab_type`) waren durch keinen Test gedeckt — genau
die Stelle, an der grüne Gates zuvor versagten. **Fix:** neuer Test
`SqliteProjectRepository_LH_FA_SLB_FND.MalformedSpaltenWerfenNeutral`
korrumpiert die gespeicherte `.bcad` white-box per rohem `UPDATE` (zulässig:
`lint`/clang-tidy und arch-check Regel D greppen nur `src/`, nicht `tests/`) und
prüft `EXPECT_THROW(load, std::runtime_error)` für vier Fälle (a) ungerade
Wertzahl, (b) unbalanciert, (c) Müll-Suffix, (d) unbekannter `slab_type`.

### MED-3 — `SlabType::Bodenplatte` mapper-Zweig ungetestet → **gefixt**
Der Round-Trip-AK fuhr nur `Decke`/`Fundament`; der dritte Enum-Wert blieb
ungeprüft (ein vertauschtes Text-Literal bliebe grün). **Fix:** dritte Platte
`SlabType::Bodenplatte` im AK ergänzt + Typ-Assert — alle drei Mapper-Zweige
round-trip-belegt.

### LOW-1 — `parseRing` schluckte leere Tokens still → **mit MED-1 gefixt**
`if (token.empty()) continue;` maskierte Komma-Korruption (`[[,1,2,3,4]]`).
Jetzt wirft ein leeres Token (strenger, konsistent mit dem deterministischen
Format).

### LOW-2 — Lade-Reihenfolge-Kommentar leicht missverständlich → belassen
Kosmetisch; der Kommentar stellt „kein FK auf load" bereits klar.

## Geprüft & korrekt (kein Handlungsbedarf)

- **`%.17g`-Puffer** `std::array<char,64>`: Worst-Case 24 Bytes
  (`-1.7976931348623157e+308`) — kein Überlauf.
- **Balancierter `[...]`-Scan**: 1/2/3 Ringe, leere cutout-Liste `[[...]]`,
  leerer Ring `[[]]`, `[]`, `""`, Tiefe-3, Extra-`[`/`]` — alle Fehlpfade
  werfen `E-IO`, kein Crash/UB.
- **Round-Trip-Symmetrie** Serialisierer ↔ Parser inkl. leerem Ring und `-0.0`.
- **FK-/Transaktions-Reihenfolge** `insertSlabs` nach `insertStoreys`, in
  derselben `BEGIN`/`COMMIT`; atomar.
- **Mapper-Totalität** alle 3 Enum-Werte + defensiver Wurf; `textToSlabType`
  wirft bei Unbekanntem (wichtig, da `slabs.slab_type` keine Schema-CHECK hat).
- **Feld-Vollständigkeit** alle 6 `Slab`-Felder round-trippen; `material_id`
  NULL, `base_z` korrekt nicht persistiert.

## Ergebnis nach Einarbeitung

`make gates` grün: **Tests 105/105** (zuvor 104, +Negativ-Parse), Coverage
**91,9 %** (von 91,4 %, Parser-Wurf-Pfade jetzt gedeckt). Keine offenen
HIGH/MEDIUM.
