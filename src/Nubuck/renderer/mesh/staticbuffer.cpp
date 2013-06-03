#include <renderer\metrics\metrics.h>
#include "../glcall.h"
#include "staticbuffer.h"

namespace R {

    StaticBuffer::StaticBuffer(GLenum type, const GLvoid* data, GLsizeiptr size) : _type(type), _id(0) {
        GL_CALL(glGenBuffers(1, &_id));
        GL_CALL(glBindBuffer(type, _id));
        GL_CALL(glBufferData(type, size, data, GL_DYNAMIC_DRAW )); // TODO
        GL_CALL(glBindBuffer(type, 0)); // TODO: bind previously bound buffer

        metrics.resources.totalVertexBufferSize += size;
    }

    StaticBuffer::~StaticBuffer(void) {
    }

    void StaticBuffer::Bind(void) const {
        GL_CALL(glBindBuffer(_type, _id));
    }

} // namespace R
