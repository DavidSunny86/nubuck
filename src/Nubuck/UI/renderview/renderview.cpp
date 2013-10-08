#include <QMouseEvent>
#include <QKeyEvent>

#include <common\common.h>
#include <world\world.h>
#include <world\events.h>
#include <renderer\effects\effectmgr.h>
#include "renderview.h"

namespace UI {

    void RenderView::initializeGL(void) {
        _renderer.Init();
    }

    void RenderView::resizeGL(int width, int height) {
        W::Event event;
        event.type = W::EVENT_RESIZE;
        W::EvArgs_Resize* args = (W::EvArgs_Resize*)event.args;
        args->width = width;
        args->height = height;
        W::world.Send(event);

        _renderer.Resize(width, height);
        Update();
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

        _renderer.Render();
    }

    void RenderView::enterEvent(QEvent* event) {
        setFocus(Qt::OtherFocusReason);
    }

    void RenderView::mousePressEvent(QMouseEvent* qevent) {
        W::Event wevent;
        wevent.type = W::EVENT_MOUSE;
        W::EvArgs_Mouse* args = (W::EvArgs_Mouse*)wevent.args;
        args->type = W::EvArgs_Mouse::MOUSE_DOWN;
        args->button = qevent->button();
        args->mods = qevent->modifiers();
        args->x = qevent->x();
        args->y = qevent->y();
        W::world.Send(wevent);
    }

    void RenderView::mouseReleaseEvent(QMouseEvent* qevent) {
        W::Event wevent;
        wevent.type = W::EVENT_MOUSE;
        W::EvArgs_Mouse* args = (W::EvArgs_Mouse*)wevent.args;
        args->type = W::EvArgs_Mouse::MOUSE_UP;
        args->button = qevent->button();
        args->mods = qevent->modifiers();
        args->x = qevent->x();
        args->y = qevent->y();
        W::world.Send(wevent);
    }

    void RenderView::mouseMoveEvent(QMouseEvent* qevent) {
        W::Event wevent;
        wevent.type = W::EVENT_MOUSE;
        W::EvArgs_Mouse* args = (W::EvArgs_Mouse*)wevent.args;
        args->type = W::EvArgs_Mouse::MOUSE_MOVE;
        args->button = qevent->button();
        args->mods = qevent->modifiers();
        args->x = qevent->x();
        args->y = qevent->y();
        W::world.Send(wevent);
    }

    void RenderView::wheelEvent(QWheelEvent* qevent) {
        W::Event wevent;
        wevent.type = W::EVENT_MOUSE;
        W::EvArgs_Mouse* args = (W::EvArgs_Mouse*)wevent.args;
        args->type = W::EvArgs_Mouse::MOUSE_WHEEL;
        args->mods = qevent->modifiers();
        args->delta = qevent->delta();
        args->x = qevent->x();
        args->y = qevent->y();
        W::world.Send(wevent);
    }

    void RenderView::keyPressEvent(QKeyEvent* qevent) {
        W::Event event;
        event.type = W::EVENT_KEY;
        W::EvArgs_Key* args =  (W::EvArgs_Key*)event.args;
        args->type = W::EvArgs_Key::KEY_DOWN;
        args->keyCode = qevent->key();
        args->autoRepeat = qevent->isAutoRepeat();
        W::world.Send(event);
    }

    void RenderView::keyReleaseEvent(QKeyEvent* qevent) {
        W::Event event;
        event.type = W::EVENT_KEY;
        W::EvArgs_Key* args =  (W::EvArgs_Key*)event.args;
        args->type = W::EvArgs_Key::KEY_UP;
        args->keyCode = qevent->key();
        W::world.Send(event);
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
#ifndef NUBUCK_MT
        W::world.Update();
#endif
    }

    QLabel* RenderView::FpsLabel(void) {
        if(!_fpsLabel) _fpsLabel = new QLabel("fps = ?");
        return _fpsLabel;
    }

} // namespace UI
