#pragma once

#include <vector>

#include <LEDA\core\list.h>

#include <math\vector2.h>

#include "..\mesh.h"

namespace R {

    leda::list<M::Vector2> ChaikinSubdiv(const leda::list<M::Vector2>& polygon);

    class PolygonMesh {
    private:
        std::vector<Vertex> _vertices;
        std::vector<Index>  _indices;
    public:
        PolygonMesh(const leda::list<M::Vector2>& polygon);

        MeshDesc GetSolidDesc(void);
    };

} // namespace R