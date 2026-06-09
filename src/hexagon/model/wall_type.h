#pragma once

namespace bcad::hexagon::model {

// Wandtyp (LH-FA-WAL-007). Steuert später Default-Material und
// Auswertungs-Kategorie; in 003a nur als Klassifikation geführt.
enum class WallType { Innen, Aussen, Trag };

}  // namespace bcad::hexagon::model
