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
// Dach bewusst NICHT enthalten (welle-3): das Dachmodell ist dicke-los (Shell
// aus geneigten Flächen) → kein wohldefiniertes Bauteil-/Material-Solid; ein
// umbauter Dachkörper-Raum gehört nicht in dieselbe Material-Volumen-Summe.
// Re-Eval bei Dach-Dicke-/Material-Semantik (spez. §1 LH-FA-EVL-001.a).
struct VolumeReport {
    // Σ Netto-Material-Volumen in m³ (Wand + Decke/Fundament + Treppe).
    // Leeres/bauteilloses Modell -> 0.
    double total_m3{0.0};

    double walls_m3{0.0};   // Σ Wand-Netto-Volumen (Footprint·Höhe − Öffnungen)
    double slabs_m3{0.0};   // Σ Platten-Volumen (Decke/Fundament/Bodenplatte)
    double stairs_m3{0.0};  // Σ Treppen-Stufenkörper (geländer-frei)
};

}  // namespace bcad::hexagon::model
