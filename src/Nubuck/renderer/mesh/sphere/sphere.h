#pragma once

#include <generic\uncopyable.h>
#include "..\mesh.h"

namespace R {

    class Sphere : private GEN::Uncopyable {
    private:
        Mesh::Vertex* _vertices;
        Mesh::Index* _indices;
        unsigned _numVerts;
    public:
        Sphere(int numSubdiv, bool smooth);
        ~Sphere(void);

        Mesh::Desc GetDesc(void);
    };

} // namespace R