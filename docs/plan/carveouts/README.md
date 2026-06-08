# Carveouts — b-cad

Dokumentierte Ausnahmen von einem Gate oder einer Architekturregel, jede
mit Auflösungs-Trigger und Folge-Slice. Aufgelöste Carveouts wandern
nach `done/` (reiner `git mv`).

## Aktive Carveouts

**Keine.** b-cad ist soeben durch den Greenfield-Bootstrap gegangen — es
gibt noch keine Gates und keinen Code, also auch keine zu schwächende
Regel. Ein Carveout ohne reales Gate wäre sinnlos.

Sobald Gates existieren (erster Code-Slice) und ein Gate strukturell rot
ist (z. B. Bootstrap-Coverage unter Zielschwelle), entsteht hier ein
`CO-<NNN>`-Eintrag mit Trigger und Folge-Slice.

## Konventionen

- Jeder aktive Carveout braucht: betroffenes Gate, Trigger, Folge-Slice,
  letzten Prüf-Termin.
- Bei Welle-Closure: Carveout-Audit zwingend — welche sind weiterhin
  gültig, welche aufgelöst?
- Siehe Kurs-Modul 7 (Carveouts).
