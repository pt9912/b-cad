---
id: welle-3-results
titel: Welle-Closure — Ergebnisnotiz welle-3-auswertung
status: done
welle: welle-3-auswertung
---

# Welle-3-Closure — Ergebnisnotiz `welle-3-auswertung`

**Welle:** welle-3-auswertung. **Zeitraum:** 2026-06-14 bis 2026-06-16.
**Autor:** Dietmar Burkard. **Closure-Datum:** 2026-06-16.

**Welle-Ziel (Roadmap):** Das Gebäudemodell **auswertbar** machen —
**Material-System** (`MAT`) + **Auswertungen** (`EVL`): Flächen-, Volumen-,
Wohnflächenberechnung und Material-/Tür-/Fensterlisten + Kosten. Auswertung ist
eine **reine read-only-Ableitung aus dem committeten Modell** (Query, kein
Geometrie-Erzeugen) über den Driving-Port `EvaluatePort` und ein
Material-Domänenmodell. Erfüllt **Meilenstein M3** („Flächen/Volumen/
Materiallisten korrekt"). Architektur-Leitplanke: **[ADR-0012](../../adr/0012-evaluations-architektur.md)**
„Evaluations-Architektur" — read-only/pull, pure Werttypen, **analytisch im Kern**
(kein OCC-`GProp`, kein `Solid.volume_mm3`-Lesen). **Kein neuer Grundsatz-ADR pro
Auswertung** (Material ist [ADR-0006](../../adr/0006-relationales-schema-design.md)-/Spec-Sache).

---

## 1. Closure-Kriterien (beobachtbar)

- **Alle Closure-Trigger der Roadmap erfüllt:** sieben Slices liegen in
  `done-archive/` (Liste §2), jeder mit `status: done`, eigener Closure-Notiz
  und Lerneintrag. Diese Notiz schließt den letzten Trigger („unabhängige
  Welle-Verifikation + Closure-Notiz inkl. zwingendem Carveout-Audit").
- **`make gates` grün** (frischer, **unabhängig reproduzierter** Lauf
  2026-06-16, Exit 0): docs-check **0 Befunde** (106 Dateien), gate-consistency,
  arch-check **Regeln A–E** (Kern OCC-/Qt-/SQLite-frei), lint 0 +
  suppression-gate, **Tests 145/145** (100 %), **Coverage 92,7 % lines**
  (2111/2278) / **100 % functions**, Schwelle 70 %. Zusätzlich **`make
  schema-check` grün** (`schema.sql == d-migrate(data-model.yaml)`, **kein
  Drift** — die Material-Persistenz war rein adapter-seitig). Vom Verifier
  **selbst** gelaufen (Honesty-Disziplin), nicht aus den Slice-Läufen übernommen.
- **Unabhängige Welle-Verifikation gelaufen** (Reviewer ≠ Autor, eigener Agent
  ohne Autoren-Kontext; §3): **Welle closure-reif, keine HIGH.** Die eine
  nicht-blockierende LOW (spez. §1 Datenfluss-Satz nannte den `wall_type`-Fallback
  im Präsens, Zurückstellung 12 Zeilen später) wurde **vor** dieser Notiz mit
  einem Inline-Marker behoben.

## 2. Gelieferter Umfang (Slices in `done-archive/`)

| Strang | Slices | Ergebnis |
|---|---|---|
| **ADR-/Spec-Leitplanke** | slice-017a | **[ADR-0012](../../adr/0012-evaluations-architektur.md)** „Evaluations-Architektur" accepted (neuer `EvaluatePort` read-only/pull, pure Werttypen, **Netto-Volumen analytisch im Kern**, Material = konsumierte Eingabe); Lastenheft EVL/MAT Outline→AK (0.1.7), Spec §1/§2.1. Zwei unabhängige Reviews (Plan + ADR-Text), je 1 HIGH eingearbeitet |
| **EVL Flächen** | slice-017b | `EvaluatePort.floorArea`/`livingArea` (EVL-001/003) als read-only-Aggregation der Raum-Netto-Flächen (`model::AreaReport`); 122 Tests |
| **EVL Volumen** | slice-017c | `EvaluatePort.volume()` (EVL-002) — Netto-Material-Volumen **analytisch im Kern** (`volume_geometry`: Wand=Footprint·Höhe−geklemmte Öffnungen, Decke=(Fläche−Ausschnitte)·Dicke, Treppe=Σ Stufenkörper); **Dach zurückgestellt** (dicke-los). [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review: **0 HIGH** (Formeln per Hand bestätigt); kritischer Plan-Fang: kein `Solid.volume_mm3`-Lesen |
| **Material-Domäne** | slice-017d | `model::Material` + Verwaltung/Zuweisung über `EditStructurePort` + read-only `effectiveMaterial` über `EvaluatePort` (MAT-001/002/003/005). **Override-Auflösung**; `wall_type`-Fallback zurückgestellt. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start): **1 HIGH** (removeMaterial = `on_delete: restrict`-Treue statt stiller Verlust) eingearbeitet |
| **Material-Persistenz** | slice-017e | `materials` + `material_id`-Round-Trip im SQLite-Adapter, **NULL-sicher** (zentrale `*Optional*`-Helfer, kein „NULL→0"), ID-/FK-Erhalt, **kein Schema-Drift**. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) + **Code-Review (höhere Latte)** je 0 HIGH |
| **EVL Listen** | slice-017f | `materialList` (EVL-004: je Material Σ Netto-Volumen über Wand+Decke, Reuse `effectiveMaterial`+`volume_geometry`; **Dach ausgenommen**) + `doorList`/`windowList` (EVL-005/006). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start): 1 LOW (§1-Konsistenz: `roofs` aus der Aggregations-Menge) |
| **Kosten** | slice-017g | `MaterialLine.cost = Menge × cost_per_m3` + `MaterialReport.total_cost` (MAT-006); kein Kennwert → `nullopt` (≠ kostenlos); `cost_per_m2`-Lücke. Additiv (kein neuer Port) |

**Bezug zu welle-1/-2:** welle-1/-1v lieferten Kern + sichtbaren Viewer, welle-2
die vier Bauteil-Familien + Persistenz mit `material_id`-FKs. welle-3 macht das
Modell **auswertbar** — additiv über `EvaluatePort`, **ohne** Port-Signatur-
Migration der Bestands-Pfade, **ohne** Geometrie-Erzeugen (reine Query).

**Nicht Teil der Welle (bewusst, ehrlich benannt):**

- **Dach-Volumen** (dicke-loses Dachmodell → kein Bauteil-Solid) → nicht in
  EVL-002 **und** nicht in der EVL-004-Materialliste (umbauter Raum ≠ Material-
  Volumen). Re-Eval mit Dach-Dicken-/Material-Semantik.
- **`wall_type`-Template-Fallback** der Material-Auflösung — die Domäne trägt
  `Wall.type` nur als Enum {Innen/Aussen/Trag} ([LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen)), keine
  material-tragende Wandtyp-**Bibliothek**-Entität. Geliefert ist **Override-only**
  (eigenes `material_id`). Die schema-treue Lösung (`wall_types`-Bibliothek +
  `wall_type_id` + Persistenz) ist ein **eigener Strang** (welle-4+,
  Projektinhaber-Entscheidung 2026-06-16) — bewusst **nicht** als Minimal-Variante
  gegen das vorhandene Schema gepfuscht.
- **`cost_per_m2`** (Flächen-Kosten) — EVL-004 führt Volumen; `density` /
  `windows.frame_material` / Material an `stairs`/`openings` — Schema-Spalten ohne
  welle-3-Domänen-/Auswertungs-Bezug. **MAT-004 Texturen** (darstellungs-nah,
  Sicht). Eck-/Aussparungs-/Miter-Näherungen des Volumens (welle-3 bewusst).
- **`DRW`-Modul** (Bemaßung/Layer/Fangpunkte/Raster) — 2D-Zeichen-UX, orthogonal
  zu M3, bei Welle-Start nach welle-5 zurückgestellt. **slice-006**
  (Drittanbieter-Attribution) bleibt in `open/`; **WAL-006-Vollumfang** offen.

## 3. Review & Verifikation vor Closure

**Zweistufige Review-Disziplin je Slice** (Kurs-Modul 10/11): (a) **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
Plan-Review** vor jedem Implementierungs-Start (unabhängig, HIGH blockiert) — über
alle 7 Slices keine Start-blockierende HIGH offen (017d trug **1 HIGH** = die
`removeMaterial`-RESTRICT-Falle, vor Start eingearbeitet); (b) **geometrielastiges
Code-Review** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) für den einen geometrieschweren Slice **017c**
(EVL-002 Volumen): **0 HIGH**, Formeln per Hand bestätigt; (c) **zusätzliches
Code-Review (höhere Review-Latte, Projektinhaber-Entscheidung)** für den
Persistenz-Slice **017e**: **0 HIGH** (1 LOW gehärtet).

**Unabhängige Welle-Verifikation** (Reviewer ≠ Autor, eigener Agent, vor dieser
Closure): selbst `make gates` (Exit 0, 145/145, 92,7 %) **und** `make schema-check`
(Exit 0, kein Drift) gelaufen; geprüft: alle 7 Slices done mit Closure+Lerneintrag;
`EvaluatePort`-Oberfläche vollständig (floorArea/livingArea/volume/materials/
effectiveMaterial/materialList/doorList/windowList); **M3 sachlich erreicht**; jede
benannte Lücke **ehrlich** dokumentiert (kein Über-Versprechen); [ADR-0012](../../adr/0012-evaluations-architektur.md)-#4-Reinheit
grep-belegt (kein `GeometryKernelPort`/`Solid.volume_mm3` im Auswertungs-Pfad);
keine aktiven Carveouts; [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)/010/011/012-Hygiene gewahrt (Lösungsfreiheit,
Header==Historie, Spec→ADR-frei).

**Ergebnis:** Welle closure-reif, **keine HIGH**.

- **LOW-1 (behoben vor Closure):** spez. §1 „Material-Auflösungsregel (Datenfluss)"
  nannte den `wall_type`-Fallback im Präsens, die Zurückstellung stand erst im
  „Auflösungs-Teilumfang" 12 Zeilen darunter — Inline-Marker „welle-3
  zurückgestellt: Override-only" ergänzt (kein Über-Versprechen, nur Lesbarkeit).

## 4. Carveout-Audit (zwingend bei Welle-Closure)

Geprüft 2026-06-16 (unabhängig in §3 bestätigt):
[`docs/plan/carveouts/README.md`](../../carveouts/README.md) führt **keine
aktiven Carveouts**; kein `CO-*`-Eintrag, kein `done/`-Unterordner. Kein Gate war
während der Welle strukturell rot, keine Architekturregel/Schwelle wurde
geschwächt — die Coverage blieb stabil hoch (92,3 % welle-2 → **92,7 %** welle-3
bei wachsendem Codeumfang), arch-check trägt unverändert Regeln A–E,
`make schema-check` ohne Drift. Nichts aufzulösen, nichts nachzutragen.

| Carveout | Status vorher | Status nachher | Aktion |
|---|---|---|---|
| — | keine aktiven | keine aktiven | — (negativer Befund, belegt) |

## 5. Lerneinträge / Steering-Loop-Zähler

Fortschreibung aus [`welle-2-results.md` §5](welle-2-results.md) plus die in
welle-3 akkumulierten Praxis-Zähler (Kurs-Regel: 2× kategorisieren, 3× Regel):

1. **„NULL-vs-0 / kein-Wert ≠ 0": Welle-Muster, 2× bestätigt.** Die zentrale
   stille-Korruptions-Klasse trat in **017e** (Persistenz: `sqlite3_column_double`
   gibt für `NULL` `0.0` → zentrale `*Optional*`-Helfer mit `SQLITE_NULL`-Check)
   und **017g** (in-memory: Material ohne `cost_per_m3` → `cost = nullopt`, nicht
   `0`) auf. Konsequenz: ein abgeleiteter Wert aus einem **optionalen Eingabe-
   Kennwert** bleibt `nullopt`, nie `0` — sonst sieht „nicht gesetzt" wie „0" aus.
   Jeweils im AK gepinnt. Gilt **persistenz- UND speicher-seitig**.
2. **„Code-Review zahlt bei Persistenz/Serialisierung": 1× (unverändert).** Der
   welle-2-Zähler (015c-`stod`) wuchs **nicht** — 017e bekam zwar proaktiv ein
   Code-Review (höhere Latte, Projektinhaber), das aber **0 HIGH** fand. Ehrlicher
   Befund: die Latte wirkte hier als **Feedforward** (formte die zentralen
   NULL-sicheren Helfer + den NULL-Korrektheits-AK **vorab**), **nicht** als
   Bug-Fang. Der Promotion-Kandidat („`MR-<NNN>`: Code-Review bei Persistenz")
   reift über **gefundene** HIGHs, nicht über saubere Läufe — Zähler bleibt **1×**.
3. **„Spec-Konsistenz zwischen zwei welle-3-Entscheidungen": 2× kategorisiert.**
   Wenn eine Entscheidung etwas zurückstellt (Dach-Volumen, `wall_type`-Fallback)
   und eine andere darauf verweist (EVL-004-Materialliste; §1-Auflösungsregel),
   driftet die Spec in eine **stille interne Inkonsistenz**. Das unabhängige
   Review fing beide: **017f-[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)** (`roofs` aus der EVL-004-Aggregations-Menge)
   und die **welle-3-Verifikation LOW-1** (Datenfluss-Präsens vs. „zurückgestellt").
   Beide vor Closure behoben.
4. **„Schicht-übergreifender Reuse hält Folge-Slices [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-frei + in einer
   Sitzung review-bar": Welle-Muster.** Die Geometrie wurde **einmal** verifiziert
   (017c-[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure), 0 HIGH); 017f (Materialliste) und 017g (Kosten) reusen
   `volume_geometry`/`effectiveMaterial` und brauchten daher **kein** [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) —
   trotz vier neuer Werttypen war 017f in einer Sitzung review-bar. „Grünes Gate
   ≠ Korrektheit" bleibt: der schärfste 017c-Fang lag im Code-Review (kein
   `Solid.volume_mm3` lesen), nicht in den Gates.
5. **„Genuine Scope-/Semantik-Gabeln an den Projektinhaber, nicht unilateral":
   Welle-Muster.** Drei substanzielle Forks wurden per Frage entschieden statt
   stillschweigend gesetzt: **Dach-Volumen** (017c: zurückstellen — dicke-los ≠
   Material-Volumen), **Material-Port** (017d: bestehende Ports statt neuer/driven
   `MaterialLibraryPort`), **`wall_type`-Fallback** (017h-Kandidat: zurückstellen
   → welle-4+, da schema-treue Lösung ein eigener Strang). Hält die Welle ehrlich
   geschnitten und die Lücken bewusst.

**Welle-Lerneintrag:** Die [ADR-0012](../../adr/0012-evaluations-architektur.md)-Leitplanke (Auswertung = **reine
analytische Kern-Query**, kein driven-Round-Trip) trug über sechs Auswertungs-
Slices — die Versuchung, das adapter-gemessene `Solid.volume_mm3` (OCC-`GProp`)
zu lesen, hätte „reine Kern-Query" **still** gekippt, ohne dass ein Test rot würde;
`make arch-check` + der [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-grep hielten die Invariante computational. Der
durable Material-Strang (017d Domäne + 017e Persistenz) und die EVL-Aggregationen
(017b/c/f/g) amortisierten sich gegenseitig: jede spätere Auswertung reuste die
frühere (effectiveMaterial → materialList → cost), ohne neue Geometrie.

## 6. Nachfolge

- **M3 (Meilenstein „Auswertbar") erreicht** — Flächen, Volumen, Wohnfläche,
  Material-/Tür-/Fensterlisten + Kosten korrekt aus dem committeten Modell
  ableitbar; Roadmap §Meilensteine mit dieser Closure auf **erreicht** gesetzt.
- **Nächste Welle (Planungs-Entscheidung, kein Automatismus):**
  `welle-4-austausch` (`IO`: IFC/DXF/STEP/STL-Adapter, PDF/PNG-Export) — Trigger:
  welle-3 done **+ ADR zur IFC-Bibliothek accepted**.
- **Offen / Teilumfänge für später (alle als Re-Eval-Trigger benannt):**
  **Wandtyp-Bibliothek** (`wall_types` + `wall_type_id` + Material-Fallback +
  Persistenz, eigener welle-4+-Strang); **Dach-Volumen** (Dicke-/Material-Semantik);
  **`cost_per_m2`** + flächen-basierte Materialliste; **MAT-004 Texturen**;
  exaktes vereinigtes Volumen (Miter-Eck / überlappende Aussparungen);
  Wohnflächen-Anrechnungsfaktoren (DIN 277/WoFlV); **WAL-006-Vollumfang**;
  **slice-006** (Drittanbieter-Attribution) in `open/`; **`DRW`** (welle-5).
- **Steering offen:** der Header-Versions-Drift-Sensor (welle-2 #6,
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)-Promotion) bleibt computational-Tooling-Kandidat; der
  Persistenz-Code-Review-Promotion-Kandidat steht weiter bei 1× (kein welle-3-Fund).
