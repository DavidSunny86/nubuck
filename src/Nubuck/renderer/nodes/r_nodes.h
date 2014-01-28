#pragma once

#include <vector>

#include <Nubuck\renderer\color\color.h>

#include <generic\uncopyable.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr_fwd.h>

namespace R {

class Nodes : private GEN::Uncopyable {
public:
    struct Node {
        M::Vector3  position;
        R::Color    color;
    };
private:
    struct Billboard { Mesh::Vertex verts[4]; };

    std::vector<Node>               _nodes;
    std::vector<Billboard>          _billboards;
    std::vector<Mesh::Index>        _billboardIndices;
    meshPtr_t                       _mesh;
    tfmeshPtr_t                     _tfmesh;

    void DestroyMesh();
public:
    Nodes() : _mesh(NULL), _tfmesh(NULL) { }
    ~Nodes();

    bool IsEmpty() const { return _nodes.empty(); }

    void Clear();
    void Push(const Node& node) { _nodes.push_back(node); }
    void Rebuild();

    void Transform(const M::Matrix4& modelView);

    R::MeshJob GetRenderJob() const;    
};

} // namespace R