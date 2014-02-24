#pragma once

#include <Nubuck\generic\pointer.h>
#include <Nubuck\system\locks\spinlock.h>
#include "meshmgr_fwd.h"
#include "mesh.h"
#include "tfmesh.h"

namespace R {

namespace MeshMgr_Impl {

template<typename TMESH> // TYPE in { Mesh, TFMesh }
struct MeshLink {
    MeshLink *prev, *next;
    TMESH* mesh;
    bool  destroy;
    MeshLink(TMESH* const mesh) : prev(NULL), next(NULL), mesh(mesh), destroy(false) { }
};

} // namespace MeshMgr_Impl

class MeshMgr {
private:
    MeshMgr_Impl::MeshLink<Mesh>*       _meshes;
    MeshMgr_Impl::MeshLink<TFMesh>*     _tfmeshes;
    SYS::SpinLock                       _meshesMtx;

    template<typename TMESH> void Link(MeshMgr_Impl::MeshLink<TMESH>** head, MeshMgr_Impl::MeshLink<TMESH>* meshPtr);
    template<typename TMESH> void R_Update(MeshMgr_Impl::MeshLink<TMESH>** head);
public:
    MeshMgr(void) : _meshes(NULL), _tfmeshes(NULL) { }

    meshPtr_t   Create(const Mesh::Desc& desc); // deep copy
    tfmeshPtr_t Create(meshPtr_t meshPtr);

    template<typename TMESH> void           Destroy(MeshMgr_Impl::MeshLink<TMESH>* meshPtr);
    template<typename TMESH> const TMESH&   GetMesh(MeshMgr_Impl::MeshLink<TMESH>* meshPtr) const;
    template<typename TMESH> TMESH&         GetMesh(MeshMgr_Impl::MeshLink<TMESH>* meshPtr);

    // methods prefixed with R_ should only be called by the renderer
    void R_Update(void);
};

extern MeshMgr meshMgr;

} // namespace R

#include "meshmgr_inl.h"