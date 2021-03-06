#pragma once

#include <Nubuck\generic\uncopyable.h>
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

        void SetColor(const Color& color);
        void Scale(float scale);

        Mesh::Desc GetDesc(void);
    };

} // namespace R