# MR-009 Geometrie-Code-Review — slice-024a (Dächer STEP-B-Rep)

**Datum:** 2026-06-19. **Reviewer:** unabhängig (≠ Autor, ohne Autoren-Kontext),
adversarial, Geometrie-Korrektheits-Fokus. **Gegenstand:** die slice-024a-Implementierung
(`meshToSolid` + Dach-Schleife in `buildSolidCompound` + OCC-freies STEP-Orakel) gegen die
023b-Wasserdichtheits-Garantie und die Spec, per
[MR-009](../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
(vor Welle-Closure; HIGHs blockieren). **Review-only.**

## Empirische Verifikation (Wegwerf-OCC-Proben, vom Reviewer wieder entfernt)
- **Alle Dachtypen + Zeltdach-Apex** (Sattel/Walm/Pult + quadratischer Walm) durch den echten
  STEP-Pfad: je **genau eine** `CLOSED_SHELL` + `MANIFOLD_SOLID_BREP`. Härtester Fall (Apex) robust.
- **Orientierung + Volumen exakt:** `BRepGProp::VolumeProperties` der echten Dach-Solids ==
  `bx·ty·d` bit-exakt (Sattel/Walm/Pult 12,6e9 mm³; Zeltdach 9,8e9 mm³). Die 023b-Außen-Orientierung
  wird durch Sewing+`MakeSolid` getreu getragen; 0,1-mm-Toleranz vs. µm-Raster verschmilzt keine
  echten Kanten.
- **fail-closed greift für offene Shells** (Würfel mit fehlender Fläche → leer).
- **fail-closed greift NICHT für invertierte Shells:** ein topologisch geschlossener, **invers
  gewickelter** Würfel besteht `BRepCheck_Analyzer.IsValid()` → wird (inside-out) zurückgegeben
  (MED-1).

## Findings

### MED-1 — `BRepCheck_Analyzer.IsValid()` fängt keine invertierte (inside-out) Solid
`IsValid()` prüft **Geschlossenheit, nicht Orientierung**. Heute **nicht erreichbar** (`roofMesh`
ist durch ein starkes Orakel außen-orientiert: `test_roof_geometry.cpp` assertiert `signedVolume ==
bx·ty·d`, nicht nur `> 0`; Proben bestätigen positive Volumina). Aber der **geteilte** Helfer
(024b-Reuse) überzeichnete seinen Vertrag.
→ **Eingearbeitet (Option b):** Header-/Vertrags-Wortlaut **geschärft** — `meshToSolid` garantiert
„*eine gültige, **geschlossene** Shell*"; Außennormalen-Korrektheit ist **vom Eingabe-Netz geerbt**
(Aufrufer-Vertrag, für Dächer 023b-Invariante), hier nicht erzwungen. Kein Volumen-Sign-Guard
ergänzt (nicht erreichbar; vermeidet TKGProp-Erweiterung in `occ_solids`).

### MED-2 — Dach-Schleife ohne Per-Bauteil-`try/catch` (Parität Wände/Decken)
Wände (`:59-69`) und Decken (`:71-84`) kapseln je `try { … } catch (std::exception&) { continue; }`
→ ein degeneriertes Bauteil wird übersprungen, der Rest exportiert. Die Dach-Schleife hatte das
**nicht** → ein OCC-`Standard_Failure` aus `Sewing`/`MakeSolid` hätte den **ganzen** Export
abgebrochen (E-IO-002) statt nur das eine Dach zu überspringen.
→ **Eingearbeitet:** Dach-Schleife in `try { … } catch (const std::exception&) { continue; }`
gekapselt (Parität, Totalität).

### LOW-1 — STL/STEP-Divergenz eines fail-closed-Dachs benennen
STL exportiert jedes nicht-leere `roofMesh`; STEP zusätzlich nur bei `meshToSolid`-Erfolg. Ein nicht
geschlossen vernähbares Dach ist STL-präsent / STEP-abwesend.
→ **Eingearbeitet:** §1 `LH-FA-IO-005.a` benennt die Divergenz explizit.

### LOW-2 — Test-Orakel deckte nur Sattel (016b/020b-Lehre)
Die Fixture testete nur ein Satteldach; Walm/Pult/Zeltdach-Apex (härtester Wasserdichtheits-Fall)
liefen ungeprüft durch den Sewing-Pfad.
→ **Eingearbeitet:** `RoofYieldsClosedShellBRepSolid` über **{Sattel, Walm, Pult, Walm-Zeltdach}**
parametriert (je `+1 CLOSED_SHELL`). `make gates` grün (206/206).

### INFO
- **INFO-1:** Single-Shell-Ablehnung (`0`/`>1` Shell → leer) korrekt für die Rechteck-Grundriss-
  Domäne (ein zusammenhängendes Dach).
- **INFO-2:** Index-Zugriff `idx[t]*3` unbounded, aber durch `roofMesh`-Konstruktion sicher (kontigue
  positions/indices); für den general-purpose-Helfer notiert.
- **INFO-3:** OCC-Kapselung (Regel C) sauber — `arch-check` grün, kein OCC-Typ-Leck über
  `ModelExporterPort`, `write()` neutralisiert `Standard_Failure` → `std::runtime_error` (E-IO-002);
  `TKShHealing` aus derselben OCC-Distribution (kein neuer `find_package`, ADR-0004 intakt); atomarer
  Temp+fsync+Rename wie STL/IFC.

## Verdikt

Keine erreichbare Geometrie-Korrektheits- oder fail-closed-Lücke für die aktuelle Dach-Domäne:
Vernähung empirisch wasserdicht, orientierungs-treu und volumen-exakt für alle drei Typen + Zeltdach.
MED-1/MED-2 sind reale Robustheits-/Vertrags-Lücken des geteilten Helfers (024b-Reuse) — behoben,
erzeugen aber **kein** falsches STEP-Solid heute.

**VERDICT: 0 HIGH, 2 MEDIUM, 2 LOW, 3 INFO — CLOSURE CLEARED**

## Einarbeitung (Autor, 2026-06-19)
MED-1 (Vertrags-Wortlaut geschärft: geschlossen ja, Orientierung geerbt), MED-2 (Dach-Schleife
`try/catch`-Parität), LOW-1 (§1-Divergenz-Satz), LOW-2 (Test über alle Dachtypen + Apex) eingearbeitet;
`make gates` grün (206/206). **0 HIGH → Closure frei.** *(Hinweis: der Review-Agent hatte beim
empirischen Proben-Lauf die unkommittierten Test-Änderungen zurückgesetzt; Fixture/Helfer/Test wurden
in der parametrierten LOW-2-Form neu eingespielt und verifiziert.)*
