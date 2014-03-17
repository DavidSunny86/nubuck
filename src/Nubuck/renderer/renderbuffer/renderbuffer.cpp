#include "renderbuffer.h"

namespace R {

Renderbuffer::Renderbuffer(GLenum type, int width, int height)
    : _width(width), _height(height)
{
    glGenRenderbuffersEXT(1, &_id);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _id);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, type, width, height);
}

Renderbuffer::~Renderbuffer() {
    glDeleteRenderbuffersEXT(1, &_id);
}

} // namespace R