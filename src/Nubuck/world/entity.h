#pragma once

#include <renderer\mesh\meshmgr.h>
#include <renderer\mesh\mesh.h>
#include <renderer\material\material.h>
#include <renderer\renderer.h>

#include <system\locks\spinlock.h>

namespace W {

    struct Event;

    class Entity {
    public:
        struct Mesh {
            R::MeshMgr::vertexHandle_t  vertices;
            R::MeshMgr::indexHandle_t   indices;
            GLenum                      primType;
        };
    private:
        int         _id;
        M::Vector3  _position;
 
        Mesh        _mesh;
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
        void SetPosition(const M::Vector3& position);
        void SetMesh(const Mesh& mesh);
        void InvalidateMesh(void);

        void PulseColor(const R::Color& targetColor, float dur);
        void LerpColor(const R::Color& targetColor, float dur);
    public:
        Entity(void);
        virtual ~Entity(void);

        int GetID(void) const { return _id; }

        virtual void Update(float secsPassed);
        void FreeResources(void);
        
        virtual void Spawn(const Event& event);
        virtual void HandleEvent(const Event& event) { }
        virtual R::RenderJob GetRenderJob(void);
    };

} // namespace W