#include <common\common.h>
#include "events.h"
#include "entity.h"

static float IP_Pulse(float l) { return sinf(M::PI * l); }
static float IP_Ident(float l) { return l; }

namespace W {

    void Entity::SetPosition(const M::Vector3& position) {
        _position = position;
    }

    void Entity::SetMeshHandle(R::MeshMgr::meshHandle_t meshHandle) {
        R::MeshMgr::Instance().Release(_meshHandle);
        _meshHandle = meshHandle;
    }

    void Entity::InvalidateMesh(void) {
        R::MeshMgr::Instance().GetMesh(_meshHandle).Invalidate();
    }

    void Entity::SetID(int id) { _id = id; }

    void Entity::ChangeColor(ChangeColorState::func_t func, const R::Color& targetColor, float dur) {
        _changeColorState.func          = func;
        _changeColorState.sourceColor   = _material.diffuseColor;
        _changeColorState.targetColor   = targetColor;
        _changeColorState.dur           = dur;
        _changeColorState.t             = 0.0f;

        _state = CHANGE_COLOR;
    }

    void Entity::PulseColor(const R::Color& targetColor, float dur) {
        ChangeColor(IP_Pulse, targetColor, dur);
    }

    void Entity::LerpColor(const R::Color& targetColor, float dur) {
        ChangeColor(IP_Ident, targetColor, dur);
    }

    Entity::Entity(void) : _id(0), _position(M::Vector3::Zero) {
        _state = IDLE;
        _meshHandle = NULL;
        _effect = R::EffectManager::Instance().GetEffect("Lit");
        _material.diffuseColor = R::Color::White;
    }

    Entity::~Entity(void) {
        R::MeshMgr::Instance().Release(_meshHandle);
    }

    void Entity::Update(float secsPassed) {
        if(CHANGE_COLOR == _state) {
            const float l = M::Min(1.0f, _changeColorState.t / _changeColorState.dur);
            _material.diffuseColor = R::Lerp(_changeColorState.sourceColor, _changeColorState.targetColor, _changeColorState.func(l));
            _changeColorState.t += secsPassed;

            if(_changeColorState.t >= _changeColorState.dur)
                _state = IDLE;
        }
    }

    void Entity::FreeResources(void) {
        _effect = GEN::Pointer<R::Effect>();
    }

    void Entity::Spawn(const Event& event) {
        SetID(event.entityId);
    }

    R::RenderJob Entity::GetRenderJob(void) {
        R::RenderJob renderJob;
        renderJob.fx        = _effect.Raw();
        renderJob.mesh      = _meshHandle;
        renderJob.material  = &_material;
        renderJob.transform = M::Mat4::Translate(_position);
        return renderJob;
    }

} // namespace W