#include <common\common.h>
#include "events.h"
#include "entity.h"

static float IP_Pulse(float l) { return sinf(M::PI * l); }
static float IP_Ident(float l) { return l; }

namespace W {

    void Entity::SetPosition(const M::Vector3& position) {
        _position = position;
    }

    void Entity::SetMeshDesc(const R::MeshDesc& meshDesc) {
        _mesh = R::meshPtr_t(new R::Mesh(meshDesc));
    }

    void Entity::InvalidateMesh(void) {
        _mesh->Invalidate();
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
        _effect = R::EffectManager::Instance().GetEffect("Lit");
        _material.diffuseColor = R::Color::White;
    }

    R::Color Lerp(const R::Color& source, const R::Color& target, float l) {
        return R::Color(
            (1.0f - l) * source.r + l * target.r,
            (1.0f - l) * source.g + l * target.g,
            (1.0f - l) * source.b + l * target.b,
            (1.0f - l) * source.a + l * target.a);
    }

    void Entity::Update(float secsPassed) {
        if(CHANGE_COLOR == _state) {
            const float l = M::Min(1.0f, _changeColorState.t / _changeColorState.dur);
            _material.diffuseColor = Lerp(_changeColorState.sourceColor, _changeColorState.targetColor, _changeColorState.func(l));
            _changeColorState.t += secsPassed;

            if(_changeColorState.t >= _changeColorState.dur)
                _state = IDLE;
        }
    }

    void Entity::FreeResources(void) {
        _mesh   = R::meshPtr_t();
        _effect = GEN::Pointer<R::Effect>();
    }

    void Entity::Spawn(const Event& event) {
        SetID(event.entityId);
    }

    R::RenderJob Entity::GetRenderJob(void) {
        R::RenderJob renderJob;
        renderJob.fx        = _effect.Raw();
        renderJob.mesh      = _mesh;
        renderJob.material  = &_material;
        renderJob.transform = M::Mat4::Translate(_position);
        return renderJob;
    }

} // namespace W