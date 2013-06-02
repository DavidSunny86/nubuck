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

    void SkinMgr::Link(SkinData* skinData) {
        _dataLock.Lock();
        skinData->prev = NULL;
        if(skinData->next = _skins)
            skinData->next->prev = skinData;
        _skins = skinData;
        _dataLock.Unlock();
    }

    SkinMgr::SkinMgr(void) : _skins(NULL) { }

    SkinMgr::handle_t SkinMgr::Create(const SkinDesc& skin) {
        SkinData* skinData = new SkinData();
        skinData->refCount = 0;
        skinData->skinDesc = skin;
        skinData->compiled = false;

        Link(skinData);

        return handle_t(skinData);
    }

    void SkinMgr::R_Compile(handle_t& handle) {
        if(!handle.Res()->compiled) {
            SkinDesc& skinDesc = handle.Res()->skinDesc;
            Skin& skin = handle.Res()->skin;

            skin.SetTexture(Skin::TL_DIFFUSE, TextureManager::Instance().Get(skinDesc.diffuseTexture));

            handle.Res()->compiled = true;
        }
    }

    void SkinMgr::R_Bind(Program& program, handle_t& handle) {
        handle.Res()->skin.Bind(BF_TEX_DIFFUSE, program);
    }

} // namespace R