#include <math\matrix3.h>
#include <renderer\renderer_local.h>
#include <renderer\effects\effectmgr.h>
#include <renderer\effects\effect.h>
#include "r_edges_local.h"
#include "r_line_edges.h"

namespace R {

LineEdges g_lineEdges;

// wEye: eye position in world space
void LineEdges::BuildEdgeBillboards(const std::vector<Edge>& edges, const M::Matrix4& world, const M::Vector3& wEye) {
    edgeBBoardsHot.resize(edges.size());

    static const M::Vector3 vertPos[] = {
        M::Vector3( 0.5f,  0.0f, 0.0f),
        M::Vector3( 0.5f,  1.0f, 0.0f),
        M::Vector3(-0.5f,  0.0f, 0.0f),
        M::Vector3(-0.5f,  1.0f, 0.0f)
    };

    unsigned numEdges = edges.size();
    for(unsigned i = 0; i < numEdges; ++i) {
        const Edge& edge = edges[i];

        const M::Vector3 p0 = M::Transform(world, edge.p0);
        const M::Vector3 p1 = M::Transform(world, edge.p1);

        const M::Vector3 wAxisY = M::Normalize(p1 - p0);
        const M::Vector3 wAxisX = M::Normalize(M::Cross(wAxisY, -p0));
        const M::Vector3 wAxisZ = M::Normalize(M::Cross(wAxisX, wAxisY));
        const M::Matrix3 rotate = M::Mat3::FromColumns(wAxisX, wAxisY, wAxisZ); // local to world space

        const float height = M::Distance(p0, p1);
        const M::Matrix3 scale = M::Mat3::Scale(edge.radius, height, 1.0f);

        const M::Matrix3 M = rotate * scale;

        for(unsigned j = 0; j < 4; ++j) {
            edgeBBoardsHot[i].verts[j].position = p0 + M::Transform(M, vertPos[j]);
            edgeBBoardsHot[i].verts[j].color = ColorTo3ub(edge.color);
        }
    }

    edgeBBoardIndices.clear();
    for(unsigned i = 0; i < 4 * numEdges; ++i) {
        if(0 < i && 0 == i % 4) {
            edgeBBoardIndices.push_back(i - 1);
            edgeBBoardIndices.push_back(i);
        }
        edgeBBoardIndices.push_back(i);
    }
    unsigned numBillboardIndices = M::Max(2u, 6 * edges.size()) - 2; // max handles case size = 0
    assert(numBillboardIndices == edgeBBoardIndices.size());
}

void LineEdges::UploadEdgeBillboards(void) {
    unsigned numBillboards = edgeBBoardsHot.size();
    if(!numBillboards) return;

    if(edgeBBoardHotVertexBuffer.IsValid()) edgeBBoardHotVertexBuffer->Destroy();
    edgeBBoardHotVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, &edgeBBoardsHot[0], sizeof(EdgeBBoardHot) * numBillboards));

    if(edgeBBoardIndexBuffer.IsValid()) edgeBBoardIndexBuffer->Destroy();
    edgeBBoardIndexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ELEMENT_ARRAY_BUFFER, &edgeBBoardIndices[0], sizeof(Mesh::Index) * edgeBBoardIndices.size()));
}

void LineEdges::BindEdgeBillboardVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_POSITION,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoardHotVertex),
        (void*)offsetof(EdgeBBoardHotVertex, position)));
    GL_CALL(glEnableVertexAttribArray(IN_POSITION));

    GL_CALL(glVertexAttribPointer(IN_COLOR,
        3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(EdgeBBoardHotVertex),
        (void*)offsetof(EdgeBBoardHotVertex, color)));
    GL_CALL(glEnableVertexAttribArray(IN_COLOR));
}

void LineEdges::DrawEdgeBillboards(const M::Matrix4& worldMat, const M::Matrix4& projectionMat, const char* fxName) {
    if(_edges.empty()) return;

    GEN::Pointer<Effect> fx = effectMgr.GetEffect(fxName);
    fx->Compile();
    Pass* pass = fx->GetPass(0);
    pass->Use();

    Program& prog = pass->GetProgram();
    Uniforms_BindBlocks(prog);
    Uniforms_BindBuffers();
    SetState(pass->GetDesc().state);

    edgeBBoardHotVertexBuffer->Bind();
    BindEdgeBillboardVertices();

    edgeBBoardIndexBuffer->Bind();
    
    unsigned numBillboardIndices = edgeBBoardIndices.size();
    GL_CALL(glDrawElements(GL_TRIANGLE_STRIP, numBillboardIndices, GL_UNSIGNED_INT, NULL));
}

void LineEdges::Draw(std::vector<Edge>& edges) {
    RemoveDegeneratedEdges(edges);
    if(!edges.empty()) {
        _stagedEdgesMtx.Lock();
        _stagedEdges.insert(_stagedEdges.end(), edges.begin(), edges.end());
        _stagedEdgesMtx.Unlock();
    }
}

void LineEdges::R_Prepare(const M::Matrix4& worldMat) {
    _stagedEdgesMtx.Lock();
    _edges = _stagedEdges;
    _stagedEdges.clear();
    _stagedEdgesMtx.Unlock();

    M::Matrix4 invWorld;
    bool ret = M::TryInvert(worldMat, invWorld);
    const M::Vector3 eyePos = M::Transform(invWorld, M::Vector3::Zero);
    BuildEdgeBillboards(_edges, worldMat, eyePos);
}

void LineEdges::R_Draw(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) {
    UploadEdgeBillboards();
    DrawEdgeBillboards(worldMat, projectionMat, "EdgeLineBillboard");
}

} // namespace R