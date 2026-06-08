# Carveouts — b-cad

Dokumentierte Ausnahmen von einem Gate oder einer Architekturregel, jede
mit Auflösungs-Trigger und Folge-Slice. Aufgelöste Carveouts wandern
nach `done/` (reiner `git mv`).

## Aktive Carveouts

**Keine.** Gates und Code existieren inzwischen (slice-001/002), aber
**kein Gate ist strukturell rot** — es gibt also keine zu schwächende
Regel. Ein Carveout ohne rotes Gate wäre sinnlos.

Sobald ein Gate strukturell rot ist (z. B. Coverage unter Zielschwelle
nach einem Schwellen-Hochschalt), entsteht hier ein
`CO-<NNN>`-Eintrag mit Trigger und Folge-Slice.

## Konventionen

- Jeder aktive Carveout braucht: betroffenes Gate, Trigger, Folge-Slice,
  letzten Prüf-Termin.
- Bei Welle-Closure: Carveout-Audit zwingend — welche sind weiterhin
  gültig, welche aufgelöst?
- Siehe Kurs-Modul 7 (Carveouts).
