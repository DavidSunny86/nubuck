#pragma once

#include <vector>

#include <generic\uncopyable.h>
#include <math\vector2.h>
#include "..\mesh.h"

namespace R {

    class Bezier : private GEN::Uncopyable {
    private:
        std::vector<M::Vector2> _points;

        std::vector<Mesh::Vertex> _vertices;        
        std::vector<Mesh::Index> _indices;

        M::Vector2 B(float t);

        void Build(void);
    public:
        Bezier(const std::vector<M::Vector2>& points);

        Mesh::Desc GetDesc(void);
    };

} // namespace R