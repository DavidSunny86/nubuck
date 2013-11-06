#pragma once

#include <renderer\renderer.h>
#include <renderer\mesh\mesh.h>

namespace W {

struct ENT_Mesh {
    bool                    isVisible;
    R::MeshMgr::meshPtr_t   meshPtr;
    R::RenderList           renderList;
};

void Mesh_Init(ENT_Mesh& mesh, R::MeshMgr::meshPtr_t meshPtr);
void Mesh_SetVisible(bool isVisible);
void Mesh_BuildRenderList(ENT_Mesh& mesh, const std::string& fxName, const M::Matrix4& transform);

} // namespace W