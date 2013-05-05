#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
#include <Windows.h> // OutputDebugString
#endif

#include <renderer\glew\glew.h>
#include <renderer\glcall.h>
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
        _id = glCreateShader(glenums[type]);
        if(!_id) {
            common.printf("glCreateShader(%s) failed.\n", names[type]);
            Crash();
        }

		const GLchar* combined[] = { "#version 330\n\n", source };

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
                    common.printf("compiling shader of type %s failed: %s\n", names[type], buffer);
#ifdef _WIN32
					std::string msg;
					msg.append("compiling shader of type '").append(names[type]).append("' failed: \n");
                    msg.append(buffer).append("\n");
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
