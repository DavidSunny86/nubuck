#include <stdio.h>
#include <tchar.h>

#include <Nubuck\common\common.h>
#include <system\winerror.h>
#include <system\opengl\opengl.h>
#include <renderer\glew\glew.h>
#include "debugtext.h"

namespace {

unsigned GetGlyphHeight(SYS::DeviceContext& dc, unsigned glyph) {
    MAT2 ident;
    FIXED f1;
    FIXED f0;
    f1.value = 1;
    f1.fract = 0;
    f0.value = 0;
    f0.fract = 0;
    ident.eM11 = ident.eM22 = f1;
    ident.eM12 = ident.eM21 = f0;

    GLYPHMETRICS metrics;
    GetGlyphOutline(dc.GetNativeHandle(), glyph, GGO_METRICS, &metrics, 0, NULL, &ident);
    return metrics.gmBlackBoxY;
}

inline void RasterPos2(const M::Vector2& v) {
    glRasterPos2f(v.x, v.y);
}

} // unnamed namespace

namespace R {

DebugText::DebugText() : _padding(2.0f, 2.0f) {
}

void DebugText::Init(SYS::DeviceContext& dc) {
    LOGFONT logFont;
    memset(&logFont, 0, sizeof(LOGFONT));
    logFont.lfHeight = 20;
    wcscpy(logFont.lfFaceName, TEXT("Arial"));
    HFONT font = CreateFontIndirect(&logFont);
    HGDIOBJ oldFont = SelectObject(dc.GetNativeHandle(), font);
    if(!wglUseFontBitmaps(dc.GetNativeHandle(), 0, 255, FONT_LIST_BASE)) {
        common.printf("ERROR - wglUseFontBitmaps() failed.\n");
        CHECK_WIN_ERROR;
    }
    _glyphHeight = GetGlyphHeight(dc, 'A');
    SelectObject(dc.GetNativeHandle(), oldFont);
}

void DebugText::Resize(int width, int height) {
    _width = width;
    _height = height;
}

void DebugText::BeginFrame() {
    glUseProgram(0);
    glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_LIST_BIT | GL_TRANSFORM_BIT);
    glDisable(GL_DEPTH_TEST);
    glColor3f(0.0f, 0.0f, 0.0f);
    glListBase(FONT_LIST_BASE);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, _width, _height, 0.0f, -1.0f, 1.0); // origin in upper left hand corner
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    _penPos = _padding + M::Vector2(0.0f, _glyphHeight);
    RasterPos2(_penPos);
}

void DebugText::EndFrame() {
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void DebugText::Printf(const char* format, ...) {
    static char buffer[4096];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    COM::ItTokenizer toks(buffer, "\n");
    COM::ItTokenizer::Token tok = toks.NextToken();
    while(toks.IsValid(tok)) {
        glCallLists(toks.Length(tok), GL_UNSIGNED_BYTE, &buffer[toks.StartIndex(tok)]);
        if(4096 > toks.EndIndex(tok) && '\n' == buffer[toks.EndIndex(tok)]) {
            _penPos.y += _padding.y + _glyphHeight;
            RasterPos2(_penPos);
        }
        tok = toks.NextToken();
    }
}

} // namespace R