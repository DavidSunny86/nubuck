#include <fstream>

#include <Nubuck\common\common.h>
#include <renderer\shader\shader.h>
#include "pass.h"

namespace R {

    void Pass::Compile(void) {
        if(_compiled) return;

        for(int i = 0; i < Shader::NUM_TYPES; ++i) {
            const std::string& filename = _desc.filenames[i];
            if(!filename.empty()) {
                std::string source = filename;
                _program.Attach(Shader::Type(i), source.c_str());
            }
        }
        _program.Link();

        _compiled = true;
    }

    Pass::Pass(const PassDesc& desc) : _desc(desc), _program(), _compiled(false) {
    }

    void Pass::Init(void) {
        if(_compiled) return;

        _program.Init();
        Compile();
    }

    const Program& Pass::GetProgram(void) const { return _program; }

    const PassDesc& Pass::GetDesc(void) const { return _desc; }

    void Pass::Use(void) const {
        if(!_compiled) {
            common.printf("using uncompiled pass '%s'.\n", _desc.name);
            Crash();
        }
        _program.Use();
    }

    Program& Pass::GetProgram(void) { return _program; }

    void Pass::Use(void) {
        if(!_compiled) Compile();
        _program.Use();
    }

} // namespace R
