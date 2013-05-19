#include <QMouseEvent>

#include <common\common.h>
#include <world\world.h>
#include <renderer\effects\effectmgr.h>
#include <world\renderworld.h>
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
        RenderWorld renderWorld(_renderer);

        _renderer.BeginFrame();
        W::world.Accept(renderWorld);
        _renderer.EndFrame(_arcballCamera.GetWorldMatrix(), _arcballCamera.GetProjectionMatrix());
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

    RenderView::RenderView(QWidget* parent) : QGLWidget(parent), _arcballCamera(DEFAULT_WIDTH, DEFAULT_HEIGHT) {
        connect(&_timer, SIGNAL(timeout()), this, SLOT(Update()));
        _timer.start();

        R::Light light;

        float dist = 20;

        light.constantAttenuation   = 1.0f;
        light.linearAttenuation     = 0.01f;
        light.quadricAttenuation    = 0.0f;

        light.position          = M::Vector3(-dist,  dist, dist);
        light.diffuseColor      = R::Color::White;
        _renderer.Add(light);

        light.position          = M::Vector3( dist, -dist, dist);
        light.diffuseColor      = R::Color::White;
        _renderer.Add(light);
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

} // namespace UI
