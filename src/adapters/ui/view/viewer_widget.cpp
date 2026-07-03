#include "adapters/ui/viewer_widget.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <QMouseEvent>
#include <QVector3D>
#include <QWheelEvent>

namespace bcad::adapters::ui {

namespace {

// GLSL 120 (Kompatibilitätsprofil; läuft auch auf Mesa/llvmpipe im
// headless Container — ADR-0009 (f)).
constexpr const char* kVertexShader = R"(
    #version 120
    attribute vec3 position;
    attribute vec3 normal;
    uniform mat4 mvp;
    varying vec3 v_normal;
    void main() {
        gl_Position = mvp * vec4(position, 1.0);
        v_normal = normal;
    }
)";

constexpr const char* kFragmentShader = R"(
    #version 120
    varying vec3 v_normal;
    uniform vec3 light_dir;
    uniform vec3 base_color;
    void main() {
        float diffuse = max(dot(normalize(v_normal), normalize(light_dir)), 0.0);
        gl_FragColor = vec4(base_color * (0.3 + 0.7 * diffuse), 1.0);
    }
)";

}  // namespace

ViewerWidget::ViewerWidget(
    const hexagon::ports::driving::ViewModelPort& view_model, QWidget* parent)
    : QOpenGLWidget(parent), scene_(view_model) {
    scene_.loadAll();  // statische Darstellung (LH-FA-D3-001)
    setWindowTitle(QStringLiteral("b-cad — 3D-Viewer (welle-1v)"));
}

void ViewerWidget::onModelChanged(
    const hexagon::ports::driven::ModelChange& change) {
    scene_.onModelChanged(change);  // Pull-State, keine Mutation
    update();                       // Repaint einplanen (queued, ADR-0009 (d))
}

void ViewerWidget::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.12F, 0.13F, 0.15F, 1.0F);
    program_.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShader);
    program_.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShader);
    program_.link();
}

void ViewerWidget::computeSceneBounds(double bounds[6]) const {
    bounds[0] = bounds[1] = bounds[2] = 0.0;
    bounds[3] = bounds[4] = bounds[5] = 0.0;
    bool first = true;
    for (const auto& [id, mesh] : scene_.wallMeshes()) {
        for (std::size_t i = 0; i + 2 < mesh.positions.size(); i += 3) {
            const double x = mesh.positions[i];
            const double y = mesh.positions[i + 1];
            const double z = mesh.positions[i + 2];
            if (first) {
                bounds[0] = bounds[3] = x;
                bounds[1] = bounds[4] = y;
                bounds[2] = bounds[5] = z;
                first = false;
            } else {
                bounds[0] = std::min(bounds[0], x);
                bounds[1] = std::min(bounds[1], y);
                bounds[2] = std::min(bounds[2], z);
                bounds[3] = std::max(bounds[3], x);
                bounds[4] = std::max(bounds[4], y);
                bounds[5] = std::max(bounds[5], z);
            }
        }
    }
}

QMatrix4x4 ViewerWidget::viewProjection() const {
    double bounds[6];
    computeSceneBounds(bounds);
    const QVector3D center(
        static_cast<float>((bounds[0] + bounds[3]) * 0.5),
        static_cast<float>((bounds[1] + bounds[4]) * 0.5),
        static_cast<float>((bounds[2] + bounds[5]) * 0.5));
    const double dx = bounds[3] - bounds[0];
    const double dy = bounds[4] - bounds[1];
    const double dz = bounds[5] - bounds[2];
    const double radius =
        std::max(1.0, 0.5 * std::sqrt((dx * dx) + (dy * dy) + (dz * dz)));
    const double distance = (radius * 2.6) / zoom_;

    const double yaw = yaw_deg_ * M_PI / 180.0;
    const double pitch = pitch_deg_ * M_PI / 180.0;
    const QVector3D eye =
        center + QVector3D(static_cast<float>(distance * std::cos(pitch) * std::cos(yaw)),
                           static_cast<float>(distance * std::cos(pitch) * std::sin(yaw)),
                           static_cast<float>(distance * std::sin(pitch)));

    QMatrix4x4 projection;
    const float aspect =
        static_cast<float>(width()) / static_cast<float>(std::max(1, height()));
    projection.perspective(45.0F, aspect, static_cast<float>(radius) * 0.01F,
                           static_cast<float>(distance + (radius * 4.0)));
    QMatrix4x4 view;
    view.lookAt(eye, center, QVector3D(0.0F, 0.0F, 1.0F));
    return projection * view;
}

void ViewerWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (scene_.wallMeshes().empty() || !program_.isLinked()) {
        return;
    }

    program_.bind();
    program_.setUniformValue("mvp", viewProjection());
    program_.setUniformValue("light_dir", QVector3D(0.4F, 0.3F, 1.0F));
    program_.setUniformValue("base_color", QVector3D(0.78F, 0.74F, 0.68F));

    // Kleine Modelle (welle-1v): Vertices je Frame als Client-Arrays
    // hochreichen — kein VBO-Lebenszyklus, Szene bleibt die einzige
    // Wahrheit (Granularität wird erst mit Latenz-Budget Thema).
    std::vector<float> positions;
    std::vector<float> normals;
    for (const auto& [id, mesh] : scene_.wallMeshes()) {
        for (const int index : mesh.indices) {
            const std::size_t base = static_cast<std::size_t>(index) * 3;
            for (int c = 0; c < 3; ++c) {
                positions.push_back(static_cast<float>(mesh.positions[base + c]));
                normals.push_back(static_cast<float>(mesh.normals[base + c]));
            }
        }
    }

    const int pos_location = program_.attributeLocation("position");
    const int normal_location = program_.attributeLocation("normal");
    program_.enableAttributeArray(pos_location);
    program_.enableAttributeArray(normal_location);
    program_.setAttributeArray(pos_location, positions.data(), 3);
    program_.setAttributeArray(normal_location, normals.data(), 3);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(positions.size() / 3));

    program_.disableAttributeArray(pos_location);
    program_.disableAttributeArray(normal_location);
    program_.release();
}

void ViewerWidget::mousePressEvent(QMouseEvent* event) {
    last_mouse_pos_ = event->pos();
}

void ViewerWidget::mouseMoveEvent(QMouseEvent* event) {
    const QPoint delta = event->pos() - last_mouse_pos_;
    last_mouse_pos_ = event->pos();
    if ((event->buttons() & Qt::LeftButton) != 0) {
        yaw_deg_ -= delta.x() * 0.5;
        pitch_deg_ = std::clamp(pitch_deg_ + (delta.y() * 0.5), -89.0, 89.0);
        update();
    }
}

void ViewerWidget::wheelEvent(QWheelEvent* event) {
    const double steps = event->angleDelta().y() / 120.0;
    zoom_ = std::clamp(zoom_ * std::pow(1.15, steps), 0.05, 50.0);
    update();
}

}  // namespace bcad::adapters::ui
