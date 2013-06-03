#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\quad\quad.h>
#include <world\events.h>
#include "ent_node.h"

namespace {

    M::Vector3 ToVector(const leda::d3_rat_point& p) {
        const leda::d3_point fp = p.to_float();
        return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
    }

} // unnamed namespace

namespace W {

    R::MeshMgr::vertexHandle_t ENT_Node::s_meshVertices;
    R::MeshMgr::indexHandle_t ENT_Node::s_meshIndices;

    void ENT_Node::InitResources(void) {
        static bool init = false;
        if(!init) {
            R::MeshDesc desc = R::CreateQuadDesc();
            s_meshVertices = R::meshMgr.Create(desc.vertices, desc.numVertices);
            s_meshIndices = R::meshMgr.Create(desc.indices, desc.numIndices);
        }
        init = true;
    }

    void ENT_Node::HandleEvent(const Event& event) {
        if(EVENT_CHANGE_COLOR == event.id) {
            ChangeColorArgs* args = (ChangeColorArgs*)event.args;
            R::Color color(args->r, args->g, args->b);

            if(ChangeColorArgs::MODE_PULSE == args->mode) PulseColor(color, 1.0f);
            if(ChangeColorArgs::MODE_LERP == args->mode) LerpColor(color, 1.0f);
        }
    }

    void ENT_Node::Spawn(const Event& event) {
        Entity::Spawn(event);

        const SpawnArgs* spawnArgs = (const SpawnArgs*)event.args;

        _node = spawnArgs->node;

        SetPosition(ToVector((*spawnArgs->G)[_node]));

        InitResources();
        
        Mesh mesh;
        mesh.vertices   = s_meshVertices;
        mesh.indices    = s_meshIndices;
        mesh.primType   = GL_TRIANGLES;
        _mesh = mesh;
    }

    void ENT_Node::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        renderJob.fx = "Lit";
        renderJob.vertices  = _mesh.vertices;
        renderJob.indices   = _mesh.indices;
        renderJob.primType  = _mesh.primType;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = GetMaterial();
        renderList.push_back(renderJob);
    }

} // namespace W