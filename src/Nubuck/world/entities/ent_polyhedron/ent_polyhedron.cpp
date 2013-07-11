#include <common\common.h>
#include <world\events.h>
#include <renderer\mesh\quad\quad.h>
#include <world\world.h> // REMOVEME
#include <world\entities\ent_face\ent_face.h> // REMOVEME
#include "ent_polyhedron.h"

static float IP_Pulse(float l) { return sinf(M::PI * l); }
static float IP_Ident(float l) { return l; }

namespace W {

    void ENT_Polyhedron::Rebuild(void) {
        _polyDesc = GEN::Pointer<R::PolyhedronMesh>(new R::PolyhedronMesh(*_G));
        
        R::Mesh::Desc desc = _polyDesc->GetDesc();
        _mesh = R::meshMgr.Create(desc);

        _faceColorStates.resize(_polyDesc->NumFaces());
        for(unsigned i = 0; i < _polyDesc->NumFaces(); ++i) {
            _faceColorStates[i].changing = false;
        }

        _isBuild = true;
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

    void ENT_Polyhedron::ChangePosition(float dur) {
        if(!_isBuild) return;
        R::Mesh::Desc desc = _polyDesc->GetDesc();
        _changePositionState.sourcePositions.resize(desc.numVertices);
        _changePositionState.targetPositions.resize(desc.numVertices);
        for(unsigned i = 0; i < desc.numVertices; ++i)
            _changePositionState.sourcePositions[i] = desc.vertices[i].position;
        _polyDesc->Update();
        for(unsigned i = 0; i < desc.numVertices; ++i)
            _changePositionState.targetPositions[i] = desc.vertices[i].position;
        _changePositionState.dur = dur;
        _changePositionState.t = 0.0f;
        _changePositionState.isChanging = true;
    }

    ENT_Polyhedron::ENT_Polyhedron(void) : _numAnimFaces(0), _isBuild(false) { 
        _solidMat.diffuseColor = R::Color::White;
        _wireMat.diffuseColor = R::Color::Black;
    }

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

            /*
            R::MeshDesc desc = _polyDesc->GetSolidDesc();
            R::meshMgr.Update(_vertices, desc.vertices, desc.numVertices);
            */
        } // if _numAnimFaces

        if(_changePositionState.isChanging) {
            R::Mesh::Desc desc = _polyDesc->GetDesc();
            float l = M::Min(1.0f, _changePositionState.t / _changePositionState.dur);
            for(unsigned i = 0; i < desc.numVertices; ++i) {
                const M::Vector3& p0 = _changePositionState.sourcePositions[i];
                const M::Vector3& p1 = _changePositionState.targetPositions[i];
                desc.vertices[i].position = (1.0f - l) * p0 + l * p1;
            }
            _changePositionState.t += secsPassed;
            if(_changePositionState.t >= _changePositionState.dur)
                _changePositionState.isChanging = false;
            _mesh->Invalidate(desc.vertices);
        }
    }

    void ENT_Polyhedron::Render(std::vector<R::RenderJob>& renderList) {
        R::RenderJob renderJob;

        // solid hull
        renderJob.fx = "Lit";
        renderJob.mesh = _mesh;
        renderJob.primType  = GL_TRIANGLE_FAN;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = _solidMat;
        // renderList.push_back(renderJob);

        // wireframe hull
        renderJob.fx = "Wireframe";
        renderJob.mesh = _mesh;
        renderJob.primType  = GL_LINE_LOOP;
        renderJob.transform = M::Mat4::Translate(GetPosition());
        renderJob.material  = _wireMat;
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
        if(EVENT_UPDATE == event.id) ChangePosition(1.0f);
    }

} // namespace W