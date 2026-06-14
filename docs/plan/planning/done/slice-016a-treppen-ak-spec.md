---
id: slice-016a
titel: Treppen — AK-Schärfung LH-FA-STR-* & Spec-Geometrie (Teilumfang gerade einläufige Treppe)
status: in-progress
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-STR-001, LH-FA-STR-002, LH-FA-STR-003, LH-FA-STR-004]
adr_refs: [ADR-0001, ADR-0006, ADR-0009, ADR-0011]
---

# Slice 016a: Treppen — AK-Schärfung & Spec-Geometrie

**Status:** done (2026-06-14). MR-006-Plan-Review gelaufen
([Report](../../../reviews/2026-06-14-slice-016a-plan.md) — **keine HIGH**;
MED-1 `StairChanged`-Begründung, MED-2 STR-004-AK-Wortlaut, MED-3 Historie-
Datum und LOW-1/LOW-2 §3-Werte/`rise`-informativ eingearbeitet); DoD
vollständig, `make gates` grün (reine Doku — docs-check 0 Befunde, Tests
105/105 unverändert). Closure-Notiz §8.

**Welle:** welle-2-bauteile (zehnter Slice; erster der Treppen-Familie,
letztes welle-2-Bauteil).

**Bezug:** LH-FA-STR-001..004 (Treppen), im Lastenheft bisher **reines
Outline** (nur Titel, Z.322–327). **Parametrisiert auf ADR-0011 (#6)** —
neuer Bauteil-Typ, **keine neue Grundsatz-ADR**; Geometrie-Entscheidung in
Lastenheft-AK + `spezifikation.md` §1 (Muster slice-014a/015a). ADR-0006
(`stairs`-Schema **liegt vor**: `from_storey_id`/`to_storey_id` (spannt zwei
Geschosse, `restrict`), `stair_type`, `start_x_mm`/`start_y_mm`, `width_mm`,
`step_count`, `rise_mm`, `tread_mm`). ADR-0009 (analytisches Netz im Kern,
Präzedenz `roof_geometry`), ADR-0001 (Kern führt).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-14.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des Treppen-Strangs
(Muster 014a/015a). Reine Doku/Entscheidung, kein Code.

---

## 1. Ziel

Die vier Treppen-Anforderungen (LH-FA-STR-001..004) bekommen lösungsfreie
AK und einen entschiedenen Geometrie-Algorithmus, bevor implementiert wird
(slice-016b). **Teilumfang welle-2: gerade einläufige Treppe** — das
`stairs`-Schema (ADR-0006) trägt genau die Parameter dafür (Startpunkt,
Breite, Stufenanzahl, Steigung, Auftritt; spannt zwei Geschosse) und **kein**
Podest-/Wendel-/Richtungs-Feld; Podest-/U-/L-/Wendeltreppen bleiben
ausdrücklich **offen** (späterer Vollumfang, analog ROF-Rechteck/SLB-Rechteck).
b-cad ist **keine Statik** (Lastenheft §6) — Steigungs-/Auftritts-/Breiten-
Bereiche sind sinnvolle parametrische Defaults, **keine** normativ erzwungene
Baurecht-Prüfung.

## 2. Definition of Done

- [x] **Lastenheft STR-001..004 von Outline auf AK-Niveau geschärft**
      (Reifephase-Klausel; **lösungsfrei, benutzer-beobachtbar** — keine
      Geometrie-Mechanik im Lastenheft-Text, MR-008): je Happy/Boundary/
      Negative. **Teilumfang-Klausel** (gerade einläufige Treppe; Podest/
      Wendel/Mehrlauf offen). Mindestens:
      **STR-001 Treppe erzeugen** (eine gerade Treppe verbindet zwei
      Geschosse — von der unteren zur oberen Ebene aufsteigend, in 3D als
      Stufenfolge sichtbar; Negative: ohne gültige Zwei-Geschoss-Spanne →
      keine Treppe, kein Absturz); **STR-002 Stufenanzahl** (parametrisch,
      Bereich §3; Boundary geklemmt); **STR-003 Laufbreite** (parametrisch,
      Bereich §3; Boundary geklemmt); **STR-004 Treppengeländer** (ein
      Geländer ist entlang der Treppe auf Handlaufhöhe sichtbar). +
      §9-Historie.
- [x] **Lastenheft-Versions-Drift behoben (Nebenbefund):** der Header
      `**Version:** 0.1.2` (Z.3) wurde von 013a/014a/015a **nicht**
      nachgezogen, während die §9-Historie auf 0.1.5 wuchs (3×-Drift). 016a
      setzt den Header auf **0.1.6** (= neueste Historie-Zeile) und schließt
      den Drift. Lerneintrag in der Closure.
- [x] **`spec/spezifikation.md` §1 präzisiert** (Sammelblock
      `LH-FA-STR-001.a`, deckt STR-001..004): gerade einläufige Treppe als
      **analytisches Stufen-Polyeder im Kern** (Präzedenz `roof_geometry`,
      **nicht** OCC — ADR-0009-Vertrag gewahrt). Festzulegen: (a) die Treppe
      spannt `from_storey` (unten) → `to_storey` (oben); **Gesamtsteigung =
      Geschosshöhe** der unteren Etage; (b) **`rise` abgeleitet** =
      Geschosshöhe / `step_count` (flächenbündiger Anschluss; persistiert als
      `stairs.rise_mm`), `tread` (Auftritt) Parameter, `step_count`/`width`
      Parameter; (c) **Stufen-Konstruktion**: `step_count` Quader, Stufe `i`
      über `x∈[i·tread,(i+1)·tread]`, `y∈[0,width]`, `z∈[0,(i+1)·rise]`
      (solides Stufenprofil, Aufstieg vom Startpunkt); (d) **Aufstiegs-
      richtung** in welle-2 **feste Konvention** (+x ab Startpunkt) — das
      Schema trägt **keine** Richtungs-Spalte, freie Rotation ist offen;
      (e) **`base_z`** = Boden der unteren Etage (welle-2-Ein-Geschoss-
      Annahme = 0, Muster `slab_geometry`); (f) **Geländer (STR-004)** =
      dünnes vertikales Element entlang der Lauf-Seite(n) auf
      `STAIR_RAILING_HEIGHT_MM` über den Stufen — generierte Geometrie; eine
      **persistierte Geländer-Option ist welle-2-offen** (kein Schema-Feld;
      die Sicht rendert das Geländer aus der Treppen-Geometrie). Klemmung/
      Totalität (`step_count`/`width`/`tread` auf §3, `E-VAL-001`;
      degenerierte Spanne/`from==to`/Geschosshöhe ≤ 0 → keine Treppe, Query
      total). **Folge-Meldung** `op = StairChanged` (neuer `op` im
      D3-002.a-Vokabular, ADR-0011 #6; an die untere Etage gebunden); **keine
      `RoomsChanged`**. + §8-Historie.
- [x] **`spec/spezifikation.md` §3 Konstanten:** `STAIR_WIDTH_MIN/MAX_MM`,
      `STAIR_STEP_COUNT_MIN/MAX`, `STAIR_TREAD_MIN/MAX_MM`,
      `STAIR_RISE_MIN/MAX_MM` (abgeleiteter Komfort-Bereich), Defaults
      (`DEFAULT_STAIR_WIDTH_MM`, `DEFAULT_STAIR_STEP_COUNT`,
      `DEFAULT_STAIR_TREAD_MM`), `STAIR_RAILING_HEIGHT_MM` — konsistent mit
      den `stairs`-Schema-Spalten und DIN-nahen Wohnbau-Werten (ohne
      Statik-Anspruch).
- [x] **Reine Doku/Entscheidung — kein Code, kein ADR** (ADR-0011 #6);
      `architecture.md` **nicht** geändert (neuer Bauteil-Typ fügt sich in
      die bestehende Hexagon-Struktur; `ViewModelPort.stairMeshes`/Edit-Ops
      sind 016b). `make gates` grün; Closure-Notiz mit Lerneintrag. **Nicht
      Teil:** slice-016b (Domänen-`Stair`-Typ, `stair_geometry`, Edit-Ops,
      ViewModel/Viewer, `StairChanged`-`op`, AK-Tests) + slice-016c
      (Persistenz `stairs`-Tabelle) + die Welle-2-Closure.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | STR-001..004 Outline → AK + Teilumfang-Klausel; Header-Version 0.1.2 → 0.1.6 (Drift-Fix); §9-Historie-Zeile 0.1.6 |
| `spec/spezifikation.md` | ändern | §1 `LH-FA-STR-001.a` Treppen-Geometrie (gerade einläufig, analytisches Stufen-Polyeder, rise abgeleitet, Geländer, StairChanged); §3 Stair-Konstanten; §8-Historie |
| `docs/reviews/2026-06-14-slice-016a-plan.md` | neu | MR-006-Report |

## 4. Trigger

- Keiner — **sofort startbar** nach MR-006-Review: reine Doku; `stairs`-Schema
  (ADR-0006) und analytisches-Netz-Muster (`roof_geometry`, ADR-0009) liegen
  vor.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-016b**
  (Treppen-Implementierung) startbar.

## 6. Risiken und offene Punkte

- **Lösungsfreiheit Lastenheft (MR-008):** „Treppe verbindet zwei Geschosse,
  Stufenfolge sichtbar", „Geländer sichtbar" sind benutzer-beobachtbar; die
  Stufen-Quader-Konstruktion, `rise = Geschosshöhe/step_count`, Aufstiegs-
  richtung und Netz-Verfahren sind Mechanik → §1, **nicht** Lastenheft.
- **Aufstiegsrichtung ohne Schema-Spalte:** `stairs` hat `start_x/y`, aber
  **keine** Richtungs-/Winkel-Spalte → welle-2 fixiert die Richtung (+x ab
  Startpunkt); freie Rotation bräuchte eine Schema-Erweiterung und bleibt
  offen. Im §1 ehrlich als Teilumfang benannt (Muster ROF „rechteckig").
- **Geländer-Persistenz (offen):** das `stairs`-Schema trägt **keine**
  Geländer-Spalte. In welle-2 wird das Geländer aus der Treppen-Geometrie
  **gerendert** (sichtbar, STR-004 erfüllt); eine persistierte An/Aus- oder
  Seiten-Option ist späterer Ausbau (Schema-Erweiterung) — analog der
  welle-2-Felder, die das Domänenmodell (noch) nicht trägt. Im §1 + Closure
  benannt; die MR-006-Linse prüft, ob das die STR-004-AK trägt.
- **`rise` abgeleitet vs. Schema-Spalte `rise_mm`:** das Schema speichert
  `rise_mm` **und** `step_count` explizit; die Spec legt fest, dass `rise`
  aus Geschosshöhe/`step_count` folgt (flächenbündiger Anschluss) und der
  gespeicherte Wert dies widerspiegelt — keine widersprüchliche
  Doppel-Eingabe. (Diskriminierung zur freien `rise`-Eingabe ist eine
  016b-/Domänen-Frage; §1 hält die Ableitung fest.)
- **Zwei-Geschoss-Spanne (`restrict`):** `stairs.from_storey_id`/
  `to_storey_id` mit `ON DELETE RESTRICT` (kein Cascade — eine Treppe hält
  ihre Geschosse) — Persistenz-Detail für 016c, hier nur benannt.
- **Versions-Drift (Nebenbefund, 3×):** Header-Bump bei Schärfung 3× vergessen
  (013a/014a/015a) → 016a fixt + Lerneintrag; ob ein Sensor (docs-check-Regel
  „Header == oberste Historie-Zeile") lohnt, ist Steering-Kandidat.
- **„Lösung schärft nie das Lastenheft" (MR-008):** strikt anwenden; die
  MR-006-Linse prüft als Konvention.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; Dichte hoch (AK-Format, Reifephase-Klausel, MR-001/MR-008,
  Wertebereich-Konvention); Phase-Reife: STR Phase 2 (Outline → AK); Risiko
  niedrig (reine Doku).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; Dichte hoch (ADR-0011-Leitplanke: kein neuer Grundsatz-ADR;
  Reuse des analytischen-Netz-Musters dokumentiert); Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **Lastenheft 0.1.6:** LH-FA-STR-001..004 von Outline auf AK-Niveau
  (gerade einläufige Treppe verbindet zwei Geschosse; Stufenanzahl 2–30,
  Laufbreite 800–2000 mm, immer sichtbares Geländer), **lösungsfrei/
  benutzer-beobachtbar** (MR-008 strikt — keine Mechanik im Lastenheft-Text);
  **Teilumfang gerade einläufig** (Podest/Wendel/Mehrlauf offen); §9-Historie.
  **Versions-Drift behoben:** Header 0.1.2 → 0.1.6 (war seit slice-012 nicht
  nachgezogen, während die Historie auf 0.1.5 wuchs).
- **`spec/spezifikation.md` §1 `LH-FA-STR-001.a`** (Sammelblock STR-001..004):
  gerade einläufige Treppe als **analytisches Stufen-Quader-Polyeder im Kern**
  (Präzedenz `roof_geometry`, kein OCC — ADR-0009 gewahrt); Geschoss-Spanne
  `from→to`, **`base_z` = Boden der unteren Etage**; **`rise =
  Geschosshöhe/step_count` abgeleitet** (kein Doppel-Eingabe, Muster
  ROF-Firsthöhe); Stufe `i` = Quader `x∈[i·tread,(i+1)·tread]`,`y∈[0,width]`,
  `z∈[0,(i+1)·rise]`; **feste +x-Aufstiegsrichtung** (Schema ohne Richtungs-
  Spalte); **Geländer = generierte Geometrie** auf `STAIR_RAILING_HEIGHT_MM`
  (kein persistierter Eigenzustand — wie abgeleitete roofs-`height_mm`);
  Klemmung step_count/width/tread (`E-VAL-001`), **`rise` informativ** (kein
  Klemmpunkt, LOW-2); Totalität (ungültige Spanne → keine Treppe);
  **`StairChanged`-`op` an `from_storey` gebunden + begründet** (MED-1; die
  Treppe spannt zwei Geschosse, Anker/`base_z` liegt unten), `stairMeshes`
  projektweit, **kein `RoomsChanged`**. **§3:** STAIR-Wertebereiche + Defaults
  + Geländerhöhe (LOW-1, mit Begründungs-Spalte). §8-Historie.
- **Kein ADR, kein Code** (ADR-0011 #6); `architecture.md` nicht geändert.
  **`make gates` grün** (2026-06-14): docs-check 75 Dateien/0 Befunde,
  arch-check, lint, **Tests 105/105 unverändert** (reine Doku), Coverage
  unverändert.

**Lerneintrag:**

- **Header-Versions-Drift (3×, jetzt behoben):** 013a/014a/015a ergänzten je
  eine §9-Historie-Zeile (0.1.3/0.1.4/0.1.5), zogen aber den Header
  `Version:` nicht nach — er stand seit slice-012 auf 0.1.2. Erst der vierte
  Schärfungs-Slice fiel auf den Drift. **Steering-Kandidat (INFO-1):** eine
  docs-check-Regel „Header-Version == oberste §9-Historie-Zeile" würde die
  Klasse computational fangen (Kurs-Regel: 3× → Sensor); bewusst **nicht** in
  016a (außerhalb des Slice-Scopes), als Kandidat notiert.
- **Abgeleitete Größe + render-only-Feature halten den Schema-Vertrag klein:**
  `rise` (aus Geschosshöhe/step_count) und das Geländer (aus der Stufen-
  Geometrie) brauchen **keine** eigene Eingabe/Schema-Spalte — exakt das
  ROF-Firsthöhe-Muster. So trägt das bestehende `stairs`-Schema die ganze
  welle-2-Treppe ohne Erweiterung, und der 013c/015c-Grundsatz „nur
  Eigenzustand muss round-trippen" greift sauber (Review bestätigt: kein
  stiller Verlust).

**Restrisiko / Nachfolge:** slice-016b (Treppen-Implementierung: Domänen-
`Stair`-Typ, `stair_geometry` analytisch, Edit-Ops, `ViewModelPort.stairMeshes`,
`StairChanged`-`op`, Geländer-Geometrie, AK-Tests) — die **+x-Richtung,
`rise`-Ableitung und Geländer-Konstruktion** sind dort die Kern-Punkte. Dann
slice-016c (Persistenz `stairs`-Tabelle, from/to_storey-Spanne `restrict`).
Danach **Welle-2-Closure** (unabhängige Verifikation + `done/welle-2-results.md`
inkl. zwingendem Carveout-Audit) — STR ist das letzte welle-2-Bauteil.
