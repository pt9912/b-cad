#pragma once

#include <map>

#include "hexagon/model/roof.h"  // RoofId
#include "hexagon/model/slab.h"  // SlabId
#include "hexagon/model/stair.h"  // StairId
#include "hexagon/model/triangle_mesh.h"
#include "hexagon/model/wall.h"  // WallId
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/ports/driving/view_model_port.h"

namespace bcad::adapters::ui {

// Szenen-Zustand des Viewers — das headless beobachtbare
// „dargestellt"-Surrogat (ADR-0009 (f)): die gehaltenen Netze je
// `element_id` plus Zähler wirksamer Szenen-Updates. Bewusst Qt-frei —
// die AK-Tests (LH-FA-D3-001/002) prüfen diesen Zustand ohne Display;
// das Qt-Widget rendert ihn nur noch.
//
// Implementiert den `ModelChangedPort` (ADR-0008): der Callback PULLT
// ausschließlich über den `ViewModelPort` und mutiert nie das Modell —
// das Re-Entranz-Verbot ist strukturell eingehalten (ADR-0009 (d)).
class ViewerScene final : public hexagon::ports::driven::ModelChangedPort {
public:
    explicit ViewerScene(const hexagon::ports::driving::ViewModelPort& view_model);

    // Initialer Stand: Szene aus allen Wand-Netzen aufbauen (statische
    // Darstellung, LH-FA-D3-001 / Split-Hälfte i).
    void loadAll();

    // ADR-0008-Callback (Push-Notify): pullt bei Wand-Ops das Netz der
    // gemeldeten Wand (Pull-State) und ersetzt es idempotent. Geschoss-
    // und Raum-Meldungen ändern die 3D-Szene nicht (Räume sind
    // 2D-Stoff) — genau EIN wirksames Szenen-Update je Wand-Meldung,
    // kein Flackern bei Mehrfach-Meldung (LH-FA-D3-002 Boundary).
    void onModelChanged(
        const hexagon::ports::driven::ModelChange& change) override;

    const std::map<hexagon::model::WallId, hexagon::model::TriangleMesh>&
    wallMeshes() const {
        return meshes_;
    }

    // Gehaltene Dach-Netze (LH-FA-ROF-*); auf `RoofChanged` neu geladen.
    const std::map<hexagon::model::RoofId, hexagon::model::TriangleMesh>&
    roofMeshes() const {
        return roof_meshes_;
    }

    // Gehaltene Platten-Netze (LH-FA-SLB-*/FND-*); auf `SlabChanged` neu
    // geladen.
    const std::map<hexagon::model::SlabId, hexagon::model::TriangleMesh>&
    slabMeshes() const {
        return slab_meshes_;
    }

    // Gehaltene Treppen-Netze (LH-FA-STR-*); auf `StairChanged` neu geladen.
    const std::map<hexagon::model::StairId, hexagon::model::TriangleMesh>&
    stairMeshes() const {
        return stair_meshes_;
    }

    // Anzahl wirksamer Szenen-Updates (Netz ersetzt/hinzugefügt/entfernt) —
    // Surrogat-Zähler für die Idempotenz-/Negative-AK.
    int effectiveUpdates() const { return effective_updates_; }

private:
    const hexagon::ports::driving::ViewModelPort& view_model_;
    std::map<hexagon::model::WallId, hexagon::model::TriangleMesh> meshes_{};
    std::map<hexagon::model::RoofId, hexagon::model::TriangleMesh> roof_meshes_{};
    std::map<hexagon::model::SlabId, hexagon::model::TriangleMesh> slab_meshes_{};
    std::map<hexagon::model::StairId, hexagon::model::TriangleMesh> stair_meshes_{};
    int effective_updates_{0};
};

}  // namespace bcad::adapters::ui
