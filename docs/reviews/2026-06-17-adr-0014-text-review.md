# Text-Review — ADR-0014 (STEP-/STL-Export-Backend), vor Accept

## Kopf

- **Review-Art:** Unabhängiges ADR-**Text-Review** vor Accept (Reviewer ≠ Autor, ohne Autoren-Kontext; Modul-10-Linse: Plan/Entscheidung gegen Spec + Accepted-ADRs, vor Diff — Reviewer kategorisiert, schlägt keine Lösung vor). Dieselbe Disziplin wie bei [`ADR-0013`](../plan/adr/0013-ifc-bibliothek.md).
- **Gegenstand:** `docs/plan/adr/0014-step-stl-export-backend.md` (Proposed) — STEP/STL-Export über OCC-DataExchange, geometrie-residenter `ModelExporterPort`.
- **Datum:** 2026-06-17.
- **Eingangs-Kontext (gelesen):** `docs/plan/adr/0002-geometrie-kern-opencascade.md` (Ausgliederung), `0013-ifc-bibliothek.md` (Schwester-ADR), `0004-toolchain-dependency-pinning.md`; `spec/architecture.md` §1.2; `spec/lastenheft.md` `LH-FA-IO-005`/006; `spec/spezifikation.md` §4/§6/§7; `src/hexagon/ports/driven/model_exporter_port.h`, `src/hexagon/services/exchange_service.{h,cpp}`, `src/adapters/io/ifc_export_adapter.h`, `src/adapters/geometry/occ_geometry_adapter.h`; `tools/arch-check.sh`; `docs/plan/adr/README.md`.

**Verdikt: 0 HIGH / 2 MED / 3 LOW / 1 INFO. Accept nicht blockiert.**

## Quellen-Konsistenz (alle PASS)

- **ADR-0002-Ausgliederung** verifiziert (0002 Z. 30–33: STEP-/Format-Export → eigene IO/Export-ADR inkl. Geometrie↔IO-Grenze).
- **ADR-0013-Schwester-Framing** verifiziert (0013 Option C: OCC-DataExchange = richtige Antwort fürs STEP/STL-Schwester-ADR, disqualifiziert für IFC).
- **`ModelExporterPort` §1.2 deklariert + real (Code)** verifiziert: Header deklariert mit slice-019b, reale `IfcExportAdapter`-Impl + Verdrahtung slice-019c.
- **`IfcExportAdapter` in `io/`, `OccGeometryAdapter` in `geometry/`** (OCC im `.cpp` gekapselt) verifiziert.
- **arch-check Regel C** (OCC `.hxx` nur in `geometry/`) **+ Regel B** (kein Adapter→Adapter) wörtlich verifiziert.
- **`LH-FA-IO-005`/006 bare Outline** (keine AK) verifiziert → „AK-Schärfung = eigener Slice (`MR-008`)" korrekt, Präzedenz slice-019a.
- **E-IO-001/E-GEO-002** (§4) + **ADR-0004-Konformität** (OCC ist bereits gepinnte apt-Abhängigkeit; zusätzliche Toolkits derselben Distribution = kein neuer Paketmanager) verifiziert.
- **ADR-Index aktualisiert** (0014-Zeile Proposed + offene-Themen-Zeile gestrichen) verifiziert.

## Entscheidungs-Tragfähigkeit (PASS)

- **Schicht-Crux SOUND:** OCC-Exporter in `geometry/` erfüllt Regel C (OCC dort erlaubt) + Regel B (kein Adapter→Adapter; Composition Root verdrahtet je Format) + ADR-0001 (Kern kennt nur den Port). „Ein Port darf Implementierungen in verschiedenen Adaptern haben" ist legitimes Hexagonal-Design, keine versteckte Kopplung. Verworfene Alternative „Exporter in `io/`" korrekt als Regel-C/B-Bruch gezeigt.
- **Backend SOUND:** „OCC-nativ, keine neue Dependency" korrekt (OCC gesetzt, ADR-0002); Ablehnung eigener STEP-Writer (Option B) wegen STEP-Schema-Größe gut begründet (Asymmetrie zu IFC Option D korrekt).
- **Dispatch-Deferral LEGITIM:** `exchange_service.cpp` hält bereits Importer+Exporter als Referenzen + `switch(format)`; STEP/STL braucht nur neue Enum-Werte + Konstruktor-Arme + Composition-Root-Verdrahtung — **kein** Kern-Redesign der Port-Semantik. Deferral der Mechanik in den Impl-Slice = ADR-0001-Kern-Hoheit (Präzedenz ADR-0013).
- Scope/Alternativen/Fitness/Re-Eval vollständig und beobachtbar.

## Findings

### MED-1 — ADR-0002-Zitat lässt die „in `src/adapters/io/`"-Klausel weg, die ADR-0014 dann *überstimmt*
ADR-0002 vermutete `io/`, ließ die Grenze aber bewusst offen; ADR-0014 entscheidet `geometry/`. Sauber (0002 ließ es offen), aber der ADR las sich wie reines „erfüllt" statt „löst eine offene Frage gegen 0002s Platzhalter". **Fix (eingearbeitet):** §Entscheidung #2 stellt explizit klar, dass 0002 `io/` *vermutete* und die Grenze offen ließ — diese ADR entscheidet sie zu `geometry/`, ohne 0002 zu widersprechen.

### MED-2 — Toolkit-Verfügbarkeit (`TKDESTEP`/`TKDESTL`/`TKRWMesh`) ist hedged, aber ohne beobachtbaren Beleg bis Build-Zeit
OCC-7.9.x hat DataExchange in `TKDE*` reorganisiert; die Namen sind plausibel, aber gegen den gepinnten Snapshot unverifiziert. Risiko: fehlt ein DataExchange-dev-Paket, bricht das „keine neue Dependency"-Versprechen in eine ADR-0004-Berührung. **Fix (eingearbeitet):** Folgepflicht verlangt nun einen beobachtbaren Impl-Slice-Beleg (`apt-get install -s libocct-data-exchange-dev` o. Ä., spike-001-Muster); löst er nicht auf → neuer apt-Eintrag = ADR-0004-Berührung, eigener Beschluss.

### LOW-1 — „`ModelExporterPort` mit slice-019c real eingeführt" vermischt Port-Deklaration und Adapter-Impl
Port-Header deklariert 019b, reale Exporter-Impl 019c. **Fix (eingearbeitet):** „Header deklariert slice-019b, erste reale Implementierung `IfcExportAdapter` slice-019c".

### LOW-2 — `ExchangeService` injiziert heute genau einen Exporter; Erweiterung trivial, aber benennenswert
Nicht versteckt (Folgepflicht nennt „Multi-Exporter"). **Akzeptiert** ohne weitere Änderung (kein Korrektheitsproblem).

### LOW-3 — STL „ASCII optional" greift in die deferierte AK
**Fix (eingearbeitet):** „binär als Backend-Default; ob ASCII zusätzlich angeboten wird, ist AK → Schärfungs-Slice".

### INFO — Spec-Nachzug §6/§7 ist real fällig (nicht nur „falls")
`spezifikation.md` §7 listet STEP-/STL-Backends als offen; mit Accept stale. **Fix (eingearbeitet):** Folgepflicht firmiert (definitiv, §7-Zeile streichen, §6 nachziehen) + bei Accept als ADR-Index-Folgepflicht-Zeile buchen (analog ADR-0013).

## Ergebnis

Keine HIGH-Findings; **Accept nicht blockiert.** Die Schicht-Entscheidung ist architektonisch korrekt, quellen-konsistent und folgt der ADR-0013-Schwester-ADR-Disziplin. MED-1 + MED-2 (+ LOWs/INFO) vor Accept eingearbeitet. Empfehlung: **Accept** durch den Projektinhaber.
