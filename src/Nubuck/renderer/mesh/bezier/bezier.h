#pragma once

#include <vector>

#include <generic\uncopyable.h>
#include <math\vector2.h>
#include "..\mesh.h"

namespace R {

    // quadric, unconstrained poly-bezier curve.
    class PolyBezier2U : private GEN::Uncopyable {
    private:
        std::vector<M::Vector2> _points;

        std::vector<Mesh::Vertex> _vertices;        
        std::vector<Mesh::Index> _indices;

        // l is the index of the first endpoint of this segment
        M::Vector2 B(int l, float t);

        void Build(void);
    public:
        /*
        format of input: e c e c e ... e c e, where e are endpoints and c are controlpoints.
        more specific: n = points.size() = 2k + 1 for a k > 0 and
        points[2i] is endpoint, points[2i + 1] is controlpoint, for 0 <= i < n - 2
        */
        PolyBezier2U(const std::vector<M::Vector2>& points);

        Mesh::Desc GetDesc(void);
    };

} // namespace R