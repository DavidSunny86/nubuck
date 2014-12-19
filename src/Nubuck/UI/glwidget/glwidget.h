#pragma once

#include <QWidget>
#include <Nubuck\generic\pointer.h>
#include <Nubuck\generic\uncopyable.h>
#include <system\timer\timer.h>

namespace SYS {

    class RenderingContext;

} // namespace SYS

namespace UI {

    class GLWidget : public QWidget, private GEN::Uncopyable {
    private:
        GEN::Pointer<SYS::RenderingContext> _rc;

        HWND        _winId;
        bool 		_isInitialized;

        bool        _resizing;
        SYS::Timer  _resizeTimer;
    protected:
        SYS::RenderingContext& GetRenderingContext();

        virtual void initializeGL(void) { }
        virtual void resizeGL(int width, int height) { }
        virtual void paintGL(void) { }
        virtual void finishResizeGL() { }

        virtual void mousePressEvent(QMouseEvent* event) override { }
        virtual void mouseReleaseEvent(QMouseEvent* event) override { }
        virtual void mouseMoveEvent(QMouseEvent* event) override { }
        virtual void wheelEvent(QWheelEvent* event) override { }

        void resizeEvent(QResizeEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

        QPaintEngine* paintEngine(void) const override { return NULL; }
    public:
        GLWidget(QWidget* parent = NULL);

        void Use(void);
        void updateGL(void);
    };

} // namespace UI