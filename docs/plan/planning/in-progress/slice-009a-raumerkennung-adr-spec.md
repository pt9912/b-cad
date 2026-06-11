---
id: slice-009a
titel: Raumerkennung — ADR-0007 Polygon-Basis & Spec-Schärfung
status: open
welle: welle-1-mvp
lastenheft_refs: [LH-FA-ROM-001]
adr_refs: [ADR-0001]
---

# Slice 009a: Raumerkennung — ADR-0007 Polygon-Basis & Spec-Schärfung

**Status:** open

**Welle:** welle-1-mvp

**Bezug:** LH-FA-ROM-001, ADR-0001. **Liefert:** ADR-0007 (entsteht in
diesem Slice; deshalb noch nicht in den Frontmatter-`adr_refs` — die
Referenz wird bei Closure ergänzt, wenn die ADR-Datei existiert).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-11.

**Schnitt-Herkunft:** Aufteilung des ursprünglichen `slice-009` in die
Entscheidungs-/Spec-Grundlage (**009a**, dieser Slice) und die
Implementierung
([**009b**](slice-009b-raumerkennung-implementierung.md)) — Ergebnis
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

- [ ] **ADR-0007 „Geometrie-Basis der Raumerkennung" accepted** (Optionen
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
- [ ] **`spec/spezifikation.md` §1 (LH-FA-ROM-001.a) präzisiert:**
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
- [ ] **§7-Pflege:** der Offene-Punkt **„Polygon-Basis"** wird
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
| diese Datei + [`slice-009b`](slice-009b-raumerkennung-implementierung.md) | ändern | Frontmatter-`adr_refs` um ADR-0007 ergänzen, sobald die Datei existiert |

## 4. Trigger

- Keiner — sofort startbar (reine Doku-/Entscheidungsarbeit; Wände als
  Segmente existieren seit slice-003a). ✓

## 5. Closure-Trigger

- DoD vollständig, ADR-0007 `Accepted`, `make gates` grün,
  Closure-Notiz geschrieben → [slice-009b](slice-009b-raumerkennung-implementierung.md)
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

*(bei Closure zu füllen: beobachtbare Kriterien + Lerneintrag)*

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
