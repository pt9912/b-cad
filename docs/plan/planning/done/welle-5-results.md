# welle-5-erweiterung — Closure

**Zeitraum:** 2026-07-02 – 2026-07-27. **Meilenstein:** **M5 „Erweiterbar"
([OBJ-004](../../../../spec/lastenheft.md#3-projektziele)) erreicht.**

**Stand bei Closure:** `make gates` **EXIT=0** (docs-check 0 Befunde / 260 Dateien · a-check 0 ·
arch-check ok · **352/352** Tests · Coverage **91,8 %**), `make io-smoke` EXIT=0,
`make schema-check` EXIT=0.

---

## 0. Die ehrliche Vorbemerkung — warum diese Closure überfällig war

**Diese Welle hat ihren Meilenstein am 2026-07-03 erreicht und ist trotzdem 24 Tage weitergelaufen.**
M5 war mit dem Plugin-Strang inhaltlich geliefert; [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md):15
(Accepted, immutabel) stellt das seit dem 2026-07-05 ausdrücklich fest und nennt die übrigen Stränge
selbst „**nicht** meilenstein-bindend". Die Roadmap führte M5 dennoch bis heute als `offen`.

Was danach lief, waren **parallele Stränge unter einem Label, das nichts mehr trug**: am Ende
**31 Slice-Pläne** mit `welle: welle-5-erweiterung`, thematisch unverbunden — Plugin-System,
2D-Zeichenwerkzeuge, Export-Refactor, Export-Provenance, Golden-Files, GUI-Bedienbarkeit und sechs
harness-steering-Quergewerke. Eine Welle hat ein Ziel und ein Ende; das hier war ein Sammelbecken.

**Gefunden hat es der Projektinhaber am 2026-07-26**, nicht der Prozess — kein Gate prüft, ob ein
Wellen-Label noch etwas bedeutet, und die Wellen-Closure ist genau der Schritt, den ein Sammelbecken
nie erzwingt. Das ist der wichtigste Lerneintrag dieser Welle (§5).

Diese Closure bucht deshalb **zwei** Dinge: M5 als erreicht — und die Stränge, die daneben liefen,
als das, was sie waren.

## 1. Closure-Kriterien (beobachtbar)

| Kriterium | Beleg |
|---|---|
| **M5-Trigger:** [OBJ-004](../../../../spec/lastenheft.md#3-projektziele) (Erweiterbarkeit durch Plugins) erfüllt | [`LH-FA-PLG-001`](../../../../spec/lastenheft.md#lh-fa-plg-001)..004 auf AK-Niveau (Lastenheft 0.1.13) + Implementierung: Plugins werden **zur Laufzeit** aus Shared Libraries geladen, versionierter ABI-Handshake lehnt unpassende **vor jeder Wirkung** ab, Lifecycle beobachtbar, Fehlverhalten isoliert ([ADR-0017](../../adr/0017-plugin-api-abi.md)) |
| Plugin-Host als **Driving Adapter**, Kern plugin-frei | `src/adapters/plugin/`; a-check trägt die Kante `pluginhost → ports_driving`, **keine** Kern-Berührung |
| AK-Tests laufen gegen **reale Module** | `tests/adapters/test_plugin_host.cpp` lädt echte `.so` durch den echten Host (Happy / werfend / ABI-Mismatch / defekt / symbollos) |
| `make gates` grün am Closure-HEAD | s. o. — 352 Tests, Coverage 91,8 % |

**Die ADR-Feststellung ist die Quelle, nicht diese Datei:**
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md):15 hält seit dem 2026-07-05 fest, dass M5
„**inhaltlich geliefert**" ist. Diese Closure zieht die Roadmap darauf nach.

## 2. Gelieferter Umfang — nach **Strängen**, nicht als eine Wellen-Leistung

29 Slices in `done/`, zwei bleiben offen (§6). Die Gliederung nach Strängen ist keine nachträgliche
Ordnung, sondern die ehrliche Beschreibung dessen, was passiert ist.

### 2.1 Plugin-Strang — **der meilenstein-bindende** (M5)

`slice-026a` (PLG-AK + Spezifikation, [ADR-0017](../../adr/0017-plugin-api-abi.md)) ·
`slice-026b` (Host-Implementierung + AK-Tests gegen reale Module).

### 2.2 DRW-Strang — 2D-Zeichnen (nicht meilenstein-bindend)

`slice-032a/b/c` (Fundament Hilfslinien + Layer: AK/Spec, Implementierung, 2D-Export,
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)) · `slice-041a` (Canvas-ADR + AK,
[ADR-0019](../../adr/0019-drw-2d-canvas.md)) · `slice-043` (interaktiver 2D-Canvas: `QPainter`-Widget,
Hilfslinien-Zug per Maus, port-freie `std::function`-Naht) · `slice-048a` (DRW-001 Fangpunkte:
AK + Spezifikation).

### 2.3 Export-Strang — Refactor, Provenance, Golden-Netz

`slice-042a/b/c/d` ([ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md):
Driven Adapter **serialisieren nur**, der Kern liefert `DerivedGeometry`/`PersistedDerivations` —
danach **keine** Adapter→Kern-Kante mehr) · `slice-045` (PDF-Metadaten) · `slice-046a/b`
(Export-Provenance sichtbar in PDF-Footer, PNG-Titelblock, STEP/STL-Header) · `slice-044a`
(byte-genaues Golden-Netz aller sechs Formate + `make golden-regen`/`golden-check`).

### 2.4 GUI-Bedienbarkeit — der Strang, der die Welle überdauert hat

`slice-047` (Projekt öffnen: [`LH-FA-BLD-003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) Outline → AK, Lastenheft 0.1.17, Benutzerhandbuch 1.1) ·
**die Vierer-Kette** `slice-054` (`ManageProjectPort`) → `slice-053` (Fenster + Menü-Handler als
Adapter) → `slice-052a` (Sitzungs-Zustand, „Speichern", dreiwertige Rückfrage; Lastenheft 0.1.18,
Handbuch 1.2) → `slice-052b` („Neues Projekt"; Lastenheft 0.1.19, Handbuch 1.3).

**Ergebnis dieses Strangs:** [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen),
[`002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern) und
[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) sind erstmals **in der Oberfläche**
erfüllbar — anlegen, öffnen, speichern, mit Warnung vor Datenverlust.

### 2.5 harness-steering — sechs Quergewerke

`slice-033` (d-check-`matrix` `adr→slice`, [MR-014](../../../../harness/conventions.md)) ·
`slice-034` (`commits`, [MR-015](../../../../harness/conventions.md)) ·
`slice-035` (`vcs`/ADR-Immutabilität, [MR-016](../../../../harness/conventions.md)) ·
`slice-036` (`planning`/Ruhe-Marker, [MR-017](../../../../harness/conventions.md)) ·
`slice-037` (`tracked`/Fresh-Clone, [MR-018](../../../../harness/conventions.md)) ·
`slice-038` (`--trace`-Report, [MR-019](../../../../harness/conventions.md)) ·
`slice-049` (Spec-Straten prozess-/zeit-rein, [MR-023](../../../../harness/conventions.md)) ·
`slice-050` (Tooling-Konsolidierung, [MR-021](../../../../harness/conventions.md)/[MR-022](../../../../harness/conventions.md)).

**Zwölf neue MRs** in dieser Welle ([MR-012](../../../../harness/conventions.md)…[MR-023](../../../../harness/conventions.md)) —
mehr als in welle-1..4 zusammen. Das ist kein Zufall: ein Sammelbecken sammelt auch Prozess-Arbeit.

## 3. Review & Verifikation vor Closure

- **Je Slice** ein unabhängiges Plan-Review
  ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)),
  seit `slice-051`-Befund als Artefakt in `docs/reviews/` geführt. Allein die GUI-Kette am 26./27.07.
  brauchte **neun Läufe mit 19 HIGH** — jeder Lauf fand etwas Echtes, **keine Zeile Code ist
  deswegen gefallen**.
- **Geometrielastige Code-Reviews** ([MR-009](../../../../harness/conventions.md)) wo einschlägig.
- **Alt-Last, benannt statt geheilt:** für `slice-045`/`046a`/`046b`/`047` existiert **kein**
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report,
  obwohl die Plan-Köpfe Ergebnisse behaupten. Nachträgliche Reports wären Fälschung;
  der Gate-Nachzug liegt als [`slice-051`](../open/slice-051-review-artefakt-pflicht.md) offen (§6).

## 4. Carveout-Audit (zwingend bei Welle-Closure)

Geprüft 2026-07-27: [`docs/plan/carveouts/README.md`](../../carveouts/README.md) führt **keine aktiven
Carveouts**; kein `CO-*`-Eintrag. **Keine Architekturregel und keine Schwelle wurde geschwächt** — im
Gegenteil, die Welle hat **verschärft**: sechs neue Gate-Module (`matrix adr→slice`, `commits`, `vcs`,
`planning`, `tracked`, `targets`), das Architektur-Gate von `arch-check.sh` auf **a-check** gehoben
([MR-013](../../../../harness/conventions.md)), zwei lint-Härtungen (`slice-027`/`031`, Vorwelle),
und mit [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) **alle**
Adapter→Kern-Kanten entfernt. Coverage 90,7 % (welle-4) → **91,8 %**.

| Carveout | Status vorher | Status nachher | Aktion |
|---|---|---|---|
| — | keine aktiven | keine aktiven | — (negativer Befund, belegt) |

## 5. Lerneinträge / Steering-Loop-Zähler

1. **Ein Wellen-Label ohne Abschluss-Kriterium hört auf, etwas zu bedeuten — und niemand merkt es.**
   Diese Welle lief 24 Tage über ihren Meilenstein hinaus, während eine `Accepted`-ADR das Gegenteil
   feststellte. **Kein Gate prüft das**, und die Wellen-Closure ist genau der Schritt, den ein
   Sammelbecken nie erzwingt. **Konsequenz in welle-6: das Ziel steht mit einem beobachtbaren Trigger
   im Wellen-Block, und die Closure wird fällig, sobald er erfüllt ist — nicht, wenn die Arbeit
   ausgeht.**
2. **Der teuerste Fehler-Typ der Welle war „die Zusage steht, aber am falschen Ort".** Dreimal in
   Folge (053 · 052a · 052b): ein Finding galt als eingearbeitet, war aber nur verschoben; ein Orakel
   prüfte die falsche Komponente; eine Gegenprobe blieb **grün**, obwohl die Zusage brach.
   **Regel-Kandidat, 3×-Regel erfüllt** — eine Orakel-Zeile benennt nicht nur die Zusicherung, sondern
   die **Komponente**, an der sie diskriminiert; Zusagen über das Zusammenspiel zweier Komponenten
   werden am **Zusammenspiel** belegt. *Die MR anzulegen ist eine Regelwerk-Entscheidung des
   Projektinhabers.*
3. **Ein Default-Argument kann ein Orakel aushebeln.** `sinks = {}` am `ManageProjectPort` hätte einen
   vergesslichen Aufrufer still durchgelassen — der Verlust wäre unter **allen** Orakeln grün
   geblieben. Wo ein Parameter verhaltenstragend ist, ist der **fehlende** Default der billigste und
   schärfste Sensor.
4. **`main.cpp` ist orakel-los per Konstruktion** — in kein Testbinary gelinkt, coverage-ausgenommen.
   Die Frage ist nie „wie teste ich `main`?", sondern „gehört dieser Schritt dorthin?". Vier Slices
   der Welle sind aus genau dieser Klasse entstanden.
5. **Eine Prüfung findet nur, was ihr Maßstab enthält.** Plan-Review, Code-Review **und** Verify
   übersahen ein falsches Benutzerhandbuch, weil die Doku-DoD-Zeile nur `CHANGELOG` + `spec/` nannte.
   Regel seither: ein Slice, der eine Anforderung benutzer-erfüllbar macht, führt **`docs/user/`** in
   seiner Doku-DoD-Zeile.
6. **Reviewer-Befunde sind Hypothesen, keine Urteile.** Mindestens zweimal war ein Finding am Artefakt
   **falsch** (Klammerform `.{h}`; eine „unvollständige" Gegenprobe, die es nicht war). Widerlegen und
   die Messung protokollieren — nicht reflexhaft einarbeiten.

## 6. Nachfolge

**welle-6-interaktiv-planen** (Projektinhaber-Entscheidung 2026-07-27):

- **Ziel:** [OBJ-001](../../../../spec/lastenheft.md#3-projektziele) — „Gebäude ohne tiefe
  CAD-Kenntnisse modellierbar". Der Benutzer legt **Bauteile in der Oberfläche** an und bearbeitet
  sie. Das Benutzerhandbuch nennt genau das heute noch „den **nächsten großen Ausbauschritt**" und
  führt es unter „In dieser Version noch **NICHT** möglich".
- **Trigger (beobachtbar):** eine **Wand** lässt sich im 2D-Canvas **zeichnen** und **parametrisch
  ändern** — ohne Kommandozeile.
- **Warum jetzt:** die Welle baut direkt auf dem, was gerade fertig wurde — der interaktive Canvas
  (`slice-043`) und die GUI-Kette (054/053/052a/052b), die dem Fenster erstmals Sensoren gegeben hat.
  Die DRW-Aids (Fang/Raster/Winkel) werden dabei **Mittel**, nicht Selbstzweck.

**Aus welle-5 übernommen, nicht erledigt:**

- [`slice-051`](../open/slice-051-review-artefakt-pflicht.md) — Review-Artefakt-Pflicht als Gate
  (Alt-Last §3).
- [`slice-044b`](../open/slice-044b-golden-import-fremd.md) — Golden-Import fremder IFC/DXF-Dateien.
- **`slice-048b`** (DRW-001-Implementierung, Fang-Punkte) — Plan noch nicht in `open/`; gehört
  inhaltlich in welle-6.
- Alt-Bestand ohne Wellen-Bindung: `slice-006` (Drittanbieter-Attribution), `slice-039a/b` (Akustik),
  `slice-040a` (GLB/CSV).

**Re-Eval-Träger** (aus welle-3/-4 weitergereicht, weiter offen): Format-Reichtum (echte
IFC-/DXF-Bibliothek, PDF-Fit-to-Page/Bemaßung), Wandtyp-Bibliothek (`wall_type`-Template-Fallback),
Observability (`TracingPort`-Anbindung).
