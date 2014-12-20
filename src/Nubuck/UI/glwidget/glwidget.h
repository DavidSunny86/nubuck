#pragma once

#include <QWidget>
#include <Nubuck\generic\pointer.h>
#include <Nubuck\generic\uncopyable.h>
#include <system\timer\timer.h>

namespace SYS {

class DeviceContext;
class RenderingContext;

} // namespace SYS

namespace UI {

class GLWidget : public QWidget, private GEN::Uncopyable {
private:
    // all gl widgets share a single rc, so they can share
    // resources.
    static GEN::Pointer<SYS::RenderingContext> shared_rc;

    HWND                                _winId;
    bool 		                        _isInitialized;
    GEN::Pointer<SYS::DeviceContext>    _dc;

    bool        _resizing;
    SYS::Timer  _resizeTimer;
protected:
    SYS::DeviceContext&     GetDeviceContext();
    SYS::RenderingContext&  GetRenderingContext();

    virtual void initializeGL() { }
    virtual void resizeGL(int width, int height) { }
    virtual void paintGL() { }
    virtual void finishResizeGL() { }

    virtual void mousePressEvent(QMouseEvent* event) override { }
    virtual void mouseReleaseEvent(QMouseEvent* event) override { }
    virtual void mouseMoveEvent(QMouseEvent* event) override { }
    virtual void wheelEvent(QWheelEvent* event) override { }

    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

    QPaintEngine* paintEngine() const override { return NULL; }
public:
    GLWidget(QWidget* parent = NULL);

    void Use();
    void updateGL();
};

} // namespace UI