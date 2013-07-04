#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\sphere\sphere.h>
#include <world\events.h>
#include "ent_node.h"

namespace {

    M::Vector3 ToVector(const leda::d3_rat_point& p) {
        const leda::d3_point fp = p.to_float();
        return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
    }

} // unnamed namespace

namespace W {

    R::meshPtr_t ENT_Node::s_mesh;
    R::Material ENT_Node::s_material;

    void ENT_Node::InitResources(void) {
        static bool init = false;
        if(!init) {
            R::Sphere sphere(2, true);
            s_mesh = R::meshMgr.Create(sphere.GetDesc());
            s_material.diffuseColor = R::Color(0.2f, 0.2f, 0.2f);
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
    }

    void ENT_Node::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        renderJob.fx = "Lit";
        renderJob.mesh = s_mesh;
        renderJob.primType  = 0;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = s_material;
        renderList.push_back(renderJob);
    }

} // namespace W