#pragma once

#include <generic\pointer.h>
#include <system\locks\spinlock.h>
#include "mesh.h"

namespace R {

    class MeshMgr {
    private:
        Mesh* _meshes;

        SYS::SpinLock _mtx; // locks mesh list
    public:
        class MeshPtr {
        private:
            Mesh* _mesh;

            void IncRef(void);
            void DecRef(void);
        public:
            MeshPtr(void) : _mesh(NULL) { }
            MeshPtr(Mesh* const mesh) : _mesh(mesh) { IncRef(); }
            MeshPtr(const MeshPtr& other) : _mesh(other._mesh) { IncRef(); }
            ~MeshPtr(void) { DecRef(); }

            MeshPtr& operator=(const MeshPtr& other) {
                if(&other != this) {
                    DecRef();
                    _mesh = other._mesh;
                    IncRef();
                }
                return *this;
            }

            bool IsValid(void) const { return _mesh; }

            Mesh*       operator->(void) { return _mesh; }
            const Mesh* operator->(void) const { return _mesh; }

        }; // class MeshPtr

        MeshMgr(void) : _meshes(NULL) { }

        MeshPtr Create(const Mesh::Desc& desc); // deep copy

        void R_Update(void);
    };

    typedef MeshMgr::MeshPtr meshPtr_t;

    extern MeshMgr meshMgr;

} // namespace R

#include "meshmgr_inl.h"