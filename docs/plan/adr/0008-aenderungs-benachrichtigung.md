# ADR-0008: Änderungs-Benachrichtigung Kern → Darstellung (Observer-Port, Push-Notify/Pull-State)

**Status:** Accepted

**Datum:** 2026-06-11

**Autor:** Dietmar Burkard

**Bezug:** LH-FA-D3-002 (Echtzeitaktualisierung), OBJ-003 (eine Quelle,
2D- und 3D-Sicht), ADR-0001 (Kern führt, Ports & Adapters), ADR-0007
(Raum-Re-Detektion als Post-Commit-Schritt)

---

## Kontext

LH-FA-D3-002 verlangt, dass Modelländerungen ohne expliziten
Aktualisierungs-Schritt des Benutzers in der Darstellung ankommen. Der
inkrementelle, transaktionale Rebuild des betroffenen Solids existiert
seit slice-003a; die Raum-Re-Detektion (ADR-0007) läuft seit
slice-009b als Post-Commit-Schritt. Offen war, **wie** die
Darstellungs-Schicht (2D und 3D, OBJ-003) von committeten Änderungen
erfährt — ohne die hexagonale Richtung zu verletzen (der Kern kennt
keine Adapter, ADR-0001).

## Entscheidung

1. **Mechanik: Observer-/Notifikations-Port (driven), synchroner
   Aufruf im Mutationspfad.** Der Kern definiert einen driven Port
   (Beobachter-Schnittstelle); Adapter registrieren sich und werden
   nach jeder committeten Modell-Mutation synchron aufgerufen. Kein
   Polling, keine asynchrone Queue (welle-1 ist single-threaded; siehe
   Alternativen).
2. **Vertrag: Push-Notify, Pull-State.** Gemeldet werden nur
   `element_id` und `op` (Operations-Art) — Vokabular deckungsgleich
   mit dem OTel-Span `bcad.geometry.rebuild` (`element_id`, `op`,
   `spezifikation.md` §5). Den aktualisierten **Stand** holt der
   Beobachter über die bestehenden Abfrage-Ports/Queries (Solid,
   Räume, Modell). Kein Zustands-Payload in der Meldung — das
   vermeidet Kopien und hält den Vertrag stabil, wenn Bauteil-Typen
   wachsen.
3. **Umfang: alle committeten Modell-Mutationen melden** —
   Geschoss-Anlage, Wand-Anlage, Wand-Parameteränderung; nach einer
   Wand-Mutation zusätzlich eine Raum-Änderungs-Meldung für das
   betroffene Geschoss (die 2D-Sicht hört auf Räume). Mehrere
   Meldungen pro Mutation sind zulässig (Mehr-Element-Updates, etwa
   künftige Wandverschneidung LH-FA-WAL-006, sind damit nicht
   verbaut). Abgelehnte/verworfene Mutationen (`E-VAL-001` Rejected,
   Null-Längen-Wand) melden nicht.
4. **Reihenfolge: Meldung nach allen Post-Commit-Schritten.** Erst
   Commit, dann Raum-Re-Detektion (ADR-0007), dann Benachrichtigung —
   ein Beobachter, der im Callback den Stand abfragt (Pull), sieht
   einen vollständig konsistenten Zustand.
5. **Multiplizität und Registrierung:** mehrere Beobachter (2D- und
   3D-Sicht, OBJ-003) über `subscribe`/`unsubscribe` am Service;
   der Service hält nicht-besitzende Referenzen, Beobachter melden
   sich vor ihrer Zerstörung ab (Lebenszyklus-Pflicht der
   Adapter-Seite).
6. **Fehlerverhalten und Re-Entranz:** Der Service **kapselt** jeden
   Beobachter-Aufruf — eine werfende Beobachter-Implementierung kippt
   die committete Mutation nicht und unterbricht die Meldung an
   weitere Beobachter nicht (Fehler wird verschluckt; sichtbar gemacht
   wird er später über die REQ-TEC-006-Telemetrie). Callbacks dürfen
   **Abfragen** ausführen, aber **keine Mutationen** auslösen
   (Re-Entranz-Verbot als Vertrags-Pflicht; technische Durchsetzung
   wird mit dem Plugin-System neu bewertet, siehe
   Re-Evaluierungs-Trigger).

## Verglichene Alternativen

### Option A — Observer-Port, synchroner Push (gewählt)

- Pro: „ohne expliziten Schritt" direkt erfüllt; deterministisch
  testbar (zählendes Double); keine Thread-/Queue-Komplexität in
  welle-1; hexagonal sauber (driven Port, Kern kennt keine Adapter).
- Contra: Beobachter laufen im Mutationspfad — ein langsamer Hörer
  bremst die Mutation (akzeptiert für welle-1; ein Latenz-Budget wäre
  eine eigene `LH-QA`-Anforderung).

### Option B — Polling durch den Adapter

- Pro: Kern bleibt komplett passiv, kein Beobachter-Lebenszyklus.
- Contra: Entweder Poll-Takt (Latenz + Leerlauf-Arbeit) oder
  Versions-Zähler-Abfrage vor jedem Frame — die „sofort"-Eigenschaft
  hinge am Takt des Adapters statt am Modell; Tests müssten Takt
  simulieren.

### Option C — Event-Queue (asynchron)

- Pro: entkoppelt Mutations- und Darstellungs-Pfad; trägt spätere
  Nebenläufigkeit.
- Contra: Reihenfolge-/Verlust-Semantik und Thread-Fragen, die
  welle-1 (single-threaded, ein Fenster) nicht hat; „sofort" wird von
  der Queue-Abarbeitung abhängig; deutlich teurere Tests.

## Konsequenzen

- Positiv: 2D- und 3D-Sicht (OBJ-003) hängen an demselben Vertrag;
  der Kern bleibt framework-frei; LH-FA-D3-002-AK sind mit einem
  zählenden Port-Double prüfbar (slice-010b).
- Folgepflicht (slice-010b): Port + `subscribe`/`unsubscribe` +
  Meldungen in `StructureEditService` (Reihenfolge nach
  `redetectRooms`), inkl. Kapselungs-Test (werfender Beobachter).
- Die Benachrichtigung ist nach der Raum-Re-Detektion das **zweite
  Vorkommen** eines nicht-werfenden Post-Commit-Schritts. Die
  Verallgemeinerung zur Konvention (`MR-<NNN>`) ist **vertagt**
  (Steering Loop: zweimal = kategorisieren, dreimal = Regel) — auch
  weil die 009-Klasse („ableitende Berechnungen sind total") dafür
  auf „Post-Commit-Schritte" verbreitert werden müsste, was eine
  eigene Begründung braucht.
- `main.cpp`/Composition Root verdrahtet künftige Sichten als
  Beobachter; bis dahin existiert schlicht kein Registrierter (leere
  Beobachter-Liste ist der Normalfall der Tests von 003a/009b und
  bleibt gültig).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| AK-Tests (slice-010b) | Meldung nach Commit/Re-Detektion; Rejected/verworfen → keine Meldung; werfender Beobachter kippt nichts und blockiert Folge-Beobachter nicht | `make test` |
| Schichtung | Port liegt unter `src/hexagon/ports/driven/`, kein Adapter-Import im Kern | `make arch-check` (ADR-0001) |

## Re-Evaluierungs-Trigger

- Nebenläufigkeit/Mehr-Fenster (Qt-Viewer-Strang, Mehrmonitorbetrieb
  LH-FA-UI-004) → Option C (Queue) neu bewerten.
- Plugin-System (LH-FA-PLG-*): fremder Code als Beobachter → Kapselung
  und Re-Entranz-Verbot technisch durchsetzen (Sandbox, LH-FA-PLG-004),
  nicht nur vertraglich.
- Ein Rebuild-/Melde-Latenz-Budget wird Anforderung (neue
  `LH-QA-<NNN>`) → synchronen Pfad gegen Budget messen.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-11 | Proposed (aus Plan-Review 010, Entscheidungs-Pflichten a–e) | slice-010a |
| 2026-06-11 | Accepted — Lastenheft-AK geschärft, spez. §1 D3-002.a ergänzt | slice-010a |
