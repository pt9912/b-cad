#pragma once

#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPoint>

#include "adapters/ui/view/viewer_scene.h"
#include "hexagon/ports/driven/model_changed_port.h"

namespace bcad::adapters::ui::view {

// driven-Seite der ui (ADR-0009, slice-029): sichtbares 3D-Fenster —
// Beobachter-Implementierung + Renderer. Rendert den `ViewerScene`-Stand
// (tessellierte Netze, gepullt über die ui-interne MeshSource-Naht) per
// QOpenGLWidget — orbitierbare Perspektive, Flat-/Lambert-Shading,
// bewusst minimal (Erweiterungen nur über ADR-0009
// §Re-Evaluierungs-Trigger).
//
// Implementiert den `ModelChangedPort` (ADR-0008/0009 (d)): der
// Callback delegiert an die Szene (Pull-State) und PLANT ein Repaint
// (`update()` ist queued) — keine Mutation, kein synchrones Rendern im
// Mutationspfad.
class ViewerWidget final : public QOpenGLWidget,
                           protected QOpenGLFunctions,
                           public hexagon::ports::driven::ModelChangedPort {
public:
    explicit ViewerWidget(const MeshSource& mesh_source,
                          QWidget* parent = nullptr);

    // ADR-0008-Callback: Szene nachziehen + Repaint einplanen.
    void onModelChanged(
        const hexagon::ports::driven::ModelChange& change) override;

    const ViewerScene& scene() const { return scene_; }

protected:
    void initializeGL() override;
    void paintGL() override;

    // Orbit-Kamera (ADR-0009 (a)/(b): minimal): Ziehen = Yaw/Pitch,
    // Rad = Zoom.
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QMatrix4x4 viewProjection() const;
    void computeSceneBounds(double bounds[6]) const;

    ViewerScene scene_;
    QOpenGLShaderProgram program_;
    QPoint last_mouse_pos_{};
    double yaw_deg_{-35.0};
    double pitch_deg_{25.0};
    double zoom_{1.0};
};

}  // namespace bcad::adapters::ui::view
