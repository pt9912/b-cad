---
id: slice-017a
titel: Auswertung & Material — ADR-0012 (Evaluations-Architektur) + Lastenheft EVL/MAT-Schärfung & Spec
status: next
welle: welle-3-auswertung
lastenheft_refs: [LH-FA-EVL-001, LH-FA-EVL-002, LH-FA-EVL-003, LH-FA-EVL-004, LH-FA-EVL-005, LH-FA-EVL-006, LH-FA-MAT-001, LH-FA-MAT-002, LH-FA-MAT-003, LH-FA-MAT-005, LH-FA-MAT-006]
adr_refs: [ADR-0001, ADR-0006, ADR-0007, ADR-0011, ADR-0012]
---

# Slice 017a: Auswertung & Material — ADR-0012 + Lastenheft-Schärfung & Spec

**Status:** next — **MR-006-Plan-Review des Projektinhabers gelaufen
(2026-06-14): 1 HIGH + 2 MED + 3 LOW/2 INFO, alle eingearbeitet** (Report
[`2026-06-14-slice-017a-plan.md`](../../../reviews/2026-06-14-slice-017a-plan.md)).
**HIGH-1 (ADR bündelt zwei Entscheidungsfelder) gelöst per Remedy 1:** ADR-0012
ist auf die **Evaluations-Architektur** verengt; die Material-Zuweisungs-
Autorität ist **Spec-Gewicht** (LH-FA-MAT-003.a), kein ADR. **Wartet auf das Go
des Projektinhabers** (HIGH gelöst → startbar).

**Welle:** welle-3-auswertung (erster Slice; **Welle-Leitplanke**, Muster
slice-013a).

**Bezug:** LH-FA-EVL-001..006 (Auswertungen) + LH-FA-MAT-001..006 (Material),
im Lastenheft bisher **reines Outline**. **Neue Grundsatz-ADR ADR-0012**
(**Evaluations-Architektur** — eng) als welle-3-Leitplanke. ADR-0006
(`materials`-Schema + `material_id`-FKs **liegen vor** — der Material-
*Persistenz*-Vertrag ist damit bereits entschieden; `layers` gehört zu
**DRW**/welle-5, nicht zu Material — LOW-1). ADR-0007 (Raumerkennung +
**Netto-Fläche** — Fundament für EVL-001/003; ROM-002/003 sind die Per-Raum-
Quelle, EVL aggregiert — LOW-2). ADR-0011 (Öffnungs-Schnittprismen — für das
Netto-Volumen, MED-2). ADR-0001 (Auswertung als reine Query/Driving-Port).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-14.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des Auswertungs-Strangs
(Muster 013a). **Reine Doku/Entscheidung, kein Code.** Mit der ADR-Verengung
(HIGH-1) trägt der Slice **eine** Grundsatz-Entscheidung (Evaluation) + zwei
**lösungsfreie Lastenheft-Schärfungen** (EVL, MAT) + Spec — kein gebündeltes
Doppel-ADR.

---

## 1. Ziel

Die welle-3-Leitplanke setzen: **ADR-0012** entscheidet die
**Evaluations-Architektur**, und das Lastenheft wird für **Auswertungen (EVL)**
und **Material (MAT)** von Outline auf AK-Niveau geschärft (lösungsfrei,
MR-008). Kernbeobachtung: **Auswertung ist eine read-only-Ableitung** — Flächen
via Shoelace auf dem Footprint (Netto, ADR-0007), **Netto-Volumen analytisch im
Kern** (footprint·Höhe − Öffnungs-Schnittprismen), Listen als Aggregation über
das Modell; kein neuer Solid, keine Mutation, keine eigene Persistenz. **Material**
ist eine Domänen-Erweiterung (Werttyp + `material_id`-Zuweisung) auf dem
bestehenden ADR-0006-Schema — die Zuweisungs-Autorität ist eine **Spec-**, keine
ADR-Frage.

## 2. Definition of Done

- [ ] **ADR-0012 „Evaluations-Architektur" accepted** (nach **unabhängigem
      ADR-Text-Review** vor Accept — Muster 013a/009a; ADR-Accept ist
      Review-Checkpoint). **Eng auf Evaluation** (HIGH-1 — **kein** Material-
      Modell-Entscheid im ADR; Material erscheint nur als *von EVL konsumierte
      Eingabe*). Entscheidet mindestens:
      **(a) Auswertungs-Port:** neuer Driving-Port `EvaluatePort` (EVL-001..006)
      **vs.** `DetectRoomsPort`-Erweiterung. *Empfehlung:* eigener
      `EvaluatePort` (Auswertung aggregiert **modellweit**; `DetectRoomsPort`
      bleibt raum-spezifisch — die `Room`-Netto-Fläche/ADR-0007 ist eine
      **Quelle**, nicht der Port).
      **(b) Ergebnis-Modell:** pure Werte (`AreaReport`/`VolumeReport`/
      `MaterialLine` o. ä.), framework-frei, read-only.
      **(c) Read-only/Pull:** Auswertung mutiert nie, on-demand aus dem
      committeten Modell (wie `DetectRoomsPort`); **keine eigene Persistenz**,
      **kein `…Changed`-`op`** (Empfehlung pull; vs. reaktiv entscheiden).
      **(d) Flächen- UND Volumen-Fundament (MED-2 — gleichrangig):** Flächen via
      **Shoelace** auf dem Footprint (Netto = äußerer Ring minus Loch-/Ausschnitt-
      Ringe, ADR-0007, **keine Doppelzählung**). **Netto-Volumen analytisch im
      Kern**: footprint-Fläche·Höhe **minus** Σ Öffnungs-Schnittprismen
      (ADR-0011/DOR-004.a/WIN-005.a) — Dach/Platte/Treppe sind ohnehin
      analytisch. **Bewusst kein OCC-`GProp` im Geometrie-Adapter**, sonst kippt
      die „reine Kern-Query"-Rahmung (driven-Round-Trip). Das ADR legt das
      explizit fest und **führt die verworfene Alternative (OCC-`GProp`-Volumen-
      messung im Geometrie-Adapter) unter „Verglichene Alternativen"** — ein
      echtes Entscheidungsprotokoll, konsistent mit ADR-0001..0011 (Forward-Note
      des Reviews).
      ADR-Index ([`docs/plan/adr/README.md`](../../adr/README.md)) aktualisiert;
      `architecture.md` ggf. um `EvaluatePort` ergänzt.
- [ ] **Lastenheft EVL-001..006 + MAT-001/002/003/005/006 von Outline auf
      AK-Niveau geschärft** (Reifephase-Klausel; **lösungsfrei, benutzer-
      beobachtbar**, MR-008): je Happy/Boundary/Negative wo sinnvoll.
      **EVL:** Flächenberechnung (EVL-001: Netto-Grundfläche je Raum/Geschoss in
      m²), Volumen (EVL-002), Wohnfläche (EVL-003), Material-/Tür-/Fensterlisten
      (EVL-004/005/006). **MAT:** Materialien verwalten/Bibliothek (MAT-001/002),
      Zuweisung an Bauteile (MAT-003), U-Wert (MAT-005), Kosten (MAT-006).
      **MAT-004 Texturen** ist **rendering-/Darstellungs-nah** → **Teilumfang
      offen** (nicht M3, gehört zur Sicht). **MR-010:** Header-`Version:` auf die
      neue §9-Historie-Zeile nachziehen. + §9-Historie.
- [ ] **`spec/spezifikation.md` präzisiert:**
      **§1 `LH-FA-EVL-001.a`** — Fläche via Shoelace (Netto, ADR-0007-Definition
      **wiederverwendet**, nicht zweite Semantik); **Netto-Volumen analytisch im
      Kern** (footprint·Höhe − Öffnungs-Prismen, MED-2); Wohnfläche-Regel (Summe
      Raum-Netto-Flächen; Anrechnungsfaktoren als **Teilumfang offen**); Listen
      als **Aggregation**. **EVL-004-Abdeckung ehrlich (MED-1):** material-
      tragend in welle-3 sind **`walls`/`roofs`/`slabs`** (`material_id`);
      **`stairs`/`openings`/`doors`/`windows` tragen kein `material_id`** (Schema)
      → als **bewusste Lücke/späterer Ausbau** ausgewiesen, keine über-
      versprochene EVL-004-AK. **Grenze ROM-002/003 ↔ EVL-001/002 (LOW-2):**
      ROM-002/003 = Per-Raum-Quelle (ADR-0007); EVL = Aggregation/Bericht — eine
      Semantik. **§2.1** — `model::Material` als pure Werte (Kennwerte aus dem
      `materials`-Schema) + die **FK-Zuweisungs-Autorität** (`material_id` direkt
      am Bauteil **vs.** über `wall_types` — strukturell, **Spec-Entscheidung**
      LH-FA-MAT-003.a, HIGH-1/INFO-1). **Die effektive Auflösungsregel** (eigenes
      `material_id`, sonst über `wall_type`) ist **Datenfluss → §1** (NIT:
      Werttyp/FK in §2.1, Auflösungs-Mechanik in §1 — bei der Umsetzung
      konsistent halten).
      **§3** ggf. Konstanten (Wohnflächen-Faktoren-Default, Default-Material).
      + §8-Historie.
- [ ] **Reine Doku/Entscheidung — kein Code.** `make gates` grün; Closure-Notiz
      mit Lerneintrag. **Nicht Teil:** Material-Implementierung (Domänentyp +
      Zuweisung + `material_id`-Persistenz-Round-Trip); EVL-Implementierung
      (`EvaluatePort`-Service + Shoelace/Netto-Volumen/Listen + AK-Tests);
      MAT-004 Texturen; DRW (welle-5).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0012-evaluations-architektur}.md` | neu | ADR-0012 **eng** (EvaluatePort, Ergebnis-Modell, read-only/pull, Flächen-+Volumen-Fundament) |
| `docs/plan/adr/README.md` | ändern | ADR-0012 im Index + Folgepflichten |
| `spec/lastenheft.md` | ändern | EVL-001..006 + MAT-001/002/003/005/006 Outline→AK; Header-Version (MR-010); §9-Historie |
| `spec/spezifikation.md` | ändern | §1 `LH-FA-EVL-001.a` (Shoelace/Netto-Volumen/Listen, MED-1/LOW-2); §2.1 `model::Material` + Zuweisungs-Autorität (HIGH-1/INFO-1); §3; §8 |
| `spec/architecture.md` | ggf. ändern | `EvaluatePort` (Driving) in §1.1, falls die ADR ihn so entscheidet |
| `docs/reviews/2026-06-14-slice-017a-plan.md` | neu ✓ | MR-006-Report (Projektinhaber) — **geschrieben** |

## 4. Trigger

- welle-2 abgeschlossen ✓ (M2; alle Bauteile + Persistenz mit `material_id`-FKs).
- **MR-006-Plan-Review (Projektinhaber) gelaufen ✓, Findings eingearbeitet** —
  HIGH-1 gelöst → **startbar nach Go**.

## 5. Closure-Trigger

- DoD vollständig (ADR-0012 accepted, Lastenheft geschärft, Spec präzisiert),
  `make gates` grün, Closure-Notiz → **slice-017b** (Material-Implementierung +
  `material_id`-Persistenz) und der EVL-Implementierungs-Strang startbar.
- **INFO-2 mittragen:** die **EVL-Implementierung** (Shoelace-Netto, Loch-
  Subtraktion, Netto-Volumen) ist **geometrie-korrektheits-nah** (Winding/
  Orientierung, keine Doppelzählung) → **MR-009 greift für den EVL-Impl-Slice**
  (nicht für dieses reine Doku-017a). Im 017b-Trigger benennen, damit es nicht
  durchrutscht.

## 6. Risiken und offene Punkte

- **Schnitt-Umfang (HIGH-1 gelöst):** mit der ADR-Verengung trägt 017a **ein**
  Evaluations-ADR + zwei **lösungsfreie** Schärfungen + Spec — kein gebündeltes
  Doppel-ADR. EVL und MAT sind verschiedenartig, aber als reine Doku-Schärfung
  (kein Code) in einem a-Slice tragbar (Muster 013a). Falls die Umsetzung zeigt,
  dass die MAT-Schärfung eigenes Gewicht hat: in einen eigenen MAT-Slice ziehen
  (kein ADR — Material ist ADR-0006-/Spec-Sache).
- **Port-Entscheidung (ADR-Kern):** `EvaluatePort` neu vs. `DetectRoomsPort`
  erweitern — die `architecture.md`-EVL-001..003-Zuordnung ist Hinweis, kein
  Zwang; eine Erweiterung würde einen raum-spezifischen Port mit modellweiter
  Aggregation überladen.
- **Material-Zuweisungs-Autorität (Spec, nicht ADR — HIGH-1):** `material_id`
  liegt **sowohl** an `walls`/`roofs`/`slabs` **als auch** an `wall_types`.
  Autorität (direkt am Bauteil vs. über den Typ) = LH-FA-MAT-003.a §2.1,
  konsistent mit ADR-0006.
- **Material-Abdeckungs-Lücke (MED-1):** `stairs`/`openings`/`doors`/`windows`
  tragen kein `material_id` (verifiziert, `schema.sql`) → EVL-004 deckt in
  welle-3 nur `walls`/`roofs`/`slabs`; der Rest ist benannter späterer Ausbau
  (Spec, nicht Lastenheft).
- **Netto-Volumen-Schicht (MED-2):** analytisch im Kern (footprint·Höhe −
  Öffnungs-Prismen), **kein** OCC-`GProp`/driven — sonst kippt „reine Query".
- **Wohnfläche (EVL-003):** Anrechnungsfaktoren fachlich aufgeladen → Teilumfang
  (Netto-Grundfläche ≈ Wohnfläche, Faktoren offen), in Spec ehrlich benannt.
- **ADR-0007-/ROM-002/003-Konsistenz (LOW-2):** eine Flächen-/Volumen-Semantik;
  ROM-002/003 (Per-Raum) als Quelle, EVL als Aggregat — Grenze in §1 ziehen.
- **Lösungsfreiheit Lastenheft (MR-008) + MR-010:** Mechanik (Shoelace/
  Aggregation) → §1; Header-Version nachziehen.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; Dichte hoch (AK-Format, Reifephase-Klausel, MR-001/008/010,
  Wertebereich-/Algorithmik-Konvention); Phase-Reife EVL/MAT Phase 2; Risiko
  niedrig (reine Doku).

### Sub-Area: Planning-Lifecycle (`docs/plan/`)

- **Modus:** GF; Dichte hoch (Grundsatz-ADR ADR-0012 **eng**, Folgepflichten,
  Immutability, unabhängiger Text-Review vor Accept); Risiko mittel — die
  Port-Entscheidung trägt Welle-Gewicht (daher der ADR-Checkpoint).

## 8. Closure-Notiz

*(folgt nach `make gates` grün.)*
