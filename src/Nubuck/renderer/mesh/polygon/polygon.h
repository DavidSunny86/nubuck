#pragma once

#include <vector>

#include <Windows.h> // defines MAX_INT
#include <LEDA\core\list.h>

#include <Nubuck\math\vector2.h>
#include <Nubuck\math\matrix3.h>
#include <Nubuck\math\matrix4.h>

#include "..\mesh.h"

namespace R {

    template<typename TYPE> // TYPE in {Vector2, Vector3}
    void Subdiv(leda::list<TYPE>& polygon);

    template<typename TYPE> // TYPE in {Vector2, Vector3}
    leda::list<TYPE> ChaikinSubdiv(const leda::list<TYPE>& polygon);

    class PolygonMesh {
    private:
        std::vector<Mesh::Vertex> _vertices;
        std::vector<Mesh::Index>  _indices;
    public:
        PolygonMesh(const leda::list<M::Vector2>& polygon, const M::Vector3& normal);
        PolygonMesh(leda::list<M::Vector3>& polygon, const M::Vector3& normal);

        void Transform(const M::Matrix3& mat);
        void Transform(const M::Matrix4& mat);

        Mesh::Desc GetDesc(void);
    };

} // namespace R

#include "polygon_inl.h"