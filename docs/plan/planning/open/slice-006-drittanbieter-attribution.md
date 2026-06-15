---
id: slice-006
titel: Drittanbieter-Lizenz-Attribution & Auslieferungs-Layout
status: open
welle: unzugeordnet (Release-Vorbereitung — siehe docs/user/releasing.md; Roadmap hat noch keine Release-Welle, §6)
lastenheft_refs: []  # [LH-QA-007](../../../../spec/lastenheft.md) (Attributions-Pflicht) vorgeschlagen, noch NICHT geschrieben — Vorbedingung, siehe §4/§6
adr_refs: [[ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md), [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md)]  # 0002: offene Lizenz-Notiz „GPL-Teile prüfen"; 0004: gepinnte Dep-Versionen = Attributions-Grundlage; 0005: Tooling+Layout (Proposed — Annahme noch offen)
---

# Slice 006: Drittanbieter-Lizenz-Attribution & Auslieferungs-Layout

**Status:** open

**Welle:** unzugeordnet (Release-Vorbereitung — an welle-1-mvp-Closure gekoppelt, siehe [`releasing.md`](../../../user/releasing.md))

**Bezug:** [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (offene Lizenz-Notiz), [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) (gepinnte Dep-Versionen = Attributions-Grundlage), [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md) (Attribution-Tooling & Layout, *Proposed*). **Ausstehend:** Annahme von [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md), [LH-QA-007](../../../../spec/lastenheft.md) (Attributions-Pflicht, noch nicht im Lastenheft) — siehe §4/§6.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-08.

---

## 1. Ziel

Jedes ausgelieferte b-cad-Artefakt trägt die vollständige
Drittanbieter-Lizenz-Attribution in einem festen Layout. Aus dem
reproduzierbaren `make fullbuild` entsteht der Baum:

```
dist/
├── bin/b-cad
└── share/doc/b-cad/
    ├── LICENSE                     (eigenes Projekt, MIT)
    ├── NOTICE                      (generiert: Copyright-Statements)
    ├── THIRD_PARTY_LICENSES.md     (generiert: Dep → SPDX → Text-Verweis)
    └── LICENSES/                   (kanonische Volltexte)
        ├── MIT.txt
        ├── LGPL-2.1-only.txt       (OpenCASCADE)
        ├── LGPL-3.0-only.txt       (Qt)
        └── OCCT-exception.txt      (Nicht-SPDX → manuell)
```

Die SPDX-IDs im Baum sind **illustrativ** — verbindlich ist das
kuratierte Manifest (§2), das gegen das real gelinkte Binary aufgelöst
wird.

Damit wird die offene Lizenz-Notiz aus
[ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) („Lizenz
(GPL-Teile) prüfen") strukturell aufgelöst und der in
[`releasing.md`](../../../user/releasing.md) als „spätere Welle"
zurückgestellte Distributions-Aspekt eingelöst.

## 2. Definition of Done

- [ ] **Manifest & Volltexte:** kuratiertes `tools/licenses-manifest.*` (Dep → Version → SPDX-ID → Quelle) als Single Source of Truth; `LICENSES/*.txt` via REUSE gefüllt, `OCCT-exception.txt` als Nicht-SPDX manuell; ScanCode-Lauf über die gelinkten/vendored Quellen nur zur **Gegenkontrolle** (kein Auto-Inventar, C++/Linker-Realität).
- [ ] **Generierung & Install-Layout:** `NOTICE` + `THIRD_PARTY_LICENSES.md` aus Manifest (+ScanCode-JSON) generiert; CMake-`install()` erzeugt den exakten `dist/`-Baum aus §1 (`bin/b-cad` + `share/doc/b-cad/...`).
- [ ] **Gate-Promotion:** `make license-check` (Allowlist + Vollständigkeit: jede gelinkte Dep hat Manifest-Eintrag + Volltext) real im Makefile mit ID-Kommentar, in `make gates`/`make fullbuild` gehängt. Sequenz: `license-check` steht **heute nicht** in AGENTS §3 — die **Annahme von [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md)** trägt zuerst die *Geplant*-Zeile nach, dieser Slice promotet sie dann nach **Real** (Promotion-Trigger, Modul 13).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `tools/licenses-manifest.*` | neu | kuratierte Dep→SPDX-Liste (Single Source of Truth), z. B. YAML |
| `LICENSES/*.txt` | neu | kanonische Volltexte an der **Repo-Wurzel** (REUSE-Quelle + manueller OCCT-exception); CMake-`install()` kopiert sie nach `share/doc/b-cad/LICENSES/` (Auslieferungs-Ziel §1) — *ein* Baum, zwei Rollen (Implementierer pflegt nicht zwei) |
| `tools/gen-attribution.*` | neu | rendert `NOTICE` + `THIRD_PARTY_LICENSES.md` aus Manifest/Scan |
| `tools/license-check.*` | neu | Allowlist- + Vollständigkeits-Gate (Geschwister zu `suppression-gate.sh`/`arch-check.sh`) |
| `CMakeLists.txt` / `cmake/*.cmake` | Änderung | `install(TARGETS …)` + `install(DIRECTORY share/doc/b-cad/…)` |
| `Makefile` | Änderung | `make license-check` real; in `gates`/`fullbuild` aggregieren |
| `AGENTS.md` §3 | Änderung | Target von „Geplant" → „Real" promoten |
| `docs/user/releasing.md` | Änderung | Checkliste um Attributions-Punkt ergänzen |

## 4. Trigger

- **slice-002 done** — Gate-Framework + AGENTS-§3-Promotions-Mechanik stehen (`suppression-gate.sh`, `arch-check.sh`, `make gates`-Aggregation); `make license-check` ist ein Geschwister-Sensor.
- **Reales Release-Binary existiert** — `bin/b-cad` mit echt gelinkten Drittanbieter-Deps (Qt/OCC/SQLite), d. h. `make fullbuild` herstellbar. *Ohne reales Binary ist Attribution Fiktion* — harter Sequenz-Constraint (AGENTS §3: „noch kein Code").
- **ADR-Kette accepted + [LH-QA-007](../../../../spec/lastenheft.md) im Lastenheft** — die Attribution begründet sich aus einer Kette noch *Proposed*er bzw. ungeschriebener Entscheidungen, die alle vor Aktivierung **Accepted** sein müssen (Annahme-, kein Schreib-Trigger für die existierenden):
  - [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) — OCC-Bindung = Lizenz-Prämisse (LGPL-2.1 + OCCT-exception). *Accepted (2026-06-09).*
  - **Qt-Framework-Bindung** — liefert die LGPL-3.0-Prämisse, ist aber **noch kein ADR** (offenes ADR-Thema, [adr/README §Offene Themen](../../adr/README.md)). Reale Vorbedingungs-Lücke.
  - [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) — gepinnte Versionen = Manifest-Grundlage (Dep→Version). *Accepted (2026-06-09).*
  - [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md) — Tooling + Layout. *Proposed* (existiert; offen nur die Annahme).
  - **[LH-QA-007](../../../../spec/lastenheft.md)** (Attributions-Pflicht, abnahmebindend) — real noch nicht im Lastenheft. Dieser Slice setzt um, schreibt das Requirement nicht.

## 5. Closure-Trigger

- DoD vollständig; `make license-check` + `make gates` grün; `dist/`-Baum aus `make fullbuild` reproduzierbar; Closure-Notiz mit Lerneintrag geschrieben.

## 6. Risiken und offene Punkte

- **Spec-Vorbedingung offen:** [`ADR-0005`](../../adr/0005-drittanbieter-lizenz-attribution.md) (Tooling=ScanCode+REUSE, `dist/`-Layout, OCCT-exception als Nicht-SPDX, kuratiertes Manifest statt Auto-Scan) existiert als *Proposed* und muss vor Aktivierung **Accepted** sein. [`LH-QA-007`](../../../../spec/lastenheft.md) (Attributions-Pflicht, abnahmebindend) ist **noch nicht im Lastenheft** — echte offene Vorbedingung. Beide sind Vorbedingung dieses Slice, nicht Teil davon — Source-Precedence-Disziplin (AGENTS §1, §2.5).
- **slice-018b-Querverweis:** Da [`LH-QA-007`](../../../../spec/lastenheft.md) noch nicht im Lastenheft steht, hat **slice-018b** (Voll-Korpus-Link-Hygiene) seine Vorkommen mit einem **Datei-Fallback-Link** (ohne Anker, auf `lastenheft.md`) versehen — gate-gültig, aber unpräzise. **Bei Aktivierung dieses Slice** sind diese Fallback-Links auf den dann existierenden **präzisen Anker** zu heben.
- **Keine Release-Welle in der Roadmap:** [`roadmap.md`](../in-progress/roadmap.md) endet bei welle-5; Distribution ist in `releasing.md` nur „spätere Welle". Welle-Zuordnung dieses Slice offen — ggf. eigener Roadmap-Eintrag + Historische Trigger-Verschiebung.
- **C++/Linker-Realität:** ScanCode findet System-/Conan-/vcpkg-Deps ohne vorliegende Quellen evtl. nicht — deshalb kuratiertes Manifest als Wahrheit, Scan nur zur Gegenkontrolle. Die Bezugsform der Deps (vendored/Conan/vcpkg/System) bestimmt, was der Scan überhaupt sieht.
- **OCCT-exception ist Nicht-SPDX:** REUSE kann den Text nicht ziehen → manuell ablegen und im Manifest als Sonderfall markieren.
- **Größenrisiko:** drei dichte DoD-Punkte. Sollte sich der Slice bei Aktivierung als nicht in einer Review-Sitzung prüfbar erweisen, `in-progress → next` zurückführen und schneiden (Manifest/Volltexte | Generierung/Install | Gate).

## 7. Closure-Notiz

<!-- Erst nach Abschluss füllen: Lerneintrag (geschärfte Regel / neuer Sensor / Spec-Lücke). -->

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Lizenz-Tooling & Dep-Manifest

- **Modus:** GF
- **Konventionen-Dichte:** jetzt niedrig (kein Manifest-Format verankert); zur Aktivierung mittel-hoch, sobald [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md) das Format und das `dist/`-Layout festschreibt (Doc führt).
- **Phase-Reife:** Phase 0 → Phase 4 mit [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md) (Konvention steht, Tooling wird daran gemessen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig — neues `tools/`-Areal, kein Bestandscode; Risiko liegt in der Vollständigkeit der Dep-Erfassung, nicht in Doku-Drift.
- **Reconciliation-Aufwand:** keiner (GF), sofern [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md) vorab accepted.

### Sub-Area: Release-/Auslieferungs-Layout (CMake install + releasing.md)

- **Modus:** Hybrid
- **Konventionen-Dichte:** niedrig. `releasing.md` ist Phase-2-Outline und verspricht die Release-Strategie (Doc führt für das „Was"), aber das konkrete `install()`-Layout existiert nirgends; `releasing.md` schiebt Distribution explizit weg.
- **Phase-Reife:** Phase 2 → Phase 4. Hybrid-Symptom: für die *Pflicht* (Artefakt trägt Attribution) führt die Doku (GF-Richtung, via [LH-QA-007](../../../../spec/lastenheft.md)), für das *konkrete Layout* gibt es noch keinen Code — [ADR-0005](../../adr/0005-drittanbieter-lizenz-attribution.md) setzt es (Proposed, vor Aktivierung anzunehmen).
- **Evidenz-/Diskrepanz-Risiko:** niedrig — kein widersprüchlicher Bestand, eher Lücke als Drift.
- **Reconciliation-Aufwand:** ~0,5 Slice-Anteil — `releasing.md`-Checkliste + Roadmap-Release-Welle nachziehen. Graduation-Trigger: **Promotion-Trigger** (Layout-Konvention nach `harness/conventions.md` als `MR-<NNN>`, sobald der erste `dist/`-Baum steht).

### Sub-Area: Quality-Gates / Sensor-Bindung

- **Modus:** GF (vorausgesetzt slice-002 hat das Gate-Framework + Promotions-Mechanik in `harness/conventions.md`/AGENTS §3 verankert).
- **Konventionen-Dichte:** mittel-hoch — Gate-Pattern aus slice-002 (`suppression-gate.sh`, `arch-check.sh`, `make gates`-Aggregation) und die AGENTS-§3-„Geplant→Real"-Promotionsregel.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig — `make license-check` folgt dem etablierten Sensor-Schema; Risiko nur, falls slice-002 das Schema nicht stabil hinterlässt.
- **Reconciliation-Aufwand:** keiner, sofern slice-002 abgeschlossen.
