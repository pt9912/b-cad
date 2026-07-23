#pragma once

#include "hexagon/model/plan_view.h"
#include "hexagon/ports/driving/plan_view_port.h"

namespace bcad::adapters::ui::command {

// driving-Seite der ui (ADR-0019, Option A): kapselt den `PlanViewPort`-Pull der
// interaktiven 2D-Zeichenfläche. Der Composition-Root verdrahtet `planView()` als
// `std::function` in den `view/`-Canvas — **kein** `command/ → view/`-Include (der
// Driving-Port-Include lebt hier in `command/`, Richtungs-Trennung; der
// Regel-B-`adapter_sink` bleibt unverändert). Reine Modelltyp-Rückgabe, Analog zur
// `ViewModelMeshSource` (3D-Read-Naht).
class PlanViewPlanSource {
public:
    explicit PlanViewPlanSource(
        const hexagon::ports::driving::PlanViewPort& port)
        : port_(port) {}

    hexagon::model::PlanView planView() const { return port_.planView(); }

private:
    const hexagon::ports::driving::PlanViewPort& port_;
};

}  // namespace bcad::adapters::ui::command
