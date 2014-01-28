#pragma once

#include <math\matrix4.h>
#include <renderer\mesh\meshmgr_fwd.h>
#include <renderer\mesh\mesh.h>
#include <renderer\giantbuffer\giant_buffer.h>

namespace R {

class TFMesh {
private:
    M::Matrix4                  _transform;
    meshPtr_t                   _meshPtr;

    std::vector<Mesh::Vertex>   _tfverts;
    gbHandle_t                  _gbHandle;
public:
    explicit TFMesh(const meshPtr_t meshPtr);
    ~TFMesh();

    const Mesh& GetMesh() const;
    Mesh&       GetMesh();

    const M::Matrix4& GetTransform() const { return _transform; }

    void SetTransform(const M::Matrix4& transform);

    void TransformVertices();

    void        R_TF_Touch();
    unsigned    R_TF_IndexOff() const;

    void        R_Touch();
    unsigned    R_IndexOff() const;
};

} // namespace R