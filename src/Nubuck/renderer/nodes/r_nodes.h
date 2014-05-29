#pragma once

#include <vector>

#include <LEDA\graph\graph.h>
#include <Nubuck\polymesh_fwd.h>
#include <Nubuck\math\vector3.h>
#include <Nubuck\renderer\color\color.h>
#include <renderer\renderer.h>

namespace M { struct Matrix4; }

namespace R {

class Nodes {
public:
    struct Node {
        leda::node  pvert; // corresponding vertex of polymesh
        M::Vector3  position;
        R::Color    color;
    };
public:
    virtual ~Nodes() { }

    virtual bool IsEmpty() const = 0;

    virtual void Rebuild(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) = 0;
    virtual void Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) = 0;

    virtual void SetColor(leda::node pv, const Color& color) = 0;

    virtual void Transform(const M::Matrix4& objToWorld) = 0;

    virtual void BuildRenderMesh() = 0;
    virtual void DestroyRenderMesh() = 0;

    virtual R::MeshJob GetRenderJob() const = 0;
};

} // namespace R