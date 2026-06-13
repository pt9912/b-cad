# Code-Review slice-015b — Decken + Fundament (Platten)

**Datum:** 2026-06-13 · **Diff:** `4b7c1f4..680fa22` (Implementierung) ·
**Reviewer:** unabhängig (AI-Harness Modul 11) · **Verfahren:** geometrie-
fokussierte Verifikation gegen die Spezifikation (nicht nur gegen Tests).

## Verdikt: NO-GO → behoben (Nachschärfung), jetzt GO

Die Kern-Mathematik der Slice ist solide und spec-konform:
**base_z je Typ** (Decke = Geschoss-Oberkante, Bodenplatte/Fundament =
−Dicke), die **MED-2-Reihenfolge** (Cutout relativ `[−ε,Dicke+ε]`,
Mesh-Translation **nach** dem Boolean) und das **`reloadKeyed`**-Refactoring
(idempotent, zählt Entfernungen, Move-Semantik) wurden verifiziert und sind
korrekt. Ein HIGH + zwei MED wurden gefunden und behoben.

## HIGH-1 — Cutout-Boolean ungetestet + `addSlabCutout` ohne Klemmung/Validierung

**Befund:** Kein Test führte je den **OCC-Cutout-Boolean** aus — der
Integrationstest rief `addSlabCutout` und prüfte nur Rückgabewert +
`SlabChanged`-Meldung, pullte danach aber nie `slabMeshes()` (der Boolean
läuft erst beim nächsten Pull). Der Kern-Test nutzt nur das analytische
Double (kann OCC-Koplanarität nicht reproduzieren). Zugleich akzeptierte
`addSlabCutout` jedes Polygon ungeprüft (`push_back` + Kommentar „auf den
Umriss begrenzt der Boolean") — ohne Validierung und ohne die von
spez. §1 LH-FA-SLB-001.a geforderte Begrenzung auf den Platten-Umriss.
Ein rand-/außenliegender Ausschnitt erzeugt die 013b-Koplanaritätsfalle.

**Fix (Nachschärfungs-Commit):**
- Neuer purer Helfer `cutoutInsideSlab(slab, cutout)` in `slab_geometry`
  (Shoelace-Fläche + Ray-Cast point-in-polygon): akzeptiert nur
  nicht-degenerierte, endliche, **vollständig innenliegende** Ausschnitte.
  Innenliegende Löcher sind koplanar-frei → **kein** lateraler Überstand
  nötig (anders als die Wandöffnung, die die Wand zwangsläufig durchspannt;
  der z-Überstand bleibt für Ober-/Unterseite). Signatur `(Slab, Footprint)`
  statt `(Footprint, Footprint)` — verschiedene Typen, vermeidet
  `bugprone-easily-swappable-parameters` (wie `openingCutPrism(Opening,Wall)`).
- `addSlabCutout` lehnt jetzt über `cutoutInsideSlab` ab (degeneriert/
  nicht-endlich/rand-/außenliegend → `false`, keine Meldung).
- **Test-Lücke geschlossen:** der Integrationstest pullt nach dem Cutout
  `slabMeshes()` erneut (echter OCC-Boolean) und prüft: mehr Dreiecke
  (Aussparung getragen) + unveränderter Umriss (innenliegend) + ein
  außenliegender Ausschnitt wird abgelehnt und ändert das Netz nicht.
  Neuer Kern-AK `CutoutNurInnenliegendAkzeptiert` (innen/außen/Kante/
  degeneriert/zu-wenig-Punkte/NaN).
- Spec §1 LH-FA-SLB-001.a + §8-Historie: Begrenzung als Ablehnung
  präzisiert (Containment-Vorbedingung).

## MEDIUM (mit HIGH-1-Fix erledigt)

- **MED-1** — `addSlabCutout` ohne NaN/degeneriert-Guard: durch
  `cutoutInsideSlab` (isfinite + Fläche > tol²) abgedeckt.
- **MED-2** — selbstschneidende Footprints: `cutoutInsideSlab` setzt einen
  einfachen Footprint als Vorbedingung voraus (dokumentiert im Header);
  die Sicht-Query bleibt total (catch → Platte überspringen).

## LOW / INFO (keine Aktion)

- `slabMeshes()` baut je Pull neu (kein Cache) — bewusst, im Scope ok.
- `slabBaseZ` Decke nutzt `storey.height_mm` (Ein-Geschoss-Annahme,
  spec-konform; Mehr-Geschoss = späterer Ausbau).
- Hexagon-Disziplin bestätigt: `slab_geometry` OCC-frei,
  `tessellateFootprint` bleibt im OCC-Adapter.

## Ergebnis

`make gates` grün nach der Nachschärfung: docs-check 0, arch-check A–E,
lint 0 + suppression-gate, **103 Tests**, Coverage **91,9 %**. HIGH-1 +
beide MED erledigt. **GO.**

**Lerneintrag:** Drittes Mal in Folge (013b, 014b, 015b) findet das
geometrie-fokussierte Code-Review einen echten Befund **hinter grünen
Gates** — diesmal eine ungetestete OCC-Boolean-Naht + fehlende
Spec-Begrenzung. Bestätigt die Regel „geometrielastige Slices vor
Welle-Closure code-reviewen" (Modul-11-Wert; computational Gates und
inferentiales Review sind komplementär).
