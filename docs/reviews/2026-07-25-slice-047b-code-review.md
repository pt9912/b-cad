# slice-047b Code-Review ([MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure))

**Review-Art:** Code-Review (fertiger Diff gegen Plan + Konventionen) — **kein** Plan-Review.

**Gegenstand:** Commit `15448d2` „feat(slice-047b): Projekt speichern/oeffnen im GUI (LH-FA-BLD-002/003)",
Range `15448d2~1..15448d2` (16 Dateien, +542/−9). **Nicht** Gegenstand: slice-047a (`70a30f1`) und die
vier slice-050-Commits davor; sie werden nur zitiert, wo 047b sie fortschreibt.

**Skill-Version:** `.harness/skills/reviewer.md` v1.0 (Accepted, 2026-07-25).

**Modell:** Claude Opus 5 (1M), unabhängiger Reviewer ≠ Autor, ohne Kontext aus der Autoren-Sitzung.

**Eingangs-Kontext (Pflichtliste der Skill-Datei):** der Diff selbst · `spec/lastenheft.md`
(LH-FA-BLD-002/003, LH-FA-D3-001/002, LH-FA-ROM-001, ACC-005) · `spec/spezifikation.md`
(§1 D3-002.a, §1 IO-007.a, §4 E-GEO-002/E-IO-001, §5 Span-Tabelle) · `spec/spezifikation-historie.md` ·
`spec/architecture.md` §1.2 · ADR-0001/0003/0008/0009/0018/0019/0020 · `AGENTS.md` §2 + §3 + §4 + §5 ·
`harness/conventions.md` (MR-006/MR-008/MR-009/MR-011/MR-017) · `.a-check.yml` · `.d-check.yml` ·
der Slice-Plan `docs/plan/planning/done/slice-047-projekt-oeffnen.md` ·
`docs/reviews/` (letzte Reports zum Modul: **kein** slice-047-Report vorhanden, s. INFO-1).

**Eigene Sensor-Läufe (nur `make`, AGENTS §2.9):**

- `make gates` auf dem Gegenstands-Stand: **EXIT=0**, 274/274 Tests, lines 91,2 % (4057/4448).
- `make test` mit lokal zurückgenommener Produktionsänderung (Experiment, danach `git checkout --`):
  siehe MEDIUM-1/2/3 — inkl. Positiv-Kontrolle, die den Rebuild-Pfad belegt.

---

## HIGH

### HIGH-1 — Die neue Spec-Aussage „Beide Wege nutzen **denselben** Kern-Use-Case" widerspricht dem Code

- **kategorie:** HIGH
- **quelle:** Source Precedence #2 (`spec/spezifikation.md` ist technisch verbindlich), LH-FA-BLD-003,
  Honesty (AGENTS §3/§5.9)
- **pfad:** `spec/spezifikation.md`:726 vs. `src/main.cpp`:209–218 und `src/main.cpp`:292
- **befund:** Der in diesem Commit eingefügte Spec-Absatz sagt normativ, CLI und GUI-Datei-Menü nutzten
  denselben Kern-Use-Case; tatsächlich ruft der CLI-Öffnen-Pfad in `runHeadlessCli`
  `repository.load(open_path)` direkt und durchläuft weder `services::openProject` noch
  `StructureEditService::replaceBuilding` — also weder Id-Zähler-Reset noch `ModelReplaced`-Meldung noch
  die Solid-Vorprüfung; das geladene `Building` erreicht den Service auf dem CLI-Weg überhaupt nie
  (`grep` über alle Aufrufer: `openProject` erscheint nur in `src/main.cpp`:292 und in zwei Tests).
  Für **Speichern** trifft die Aussage zu (beide Wege rufen `services::saveProject`), für **Öffnen** nicht.
- **verifizierbar:** ja — `grep -rn "openProject\|replaceBuilding" src/ tests/` (kein CLI-Treffer) und
  Lesen von `runHeadlessCli`; kein Gate deckt es ab (`make gates` ist auf beiden Ständen grün).

---

## MEDIUM

### MEDIUM-1 — Die `redetectRooms`-Schleife in `replaceBuilding` ist durch keinen Test gedeckt

- **kategorie:** MEDIUM
- **quelle:** MR-009 (nicht diskriminierende Zusicherung), LH-FA-ROM-001, Slice-Plan §3
  („**Unit-Test:** nach dem Laden eines geroomten Projekts ist `floorArea` nicht leer")
- **pfad:** `src/hexagon/services/structure_edit_service.cpp`:386–389; erwarteter Test-Ort
  `tests/hexagon/test_manage_project.cpp`:131–189
- **befund:** Entfernt man die `redetectRooms`-Schleife lokal, bleibt `make test` bei **274/274** grün;
  kein Test prüft nach `replaceBuilding`/`openProject` Räume, `rooms(...)` oder `floorArea` — das im Plan
  als DoD benannte `floorArea`-Orakel existiert in keiner Testdatei (`grep floorArea tests/` trifft nur
  EVL-Tests ohne Modell-Ersetzung), und das Fixture `loadedProject()` enthält mit **einer** Wand ohnehin
  keinen geschlossenen Raum.
- **verifizierbar:** ja — `make test` nach lokalem Entfernen der Schleife (durchgeführt: grün);
  Positiv-Kontrolle s. MEDIUM-2.

### MEDIUM-2 — 7 der 9 Id-Zähler-Resets sind ungetestet

- **kategorie:** MEDIUM
- **quelle:** MR-009 (nicht diskriminierende Zusicherung), Klassifikation „Korrektheit im kritischen Pfad:
  Id-Vergabe"
- **pfad:** `src/hexagon/services/structure_edit_service.cpp`:394–402;
  `tests/hexagon/test_manage_project.cpp`:145–160
- **befund:** Der Test prüft nur `next_wall_id_` und `next_layer_id_`; entfernt man die übrigen sieben
  Resets (`storey`, `opening`, `roof`, `slab`, `stair`, `material`, `guide_line`), bleibt `make test` bei
  **274/274** grün. Positiv-Kontrolle: entfernt man zusätzlich die Wand-/Ebenen-Resets, fällt **genau ein**
  Test (`ManageProject_047b.ReplaceBuildingRebuildsDerivedStateAndResetsCounters`) — der Rebuild-Pfad des
  Experiments ist damit belegt, das grüne Ergebnis oben also aussagekräftig.
- **verifizierbar:** ja — zwei `make test`-Läufe (durchgeführt: 274/274 grün bzw. 1 Fehlschlag).

### MEDIUM-3 — Die zugesagte Transaktionalität von `replaceBuilding` ist durch keinen Test gedeckt

- **kategorie:** MEDIUM
- **quelle:** MR-009, AGENTS §2.2-Nachbarschaft (Persistenz/Geometrie-Transaktionalität),
  Header-Zusage in `src/hexagon/services/structure_edit_service.h`:201–210
- **pfad:** `src/hexagon/services/structure_edit_service.cpp`:373–391;
  `tests/hexagon/test_manage_project.cpp`:220–246
- **befund:** Vertauscht man Commit und Trial-Bau (`building_ = std::move(building)` **vor** die
  Solid-Schleife), bleibt `make test` bei **274/274** grün; kein Test lässt den Geometrie-Kern beim
  Laden werfen. Der vorhandene Fehlerfall-Test deckt ausschließlich das werfende **Repository**
  (`ThrowingRepository`), also den Pfad, der `replaceBuilding` gar nicht erst erreicht — die im Header
  und in der Commit-Message zugesagte E-GEO-002-Transaktionalität hat kein Orakel.
- **verifizierbar:** ja — `make test` nach lokaler Umstellung der Reihenfolge (durchgeführt: grün).

### MEDIUM-4 — Neuer `op`-Wert ohne den in diesem Repo üblichen Spec-/Historie-Nachzug; element-lose Meldung vs. ADR-0008 #2

- **kategorie:** MEDIUM
- **quelle:** [ADR-0008](../plan/adr/0008-aenderungs-benachrichtigung.md) Entscheidung 2
  („Gemeldet werden **nur** `element_id` und `op`"), `spec/spezifikation.md` §1 D3-002.a,
  AGENTS §4 (op-Vokabular gehört in die Spezifikation, MR-008)
- **pfad:** `src/hexagon/ports/driven/model_changed_port.h`:38–46;
  `spec/spezifikation.md`:344–380 (unverändert); `spec/spezifikation-historie.md` (unverändert)
- **befund:** `ModelReplaced` trägt laut eigenem Kommentar bewusst **keine** sinnvolle Element-Id, während
  ADR-0008 #2 und der §1-D3-002.a-Vertrag die Meldung als `element_id` + `op` beschreiben; der §1-Block
  wurde nicht ergänzt und die Spec-Historie erhielt keine Zeile — anders als bei jedem früheren neuen
  `op` (`RoofChanged` §1:212, `SlabChanged` §1:265, `StairChanged` §1:334, letzterer ausdrücklich als
  „neuer `op` im D3-002.a-§5-Span-Vokabular" in der Historie geführt).
- **verifizierbar:** teilweise — kein Gate (`docs-check`/`matrix`/`ids` prüfen Vokabular-Vollständigkeit
  nicht); nachlesbar durch Diff von `spec/spezifikation.md` gegen die drei genannten §1-Stellen.

### MEDIUM-5 — Der einzige neue Spec-Absatz steht im §1-Block des PDF-/PNG-Exports

- **kategorie:** MEDIUM
- **quelle:** `spec/spezifikation.md` §1-Gliederung (Block je Anforderung), MR-011 (Referenz-Integrität)
- **pfad:** `spec/spezifikation.md`:723–731 unter der Überschrift `### LH-FA-IO-007.a — PDF-/PNG-Export
  (Mapping, Teilumfang)` (Zeile 668; nächste Überschrift Zeile 747)
- **befund:** Die BLD-002/003-Aussage („Aufruf-Wege", Id-Kollisionsfreiheit, Sichten nach dem Öffnen) ist
  im §1-Block der PDF-/PNG-Export-Anforderung abgelegt; einen §1-Block zu BLD-002/003 gibt es nicht, und
  der Absatz steht damit unter einer Überschrift, deren Anforderung er nicht präzisiert.
- **verifizierbar:** ja, durch Lesen der Überschriften-Struktur (`grep -n "^###" spec/spezifikation.md`);
  kein Gate greift (die `matrix`-Regel prüft nur die Referenz-Richtung, nicht die Block-Zuordnung).

### MEDIUM-6 — Kein CHANGELOG-Eintrag für 047b, obwohl neue öffentliche Kern-API entsteht

- **kategorie:** MEDIUM
- **quelle:** AGENTS §5.8 („Doku/Indizes aktualisieren, falls ein öffentlicher Vertrag berührt"),
  gelebte Repo-Praxis (Keep-a-Changelog, Eintrag je Slice — 047a hat einen)
- **pfad:** CHANGELOG (unverändert in diesem Commit; `grep 047` trifft nur 047a-Zeilen);
  Slice-Plan `docs/plan/planning/done/slice-047-projekt-oeffnen.md`:107
- **befund:** Der Commit fügt drei öffentliche Verträge hinzu (`services::openProject`,
  `StructureEditService::replaceBuilding`, `ModelChangeOp::ModelReplaced`) und ändert das Startverhalten
  des GUI, schreibt aber keinen CHANGELOG-Eintrag; die entsprechende Doku-Zeile im Slice-Plan wurde im
  selben Commit abgehakt (die DoD-Buchhaltung selbst ist Verifier-Sache, s. INFO-4).
- **verifizierbar:** ja — `git show 15448d2 --stat` (kein CHANGELOG) und `grep -n "047" CHANGELOG.md`.

### MEDIUM-7 — Die HIGH-3-Auflösung (`setActiveStorey`/`setTarget`) hat außerhalb der coverage-ausgenommenen `main.cpp` keinen Aufrufer

- **kategorie:** MEDIUM
- **quelle:** MR-009 (fehlender Negativtest bei neuem öffentlichem Vertrag), Slice-Plan §3
  („ein abonnierter headless `CanvasWidget`/`ViewerScene` spiegelt ihn")
- **pfad:** `src/adapters/ui/view/canvas_widget.cpp`:23–30,
  `src/adapters/ui/command/edit_drawing_guide_line_sink.h`:35–39; einzige Aufrufer
  `src/main.cpp`:277–278
- **befund:** Beide neuen Setter werden ausschließlich aus dem Composition-Root gerufen, der von der
  Coverage ausgenommen und nicht test-gedeckt ist; der neue Adapter-Test abonniert nur eine `ViewerScene`,
  kein `CanvasWidget`, und prüft weder das aktive Geschoss nach dem Laden noch ein `addGuideLine` auf dem
  geladenen Stand — das im Plan als eigentliche HIGH-3-Symptomatik beschriebene Verhalten (leerer Canvas,
  abgelehnte Hilfslinie) hat kein Orakel.
- **verifizierbar:** ja — `grep -rn "setActiveStorey\|setTarget" src/ tests/` (nur `main.cpp`).

### MEDIUM-8 — Das GUI-Öffnen mutiert das gerade geladene Modell (legt eine Ebene „Canvas" an)

- **kategorie:** MEDIUM
- **quelle:** LH-FA-BLD-003 („Modellbaum … vollständig wiederhergestellt"), LH-FA-BLD-002-AK
  („erneutes Laden ergibt ein identisches Modell"), der in diesem Commit ergänzte Spec-Satz
  `spec/spezifikation.md`:729
- **pfad:** `src/main.cpp`:266–273
- **befund:** Enthält die geöffnete Projektdatei keine Ebenen, legt `reresolve_after_open` unmittelbar
  nach dem Laden über `service.addLayer` eine Ebene „Canvas" im geladenen Modell an; der Stand im
  Speicher weicht danach vom Dateiinhalt ab und ein anschließendes „Speichern unter" schreibt die
  zusätzliche Ebene mit. Es ergeht dazu keine Meldung (`addLayer` hat vertragsgemäß kein `op`).
- **verifizierbar:** ja — Projekt ohne Ebenen speichern, öffnen, erneut speichern und die `layers`-Tabelle
  beider Dateien vergleichen; kein Test deckt den Fall.

### MEDIUM-9 — Der in `spec/architecture.md` geführte `ManageProjectPort` bleibt unbedient; der Öffnen-Weg fährt über die konkrete Service-Klasse

- **kategorie:** MEDIUM
- **quelle:** `spec/architecture.md` §1.2 (Port-Tabelle: „`ManageProjectPort` | Projekt anlegen, speichern,
  laden, versionieren | LH-FA-BLD-001..004, ACC-005"), [ADR-0001](../plan/adr/0001-hexagonale-architektur.md)
- **pfad:** `spec/architecture.md`:76 und :136 vs. `src/hexagon/ports/driving/` (kein solcher Port);
  `src/hexagon/services/manage_project.h`:37–39
- **befund:** Der neue Öffnen-Use-Case ist eine freie Funktion, die die **konkrete** Klasse
  `StructureEditService&` nimmt (nicht einen Driving-Port); die Architektur-Doku führt für genau diese
  Anforderungen weiterhin einen `ManageProjectPort`, den es im Quellbaum nicht gibt. Das Muster stammt aus
  047a (dort im Kommentar als „der vorgesehene `ManageProjectPort`" bezeichnet) und wird hier fortgeschrieben.
- **verifizierbar:** ja — `ls src/hexagon/ports/driving/` gegen `spec/architecture.md`:76/:136;
  `make a-check` ist grün (die Kante `services → services` und der Composition-Root sind erlaubt).

### MEDIUM-10 — Ganzdatei-Ablehnung beim GUI-Öffnen ist weder spezifiziert noch getestet

- **kategorie:** MEDIUM
- **quelle:** LH-FA-BLD-003, E-GEO-002 (`spec/spezifikation.md` §4), der neue Spec-Satz
  `spec/spezifikation.md`:727 (nennt als Fehlerbilder nur „fehlende/korrupte Datei, nicht beschreibbarer Pfad")
- **pfad:** `src/hexagon/services/structure_edit_service.cpp`:378–381; `src/main.cpp`:291–303
- **befund:** Wirft der Solid-Bau **einer** geladenen Wand, schlägt das Öffnen der gesamten Datei fehl
  (Meldung, alter Stand bleibt); derselbe Pfad ist im Query-Zweig bewusst total (`wallMesh` fängt
  E-GEO-002 und liefert `nullopt`), und dieselbe Datei lässt sich über die CLI (`--open`) weiterhin laden
  und exportieren. Das Fehlerbild ist in der neuen Spec-Notiz nicht genannt und in keinem Test belegt.
- **verifizierbar:** ja — Testlauf mit einem werfenden `GeometryKernelPort`-Double wäre das Orakel;
  existiert nicht.

---

## LOW

### LOW-1 — Teil-Auflösung ohne Rückmeldung in `reresolve_after_open`

- **kategorie:** LOW · **quelle:** Maintainability · **pfad:** `src/main.cpp`:260–279
- **befund:** Beide frühen `return`s (leeres Geschoss-Set; `addLayer` liefert `nullopt`) lassen
  `guide_sink` **und** `canvas` auf den Ids des alten Modells stehen — genau der Zustand, den HIGH-3
  beseitigen sollte —, ohne dass der Benutzer davon erfährt.
- **verifizierbar:** nein (kein Sensor; Lesen des Composition-Roots).

### LOW-2 — `effectiveUpdates()` zählt den Voll-Neuaufbau nicht mit

- **kategorie:** LOW · **quelle:** [ADR-0009](../plan/adr/0009-gui-framework-qt6.md) (f)
  („Szenen-Zustand + Zähler wirksamer Szenen-Updates" als Darstellungs-Surrogat) ·
  **pfad:** `src/adapters/ui/view/viewer_scene.cpp`:86–93 gegen `src/adapters/ui/view/viewer_scene.h`:66–68
- **befund:** Der `ModelReplaced`-Zweig ruft `loadAll()`, erhöht aber `effective_updates_` nicht, obwohl
  der Accessor als „Netz ersetzt/hinzugefügt/entfernt" dokumentiert ist; der dokumentierte Surrogat-Zähler
  bleibt ausgerechnet bei der größten Szenen-Änderung unverändert.
- **verifizierbar:** ja — `effectiveUpdates()` vor/nach `openProject` im vorhandenen Adapter-Test.

### LOW-3 — Der Plan-Kopf widerspricht dem im selben Commit abgehakten DoD

- **kategorie:** LOW · **quelle:** Doku-Drift ·
  **pfad:** `docs/plan/planning/done/slice-047-projekt-oeffnen.md`:4, :12–21
- **befund:** Alle 047b-DoD-Haken sind gesetzt, während Kopf-Frontmatter (`status: open`) und Kopftext
  („047b offen", „… dann startbar", „Der Umbrella bleibt in `open/`") den Vorzustand beschreiben; die
  Datei liegt seit `daa7ab8` in `in-progress/`.
- **verifizierbar:** ja — Lesen der Datei; das `planning`-Gate prüft nur den Ruhe-Marker (MR-017), nicht
  Status vs. Verzeichnis.

### LOW-4 — „Speichern unter" ohne Default-Suffix

- **kategorie:** LOW · **quelle:** Maintainability/UX-Rand · **pfad:** `src/main.cpp`:308–310
- **befund:** `QFileDialog::getSaveFileName` wird ohne Default-Suffix aufgerufen; tippt der Benutzer keine
  Endung, entsteht eine Datei ohne `.bcad`, die der Öffnen-Dialog mit seinem Vorgabefilter
  `b-cad-Projekt (*.bcad)` nicht mehr anzeigt.
- **verifizierbar:** nein (GUI-Interaktion, coverage-ausgenommen).

### LOW-5 — Doppelter Datei-Kopf im Kern-Test

- **kategorie:** LOW · **quelle:** Maintainability · **pfad:** `tests/hexagon/test_manage_project.cpp`:1–14
- **befund:** Die Datei trägt jetzt zwei einleitende Beschreibungs-Absätze; der zweite („Save-Use-Case
  (slice-047a) …") beschreibt nur noch den halben Inhalt der Datei.
- **verifizierbar:** nein.

### LOW-6 — Zusätzliche Leerzeile vor `int main`

- **kategorie:** LOW · **quelle:** Maintainability · **pfad:** `src/main.cpp`:326–327
- **befund:** Der Diff fügt nach `}  // namespace` eine zweite Leerzeile ein — folgenloses Diff-Rauschen
  (clang-format/lint schlagen nicht an, `make lint` ist grün).
- **verifizierbar:** ja — `make lint` (grün, also kein Gate-Bezug).

---

## INFO

1. **Kein Review-Artefakt zu slice-047.** In `docs/reviews/` existiert weder ein Plan- noch ein
   Code-Review zu 047/047a/047b; die Kopf-Behauptung des Plans („MR-006 2026-07-24 — 3 HIGH / 2 MED")
   ist damit unbelegt und war für diesen Lauf nicht nachprüfbar — die fünf Punkte wurden deshalb
   ausschließlich am Code geprüft (Ergebnis: HIGH-1/HIGH-2/HIGH-3 sind im Code umgesetzt, ihre Absicherung
   siehe MEDIUM-1/2/3/7; MED-1/MED-2 sind umgesetzt). Zuständig: Autor/Prozess — ein ungetrackter Plan
   `slice-051-review-artefakt-pflicht` adressiert genau diese Lücke.
2. **ACC-002-Beleg-Pfad.** `installFileMenu` läuft auch im `--acc-002-beleg`-Zweig, das Fenster trägt beim
   Grab jetzt eine Menüleiste; das committete Beleg-Artefakt (1280×800) stammt aus slice-012 und wurde
   seither weder nach der Tab-Umstellung (slice-043) noch hier neu erzeugt. Bewertung gehört zum manuellen
   Abnahme-Schritt (kein Gate).
3. **Honesty-Nachlauf.** Die Commit-Message-Behauptung „make gates EXIT=0, 274/274 Tests, Coverage 91,2 %"
   ist reproduziert worden (eigener Lauf, identische Werte) — kein Befund.
4. **DoD-Haken-Buchhaltung.** Die Bewertung der gesetzten Häkchen ist Verifier-Aufgabe (Modul 11); hier
   werden sie nur als Beleg für MEDIUM-6 und LOW-3 zitiert.

---

## Negativbefunde (geprüft, ohne Befund)

- **Beobachter-Vollständigkeit (`ModelChangedPort`).** Alle drei Produktions-Implementierungen geprüft:
  `ViewerScene` (neuer `loadAll()`-Fall), `ViewerWidget` (delegiert an seine eigene `ViewerScene` und
  plant Repaint) und `CanvasWidget` (op-agnostisch, pullt `planView()`) — **kein** Beobachter bleibt bei
  `ModelReplaced` stale. Der `switch` in `viewer_scene.cpp` hat kein `default:`, ein künftiger `op` bricht
  daher den Build statt still durchzufallen. Die übrigen Treffer sind Test-Doubles.
- **Transaktionalität von `replaceBuilding` (Code-Pfad).** Trial-Solids entstehen vollständig vor
  `building_ = std::move(building)`; `buildWallSolid` liest ausschließlich die **übergebenen** Wand-/
  Öffnungs-Listen und `geometry_` (kein Rückgriff auf `building_`, also keine Vermischung von altem und
  neuem Stand); die Post-Commit-Schritte sind total (`detectRooms` ist als „TOTAL, wirft kein E-GEO-002"
  spezifiziert, Zähler-Zuweisungen sind arithmetisch, `notifyListeners` kapselt werfende Beobachter).
  Ein Pfad, der ein **teilweise** ersetztes Modell hinterlässt, wurde nicht gefunden. (Die *Absicherung*
  dieser Eigenschaft ist MEDIUM-3, nicht die Eigenschaft selbst.)
- **Id-Zähler-Reset (Vollständigkeit + Semantik).** Alle neun `next_*_id_`-Member des Service sind
  abgedeckt (Abgleich gegen `structure_edit_service.h`:278–286); jeder wird auf `max(id über die
  zugehörige geladene Liste) + 1` gesetzt, leere Liste → 1. Alle neun Element-Arten hängen als
  Top-Level-Vektoren am `Building` (also ist die Maximums-Menge die richtige), Räume tragen keinen
  vergebenen Zähler. Das mögliche **Herabsetzen** eines Zählers ist korrekt, weil der alte Stand
  vollständig verworfen wird. Der SQLite-Loader stellt die persistierten Ids original wieder her —
  der Reset ist also tatsächlich nötig.
- **Fehlerbehandlung des GUI-Pfads.** Abbruch im Datei-Dialog (leerer Pfad) → sofortiges `return`, kein
  Zustandswechsel; Wurf im Öffnen-Handler → `QMessageBox::critical`, Modell nachweislich unverändert
  (`openProject` lädt erst, ersetzt danach); Wurf im Speichern-Handler ebenso; kein Fensterwechsel/Titel
  bei Fehlschlag. Beide `catch`-Zweige fangen `const std::exception&`, was die neutralen
  `std::runtime_error`-Würfe von Repository und Service abdeckt.
- **Hexagonale Richtung / Port-Grenzen (ADR-0001/0008/0009/0019).** Kein neuer Adapter→Kern-Include:
  `manage_project.h → structure_edit_service.h` ist `services → services`; `main.cpp` ist der in
  `.a-check.yml` deklarierte `composition_root`; `ui/view` importiert weiterhin nur `ports/driven`
  (erlaubte ADR-0009-(d)-Import-Regel), `ui/command` bleibt unverändert port-frei zum Canvas
  (Option-A-Verdrahtung von slice-043 unangetastet). `make a-check` und `make arch-check` grün.
- **Re-Entranz-Verbot (ADR-0008 #6).** Kein Beobachter-Callback löst eine Mutation aus; die einzige
  Mutation nach dem Laden (`reresolve_after_open`) läuft im Handler **nach** `openProject`, nicht im
  Callback (inhaltlich siehe MEDIUM-8).
- **Persistenz-Atomarität (AGENTS §2.2).** Kein neuer Schreibpfad; der GUI-Save geht über den
  unveränderten `services::saveProject` → `repository.save` (Temp+Rename); `schema.sql`/`data-model.yaml`
  unberührt.
- **Suppression-Verbot (§2.4) und ADR-Immutabilität (§2.5).** Keine `NOLINT`/`#pragma`-Suppression im
  Diff; keine ADR-Datei angefasst.
- **MR-008 (Lösungsfreiheit).** `spec/lastenheft.md` ist unverändert; die Lösungsmechanik (`--save`/
  `--open`, Datei-Menü) steht in `spec/spezifikation.md` — regelkonform (Einordnung des Absatzes s. MEDIUM-5).
- **Diskriminierung der beiden Adapter-Tests.** Der Roundtrip-Test startet bewusst mit drei Wänden gegen
  zwei geladene und prüft zusätzlich, dass das Netz der Alt-Wand verschwindet — er würde ohne den
  `ModelReplaced`-Fall rot (die Zusicherung ist also diskriminierend, anders als MEDIUM-1/2/3);
  der Fehlerfall-Test hält Wand- und Netz-Zahl fest. Beide laufen über echten OCC-Adapter und echte
  SQLite-Datei und räumen ihre Temp-Dateien ab.
- **Test-Registrierung und Gate-Lauf.** `tests/CMakeLists.txt` führt die neue Datei; `make gates`
  reproduziert grün (274/274, 91,2 % Zeilen) — die Test-Anzahl der Commit-Message stimmt.
- **Werkzeugbindung (§2.3/§2.9).** Dieser Lauf hat ausschließlich `make`-Targets und `git` benutzt; die
  lokalen Experimente wurden mit `git checkout --` zurückgenommen, `git status` ist wieder auf dem
  Ausgangsstand (nur die beiden vorgefundenen gestagten Dateien plus dieser Report).

---

## Kategorie-Summary

| Kategorie | Anzahl |
|---|---|
| HIGH | 1 |
| MEDIUM | 10 |
| LOW | 6 |
| INFO | 4 |

## Verdikt

**1 HIGH → blockierend.** Der eigentliche Mechanismus trägt: `replaceBuilding` ist im Code-Pfad
transaktional, der Zähler-Reset ist vollständig und semantisch richtig, `ModelReplaced` erreicht jeden
Produktions-Beobachter, und die GUI-Fehlerbehandlung lässt das Modell in allen geprüften Fällen konsistent.
Blockierend ist **HIGH-1**: die in diesem Commit in eine verbindliche Quelle geschriebene Aussage, CLI und
GUI nutzten denselben Kern-Use-Case, ist am Code widerlegt — der CLI-Öffnen-Pfad geht an Naht, Zähler-Reset
und Meldung vorbei.

Der zweite Schwerpunkt ist die **Sensor-Lage**: drei der vier neuen Kern-Zusagen (Raum-Neuerkennung,
sieben von neun Zählern, Transaktionalität) sind empirisch als **nicht diskriminierend** nachgewiesen —
man kann sie zurücknehmen, ohne dass `make test` es merkt —, und die HIGH-3-Auflösung hat außerhalb der
coverage-ausgenommenen `main.cpp` überhaupt keinen Aufrufer. Die Doku-Seite (CHANGELOG, §1-Nachzug für den
neuen `op`, Ablage des Spec-Absatzes) ist der dritte Block.
