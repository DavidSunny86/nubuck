#pragma once

#include "meshmgr.h"

namespace R {

    // MeshMgr::MeshPtr Impl

    inline void MeshMgr::MeshPtr::IncRef(void) {
        if(_mesh) {
            _mesh->mgrLink.mtx.Lock();
            _mesh->mgrLink.refCount++;
            _mesh->mgrLink.mtx.Unlock();
        }
    }

    inline void MeshMgr::MeshPtr::DecRef(void) {
        if(_mesh) {
            _mesh->mgrLink.mtx.Lock();
            _mesh->mgrLink.refCount--;
            _mesh->mgrLink.mtx.Unlock();
        }
    }

} // namespace R