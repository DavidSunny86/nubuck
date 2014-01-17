#pragma once

#include <generic\uncopyable.h>
#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>
#include "r_edge.h"

namespace R {

class CylinderEdges : private GEN::Uncopyable, public Renderable {
private:
    struct EdgeBBoxVertex {
        M::Vector3  position;
        Color4ub    color;
        M::Vector3  A[4];
        float       halfHeightSq;
        float       radiusSq;
    };

    SYS::SpinLock               _stagedEdgesMtx;
    std::vector<Edge>           _stagedEdges;

    std::vector<Edge>           _edges;               
    std::vector<EdgeBBoxVertex> edgeBBoxVertices;
    std::vector<Mesh::Index>    edgeBBoxIndices;
    GEN::Pointer<StaticBuffer>  edgeBBoxVertexBuffer;
    GEN::Pointer<StaticBuffer>  edgeBBoxIndexBuffer;

    void BindEdgeBBoxAttributes(void);
    void UnbindAttributes(void);
    void ReserveEdgeBBoxBuffers(void);
    void CreateEdges(void);
    void UploadEdges(void);
    void DrawEdges(const M::Matrix4& worldMat, const M::Matrix4& projectionMat);
public:
    void Draw(std::vector<Edge>& edges);

    void R_Prepare(const M::Matrix4& worldMat) override;
    void R_Draw(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) override;
};

extern CylinderEdges g_cylinderEdges;

} // namespace R