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

void MeshMgr::R_Update(void) {
    MeshLink *next, *it = _meshes;
    while(it) {
        next = it->next;
        if(it->destroy) {
            if(it->next) it->next->prev = it->prev;
            if(it->prev) it->prev->next = it->next;
            if(_meshes == it) _meshes = it->next;
            assert(it->mesh);
            delete it->mesh;
            delete it;
        }
        it = next;
    }
}

} // namespace R
