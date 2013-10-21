#pragma once

#include <vector>
#include <renderer\mesh\mesh.h>

namespace R {

class Plane {
private:
    std::vector<Mesh::Vertex>   _vertices;
    std::vector<Mesh::Index>    _indices;

    struct AdjVert { int drow, dcol; };
public:
    typedef float (*heightFunc_t)(float x, float y);

    Plane(int subdiv, float size, heightFunc_t heightFunc, bool flip = false);

    Mesh::Desc GetDesc(void);
};

} // namespace R