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

    struct PlaneDesc {
        struct Sample2 { float x, y; };
        typedef float (*heightFunc_t)(float x, float y);

        heightFunc_t    heightFunc;
        bool            flip;
        float           size;
        int             subdiv;
        Sample2*        addSamples;
        unsigned        numAddSamples;
    };

    Plane(const PlaneDesc& desc);

    Mesh::Desc GetDesc(void);
};

} // namespace R