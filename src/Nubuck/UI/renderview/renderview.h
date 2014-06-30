#pragma once

#include <renderer\glew\glew.h>
#include <QLabel>
#include <QGLWidget>

#include <system\timer\timer.h>
#include <world\world.h>
#include <renderer\renderer.h>
#include <renderer\debugtext\debugtext.h>
#include <camera\arcball_camera.h>
#include <UI\glwidget\glwidget.h>

namespace UI {

    class RenderView : public GLWidget {
        Q_OBJECT
    private:
        QLabel* _fpsLabel;

        int         _numFrames;
        float       _time;
        SYS::Timer  _rtimer;

        R::Renderer     _renderer;
        R::RenderList   _renderList;
        R::DebugText    _debugText;
    protected:
        void initializeGL(void) override;
        void resizeGL(int width, int height) override;
        void finishResizeGL() override;

        bool focusNextPrevChild(bool next) override;

        void enterEvent(QEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;
    public slots:
        void OnSetBackgroundColor(const R::Color& color);
        void OnSetBackgroundColor(const QColor& color);
    public:
        typedef GLWidget glWidget_t;

        enum {
            DEFAULT_WIDTH  = 800,
            DEFAULT_HEIGHT = 400
        };

        RenderView(QWidget* parent = NULL);
        ~RenderView(void);

        const R::Renderer& GetRenderer() const;

        void Render();

        QLabel* FpsLabel(void);
    };

} // namespace UI
