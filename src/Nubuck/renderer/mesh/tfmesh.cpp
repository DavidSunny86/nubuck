#include <renderer\mesh\meshmgr.h>
#include "tfmesh.h"

namespace R {

TFMesh::TFMesh(const meshPtr_t meshPtr) : 
_transform(M::Mat4::Identity()), 
_meshPtr(meshPtr),
_gbHandle(GB_INVALID_HANDLE)
{ }

TFMesh::~TFMesh() {
    if(GB_INVALID_HANDLE != _gbHandle) GB_FreeMemItem(_gbHandle);
}

const Mesh& TFMesh::GetMesh() const { return meshMgr.GetMesh(_meshPtr); }

Mesh& TFMesh::GetMesh() { return meshMgr.GetMesh(_meshPtr); }

void TFMesh::SetTransform(const M::Matrix4& transform) {
    _transform = transform;
}

void TFMesh::TransformVertices() {
    const Mesh& mesh = meshMgr.GetMesh(_meshPtr);
    _tfverts = mesh.GetVertices();
    for(unsigned i = 0; i < _tfverts.size(); ++i)
        _tfverts[i].position = M::Transform(_transform, _tfverts[i].position);
}

void TFMesh::R_TF_Touch() {
    if(GB_INVALID_HANDLE == _gbHandle) {
        _gbHandle = GB_AllocMemItem(&_tfverts[0], _tfverts.size());
    }
    GB_Invalidate(_gbHandle);
    GB_Touch(_gbHandle);
}

unsigned TFMesh::R_TF_IndexOff() const {
    return GB_GetOffset(_gbHandle) / sizeof(Mesh::Vertex);
}

void TFMesh::R_Touch() { meshMgr.GetMesh(_meshPtr).R_AllocBuffer(); }

unsigned TFMesh::R_IndexOff() const {
    return meshMgr.GetMesh(_meshPtr).R_IndexOff();
}

} // namespace R