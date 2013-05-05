#pragma once

#include <generic\uncopyable.h>
#include <renderer\glew\glew.h>

namespace R {

    class StaticBuffer : private GEN::Uncopyable {
    private:
        GLenum _type;
        GLuint _id;
    public:
        StaticBuffer(GLenum type, const GLvoid* data, GLsizeiptr size);
        ~StaticBuffer(void);

        GLuint GetID(void) const { return _id; }

        void Bind(void) const;
    };

} // namespace R
