#include <common\common.h>
#include "meshmgr.h"

namespace R {

MeshMgr meshMgr;

MeshMgr::meshPtr_t MeshMgr::Create(const Mesh::Desc& desc) {
    Mesh* mesh = new Mesh(desc);
    MeshLink* link = new MeshLink(mesh);
    _meshesMtx.Lock();
    link->next = _meshes;
    if(_meshes) _meshes->prev = link;
    _meshes = link;
    _meshesMtx.Unlock();
    return link;
}

} // namespace R
