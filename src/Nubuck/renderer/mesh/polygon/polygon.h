#pragma once

#include <vector>

#include <LEDA\core\list.h>

#include <math\vector2.h>
#include <math\matrix3.h>
#include <math\matrix4.h>

#include "..\mesh.h"

namespace R {

    template<typename TYPE> // TYPE in {Vector2, Vector3}
    void Subdiv(leda::list<TYPE>& polygon);

    template<typename TYPE> // TYPE in {Vector2, Vector3}
    leda::list<TYPE> ChaikinSubdiv(const leda::list<TYPE>& polygon);

    class PolygonMesh {
    private:
        std::vector<Vertex> _vertices;
        std::vector<Index>  _indices;
    public:
        PolygonMesh(const leda::list<M::Vector2>& polygon, const M::Vector3& normal);
        PolygonMesh(leda::list<M::Vector3>& polygon, const M::Vector3& normal);

        void Transform(const M::Matrix3& mat);
        void Transform(const M::Matrix4& mat);

        MeshDesc GetSolidDesc(void);
    };

} // namespace R

#include "polygon_inl.h"