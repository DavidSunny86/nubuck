#include <string.h>

#include "meshmgr.h"

namespace R {

    MeshMgr::Node* MeshMgr::_CreateMesh(const GEN::Pointer<MeshDesc>& desc) {
        Node* node = new Node(desc);
        node->prev = NULL;
        node->next = _meshes;
        if(node->next) node->next->prev = node;
        _meshes = node;
        return node;
    }

    MeshMgr::Node* MeshMgr::_FindMesh(const char* name) {
        Node* node = _meshes;
        while(node && (!node->name || strcmp(name, node->name))) node = node->next;
        return node;
    }

    MeshMgr::Node::Node(const GEN::Pointer<MeshDesc>& desc) 
        : prev(NULL), next(NULL), refCnt(0), name(NULL), mesh(desc) { }

    MeshMgr::MeshMgr(void) : _meshes(NULL) { }

    MeshMgr::~MeshMgr(void) { Clear(); }

    void MeshMgr::RegisterMesh(const GEN::Pointer<MeshDesc>& desc, const char* name) {
        _meshesLck.Lock();
        if(!_FindMesh(name)) {
            Node* node = _CreateMesh(desc);
            node->refCnt = 1;
            node->name = name;
        }
        _meshesLck.Unlock();
    }

    MeshMgr::meshHandle_t MeshMgr::CreateMesh(const GEN::Pointer<MeshDesc>& desc) {
        _meshesLck.Lock();
        Node* node = _CreateMesh(desc);
        node->refCnt = 1;
        _meshesLck.Unlock();
        return node;
    }

    MeshMgr::meshHandle_t MeshMgr::FindMesh(const char* name) {
        Node* node = NULL;
        _meshesLck.Lock();
        if(node = _FindMesh(name)) node->refCnt++;
        _meshesLck.Unlock();
        return node;
    }

    Mesh& MeshMgr::GetMesh(meshHandle_t mesh) {
        return mesh->mesh;
    }

    void MeshMgr::Release(meshHandle_t meshHandle) {
        _meshesLck.Lock();
        if(meshHandle) meshHandle->refCnt--;
        _meshesLck.Unlock();
    }

    void MeshMgr::Update(void) {
        _meshesLck.Lock();
        Node* node = _meshes;
        while(node) {
            Node* next = node->next;
            if(!node->name && !node->refCnt) {
                if(_meshes == node) _meshes = next;
                if(node->next) node->next->prev = node->prev;
                if(node->prev) node->prev->next = node->next;
                delete node;
            }
            node = next;
        }
        _meshesLck.Unlock();
    }

    void MeshMgr::Clear(void) {
        _meshesLck.Lock();
        Node* node = _meshes;
        while(node) {
            Node* next = node->next;
            delete node;
            node = next;
        }
        _meshes = NULL;
        _meshesLck.Unlock();
    }

} // namespace R
