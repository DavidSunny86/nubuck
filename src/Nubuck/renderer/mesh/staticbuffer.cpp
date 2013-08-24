#include <renderer\metrics\metrics.h>
#include "../glcall.h"
#include "staticbuffer.h"

namespace R {

    StaticBuffer::StaticBuffer(GLenum type, const GLvoid* data, GLsizeiptr size) : _type(type), _id(0), _size(size) {
        GL_CALL(glGenBuffers(1, &_id));
        GL_CALL(glBindBuffer(type, _id));
        GL_CALL(glBufferData(type, size, data, GL_STATIC_DRAW ));
        GL_CALL(glBindBuffer(type, 0)); // TODO: bind previously bound buffer

        metrics.resources.totalVertexBufferSize += size;
    }

    StaticBuffer::~StaticBuffer(void) {
    }

    void StaticBuffer::Bind(void) const {
        GL_CALL(glBindBuffer(_type, _id));
    }

    void StaticBuffer::Discard(void) {
        GL_CALL(glBindBuffer(_type, _id));
        GL_CALL(glBufferData(_type, _size, NULL, GL_STATIC_DRAW));
    }

    void StaticBuffer::Update(const GLvoid* data, GLsizeiptr size) {
        assert(size <= _size);
        GL_CALL(glBindBuffer(_type, _id));
        GL_CALL(glBufferData(_type, size, data, GL_STATIC_DRAW));
    }

} // namespace R
