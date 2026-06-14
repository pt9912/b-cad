# ADR-0012: Evaluations-Architektur (Auswertungs-Port, read-only-Ableitung)

**Status:** Accepted

**Datum:** 2026-06-14

**Autor:** Dietmar Burkard (slice-017a, ausgearbeitet im AI-Harness-Lauf)

**Bezug:** LH-FA-EVL-001..006 (Flächen/Volumen/Wohnfläche/Material-/Tür-/
Fensterlisten), ADR-0001 (Kern führt; Driving-Port = Use-Case-Schnittstelle),
ADR-0006 (`materials`-Schema + `material_id`-FKs — Material-Persistenz bereits
entschieden), ADR-0007 (Raumerkennung, **Netto-Fläche** — Quelle für
EVL-001/003), ADR-0011 (Öffnungs-Schnittprismen — Grundlage des Netto-Volumens)

---

## Kontext

welle-3-auswertung macht das Gebäudemodell **auswertbar** (Meilenstein M3):
Flächen-, Volumen-, Wohnflächenberechnung (LH-FA-EVL-001..003) und Material-/
Tür-/Fensterlisten (LH-FA-EVL-004..006). Anders als welle-2 erzeugt Auswertung
**keine** Geometrie — sie **leitet** Information aus dem bereits committeten
Modell ab. Vor der Implementierung (slice-017b ff.) sind drei Lösungsfragen
offen, die der reine Spec-Text nicht entscheidet:

1. **Port-Einordnung:** Über welche Schnittstelle wird ausgewertet? Ein neuer
   Driving-Port, oder die Erweiterung des bestehenden `DetectRoomsPort`
   (heute nur `rooms(storey)`)? `architecture.md` §1.1 ordnet EVL-001..003
   bereits dem `DetectRoomsPort` zu — ein Hinweis, kein Zwang.
2. **Flächen- UND Volumen-Fundament + Schicht:** Worauf rechnet die
   Auswertung, und **in welcher Schicht**? Eine OCC-Volumenmessung (`GProp`)
   läge im Geometrie-Adapter (driven); eine analytische Rechnung läge im Kern.
   Die Wahl entscheidet, ob Auswertung eine **reine Kern-Query** bleibt.
3. **Reaktivität:** Ist eine Auswertung ein reaktives Szenen-Element
   (`…Changed`-`op`, ADR-0008) oder ein pull-on-demand abgeleiteter Bericht?

**Nicht offen** (bewusst außerhalb dieser ADR — Scope-Verengung, slice-017a
Plan-Review HIGH-1): Der **Material-Persistenz-Vertrag** liegt in ADR-0006
(`materials` + `material_id`-FKs an `walls`/`roofs`/`slabs`/`wall_types`). Die
**Material-Domänen- und Zuweisungs-Autorität** (eigenes `material_id` vs. über
`wall_type`) ist eine **Spezifikations-Entscheidung** (LH-FA-MAT-003.a,
`spezifikation.md` §2.1), **kein** ADR-Gewicht — Material erscheint in dieser
ADR nur als **von der Auswertung konsumierte Eingabe**. (Eine eigene
Material-Architektur-ADR wäre ein irreführend gebündeltes Leitplanken-ADR —
Präzedenz slice-009-Split.)

## Entscheidung

1. **Auswertungs-Port — neuer Driving-Port `EvaluatePort` (read-only Query).**
   EVL-001..006 werden über einen **neuen** `EvaluatePort` (`src/hexagon/ports/
   driving/`) bedient, **nicht** über eine Erweiterung von `DetectRoomsPort`.
   Begründung: Auswertung **aggregiert modellweit** (über Räume, Bauteile,
   Material hinweg), während `DetectRoomsPort` eine **raum-spezifische** Query
   ist (`rooms(storey)`). Die `Room`-Netto-Fläche (ADR-0007) ist eine **Quelle**
   für EVL-001/003, nicht der Port. Wie bei `DetectRoomsPort` ist `EvaluatePort`
   eine **reine Abfrage** — vom `StructureEditService` (oder einem eigenen
   Auswertungs-Service) implementiert; die exakte Methoden-Signatur legt
   slice-017b fest (ADR-0001-Kern-Hoheit).

2. **Ergebnis-Modell — pure Werttypen.** Auswertungs-Ergebnisse sind
   **framework-freie Werttypen** in `src/hexagon/model/` (z. B. `AreaReport`,
   `VolumeReport`, `MaterialLine` — exakte Form in slice-017b/spez. §2.1); kein
   OCC-/Qt-/SQLite-Typ, read-only. Die Sicht/UI konsumiert sie wie die
   `TriangleMesh`-Netze: über den Port, ohne Adapter-Typ-Leck.

3. **Read-only / Pull — keine Mutation, keine Persistenz, kein `op`.**
   Auswertung **mutiert das Modell nie** und wird **on-demand** aus dem
   committeten Stand abgeleitet (wie `DetectRoomsPort`). **Keine eigene
   Persistenz** (Auswertungen werden nicht gespeichert, sondern bei Bedarf neu
   berechnet) und **kein neuer `ModelChangedPort`-`op`** — eine Auswertung ist
   ein **abgeleiteter Bericht**, kein gerendertes Szenen-Element (Pull-on-demand,
   nicht reaktiv).

4. **Flächen- und Volumen-Fundament — analytisch im Kern (getrennte Polygone).**
   **Fläche (EVL-001/003)** via **Shoelace auf dem Raum-Netto-Polygon**:
   ADR-0007-**Innenkanten-Ring minus Loch-Ringe** — die Netto-Flächen-Definition
   wird **wiederverwendet** (keine zweite Semantik, keine Doppelzählung). Dies
   ist das „auf der Footprint-Fläche/Shoelace, nicht Länge·Stärke"-Fundament
   (`spezifikation.md` §1 ROM-001.a; ROM-002/003 sind die Per-Raum-Quelle, EVL
   aggregiert).
   **Volumen (EVL-002)** als **Bauteil-Netto-Volumen analytisch im Kern** —
   Dach/Platte/Treppe aus ihrem analytischen Solid. **Wand** =
   Bauteil-Footprint-Fläche (Shoelace) · Höhe **minus** das **real entfernte,
   geklemmte Öffnungsvolumen** je Öffnung (`width · thickness · clamped_height`,
   `clamped_height = min(sill+height, Wandhöhe) − max(0, sill)`) — **NICHT** das
   überstands-behaftete Roh-Schnittprisma (das per `OPENING_CUT_OVERSHOOT_MM`
   lateral und in der Höhe übersteht und **mehr** als das real subtrahierte
   Volumen umfasst — `opening_geometry.cpp`, `spezifikation.md` §1 DOR-004.a;
   die Schnittprismen sind das **Boolean-Werkzeug**, nicht das Volumen-Maß).
   **Eck-Näherung (welle-3, bewusst benannt):** die Summe der Wand-Volumina
   **doppelzählt** den Miter-Sporn endpunkt-verbundener Wände (Grad-2-Eckkeil,
   slice-012-Footprint) — eine **kleine Über-Zählung**, in welle-3 in Kauf
   genommen; das **exakte** vereinigte Volumen (Footprint-Union je Geschoss) ist
   **Re-Eval-Trigger**, parallel zum WAL-006-Vollumfang (Schnittpunkte/T-Stöße).
   **Bewusst KEIN OCC-`GProp` im Geometrie-Adapter** — eine driven-Volumenmessung
   würde die „reine Kern-Query"-Eigenschaft (#1/#3) brechen (Round-Trip durch den
   Adapter für eine read-only-Ableitung). Die Auswertung bleibt **eine reine
   Kern-Query ohne `GeometryKernelPort`-Aufruf**.

5. **Material als konsumierte Eingabe.** EVL-004 (Materiallisten) gruppiert/
   summiert über die **material-tragenden Bauteile** (`material_id`-FK); in
   welle-3 sind das **`walls`/`roofs`/`slabs`** — `stairs`/`openings`/`doors`/
   `windows` tragen **kein** `material_id` (Schema) und sind eine **benannte
   Lücke/späterer Ausbau** (`spezifikation.md`, nicht Lastenheft).
   **`windows.frame_material` ist Freitext (`TEXT`, kein `materials`-FK)** und
   gehört **nicht** in die EVL-004-Material-FK-Aggregation — sonst vermengte die
   Materialliste zwei inkonsistente Quellen (FK + Freitext); das Fenster-
   Rahmenmaterial ist Teil des benannten Späterausbaus. Das Material-Modell
   selbst (Werttyp + Auflösungs-/Zuweisungs-Regel) entscheidet die Spezifikation
   (siehe Kontext), nicht diese ADR.

## Verglichene Alternativen

### Port-Einordnung (Entscheidung #1)

**Option A — neuer `EvaluatePort` (gewählt).** Pro: kohärente Verantwortung
(modellweite Auswertung); `DetectRoomsPort` bleibt schlank/raum-spezifisch;
EVL-004..006 (Listen) haben keinen Raum-Bezug und passten ohnehin nicht in
`rooms()`. Contra: ein weiterer Driving-Port.

**Option B — `DetectRoomsPort` erweitern.** Pro: die `architecture.md`-
Zuordnung EVL-001..003 → `DetectRoomsPort` wäre wörtlich erfüllt; ein Port
weniger. Contra: überlädt einen raum-spezifischen Port mit modellweiter
Aggregation; die Listen (EVL-004..006) sprengen die `rooms()`-Semantik; höhere
Cognitive-Complexity einer Misch-Schnittstelle.

### Volumen-Schicht (Entscheidung #4)

**Option A — analytisches Netto-Volumen im Kern (gewählt).** Pro: hält
Auswertung als **reine Kern-Query** (kein driven-Round-Trip für eine read-only-
Ableitung); konsistent mit der analytischen Geometrie von Dach/Platte/Treppe;
das Wand-Netto-Volumen ist deterministisch (Footprint·Höhe − Prismen) ohne
OCC. Contra: für künftige nicht-prismatische/gekrümmte Solids müsste die
analytische Formel erweitert werden (Re-Eval-Trigger).

**Option B — OCC-`GProp`-Volumenmessung im Geometrie-Adapter (driven).** Pro:
misst beliebige Solids exakt, auch komplexe Booleans. Contra: verlegt eine
**read-only-Auswertung** in den Geometrie-Adapter und macht aus einer Kern-
Query einen driven-Round-Trip — bricht die „Auswertung = reine Ableitung"-
Rahmung (#1/#3); OCC-Abhängigkeit für eine reine Zahl; schlechter testbar
(braucht den echten Adapter statt analytischer Orakel).

### Reaktivität (Entscheidung #3)

**Option A — Pull-on-demand, kein `op` (gewählt).** Pro: eine Auswertung ist
ein abgeleiteter Bericht, kein Szenen-Element — ein `op` ohne gerendertes
Korrelat wäre Vokabular ohne Beobachtungs-Bezug (vgl. ADR-0011 #4-Argument);
der Konsument pullt bei Bedarf (Muster `DetectRoomsPort`). Contra: ein
künftiges Live-Dashboard müsste pollen/selbst neu pullen.

**Option B — reaktiver `EvaluationChanged`-`op`.** Pro: Live-Aktualisierung
ohne Pull. Contra: jede Modell-Mutation müsste eine modellweite Neu-Auswertung
auslösen (teuer, ohne sichtbares Szenen-Korrelat); Vokabular-Zuwachs ohne
welle-3-Bedarf.

## Konsequenzen

- **Positiv:** Auswertung fügt sich **ohne Schema-Änderung, ohne neue
  Notification-Vokabel und ohne OCC-Abhängigkeit** in die bestehenden Verträge
  (ADR-0001-Schichtung, ADR-0007-Netto-Fläche, ADR-0011-Schnittprismen); sie
  bleibt eine **reine, analytisch testbare Kern-Query** (analytische Orakel,
  keine Adapter im Testpfad).
- **Folgepflicht (slice-017a):** **`architecture.md` §1.1 nachziehen** —
  EVL-001..006 wandern von der heutigen `DetectRoomsPort`-Zuordnung (§1.1,
  Z. 78) auf den neuen Driving-Port `EvaluatePort`; sonst widerspräche die
  Architektur-Doku dauerhaft dieser Leitplanke.
- **Folgepflicht (slice-017b ff.):** `EvaluatePort` (Driving) + Ergebnis-
  Werttypen; Auswertungs-Service mit **Shoelace-Netto-Fläche** (Raum-Polygon),
  **analytischem Netto-Volumen** (Bauteil-Footprint·Höhe − **geklemmtes
  Öffnungsvolumen**, #4) und **Listen-Aggregation**; AK-Tests mit `LH-FA-EVL-*`
  im Namen. **MR-009 greift für den EVL-Impl-Slice** — die Shoelace-Netto-/
  Loch-Subtraktion ist geometrie-korrektheits-nah (Winding/Orientierung, keine
  Doppelzählung), also unabhängiges geometrielastiges Code-Review vor
  Welle-Closure.
- **Material getrennt:** der Material-Domänentyp + die Zuweisungs-Autorität
  bleiben Spezifikations-/ADR-0006-Sache (slice-017a-Spec / Material-Strang);
  diese ADR konsumiert Material nur.
- **ADR-0006/0007/0011 bleiben unverändert gültig** — diese ADR baut auf ihnen
  auf, ändert sie nicht.
- **Offen (bewusst):** material-tragende Erweiterung auf
  `stairs`/`openings`/`windows`; nicht-prismatische Volumen; reaktive
  Auswertung — alles Re-Eval-Trigger.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| AK-Tests (slice-017b) | EVL Happy/Boundary/Negative gegen **analytische** Werte (Shoelace-Netto-Fläche = äußerer Ring − Löcher; Netto-Volumen = Footprint·Höhe − Öffnungs-Prismen; leeres Modell → Null/leer, kein Wurf) | `make test` |
| Schichtung | `EvaluatePort`/Auswertungs-Service bleiben **pure Domäne** — **kein** OCC-/Qt-/SQLite-Typ, **kein `GeometryKernelPort`-Aufruf** für Volumen (analytisch im Kern); kein Adapter-Leck | `make arch-check` (ADR-0001) |
| Geometrie-Korrektheit | EVL-Impl (Shoelace-Winding, Loch-Subtraktion, Netto-Volumen) unabhängig code-reviewt vor Welle-Closure | MR-009 (`harness/conventions.md`) |

## Re-Evaluierungs-Trigger

- **Material-tragende Bauteile erweitern** (`stairs`/`openings`/`windows`
  brauchen `material_id`) → ADR-0006-Schema-Schärfung + EVL-004-Abdeckung neu
  bewerten.
- **Nicht-prismatische/komplexe Volumen** (gekrümmte Solids, Boolean-
  Verschneidungen, die nicht Footprint·Höhe−Prismen sind) → analytisches
  Netto-Volumen vs. OCC-`GProp` (Option B) neu abwägen (dann Supersedes/Folge-
  ADR, kein stiller Adapter-Round-Trip).
- **Exaktes vereinigtes Volumen** (Eck-Überlappung endpunkt-verbundener Wände,
  Grad-≥3-Knoten / WAL-006-Vollumfang) → die welle-3-Eck-Näherung (#4,
  Miter-Sporn-Doppelzählung) durch ein **Footprint-Union je Geschoss** ersetzen.
- **Reaktive Auswertung** (Live-Dashboard, das Pull nicht trägt) →
  `EvaluationChanged`-`op` neu bewerten.
- **Auswertung mit eigener Persistenz** (gespeicherte Berichte/Snapshots) →
  read-only-/Pull-Entscheidung (#3) gegen einen Persistenz-Vertrag prüfen.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-14 | Proposed (slice-017a, ADR + EVL/MAT-AK-Schärfung) | slice-017a |
| 2026-06-14 | **Accepted** — unabhängiger Text-Review vor Accept (1 HIGH + 2 MED + 2 LOW eingearbeitet): HIGH-1 Netto-Volumen geklemmtes reales Öffnungsvolumen statt Roh-Prisma + benannte Eck-Näherung (Miter-Doppelzählung, Re-Eval-Trigger); MED-1 `architecture.md`-Nachzug (EVL → `EvaluatePort`); MED-2 `windows.frame_material`-Freitext aus EVL-004 ausgenommen; Accept durch Projektinhaber | slice-017a |
