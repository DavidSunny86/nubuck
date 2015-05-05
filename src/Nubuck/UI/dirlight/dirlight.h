#pragma once

#include <UI\glwidget\glwidget.h>
#include <renderer\renderer.h>
#include <camera\arcball_camera.h>

namespace UI {

class ColorButton;

class DirLight : public GLWidget {
    Q_OBJECT
private:
    R::meshPtr_t    _sphereMesh;
    R::tfmeshPtr_t  _sphereTFMesh;

    ArcballCamera   _camera;
    bool            _isDragging;

    M::Vector3      _lightDir;
    R::Color        _lightCol;

    std::string     _renderContext;

    void SetupLights(R::RenderList& renderList);
protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
public slots:
    void OnColorChanged(float r, float g, float b);
signals:
    void SigDirectionChanged(float x, float y, float z);
public:
    DirLight();

    const M::Vector3&   GetDirection() const { return _lightDir; }
    const R::Color&     GetColor() const { return _lightCol; }

    void SetDirection(const M::Vector3& dir);

    void SetRenderContext(const std::string& context);

    QSize sizeHint() const override;
};

// preview + buttons
class DirLightControls : public QWidget {
    Q_OBJECT
private:
    int             _lightIdx;
    DirLight*       _dirLight;
    ColorButton*    _colorButton;
private slots:
    void OnLightChanged();
public:
    explicit DirLightControls(const int lightIdx);
};

} // namespace UI