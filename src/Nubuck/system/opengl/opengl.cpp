#include <renderer\glew\glew.h>
#include <renderer\glew\wglew.h>

#include <common\common.h>
#include <renderer\glcall.h>

#include <system\winerror.h>
#include "..\window\window.h"
#include "opengl.h"

namespace SYS {

    DeviceContext::DeviceContext(Window& window)
        : _windowHandle(window.GetNativeHandle()) 
    {
        _contextHandle = GetDC(_windowHandle);
    }

    DeviceContext::DeviceContext(HWND windowHandle) : _windowHandle(windowHandle) {
        _contextHandle = GetDC(_windowHandle);
    }

    DeviceContext::~DeviceContext(void) {
        ReleaseDC(_windowHandle, _contextHandle);
    }

    bool RenderingContext::_extensionsInitialized = false;
    int RenderingContext::_major = -1;
    int RenderingContext::_minor = -1;

    void RenderingContext::InitExtensions(void) {
        if(_extensionsInitialized) return;

        Window dummyWindow(NULL, TEXT(""), 0, 0);
        dummyWindow.SetExitOnClose(false);

        DeviceContext dummyDeviceContext(dummyWindow);

        PIXELFORMATDESCRIPTOR pfd;
        if(FALSE == SetPixelFormat(dummyDeviceContext.GetNativeHandle(), 1, &pfd)) {
            common.printf("ERROR - SetPixelFormat failed.\n");
            CHECK_WIN_ERROR;
        }

        HGLRC dummyGLRC = wglCreateContext(dummyDeviceContext.GetNativeHandle());
        wglMakeCurrent(dummyDeviceContext.GetNativeHandle(), dummyGLRC);

        if(0 == dummyDeviceContext.GetNativeHandle() || 0 == dummyGLRC) {
            common.printf("ERROR - creating dummy context failed.\n");
        }

        GLenum ret;
        if(GLEW_OK != (ret = glewInit())) {
            common.printf("ERROR - glewInit() failed with code %d, \"%s\"\n",
                    ret, glewGetErrorString(ret));
            Crash();
        }
        common.printf("INFO - using glew version %s.\n", glewGetString(GLEW_VERSION));

        const GLubyte* glVersion = glGetString(GL_VERSION);
        common.printf("INFO - supported GL version: '%s'.\n", glVersion);
        /* requires GL v3.0 or greater
        glGetIntegerv(GL_MAJOR_VERSION, &_major);
        glGetIntegerv(GL_MINOR_VERSION, &_minor);
        */

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(dummyGLRC);

        _extensionsInitialized = true;
    }

    void RenderingContext::Init(void) {
        InitExtensions();
        CHECK_WIN_ERROR;

        int msaa_enabled    = GL_FALSE;
        int msaa_numSamples = 0;

        if(GLEW_ARB_multisample) {
            msaa_enabled    = GL_TRUE;
            msaa_numSamples = 2;
        }
        common.printf("INFO - MSAA enabled: %d, number of samples: %d\n", msaa_enabled, msaa_numSamples);

        int pixAttribs[] = {
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_ACCELERATION_ARB,	WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,		32,
            WGL_DEPTH_BITS_ARB,		24,
            WGL_STENCIL_BITS_ARB,   8,
            WGL_DOUBLE_BUFFER_ARB,	GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB,	msaa_enabled,
            WGL_SAMPLES_ARB,		msaa_numSamples,
            WGL_PIXEL_TYPE_ARB,		WGL_TYPE_RGBA_ARB,
            0
        };

        int pixFormat = -1;
        unsigned pixCount = 0;

        wglChoosePixelFormatARB(_deviceContext.GetNativeHandle(),
            &pixAttribs[0], NULL, 1,
            &pixFormat, &pixCount);

        if(0 > pixFormat) {
            common.printf("ERROR - wglChoosePixelFormatARB failed. pixelFormat is %d.\n", pixFormat);
        }

        PIXELFORMATDESCRIPTOR pfd;
        if(FALSE == SetPixelFormat(_deviceContext.GetNativeHandle(), pixFormat, &pfd)) {
            common.printf("ERROR - SetPixelFormat failed.\n");
            CHECK_WIN_ERROR;
        }

        /*
        GLint attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0
        };
        */
        GLint attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
            WGL_CONTEXT_MINOR_VERSION_ARB, 1,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            0
        };

        _contextHandle = wglCreateContextAttribsARB(_deviceContext.GetNativeHandle(), NULL, attribs);

        if(NULL == _contextHandle) {
            common.printf("ERROR - wglCreateContextAttribs failed. major = %d, minor = %d.\n",
                attribs[1], attribs[3]);
        }

        CHECK_WIN_ERROR;
    }

    RenderingContext::RenderingContext(Window& window) 
        : _contextHandle(NULL), _deviceContext(window), _initialized(false) 
    {
        Init();
    }

    RenderingContext::RenderingContext(HWND windowHandle)
        : _contextHandle(NULL), _deviceContext(windowHandle), _initialized(false)
    {
        Init();
    }

    RenderingContext::~RenderingContext(void) {
        if(NULL != _contextHandle) {
            /*
            TODO: crashes on nvidia 540m
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(_contextHandle);
            */
        }
    }

    void RenderingContext::Use(void) {
        wglMakeCurrent(_deviceContext.GetNativeHandle(), _contextHandle);

        CHECK_WIN_ERROR;
        GL_CHECK_ERROR;
    }

    void RenderingContext::Flip(void) {
        wglSwapLayerBuffers(_deviceContext.GetNativeHandle(), WGL_SWAP_MAIN_PLANE);
    }

    bool IsRenderingContextActive(void) {
        return NULL != wglGetCurrentContext();
    }

} // namespace SYS
