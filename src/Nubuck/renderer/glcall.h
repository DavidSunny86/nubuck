#pragma once

#include <common\common.h>

#define GL_CALL(x) \
    do { \
        x; \
        GLint error; \
        while((error = glGetError())) { \
            common.printf("calling %s failed with error %X in %s:%d\n", \
                #x, error, __FILE__, __LINE__); \
            Crash(); \
        } \
    } while(false)

#define GL_CHECK_ERROR \
    do { \
        GLint error; \
        if((error = glGetError())) { \
            common.printf("ERROR - in %s:%d: glGetError() == %d, '%s'.\n", \
                __FILE__, __LINE__, error, gluErrorString(error)); \
            Crash(); \
        } \
    } while(false)
