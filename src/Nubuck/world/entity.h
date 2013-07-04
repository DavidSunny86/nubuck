#pragma once

#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\mesh.h>
#include <renderer\material\material.h>
#include <renderer\renderer.h>

#include <system\locks\spinlock.h>

namespace W {

    // register new entity types with W::world.RegisterEntity.
    // entity types that are known at starting time should be registered
    // in RunNubuck() (see nubuck.cpp)

    struct Event;

    class Entity {
    private:
        int         _id;
        bool        _isDead;
        M::Vector3  _position;

        R::Material _material;

        enum State {
            IDLE = 0,
            CHANGE_COLOR
        } _state;

        struct ChangeColorState {
            typedef float (*func_t)(float); // interpolation function

            func_t      func;
            R::Color    sourceColor;
            R::Color    targetColor;
            float       dur;
            float       t;
        } _changeColorState;

        void SetID(int id);
        void ChangeColor(ChangeColorState::func_t func, const R::Color& targetColor, float dur);
    protected:
        const M::Vector3& GetPosition(void) const;
        const R::Material& GetMaterial(void) const;

        void SetPosition(const M::Vector3& position);
        void InvalidateMesh(void);

        void PulseColor(const R::Color& targetColor, float dur);
        void LerpColor(const R::Color& targetColor, float dur);
    public:
        Entity(void);
        virtual ~Entity(void);

        int GetID(void) const { return _id; }
        bool IsDead(void) const { return _isDead; }

        void Destroy(void) { _isDead = true; }

        virtual void Update(float secsPassed);
        virtual void Render(std::vector<R::RenderJob>& renderList);
        void FreeResources(void);
        
        virtual void Spawn(const Event& event);
        virtual void HandleEvent(const Event& event) { }
    };

} // namespace W