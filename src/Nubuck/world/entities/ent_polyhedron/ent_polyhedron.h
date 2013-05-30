#pragma once

#include <common\types.h>
#include <world\entity.h>
#include <renderer\renderer.h>
#include <renderer\mesh\polyhedron\polyhedron.h>

namespace W {

    class ENT_Polyhedron : public Entity {
    private:
        struct ColorState {
            typedef float (*func_t)(float); // interpolation function

            bool        changing;
            func_t      func;
            R::Color    sourceColor;
            R::Color    targetColor;
            float       dur;
            float       t;

            leda::edge  rep;
        };

        GEN::Pointer<R::PolyhedronMesh> _polyDesc;
        std::vector<ColorState> _faceColorStates;
        int _numAnimFaces;

        const graph_t* _G;

        std::vector<Event> _evBuffer;

        Mesh _solidMesh;
        Mesh _wireMesh;

        void Rebuild(void);
        void ChangeFaceColor(ColorState::func_t func, leda::edge edge, const R::Color& targetColor, float dur);
    public:
        struct SpawnArgs {
            const graph_t* G;
        };

        ENT_Polyhedron(void);

        void Update(float secsPassed) override;
        void Render(std::vector<R::RenderJob>& renderList) override;

        void Spawn(const Event& event);

        void HandleEvent(const Event& event) override;
    };

} // namespace W