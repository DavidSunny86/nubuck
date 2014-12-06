#include <renderer\glew\glew.h>
#include <Nubuck\common\common.h>
#include "renderer.h"

#define CASE_ENUM_STR(x) case x: return #x

namespace R {

    const char* SourceToStr(GLenum source) {
        switch(source) {
        CASE_ENUM_STR(GL_DEBUG_SOURCE_API_ARB);
        CASE_ENUM_STR(GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
        CASE_ENUM_STR(GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
        CASE_ENUM_STR(GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
        CASE_ENUM_STR(GL_DEBUG_SOURCE_APPLICATION_ARB);
        CASE_ENUM_STR(GL_DEBUG_SOURCE_OTHER_ARB);
        };
        return "<unknown source>";
    }

    const char* TargetToStr(GLenum target) {
        switch(target) {
        CASE_ENUM_STR(GL_DEBUG_TYPE_ERROR_ARB);
        CASE_ENUM_STR(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
        CASE_ENUM_STR(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
        CASE_ENUM_STR(GL_DEBUG_TYPE_PORTABILITY_ARB);
        CASE_ENUM_STR(GL_DEBUG_TYPE_PERFORMANCE_ARB);
        CASE_ENUM_STR(GL_DEBUG_TYPE_OTHER_ARB);
        };
        return "<unkown target>";
    }

    const char* SeverityToStr(GLenum severity) {
        switch(severity) {
        CASE_ENUM_STR(GL_DEBUG_SEVERITY_HIGH_ARB);
        CASE_ENUM_STR(GL_DEBUG_SEVERITY_MEDIUM_ARB);
        CASE_ENUM_STR(GL_DEBUG_SEVERITY_LOW_ARB);
        CASE_ENUM_STR(GL_DEBUG_SEVERITY_NOTIFICATION); // KHR_debug ext
        };
        return "<unknown severity>";
    }

    void __stdcall DebugOutputCallback(
        GLenum source,
        GLenum target,
        GLuint id,
        GLenum severity,
        GLsizei /* length */,
        const char* message,
        void* /* userParam */)
    {
        const char* fmt =
            "GL_DEBUG_OUTPUT {\n"
            "   source: %s\n"
            "   target: %s\n"
            "   id: %d\n"
            "   severity: %s\n"
            "   message: %s\n"
            "}\n";
        /*
        FIXME: using common.printf triggers breakpoints all over
        the place
        */
        printf(fmt,
            SourceToStr(source),
            TargetToStr(target),
            id,
            SeverityToStr(severity),
            message);
    }

    void InitDebugOutput(void) {
        PFNGLDEBUGMESSAGECALLBACKARBPROC DebugMessageCallbackARB = NULL;
		DWORD lastError = GetLastError();
        if(DebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB")) {
            common.printf("INFO - GL_ARB_debug_output supported.\n");
            DebugMessageCallbackARB(DebugOutputCallback, NULL);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        } else {
			common.printf("INFO - GL_ARB_debug_output not supported.\n");
			SetLastError(lastError);
		}
    }

} // namespace R