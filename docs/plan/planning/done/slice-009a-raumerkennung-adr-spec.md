---
id: slice-009a
titel: Raumerkennung — ADR-0007 Polygon-Basis & Spec-Schärfung
status: done
welle: welle-1-mvp
lastenheft_refs: [LH-FA-ROM-001]
adr_refs: [ADR-0001, ADR-0007]
---

# Slice 009a: Raumerkennung — ADR-0007 Polygon-Basis & Spec-Schärfung

**Status:** done

**Welle:** welle-1-mvp

**Bezug:** LH-FA-ROM-001, ADR-0001. **Geliefert:**
[ADR-0007](../../adr/0007-raumerkennung-geometrie-basis.md) (in diesem
Slice entstanden und accepted).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-11.

**Schnitt-Herkunft:** Aufteilung des ursprünglichen `slice-009` in die
Entscheidungs-/Spec-Grundlage (**009a**, dieser Slice) und die
Implementierung
([**009b**](../open/slice-009b-raumerkennung-implementierung.md)) — Ergebnis
des Plan-Reviews (Findings H1/M1/M2): ADR-0007 trägt mehr
Entscheidungsgewicht als ursprünglich geplant, und ein ADR-Accept ist
ein Review-Checkpoint, der nicht mitten in einem Implementierungs-Slice
liegen soll (Präzedenz: slice-007 vor 008a/b; slice-003-Split).

---

## 1. Ziel

Die Raumerkennung (LH-FA-ROM-001) bekommt eine **entschiedene und
prüfbare Spezifikations-Grundlage**, bevor implementiert wird. ADR-0007
entscheidet die Lösungsfragen, die Spezifikation wird daran geschärft —
ADR schärft die Spezifikation, nie das Lastenheft.

## 2. Definition of Done

- [x] **ADR-0007 „Geometrie-Basis der Raumerkennung" accepted** (Optionen
      mit Trade-offs, MADR-Form) und im ADR-Index ergänzt. Die ADR
      entscheidet **zwei** Lösungsfragen:
      (a) **Polygon-Basis** — Wand-Mittellinie vs. Innenkante (Wirkung
      auf die spätere Wohnflächenberechnung LH-FA-EVL-003 als Trade-off
      benennen; bei Innenkante ist der nötige Offset-Schritt
      Wandstärke/2 Teil der Entscheidung — der Algorithmus folgt der
      ADR, nicht umgekehrt);
      (b) **Raum-Repräsentation bei Verschachtelung** — wie der äußere
      Raum verschachtelter Wandzüge dargestellt wird (Polygon mit Loch
      vs. einfaches Polygon mit entkoppelter Netto-Fläche), sodass
      „keine Flächen-Doppelzählung" (LH-FA-ROM-001 Boundary)
      darstellbar und testbar ist. Wirkung auf `rooms.polygon_json`
      (ADR-0006, `spec/data-model.yaml`) als Konsequenz benennen.
- [x] **`spec/spezifikation.md` §1 (LH-FA-ROM-001.a) präzisiert:**
      (1) Polygon-Basis und Verschachtelungs-Repräsentation gemäß
      ADR-0007;
      (2) **`E-GEO-002`-Auslösebedingung für die Raumerkennung
      definiert** (welcher Input erreicht die Erkennung „degeneriert",
      wo Null-Längen-Wände schon bei LH-FA-WAL-001 verworfen werden —
      z. B. kollabierende Zyklen unterhalb der Toleranz; konkrete
      Festlegung im Slice);
      (3) **Welle-1-Einschränkung festgehalten:** Graph-Knoten sind
      Endpunkte (Punkt-Gleichheit über `GEOMETRY_TOLERANCE_MM`);
      Schnittpunkte als Knoten erst mit Wandverschneidung
      (LH-FA-WAL-006);
      (4) **Erkennungs-Trigger präzisiert:** „automatisch … when er
      geschlossen wird" (LH-FA-ROM-001 Happy) heißt: die Erkennung wird
      bei Modell-Mutation (Wand anlegen/ändern) im Service ausgelöst —
      kein manueller Abruf-Schritt nötig; UI-Ereignisse sind
      Adapter-Belang. Der Lastenheft-Wortlaut bleibt unverändert
      maßgeblich.
- [x] **§7-Pflege:** der Offene-Punkt **„Polygon-Basis"** wird
      geschlossen; der zweite Raumerkennungs-Punkt
      (Performance-Zielkomplexität, M3) bleibt ausdrücklich offen.
      `make gates` grün (docs-check über die geänderten Spec-/
      ADR-Dateien); Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0007-raumerkennung-geometrie-basis}.md` | neu | Lösungs-Entscheidung (a)+(b), MADR-Form, schärft `spezifikation.md` |
| `docs/plan/adr/README.md` | ändern | ADR-Index ergänzen (AGENTS §4) |
| `spec/spezifikation.md` | ändern | §1 präzisieren (vier Punkte aus DoD-2); §7 Punkt „Polygon-Basis" schließen |
| diese Datei + [`slice-009b`](../open/slice-009b-raumerkennung-implementierung.md) | ändern | Frontmatter-`adr_refs` um ADR-0007 ergänzen, sobald die Datei existiert |

## 4. Trigger

- Keiner — sofort startbar (reine Doku-/Entscheidungsarbeit; Wände als
  Segmente existieren seit slice-003a). ✓

## 5. Closure-Trigger

- DoD vollständig, ADR-0007 `Accepted`, `make gates` grün,
  Closure-Notiz geschrieben → [slice-009b](../open/slice-009b-raumerkennung-implementierung.md)
  wird startbar.

## 6. Risiken und offene Punkte

- Die `E-GEO-002`-Bedingung (DoD-2 Punkt 2) ist eine echte
  Spec-Entscheidung, kein Formalakt — wenn sich kein realistischer
  Degenerations-Input findet, ist die ehrliche Antwort „Raumerkennung
  wirft kein `E-GEO-002`" und der Negativtest in 009b entfällt
  begründet (Spec sagt das dann explizit).
- Wohnflächenberechnung (LH-FA-EVL-003, welle-3) hängt an der
  Polygon-Basis — die ADR muss den Trade-off dokumentieren, nicht
  lösen.

## 7. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- ADR-0007 mit Status `Accepted`, Index-Zeile und
  Folgepflicht-Eintrag (Umsetzung → slice-009b) im ADR-Index.
- `spec/spezifikation.md` §1: alle vier DoD-Punkte umgesetzt
  (Innenkanten-Basis + Ring-Modell; `E-GEO-002`-Entscheidung
  „Erkennung total, kein Fehler"; Endpunkt-Knoten-Einschränkung
  welle-1; Auslösung bei Modell-Mutation). §7: Punkt „Polygon-Basis"
  als entschieden markiert, Performance-Punkt (M3) bleibt offen.
  §8-Historie-Zeile ergänzt.
- `make gates` grün (docs-check deckt ADR, Spec, Index und beide
  Slice-Pläne ab).

**Lerneintrag:**

- **Spec-Lücke geschlossen + geschärfte Regel (Kandidat):** Die offene
  `E-GEO-002`-Frage (Plan-Review-Finding M3) löste sich nicht durch
  eine Auslösebedingung, sondern durch eine Klassen-Entscheidung:
  *Fehler-Codes gehören zu mutierenden Operationen; ableitende
  Berechnungen (Erkennung, künftig Auswertungen) sind total und
  liefern leere Ergebnisse statt Fehler.* Beim zweiten Vorkommen
  (ROM-002/003 oder EVL) als Konvention/`MR-<NNN>` verallgemeinern
  (Steering Loop: nicht beim ersten Mal).
- **Konsequenz weitergereicht:** `rooms.polygon_json` (ADR-0006) muss
  bei Persistierung die Ring-Struktur tragen — im ADR-0007
  §Konsequenzen festgehalten; ADR-0006 bleibt unverändert gültig
  (JSON-Form ist bewusst formvariabel).

**Restrisiko / Nachfolge:** Umsetzung inkl. Boundary-Netto-Flächen-Test
in slice-009b (ADR-Folgepflichten-Tabelle); Offset-Robustheit gegen
T-Stöße erst mit LH-FA-WAL-006 (Re-Evaluierungs-Trigger in ADR-0007).

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF
- **Konventionen-Dichte:** hoch — AK-Format, §7-Offene-Punkte-Disziplin,
  ADR-Schärfungs-Regel (`harness/conventions.md` MR-001).
- **Phase-Reife:** Phase 3 (§1-Algorithmus Outline mit deklariertem
  offenem Punkt — genau den schließt der Slice).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF
- **Konventionen-Dichte:** hoch — ADR-Konvention (MADR, Accepted
  immutable, Index-Pflicht), Slice-Lifecycle.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).
