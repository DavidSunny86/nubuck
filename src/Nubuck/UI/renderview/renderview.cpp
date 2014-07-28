#include <QMouseEvent>
#include <QKeyEvent>

#include <Nubuck\common\common.h>
#include <system\opengl\opengl.h>
#include <UI\window_events.h>
#include <UI\outliner\outliner.h>
#include <world\world.h>
#include <operators\operators.h>
#include <renderer\metrics\metrics.h>
#include <renderer\effects\effectmgr.h>
#include "renderview.h"

#include <operators\operators.h> // !!!

namespace UI {

    void RenderView::initializeGL(void) {
        _renderer.Init();
        _debugText.Init(GetRenderingContext().GetDeviceContext());
    }

    void RenderView::resizeGL(int width, int height) {
        EV::Params_Resize args;
        args.width = width;
        args.height = height;
        W::world.Send(EV::def_Resize.Create(args));

        _renderer.Resize(width, height);
        _debugText.Resize(width, height);
        Render();
    }

    void RenderView::finishResizeGL() {
        _renderer.FinishResize();
    }

    bool RenderView::focusNextPrevChild(bool) {
        // do not switch to next widget in tab order.
        // generate KeyEvent with key = Key_Tab instead.
        return false;
    }

    void RenderView::enterEvent(QEvent* event) {
        setFocus(Qt::OtherFocusReason);
    }

    void RenderView::mousePressEvent(QMouseEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_DOWN;
        args.button = qevent->button();
        args.mods = qevent->modifiers();
        args.x = qevent->x();
        args.y = qevent->y();
		args.ret = NULL;

        EV::Event event = EV::def_Mouse.Create(args);
        OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::ALWAYS);
    }

    void RenderView::mouseReleaseEvent(QMouseEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_UP;
        args.button = qevent->button();
        args.mods = qevent->modifiers();
        args.x = qevent->x();
        args.y = qevent->y();
		args.ret = NULL;

        EV::Event event = EV::def_Mouse.Create(args);
        OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::ALWAYS);
    }

    void RenderView::mouseMoveEvent(QMouseEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_MOVE;
        args.button = qevent->button();
        args.mods = qevent->modifiers();
        args.x = qevent->x();
        args.y = qevent->y();
		args.ret = NULL;

        EV::Event event = EV::def_Mouse.Create(args);
        OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::ALWAYS);
    }

    void RenderView::wheelEvent(QWheelEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_WHEEL;
        args.mods = qevent->modifiers();
        args.delta = qevent->delta();
        args.x = qevent->x();
        args.y = qevent->y();

        EV::Event event = EV::def_Mouse.Create(args);
        OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::ALWAYS);
    }

    void RenderView::keyPressEvent(QKeyEvent* qevent) {
        // screenshot
        if(Qt::Key_F12 == qevent->key() && !qevent->isAutoRepeat()) {
            _renderer.Screenshot();
            return;
        }

        if(Qt::Key_Tab == qevent->key() && !qevent->isAutoRepeat()) {
            W::world.GetEditMode().CycleModes();
            return;
        }

        EV::Params_Key args;
        args.type = EV::Params_Key::KEY_DOWN;
        args.keyCode = qevent->key();
        args.nativeScanCode = qevent->nativeScanCode();
        args.autoRepeat = qevent->isAutoRepeat();
        args.mods = qevent->modifiers();


        EV::Event event = EV::def_Key.Create(args);
        OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::ALWAYS);
    }

    void RenderView::keyReleaseEvent(QKeyEvent* qevent) {
        EV::Params_Key args;
        args.type = EV::Params_Key::KEY_UP;
        args.keyCode = qevent->key();
        if(!qevent->isAutoRepeat()) W::world.Send(EV::def_Key.Create(args));
    }

    void RenderView::OnSetBackgroundColor(const R::Color& color) {
        _renderer.SetBackgroundColor(color);
    }

    void RenderView::OnSetBackgroundColor(const QColor& color) {
        _renderer.SetBackgroundColor(R::Color(color.redF(), color.greenF(), color.blueF()));
    }

    RenderView::RenderView(QWidget* parent)
        : glWidget_t(parent)
        , _fpsLabel(NULL)
        , _time(0.0f)
    {
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);
    }

    RenderView::~RenderView(void) {
		R::effectMgr.FreeResources();
    }

    const R::Renderer& RenderView::GetRenderer() const {
        return _renderer;
    }

    void RenderView::Render() {
        _time += M::Max(0.0f, _rtimer.Stop());
        if(1.0f <= _time) {
            float fps = _numFrames / _time;
            _numFrames = 0;
            _time = 0.0f;

            if(_fpsLabel) _fpsLabel->setText(QString("fps = %1").arg(fps));
        }
        _numFrames++;
        _rtimer.Start();

        _renderList.Clear();
        W::world.Render(_renderList);
        OP::g_operators.GetMeshJobs(_renderList.meshJobs);

        _renderer.BeginFrame();
        _renderer.Render(_renderList);
        _renderer.EndFrame();

        _debugText.BeginFrame();
        _debugText.Printf("frame time: %f\n", R::metrics.frame.time);
        _debugText.Printf("number of draw calls: %d\n", R::metrics.frame.numDrawCalls);
        _debugText.Printf("operator driver queue size: %d\n", OP::g_operators.GetDriverQueueSize());
        _debugText.EndFrame();

        updateGL();
    }

    QLabel* RenderView::FpsLabel(void) {
        if(!_fpsLabel) _fpsLabel = new QLabel("fps = ?");
        return _fpsLabel;
    }

} // namespace UI
