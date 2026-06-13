---
id: slice-013a
titel: Bauteil-Hosting & Wandöffnungs-Modell — ADR + AK-Schärfung DOR/WIN
status: next
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-DOR-001, LH-FA-DOR-002, LH-FA-DOR-003, LH-FA-DOR-004, LH-FA-WIN-001, LH-FA-WIN-002, LH-FA-WIN-003, LH-FA-WIN-004, LH-FA-WIN-005]
adr_refs: [ADR-0001, ADR-0002, ADR-0006, ADR-0007, ADR-0008]  # ADR-0011 (Bauteil-Hosting/Öffnung) entsteht in diesem Slice
---

# Slice 013a: Bauteil-Hosting & Wandöffnungs-Modell — ADR + AK-Schärfung DOR/WIN

**Status:** next (Plan geschrieben; **MR-006-Plan-Review gelaufen
2026-06-13 — keine HIGH, 3 MED + 3 LOW eingearbeitet**;
implementierungsbereit).

**Welle:** welle-2-bauteile (erste Slice der Welle).

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-013a-plan.md) (M/L-IDs) —
keine HIGH; M1 (ADR-0001/0002-Hoheit), M2 (`docs/reviews/`-Zeile),
M3 (schema-check-Beleg) und L1–L3 eingearbeitet.

**Bezug:** LH-FA-DOR-001..004, LH-FA-WIN-001..005 (im Lastenheft bisher
**Outline** — dieser Slice schärft sie auf AK-Niveau, Reifephase-Klausel,
Präzedenz slice-009a/010a/012). ADR-0002 (OCC-Backend: Booleans /
Wandöffnungen — bereits für LH-FA-DOR-004/WIN-005 reserviert), ADR-0006
(`openings → doors/windows`-Spezialisierung im Schema), ADR-0007
(Raumerkennung — Öffnung darf den Wandzug nicht verändern), ADR-0008
(Benachrichtigungs-Vertrag Kern → Darstellung), ADR-0001 (Schichtung).
**Geliefert:** ADR-0011 (in diesem Slice entstanden und accepted).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des Tür-/Fenster-
Strangs (Muster slice-009a/010a/011a): Türen und Fenster brauchen ein
**entschiedenes Hosting- und Öffnungs-Modell** (wie wird ein
wand-gehostetes Bauteil repräsentiert, wie schneidet es die Wand, wer
meldet die geänderte Wand-Geometrie) **vor** der Implementierung
(slice-013b). Diese ADR ist zugleich die **Leitplanke** für die übrigen
Bauteil-Module der Welle (ROF/SLB/FND/STR) — Pflicht (f) verallgemeinert
das Bauteil-Erweiterungs-Muster, damit die Folge-Slices keine je eigene
Grundsatz-ADR brauchen.

---

## 1. Ziel

Türen (LH-FA-DOR-*) und Fenster (LH-FA-WIN-*) bekommen **prüfbare,
lösungsfreie Akzeptanzkriterien** im Lastenheft und ein **entschiedenes
technisches Modell** (ADR-0011 + Spezifikation §1), bevor implementiert
wird. Kern der Entscheidung ist die **automatische Wandöffnung**
(LH-FA-DOR-004/WIN-005): ein platziertes Tür-/Fenster-Bauteil erzeugt
eine Öffnung in seiner Wirtswand. Vorhandene Basis: das OCC-Boolean-
Backend ist in ADR-0002 für genau diesen Fall reserviert; die
Footprint-Extrusion des Wand-Solids existiert seit slice-012; das
Schema sieht die `openings`-Spezialisierung vor (ADR-0006); der
Benachrichtigungs-Vertrag existiert (ADR-0008, `WallGeometryChanged`).
Neu sind die AK, das Hosting-/Öffnungs-Modell und das verallgemeinerte
Bauteil-Erweiterungs-Muster für die Welle.

## 2. Definition of Done

- [ ] **Lastenheft LH-FA-DOR-001..004 + LH-FA-WIN-001..005 geschärft**
      (Outline → AK-Niveau, Reifephase-Klausel; **lösungsfrei und
      benutzer-beobachtbar** — keine Lösungsmechanik im Lastenheft-Text;
      die gehört in DoD-2/ADR-0011 und DoD-3/Spec, sonst würde die
      Entscheidung ins Lastenheft vorgezogen, Präzedenz Plan-Review 010
      F1): je Happy/Boundary/Negative. Mindestens:
      **Türen** — platzieren an einer Wirtswand (DOR-001), verschieben
      entlang der Wand (DOR-002), Parameter Breite/Höhe/**Anschlag**
      (DOR-003), **automatische Wandöffnung** (DOR-004: nach Platzieren
      ist die Wand an der Tür-Position durchbrochen, in 3D sichtbar).
      **Fenster** — platzieren (WIN-001), verschieben (WIN-002),
      Parameter (WIN-003), **Brüstungshöhe** (WIN-004: Öffnung beginnt
      erst oberhalb der Brüstung), automatische Wandöffnung (WIN-005).
      Boundary (benutzer-beobachtbar, lösungsfrei): Öffnung **breiter
      als die Wand bzw. über das Wandende hinaus** → geklemmt/abgelehnt
      (Wertebereich, kein Loch außerhalb der Wand); **Öffnung höher als
      die Wand** → geklemmt auf Wandhöhe; **zwei überlappende
      Öffnungen** in derselben Wand → definiertes Verhalten (Wortlaut
      lösungsfrei). Negative: Tür/Fenster **ohne Wirtswand** →
      abgelehnt, kein verwaistes Bauteil. + Historie-Zeile.
- [ ] **ADR-0011 „Bauteil-Hosting & Wandöffnungs-Modell" accepted**
      (MADR-Form, Optionen mit Trade-offs). Entscheidungs-Pflichten:
      (a) **Domänen-Repräsentation** wand-gehosteter Bauteile: Opening
          als Kind/Teil der Wand vs. eigenständiges `model`-Element mit
          Wand-Referenz; Position entlang der Wandachse (Parameter,
          Wertebereiche-Quelle); pure Werte, framework-frei (ADR-0001);
      (b) **Geometrie-Strategie:** Öffnung als **boolesche Subtraktion**
          des Öffnungs-Quaders vom Wand-Solid über `GeometryKernelPort`;
          **Zusammenspiel mit der Footprint-Extrusion** (slice-012: Kern
          liefert Footprint-Polygon, OCC extrudiert/tesselliert) — wird
          vom extrudierten Footprint-Solid subtrahiert; Port-Signatur-
          Folge (neue Operation vs. Erweiterung von `extrudeFootprint`).
          **Zuständigkeits-Trennung (M1, slice-012 §6):** die
          Port-**Signatur** ist **ADR-0001-Hoheit des Kerns**, nicht
          ADR-0002 — ADR-0002 liefert nur das OCC-**Boolean-Backend**
          hinter dem Port. Kein OCC-Typ verlässt den Adapter (Regel C);
      (c) **Benachrichtigung & Transaktion:** Öffnungs-Mutation ändert
          die Wand-Geometrie → meldet (bestehendes `WallGeometryChanged`
          vs. neues `op`; Vokabular konsistent zu spez. §5), Reihenfolge
          zu `RoomsChanged`; **Post-Commit total** und transaktional
          (schlägt der Boolean fehl, bleibt das Modell unverändert —
          Präzedenz slice-012 Fehlerfall-AK);
      (d) **Persistenz:** trägt die `openings → doors/windows`-
          Spezialisierung aus ADR-0006 unverändert, oder ist eine
          Schema-Schärfung nötig (Position, Anschlag, Brüstung)?
          Drift-Disziplin `schema.sql`/`data-model.yaml` (ADR-0006,
          `make schema-check`);
      (e) **Raumerkennung unberührt:** eine Öffnung verändert **keinen
          Wandzug** und keine Wand-Achse → ADR-0007-Erkennung und die
          ROM-AK-Tests bleiben textlich unverändert grün (explizit
          festgehalten); ebenso die Footprint-/Eckenschluss-Geometrie
          (slice-012) — Öffnung wirkt nur auf das 3D-Solid, nicht auf
          den Grundriss-Footprint (oder, falls doch 2D-sichtbar:
          entschieden und begründet);
      (f) **Bauteil-Erweiterungs-Muster (Welle-Leitplanke):** wie kommt
          ein **neuer parametrischer Bauteil-Typ** generell in
          Domain-Modell · `EditStructurePort`-Operationen ·
          `GeometryKernelPort` · `ModelChangedPort`-`op` · Persistenz
          (ADR-0006-Schema) · Viewer-Szene — als Muster, dem die
          Folge-Slices ROF/SLB/FND/STR der Welle folgen, ohne je eigene
          Grundsatz-ADR (nur eigene Geometrie-/Spec-Entscheidung).
          **Beobachtbar (L3):** ADR-0011 enthält einen benannten
          Abschnitt „Bauteil-Erweiterungs-Muster" mit einer
          **nummerierten Integrations-Schritt-Liste**, am
          Tür/Fenster-Fall belegt.
      ADR-Index aktualisiert (Zeile + ggf. Folgepflicht slice-013b).
- [ ] **`spec/spezifikation.md` §1 präzisiert** (Öffnungs-Block):
      **hier — nicht im Lastenheft —** leben Öffnungs-Algorithmus
      (boolesche Subtraktion, Öffnungs-Quader aus Position + Breite +
      Höhe + Brüstung/Anschlag), Wertebereiche/Default-Klemmung,
      Fehler-Codes (Muster `E-…`), das Notification-`op` und der
      Determinismus; §8-Historie. `spec/architecture.md` Port-Zeile(n)
      nachziehen, falls die ADR-(b)-Entscheidung die `GeometryKernelPort`-
      Signatur ändert (sichere Änderung, nicht „ggf.", falls (b) so
      entscheidet). `make gates` grün **und — falls (d) eine
      Schema-Schärfung beschließt — zusätzlich `make schema-check` grün**
      (separater Lauf, **nicht** in `gates`, d-migrate aus dem Gate-Pfad,
      MR-007/M3). Closure-Notiz mit Lerneintrag.
- [ ] **Reine Doku/Entscheidung — kein Code, keine Tests** in diesem
      Slice (Präzedenz 009a/010a). Die AK-Tests + Implementierung sind
      slice-013b. **Nicht Teil dieser DoD:** die Welle-Closure
      `welle-2-bauteile`.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0011-bauteil-hosting-wandoeffnung}.md` | neu | Lösungs-Entscheidung Hosting/Öffnung/Boolean/Notification/Persistenz + Bauteil-Erweiterungs-Muster |
| `docs/plan/adr/README.md` | ändern | Index-Zeile ADR-0011 + Folgepflicht (slice-013b) + §Offene-Themen ggf. |
| `spec/lastenheft.md` | ändern | DOR-001..004 + WIN-001..005 von Outline auf AK-Niveau (Reifephase-Klausel); §9-Historie |
| `spec/spezifikation.md` | ändern | §1 Öffnungs-Algorithmus + Wertebereiche/Fehler-Codes + Notification-`op`; §8-Historie |
| `spec/architecture.md` | ggf. ändern | `GeometryKernelPort`-/`EditStructurePort`-Zeile, falls ADR-0011 (b) die Signatur ändert — Begründung in der Closure-Notiz |
| `spec/data-model.yaml`, `src/adapters/persistence/schema.sql` | ggf. ändern | nur falls ADR-0011 (d) eine Schema-Schärfung beschließt; dann d-migrate-validiert + `make schema-check` (ADR-0006) |
| `docs/plan/planning/in-progress/roadmap.md` | (bereits gesetzt) | welle-2-Aktivierung erfolgte beim Welle-Start; hier nur Drift, falls die ADR den Slice-Schnitt der Welle ändert |
| `docs/reviews/{2026-06-13-slice-013a-plan}.md` | neu | MR-006-Plan-Review-Report (M2; Findings vor Start eingearbeitet) |
| diese Datei | ändern | Frontmatter-`adr_refs` um ADR-0011 ergänzen, sobald die ADR-Nummer steht; die analoge Ergänzung in slice-013b greift erst, **sobald 013b als Folgepflicht entsteht** (L1) |

## 4. Trigger

- Keiner — **sofort startbar** nach MR-006-Plan-Review: reine
  Doku-/Entscheidungsarbeit; das OCC-Boolean-Backend ist in ADR-0002
  reserviert, die Footprint-Extrusion existiert seit slice-012, der
  Benachrichtigungs-Vertrag seit ADR-0008.

## 5. Closure-Trigger

- DoD vollständig, ADR-0011 `Accepted`, `make gates` grün,
  Closure-Notiz geschrieben → **slice-013b** (Implementierung Türen +
  Fenster inkl. AK-Tests, Viewer-Folgen, Persistenz) wird startbar; die
  Bauteil-Leitplanke (f) gilt ab dann für die übrigen Welle-Slices
  (ROF/SLB/FND/STR).

## 6. Risiken und offene Punkte

- **Sitzungs-Umfang an der Obergrenze:** DoD-2 bündelt sechs
  Entscheidungs-Facetten *einer* Hosting-/Öffnungs-Mechanik plus die
  Welle-Leitplanke (f). Kippt die Review-Sitzung, ist der natürliche
  Split-Punkt: **(i)** Hosting + Öffnungs-Boolean + Notification (a–e)
  → **(ii)** Verallgemeinerung des Bauteil-Erweiterungs-Musters (f) als
  eigener Mini-Slice. Pflicht (f) ist nur Leitplanke, kein Code —
  notfalls vertagbar (Präzedenz 003/009/010-Split). **Zweiter
  Split-Punkt (L2):** die Lastenheft-AK-Schärfung (DoD-1) ist von der
  ADR-Entscheidung (DoD-2) trennbar — kippt die Sitzung früh, kann DoD-1
  (reine AK, lösungsfrei) als eigener Schärfungs-Slice vorab schließen.
- **Lösungsfreiheit des Lastenhefts (Wiederholungsfall):** „automatische
  Wandöffnung" ist benutzer-beobachtbar (die Wand ist durchbrochen);
  **boolesche Subtraktion** ist Mechanik und darf **nicht** ins
  Lastenheft (Spec/ADR). Der Zähler „Lösung schärft nie das Lastenheft"
  steht bei 3× (welle-1v-results §5) — hier greift er erneut und ist der
  Festschreibungs-Anlass (AGENTS §2 oder neuer `MR-<NNN>`), zu
  entscheiden im Review.
- **Footprint-Interaktion (slice-012):** Die Öffnung subtrahiert vom
  extrudierten Footprint-Solid. Ob die Footprint-Hoheit im Kern
  (slice-012) eine Port-Signatur-Änderung erzwingt (z. B.
  `extrudeFootprint` + Öffnungs-Liste vs. nachgelagerter Boolean), ist
  eine ADR-(b)-Entscheidung — **nicht** vorab im Plan fixiert
  (Lösungsfreiheit der Ebenen, Präzedenz 011-F3).
- **Raumerkennung & Volumen-Semantik:** Öffnung darf ADR-0007
  (Wandzug/Innenkante) nicht berühren; das Wand-Volumen sinkt um das
  Öffnungs-Volumen (LH-FA-EVL-* spätere Welle — Hinweis in spez. §1, wie
  slice-012 für die Footprint-Fläche).
- **Persistenz-Drift:** Falls (d) eine Schema-Schärfung beschließt,
  zieht der `make schema-check`-Drift-Gate sofort — `data-model.yaml`
  und `schema.sql` müssen im selben Slice konsistent bleiben (ADR-0006).
- **Bauteil-Erweiterungs-Muster vs. Über-Generalisierung:** Pflicht (f)
  soll ein *Muster* festhalten, das am konkreten Tür/Fenster-Fall belegt
  ist — **kein** spekulativer Rahmen für ROF/SLB/FND/STR, deren
  Geometrie sich stark unterscheidet (Dach ≠ Öffnung). Die Leitplanke
  betrifft den *Integrations-Pfad* (Domain/Port/Persistenz/Notification/
  Viewer), nicht die je eigene Geometrie-Entscheidung.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF
- **Konventionen-Dichte:** hoch — AK-Format, Reifephase-Klausel,
  ADR-Schärfungs-Regel (MR-001), Wertebereich-/Fehler-Code-Konvention.
- **Phase-Reife:** Phase 2 für DOR/WIN (Outline — genau das hebt der
  Slice auf AK-Niveau); Spezifikation §1 Phase 3.
- **Evidenz-/Diskrepanz-Risiko:** niedrig (reine Doku).
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Planning-Lifecycle (ADRs)

- **Modus:** GF
- **Konventionen-Dichte:** hoch — MADR, `Accepted` immutable,
  Index-Pflicht, Referenz-Richtung (nur aktive ADRs).
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).
</content>
