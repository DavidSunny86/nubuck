#pragma once

#include <Nubuck\generic\uncopyable.h>
#include <renderer\glew\glew.h>

namespace R {

class Renderbuffer : private GEN::Uncopyable {
private:
    GLuint  _id;
    int     _width;
    int     _height;
public:
    struct Type {
        enum {
            // must be color-renderable, depth-renderable or stencil-renderable
            RGBA4           = GL_RGBA4,
            DEPTH_COMPONENT = GL_DEPTH_COMPONENT
        };
    };

    Renderbuffer(GLenum type, int width, int height);
    ~Renderbuffer();

    GLuint  GetID() const { return _id; }
    int     Width() const { return _width; }
    int     Height() const { return _height; }
};

} // namespace R