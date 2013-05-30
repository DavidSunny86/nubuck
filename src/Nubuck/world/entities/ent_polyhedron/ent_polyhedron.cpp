#include <common\common.h>
#include <world\events.h>
#include <renderer\mesh\quad\quad.h>
#include "ent_polyhedron.h"

static float IP_Pulse(float l) { return sinf(M::PI * l); }
static float IP_Ident(float l) { return l; }

namespace W {

    void ENT_Polyhedron::Rebuild(void) {
        _polyDesc = GEN::Pointer<R::PolyhedronMesh>(new R::PolyhedronMesh(*_G));
        
        _solidMesh = MeshFromDesc(_polyDesc->GetSolidDesc());
        _wireMesh = MeshFromDesc(_polyDesc->GetWireframeDesc());

        _faceColorStates.resize(_polyDesc->NumFaces());
        for(unsigned i = 0; i < _polyDesc->NumFaces(); ++i) {
            _faceColorStates[i].changing = false;
        }
    }

    void ENT_Polyhedron::ChangeFaceColor(ColorState::func_t func, leda::edge edge, const R::Color& targetColor, float dur) {
        ColorState& state = _faceColorStates[_polyDesc->FaceOf(edge)];
        state.changing      = true;
        state.func          = func;
        state.sourceColor   = R::Color::White;
        state.targetColor   = targetColor;
        state.dur           = dur;
        state.t             = 0.0f;
        state.rep           = edge;
    }

    ENT_Polyhedron::ENT_Polyhedron(void) : _numAnimFaces(0) { }

    void ENT_Polyhedron::Update(float secsPassed) {
        Entity::Update(secsPassed);

        while(!_evBuffer.empty()) {
            Event event = _evBuffer.back();
            _evBuffer.pop_back();

            if(EVENT_CHANGE_COLOR == event.id) {
                ChangeColorArgs* args = (ChangeColorArgs*)event.args;
                R::Color targetColor(args->r, args->g, args->b);
                ColorState::func_t func = NULL;
                if(ChangeColorArgs::MODE_PULSE == args->mode) func = IP_Pulse;
                if(ChangeColorArgs::MODE_LERP == args->mode) func = IP_Ident;
                ChangeFaceColor(func, args->edge, targetColor, 1.0f);
                _numAnimFaces++;
            }
        }
        _evBuffer.clear();

        if(_numAnimFaces) {
            for(unsigned i = 0; i < _polyDesc->NumFaces(); ++i) {
                ColorState& state = _faceColorStates[i];
                if(state.changing) {
                    const float l = M::Min(1.0f, state.t / state.dur);
                    R::Color color = R::Lerp(state.sourceColor, state.targetColor, state.func(l));
                    _polyDesc->Set(state.rep, color);
                    state.t += secsPassed;

                    if(state.t >= state.dur) {
                        state.changing = false;
                        _numAnimFaces--;
                    }
                }
            } // forall faces

            InvalidateMesh();
        } // if _numAnimFaces
    }

    void ENT_Polyhedron::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        // solid hull
        renderJob.fx = "Lit";
        renderJob.vertices  = _solidMesh.vertices;
        renderJob.indices   = _solidMesh.indices;
        renderJob.primType  = _solidMesh.primType;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = GetMaterial();
        renderList.push_back(renderJob);

        // wireframe hull
        renderJob.fx = "Wireframe";
        renderJob.vertices  = _wireMesh.vertices;
        renderJob.indices   = _wireMesh.indices;
        renderJob.primType  = _wireMesh.primType;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = GetMaterial();
        renderList.push_back(renderJob);
    }

    void ENT_Polyhedron::Spawn(const Event& event) {
        Entity::Spawn(event);

        const SpawnArgs* spawnArgs = (const SpawnArgs*)event.args;
        _G = spawnArgs->G;

        /*
        NOTE: ignore this for now...
        if(!spawnArgs->G->empty())
            _polyDesc = GEN::Pointer<R::PolyhedronMesh>(new R::PolyhedronMesh(*_G));
        */
    }

    void ENT_Polyhedron::HandleEvent(const Event& event) {
        if(EVENT_REBUILD == event.id) {
            Rebuild();

            // drop events that refer to former states of the graph
            _evBuffer.clear();
        }
        if(EVENT_CHANGE_COLOR == event.id) {
            _evBuffer.push_back(event);
        }
    }

} // namespace W