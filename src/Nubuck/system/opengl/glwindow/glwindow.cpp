#include <Windows.h>
#include <gl\GLU.h>

#include <renderer\glcall.h>

#include "glwindow.h"

namespace SYS {

void GLWindow::ResizeEvent(unsigned width, unsigned height) {
    base_t::ResizeEvent(width, height);

    glViewport(0, 0, width, height);
}

GLWindow::GLWindow(HINSTANCE instance, const TCHAR* title, unsigned width, unsigned height, DWORD style)
    : Window(instance, title, width, height, style),
      _time(0.0f),
      _frameTime(0.0f),
      _frames(0),
      _fps(0)
{
    _dc = GEN::MakePtr(new DeviceContext(GetNativeHandle()));
    _rc = GEN::MakePtr(new RenderingContext(_dc->GetNativeHandle()));
    _rc->MakeCurrent(_dc->GetNativeHandle());

    glClearColor(0.0f, 0.125f, 0.6f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);

    ResizeEvent(width, height);
}

void GLWindow::operator()(float secsPassed) {
    _frameTime += secsPassed;
    if(1.0f <= _frameTime) {
        _fps = _frames;
        _frames = 0;
        _frameTime = 0.0f;
    }
    Render();
    _frames++;
    _dc->Flip();
    
    if(1.0f <= secsPassed) {
        secsPassed = 1.0f;
    }

    _time += secsPassed;

    Move(secsPassed);
}

} // namespace SYS