---
id: slice-014a
titel: Dach — AK-Schärfung LH-FA-ROF-* & Spec-Geometrie (Teilumfang Rechteck-Grundriss)
status: done
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-ROF-001, LH-FA-ROF-002, LH-FA-ROF-003, LH-FA-ROF-004, LH-FA-ROF-005]
adr_refs: [ADR-0001, ADR-0002, ADR-0006, ADR-0011]
---

# Slice 014a: Dach — AK-Schärfung & Spec-Geometrie

**Status:** done (2026-06-13). MR-006-Plan-Review gelaufen (keine HIGH,
3 MED/3 LOW eingearbeitet); DoD vollständig, `make gates` grün.
Closure-Notiz §8.

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-014a-plan.md) — keine HIGH
(„kein ADR" durch ADR-0011 #6 + slice-013a §6 gedeckt); MED-1 (§1-Block-
Benennung als Sammelblock), MED-2 (Grundriss-Herkunft als §1-/DoD-Zeile),
MED-3 (Split-Punkt), LOW-1 (`height_mm` abgeleitet), LOW-2 (Walm-
Einrückbetrag deterministisch) eingearbeitet.

**Welle:** welle-2-bauteile (vierter Slice; erster der Dach-Familie).

**Bezug:** LH-FA-ROF-001..005 (im Lastenheft bisher **Outline**).
**Parametrisiert auf ADR-0011 (#6, Bauteil-Erweiterungs-Muster)** —
das Dach ist ein neuer parametrischer Bauteil-Typ und braucht **keine
neue Grundsatz-ADR**; seine **Geometrie-Entscheidung lebt in Lastenheft-AK
+ `spezifikation.md` §1** (Muster slice-012-Eckenschluss/slice-009a).
ADR-0002 (OCC-Backend für das Solid), ADR-0006 (`roofs`-Schema **liegt
vor**), ADR-0001 (Kern führt, Port-Signatur = Kern-Hoheit).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des Dach-Strangs
(Muster 009a/010a/013a): die Dach-Geometrie (Sattel/Walm/Pult, Neigung,
Überstand) braucht **prüfbare AK + einen entschiedenen Konstruktions-
Algorithmus**, bevor implementiert wird (slice-014b). Reine
Doku/Entscheidung, kein Code.

---

## 1. Ziel

Die Dach-Anforderungen LH-FA-ROF-001..005 bekommen **lösungsfreie,
benutzer-beobachtbare Akzeptanzkriterien** und einen **entschiedenen
Geometrie-Algorithmus** (`spezifikation.md` §1), bevor implementiert
wird. **Reifephase-Teilumfang welle-2: rechteckiger Grundriss** —
Sattel-, Walm- und Pultdach über einem **rechteckigen** Dach-Grundriss
mit parametrischer Neigung und Überstand. Komplexe (L-/U-förmige)
Polygon-Grundrisse bleiben ausdrücklich offen (späterer Vollumfang,
analog WAL-006-Teilumfang).

## 2. Definition of Done

- [x] **Lastenheft LH-FA-ROF-001..005 von Outline auf AK-Niveau
      geschärft** (Reifephase-Klausel; **lösungsfrei, benutzer-
      beobachtbar** — keine Konstruktions-Mechanik im Lastenheft-Text,
      die gehört in §1/DoD-2): je Happy/Boundary/Negative. Mindestens:
      **Pultdach** (ROF-003: eine geneigte Dachfläche über dem Grundriss);
      **Satteldach** (ROF-001: zwei geneigte Flächen treffen sich am
      First); **Walmdach** (ROF-002: an allen Seiten geneigte Flächen,
      First kürzer als der Grundriss); **Neigung** (ROF-004: parametrisch,
      Bereich in §3; steilere Neigung → höherer First, in 3D sichtbar);
      **Überstand** (ROF-005: parametrisch; das Dach kragt um den
      Überstand über den Grundriss hinaus). **Teilumfang-Klausel:** gilt
      für einen **rechteckigen** Dach-Grundriss; komplexe Polygon-
      Grundrisse ausdrücklich offen. Boundary: Neigung/Überstand am
      Grenzwert akzeptiert, außerhalb geklemmt + Hinweis (`E-VAL-001`).
      Negative: nicht-rechteckiger/degenerierter Grundriss → kein Dach,
      kein Absturz. + §9-Historie.
- [x] **`spec/spezifikation.md` §1 präzisiert** (Dach-Sammelblock
      `LH-FA-ROF-001.a` — **deckt ROF-001..005** explizit, Muster
      `LH-FA-DOR-004.a`/`WIN-005.a`; MED-1): der Konstruktions-Algorithmus
      über einem rechteckigen Grundriss `b × t` — **§1 legt die
      Grundriss-Herkunft fest** (welle-2: **explizite Rechteck-Parameter
      `b × t`**; Auto-Ableitung aus dem Gebäudeumriss offen — MED-2);
      **Traufrechteck** = Grundriss ringsum um den Überstand vergrößert;
      **Pult** = eine von Traufe zu Traufe geneigte Fläche (Firsthöhe =
      `t_eaves · tan(Neigung)`); **Sattel** = First mittig entlang der
      **längeren** Achse, zwei Flächen (Firsthöhe =
      `(kürzere_eaves/2) · tan(Neigung)`); **Walm** = zusätzlich an den
      Giebelseiten geneigte Flächen, First um einen **deterministisch
      festgelegten Einrückbetrag** kürzer (aus Neigung + halber
      Giebelbreite; LOW-2). **`height_mm` ist die abgeleitete Firsthöhe**
      (nicht eingegeben — konsistent zum nullable `roofs.height_mm`,
      LOW-1). Determinismus + Totalität (degenerierter/nicht-rechteckiger
      Grundriss → kein Solid, kein Wurf in der Query). Fehler-Code-
      Wiederverwendung (`E-VAL-001` Klemmung, `E-GEO-002` mutierende Op).
      + §8-Historie.
- [x] **`spec/spezifikation.md` §3 Konstanten:** `ROOF_PITCH_MIN/MAX_DEG`,
      `ROOF_OVERHANG_MIN/MAX_MM` + Defaults bei Anlage
      (`DEFAULT_ROOF_PITCH_DEG`, `DEFAULT_ROOF_OVERHANG_MM` — konsistent
      mit den `roofs`-Schema-Defaults 30°/500 mm, ADR-0006).
- [x] **Reine Doku/Entscheidung — kein Code, keine Tests, kein ADR**
      (ADR-0011 #6 deckt den Integrations-Pfad; die Dach-Geometrie ist
      eine Spec-Entscheidung). `make gates` grün; Closure-Notiz mit
      Lerneintrag. **Nicht Teil dieser DoD:** slice-014b
      (Domänen-`Roof`-Typ, `GeometryKernelPort`-Dach-Operation,
      `EditStructurePort`-Ops, Viewer, Persistenz `roofs`-Tabelle,
      AK-Tests) und die Welle-Closure.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | ROF-001..005 Outline → AK (Reifephase, Rechteck-Teilumfang); §9-Historie |
| `spec/spezifikation.md` | ändern | §1 `LH-FA-ROF-001.a` Dach-Geometrie-Algorithmus; §3 Neigungs-/Überstands-Konstanten + Defaults; §8-Historie |
| `spec/architecture.md` | ggf. ändern | `GeometryKernelPort`-Zeile um Dach-Solid-Verantwortung, falls die Spec-Entscheidung eine Port-Erweiterung impliziert — sonst Begründung in der Closure (Port-Signatur ist 014b) |
| `docs/reviews/{2026-06-13-slice-014a-plan}.md` | neu | MR-006-Report |

## 4. Trigger

- Keiner — **sofort startbar** nach MR-006-Review: reine
  Doku/Entscheidung; das OCC-Backend (ADR-0002) und das `roofs`-Schema
  (ADR-0006) liegen vor.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-014b**
  (Dach-Implementierung) wird startbar.

## 6. Risiken und offene Punkte

- **Lösungsfreiheit des Lastenhefts (Wiederholungsfall):** „zwei
  geneigte Flächen treffen sich am First" ist benutzer-beobachtbar;
  Trauf-Rechteck/`tan(Neigung)`-Höhenformel sind Mechanik und gehören in
  §1, nicht ins Lastenheft. Der Zähler „Lösung schärft nie das
  Lastenheft" steht bei 3× (welle-1v-results §5) — hier erneut Anlass
  zur Festschreibung (im Review entscheiden).
- **Rechteck-Teilumfang (Reifephase):** der Walmdach-Algorithmus über
  einem **allgemeinen** Polygon ist eine Straight-Skeleton-Berechnung
  (welle-2-überschwer). Der Rechteck-Teilumfang ist analytisch
  geschlossen und genügt ACC-001 („1 Dach"). Die Teilumfang-Klausel
  muss im Lastenheft **explizit** stehen (Muster WAL-006), damit der
  Vollumfang nicht still behauptet wird.
- **Dach-Grundriss-Herkunft:** explizit (Parameter) vs. aus dem
  Geschoss-Wandumriss abgeleitet. Für welle-2 ist die **explizite**
  Rechteck-Vorgabe der bounded-Weg (keine Kopplung an Wand-Mutationen/
  Notification); Auto-Ableitung aus dem Gebäudeumriss ist späterer
  Ausbau. Die genaue Quelle ist eine §1-Spec-Entscheidung dieses Slice
  (lösungsfrei im Lastenheft halten).
- **Port-Mechanik nicht vorab fixiert (Lösungsfreiheit der Ebenen):**
  ob der `GeometryKernelPort` eine eigene Dach-Operation bekommt oder
  der Kern das Dach-Polyeder als Wert liefert, entscheidet **014b** —
  dieser Slice spezifiziert nur die Geometrie (Präzedenz 011-F3/013a).
- **Default-Konsistenz:** §3-Defaults (30°/500 mm) müssen zu den
  `roofs`-Schema-Defaults (ADR-0006) passen — sonst Drift; im Review
  gegen `data-model.yaml` prüfen.
- **Split-Punkt (MED-3):** kippt die Sitzung, ist DoD-1 (lösungsfreie
  Lastenheft-AK) **vorab schließbar** (hängt nicht von der §1-Mechanik
  ab) → DoD-2/3 (§1-Algorithmus + §3-Konstanten) als zweiter Schritt.
- **„Lösung schärft nie das Lastenheft": 4. Anlass in Folge**
  (009a/010a/012/013a). Der Review nennt die Festschreibung „überfällig"
  (INFO-1) — Kandidat für AGENTS §2 / neuer `MR-<NNN>`; in der Closure
  als fälligen Steering-Schritt vermerken (nicht in diesem Doku-Slice
  improvisieren).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; Dichte hoch (AK-Format, Reifephase-Klausel,
  ADR-Schärfungs-Regel MR-001, Wertebereich-/Fehler-Code-Konvention);
  Phase-Reife: ROF Phase 2 (Outline → AK); Risiko niedrig (reine Doku).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; Dichte hoch (ADR-0011-Leitplanke: kein neuer
  Grundsatz-ADR; Reifephase-Teilumfang dokumentiert); Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **Lastenheft 0.1.4:** LH-FA-ROF-001..005 von Outline auf AK-Niveau
  (Sattel/Walm/Pult, Neigung 5–60°, Überstand 0–1500 mm), lösungsfrei/
  benutzer-beobachtbar; **Teilumfang rechteckiger Grundriss** explizit
  (komplexe Polygon-Grundrisse offen, Muster WAL-006); §9-Historie.
- **`spec/spezifikation.md` §1 `LH-FA-ROF-001.a`** (Sammelblock, deckt
  ROF-001..005): Grundriss-Herkunft (explizite Rechteck-Parameter `b×t`,
  welle-2), Traufrechteck aus Überstand, Pult/Sattel/Walm-Konstruktion +
  Höhenformeln, Walm-Einrückbetrag deterministisch, Firsthöhe abgeleitet
  (`roofs.height_mm` nullable), Klemmung/Totalität (`E-VAL-001`/
  `E-GEO-002`). **§3:** `ROOF_PITCH_MIN/MAX_DEG`, `ROOF_OVERHANG_MIN/MAX_MM`
  + Defaults (30°/500 mm, deckungsgleich `roofs`-Schema ADR-0006);
  §8-Historie.
- **Kein ADR, kein Code** (ADR-0011 #6); `architecture.md` bewusst nicht
  geändert (Port-Signatur ist 014b; Lösungsfreiheit der Ebenen).
- **`make gates` grün** (2026-06-13): docs-check über alle geänderten
  Artefakte, gate-consistency, arch-check, lint, Tests, Coverage
  unverändert.

**Lerneintrag / fälliger Steering-Schritt:**

- **„Lösung schärft nie das Lastenheft": 4. Vorkommen in Folge**
  (009a/010a/012/013a → jetzt 014a). Der Plan-Review nennt die
  Festschreibung **„überfällig"** (INFO-1). **Fälliger Steering-Schritt
  (eigener Mini-Slice/Commit, nicht in diesem Doku-Slice improvisiert):**
  als Konvention in AGENTS §2 oder neuer `MR-<NNN>` festschreiben (die
  Trennung „AK benutzer-beobachtbar im Lastenheft, Mechanik in
  Spezifikation"). Bis dahin gilt sie als geübte, im Review erzwungene
  Praxis.
- **Schema-Voraussicht (3. Vorkommen):** das `roofs`-Schema (ADR-0006)
  trug Typ/Neigung/Überstand/Footprint bereits — die §3-Defaults mussten
  nur darauf abgestimmt werden, keine Schema-Schärfung.

**Restrisiko / Nachfolge:** slice-014b (Dach-Implementierung: Domänen-
`Roof`-Typ, `GeometryKernelPort`-Dach-Operation/Polyeder,
`EditStructurePort`-Ops, Viewer, Persistenz `roofs`-Tabelle, AK-Tests)
wird startbar — MR-006-Review davor. Walm-Algorithmus über **allgemeine
Polygone** (Straight-Skeleton) bleibt Vollumfang einer späteren Welle.
