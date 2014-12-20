#pragma once

#include <UI\glwidget\glwidget.h>
#include <renderer\renderer.h>

namespace UI {

class DirLight : public GLWidget {
private:
    R::meshPtr_t    _sphereMesh;
    R::tfmeshPtr_t  _sphereTFMesh;
protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
public:
    QSize sizeHint() const override;
};

} // namespace UI