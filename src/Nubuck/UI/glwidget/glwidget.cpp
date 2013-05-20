#include <assert.h>

#include <QResizeEvent>
#include <common\common.h>
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

    void GLWidget::resizeEvent(QResizeEvent* event) {
        Initialize();
        resizeGL(event->size().width(), event->size().height());
    }

    void GLWidget::paintEvent(QPaintEvent*) {
        updateGL();
    }

    GLWidget::GLWidget(QWidget* parent) : QWidget(parent), _winId(NULL), _isInitialized(false) {
        setAttribute(Qt::WA_PaintOnScreen, true); // avoids flimmering
    }

    void GLWidget::updateGL(void) {
        Initialize();
        paintGL();
        _rc->Flip();
    }

} // namespace UI