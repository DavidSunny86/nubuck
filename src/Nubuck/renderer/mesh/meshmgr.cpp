#include <common\common.h>
#include "meshmgr.h"

namespace R {

    MeshMgr meshMgr;

    MeshMgr::MeshPtr MeshMgr::Create(const Mesh::Desc& desc) {
        Mesh* mesh = new Mesh(desc);
        Mesh::MgrLink& link = mesh->mgrLink;
        link.refCount = 0;

        _mtx.Lock();
        link.next = _meshes;
        link.prev = NULL;
        if(link.next) link.next->mgrLink.prev = mesh;
        _meshes = mesh;
        _mtx.Unlock();

        return MeshPtr(mesh);
    }

    void MeshMgr::R_Update(void) {
        _mtx.Lock();
        Mesh *next, *it = _meshes;
        while(it) {
            Mesh::MgrLink& link = it->mgrLink;
            next = link.next;
            if(0 >= link.refCount) {
                if(link.next) link.next->mgrLink.prev = link.prev;
                if(link.prev) link.prev->mgrLink.next = link.next;
                if(_meshes == it) _meshes = next;
                delete it;
            }
            it = next;
        }
        _mtx.Unlock();
    }

} // namespace R
