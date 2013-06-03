#pragma once

#include <renderer\mesh\staticbuffer.h>
#include "meshmgr.h"

namespace R {

    // ==================================================
    // MeshMgr::Handle Impl
    // ==================================================

    template<typename TYPE>
    MeshMgr::MeshData<TYPE>* MeshMgr::Handle<TYPE>::Res(void) { return _res; }

    template<typename TYPE>
    MeshMgr::Handle<TYPE>::Handle(MeshMgr::MeshData<TYPE>* const res) : _res(res) { _res->IncRef(); }

    template<typename TYPE>
    MeshMgr::Handle<TYPE>::Handle(void) : _res(NULL) { }

    template<typename TYPE>
    MeshMgr::Handle<TYPE>::Handle(const Handle& other) : _res(other._res) { if(_res) _res->IncRef(); }

    template<typename TYPE>
    MeshMgr::Handle<TYPE>::~Handle(void) { if(_res) _res->DecRef(); }

    template<typename TYPE>
    MeshMgr::Handle<TYPE>& MeshMgr::Handle<TYPE>::operator=(const Handle& other) {
        if(&other != this) {
            if(_res) _res->DecRef();
            if(_res = other._res)
                _res->IncRef();
        }
        return *this;
    }

    template<typename TYPE>
    bool MeshMgr::Handle<TYPE>::IsValid(void) const {
        return NULL != _res;
    }

    // ==================================================
    // MeshMgr::MeshData Impl
    // ==================================================

    template<typename TYPE>
    void MeshMgr::MeshData<TYPE>::IncRef(void) {
        refCountLock.Lock();
        refCount++;
        refCountLock.Unlock();
    }

    template<typename TYPE>
    void MeshMgr::MeshData<TYPE>::DecRef(void) {
        refCountLock.Lock();
        refCount--;
        refCountLock.Unlock();
    }

    // ==================================================
    // MeshMgr Impl
    // ==================================================

    template<typename TYPE>
    MeshMgr::Handle<TYPE> MeshMgr::Create(const TYPE* const data, int num) {
        TYPE* copy = new TYPE[num];
        for(int i = 0; i < num; ++i) copy[i] = data[i];

        MeshData<TYPE>* meshData = new MeshData<TYPE>;
        meshData->refCount = 0;
        meshData->data = copy;
        meshData->num = num;
        meshData->compiled = false;

        Link(meshData);

        return Handle<TYPE>(meshData);
    }

    template<typename TYPE> struct BufferType;
    template<> struct BufferType<Vertex> { enum { VALUE = GL_ARRAY_BUFFER }; };
    template<> struct BufferType<Index> { enum { VALUE = GL_ELEMENT_ARRAY_BUFFER }; };

    template<typename TYPE>
    void MeshMgr::R_Compile(Handle<TYPE>& handle) {
        MeshData<TYPE>* meshData = handle.Res();
        if(!meshData->compiled) {
            meshData->buffer = GEN::Pointer<StaticBuffer>(new StaticBuffer(
                BufferType<TYPE>::VALUE, meshData->data, meshData->num * sizeof(TYPE)));
            meshData->compiled = true;
        }
    }

    template<typename TYPE>
    void MeshMgr::R_Bind(Handle<TYPE>& handle) {
        handle.Res()->buffer->Bind();
    }

    template<typename TYPE>
    int MeshMgr::R_Size(Handle<TYPE>& handle) {
        return handle.Res()->num;
    }

} // namespace R