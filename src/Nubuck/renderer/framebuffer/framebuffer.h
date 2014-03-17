#pragma once

#include <renderer\glew\glew.h>

namespace R {

class Renderbuffer;
class Texture;

class Framebuffer {
private:
    GLuint _id;
public:
    struct Type {
        enum Enum {
            DEPTH_ATTACHMENT    = GL_DEPTH_ATTACHMENT,
            COLOR_ATTACHMENT_0  = GL_COLOR_ATTACHMENT0,
            COLOR_ATTACHMENT_1  = GL_COLOR_ATTACHMENT1,
            COLOR_ATTACHMENT_2  = GL_COLOR_ATTACHMENT2,
            COLOR_ATTACHMENT_3  = GL_COLOR_ATTACHMENT3
        };
    };

    Framebuffer();
    ~Framebuffer();

    // do not delete buffers that are attached to unbound framebuffers!
    void Attach(GLenum type, const Renderbuffer& renderbuffer);
    void Attach(GLenum type, const Texture& texture);
    void Bind();
};

void BindWindowSystemFramebuffer();

} // namespace R