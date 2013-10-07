#include <functional>
#include <algorithm>

#include <common\common.h>
#include <algdriver\algdriver.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "polyhedron.h"
#include "entity.h"
#include "world.h"

namespace W {

    static SYS::SpinLock entIdCntMtx;
    static unsigned entIdCnt = 0;
    
    SYS::Semaphore g_worldSem(0);
    static int rlIdx = 1;

    World world;

    static M::Vector3 Transform(const M::Matrix4& mat, const M::Vector3& vec, float w) {
		M::Vector3 ret;
		for(int i = 0; i < 3; ++i) {
			float el = 0.0f;
			for(int j = 0; j < 3; ++j) {
				el += mat.mat[i + 4 * j] * vec.vec[j];
			}
			ret.vec[i] = el + w * mat.mat[i + 12];
		}
		return ret;
    }

    // given a point p in window space, return a point q in world space
    // that lies on the near clipping plane with the same window space
    // coordinates as p
    static M::Vector3 UnprojectPoint(
        const M::Matrix4& projectionMat, 
        const M::Matrix4& worldMat, 
        float screenWidth, 
        float screenHeight, 
        const M::Vector2& p) 
    {
        M::Vector3 q; 
        
        // in NDC
        q.x = 2.0f * (p.x / screenWidth) - 1.0f;
        q.y = -(2.0f * (p.y / screenHeight) - 1.0f);
        q.z = -1.0f; // on near clipping plane

        // clip.w = -eye.z, where eye.z is z-coord 
        // of near clipping plane
        const float fov = M::Deg2Rad(45.0f);
        float clipW = 1.0f / tan(0.5f * fov);

        // in clip space
        q *= clipW; // q.w = clipW

        M::Matrix4 M;
        M::TryInvert(projectionMat, M);
        M::Vector3 t = Transform(M, q, clipW); // in world space
        printf("eye = %f, %f, %f\n", t.x, t.y, t.z);
        M::Matrix3 normalMat = M::Inverse(M::RotationOf(worldMat));
        return Transform(normalMat, t);
    }

	static void AddRenderJobs(const ENT_Polyhedron* polyhedron) {
        R::RenderList& _renderList = R::g_renderLists[rlIdx];
		_renderList.jobs.insert(_renderList.jobs.end(),
			polyhedron->renderList.jobs.begin(),
			polyhedron->renderList.jobs.end());
        _renderList.nodePositions.insert(_renderList.nodePositions.end(),
            polyhedron->renderList.nodePositions.begin(),
            polyhedron->renderList.nodePositions.end());
        _renderList.nodeColors.insert(_renderList.nodeColors.end(),
            polyhedron->renderList.nodeColors.begin(),
            polyhedron->renderList.nodeColors.end());
        _renderList.edges.insert(_renderList.edges.end(),
            polyhedron->renderList.edges.begin(),
            polyhedron->renderList.edges.end());
	}

    void World::SetupLights(void) {
        R::Light light;

        R::RenderList& _renderList = R::g_renderLists[rlIdx];
        _renderList.lights.clear();

        _renderList.dirLights[0].direction      = M::Vector3(-1.0f,  1.0f,  0.0f);
        _renderList.dirLights[0].diffuseColor   = R::Color(0.6f, 0.85f, 0.91f);

        _renderList.dirLights[1].direction      = M::Vector3( 1.0f,  1.0f,  0.0f);
        _renderList.dirLights[1].diffuseColor   = R::Color(1.0f, 0.5f, 0.15f);

        _renderList.dirLights[2].direction      = M::Vector3( 0.0f, -1.0f, -1.0f);
        _renderList.dirLights[2].diffuseColor   = R::Color(1.0f, 1.0f, 1.0f);

        float dist = 20;

        light.constantAttenuation   = 1.0f;
        light.linearAttenuation     = 0.01f;
        light.quadricAttenuation    = 0.0f;

        light.position          = M::Vector3(-dist,  dist, dist);
        // light.diffuseColor      = R::Color::Red;
        light.diffuseColor = R::Color(0.8f, 0.8f, 0.8f);
        _renderList.lights.push_back(light);

        light.position          = M::Vector3( dist,  dist, dist);
        // light.diffuseColor      = R::Color::White;
        light.diffuseColor = R::Color(0.8f, 0.8f, 0.8f);
        _renderList.lights.push_back(light);

        light.position          = M::Vector3( dist, -dist, dist);
        // light.diffuseColor      = R::Color::Blue;
        light.diffuseColor = R::Color(0.8f, 0.8f, 0.8f);
        _renderList.lights.push_back(light);
    }

    static float mouseX, mouseY;
    static float screenWidth, screenHeight;
    static float aspect;

    void World::HandleMouseEvent(const Event& event) {
        EvArgs_Mouse* args = (EvArgs_Mouse*)event.args;

        if(EvArgs_Mouse::MOUSE_DOWN == args->type) {
            if(EvArgs_Mouse::BUTTON_LEFT == args->button) {
                if(_isGrabbing) {
                    _isGrabbing = false;

                    for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
                        ENT_Polyhedron& ph = *_polyhedrons[i];
                        leda::node n;
                        forall_nodes(n, *ph.G) {
                            const M::Vector3& p = ph.nodes.positions[n->id()];
                            (*ph.G)[n] = point_t(p.x, p.y, p.z);
                        }
                    }
                    ALG::gs_algorithm.GetPhase()->OnNodesMoved();
                }

                if(EvArgs_Mouse::MODIFIER_SHIFT == args->mods)
                    _camArcball.StartZooming(args->x, args->y);
                else _camArcball.StartDragging(args->x, args->y);
            }
            if(EvArgs_Mouse::BUTTON_RIGHT == args->button) {
                if(_isGrabbing) {
                    for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
                        ENT_Polyhedron& ph = *_polyhedrons[i];
                        ph.nodes.positions = ph.nodes.oldPositions;
                    }

                    _isGrabbing = false;
                }

                // _camArcball.StartPanning(args->x, args->y);

                // picking
                M::Matrix4 projectionMat = M::Mat4::Perspective(45.0f, aspect, 0.1f, 1000.0f);
                M::Matrix4 invWorldMat;
                M::TryInvert(_camArcball.GetWorldMatrix(), invWorldMat);
                M::Vector3 rayOrig = M::Transform(invWorldMat, M::Vector3::Zero);
                mouseX = args->x;
                mouseY = args->y;
                M::Vector3 rayDir = UnprojectPoint(projectionMat, _camArcball.GetWorldMatrix(), screenWidth, screenHeight, M::Vector2(mouseX, mouseY));
                rayDir.Normalize();

                printf("orig = %f, %f, %f\n", rayOrig.x, rayOrig.y, rayOrig.z);
                printf("dir = %f, %f, %f\n", rayDir.x, rayDir.y, rayDir.z);
                for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
                    leda::node hitNode = NULL;
                    ENT_Polyhedron& ph = *_polyhedrons[i];
                    if(Polyhedron_RaycastNodes(ph, rayOrig, rayDir, hitNode)) {
                        printf("Hit!\n");
                        ph.selection.nodes[hitNode->id()] = !ph.selection.nodes[hitNode->id()];
                    } else printf("No hit...\n");
                }
            }
        }

        if(EvArgs_Mouse::MOUSE_UP == args->type) {
            if(EvArgs_Mouse::BUTTON_LEFT  == args->button) {
                _camArcball.StopDragging();
                _camArcball.StopZooming();
            }
            if(EvArgs_Mouse::BUTTON_RIGHT == args->button)
                _camArcball.StopPanning();
        }

        if(EvArgs_Mouse::MOUSE_MOVE == args->type) {
            _camArcball.Drag(args->x, args->y);
            _camArcball.Pan(args->x, args->y);
            _camArcball.Zoom(args->x, args->y);
            mouseX = args->x;
            mouseY = args->y;
        }

        if(EvArgs_Mouse::MOUSE_WHEEL == args->type) {
            if(args->delta > 0) _camArcball.ZoomIn();
            if(args->delta < 0) _camArcball.ZoomOut();
        }
    }

    ENT_Polyhedron* World::FindByEntityID(unsigned entId) {
        for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
            if(_polyhedrons[i]->entId == entId) return _polyhedrons[i];
        }
        return NULL;
    }

	World::World(void) : _camArcball(800, 400) /* init values arbitrary */ {
        _isGrabbing = false;

        SetupLights();
	}

    void World::Send(const Event& event) {
        _eventsMtx.Lock();
        _events.push(event);
        _eventsMtx.Unlock();
    }

	unsigned World::SpawnPolyhedron(graph_t* const G) {
        entIdCntMtx.Lock();
        unsigned entId = entIdCnt++;
        entIdCntMtx.Unlock();

        Event event;
        event.type = EVENT_SPAWN_POLYHEDRON;
        EvArgs_SpawnPolyhedron* args = (EvArgs_SpawnPolyhedron*)event.args;
        args->h = entId;
        args->G = G;
        Send(event);

        return entId;
	}

    void World::Update(void) {
        _secsPassed = _timer.Stop();
        _timePassed += _secsPassed;
        _timer.Start();

        SetupLights();

        bool done = false;
        while(!done) {
            Event event;
            _eventsMtx.Lock();
            if(_events.empty()) done = true;
            else {
                event = _events.front();
                _events.pop();
            }
            _eventsMtx.Unlock();
            if(done) break;

            if(EVENT_APOCALYPSE == event.type) {
                _polyhedrons.clear();
            }

            if(EVENT_SPAWN_POLYHEDRON == event.type) {
                EvArgs_SpawnPolyhedron* args = (EvArgs_SpawnPolyhedron*)event.args;
                ENT_Polyhedron* ph = new ENT_Polyhedron();
                ph->entId = args->h;
                ph->G = args->G;
                Polyhedron_Init(*ph);
                Polyhedron_Rebuild(*ph);
                Polyhedron_Update(*ph);
                _polyhedrons.push_back(ph);
            }

            if(EVENT_DESTROY_POLYHEDRON == event.type) {
                EvArgs_DestroyPolyhedron* args = (EvArgs_DestroyPolyhedron*)event.args;
                for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
                    if(_polyhedrons[i]->entId == args->entId) {
                        std::swap(_polyhedrons[i], _polyhedrons.back());
                        _polyhedrons.erase(_polyhedrons.end() - 1);
                    }
                }
            }

            if(EVENT_REBUILD == event.type) {
                EvArgs_Rebuild* args = (EvArgs_Rebuild*)event.args;
                ENT_Polyhedron* ph = FindByEntityID(args->entId);
                if(ph) {
                    Polyhedron_Rebuild(*ph);
                    Polyhedron_Update(*ph);
                }
            }

            if(EVENT_SET_NODE_COLOR == event.type) {
                EvArgs_SetNodeColor* args = (EvArgs_SetNodeColor*)event.args;
                ENT_Polyhedron* ph = FindByEntityID(args->entId);
                if(ph) {
                    ph->nodes.colors[args->node->id()] = args->color;
                }
            }

            if(EVENT_SET_FACE_COLOR == event.type) {
                EvArgs_SetFaceColor* args = (EvArgs_SetFaceColor*)event.args;
                ENT_Polyhedron* ph = FindByEntityID(args->entId);
                /*
                PolyhedronHullFaceList& face = ph.hull.faceLists[ph.hull.edges[args->edge->id()].faceIdx];
                for(unsigned i = 0; i < face.size; ++i) {
                    ph.hull.vertices[face.base + i].color = args->color;
                }
                Polyhedron_Update(ph);
                */
                // Polyhedron_AddCurve(ph, args->edge, args->color);
                Polyhedron_SetFaceColor(*ph, args->edge, args->color);
            }

            if(EVENT_RESIZE == event.type) {
                EvArgs_Resize* args = (EvArgs_Resize*)event.args;
                _camArcball.SetScreenSize(args->width, args->height);
                aspect = (float)args->width / args->height;
                screenWidth = args->width;
                screenHeight = args->height;
            }

            if(EVENT_MOUSE == event.type) HandleMouseEvent(event);

            if(EVENT_KEY == event.type) {
                EvArgs_Key* args = (EvArgs_Key*)event.args;
                if(!_isGrabbing && 'G' == args->keyCode) {
                    if(W::EvArgs_Key::KEY_DOWN == args->type) {
                        _grabPivot.x = mouseX;
                        _grabPivot.y = mouseY;
                        _isGrabbing = true;

                        for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
                            ENT_Polyhedron& ph = *_polyhedrons[i];
                            ph.nodes.oldPositions = ph.nodes.positions;
                        }
                    } else _isGrabbing = false;
                }
            }
        } // while(!done)

        if(_isGrabbing) {
            float dx = mouseX - _grabPivot.x;
            float dy = -mouseY + _grabPivot.y;
            dx *= 0.05f;
            dy *= 0.05f;
            for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
                ENT_Polyhedron& ph = *_polyhedrons[i];
                unsigned numNodes = ph.nodes.positions.size();
                for(unsigned j = 0; j < numNodes; ++j) {
                    if(ph.selection.nodes[j]) {
                        ph.nodes.positions[j].x = ph.nodes.oldPositions[j].x + dx;
                        ph.nodes.positions[j].y = ph.nodes.oldPositions[j].y + dy;
                    }
                }
            }
        }

        for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
            for(unsigned j = 0; j < _polyhedrons[i]->hull.curves.size(); ++j)
                Polyhedron_UpdateCurve(_polyhedrons[i]->hull.curves[j], _secsPassed);
        }

        for(unsigned i = 0; i < _polyhedrons.size(); ++i) {
            Polyhedron_UpdateFaceColors(*_polyhedrons[i], _secsPassed);
        }

        for(unsigned i = 0; i < _polyhedrons.size(); ++i)
            Polyhedron_BuildRenderList(*_polyhedrons[i]);

        R::RenderList& _renderList = R::g_renderLists[rlIdx];
        _renderList.worldMat = _camArcball.GetWorldMatrix();
		_renderList.jobs.clear();
        _renderList.nodePositions.clear();
        _renderList.nodeColors.clear();
        _renderList.edges.clear();
		std::for_each(_polyhedrons.begin(), _polyhedrons.end(),
			std::bind(AddRenderJobs, std::placeholders::_1));

        g_worldSem.Signal();
        R::g_rendererSem.Wait();
        rlIdx = 1 - rlIdx;
    }

    IPolyhedron* World::CreatePolyhedron(graph_t& G) {
        return new Polyhedron(G);
    }

    DWORD World::Thread_Func(void) {
        Polyhedron_InitResources();
        while(true) {
            Update();
            // Sleep(10);
        }
    }

} // namespace W
