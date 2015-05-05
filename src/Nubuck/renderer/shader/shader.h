#pragma once

#include <vector>
#include <string>
#include <Nubuck\generic\uncopyable.h>
#include <renderer\glew\glew.h>

namespace R {

struct AttributeLocation {
    std::string name;
    int         loc;
};

class Shader : private GEN::Uncopyable {
private:
    GLuint                          _id;
    std::vector<AttributeLocation>  _attribLocs;
    std::vector<std::string>        _materialUniforms;
public:
    enum Type {
        VERTEX = 0,
        FRAGMENT,
        GEOMETRY,

        NUM_TYPES
    };

    Shader(Type type, const GLchar* source);
    ~Shader(void);

    GLuint GetID(void) const { return _id; }

    const std::vector<AttributeLocation>&   GetAttributeLocations() const { return _attribLocs; }
    const std::vector<std::string>&         GetMaterialUniforms() const { return _materialUniforms; }
};

} // namespace R
