#include <math\matrix3.h>
#include <renderer\renderer_local.h>
#include <renderer\effects\effectmgr.h>
#include <renderer\effects\effect.h>
#include <renderer\program\program.h>
#include "r_nodes.h"

namespace R {

Nodes g_nodes;

void Nodes::ReserveBillboards(void) {
    unsigned numBillboards = _nodes.size();
    if(numBillboards <= billboardsHot.size()) return;

    unsigned numBillboardIndices = 6 * numBillboards - 2;

    billboardsHot.clear();
    billboardsHot.resize(numBillboards);

    static const BillboardColdVertex coldVertices[4] = {
        { M::Vector2( 1.0f, -1.0f) },
        { M::Vector2( 1.0f,  1.0f) },
        { M::Vector2(-1.0f, -1.0f) },
        { M::Vector2(-1.0f,  1.0f) }
    };
    billboardsCold.clear();
    billboardsCold.resize(numBillboards);
    for(unsigned i = 0; i < numBillboards; ++i) {
        memcpy(billboardsCold[i].verts, coldVertices, sizeof(coldVertices));
    }

    billboardIndices.clear();
    billboardIndices.reserve(numBillboardIndices);
    for(unsigned i = 0; i < 4 * numBillboards; ++i) {
        if(0 < i && 0 == i % 4) {
            billboardIndices.push_back(i - 1);
            billboardIndices.push_back(i);
        }
        billboardIndices.push_back(i);
    }
    assert(numBillboardIndices == billboardIndices.size());
}

void Nodes::ReserveBillboardBuffers(void) {
    unsigned numBillboards = _nodes.size();
    if(billboardHotVertexBuffer.IsValid() && sizeof(BillboardHot) * numBillboards <= billboardHotVertexBuffer->GetSize()) return;

    if(billboardHotVertexBuffer.IsValid()) billboardHotVertexBuffer->Destroy();
    billboardHotVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, NULL, sizeof(BillboardHot) * numBillboards));

    if(billboardColdVertexBuffer.IsValid()) billboardColdVertexBuffer->Destroy();
    billboardColdVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, &billboardsCold[0], sizeof(BillboardCold) * numBillboards));

    unsigned numBillboardIndices = 6 * numBillboards - 2;
    if(billboardIndexBuffer.IsValid()) billboardIndexBuffer->Destroy();
    billboardIndexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ELEMENT_ARRAY_BUFFER, &billboardIndices[0], sizeof(Mesh::Index) * numBillboardIndices));
}

// camera position at 0
static M::Matrix4 Billboard_FaceCamera(const M::Matrix4& worldMat, const M::Vector3& billboardPos) {
    const M::Vector3 p = M::Transform(worldMat, billboardPos);
    const M::Vector3 v2 = M::Normalize(-M::Vector3(p.x, 0.0f, p.z));
    const M::Vector3 v1 = M::Vector3(0.0f, 1.0f, 0.0f);
    const M::Vector3 v0 = M::Normalize(M::Cross(v1, v2));
    const M::Matrix3 M = M::Mat3::FromColumns(v0, v1, v2);
    const M::Vector3 w2 = M::Normalize(-M::Vector3(0.0f, p.y, p.z));
    const M::Vector3 w0 = M::Vector3(1.0f, 0.0f, 0.0f);
    const M::Vector3 w1 = M::Normalize(M::Cross(w0, w2));
    const M::Matrix3 N = M::Mat3::FromColumns(w0, w1, w2);
    return M::Mat4::FromRigidTransform(M * N, p);
}

// omits scale
static M::Matrix4 Billboard_FaceViewingPlane(const M::Matrix4& worldMat, const M::Vector3& billboardPos) {
    const M::Vector3 p = M::Transform(worldMat, billboardPos);
    return M::Matrix4(
        1.0f, 0.0f, 0.0f, worldMat.m03 + p.x,
        0.0f, 1.0f, 0.0f, worldMat.m13 + p.y,
        0.0f, 0.0f, 1.0f, worldMat.m23 + p.z,
        0.0f, 0.0f, 0.0f, 1.0f);
}

void Nodes::BuildBillboards(const M::Matrix4& worldMat) {
	float nodeSize = cvar_r_nodeSize;
    const BillboardHotVertex hotVertices[4] = {
        { M::Vector3( nodeSize, -nodeSize, 0.0f) },
        { M::Vector3( nodeSize,  nodeSize, 0.0f) },
        { M::Vector3(-nodeSize, -nodeSize, 0.0f) },
        { M::Vector3(-nodeSize,  nodeSize, 0.0f) }
    };
    unsigned numBillboards = _nodes.size();
    for(unsigned i = 0; i < numBillboards; ++i) {
        for(unsigned k = 0; k < 4; ++k) {
            billboardsHot[i].verts[k].position = M::Transform(worldMat, _nodes[i].position) + hotVertices[k].position;
            billboardsHot[i].verts[k].color = _nodes[i].color;
        }
    }
}

void Nodes::Draw(const std::vector<Node>& nodes) {
    if(!nodes.empty()) {
        _stagedNodesMtx.Lock();
        _stagedNodes.insert(_stagedNodes.end(), nodes.begin(), nodes.end());
        _stagedNodesMtx.Unlock();
    }
}

void Nodes::BindHotBillboardVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_POSITION,
        3, GL_FLOAT, GL_FALSE, sizeof(BillboardHotVertex),
        (void*)offsetof(BillboardHotVertex, position)));
    GL_CALL(glEnableVertexAttribArray(IN_POSITION));

    GL_CALL(glVertexAttribPointer(IN_COLOR,
        4, GL_FLOAT, GL_FALSE, sizeof(BillboardHotVertex),
        (void*)offsetof(BillboardHotVertex, color)));
    GL_CALL(glEnableVertexAttribArray(IN_COLOR));
}

void Nodes::BindColdBillboardVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_TEXCOORDS,
        2, GL_FLOAT, GL_FALSE, sizeof(BillboardColdVertex),
        (void*)offsetof(BillboardColdVertex, texCoords)));
    GL_CALL(glEnableVertexAttribArray(IN_TEXCOORDS));
}

void Nodes::R_Prepare(const M::Matrix4& worldMat) {
    _stagedNodesMtx.Lock();
    _nodes = _stagedNodes;
    _stagedNodes.clear();
    _stagedNodesMtx.Unlock();

    if(!_nodes.empty()) {
        ReserveBillboards();
        BuildBillboards(worldMat);
    }
}

void Nodes::DrawBillboards(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) {
    unsigned numBillboards = _nodes.size();
    unsigned numBillboardIndices = 6 * numBillboards - 2;

    GEN::Pointer<Effect> fx = effectMgr.GetEffect("NodeBillboard");
    fx->Compile();
    Pass* pass = fx->GetPass(0);
    pass->Use();

    Program& prog = pass->GetProgram();
    Uniforms_BindBlocks(prog);
    Uniforms_BindBuffers();
    SetState(pass->GetDesc().state);

    billboardHotVertexBuffer->Bind();
    BindHotBillboardVertices();

    billboardColdVertexBuffer->Bind();
    BindColdBillboardVertices();

    billboardIndexBuffer->Bind();

    glDrawElements(GL_TRIANGLE_STRIP, numBillboardIndices, GL_UNSIGNED_INT, NULL);
}

void Nodes::R_Draw(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) {
    unsigned numBillboards = _nodes.size();
    if(numBillboards) {
        ReserveBillboardBuffers();
        billboardHotVertexBuffer->Update_Mapped(0, sizeof(BillboardHot) * numBillboards, &billboardsHot[0]);
        DrawBillboards(worldMat, projectionMat);
        billboardHotVertexBuffer->Discard();
    }
}

} // namespace R