#include "ent_mesh.h"

namespace W {

void Mesh_Init(ENT_Mesh& mesh, R::MeshMgr::meshPtr_t meshPtr) {
    mesh.isVisible  = true;
    mesh.meshPtr    = meshPtr;
}

void Mesh_SetVisible(ENT_Mesh& mesh, bool isVisible) {
    mesh.isVisible = isVisible;
}

void Mesh_BuildRenderList(ENT_Mesh& mesh, const std::string& fxName, const M::Matrix4& transform) {
    mesh.renderList.meshJobs.clear();
    if(mesh.isVisible) {
        R::MeshJob rjob;
        // rjob.fx         = "LitDirectionalTransparent";
        rjob.fx         = fxName;
        rjob.material   = R::Material::White;
        rjob.mesh       = mesh.meshPtr;
        rjob.primType   = 0;
        rjob.transform  = transform;
        mesh.renderList.meshJobs.push_back(rjob);
    }
}

} // namespace W