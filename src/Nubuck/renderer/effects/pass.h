#pragma once

#include <string>

#include <generic\pointer.h>
#include <generic\uncopyable.h>
#include <renderer\color\color.h>
#include <renderer\shader\shader.h>
#include <renderer\program\program.h>

namespace R {

    struct State {
        struct {
            GLboolean   enabled;
            GLenum      srcFactor;
            GLenum      dstFactor;
        } blend;

        struct {
            GLboolean   enabled;
            GLboolean   maskEnabled;
            GLenum      func;
        } depth;

        struct {
            float       lineWidth;
        } raster;

        Color color;

        void SetDefault(void) {
            blend.enabled       = GL_FALSE;
            blend.srcFactor     = GL_ONE;
            blend.dstFactor     = GL_ZERO;

            depth.enabled       = GL_TRUE;
            depth.maskEnabled   = GL_TRUE;
            depth.func          = GL_LESS;

            raster.lineWidth    = 1.0f;

            color               = Color::White;
        }
    };

    enum PassType {
        DEFAULT,            // draw scene once
        FIRST_LIGHT_PASS,   // draw scene once for first light source
        LIGHT_PASS          // draw scene once for every light source except the first one
    };

    enum PassFlags {
        USE_COLOR       = (1 << 0),
        USE_TEX_DIFFUSE = (1 << 1),
        USE_TIME        = (1 << 2),
        USE_MATERIAL    = (1 << 3)
    };

    struct PassDesc {
        std::string name;
        std::string filenames[Shader::NUM_TYPES]; // index with Shader::Type

        State       state;
        PassType    type;
        int         flags;
    };

    class Pass : private GEN::Uncopyable {
    private:
        PassDesc    _desc;
        Program     _program;
        bool        _compiled;

        void Compile(void);
    public:
        explicit Pass(const PassDesc& desc);

        void Init(void); // requiers gl context

        const Program&  GetProgram(void) const;
        const PassDesc& GetDesc(void) const;
        void            Use(void) const;

        Program&    GetProgram(void);
        void        Use(void);
    };


} // namespace R
