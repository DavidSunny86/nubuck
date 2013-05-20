#pragma once

#include <QWidget>
#include <generic\pointer.h>
#include <generic\uncopyable.h>

namespace SYS {

    class RenderingContext;

} // namespace SYS

namespace UI {

    class GLWidget : public QWidget, private GEN::Uncopyable {
    private:
        GEN::Pointer<SYS::RenderingContext> _rc;

        HWND _winId;
        bool _isInitialized;

        void Initialize(void);
    protected:
        virtual void initializeGL(void) { }
        virtual void resizeGL(int width, int height) { }
        virtual void paintGL(void) { }

        virtual void mousePressEvent(QMouseEvent* event) override { }
        virtual void mouseReleaseEvent(QMouseEvent* event) override { }
        virtual void mouseMoveEvent(QMouseEvent* event) override { }
        virtual void wheelEvent(QWheelEvent* event) override { }

        void resizeEvent(QResizeEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

        QPaintEngine* paintEngine(void) const override { return NULL; }
    public:
        GLWidget(QWidget* parent = NULL);

        void updateGL(void);
    };

} // namespace UI