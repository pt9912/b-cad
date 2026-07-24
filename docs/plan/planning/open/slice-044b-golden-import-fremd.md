---
id: slice-044b
titel: Golden files — Import-Golden fremd (IFC/DXF anderer Tools) — die 044b-Naht
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0013](../../adr/0013-ifc-bibliothek.md), [ADR-0015](../../adr/0015-dxf-backend.md)]
---

# Slice 044b: Import-Golden fremd (IFC/DXF) — Fremd-Codec-Konformanz

**Status:** open — **Skelett** ([MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)(3):
Scope-Reservierung + Vorgänger-Verweis, klar als Skelett markiert; **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
+ Detail-Schnitt folgen beim Start**). Ausgegliedert aus dem 044-Umbrella-Plan; nutzt die von
**[`slice-044a`](../done/slice-044a-golden-export-infra.md)** (done-Kandidat) gelieferte Infrastruktur
(`BCAD_TEST_GOLDEN_DIR`, `.gitattributes`, `golden`-Baum).

**Welle:** welle-5-erweiterung. **QA-/Test-Infrastruktur-Slice.** **Vorgänger:** slice-044a.

## 1. Ziel (Skelett)

**Import-Golden fremd (nur IFC + DXF — die einzigen Import-Formate):** committete, **fremd-erzeugte** Dateien
(anderer Codec/Tool) als Eingabe; ein Test importiert über den echten `ExchangeService` und prüft das
Ergebnis-Modell → **Fremd-Codec-Konformanz** (belegt, dass b-cad *anderer Tools* Bytes liest, nicht nur seinen
eigenen Roundtrip). STEP/STL/PDF/PNG haben **keinen** Import → kein Fremd-Import-Golden.

## 2. Reservierter Scope / bekannte Prüfsteine (beim Start schärfen)

- **Fremd-Dateien beschafft + gevettet + committet** unter `tests/adapters/golden/foreign/`:
  - **IFC** aus [buildingSMART/Sample-Test-Files](https://github.com/buildingSMART/Sample-Test-Files) (**CC-BY-4.0**).
    **MED-1 (blockierte den 044b-Start im Review):** b-cads IFC-Import wirft
    [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (Ganzdatei-Ablehnung), wenn
    *einer* `IfcWall` die `RepresentationIdentifier='Axis'`-Polyline **oder** die
    `IFCRELCONTAINEDINSPATIALSTRUCTURE`-Verortung fehlt → die Fixture muss **je Wand** beides tragen (ggf. eine
    minimale konforme IFC4 gezielt erzeugen).
  - **DXF** aus einem **MIT**-Repo ([ezdxf](https://github.com/mozman/ezdxf)/[ixmilia](https://github.com/ixmilia/dxf)
    bevorzugt; [netDxf](https://github.com/haplokuon/netDxf) meiden — Lizenz-Historie); eine **2D-`LINE`**-Grundriss-
    Datei (b-cads DXF ist **2D-only**, [ADR-0015](../../adr/0015-dxf-backend.md) — 3D wie `3DFACE` importiert 0 Entities).
  - **Vetting-Pflicht:** jede Kandidatin **durch b-cads Importer jagen**; nur behalten, wenn sie importierbaren
    Inhalt (≥1 Wand/Linie) trägt.
- **Provenance-/Attributions-Manifest** `tests/adapters/golden/foreign/PROVENANCE.md`: je Datei Quelle-URL, Lizenz,
  Attributions-Text, `sha256`; CC-BY-Namensnennung (buildingSMART). Token-frei
  ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371)).
- **Import-Golden-Test** `tests/adapters/test_golden_import.cpp`: importiert je (kuratierte) Fremd-Datei über den echten
  `ExchangeService`; Zähl-/Wert-Orakel (Geschosse/Wände/Segmente > 0 + bekannte Werte; **kein Wurf**). Kein
  `find(...)/erase`-String-Mutations-Muster.

## 3. Trigger

- [`slice-044a`](../done/slice-044a-golden-export-infra.md) done (Infrastruktur steht).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert; MED-1-Fixture-Kuratierung als DoD.
