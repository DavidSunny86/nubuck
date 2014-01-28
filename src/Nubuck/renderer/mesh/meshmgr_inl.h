#pragma once

#include <system\locks\scoped_lock.h>
#include "meshmgr.h"

namespace R {

template<typename TMESH> 
void MeshMgr::Link(MeshMgr_Impl::MeshLink<TMESH>** head, MeshMgr_Impl::MeshLink<TMESH>* link) {
    SYS::ScopedLock lock(_meshesMtx);
    link->next = *head;
    if(*head) (*head)->prev = link;
    *head = link;
}

template<typename TMESH>
void MeshMgr::R_Update(MeshMgr_Impl::MeshLink<TMESH>** head) {
    SYS::ScopedLock lock(_meshesMtx);
    MeshMgr_Impl::MeshLink<TMESH> *next, *it = *head;
    while(it) {
        next = it->next;
        if(it->destroy) {
            if(it->next) it->next->prev = it->prev;
            if(it->prev) it->prev->next = it->next;
            if(*head == it) *head = it->next;
            assert(it->mesh);
            delete it->mesh;
            delete it;
        }
        it = next;
    }
}

template<typename TMESH> 
inline void MeshMgr::Destroy(MeshMgr_Impl::MeshLink<TMESH>* meshPtr) { 
    meshPtr->destroy = true; 
}

template<typename TMESH> 
inline const TMESH& MeshMgr::GetMesh(MeshMgr_Impl::MeshLink<TMESH>* meshPtr) const { 
    return *meshPtr->mesh; 
}

template<typename TMESH> 
inline TMESH& MeshMgr::GetMesh(MeshMgr_Impl::MeshLink<TMESH>* meshPtr) { 
    return *meshPtr->mesh; 
}

} // namespace R