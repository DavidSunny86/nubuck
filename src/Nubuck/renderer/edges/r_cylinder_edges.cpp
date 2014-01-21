#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include "r_cylinder_edges.h"

namespace R {

void CylinderEdges::DestroyMesh() {
    if(_mesh) {
        meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
}

static M::Matrix4 RotationOf(const M::Matrix4& mat) {
    return M::Matrix4(
        mat.m00, mat.m01, mat.m02, 0.0f,
        mat.m10, mat.m11, mat.m12, 0.0f,
        mat.m20, mat.m21, mat.m22, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

void CylinderEdges::RebuildVertices(const M::Matrix4& transform) {
    _edgeBBoxVertices.clear();
    for(unsigned i = 0; i < _edges.size(); ++i) {
        const FatEdge& edge = _edges[i];
        const M::Vector3 p0 = M::Transform(transform, edge.p0);
        const M::Vector3 p1 = M::Transform(transform, edge.p1);
        const M::Vector3 center = 0.5f * (p1 + p0);
        M::Matrix4 objectToWorld = M::Mat4::Translate(center) * R::RotationOf(transform) * edge.Rt;

        const float h = 0.5f * M::Length(p1 - p0);
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
        Mesh::Vertex vertex;
        // vertex.color = ColorTo4ub(edge.color);
        vertex.color = edge.color;
        vertex.A[0] = M::Vector3(objectToWorld.m00, objectToWorld.m10, objectToWorld.m20);
        vertex.A[1] = M::Vector3(objectToWorld.m01, objectToWorld.m11, objectToWorld.m21);
        vertex.A[2] = M::Vector3(objectToWorld.m02, objectToWorld.m12, objectToWorld.m22);
        vertex.A[3] = M::Vector3(objectToWorld.m03, objectToWorld.m13, objectToWorld.m23);
        /*
        vertex.halfHeightSq = h * h;
        vertex.radiusSq = edge.radius * edge.radius;
        */
        vertex.texCoords.x = h * h;
        vertex.texCoords.y = edge.radius * edge.radius;
        for(unsigned i = 0; i < numVertices; ++i) {
            vertex.position = M::Transform(objectToWorld, bboxVertexPositions[i]);
            _edgeBBoxVertices.push_back(vertex);
        }
    } // forall edges

    if(_mesh) meshMgr.GetMesh(_mesh).Invalidate(&_edgeBBoxVertices[0]);
}

CylinderEdges::~CylinderEdges() {
    DestroyMesh();
}

void CylinderEdges::Clear() {
    _edges.clear();
    DestroyMesh();
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

void CylinderEdges::Rebuild() {
    // RemoveDegeneratedEdges(_edges);
    if(_edges.empty()) return;

    for(unsigned i = 0; i < _edges.size(); ++i) {
        FatEdge& edge = _edges[i];
        M::Matrix4 R = AlignZ(edge.p1 - edge.p0);
        edge.Rt = M::Transpose(R);
    }

    std::vector<Mesh::Index> edgeBBoxIndices;
    unsigned baseIdx = 0;
    for(unsigned i = 0; i < _edges.size(); ++i) {
        const unsigned numVertices = 8;
        Mesh::Index bboxIndices[] = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
        const unsigned numIndices = 14;
        for(unsigned i = 0; i < numIndices; ++i) edgeBBoxIndices.push_back(baseIdx + bboxIndices[i]);
        edgeBBoxIndices.push_back(Mesh::RESTART_INDEX);
        baseIdx += numVertices;
    }

    RebuildVertices(M::Mat4::Identity());

    assert(NULL == _mesh);

    Mesh::Desc meshDesc;
    meshDesc.vertices = &_edgeBBoxVertices[0];
    meshDesc.numVertices = _edgeBBoxVertices.size();
    meshDesc.indices = &edgeBBoxIndices[0];
    meshDesc.numIndices = edgeBBoxIndices.size();
    meshDesc.primType = GL_TRIANGLE_STRIP;

    _mesh = meshMgr.Create(meshDesc);
}

void CylinderEdges::SetTransform(const M::Matrix4& transform, const M::Matrix4&) {
    RebuildVertices(transform);
}

MeshJob CylinderEdges::GetRenderJob() const {
    assert(!IsEmpty());

    MeshJob meshJob;
    meshJob.fx = "EdgeBillboard";
    meshJob.mesh = _mesh;
    meshJob.material = Material::White;
    meshJob.primType = 0;
    meshJob.transform = M::Mat4::Identity();
    return meshJob;
}

} // namespace R