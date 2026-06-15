---
id: slice-012
titel: Eckenschluss endpunkt-verbundener Wände (LH-FA-WAL-006-Teilumfang)
status: done
welle: welle-1v-viewer (Scope-Erweiterung per Roadmap-Drift-Eintrag 2026-06-12)
lastenheft_refs: [LH-FA-WAL-006, LH-FA-D3-001, LH-FA-D3-002]  # Teilumfang WAL-006; sichtbar im Viewer
adr_refs: [ADR-0001, ADR-0002, ADR-0007, ADR-0008, ADR-0009, ADR-0010]  # 0010: Beleg-Renderweg (W3-Q6)
---

# Slice 012: Eckenschluss endpunkt-verbundener Wände

**Status:** done (2026-06-12).

**Welle:** welle-1v-viewer (Scope-Erweiterung — Auslöser: der
ACC-002-Beleg zeigte offene Außenecken, der Projektinhaber hat die
DoD-4-Abnahme von slice-011b daraufhin zurückgestellt; siehe
Drift-Tabelle der Roadmap).

**Bezug:** LH-FA-WAL-006 „Wand verbinden" (bisher Outline — dieser
Slice schärft und implementiert den **Teilumfang Eckenschluss an
endpunkt-verbundenen Ecken**; Schnittpunkte/T-Stöße bleiben
WAL-006-Vollumfang einer späteren Welle, spez. §1 Schritt 1 bleibt
gültig). ADR-0007 (dieselbe Endpunkt-Knoten-Graphik wie die
Raumerkennung), ADR-0008 §3 (**Mehr-Element-Updates ausdrücklich
nicht verbaut** — genau dieser Fall), ADR-0009 (Darstellungs-Kette).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-12.

**Plan-Review (MR-006):**
[W3-Report](../../../reviews/2026-06-12-slice-012-plan.md) (W3-IDs;
2 HIGH/7 MED/10 LOW nach Dedup) — alle Findings eingearbeitet,
HIGHs vor Implementierungs-Start behoben.

**Schnitt-Herkunft:** Abnahme-Befund am ACC-002-Beleg (slice-011b
DoD-4): Wände berühren sich an Ecken nur, statt zu verschneiden —
an jeder Außenecke fehlt das ½×½-Stärke-Quadrat
(Befund-Grundriss:
[`acc-002-befund-2d-ecken.png`](../done/acc-002-befund-2d-ecken.png)).
Kein Viewer-Fehler (2D-/3D-Darstellung sind deckungsgleich
modell-treu), sondern fehlender WAL-006-Umfang. Kombinierter Spec+Code-Slice (kein
a/b-Split): die Lastenheft-Schärfung ist eng umrissen, die
Mechanik-Entscheidungen sind durch ADR-0007/0008/0009 bereits
gedeckt — es entsteht keine neue ADR (Begründung §6).

---

## 1. Ziel

Endpunkt-verbundene Wände (genau zwei Wände am gemeinsamen Punkt —
der Normalfall jedes Wandzugs) schließen ihre Ecke körperlich: Die
Darstellung zeigt keine Kerbe/kein Loch mehr an Außenecken, 2D wie
3D. Die Footprint-Berechnung wandert dafür vom Geometrie-Adapter in
den **Kern** (nur dort ist das Nachbar-Wissen des Gebäude-Graphen
vorhanden — dieselben Endpunkt-Knoten wie ADR-0007); der
`GeometryKernelPort` extrudiert/tesselliert künftig ein
**Footprint-Polygon** statt es selbst aus der Einzelwand zu raten.

## 2. Definition of Done

- [x] **Lastenheft LH-FA-WAL-006 von Outline auf Teilumfang-AK
      geschärft** (Reifephase-Klausel; lösungsfrei,
      benutzer-beobachtbar): Happy (zwei Wände mit gemeinsamem
      Endpunkt im Winkel **und gleicher Höhe** → geschlossene Ecke
      ohne Kerbe/Loch, in 2D- und 3D-Darstellung), Boundary
      (kollineare Fortsetzung gleicher Stärke → glatter Übergang;
      kollineare Fortsetzung **ungleicher Stärke** → stumpfer Stoß
      ohne Loch (W3-Q7); sehr spitzer Winkel → **die Eck-Geometrie
      ragt höchstens um die größere der beiden Wandstärken über den
      gemeinsamen Endpunkt hinaus**, sonst stumpfes Ende (messbar,
      W3-P3); **ungleiche Wandhöhen** → Ecke geschlossen bis zur
      niedrigeren Wandhöhe, darüber stumpf (W3-P4)),
      Negative/Abgrenzung (≥ 3 Wände am Punkt sowie Berührung ohne
      gemeinsamen Endpunkt → unverändert stumpf; **Vollumfang
      „Schnittpunkte als Knoten" bleibt ausdrücklich offen** und
      wird hier nicht behauptet). + Historie-Zeile.
- [x] **`spec/spezifikation.md` präzisiert:** (a) §1 Footprint-Regel
      (Eck-Konstruktion an Grad-2-Endpunkt-Knoten: Seitenkanten
      benachbarter Wände im Schnittpunkt verbunden — Prinzip
      **analog** der ADR-0007-Innenkante, normativ festgelegt
      **hier**, nicht in der ADR (W3-Q2); Begrenzung + Rückfall
      stumpf); (b) §3 die **Begrenzungs-Formel** als
      Konstanten-Zeile (`WALL_MITER_LIMIT = max(Stärke_A, Stärke_B)`
      über den gemeinsamen Endpunkt hinaus, W3-P3); (c) D3-002.a
      §Umfang: **Folge-Meldung für Nachbar-Wände**
      (`WallGeometryChanged`) als Mehr-Element-Update gemäß
      ADR-0008 §3, mit **festgelegter Reihenfolge**: auslösende
      Wand-Op → Nachbar-Meldungen einzeln → `RoomsChanged`
      (W3-P9); der neue `op`-Wert ist zugleich Span-Vokabular
      (`bcad.geometry.rebuild`, §5 enumeriert nicht — W3-Q5);
      (d) die zwei bestehenden §1-Verweise „… mit der
      Wandverschneidung (LH-FA-WAL-006)" (Schritt 1 + Schritt 3) auf
      **„WAL-006-Vollumfang"** präzisiert — sie bleiben nach dem
      Teilumfang offen (W3-Q3). + §8-Historie.
- [x] **Kern:** `model::Footprint` (Polygon, pure Werte);
      Footprint-Berechnung im `StructureEditService` (Butt-Enden wie
      bisher; Miter an Grad-2-Knoten mit Toleranz
      `GEOMETRY_TOLERANCE_MM`, Begrenzung + Rückfall stumpf —
      **total**, wirft nie); `GeometryKernelPort` auf
      `extrudeFootprint`/`tessellateFootprint` (Polygon + Höhe)
      umgestellt; **transaktionale Garantie bleibt:** neue Solids
      (mutierte Wand + betroffene Nachbarn) werden VOR dem Commit
      berechnet — schlägt eines fehl, bleibt das Modell unverändert.
- [x] **Mehr-Element-Update:** Mutationen, die Nachbar-Footprints
      ändern (Wand-Anlage → Nachbar-Grad 1→2; Stärke-Änderung →
      Miter-Geometrie), rebuilden die betroffenen Nachbarn und melden
      sie einzeln (`WallGeometryChanged`, neues `op` im
      ADR-0008-Vokabular); Höhen-Änderung erzeugt KEINE
      Nachbar-Meldung (Footprint unberührt). Rejected/verworfen
      meldet weiterhin nichts.
- [x] **Viewer folgt:** `ViewerScene` behandelt `WallGeometryChanged`
      wie die übrigen Wand-Ops (Pull + idempotentes Ersetzen).
- [x] **AK-Tests mit `LH-FA-WAL-006` im Namen** (Kern gegen
      analytisches Double mit Shoelace-Volumen + OCC-Adapter +
      Szene): Happy (rechter Winkel → Ecke körperlich geschlossen:
      **Netz-/Footprint-Bounding-Box überdeckt den äußeren Eckpunkt**
      (gemeinsamer Endpunkt ± halbe Stärken) und ein zuvor offener
      Kerben-Punkt liegt im Wand-Footprint, W3-P10 — *kein*
      Volumen-Vergleich: der symmetrische Eckschnitt erhält das
      Einzelwand-Volumen (Trapez, gleiche Mittellinie)), Boundary
      (kollinear gleich/ungleich
      stark; Spitzwinkel → Begrenzung greift, Rückfall stumpf;
      **Grad-Übergang 2→3: dritte Wand an vermiterter Ecke →
      Bestandswände werden stumpf UND gemeldet**, W3-Q8), Negative
      (Grad ≥ 3 → stumpf; Einzelwand unverändert: Volumen =
      Länge·Stärke·Höhe), Folge-Meldung (Stärke-Änderung →
      Nachbar-Szene-Netz aktualisiert; Höhen-Änderung → keine
      Nachbar-Meldung), **Fehlerfall-AK für den Mehr-Element-Pfad**
      (W3-P2, steuerbares Double wirft beim Nachbar-Rebuild →
      Modell, Solids und Szene unverändert, keine Meldung —
      Transaktions-Garantie real gemessen, nicht behauptet).
      **Regressions-Aussage zweigeteilt (W3-P1/Q1):** (i) Tests ohne
      Port-Berührung (Szene-AK, Notifikation, Räume,
      Service-Klemmung) bleiben **textlich unverändert** grün;
      (ii) signatur-migrierte Tests (Doubles,
      `test_occ_geometry_adapter.cpp`) behalten **identische
      Orakel-Werte** (Länge·Stärke·Höhe; Degenerations-Fall wird auf
      degeneriertes Polygon portiert). Belegt per
      `make test`-Output, keine behauptete Testzahl.
- [x] **ACC-002-Beleg regeneriert** (`make acc-002-beleg`):
      geschlossene Außenecken sichtbar (Stand `8fe8dad`, 2D-
      Verifikation committet); Begleit-`.md` aktualisiert;
      **Abnahme durch den Projektinhaber erteilt** (Runde 2,
      2026-06-12 — Abnahme-Satz in `acc-002-beleg.md`).
- [x] `make gates` grün; Closure-Notiz mit Lerneintrag;
      CHANGELOG-Slice-Eintrag (MR-004).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | WAL-006 Outline → Teilumfang-AK (Reifephase-Klausel) |
| `spec/spezifikation.md` | ändern | §1 Footprint-/Eck-Regel + D3-002.a-Umfang (`WallGeometryChanged`) + §8 |
| `src/hexagon/model/{footprint}.h` | neu | Footprint-Polygon (pure Werte) |
| `src/hexagon/ports/driven/geometry_kernel_port.h` | ändern | `extrudeFootprint`/`tessellateFootprint` statt Wand-Rechteck im Adapter |
| `src/hexagon/ports/driven/model_changed_port.h` | ändern | `op`-Vokabular + `WallGeometryChanged` |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Footprint-Berechnung (Miter an Grad-2-Knoten, total), Nachbar-Rebuild + -Meldung, Transaktions-Garantie |
| `src/adapters/geometry/occ_geometry_adapter.{h,cpp}` | ändern | Polygon-Extrusion/-Tessellation (N-Eck statt Rechteck) |
| `src/adapters/ui/viewer_scene.cpp` | ändern | `WallGeometryChanged` behandeln |
| `spec/architecture.md` | ändern | `GeometryKernelPort`-Zeile (§1.2) + Geometrie-Adapter-Zeile auf Footprint-Vertrag — sichere Änderung, nicht „ggf." (W3-P5) |
| `tests/hexagon/analytic_geometry_double.h`, `tests/hexagon/test_structure_edit_service.cpp` | ändern | Doubles auf Footprint-Signatur (Shoelace-Volumen) |
| `tests/adapters/test_occ_geometry_adapter.cpp` | ändern | Portierung auf Footprint-Signatur, identische Orakel-Werte (W3-Q1) |
| `tests/{hexagon,adapters}/{**}` | neu/ändern | WAL-006-AK-Tests (Kern, OCC, Szene, Folge-Meldung, Fehlerfall) |
| `tests/CMakeLists.txt` | ändern | neue Testdateien registrieren (W3-P8) |
| `docs/plan/planning/done/{acc-002-beleg}.{*}` | ändern | Beleg regenerieren + Begleit-.md |
| `docs/plan/planning/done/slice-011b-viewer-adapter-implementierung.md` | ändern | DoD-4-Abnahme-Nachtrag nach regeneriertem Beleg (W3-P11) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Drift-Eintrag + Closure-Trigger-Zeile slice-012 (Scope-Erweiterung) |
| `docs/reviews/{2026-06-12-slice-012-plan}.md` | neu | MR-006-Review-Report (W3-P12) |

## 4. Trigger

- Abnahme-Befund slice-011b DoD-4 ✓ (2026-06-12); unabhängiges
  Plan-Review gemäß MR-006 vor Implementierungs-Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Beleg regeneriert →
  slice-011b-DoD-4-Abnahme wird erneut vorgelegt; Welle-Closure
  `welle-1v-viewer` erst danach (Reihenfolge: 011b done braucht die
  Abnahme, dann Verifikation + `welle-1v-results.md`).

## 6. Risiken und offene Punkte

- **Keine neue ADR — Begründung (präzisiert nach W3-Q2/Q4/P5):** Die
  Eck-Konstruktion folgt dem Offset-Schnitt-Prinzip **analog**
  ADR-0007 (dort normativ nur für die Innenkante entschieden) — die
  neuen Festlegungen (Begrenzung, Rückfall, Eck-Volumen-Zuordnung)
  werden **normativ in Lastenheft-AK + spez. §1/§3** getroffen, dem
  reviewten Entscheidungs-Artefakt dieses kombinierten Slice. Das
  Mehr-Element-Update ist in ADR-0008 §3 **zulässig und mit WAL-006
  namentlich antizipiert** („nicht verbaut"); das neue `op` lebt im
  Code-/Spec-Vokabular, nicht im immutablen ADR-Text. Der
  Port-Signatur-Wechsel berührt **ADR-0002 nicht**: die ADR legt nur
  das OCC-Backend hinter dem Port fest („nur das Backend des
  GeometryKernelPort"), nicht die Port-Signatur — die gehört zu
  ADR-0001-Hoheit des Kerns. Die Darstellungs-Kette ist ADR-0009.
- **Unbedingter Split-Punkt (W3-P7):** **(i)** verhaltensneutrale
  Footprint-Port-Umstellung (Butt-Footprint = bisheriges Rechteck;
  Doubles/OCC-Tests migriert, identische Orakel, `make gates` grün)
  → **(ii)** Miter an Grad-2-Knoten + Nachbar-Rebuild/Meldung +
  LH-/Spec-Schärfung + Beleg. Hälfte (i) ist einzeln prüfbar und
  committbar.
- **WIP-Hinweis:** slice-011b bleibt parallel in `in-progress/` —
  bewusst dokumentierte Ausnahme vom WIP-Limit 1: dort steht
  ausschließlich der manuelle DoD-4-Abnahme-Schritt aus, der von
  diesem Slice abhängt (keine parallele Implementierungs-Arbeit).
- **Port-Signatur-Bruch ist gewollt:** `extrudeWall(Wall)` riet das
  Footprint aus der Einzelwand — mit Nachbar-Wissen ist das die
  falsche Zuständigkeit (Kern kennt den Graphen, ADR-0001-Richtung).
  Alle Implementierer/Doubles werden im Slice mitgezogen; ein
  Kompatibilitäts-Pfad wäre tote Doppelung.
- **Volumen-Semantik:** Miter-Ecken ändern Wand-Volumina an
  verbundenen Ecken (Eck-Dreiecke wandern zwischen den Wänden) —
  Einzelwand-Werte bleiben exakt; die LH-FA-EVL-*-Auswertungen
  (spätere Welle) müssen auf der Footprint-Fläche aufsetzen
  (Shoelace), nicht auf Länge·Stärke (Hinweis in spez. §1).
- **Doppel-Zeichnung derselben Ecke** (zwei Wände identischer Lage)
  und Antiparallel-Fortsetzung: über Miter-Begrenzung + Rückfall
  stumpf abgedeckt (total, kein Wurf).
- **Raumerkennung unberührt:** ADR-0007-Innenkante rechnet weiter auf
  Segment-Achsen + Stärken — Footprint-Änderung betrifft nur die
  Darstellung/Volumina; AK-Tests der ROM-Familie bleiben unverändert
  grün (Sensor dafür: bestehende Suite).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; Dichte hoch (Reifephase-Klausel, „Lösung schärft nie
  das Lastenheft", ADR-0008-Vokabular); Risiko niedrig.

### Sub-Area: Kern (`src/hexagon/`)

- **Modus:** GF; Dichte hoch (framework-frei, Transaktions-Garantie,
  Totalität der Post-Commit-Schritte); Risiko mittel
  (Geometrie-Mathematik — durch analytische AK gedeckt).

### Sub-Area: Adapter (`geometry/`, `ui/`)

- **Modus:** GF; Dichte hoch (Regel C/E, kein OCC/Qt-Leck); Risiko
  niedrig (Polygon statt Rechteck ist im OCC-Adapter ein kleiner
  Schritt).

### Sub-Area: Test-Infrastruktur (`tests/`) (W3-P6)

- **Modus:** GF; Dichte hoch (LH-ID-Namenspflicht, Determinismus,
  gemeinsames Double mit neuem Shoelace-Orakel, Registrierung in
  `tests/CMakeLists.txt`); Risiko mittel (Orakel-Wechsel — durch
  identische Einzelwand-Werte gegengeprüft).

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- 8 AK-Tests mit `LH-FA-WAL-006` im Namen, alle grün (63/63 gesamt):
  Happy (Kerben-Probe per Punkt-im-Polygon, äußerer Eckpunkt auf der
  Kontur, Flächen-Erhalt des symmetrischen Eckschnitts), Boundary
  (kollinear gleich/ungleich stark → stumpf; Spitzwinkel →
  `WALL_MITER_LIMIT` greift; Grad-2→3-Rückbau stumpf + Meldung),
  Negative (Grad ≥ 3, fremdes Geschoss), Folge-Meldung
  (Reihenfolge Op → Nachbar → RoomsChanged; Höhe meldet keinen
  Nachbarn), Fehlerfall-Transaktion (W3-P2: Wurf beim
  Nachbar-Rebuild → Modell/Solids unverändert, keine Meldung) +
  Szene-Nachbar-Folgen (Viewer).
- Regressions-Aussage (W3-P1) eingehalten: Tests ohne Port-Berührung
  textlich unverändert grün; migrierte Tests (Doubles, OCC) mit
  identischen Orakel-Werten; Beleg: `make test` 63/63.
- `make gates` grün (2026-06-12, Commit `8fe8dad`): docs-check
  0 ERROR/WARN, arch-check A–E, lint 0 Befunde, Coverage **94,2 %**
  (846/898).
- Lastenheft 0.1.2 + spez. §1 LH-FA-WAL-006.a/§3/D3-002.a +
  architecture.md-Portzeile nachgezogen; ACC-002-Beleg regeneriert
  (geschlossene Ecken, 2D-Verifikationsbild committet).

**Lerneintrag:**

- **Der Abnahme-Schritt ist ein Sensor:** Der Befund „offene Ecken"
  war in keiner AK-Lücke der Tests, sondern nur am benutzer-
  beobachtbaren Artefakt sichtbar — der manuelle Abnahme-Schritt
  (ADR-0009 (f), bewusst kein Gate) hat damit seinen ersten echten
  Fund geliefert. Bestätigt die R1-H3-Korrektur, den Beleg ernsthaft
  zu konstruieren statt pro forma.
- **Volumen-Intuition geprüft, bevor sie AK wurde:** Der geplante
  Happy-AK „Summen-Volumen wächst" war geometrisch falsch (der
  symmetrische Eckschnitt erhält die Trapez-Fläche) — beim
  Durchrechnen vor der Implementierung entdeckt und durch die
  Kerben-Probe ersetzt. Praxis-Kandidat: *quantitative AK vor
  Implementierungs-Start einmal von Hand durchrechnen* (1. Vorkommen,
  kategorisiert).

**Abnahme:** Runde 2 am 2026-06-12 durch den Projektinhaber erteilt
(Abnahme-Satz in `acc-002-beleg.md`) — DoD vollständig.

**Restrisiko / Nachfolge:** Welle-Closure `welle-1v-viewer` als
separater Schritt mit unabhängiger Verifikation. WAL-006-Vollumfang (Schnittpunkte als
Knoten, T-Stöße, exakte Stufe kollinearer Ungleich-Stärken) bleibt
ausdrücklich offen; LH-FA-EVL-* müssen auf Footprint-Fläche aufsetzen
(spez. §1-Hinweis).
