#include <maxint.h>

#include <Nubuck\polymesh.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include "r_cylinder_edges.h"

namespace R {

void CylinderEdges::DestroyMesh() {
    if(_mesh) {
        meshMgr.Destroy(_mesh);
        _mesh = NULL;
    }
    if(_tfmesh) {
        meshMgr.Destroy(_tfmesh);
        _tfmesh = NULL;
    }
}

static M::Matrix4 RotationOf(const M::Matrix4& mat) {
    return M::Matrix4(
        mat.m00, mat.m01, mat.m02, 0.0f,
        mat.m10, mat.m11, mat.m12, 0.0f,
        mat.m20, mat.m21, mat.m22, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

void CylinderEdges::RebuildVertices(unsigned edgeIdx, const M::Matrix4& transform) {
    const FatEdge& edge = _edges[edgeIdx];
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
    R::Color bboxVertexColors[] = {
        edge.color0,
        edge.color0,
        edge.color1,
        edge.color1,
        edge.color0,
        edge.color0,
        edge.color1,
        edge.color1
    };
    const unsigned numVertices = 8;
    Mesh::Vertex vertex;
    // vertex.color = ColorTo4ub(edge.color);
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
        vertex.color = bboxVertexColors[i];
        _edgeBBoxVertices[edgeIdx * numVertices + i] = vertex;
    }
}

CylinderEdges::~CylinderEdges() {
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

void CylinderEdges::Rebuild(const std::vector<Edge>& edges) {
	SYS::ScopedLock lock(_mtx);

    _edges.clear();
	for(unsigned i = 0; i < edges.size(); ++i)
		_edges.push_back(FatEdge(edges[i]));

    // RemoveDegeneratedEdges(_edges);
    if(_edges.empty()) return;

    for(unsigned i = 0; i < _edges.size(); ++i) {
        FatEdge& edge = _edges[i];
        M::Matrix4 R = AlignZ(edge.p1 - edge.p0);
        edge.Rt = M::Transpose(R);
    }

    _edgeBBoxIndices.clear();
    unsigned baseIdx = 0;
    for(unsigned i = 0; i < _edges.size(); ++i) {
        const unsigned numVertices = 8;
        Mesh::Index bboxIndices[] = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
        const unsigned numIndices = 14;
        for(unsigned i = 0; i < numIndices; ++i) _edgeBBoxIndices.push_back(baseIdx + bboxIndices[i]);
        _edgeBBoxIndices.push_back(Mesh::RESTART_INDEX);
        baseIdx += numVertices;
    }

    _edgeBBoxVertices.clear();
    const unsigned numVerticesPerEdge = 8;
    _edgeBBoxVertices.resize(numVerticesPerEdge * _edges.size());
    for(unsigned i = 0; i < _edges.size(); ++i)
        RebuildVertices(i, M::Mat4::Identity());

    // rebuild mesh
    _meshDesc.vertices = &_edgeBBoxVertices[0];
    _meshDesc.numVertices = _edgeBBoxVertices.size();
    _meshDesc.indices = &_edgeBBoxIndices[0];
    _meshDesc.numIndices = _edgeBBoxIndices.size();
    _meshDesc.primType = GL_TRIANGLE_STRIP;

    if(_mesh) DestroyMesh();

    if(!_edges.empty()) {
        M::Matrix4 lastTransform = M::Mat4::Identity();
        if(_tfmesh) lastTransform = meshMgr.GetMesh(_tfmesh).GetTransform();

        _mesh = meshMgr.Create(_meshDesc);
        _tfmesh = meshMgr.Create(_mesh);
        meshMgr.GetMesh(_tfmesh).SetTransform(lastTransform);
    }
}

void CylinderEdges::Update(const leda::nb::RatPolyMesh& mesh, const std::vector<M::Vector3>& fpos) {
    typedef leda::nb::RatPolyMesh::State state_t;
	SYS::ScopedLock lock(_mtx);
    for(unsigned i = 0; i < _edges.size(); ++i) {
        FatEdge& edge = _edges[i];
        int state = M::Max(mesh.state_of(edge.pe), M::Max(mesh.state_of(edge.v0), mesh.state_of(edge.v1)));
        COM_assert(state_t::TOPOLOGY_CHANGED > state);
        if(state_t::GEOMETRY_CHANGED == state) {
            edge.p0 = fpos[edge.v0->id()];
            edge.p1 = fpos[edge.v1->id()];
            edge.color0 = mesh.color_of(edge.pe);
            edge.color1 = mesh.color_of(mesh.reversal(edge.pe));
            M::Matrix4 R = AlignZ(edge.p1 - edge.p0);
            edge.Rt = M::Transpose(R);
            RebuildVertices(i, M::Mat4::Identity());
            if(_mesh) {
                const unsigned numVertices = 8;
                unsigned off = i * sizeof(Mesh::Vertex) * numVertices;
                unsigned size = sizeof(Mesh::Vertex) * numVertices;

                meshMgr.GetMesh(_mesh).Invalidate(&_edgeBBoxVertices[0], off, size);
            }
        }
    }
    // if(_mesh) meshMgr.GetMesh(_mesh).Invalidate(&_edgeBBoxVertices[0]);
}

void CylinderEdges::SetTransform(const M::Matrix4& transform, const M::Matrix4&) {
	SYS::ScopedLock lock(_mtx);
    if(_tfmesh) meshMgr.GetMesh(_tfmesh).SetTransform(transform);
}

void CylinderEdges::BuildRenderMesh() {
	SYS::ScopedLock lock(_mtx);

    // nothing to do here
}

void CylinderEdges::DestroyRenderMesh() {
    DestroyMesh();
}

MeshJob CylinderEdges::GetRenderJob() const {
	SYS::ScopedLock lock(_mtx);

	assert(!_edges.empty());

    MeshJob meshJob;
    meshJob.fx = "EdgeBillboard";
    meshJob.tfmesh = _tfmesh;
    meshJob.material = Material::White;
    meshJob.primType = 0;
    return meshJob;
}

} // namespace R