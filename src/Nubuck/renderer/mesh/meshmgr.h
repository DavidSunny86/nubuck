#pragma once

#include <generic\pointer.h>
#include <system\locks\spinlock.h>
#include "mesh.h"

namespace R {

    class StaticBuffer;

    class MeshMgr {
    private:
        template<typename TYPE> // TYPE in {Vertex, Index}
        struct MeshData {
            MeshData *prev, *next;

            int             refCount;
            SYS::SpinLock   refCountLock;

            TYPE*       data;
            int         num;

            GEN::Pointer<StaticBuffer> buffer;

            void IncRef(void);
            void DecRef(void);
        };

        typedef MeshData<Vertex>    vrtData_t;
        typedef MeshData<Index>     idxData_t;

        // synchronizes access to both vertex and index data
        SYS::SpinLock   _dataLock;

        vrtData_t*  _vertices;
        idxData_t*  _indices;

        void Link(vrtData_t* vrtData);
        void Link(idxData_t* idxData);

        void FreeUnused(vrtData_t* vrtData);
        void FreeUnused(idxData_t* idxData);
    public:
        template<typename TYPE>
        class Handle {
            friend class MeshMgr;
        private:
            MeshData<TYPE>* _res;
            MeshData<TYPE>* Res(void);
            Handle(MeshData<TYPE>* const res);
        public:
            Handle(void);
            ~Handle(void);
            Handle(const Handle& other);
            Handle& operator=(const Handle& other);
            bool IsValid(void) const;
        };

        typedef Handle<Vertex> vertexHandle_t;
        typedef Handle<Index> indexHandle_t;

        MeshMgr(void);

        template<typename TYPE> // TYPE in {Vertex, Index}
        Handle<TYPE> Create(const TYPE* const data, int num);

        template<typename TYPE> // TYPE in {Vertex, Index}
        void R_Compile(Handle<TYPE>& handle);

        template<typename TYPE> // TYPE in {Vertex, Index}
        void R_Bind(Handle<TYPE>& handle);

        template<typename TYPE> // TYPE in {Vertex, Index}
        int R_Size(Handle<TYPE>& handle);

        void R_FrameUpdate(void);
    };

    extern MeshMgr meshMgr;

} // namespace R

#include "meshmgr_inl.h"