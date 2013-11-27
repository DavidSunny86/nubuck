#include <renderer\renderer_local.h>
#include <renderer\effects\effectmgr.h>
#include <renderer\effects\effect.h>
#include "r_edges_local.h"
#include "r_cylinder_edges.h"

namespace R {

CylinderEdges g_cylinderEdges;

void CylinderEdges::BindEdgeBBoxVertices(void) {
    GL_CALL(glVertexAttribPointer(IN_POSITION,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)offsetof(EdgeBBoxVertex, position)));
    GL_CALL(glEnableVertexAttribArray(IN_POSITION));

    GL_CALL(glVertexAttribPointer(IN_COLOR,
        4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(EdgeBBoxVertex),
        (void*)offsetof(EdgeBBoxVertex, color)));
    GL_CALL(glEnableVertexAttribArray(IN_COLOR));

    GL_CALL(glVertexAttribPointer(IN_A0,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 0)));
    GL_CALL(glEnableVertexAttribArray(IN_A0));

    GL_CALL(glVertexAttribPointer(IN_A1,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 1)));
    GL_CALL(glEnableVertexAttribArray(IN_A1));

    GL_CALL(glVertexAttribPointer(IN_A2,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 2)));
    GL_CALL(glEnableVertexAttribArray(IN_A2));

    GL_CALL(glVertexAttribPointer(IN_A3,
        3, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, A) + sizeof(M::Vector3) * 3)));
    GL_CALL(glEnableVertexAttribArray(IN_A3));

    GL_CALL(glVertexAttribPointer(IN_HALF_HEIGHT_SQ,
        1, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, halfHeightSq))));
    GL_CALL(glEnableVertexAttribArray(IN_HALF_HEIGHT_SQ));

    GL_CALL(glVertexAttribPointer(IN_RADIUS_SQ,
        1, GL_FLOAT, GL_FALSE, sizeof(EdgeBBoxVertex),
        (void*)(offsetof(EdgeBBoxVertex, radiusSq))));
    GL_CALL(glEnableVertexAttribArray(IN_RADIUS_SQ));
}

void CylinderEdges::ReserveEdgeBBoxBuffers(void) {
    // numbers per bbox
    const unsigned numVertices = 8;
    const unsigned numIndices = 14 + 1; // including restart index

    unsigned numEdges = _edges.size();

    unsigned vbSize = sizeof(EdgeBBoxVertex) * numVertices * numEdges;
    unsigned ibSize = sizeof(Mesh::Index) * numIndices * numEdges;

    if(edgeBBoxVertexBuffer.IsValid() && edgeBBoxVertexBuffer->GetSize() >= vbSize) return;

    if(edgeBBoxVertexBuffer.IsValid()) edgeBBoxVertexBuffer->Destroy();
    edgeBBoxVertexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ARRAY_BUFFER, NULL, vbSize));
    if(edgeBBoxIndexBuffer.IsValid()) edgeBBoxIndexBuffer->Destroy();
    edgeBBoxIndexBuffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL, ibSize));
}

// rotates the coordinate space such that the new z axis coincides with the vector d.
// example. AlignZ(d) * d = Length(d) * (0, 0, 1)
static M::Matrix4 AlignZ(const M::Vector3& d) {
    const float len_yz_sq = d.y * d.y + d.z * d.z;
    const float len = d.Length();
    assert(0.0f < len);
    if(0.0f == len_yz_sq) { // case d, x collinear => rot_x = 0
        return M::Matrix4(
            0.0f, 0.0f, -d.x / len, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            d.x / len, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
            );
    } else {
        const float len_yz = sqrt(len_yz_sq);
        return M::Matrix4(
            len_yz * len_yz, -d.x * d.y, -d.x * d.z, 0.0f,
            0.0f, len * d.z, -len * d.y, 0.0f,
            len_yz * d.x, len_yz * d.y, len_yz * d.z, 0.0f,
            0.0f, 0.0f, 0.0f, len_yz * len
            ) / (len_yz * len);
    }
}

void CylinderEdges::CreateEdges(void) {
    if(_edges.empty()) return;

    edgeBBoxVertices.clear();
    edgeBBoxIndices.clear();
    unsigned baseIdx = 0;
    for(unsigned i = 0; i < _edges.size(); ++i) {
        const Edge& edge = _edges[i];
        const M::Vector3 center = 0.5f * (edge.p1 + edge.p0);
        M::Matrix4 R = AlignZ(edge.p1 - edge.p0);
        M::Matrix4 objectToWorld = M::Mat4::Translate(center) * M::Transpose(R);

        const float h = 0.5f * M::Length(edge.p1 - edge.p0);
        const float r = edge.radius;
        M::Vector3 bboxVertexPositions[] = {
            M::Vector3( r,  r, -h),
            M::Vector3(-r,  r, -h),
            M::Vector3( r,  r,  h),
            M::Vector3(-r,  r,  h),
            M::Vector3( r, -r, -h),
            M::Vector3(-r, -r, -h),
            M::Vector3(-r, -r,  h),
            M::Vector3( r, -r,  h)
        };
        const unsigned numVertices = 8;
        Mesh::Index bboxIndices[] = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
        const unsigned numIndices = 14;
        EdgeBBoxVertex vertex;
        vertex.color = ColorTo4ub(edge.color);
        vertex.A[0] = M::Vector3(objectToWorld.m00, objectToWorld.m10, objectToWorld.m20);
        vertex.A[1] = M::Vector3(objectToWorld.m01, objectToWorld.m11, objectToWorld.m21);
        vertex.A[2] = M::Vector3(objectToWorld.m02, objectToWorld.m12, objectToWorld.m22);
        vertex.A[3] = M::Vector3(objectToWorld.m03, objectToWorld.m13, objectToWorld.m23);
        vertex.halfHeightSq = h * h;
        vertex.radiusSq = edge.radius * edge.radius;
        for(unsigned i = 0; i < numVertices; ++i) {
            vertex.position = M::Transform(objectToWorld, bboxVertexPositions[i]);
            edgeBBoxVertices.push_back(vertex);
        }
        for(unsigned i = 0; i < numIndices; ++i) edgeBBoxIndices.push_back(baseIdx + bboxIndices[i]);
        edgeBBoxIndices.push_back(Mesh::RESTART_INDEX);
        baseIdx += numVertices;
    } // for all edges
}

void CylinderEdges::UploadEdges(void) {
    if(_edges.empty()) return;
    edgeBBoxVertexBuffer->Update_Mapped(0, sizeof(EdgeBBoxVertex) * edgeBBoxVertices.size(), &edgeBBoxVertices[0]);
    edgeBBoxIndexBuffer->Update_Mapped(0, sizeof(Mesh::Index) * edgeBBoxIndices.size(), &edgeBBoxIndices[0]);
}

void CylinderEdges::DrawEdges(const M::Matrix4& projectionMat, const M::Matrix4& worldMat) {
    GEN::Pointer<Effect> fx = effectMgr.GetEffect("EdgeBillboard");
    // GEN::Pointer<Effect> fx = effectMgr.GetEffect("GenericWireframe");
    fx->Compile();
    Pass* pass = fx->GetPass(0);
    pass->Use();

    Program& prog = pass->GetProgram();
    Uniforms_BindBlocks(prog);
    Uniforms_BindBuffers();
    SetState(pass->GetDesc().state);

    edgeBBoxVertexBuffer->Bind();
    BindEdgeBBoxVertices();

    edgeBBoxIndexBuffer->Bind();

    glDrawElements(GL_TRIANGLE_STRIP, edgeBBoxIndices.size(), GL_UNSIGNED_INT, NULL);
}

void CylinderEdges::Draw(std::vector<Edge>& edges) {
    RemoveDegeneratedEdges(edges);
    if(!edges.empty()) {
        _stagedEdgesMtx.Lock();
        _stagedEdges.insert(_stagedEdges.end(), edges.begin(), edges.end());
        _stagedEdgesMtx.Unlock();
    }
}

void CylinderEdges::R_Prepare(const M::Matrix4&) {
    _stagedEdgesMtx.Lock();
    _edges = _stagedEdges;
    _stagedEdges.clear();
    _stagedEdgesMtx.Unlock();

    CreateEdges();
}

void CylinderEdges::R_Draw(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) {
    ReserveEdgeBBoxBuffers();
    UploadEdges();
    DrawEdges(worldMat, projectionMat);
}

} // namespace R