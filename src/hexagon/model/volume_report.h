#pragma once

namespace bcad::hexagon::model {

// Ergebnis einer Volumen-Auswertung (LH-FA-EVL-002, Evaluations-Architektur):
// das gebäudeweite Netto-MATERIAL-Volumen in Kubikmetern plus die Bauteiltyp-
// Subtotale. Reiner Werttyp — framework-frei (kein OCC/Qt/SQLite), read-only
// ANALYTISCH IM KERN abgeleitet (Footprint-Fläche · Höhe − geklemmtes
// Öffnungsvolumen je Wand; (Fläche − Ausschnitte) · Dicke je Platte;
// Stufenkörper je Treppe), nie persistiert. Die Auswertung mutiert nichts und
// meldet nichts (kein op); insbesondere wird NICHT das adapter-gemessene
// Solid.volume_mm3 gelesen (das wäre eine driven-Volumenmessung).
//
// Dach (slice-023b): seit `LH-FA-ROF-006` ist das Dach ein Volumenkörper der
// Dicke d → das Dach-Material-Volumen (projizierte Trauf-Grundfläche · Dicke,
// `bx·ty·d`) ist enthalten (`roofs_m3`); analytisch im Kern, kein
// `Solid.volume_mm3` (spez. §1 LH-FA-ROF-001.a / LH-FA-EVL-001.a).
struct VolumeReport {
    // Σ Netto-Material-Volumen in m³ (Wand + Decke/Fundament + Treppe + Dach).
    // Leeres/bauteilloses Modell -> 0.
    double total_m3{0.0};

    double walls_m3{0.0};   // Σ Wand-Netto-Volumen (Footprint·Höhe − Öffnungen)
    double slabs_m3{0.0};   // Σ Platten-Volumen (Decke/Fundament/Bodenplatte)
    double stairs_m3{0.0};  // Σ Treppen-Stufenkörper (geländer-frei)
    double roofs_m3{0.0};   // Σ Dach-Volumen (Trauf-Grundfläche · Dicke, ROF-006)
};

}  // namespace bcad::hexagon::model
