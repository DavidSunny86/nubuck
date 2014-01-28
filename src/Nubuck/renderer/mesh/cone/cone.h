#pragma once

#include <vector>
#include <renderer\mesh\mesh.h>

namespace R {

class Cone {
private:
    std::vector<Mesh::Vertex>   _vertices;
    std::vector<Mesh::Index>    _indices;
public:
    Cone(float radius, float height, int numSlices = 10, const Color& color = Color::White);

    Mesh::Desc GetDesc();
};

} // namespace R