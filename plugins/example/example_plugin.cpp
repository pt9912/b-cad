// Beispiel-Plugin (ADR-0017; AK-Fixture für LH-FA-PLG-001/003/004):
// führt in onLoad eine echte Edit-Mutation über den vermittelten
// Driving-Port aus — inkl. eines bewusst außerhalb des Wertebereichs
// liegenden Parameters (Klemm-Beleg: dieselben Prüf-/Klemm-Regeln wie
// manuelle Eingaben, kein Nebeneingang). Hält nach onUnload KEINE
// Port-/Kontext-Referenz (Vertragspflicht).
#include <cstdint>
#include <string>

#include "hexagon/model/segment.h"
#include "plugin_api/plugin.h"
#include "plugin_api/plugin_abi.h"
#include "plugin_api/plugin_context.h"

namespace {

namespace model = bcad::hexagon::model;

class ExamplePlugin : public bcad::plugin_api::Plugin {
public:
    std::string name() const override { return "bcad-example"; }

    void onLoad(bcad::plugin_api::PluginContext& context) override {
        auto& edit = context.edit();
        const auto storey = edit.addStorey(2500.0);
        const auto wall = edit.addWall(
            storey, model::Segment{model::Point2D{0.0, 0.0},
                                   model::Point2D{5000.0, 0.0}});
        if (wall.has_value()) {
            // 20 mm liegt unter der Untergrenze (50 mm) -> wird wie jede
            // manuelle Eingabe geklemmt (E-VAL-001; Sandbox-AK-Beleg).
            edit.setWallThickness(*wall, 20.0);
        }
        context_ = &context;
    }

    void onUnload() override { context_ = nullptr; }

private:
    bcad::plugin_api::PluginContext* context_{nullptr};
};

}  // namespace

extern "C" std::uint32_t bcad_plugin_abi_version() {
    return bcad::plugin_api::kAbiVersion;
}

extern "C" bcad::plugin_api::Plugin* bcad_plugin_create() {
    return new ExamplePlugin;
}

extern "C" void bcad_plugin_destroy(bcad::plugin_api::Plugin* plugin) {
    delete plugin;
}
