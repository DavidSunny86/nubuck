#include <renderer\metrics\metrics.h>
#include "../glcall.h"
#include "staticbuffer.h"

namespace R {

    StaticBuffer::StaticBuffer(GLenum type, const GLvoid* data, GLsizeiptr size) : _type(type), _id(0), _size(size) {
        GL_CALL(glGenBuffers(1, &_id));
        GL_CALL(glBindBuffer(type, _id));
        GL_CALL(glBufferData(type, size, data, GL_DYNAMIC_DRAW));
        GL_CALL(glBindBuffer(type, 0)); // TODO: bind previously bound buffer

        metrics.resources.totalVertexBufferSize += size;
    }

    StaticBuffer::~StaticBuffer(void) {
    }

    void StaticBuffer::Destroy(void) {
        GL_CALL(glDeleteBuffers(1, &_id));
        metrics.resources.totalVertexBufferSize -= _size;
        _id = 0;
        _size = 0;
    }

    void StaticBuffer::Bind(void) const {
        GL_CALL(glBindBuffer(_type, _id));
    }

    void StaticBuffer::Discard(void) {
        GL_CALL(glBindBuffer(_type, _id));
        GL_CALL(glBufferData(_type, _size, NULL, GL_STATIC_DRAW));
    }

    void StaticBuffer::Update_SubData(GLintptr offset, GLsizeiptr size, const GLvoid* data) {
        assert(offset + size <= _size);
        GL_CALL(glBindBuffer(_type, _id));
        GL_CALL(glBufferSubData(_type, offset, size, data));
    }

    void StaticBuffer::Update_Mapped(GLintptr offset, GLsizeiptr size, const GLvoid* data) {
        assert(size <= _size);
        GL_CALL(glBindBuffer(_type, _id));
        void* ptr = glMapBufferRange(_type, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        assert(ptr);
        memcpy(ptr, data, size);
        GL_CALL(glUnmapBuffer(_type));
    }

} // namespace R
