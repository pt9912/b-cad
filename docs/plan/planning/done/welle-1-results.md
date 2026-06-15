---
id: welle-1-results
titel: Welle-Closure — Ergebnisnotiz welle-1-mvp
status: done
welle: welle-1-mvp
---

# Welle-1-Closure — Ergebnisnotiz `welle-1-mvp`

**Welle:** welle-1-mvp. **Zeitraum:** 2026-06-08 bis 2026-06-12.
**Autor:** Dietmar Burkard. **Closure-Datum:** 2026-06-12.

**Welle-Ziel (Roadmap):** Ein lauffähiges b-cad, mit dem sich ein
Einfamilienhaus im Sinne von [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien) in Grundzügen erstellen lässt —
Projekt anlegen/speichern/laden, Geschosse, Wände, automatische
Raumerkennung, 3D-Extrusion in Echtzeit **als Kern-Vertrag**. Die
*sichtbare* 3D-Darstellung ([ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)) liefert `welle-1v-viewer`
(Scope-Entscheidung slice-010a, Roadmap-Drift-Tabelle 2026-06-11).

---

## 1. Closure-Kriterien (beobachtbar)

- **Alle Closure-Trigger der Roadmap erfüllt:** 13 Slices + spike-001
  liegen in `done/`, jeder mit eigener Closure-Notiz und Lerneintrag
  (Liste in §2). Diese Notiz schließt den letzten Trigger
  („Closure-Notiz in `done/welle-1-results.md`").
- **`make gates` grün** (Lauf 2026-06-12): docs-check 0 ERROR/WARN,
  gate-consistency, arch-check, clang-tidy 0 Befunde +
  suppression-gate, Tests grün, Coverage 93,8 % lines / 100 %
  functions (Schwelle 70 %).
- **Unabhängiges Code-Review + Welle-Verifikation gelaufen, alle
  Findings behoben** (§3, Commit `330d5d0`).

## 2. Gelieferter Umfang (Slices in `done/`)

| Slice | Ergebnis |
|---|---|
| spike-001 | Toolchain-Reproduzierbarkeit erprobt |
| slice-001 | Build-Skelett & DevContainer (`make build` grün) |
| slice-002 | Code-Gates real & gepromotet (`make gates` grün) |
| slice-003a | Domain-Kern & Wände mit Grenzwert-Verhalten ([LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)/002/003, [LH-FA-BLD-001](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen), [LH-FA-FLR-001](../../../../spec/lastenheft.md#modul-geschosse-flr)), OCC-frei |
| slice-003b | OCC-Extrusion ([LH-FA-D3-001](../../../../spec/lastenheft.md#modul-3d-modellierung-d3)) hinter `GeometryKernelPort` + arch-check Regel C |
| slice-004 | Gepinnte Toolchain 26.04/node24, Digest+Snapshot ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)) |
| slice-005 | Gate-Consistency-Sensor |
| slice-007 | Datenmodell-Definition (`spec/data-model.yaml`) + [ADR-0006](../../adr/0006-relationales-schema-design.md) |
| slice-008a | Persistenz speichern/laden, atomar via Temp+Rename ([LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/003, [ADR-0003](../../adr/0003-persistenz-sqlite.md)) |
| slice-008b | Crash-Recovery (`kill -9`, [LH-QA-005](../../../../spec/lastenheft.md#lh-qa-005--crash-recovery)) + Fehlercodes [E-IO-001](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/002 |
| slice-009a | [ADR-0007](../../adr/0007-raumerkennung-geometrie-basis.md) accepted (Innenkante + Ring-Modell) + Spec-Schärfung Raumerkennung |
| slice-009b | Raum-Autoerkennung implementiert ([LH-FA-ROM-001](../../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen)), 5 AK-Tests |
| slice-010a | [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung) auf AK-Niveau + [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) accepted (Observer-Port) |
| slice-010b | Kern-Benachrichtigung implementiert ([LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)), 6 AK-Tests |

**Nicht Teil der Welle (bewusst):** sichtbarer 3D-Viewer →
`welle-1v-viewer` ([ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien) bleibt offen, bis er geliefert ist);
slice-006 (Drittanbieter-Attribution) war nie Closure-Trigger und
bleibt in `open/` (Vorbedingungen siehe Slice).

## 3. Review & Verifikation vor Closure

Unabhängiges Code-Review (Welle-1-Review) plus Welle-Verifikation
gegen die AK, vor dieser Closure durchgeführt; **alle Findings
behoben** in Commit `330d5d0` (2026-06-11):

- **M1 (Reichweite, wichtigster Fund):** Raumerkennung erkannte nur
  Grad-2-Zyklen — geteilte Wände (Grad-3-Knoten, Normalfall realer
  Grundrisse) erzeugten nicht beide Räume. Fix: planare
  Flächen-Traversierung über winkelsortierte Halbkanten; neuer
  AK-Test `GeteilteWandErzeugtBeideRaeume`.
- **M2:** Kollineare Nachbarkanten ungleicher Stärke als dokumentierte
  Welle-1-Näherung definiert und mit gepinntem Erwartungswert getestet.
- **M3, L1–L4 + Verifier-Befunde** ([LH-FA-ROM-001](../../../../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)):
  siehe Commit-Message `330d5d0`.

## 4. Carveout-Audit (zwingend bei Welle-Closure)

Geprüft 2026-06-12: **keine aktiven Carveouts**
(`docs/plan/carveouts/README.md`) — kein Gate war während der Welle
strukturell rot, keine Ausnahme wurde geschwächt. Nichts aufzulösen.

## 5. Lerneinträge / Steering-Loop-Zähler

Aus den Slice-Closures der Welle akkumulierte Praxis-Zähler
(Kurs-Regel: 2× kategorisieren, 3× Regel) — **Stand bei
Welle-Closure, hier festgeschrieben:**

1. **„Unabhängiges Plan-Review vor Implementierungs-Start": 2×
   bewährt** (slice-009: HIGH H1 → ADR-Scope + Split; slice-010:
   HIGH F1 → Lastenheft-Grenzverletzung abgefangen). Beim
   3. Vorkommen als Konvention festschreiben (AGENTS §5 oder
   `MR-<NNN>`).
2. **„Lösung schärft nie das Lastenheft": 2× geprobt** (Review-009
   M4, Review-010 F1/F2) — Konventions-Kandidat, in der
   010a-Closure erwähnt.
3. **„Post-Commit-Schritte sind total": 2×** — bereits im Repo
   dokumentiert ([ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) §Konsequenzen); vertagt auf 3. Vorkommen
   (Kandidaten: Autosave [LH-QA-004](../../../../spec/lastenheft.md#lh-qa-004--autosave), OTel [REQ-TEC-006](../../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec)).

**Welle-Lerneintrag (geschärfte Regel als Kandidat):** Der größte
Review-Fund der Welle (M1, §3) lag in der *Reichweite* einer
Implementierung (Normalfall-Topologie nicht abgedeckt), nicht in
ihrer Korrektheit auf den getesteten Fällen — AK-Tests decken nur
ab, was das Lastenheft als Beispiel nennt. Konsequenz für kommende
Wellen: Bei topologie-/geometrielastigen Anforderungen gehört der
*strukturelle Normalfall* (hier: geteilte Wände) explizit in die
Boundary-AK, nicht erst ins Review.

## 6. Nachfolge

- **M1 (Meilenstein „Lauffähiges MVP"): erreicht** — [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Kern
  erstellbar + `make gates` grün; Viewer ist per Drift-Entscheidung
  2026-06-11 nicht Teil des M1-Triggers.
- Kandidaten für die nächste Welle: `welle-1v-viewer` (braucht
  zuerst GUI-Grundsatz-ADR Qt 6), `welle-2-bauteile`, oder
  Zwischenschritt slice-006 (Attribution). Start ist eine
  Planungs-Entscheidung, kein Automatismus.
