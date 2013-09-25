#pragma once

#include <vector>
#include <generic\uncopyable.h>
#include "..\mesh.h"

namespace R {

    class Cylinder : private GEN::Uncopyable {
    private:
        std::vector<Mesh::Vertex> _vertices;
        std::vector<Mesh::Index> _indices;
    public:
        Cylinder(float radius, float height, unsigned numSlices = 10, bool caps = true);

        Mesh::Desc GetDesc(void);
    };

} // namespace R