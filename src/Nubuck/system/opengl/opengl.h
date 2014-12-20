#pragma once

#include <exception>
#include <Windows.h>

#include <Nubuck\generic\uncopyable.h>

namespace SYS {

struct PixelFormatException : std::exception {
    const char* what() const {
        return "setting pixel format failed";
    }
};

class DeviceContext : private GEN::Uncopyable {
protected:
    HWND    _hwnd;
    HDC     _hdc;
public:
    explicit DeviceContext(const HWND hwnd);
    virtual ~DeviceContext();

    HDC GetNativeHandle() { return _hdc; }

    void SetPixelFormat();
    void Flip();
};

class RenderingContext : private GEN::Uncopyable {
private:
    HGLRC   _hrc;
    bool    _initialized;

    static bool _extensionsInitialized;
    static int _major, _minor;
    static void InitExtensions();
public:
    explicit RenderingContext(const HDC hdc);
    ~RenderingContext();

    void MakeCurrent(const HDC hdc);
};

bool IsRenderingContextActive();
void InitializeGLExtensions();

} // namespace SYS