#include <QMouseEvent>
#include <QKeyEvent>

#include <Nubuck\common\common.h>
#include <UI\window_events.h>
#include <UI\outliner\outliner.h>
#include <world\world.h>
#include <operators\operators.h>
#include <renderer\effects\effectmgr.h>
#include "renderview.h"

namespace UI {

    void RenderView::initializeGL(void) {
        _renderer.Init();
    }

    void RenderView::resizeGL(int width, int height) {
        EV::Params_Resize args;
        args.width = width;
        args.height = height;
        W::world.Send(EV::def_Resize.Create(args));

        _renderer.Resize(width, height);
        // Update();
    }

    void RenderView::paintGL(void) {
        if(1.0f <= (_time += _rtimer.Stop())) {
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
        if(!OP::g_operators.MouseEvent(event)) W::world.Send(event);
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
        if(!OP::g_operators.MouseEvent(event)) W::world.Send(event);
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
        if(!OP::g_operators.MouseEvent(event)) W::world.Send(event);
    }

    void RenderView::wheelEvent(QWheelEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_WHEEL;
        args.mods = qevent->modifiers();
        args.delta = qevent->delta();
        args.x = qevent->x();
        args.y = qevent->y();
        W::world.Send(EV::def_Mouse.Create(args));
    }

    void RenderView::keyPressEvent(QKeyEvent* qevent) {
        EV::Params_Key args;
        args.type = EV::Params_Key::KEY_DOWN;
        args.keyCode = qevent->key();
        args.autoRepeat = qevent->isAutoRepeat();
        W::world.Send(EV::def_Key.Create(args));
    }

    void RenderView::keyReleaseEvent(QKeyEvent* qevent) {
        EV::Params_Key args;
        args.type = EV::Params_Key::KEY_UP;
        args.keyCode = qevent->key();
        if(!qevent->isAutoRepeat()) W::world.Send(EV::def_Key.Create(args));
    }

    RenderView::RenderView(QWidget* parent) : glWidget_t(parent), _fpsLabel(NULL) {        
        setMouseTracking(true);

        connect(&_timer, SIGNAL(timeout()), this, SLOT(Update()));
        _timer.start();
    }

    RenderView::~RenderView(void) {
		R::effectMgr.FreeResources();
    }

    void RenderView::Update(void) {
        updateGL();
    }

    QLabel* RenderView::FpsLabel(void) {
        if(!_fpsLabel) _fpsLabel = new QLabel("fps = ?");
        return _fpsLabel;
    }

} // namespace UI
