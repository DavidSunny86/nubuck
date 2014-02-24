#pragma once

#include <Nubuck\common\common.h>

#ifdef NUBUCK_GLCALL
#define GL_CALL(x) \
    do { \
        x; \
        GLint error; \
        while((error = glGetError())) { \
            common.printf("ERROR - in %s:%d: calling %s failed with error 0x%X, '%s'.\n", \
                __FILE__, __LINE__, #x, error, gluErrorString(error)); \
            Crash(); \
        } \
    } while(false)
#else
#define GL_CALL(x) x
#endif // NUBUCK_GLCALL

#define GL_CHECK_ERROR \
    do { \
        GLint error; \
        if((error = glGetError())) { \
            common.printf("ERROR - in %s:%d: glGetError() == 0x%X, '%s'.\n", \
                __FILE__, __LINE__, error, gluErrorString(error)); \
            Crash(); \
        } \
    } while(false)
