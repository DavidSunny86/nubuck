#include <assert.h>

#include <QResizeEvent>
#include <Nubuck\common\common.h>
#include <system\opengl\opengl.h>
#include "glwidget.h"

namespace UI {

GEN::Pointer<SYS::RenderingContext> GLWidget::shared_rc;

SYS::DeviceContext& GLWidget::GetDeviceContext() {
    COM_assert(_dc.IsValid());
    return *_dc;
}

SYS::RenderingContext& GLWidget::GetRenderingContext() {
    COM_assert(shared_rc.IsValid());
    return *shared_rc;
}

void GLWidget::resizeEvent(QResizeEvent* event) {
    Use();
    resizeGL(event->size().width(), event->size().height());
    _resizing = true;
    _resizeTimer.Start();
}

void GLWidget::paintEvent(QPaintEvent*) {
    updateGL();
}

GLWidget::GLWidget(QWidget* parent) 
    : QWidget(parent)
    , _winId(NULL)
    , _isInitialized(false)
    , _resizing(false)
{
    setAttribute(Qt::WA_PaintOnScreen, true); // avoids flimmering
}

void GLWidget::Use() {
    if(!_isInitialized) {
        _winId = winId();

        assert(!_dc.IsValid());
        _dc = GEN::MakePtr(new SYS::DeviceContext(_winId));
        _dc->SetPixelFormat();

        if(!shared_rc.IsValid()) {
            shared_rc = GEN::Pointer<SYS::RenderingContext>(new SYS::RenderingContext(_dc->GetNativeHandle()));
        }
        shared_rc->MakeCurrent(_dc->GetNativeHandle());

        initializeGL();
        _isInitialized = true;
    }

    if(winId() != _winId) {
        const char* msg =
            "ERROR - GLWidget: the window handle became invalid. the most likely reason "
            "for this is reparenting of the widget. don't do this.\n";
        common.printf(msg);
        Crash();
    }

    shared_rc->MakeCurrent(_dc->GetNativeHandle());
}

void GLWidget::updateGL() {
    Use();
    paintGL();
    _dc->Flip();

    const float resizeTimeout = 4.0f;
    if(_resizing && resizeTimeout < _resizeTimer.Stop()) { // Timer::Stop doesn't actually stop the timer!
        finishResizeGL();
        _resizing = false;
    }
}

} // namespace UI