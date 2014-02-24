#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
#include <Windows.h> // OutputDebugString
#endif

#include <Nubuck\renderer\color\color.h>

#include <Nubuck\common\common.h>
#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix3.h>
#include <Nubuck\math\matrix4.h>
#include <renderer\glcall.h>
#include <renderer\shader\shader.h>
#include <renderer\program\program.h>

namespace R {

    enum { INVALID_ID = 0 };

    void Program::BindAttributeLocations(const std::vector<AttributeLocation>& locs) {
        assert(INVALID_ID != _id);
        for(std::vector<AttributeLocation>::const_iterator it(locs.begin()); locs.end() != it; ++it) {
            GL_CALL(glBindAttribLocation(_id, it->loc, it->name.c_str()));
        }
    }

    Program::Program(void) : _id(INVALID_ID), _linked(false) {
    }

    Program::~Program(void) {
        if(INVALID_ID != _id) GL_CALL(glDeleteProgram(_id));
    }

    void Program::Init(void) {
        _id = glCreateProgram();
        if(INVALID_ID == _id) {
            common.printf("creating program failed.\n");
            Crash();
        }
    }

    void Program::Use(void) const {
        if(!_linked) {
            common.printf("using unlinked program.\n");
            Crash();
        }
        GL_CALL(glUseProgram(_id));
    }

    void Program::Attach(const Shader& shader) {
        _linked = false;
        GL_CALL(glAttachShader(_id, shader.GetID()));
        BindAttributeLocations(shader.GetAttributeLocations());
    }

    // type must be in Shader::Type
    void Program::Attach(int type, const GLchar* source) {
        Shader shader(Shader::Type(type), source);
        Attach(shader);
    }

    void Program::Link(void) {
        GL_CALL(glLinkProgram(_id));

        GLint linked = GL_FALSE;
        GL_CALL(glGetProgramiv(_id, GL_LINK_STATUS, &linked));
        if(GL_TRUE != linked) {
            GLint len = 0;
            GL_CALL(glGetProgramiv(_id, GL_INFO_LOG_LENGTH, &len));
            if(0 < len) {
                GLchar* buffer = (GLchar*)malloc(len);
                if(buffer) {
                    GL_CALL(glGetProgramInfoLog(_id, len, NULL, buffer));
                    common.printf("linking program failed: %s\n", buffer);
#ifdef _WIN32
                    std::string msg;
                    msg.append("linking program failed: \n");
                    msg.append(buffer).append("\n");
                    OutputDebugStringA(msg.c_str());
#endif
                    free(buffer);
                }
            }
        }
        assert(GL_TRUE == linked);
        _linked = true;
    }

    void Program::Use(void) {
        if(!_linked) Link();
        GL_CALL(glUseProgram(_id));
    }

    GLint Program::GetUniformLocation(const char* name) {
        std::unordered_map<std::string, GLint>::const_iterator locIt(_uniformLocations.find(name));
        if(_uniformLocations.end() != locIt) return locIt->second;
        if(!_linked) Link();
        GLint loc = glGetUniformLocation(_id, name);
        if(0 > loc) {
            common.printf("uniform %s not found.\n", name);
#ifdef _WIN32
            std::string msg;
            msg.append("uniform ").append(name).append(" not found");
            OutputDebugStringA(msg.c_str());
#endif
            Crash();
        }
        _uniformLocations[name] = loc;
        return loc;
    }

    GLint Program::GetAttributeLocation(const char* name) {
        if(!_linked) Link();
        GLint loc = glGetAttribLocation(_id, name);
        if(0 > loc) {
            common.printf("attribut %s not found.\n", name);
            Crash();
        }
        return loc;
    }

    void Program::SetUniform(const char* name, int value) {
        GLint loc = GetUniformLocation(name);
        if(0 <= loc) glUniform1i(loc, value);
    }

    void Program::SetUniform(const char* name, float value) {
        GLint loc = GetUniformLocation(name);
        if(0 <= loc) glUniform1f(loc, value);
    }

    void Program::SetUniform(const char* name, const M::Vector3& vector) {
        GLint loc = GetUniformLocation(name);
        if(0 <= loc) glUniform3f(loc, vector.x, vector.y, vector.z);
    }

    void Program::SetUniform(const char* name, const M::Matrix3& matrix) {
        GLint loc = GetUniformLocation(name);
        if(0 <= loc) glUniformMatrix3fv(loc, 1, GL_FALSE, matrix.mat);
    }

    void Program::SetUniform(const char* name, const M::Matrix4& matrix) {
        GLint loc = GetUniformLocation(name);
        if(0 <= loc) glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.mat);
    }

    void Program::SetUniform(const char* name, const R::Color& color) {
        GLint loc = GetUniformLocation(name);
        if(0 <= loc) glUniform4f(loc, color.r, color.g, color.b, color.a);
    }

    void Program::UnbindAll(void) {
        GL_CALL(glUseProgram(0));
    }

} // namespace R
