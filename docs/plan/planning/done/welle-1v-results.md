---
id: welle-1v-results
titel: Welle-Closure — Ergebnisnotiz welle-1v-viewer
status: done
welle: welle-1v-viewer
---

# Welle-1v-Closure — Ergebnisnotiz `welle-1v-viewer`

**Welle:** welle-1v-viewer. **Zeitraum:** 2026-06-12 bis 2026-06-13.
**Autor:** Dietmar Burkard. **Closure-Datum:** 2026-06-13.

**Welle-Ziel (Roadmap):** Die *sichtbare* Hälfte des Echtzeit-Vertrags
— ein Qt-6-3D-Viewer (Driving Adapter) stellt das extrudierte
Gebäudemodell dar und folgt committeten Änderungen über den
[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)-Vertrag. Erfüllt **[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)** („Das Gebäude wird automatisch
als 3D-Modell dargestellt") und die sichtbare Hälfte von
**[LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)** (Scope-Herkunft: slice-010a, Roadmap-Drift-Tabelle
2026-06-11). In der Welle hinzugekommen: **slice-012**
(Eckenschluss endpunkt-verbundener Wände, [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)-Teilumfang) —
ausgelöst durch den Abnahme-Befund am [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg.

---

## 1. Closure-Kriterien (beobachtbar)

- **Alle Closure-Trigger der Roadmap erfüllt:** drei Slices liegen in
  `done/` (Liste §2), jeder mit eigener Closure-Notiz und Lerneintrag.
  Der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg ist **vom Projektinhaber abgenommen** (Runde 2,
  2026-06-12 — Abnahme-Satz in
  [`acc-002-beleg.md`](acc-002-beleg.md)). Diese Notiz schließt den
  letzten Trigger („Closure-Notiz in `done/welle-1v-results.md` inkl.
  zwingendem Carveout-Audit").
- **`make gates` grün** (frischer Lauf 2026-06-13 am aktuellen HEAD
  `8baf2d1`, Exit 0): docs-check 0 ERROR/WARN, gate-consistency ok
  (inkl. des neuen Nicht-Gate-Targets `make run`), arch-check
  Regeln A–E (hexagonale Schichtung + Qt-Grenze [ADR-0009](../../adr/0009-gui-framework-qt6.md)),
  lint 0 Befunde + suppression-gate, **Tests 63/63** (100 %),
  **Coverage 94,2 % lines** (846/898) / **100 % functions** (120/120),
  Schwelle 70 %. Der Lauf ist frisch reproduziert, nicht aus den
  Slice-Läufen übernommen (Honesty-Disziplin) — seit dem
  slice-012-Lauf (`8fe8dad`) änderte sich nur Doku, Belege,
  Slice-Moves und das neue Nicht-Gate-Target `make run`; kein
  `src/`-Code, kein Test, keine Gate-Logik in `tools/`.
- **Unabhängige Welle-Verifikation gelaufen** (Reviewer ≠ Autor, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-
  Disziplin; §3): Welle abschlussreif, keine HIGH/MEDIUM, ein LOW
  (terminologisch, nicht-blockierend).

## 2. Gelieferter Umfang (Slices in `done/`)

| Slice | Ergebnis |
|---|---|
| slice-011a | [ADR-0009](../../adr/0009-gui-framework-qt6.md) „GUI-Framework-Bindung Qt 6 (Driving Adapter)" accepted (Pflichten a–f) + Spec-Operationalisierung „sichtbar" |
| slice-011b | Sichtbarer 3D-Viewer: Qt-6-Widgets-Adapter, Tessellation über `ViewModelPort` (kein OCC in der GUI), Szenen-Surrogat + display-freie AK-Tests, arch-check **Regel E** (Qt nur `adapters/ui/` + `main.cpp`), `make acc-002-beleg` (manueller Abnahme-Schritt, kein Gate); **[ADR-0010](../../adr/0010-headless-gl-xvfb.md)** (Headless-GL via Xvfb/llvmpipe) entstand als Implementierungs-Befund |
| slice-012 | Eckenschluss endpunkt-verbundener Wände ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)-Teilumfang): `model::Footprint` + Footprint-Hoheit im Kern (Grad-2-Miter mit `WALL_MITER_LIMIT`, total), `GeometryKernelPort` auf `extrudeFootprint`/`tessellateFootprint`, Mehr-Element-Update `WallGeometryChanged` (transaktional), 8 WAL-006-AK-Tests; [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg ohne offene Außenecken regeneriert |

**Bezug zu welle-1:** welle-1-mvp lieferte den Kern-Vertrag
(Projekt/Geschosse/Wände/Raumerkennung/OCC-Extrusion +
Echtzeit-Benachrichtigung als Kern). welle-1v-viewer liefert die
*sichtbare* Hälfte und schließt damit **[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)**, das in welle-1
bewusst offen blieb
([`welle-1-results.md` §2](welle-1-results.md)).

**Nicht Teil der Welle (bewusst):** slice-006
(Drittanbieter-Attribution) war nie Closure-Trigger und bleibt in
`open/` (Vorbedingungen siehe Slice). WAL-006-**Vollumfang**
(Schnittpunkte/T-Stöße als Knoten) bleibt ausdrücklich offen — nur der
Eckenschluss-Teilumfang wurde geschärft und geliefert.

## 3. Review & Verifikation vor Closure

Unabhängige Welle-Verifikation (Reviewer ≠ Autor, eigener Agent ohne
Autoren-Kontext) gegen DoD/Spec, vor dieser Closure durchgeführt
(Kurs-Modul 11: „Hat das Gebaute Plan/DoD/Spec umgesetzt?"). Geprüft:
alle drei Closure-Trigger-Slices real in `done/`; [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien) + sichtbare
Hälfte [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung) erfüllt; [ADR-0009](../../adr/0009-gui-framework-qt6.md)/0010-Vertrag deckungsgleich mit
dem Code (kein OCC-/Qt-Leck aus `adapters/ui/`; `ViewModelPort` real;
arch-check Regel E real in `tools/arch-check.sh`); jede DoD-Zeile durch
reale Artefakte gedeckt (AK-Tests mit `LH-`-ID im Namen, Beleg-PNG +
Abnahme-Satz, Footprint im Kern, `WallGeometryChanged`); Spec-Grenze
sauber (WAL-006 nur als Teilumfang geschärft, Vollumfang nirgends
behauptet); Slices referenzieren nur **aktive** ADRs.

**Ergebnis:** Welle abschlussreif, **keine HIGH, keine MEDIUM**.

- **LOW-1 (terminologisch, nicht-blockierend):** [ADR-0010](../../adr/0010-headless-gl-xvfb.md) trägt im
  Titel/Bezug „**präzisiert** [ADR-0009](../../adr/0009-gui-framework-qt6.md) (f)" statt der von
  [`AGENTS.md` §2.5](../../../../AGENTS.md) für Korrekturen genannten
  `Supersedes`-Form. Bewertung: [ADR-0010](../../adr/0010-headless-gl-xvfb.md) ändert [ADR-0009](../../adr/0009-gui-framework-qt6.md) **nicht in
  der Substanz** (nur das Mechanik-Detail offscreen-QPA → Xvfb/llvmpipe);
  [ADR-0009](../../adr/0009-gui-framework-qt6.md) bleibt `Accepted`/aktiv und reserviert „Supersedes-ADR"
  ausdrücklich für künftige Substanz-Revisionen
  (Re-Evaluierungs-Trigger, [ADR-0009](../../adr/0009-gui-framework-qt6.md)).
  Die Beziehung ist im ADR-Index dokumentiert
  ([`adr/README.md`](../../adr/README.md)). Kein verschleiertes Detail
  → als Steering-Kandidat geführt (§5), nicht als Defekt.

## 4. Carveout-Audit (zwingend bei Welle-Closure)

Geprüft 2026-06-13 (unabhängig in §3 bestätigt):
[`docs/plan/carveouts/README.md`](../../carveouts/README.md) führt
**keine aktiven Carveouts**; ein `done/`-Unterordner existiert nicht
(keine aufgelösten). Kein Gate war während der Welle strukturell rot,
keine Architekturregel oder Schwelle wurde geschwächt — im Gegenteil
wuchs der arch-check um die reale **Regel E** (Qt-Grenze). Nichts
aufzulösen, nichts nachzutragen.

| Carveout | Status vorher | Status nachher | Aktion |
|---|---|---|---|
| — | keine aktiven | keine aktiven | — (negativer Befund, korrekt belegt) |

## 5. Lerneinträge / Steering-Loop-Zähler

Fortschreibung der Zähler aus
[`welle-1-results.md` §5](welle-1-results.md) plus die in dieser Welle
akkumulierten Praxis-Zähler (Kurs-Regel: 2× kategorisieren, 3× Regel):

1. **„Unabhängiges Plan-Review vor Implementierungs-Start": als
   Konvention festgeschrieben.** Der in welle-1 bei 2× stehende Zähler
   (welle-1-results §5 kannte slice-011 zeitlich noch nicht) erreichte
   beim 3. Vorkommen (slice-011, Review 2026-06-12: 3 HIGH)
   die Regel-Schwelle und wurde als
   [`MR-006`](../../../../harness/conventions.md) (+ AGENTS §5
   Workflow-Schritt 5) festgeschrieben. In dieser Welle bereits
   wirksam: slice-011b (R1/W2-Findings) und slice-012 (W3: 2 HIGH vor
   Start behoben). **Zähler geschlossen → Konvention.**
2. **„Lösung schärft nie das Lastenheft": 3× — Regel-Kandidat.**
   slice-012 schärfte [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) ausdrücklich nur über die
   Reifephase-Klausel (Outline → Teilumfang-AK, lösungsfrei) und hielt
   den Vollumfang offen; die Implementierungs-Festlegungen (Begrenzung,
   Rückfall) leben in Spec §1/§3, nicht im Lastenheft. Mit Review-009
   M4 und Review-010 F1/F2 ist das das **3. Vorkommen** — reif zur
   Festschreibung (AGENTS §2 oder `MR-<NNN>`), nächste Welle.
3. **„Der Abnahme-Schritt ist ein Sensor": 1× kategorisiert** (neuer
   Zähler, slice-012-Lerneintrag). Der [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg (bewusst **kein**
   Gate, [ADR-0009](../../adr/0009-gui-framework-qt6.md) (f)) fand die offenen Außenecken, die in keiner
   AK-Test-Lücke standen — der erste echte Fund des manuellen
   Abnahme-Schritts. Bestätigt die R1-H3-Korrektur (Beleg ernsthaft
   konstruieren).
4. **„Umgebungsabhängige Mechanik-Details vor ADR-Accept mit
   Minimal-Probe belegen": 1× kategorisiert** (slice-011b-Lerneintrag).
   [ADR-0009](../../adr/0009-gui-framework-qt6.md) (f) traf eine faktisch falsche Plattform-Aussage
   (offscreen-QPA trägt kein GL), entdeckt erst beim echten Lauf →
   Präzisierung [ADR-0010](../../adr/0010-headless-gl-xvfb.md).
5. **„Quantitative AK vor Implementierungs-Start einmal von Hand
   durchrechnen": 1× kategorisiert** (slice-012-Lerneintrag). Der
   geplante Happy-AK „Summen-Volumen wächst" war geometrisch falsch
   (symmetrischer Eckschnitt erhält die Trapez-Fläche) — beim
   Durchrechnen vor der Implementierung entdeckt.
6. **„ADR-Beziehungstyp ‚präzisiert' vs. ‚Supersedes' für
   nicht-substanzielle Detailkorrekturen": 1× kategorisiert**
   (Verifier-LOW, §3). Beobachtung: AGENTS §2.5 kennt nur „Supersedes";
   [ADR-0010](../../adr/0010-headless-gl-xvfb.md) nutzt „präzisiert" für eine reine Mechanik-Detailkorrektur
   ohne Substanzänderung. Bei Wiederholung als bewusste Variante
   deklarieren oder auf „Supersedes" vereinheitlichen.
7. **„Post-Commit-Schritte sind total": weiterhin 2×** (unverändert
   ggü. welle-1; slice-012s totale Footprint-/Miter-Berechnung und
   transaktionale Mehr-Element-Garantie sind bereits durch [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)
   §Konsequenzen gedeckt, kein neues Vorkommen einer *neuen* Regel).

**Welle-Lerneintrag:** Der schärfste Fund dieser Welle lag **nicht** in
den automatischen Gates, sondern am benutzer-beobachtbaren
Abnahme-Artefakt (offene Außenecken am [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg). Das bestätigt
den welle-1-Lerneintrag aus der anderen Richtung: dort fehlte der
*strukturelle Normalfall* in der Boundary-AK; hier fehlte ein
*sichtbares Geometrie-Merkmal*, das kein analytischer Test als Lücke
zeigte. Konsequenz: ein ernsthaft konstruierter, abgenommener
Sicht-Beleg gehört für jede benutzer-sichtbare Anforderung zum
Closure-Pfad — er ist ein Sensor eigener Klasse, kein pro-forma-Artefakt.

## 6. Nachfolge

- **[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien) erfüllt** — der sichtbare 3D-Viewer ist geliefert und
  abgenommen; der in welle-1 offen gelassene Punkt ist geschlossen.
- **M1 (Meilenstein „Lauffähiges MVP")** bleibt erreicht (welle-1);
  der Viewer war per Drift-Entscheidung 2026-06-11 nicht Teil des
  M1-Triggers.
- Kandidaten für die nächste Welle: `welle-2-bauteile` (Türen/Fenster
  mit Wandöffnung, Treppen, Decken/Dach — Meilenstein M2), oder
  Zwischenschritt slice-006 (Drittanbieter-Attribution). Start ist eine
  Planungs-Entscheidung, kein Automatismus.
- **Offen aus dieser Welle:** WAL-006-Vollumfang (Schnittpunkte/T-Stöße
  als Knoten); LH-FA-EVL-*-Auswertungen müssen auf der Footprint-Fläche
  (Shoelace) aufsetzen, nicht auf Länge·Stärke (Spec §1-Hinweis).
</content>
</invoke>
