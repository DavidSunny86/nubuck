#include "skinmgr.h"

namespace R {

    SkinMgr skinMgr;

    // ==================================================
    // SkinMgr::Handle Impl
    // ==================================================

    SkinMgr::SkinData* SkinMgr::Handle::Res(void) { return _res; }

    SkinMgr::Handle::Handle(SkinMgr::SkinData* const res) : _res(res) { _res->IncRef(); }

    SkinMgr::Handle::Handle(void) : _res(NULL) { }

    SkinMgr::Handle::Handle(const Handle& other) : _res(other._res) { if(_res) _res->IncRef(); }

    SkinMgr::Handle::~Handle(void) { if(_res) _res->DecRef(); }

    SkinMgr::Handle& SkinMgr::Handle::operator=(const Handle& other) {
        if(&other != this) {
            if(_res) _res->DecRef();
            if(_res = other._res)
                _res->IncRef();
        }
        return *this;
    }

    bool SkinMgr::Handle::IsValid(void) const {
        return NULL != _res;
    }

    // ==================================================
    // SkinMgr::SkinData Impl
    // ==================================================

    void SkinMgr::SkinData::IncRef(void) {
        refCountLock.Lock();
        refCount++;
        refCountLock.Unlock();
    }

    void SkinMgr::SkinData::DecRef(void) {
        refCountLock.Lock();
        refCount--;
        refCountLock.Unlock();
    }

    // ==================================================
    // SkinMgr Impl
    // ==================================================

    SkinMgr::SkinMgr(void) : _skins(NULL) { }

    SkinMgr::handle_t SkinMgr::Create(const Skin& skin) {
        return handle_t();
    }


} // namespace R