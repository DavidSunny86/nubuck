#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\sphere\sphere.h>
#include <world\events.h>
#include "ent_node.h"

COM::Config::Variable<float> cvar_nodeSize("nodeSize", 1.0f);
COM::Config::Variable<int> cvar_nodeSubdiv("nodeSubdiv", 2);

namespace {

    M::Vector3 ToVector(const leda::d3_rat_point& p) {
        const leda::d3_point fp = p.to_float();
        return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
    }

} // unnamed namespace

namespace W {

    void ENT_Node::ChangePosition(const M::Vector3& targetPosition, float dur) {
        _changePositionState.sourcePosition = GetPosition();
        _changePositionState.targetPosition = targetPosition;
        _changePositionState.dur            = dur;
        _changePositionState.t              = 0.0f;
        _changePositionState.isChanging     = true;
    }

    ENT_Node::ConfigObserver ENT_Node::s_configObs;
    R::meshPtr_t ENT_Node::s_mesh;
    R::Material ENT_Node::s_material;

    void ENT_Node::ConfigObserver::CVAR_Changed(const std::string&) {
        ENT_Node::CreateMesh();
    }

    void ENT_Node::CreateMesh(void) {
        R::Sphere sphere(cvar_nodeSubdiv, true);
        sphere.Scale(cvar_nodeSize);
        s_mesh = R::meshMgr.Create(sphere.GetDesc());
    }

    void ENT_Node::InitResources(void) {
        static bool init = false;
        if(!init) {
            cvar_nodeSize.Register(&s_configObs);
            cvar_nodeSubdiv.Register(&s_configObs);
            CreateMesh();
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

        if(EVENT_UPDATE == event.id) {
            M::Vector3 vec = ToVector((*_G)[_node]);
            ChangePosition(vec, 1.0f);
        }
    }

    void ENT_Node::Spawn(const Event& event) {
        Entity::Spawn(event);

        const SpawnArgs* spawnArgs = (const SpawnArgs*)event.args;

        _G = spawnArgs->G;
        _node = spawnArgs->node;

        SetPosition(ToVector((*spawnArgs->G)[_node]));

        InitResources();
        SetMaterial(s_material);
    }

    void ENT_Node::Update(float secsPassed) {
        Entity::Update(secsPassed);
        if(_changePositionState.isChanging) {
            float l = M::Min(1.0f, _changePositionState.t / _changePositionState.dur);
            SetPosition((1.0f - l) * _changePositionState.sourcePosition + l * _changePositionState.targetPosition);
            _changePositionState.t += secsPassed;
            if(_changePositionState.t >= _changePositionState.dur)
                _changePositionState.isChanging = false;
        }
    }

    void ENT_Node::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        renderJob.fx = "Lit";
        renderJob.mesh = s_mesh;
        renderJob.primType  = 0;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = GetMaterial();
        renderList.push_back(renderJob);
    }

} // namespace W