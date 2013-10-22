#include "ent_mesh.h"

namespace W {

void Mesh_Init(ENT_Mesh& mesh, R::MeshMgr::meshPtr_t meshPtr) {
    mesh.isVisible  = true;
    mesh.meshPtr    = meshPtr;
}

void Mesh_SetVisible(ENT_Mesh& mesh, bool isVisible) {
    mesh.isVisible = isVisible;
}

void Mesh_BuildRenderList(ENT_Mesh& mesh) {
    mesh.renderList.jobs.clear();
    if(mesh.isVisible) {
        R::RenderJob rjob;
        rjob.fx         = "LitDirectional";
        rjob.material   = R::Material::White;
        rjob.mesh       = mesh.meshPtr;
        rjob.primType   = 0;
        rjob.transform  = M::Mat4::Identity();
        mesh.renderList.jobs.push_back(rjob);
    }
}

} // namespace W