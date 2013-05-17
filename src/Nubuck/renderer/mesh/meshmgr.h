#pragma once

#include <generic\singleton.h>
#include <system\locks\spinlock.h>
#include "mesh.h"

namespace R {

    class MeshMgr : public GEN::Singleton<MeshMgr> {
        friend class GEN::Singleton<MeshMgr>;
    private:
        struct Node {
            Node *prev, *next;
            int refCnt;
            const char* name;
            Mesh mesh;

            Node(const MeshDesc& desc);
        };

        Node* _meshes;
        SYS::SpinLock _meshesLck;

        Node* _CreateMesh(const MeshDesc& desc);
        Node* _FindMesh(const char* name);

        MeshMgr(void);
    public:
        typedef Node* meshHandle_t;

        ~MeshMgr(void);

        void RegisterMesh(const MeshDesc& desc, const char* name);
        meshHandle_t FindMesh(const char* name);
        meshHandle_t CreateMesh(const MeshDesc& desc);

        Mesh& GetMesh(meshHandle_t meshHandle);
        void Release(meshHandle_t meshHandle);

        void Update(void);
        void Clear(void);
    };

} // namespace R
