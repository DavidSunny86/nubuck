#include <assert.h>

#include <QResizeEvent>
#include <Nubuck\common\common.h>
#include <system\opengl\opengl.h>
#include "glwidget.h"

namespace UI {

    void GLWidget::Initialize(void) {
        if(!_isInitialized) {
            _winId = winId();
            _rc = GEN::Pointer<SYS::RenderingContext>(new SYS::RenderingContext(_winId));
            _rc->Use();

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
    }

    SYS::RenderingContext& GLWidget::GetRenderingContext() {
        return *_rc;
    }

    void GLWidget::resizeEvent(QResizeEvent* event) {
        Initialize();
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

    void GLWidget::updateGL(void) {
        Initialize();
        paintGL();
        _rc->Flip();

        const float resizeTimeout = 4.0f;
        if(_resizing && resizeTimeout < _resizeTimer.Stop()) { // Timer::Stop doesn't actually stop the timer!
            finishResizeGL();
            _resizing = false;
        }
    }

} // namespace UI