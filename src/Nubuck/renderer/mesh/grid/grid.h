#pragma once

#include <vector>
#include <renderer\mesh\mesh.h>

namespace R {

class Grid {
private:
    std::vector<Mesh::Vertex>   _vertices;
    std::vector<Mesh::Index>    _indices;
public:
    Grid(int subdiv, float size);

    Mesh::Desc GetDesc();
};

} // namespace R