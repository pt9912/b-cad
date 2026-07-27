#include "adapters/ui/command/project_menu_handler.h"

namespace bcad::adapters::ui::command {

hexagon::ports::driving::DrawingTargetResolution ProjectMenuHandler::open(
    const std::filesystem::path& path) {
    // Die gehaltenen Senken werden DURCHGEREICHT — nicht `{}`. Ohne sie bliebe
    // das Zeichen-Ziel auf den Ids des alten Modells stehen (slice-047-B4);
    // Orakel slice-053 §3-Zeile 6 prueft genau das.
    return project_.openProject(path, sinks_);
}

void ProjectMenuHandler::saveAs(const std::filesystem::path& path) {
    project_.saveProject(path);
}

}  // namespace bcad::adapters::ui::command
