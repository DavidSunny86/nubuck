#pragma once

#include <Nubuck\generic\uncopyable.h>
#include <renderer\glew\glew.h>

namespace R {

    class StaticBuffer : private GEN::Uncopyable {
    private:
        GLenum      _type;
        GLuint      _id;
        GLsizeiptr  _size;
    public:
        StaticBuffer(GLenum type, const GLvoid* data, GLsizeiptr size);
        ~StaticBuffer(void);

        void Destroy(void);

        GLuint GetID(void) const { return _id; }
        GLsizeiptr GetSize(void) const { return _size; }

        void Bind(void) const;

        void Discard(void); // Update(NULL), orphans buffer
        void Update_SubData(GLintptr offset, GLsizeiptr size, const GLvoid* data);
        void Update_Mapped(GLintptr offset, GLsizeiptr size, const GLvoid* data);
    };

} // namespace R
