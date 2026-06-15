---
id: welle-2-results
titel: Welle-Closure — Ergebnisnotiz welle-2-bauteile
status: done
welle: welle-2-bauteile
---

# Welle-2-Closure — Ergebnisnotiz `welle-2-bauteile`

**Welle:** welle-2-bauteile. **Zeitraum:** 2026-06-13 bis 2026-06-14.
**Autor:** Dietmar Burkard. **Closure-Datum:** 2026-06-14.

**Welle-Ziel (Roadmap):** Alle parametrischen Bauteile über die Wände
hinaus — **Türen + Fenster** (mit automatischer Wandöffnung), **Treppen**,
**Dach**, **Decken**, **Fundament**. Jedes Modul vom Lastenheft-Outline auf
AK-Niveau geschärft (Reifephase-Klausel), in `spec/spezifikation.md`
spezifiziert, hinter den bestehenden Ports (`EditStructurePort`/
`GeometryKernelPort`/`ViewModelPort`) implementiert, der [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)-Mechanik
im Viewer folgend und über das [ADR-0006](../../adr/0006-relationales-schema-design.md)-Schema persistiert. Erfüllt
**Meilenstein M2** („Haus mit Türen, Fenstern, Dach vollständig") und die
**Bauteil-Hälfte von [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)**. Architektur-Leitplanke: **[ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md) (#6)**
„Bauteil-Erweiterungs-Muster" — **kein neuer Grundsatz-ADR pro Bauteil**.

---

## 1. Closure-Kriterien (beobachtbar)

- **Alle Closure-Trigger der Roadmap erfüllt:** zwölf Slices liegen in
  `done/` (Liste §2), jeder mit eigener Closure-Notiz und Lerneintrag; je
  Bauteil-Familie die Trias **a (Lastenheft-AK-Schärfung) · b (Implementierung)
  · c (Persistenz)**. Diese Notiz schließt den letzten Trigger
  („unabhängige Welle-Verifikation + Closure-Notiz inkl. zwingendem
  Carveout-Audit").
- **`make gates` grün** (frischer Lauf 2026-06-14 am HEAD `d7073fb`,
  Exit 0): docs-check **0 Befunde** (80 Dateien), gate-consistency,
  arch-check **Regeln A–E** (Kern OCC-/Qt-/SQLite-frei; OCC nur Geometrie-,
  SQLite nur Persistenz-, Qt nur UI-Adapter), lint 0 + suppression-gate,
  **Tests 116/116** (100 %), **Coverage 92,3 % lines** (1843/1997) /
  **100 % functions** (248/248), Schwelle 70 %. Frisch reproduziert
  (Honesty-Disziplin), nicht aus den Slice-Läufen übernommen.
- **Unabhängige Welle-Verifikation gelaufen** (Reviewer ≠ Autor, eigener
  Agent ohne Autoren-Kontext; §3): **Welle closure-reif, keine HIGH.** Die
  zwei nicht-blockierenden Auflagen (Frontmatter-Status der 016-Slices, drei
  DoD-Checkboxen) wurden **vor** dieser Notiz behoben.

## 2. Gelieferter Umfang (Slices in `done/`)

| Familie | Slices | Ergebnis |
|---|---|---|
| **ADR-Leitplanke** | slice-013a | **[ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md)** „Bauteil-Hosting & Wandöffnungs-Modell" accepted (Hosting, Öffnungs-Boolean, Notification, Persistenz + verallgemeinertes Erweiterungs-Muster #6 als Welle-Leitplanke); Lastenheft DOR/WIN Outline→AK |
| **Türen + Fenster** | slice-013b/c | Öffnungen als wand-gehostete Bauteile (`model::Opening` + `CutPrism`, OCC-`BRepAlgoAPI_Cut`/TKBO), automatische Wandöffnung sichtbar, `WallGeometryChanged`; Persistenz `openings`/`doors`/`windows`-CTI. Code-Review 013b: 1 HIGH (lateraler Cutter-Überstand) gefixt |
| **Dach** | slice-014a/b/c | Sattel/Walm/Pult über rechteckigem Grundriss (analytisches `roof_geometry`, kein OCC), `RoofChanged`; Persistenz `roofs`/`footprint_json` (erste JSON-Ser/De). Code-Review 014b: 1 HIGH gefixt |
| **Decken + Fundament** | slice-015a/b/c | horizontale Platten (`slab_geometry`, base_z via Mesh-Translation, Ausschnitt-Boolean), `SlabChanged`; Persistenz `slabs`/verschachteltes `polygon_json` (footprint + cutouts). Code-Review 015b: 1 HIGH (OCC-Cutout ungetestet) gefixt; 015c-Code-Review: 1 MED (`stod`-Totalität) |
| **Treppen** | slice-016a/b/c | gerade einläufige Treppe (analytisches Stufen-Polyeder + Geländer, kein OCC, `rise` abgeleitet), `StairChanged`; Persistenz `stairs` (`rise_mm` write-derived). Code-Review 016b: **keine HIGH** — Test-Orakel gehärtet (M1/M2/M3) |

**Bezug zu welle-1/-1v:** welle-1 lieferte den Kern-Vertrag
(Projekt/Geschosse/Wände/Raumerkennung/OCC-Extrusion + Echtzeit-
Benachrichtigung), welle-1v die sichtbare Hälfte (3D-Viewer, [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)).
welle-2 baut **vier Bauteil-Familien** additiv darauf — ohne Port-Signatur-
Migration der bestehenden Wand-/Viewer-Pfade.

**Nicht Teil der Welle (bewusst, ehrlich benannt):**

- **Teilumfänge:** Dach + Platten über **rechteckigem** Grundriss; gerade
  **einläufige** Treppe mit fester **+x-Richtung** (Schema ohne Richtungs-
  Spalte). Komplexe Polygon-Grundrisse, Podest-/U-/L-/Wendeltreppen, freie
  Rotation, echte Mehr-Geschoss-Elevation bleiben **ausdrücklich offen**.
- **Abgeleitete / render-only Felder ohne Persistenz:** Dach-Firsthöhe,
  Slab-base_z, Treppen-`rise` (write-derived) und das Treppen-Geländer
  (render-only) tragen keinen persistierbaren Eigenzustand — kein stiller
  Verlust (Muster roofs-`height_mm`).
- **slice-006** (Drittanbieter-Attribution) war nie Closure-Trigger, bleibt
  in `open/`. **WAL-006-Vollumfang** (Schnittpunkte/T-Stöße als Knoten)
  bleibt aus welle-1v offen — kein welle-2-Blocker.

## 3. Review & Verifikation vor Closure

**Zweistufige Review-Disziplin je Slice** (Kurs-Modul 11): (a) **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
Plan-Review** vor jedem Implementierungs-Start (unabhängig, HIGH blockiert) —
über alle 12 Slices keine HIGH, durchweg MED/LOW eingearbeitet; (b)
**geometrielastiges Code-Review** nach jeder geometrieschweren Implementierung
(013b/014b/015b/016b) + Persistenz (015c).

**Unabhängige Welle-Verifikation** (Reviewer ≠ Autor, eigener Agent, vor
dieser Closure): geprüft 1–8 (Gates real grün; alle vier Familien vollständig
mit Domäne + Geometrie + ViewModel/Viewer + Edit-Ops + Persistenz;
Traceability je Modul über AK-Tests mit `LH-`-ID im Namen; [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md)-#6-Index
vollständig **erfüllt**; keine aktiven Carveouts; M2 sachlich erreicht;
Teilumfänge ehrlich benannt; keine halluzinierten Gates).

**Ergebnis:** Welle closure-reif, **keine HIGH**.

- **MED-1 (behoben vor Closure):** die drei `slice-016a/b/c` trugen im YAML-
  Frontmatter noch `status: in-progress` (Body/Ordner/Roadmap führten sie als
  done) — reines Abhak-Versäumnis der zuletzt geschlossenen Familie, korrigiert.
- **LOW-1 (behoben):** je eine unmarkierte DoD-Checkbox in 014b/015b/016b
  (faktisch durch den realen Gate-Lauf eingelöst), abgehakt.

## 4. Carveout-Audit (zwingend bei Welle-Closure)

Geprüft 2026-06-14 (unabhängig in §3 bestätigt):
[`docs/plan/carveouts/README.md`](../../carveouts/README.md) führt **keine
aktiven Carveouts**; kein `CO-*`-Eintrag, kein `done/`-Unterordner (nie einer
existiert). Kein Gate war während der Welle strukturell rot, keine
Architekturregel/Schwelle wurde geschwächt — im Gegenteil **stieg** die
Coverage (welle-1v 94,2 % bei kleinem Kern → 92,3 % bei ~2× Codeumfang, weit
über Schwelle 70 %) und der arch-check trägt unverändert Regeln A–E. Nichts
aufzulösen, nichts nachzutragen.

| Carveout | Status vorher | Status nachher | Aktion |
|---|---|---|---|
| — | keine aktiven | keine aktiven | — (negativer Befund, belegt) |

## 5. Lerneinträge / Steering-Loop-Zähler

Fortschreibung aus [`welle-1v-results.md` §5](welle-1v-results.md) plus die in
welle-2 akkumulierten Praxis-Zähler (Kurs-Regel: 2× kategorisieren, 3× Regel):

1. **„Lösung schärft nie das Lastenheft": geschlossen → Konvention [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei).**
   Der in welle-1v bei 3× stehende Zähler wurde zu Wellenbeginn als
   [`MR-008`](../../../../harness/conventions.md) (+ AGENTS §4) festgeschrieben.
   In welle-2 durchgängig wirksam: jede a-Slice (013a/014a/015a/016a) schärfte
   das Lastenheft **lösungsfrei**, die Mechanik (Algorithmen, Formeln,
   `op`-Vokabular) lebt in `spec/spezifikation.md` §1/§3. **Zähler geschlossen.**
2. **„Geometrielastige Slices vor Welle-Closure code-reviewen": 4× — reif zur
   Festschreibung.** 013b/014b/015b ergaben je **1 HIGH trotz grüner Gates**
   (lateraler Überstand / Orientierung / ungetesteter OCC-Boolean); 016b ergab
   **erstmals keinen HIGH** — die Serie brach, plausibel weil die Treppe rein
   analytisch (kein OCC-Boolean) ist und die Geländer-Sonde schon im
   Plan-Review gefangen wurde. Die Praxis ist real 4× angewandt → **Kandidat
   für Festschreibung (`MR-<NNN>`) zu welle-3-Start.**
3. **„Grünes Gate ≠ Korrektheit/Spec-Treue": durchgängig bestätigt.** Der
   schärfste Fund je geometrieschwerem Slice lag **außerhalb** der automatischen
   Gates (Code-Review), nicht darin — die Fortsetzung des welle-1v-Lerneintrags
   („der schärfste Fund lag am Abnahme-Artefakt").
4. **„Code-Review zahlt auch bei *mechanischer* Persistenz": 1× kategorisiert**
   (015c). Der `stod`-Totalitäts-Fund (Müll-Suffix still verschluckt) zeigte:
   nicht nur Geometrie-Slices tragen die HIGH-trotz-grüner-Gates-Klasse — jeder
   Slice mit eigener **Parsing-/Format-Logik** tut es.
5. **„Test-Orakel müssen die Geometrie absichern, nicht nur der Code": 1×
   kategorisiert** (016b-Code-Review). Erstmals fand das Code-Review **keinen
   Code-Defekt**, aber **Test-Lücken** (invertierte Normalen, Stufen-Spalt wären
   durchgerutscht) — die Sicherheit kam aus dem Code, nicht den Tests. Konsequenz:
   geschlossene-Körper-/Bündigkeits-Sonden ergänzt (Divergenzsatz, x-Kanten).
6. **„Lastenheft-Header-Versions-Drift": 3× → Steering-Kandidat.** 013a/014a/015a
   ergänzten je eine §9-Historie-Zeile, zogen aber den `Version:`-Header nicht
   nach (blieb seit slice-012 auf 0.1.2 bei Historie 0.1.5); erst 016a fiel auf
   den Drift und korrigierte (→ 0.1.6). Eine docs-check-Regel „Header ==
   oberste §9-Historie-Zeile" würde die Klasse computational fangen — **Kandidat,
   bewusst außerhalb der Welle.**
7. **„Abgeleitete/render-only Felder halten den Schema-Vertrag klein": Welle-
   Muster.** Dach-Firsthöhe, Slab-base_z, Treppen-`rise` (write-derived) und das
   Geländer (render-only) brauchten **kein** eigenes Domänen-/Schema-Feld —
   Single-Source der Ableitung im Kern, der Adapter/Viewer konsumiert sie. Die
   Trennlinie **„domänen-getragen mit Eigenzustand ⇒ round-trippt, sonst nicht"**
   trägt jetzt über vier Familien hinweg konsistent in beide Richtungen
   (013c/015c-cutouts round-trippen ↔ roofs-height/Treppen-rise nicht).

**Welle-Lerneintrag:** Die [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md)-Leitplanke (#6, ein Erweiterungs-Muster
statt ein ADR je Bauteil) hat sich über **vier** Familien getragen — ohne
neuen Grundsatz-ADR, ohne Port-Signatur-Migration der Bestands-Pfade. Der
gemeinsame `reloadKeyed`-Viewer-Helfer (Roof+Slab+Stair) und das geteilte
Klemm-/Notification-Muster zeigen: ein gut geschnittenes Erweiterungs-Muster
amortisiert sich ab dem dritten Bauteil sichtbar.

## 6. Nachfolge

- **M2 (Meilenstein „Vollständige Bauteile") erreicht** — Türen/Fenster/Dach
  (+ Decken/Fundament/Treppen) erstellbar, sichtbar und persistent; Roadmap
  §Meilensteine mit dieser Closure auf **erreicht** gesetzt.
- **[ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Bauteil-Hälfte** geliefert (parametrische Bauteile am Gebäudemodell).
- **Nächste Welle (Planungs-Entscheidung, kein Automatismus):**
  `welle-3-auswertung` (Material `MAT`, Auswertungen `EVL`, Bemaßung/Layer
  `DRW`). **EVL muss auf der Footprint-Fläche (Shoelace) aufsetzen**, nicht auf
  Länge·Stärke (Spec §1-Hinweis, welle-1v §6).
- **Offen / Teilumfänge für später:** komplexe Polygon-Grundrisse (Dach/Platte),
  Podest-/Wendeltreppe + freie Rotation, Mehr-Geschoss-Elevation, Material/
  Bewehrung, `openings.name`/Tür-Fenster-Detailfelder; **WAL-006-Vollumfang**
  (aus welle-1v); slice-006 (Drittanbieter-Attribution) in `open/`.
- **Steering vor welle-3-Start:** Zähler #2 (geometrielastiges Code-Review) und
  #6 (Header-Versions-Drift-Sensor) zur Festschreibung erwägen.
