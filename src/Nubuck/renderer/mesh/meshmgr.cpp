#include <Nubuck\common\common.h>
#include "meshmgr.h"

namespace R {

MeshMgr meshMgr;

meshPtr_t MeshMgr::Create(const Mesh::Desc& desc) {
    Mesh* mesh = new Mesh(desc);
    MeshMgr_Impl::MeshLink<Mesh>* link = new MeshMgr_Impl::MeshLink<Mesh>(mesh);
    Link(&_meshes, link);
    return link;
}

tfmeshPtr_t MeshMgr::Create(meshPtr_t meshPtr) {
    TFMesh* tfmesh = new TFMesh(meshPtr);
    MeshMgr_Impl::MeshLink<TFMesh>* link = new MeshMgr_Impl::MeshLink<TFMesh>(tfmesh);
    Link(&_tfmeshes, link);
    return link;
}

void MeshMgr::R_Update() {
    R_Update(&_meshes);
    R_Update(&_tfmeshes);
}

} // namespace R
