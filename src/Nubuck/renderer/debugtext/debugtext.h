#pragma once

#include <Windows.h>
#include <Nubuck\math\vector2.h>

namespace SYS { class DeviceContext; }

namespace R {

class DebugText {
private:
    enum { FONT_LIST_BASE = 1000 };

    HFONT       _font;
    M::Vector2  _padding;
    unsigned    _glyphHeight;

    int         _width, _height;

    M::Vector2  _penPos;
public:
    DebugText();

    void Init(SYS::DeviceContext& dc);

    void Resize(int width, int height);

    void BeginFrame();
    void Printf(const char* format, ...);
    void EndFrame();
};

} // namespace R