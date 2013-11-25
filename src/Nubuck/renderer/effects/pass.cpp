#include <fstream>

#include <common\common.h>
#include <renderer\shader\shader.h>
#include "pass.h"

namespace {

    std::string ReadFile(const std::string& filename) {
        std::string text;

        std::ifstream file(filename.c_str());

        if(!file.is_open()) {
            common.printf("ReadFile: unable to open file '%s'.\n", filename.c_str());
            Crash();
        }

        std::string line;
        while(file.good()) {
            std::getline(file, line);
            text += line + '\n';
        }

        return text;
    }

    std::string PreprocessShaderSource(const std::string& source) {
        if(source.empty()) return std::string();
        std::string out;
        unsigned i = 0;
        while(i < source.length()) {
            std::string line;
            while(i < source.length() && '\n' != source[i]) 
                line += source[i++];
            line += source[i++];
            ctoken_t* tokens = NULL;
            COM_Tokenize(&tokens, line.c_str(), '\0');
            ctoken_t* tok = tokens;
            if(tok && '#' == tok->string[0]) {
                if(!strncmp("#include", tok->string, MAX_TOKEN)) {
                    tok = tok->next;
                    unsigned tokLen = strnlen(tok->string, MAX_TOKEN);
                    if('<' != tok->string[0] || 1 > tokLen || '>' != tok->string[tokLen - 1]) {
                        common.printf("PreprocessShaderSource: invalid syntax for include path. expected '<PATH>', got '%s'\n", tok->string);
                        Crash();
                    }
                    if(tok->next) {
                        common.printf("PreprocessShaderSource: expected end of line after '#include <...>', got '%s'\n", tok->string);
                        Crash();
                    }
                    std::string incl(tok->string + 1, tokLen - 2);
                    out += PreprocessShaderSource(ReadFile(common.BaseDir() + incl));
                } else {
                    printf("PreprocessShaderSource: unknown directive '%s'\n", tok->string);
                    Crash();
                }
            } else out += line;
            COM_FreeTokens(tokens);
        }
        return out;
    }

} // unnamed namespace

namespace R {

    void Pass::Compile(void) {
        if(_compiled) return;

        for(int i = 0; i < Shader::NUM_TYPES; ++i) {
            const std::string& filename = _desc.filenames[i];
            if(!filename.empty()) {
                std::string source = PreprocessShaderSource(filename);
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
