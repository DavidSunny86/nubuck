#pragma once

#include <vector>
#include <generic\uncopyable.h>
#include <renderer\renderer.h>
#include "r_edge.h"

namespace R {

class LineEdges : private GEN::Uncopyable, public Renderable {
private:
    struct EdgeBBoardHotVertex {
        M::Vector3  position;
        Color3ub    color;
    };

    struct EdgeBBoardHot {
        EdgeBBoardHotVertex verts[4];
    };

    SYS::SpinLock               _stagedEdgesMtx;
    std::vector<Edge>           _stagedEdges;

    std::vector<Edge>           _edges;
    std::vector<EdgeBBoardHot>  edgeBBoardsHot;
    std::vector<Mesh::Index>    edgeBBoardIndices;
    GEN::Pointer<StaticBuffer>  edgeBBoardHotVertexBuffer;
    GEN::Pointer<StaticBuffer>  edgeBBoardIndexBuffer;

    void BuildEdgeBillboards(const std::vector<Edge>& edges, const M::Matrix4& world, const M::Vector3& wEye);
    void UploadEdgeBillboards(void);
    void BindEdgeBillboardVertices(void);
    void DrawEdgeBillboards(const M::Matrix4& worldMat, const M::Matrix4& projectionMat, const char* fxName);
public:
    void Draw(std::vector<Edge>& edges);

    void R_Prepare(const M::Matrix4& worldMat) override;
    void R_Draw(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) override;
};

extern LineEdges g_lineEdges;

} // namespace R