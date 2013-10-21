#pragma once

#include <generic\pointer.h>
#include <system\locks\spinlock.h>
#include "mesh.h"

namespace R {

class MeshMgr {
private:
    struct MeshLink {
        MeshLink *prev, *next;
        Mesh* mesh;
        MeshLink(Mesh* const mesh) : prev(NULL), next(NULL), mesh(mesh) { }
    };
    MeshLink*       _meshes;
    SYS::SpinLock   _meshesMtx;
public:
    typedef MeshLink* meshPtr_t;

    MeshMgr(void) : _meshes(NULL) { }

    meshPtr_t Create(const Mesh::Desc& desc); // deep copy

    const Mesh& GetMesh(meshPtr_t meshPtr) const { return *meshPtr->mesh; }
    Mesh& GetMesh(meshPtr_t meshPtr) { return *meshPtr->mesh; }

    // methods prefixed with R_ should only be called by the renderer
    void R_Update(void) { }
};

extern MeshMgr meshMgr;

} // namespace R

#include "meshmgr_inl.h"