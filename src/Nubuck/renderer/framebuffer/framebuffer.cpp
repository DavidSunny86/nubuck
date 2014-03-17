#include <renderer\renderbuffer\renderbuffer.h>
#include <renderer\textures\texture.h>
#include "framebuffer.h"

namespace R {

Framebuffer::Framebuffer() {
    glGenFramebuffersEXT(1, &_id);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffersEXT(1, &_id);
}

void Framebuffer::Attach(GLenum type, const Renderbuffer& renderbuffer) {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _id);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, type, GL_RENDERBUFFER_EXT, renderbuffer.GetID());
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    // TODO: error checking!
}

void Framebuffer::Attach(GLenum type, const Texture& texture) {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _id);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, type, GL_TEXTURE_2D, texture.GetID(), 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    // TODO: error checking!
}

void Framebuffer::Bind() {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _id);
}

void BindWindowSystemFramebuffer() {
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

} // namespace R