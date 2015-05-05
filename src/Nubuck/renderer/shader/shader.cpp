#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
#include <Windows.h> // OutputDebugString
#endif

#include <renderer\glew\glew.h>
#include <renderer\glcall.h>
#include <renderer\shader\shader_preprocessor\shader_preprocessor.h>
#include "shader.h"

namespace R {

// index with Shader::Type
static GLenum glenums[] = {
    GL_VERTEX_SHADER,       // VERTEX
    GL_FRAGMENT_SHADER,     // FRAGMENT
    GL_GEOMETRY_SHADER      // GEOMETRY
};

// index with Shader::Type
static const char* names[] = {
    "GL_VERTEX_SHADER",     // VERTEX
    "GL_FRAGMENT_SHADER",   // FRAGMENT
    "GL_GEOMETRY_SHADER"    // GEOMETRY
};

Shader::Shader(Type type, const GLchar* source) {
    std::string ppsource;
    if(!SPP::PreprocessShaderSource(source, ppsource, _attribLocs, _materialUniforms)) {
        common.printf("PreprocessShaderSource failed.\n");
        common.printf("<<<<<<<<<<<<<\n");
        common.printf("%s\n", source);
        common.printf(">>>>>>>>>>>>>\n");
        Crash();
    }

    _id = glCreateShader(glenums[type]);
    if(!_id) {
        common.printf("glCreateShader(%s) failed.\n", names[type]);
        Crash();
    }

    const char* preamble =
        // "#version 120                                           \n"
        "#version 330                                           \n" // TODO: let effect chose glsl version
        "#extension GL_ARB_uniform_buffer_object : enable       \n"
        "\n";

    const GLchar* combined[] = { preamble, ppsource.c_str() };

    GL_CALL(glShaderSource(_id, 2, combined, NULL));
    GL_CALL(glCompileShader(_id));

    GLint compiled = GL_FALSE;
    GL_CALL(glGetShaderiv(_id, GL_COMPILE_STATUS, &compiled));
    if(GL_TRUE != compiled) {
        GLint len = 0;
        GL_CALL(glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &len));
        if(0 < len) {
            GLchar* buffer = (GLchar*)malloc(len);
            if(buffer) {
                GL_CALL(glGetShaderInfoLog(_id, len, NULL, buffer));
                std::string msg;
                msg.append("compiling shader of type '").append(names[type]).append("' failed: \n");
                msg.append("<<<<<\n");
                msg.append(combined[0]).append(combined[1]);
                msg.append(">>>>>\n");
                msg.append(buffer).append("\n");
                common.printf("%s", msg.c_str());
#ifdef _WIN32
                OutputDebugStringA(msg.c_str());
#endif
                free(buffer);
            }
        }
    }
    assert(GL_TRUE == compiled);
}

Shader::~Shader(void) {
    glDeleteShader(_id);
}

} // namespace R
