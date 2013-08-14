#include <QMouseEvent>

#include <common\common.h>
#include <world\world.h>
#include <renderer\effects\effectmgr.h>
#include "renderview.h"

namespace UI {

    void RenderView::initializeGL(void) {
        _renderer.Init();
    }

    void RenderView::resizeGL(int width, int height) {
        _renderer.Resize(width, height);
        _arcballCamera.SetScreenSize(width, height);
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

        W::world.CopyRenderList(_renderList);
        _renderer.SetRenderList(_renderList);
        _renderer.Render();
    }

    void RenderView::mousePressEvent(QMouseEvent* event) {
        if(Qt::LeftButton == event->button())
            _arcballCamera.StartDragging(event->x(), event->y());
        if(Qt::RightButton == event->button())
            _arcballCamera.StartPanning(event->x(), event->y());
    }

    void RenderView::mouseReleaseEvent(QMouseEvent* event) {
        if(Qt::LeftButton == event->button())
            _arcballCamera.StopDragging();
        if(Qt::RightButton == event->button())
            _arcballCamera.StopPanning();
    }

    void RenderView::mouseMoveEvent(QMouseEvent* event) {
        if(_arcballCamera.Drag(event->x(), event->y())) Update();
        if(_arcballCamera.Pan(event->x(), event->y())) Update();
    }

    void RenderView::wheelEvent(QWheelEvent* event) {
        if(event->delta() > 0) _arcballCamera.ZoomIn();
        if(event->delta() < 0) _arcballCamera.ZoomOut();
        Update();
    }

    RenderView::RenderView(QWidget* parent) : glWidget_t(parent), _fpsLabel(NULL), _arcballCamera(DEFAULT_WIDTH, DEFAULT_HEIGHT) {        
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
