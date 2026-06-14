#pragma once

#include <vector>

namespace bcad::hexagon::model {

// Ergebnis einer Flächen-Auswertung (LH-FA-EVL-001/003, ADR-0012):
// die Per-Raum-Netto-Grundflächen und deren Summe, in Quadratmetern.
// Reiner Werttyp — framework-frei (kein OCC/Qt/SQLite), read-only abgeleitet
// aus den erkannten Räumen (Room.net_area_mm2 in mm², ADR-0007), nie
// persistiert. Die Auswertung mutiert nichts und meldet nichts (kein op).
struct AreaReport {
    // Summe der enthaltenen Flächen in m² (EVL-001 „Summe je Geschoss",
    // EVL-003 Wohnfläche). Geschoss/Modell ohne Räume -> 0.
    double total_m2{0.0};

    // Netto-Grundfläche je Raum in m² (EVL-001 „je Raum"), in Erkennungs-
    // Reihenfolge. Leer, wenn kein Raum vorliegt.
    std::vector<double> room_areas_m2{};

    // Anzahl der berücksichtigten Räume (= room_areas_m2.size()).
    int room_count() const { return static_cast<int>(room_areas_m2.size()); }
};

}  // namespace bcad::hexagon::model
