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
    public:
        enum InputMode {
            CAMERA, PEN
        };
    private:
        QLabel* _fpsLabel;

        int         _numFrames;
        float       _time;
        SYS::Timer  _rtimer;

        R::Color _bgColor;

        struct Gradient {
            R::meshPtr_t    mesh;
            R::tfmeshPtr_t  tfmesh;

            bool            show;

            Gradient() : mesh(NULL), tfmesh(NULL), show(true) { }
        } _bgGradient; // background gradient

        void BuildBackgroundGradient();
        void RenderBackgroundGradient(R::RenderList& renderList);

        R::RenderList               _renderList;
        std::vector<R::PenVertex>   _pen;
        R::DebugText                _debugText;

        bool _screenshotRequested;
        bool _largeScreenshotRequested;

        InputMode   _inputMode;
        bool        _isPenDown;

        void EmitPenVertex(const QPointF& p);
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
        void OnSetBackgroundColor(float r, float g, float b);
        void OnSetBackgroundColor(const R::Color& color);
        void OnSetBackgroundColor(const QColor& color);
        void OnShowBackgroundGradient(bool show);
        void OnShowBackgroundGradient(int show); // hide if show = 0
    public:
        static R::Color defaultBackgroundColor;

        typedef GLWidget glWidget_t;

        enum {
            DEFAULT_WIDTH  = 800,
            DEFAULT_HEIGHT = 400
        };

        RenderView(QWidget* parent = NULL);
        ~RenderView(void);

        const R::Renderer& GetRenderer() const;

        const R::Color& GetBackgroundColor() const;
        bool            IsBackgroundGradient() const;

        void Render();

        QLabel* FpsLabel(void);
    };

} // namespace UI
