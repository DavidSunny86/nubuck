#pragma once

#include <renderer\glew\glew.h>

namespace R {

struct State {
    struct Culling {
        struct Hardware {
            GLboolean   enabled;
            GLenum      cullFace;
        } hw;
        struct Software {
            GLboolean   enabled;
            GLenum      cullFace;
            float       alpha;
        } sw;
    } culling;

    struct Blending {
        GLboolean   enabled;
        GLenum      srcFactor;
        GLenum      dstFactor;
    } blend;

    struct DepthBuffer {
        GLboolean   enabled;
        GLboolean   maskEnabled;
        GLenum      func;
    } depth;

    struct StencilBuffer {
        GLboolean   enabled;
        struct Func {
            GLenum  func;
            GLint   ref;
            GLuint  mask;
        } func;
        struct Op {
            struct Front {
                GLenum  fail;
                GLenum  zfail;
                GLenum  zpass;
            } front;
            struct Back {
                GLenum  fail;
                GLenum  zfail;
                GLenum  zpass;
            } back;
        } op;
    } stencil;

    struct Rasterizing {
        float       pointSize;
        float       lineWidth;

        struct LineStipple {
            GLboolean   enabled;
            GLint       factor;
            GLuint      pattern;
        } lineStipple;
    } raster;

    struct ColorBuffer {
        struct Mask {
            GLboolean red;
            GLboolean green;
            GLboolean blue;
            GLboolean alpha;
        } maskEnabled;
    } color;

};

void SetDefaultState(State& state);

} // namespace R