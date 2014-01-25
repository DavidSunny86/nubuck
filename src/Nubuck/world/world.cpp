#include <functional>
#include <algorithm>

#include <common\common.h>
#include <algdriver\algdriver.h>
#include <system\locks\scoped_lock.h>
#include <renderer\mesh\plane\plane.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\cylinder\cylinder.h>
#include <renderer\nodes\r_nodes.h>
#include <renderer\edges\r_cylinder_edges.h>
#include <renderer\edges\r_line_edges.h>
#include <events\event_defs.h>
#include <UI\outliner\outliner.h>
#include <operators\operators.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include <world\entities\ent_mesh\ent_mesh.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <world\proxies\proxy_polyhedron.h>
#include <world\proxies\proxy_mesh.h>
#include "entity.h"
#include "world.h"

namespace W {

    static SYS::SpinLock entIdCntMtx;
    static unsigned entIdCnt = 0;
    
    SYS::Semaphore g_worldSem(0);
    static int rlIdx = 1;

    World world;

    BEGIN_EVENT_HANDLER(World)
        EVENT_HANDLER(EV::def_Apocalypse,           &World::Event_Apocalypse)
        EVENT_HANDLER(EV::def_LinkEntity,           &World::Event_LinkEntity)
        EVENT_HANDLER(EV::def_SpawnPolyhedron,      &World::Event_SpawnPolyhedron)
        EVENT_HANDLER(EV::def_SpawnMesh,            &World::Event_SpawnMesh)
        EVENT_HANDLER(EV::def_DestroyEntity,        &World::Event_DestroyEntity)
        EVENT_HANDLER(EV::def_Rebuild,              &World::Event_Rebuild)
        EVENT_HANDLER(EV::def_SetVisible,           &World::Event_SetVisible)
        EVENT_HANDLER(EV::def_SetName,              &World::Event_SetName)
        EVENT_HANDLER(EV::def_SetPosition,          &World::Event_SetPosition)
        EVENT_HANDLER(EV::def_SetScale,             &World::Event_SetScale)
        EVENT_HANDLER(EV::def_SetRotation,          &World::Event_SetRotation)
        EVENT_HANDLER(EV::def_SetRenderFlags,       &World::Event_SetRenderFlags)
        EVENT_HANDLER(EV::def_SetPickable,          &World::Event_SetPickable)
        EVENT_HANDLER(EV::def_SetNodeColor,         &World::Event_SetNodeColor)
        EVENT_HANDLER(EV::def_SetEdgeColor, 		&World::Event_SetEdgeColor)
        EVENT_HANDLER(EV::def_SetFaceColor, 		&World::Event_SetFaceColor)
        EVENT_HANDLER(EV::def_SetFaceVisibility, 	&World::Event_SetFaceVisibility)
        EVENT_HANDLER(EV::def_SetHullAlpha,         &World::Event_SetHullAlpha)
        EVENT_HANDLER(EV::def_SetEdgeBaseColor, 	&World::Event_SetEdgeBaseColor)
        EVENT_HANDLER(EV::def_SetEdgeRadius,        &World::Event_SetEdgeRadius)
        EVENT_HANDLER(EV::def_SetEffect,            &World::Event_SetEffect)
        EVENT_HANDLER(EV::def_Resize,               &World::Event_Resize)
        EVENT_HANDLER(EV::def_Mouse,                &World::Event_Mouse)
        EVENT_HANDLER(EV::def_Key,                  &World::Event_Key)
    END_EVENT_HANDLER

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
        // printf("eye = %f, %f, %f\n", t.x, t.y, t.z);
        M::Matrix3 normalMat = M::Inverse(M::RotationOf(worldMat));
        return Transform(normalMat, t);
    }

    void World::SetupLights(R::RenderList& renderList) {
        renderList.dirLights[0].direction      = M::Vector3(-1.0f,  1.0f,  0.0f);
        renderList.dirLights[0].diffuseColor   = R::Color(0.6f, 0.85f, 0.91f);

        renderList.dirLights[1].direction      = M::Vector3( 1.0f,  1.0f,  0.0f);
        renderList.dirLights[1].diffuseColor   = R::Color(1.0f, 0.5f, 0.15f);

        renderList.dirLights[2].direction      = M::Vector3( 0.0f, -0.5f, -1.5f);
        renderList.dirLights[2].diffuseColor   = R::Color(1.0f, 1.0f, 1.0f);
    }

    static float mouseX, mouseY;
    static float screenWidth, screenHeight;
    static float aspect;

    void World::Event_Mouse(const EV::Event& event) {
        const EV::Params_Mouse& args = EV::def_Mouse.GetArgs(event);

        if(EV::Params_Mouse::MOUSE_DOWN == args.type) {
            M::Matrix4 projectionMat = M::Mat4::Perspective(45.0f, aspect, 0.1f, 1000.0f);
            M::Matrix4 invWorldMat;
            M::TryInvert(_camArcball.GetWorldMatrix(), invWorldMat);
            M::Vector3 rayOrig = M::Transform(invWorldMat, M::Vector3::Zero);
            mouseX = args.x;
            mouseY = args.y;
            M::Vector3 rayDir = UnprojectPoint(projectionMat, _camArcball.GetWorldMatrix(), screenWidth, screenHeight, M::Vector2(mouseX, mouseY));
            rayDir.Normalize();
            printf("orig = %f, %f, %f\n", rayOrig.x, rayOrig.y, rayOrig.z);
            printf("dir = %f, %f, %f\n", rayDir.x, rayDir.y, rayDir.z);

            if(EV::Params_Mouse::BUTTON_LEFT == args.button) {
                if(_isGrabbing) {
                    _isGrabbing = false;

                    /*
                    TODO
                    for(unsigned i = 0; i < _entities.size(); ++i) {
                        if(ENT_POLYHEDRON == _entities[i]->type) {
                            ENT_Polyhedron& ph = *_entities[i]->polyhedron;
                            leda::node n;
                            forall_nodes(n, *ph.G) {
                                if(ph.selection.nodes[n->id()]) {
                                    const M::Vector3& p = ph.nodes.positions[n->id()];
                                    (*ph.G)[n] = point_t(leda::rational(p.x), leda::rational(p.y), leda::rational(p.z));
                                }
                            }
                        }
                    }
                    GEN::Pointer<IPhase> phase = ALG::gs_algorithm.GetPhase();
                    if(phase.IsValid()) phase->OnNodesMoved();
                    */
                }

                if(EV::Params_Mouse::MODIFIER_SHIFT == args.mods)
                    _camArcball.StartZooming(args.x, args.y);
                else _camArcball.StartDragging(args.x, args.y);

                // pick faces
                for(unsigned i = 0; i < _entities.size(); ++i) {
                    if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                        ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                        if(!ph.isPickable) continue;
                        leda::edge hitFace = NULL;
                        if(Polyhedron_RaycastFaces(ph, rayOrig, rayDir, hitFace)) {
                            printf("hit face!\n");
                        } else printf("no face hit\n");
                    }
                }
            }
            if(EV::Params_Mouse::BUTTON_RIGHT == args.button) {
                if(_isGrabbing) {
                    for(unsigned i = 0; i < _entities.size(); ++i) {
                        if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                            ph.nodes.positions = ph.nodes.oldPositions;
                        }
                    }
                    // update
                    /* TODO
                    for(unsigned i = 0; i < _entities.size(); ++i) {
                        if(ENT_POLYHEDRON == _entities[i]->type) {
                            ENT_Polyhedron& ph = *_entities[i]->polyhedron;
                            leda::node n;
                            forall_nodes(n, *ph.G) {
                                if(ph.selection.nodes[n->id()]) {
                                    const M::Vector3& p = ph.nodes.positions[n->id()];
                                    (*ph.G)[n] = point_t(leda::rational(p.x), leda::rational(p.y), leda::rational(p.z));
                                }
                            }
                        }
                    }
                    GEN::Pointer<IPhase> phase = ALG::gs_algorithm.GetPhase();
                    if(phase.IsValid()) phase->OnNodesMoved();
                    */

                    _isGrabbing = false;
                } else _camArcball.StartPanning(args.x, args.y);

                for(unsigned i = 0; i < _entities.size(); ++i) {
                    if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                        ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                        leda::node hitNode = NULL;
                        if(!ph.isPickable) continue;
                        if(Polyhedron_RaycastNodes(ph, rayOrig, rayDir, hitNode)) {
                            printf("Hit!\n");
                            ph.selection.nodes[hitNode->id()] = !ph.selection.nodes[hitNode->id()];
                        } else printf("No hit...\n");
                    }
                }
            }
        }

        if(EV::Params_Mouse::MOUSE_UP == args.type) {
            if(EV::Params_Mouse::BUTTON_LEFT  == args.button) {
                _camArcball.StopDragging();
                _camArcball.StopZooming();
            }
            if(EV::Params_Mouse::BUTTON_RIGHT == args.button)
                _camArcball.StopPanning();
        }

        if(EV::Params_Mouse::MOUSE_MOVE == args.type) {
            _camArcball.Drag(args.x, args.y);
            _camArcball.Pan(args.x, args.y);
            _camArcball.Zoom(args.x, args.y);
            mouseX = args.x;
            mouseY = args.y;

            OP::g_operators.OnMouseMove(M::Vector2(mouseX, mouseY));
        }

        if(EV::Params_Mouse::MOUSE_WHEEL == args.type) {
            if(args.delta > 0) _camArcball.ZoomIn();
            if(args.delta < 0) _camArcball.ZoomOut();
            OP::g_operators.OnMouseMove(M::Vector2(mouseX, mouseY));
        }
    }

    GEN::Pointer<Entity> World::FindByEntityID(unsigned entId) {
        for(unsigned i = 0; i < _entities.size(); ++i) {
            if(_entities[i]->GetID() == entId) return _entities[i];
        }
        return GEN::Pointer<Entity>();
    }

    void World::GetInfo(Entity& ent, EntityInf& inf) {
        inf.entId   = ent.GetID();
        inf.name    = ent.GetName();
        
        if(EntityType::ENT_POLYHEDRON == ent.GetType()) {
            ENT_Polyhedron& ph = *(static_cast<ENT_Polyhedron*>(&ent));
            INF_Polyhedron* phInf = new INF_Polyhedron;
            Polyhedron_GetInfo(ph, *phInf);
            inf.inf = phInf;
        }
    }
        
    void World::Event_Apocalypse(const EV::Event& event) {
        _entities.clear();
    }

    void World::Event_LinkEntity(const EV::Event& event) {
        const EV::Params_LinkEntity& args = EV::def_LinkEntity.GetArgs(event);
        GEN::Pointer<Entity> entity(args.entity);
        _entities.push_back(entity);
    }

    void World::Event_SpawnPolyhedron(const EV::Event& event) {
        const EV::Params_SpawnPolyhedron& args = EV::def_SpawnPolyhedron.GetArgs(event);

        GEN::Pointer<ENT_Polyhedron> ph(new ENT_Polyhedron());
        ph->SetType(EntityType::ENT_POLYHEDRON);
        ph->SetID(args.entId);
        ph->SetFxName("LitDirectional");

        EntTransform transform;
        transform.position = M::Vector3::Zero;
        transform.scale = M::Vector3(1.0f, 1.0f, 1.0f);
        transform.rotation = M::Mat3::Identity();
        ph->SetTransform(transform);

        const graph_t& G = *args.G;
        Polyhedron_Init(*ph);
        Polyhedron_Rebuild(*ph, G, *args.cachedNodes, *args.cachedEdges);
        Polyhedron_Update(*ph, G);
        _entities.push_back(ph);

        UI::Outliner::Instance()->Send(event);

        EntityInf* inf = new EntityInf;
        GetInfo(*ph, *inf);
        EV::Params_EntityInfo infArgs;
        infArgs.entType = EntityType::ENT_POLYHEDRON;
        infArgs.inf = inf;
        UI::Outliner::Instance()->Send(EV::def_EntityInfo.Create(infArgs));

        event.Accept();
    }

    void World::Event_SpawnMesh(const EV::Event& event) {
        const EV::Params_SpawnMesh& args = EV::def_SpawnMesh.GetArgs(event);
        GEN::Pointer<ENT_Mesh> mesh(new ENT_Mesh());
        mesh->SetType(EntityType::ENT_MESH);
        mesh->SetID(args.entId);
        mesh->SetFxName("LitDirectional");

        EntTransform transform;
        transform.position = M::Vector3::Zero;
        transform.scale = M::Vector3(1.0f, 1.0f, 1.0f);
        transform.rotation = M::Mat3::Identity();
        mesh->SetTransform(transform);

        Mesh_Init(*mesh, args.meshPtr);
        _entities.push_back(mesh);
    }

    void World::Event_DestroyEntity(const EV::Event& event) {
        const EV::Params_DestroyEntity& args = EV::def_DestroyEntity.GetArgs(event);
        for(unsigned i = 0; i < _entities.size(); ++i) {
            if(_entities[i]->GetID() == args.entId) {
                std::swap(_entities[i], _entities.back());
                _entities.erase(_entities.end() - 1);
            }
        }
    }

    void World::Event_Rebuild(const EV::Event& event) {
        const EV::Params_Rebuild& args = EV::def_Rebuild.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            const graph_t& G = *args.G;
            Polyhedron_Rebuild(ph, G, *args.cachedNodes, *args.cachedEdges);
            Polyhedron_Update(ph, G);
        }

        event.Accept();
    }

    void World::Event_SetVisible(const EV::Event& event) {
        const EV::Params_SetVisible& args = EV::def_SetVisible.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_MESH == entity->GetType()) {
            ENT_Mesh& mesh = static_cast<ENT_Mesh&>(*entity);
            mesh.isVisible = args.isVisible;
        }
    }

    void World::Event_SetName(const EV::Event& event) {
        const EV::Params_SetName& args = EV::def_SetName.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        assert(entity.IsValid());
        entity->SetName(args.name);

        UI::Outliner::Instance()->Send(event);
    }

    void World::Event_SetPosition(const EV::Event& event) {
        const EV::Params_SetPosition& args = EV::def_SetPosition.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid()) entity->GetTransform().position = args.pos;
    }

    void World::Event_SetScale(const EV::Event& event) {
        const EV::Params_SetScale& args = EV::def_SetScale.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid()) entity->GetTransform().scale = M::Vector3(args.sx, args.sy, args.sz);
    }

    void World::Event_SetRotation(const EV::Event& event) {
        const EV::Params_SetRotation& args = EV::def_SetRotation.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid()) entity->GetTransform().rotation = args.mat;
    }

    void World::Event_SetRenderFlags(const EV::Event& event) {
        const EV::Params_SetRenderFlags& args = EV::def_SetRenderFlags.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            ph.renderFlags = args.flags;
        }
    }

    void World::Event_SetPickable(const EV::Event& event) {
        const EV::Params_SetPickable& args = EV::def_SetPickable.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            ph.isPickable = args.isPickable;
        }
    }

    void World::Event_SetNodeColor(const EV::Event& event) {
        const EV::Params_SetNodeColor& args = EV::def_SetNodeColor.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            ph.nodes.colors[args.node->id()] = args.color;
        }
    }

    void World::Event_SetEdgeColor(const EV::Event& event) {
        const EV::Params_SetEdgeColor& args = EV::def_SetEdgeColor.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            ph.edges.colors[args.edge->id()].cur = args.color;
        }
    }

    void World::Event_SetFaceColor(const EV::Event& event) {
        const EV::Params_SetFaceColor& args = EV::def_SetFaceColor.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            /*
            PolyhedronHullFaceList& face = ph.hull.faceLists[ph.hull.edges[args->edge->id()].faceIdx];
            for(unsigned i = 0; i < face.size; ++i) {
                ph.hull.vertices[face.base + i].color = args->color;
            }
            Polyhedron_Update(ph);
            */
            // Polyhedron_AddCurve(ph, args->edge, args->color);
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            Polyhedron_SetFaceColor(ph, args.edge, args.color);
        }
    }

    void World::Event_SetFaceVisibility(const EV::Event& event) {
        const EV::Params_SetFaceVisibility& args = EV::def_SetFaceVisibility.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            Polyhedron_SetFaceVisibility(ph, args.edge, args.visible);
            // Polyhedron_Update(ph); TODO
        }
    }

    void World::Event_SetHullAlpha(const EV::Event& event) {
        const EV::Params_SetHullAlpha& args = EV::def_SetHullAlpha.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        if(entity.IsValid() && EntityType::ENT_POLYHEDRON == entity->GetType()) {
            ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
            Polyhedron_SetHullAlpha(ph, args.alpha);
        }
    }

    void World::Event_SetEdgeBaseColor(const EV::Event& event) {
        const EV::Params_SetEdgeBaseColor& args = EV::def_SetEdgeBaseColor.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        assert(entity.IsValid());
        assert(EntityType::ENT_POLYHEDRON == entity->GetType());
        ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
        ph.edges.baseColor = args.color;
    }

    void World::Event_SetEdgeRadius(const EV::Event& event) {
        const EV::Params_SetEdgeRadius& args = EV::def_SetEdgeRadius.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        assert(entity.IsValid());
        assert(EntityType::ENT_POLYHEDRON == entity->GetType());
        ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*entity);
        ph.edges.radius = args.radius;
    }

    void World::Event_SetEffect(const EV::Event& event) {
        const EV::Params_SetEffect& args = EV::def_SetEffect.GetArgs(event);
        GEN::Pointer<Entity> entity = FindByEntityID(args.entId);
        assert(entity.IsValid());
        entity->SetFxName(args.fxName);
    }

    void World::Event_Resize(const EV::Event& event) {
        const EV::Params_Resize& args = EV::def_Resize.GetArgs(event);
        _camArcball.SetScreenSize(args.width, args.height);
        aspect = (float)args.width / args.height;
        screenWidth = args.width;
        screenHeight = args.height;
    }

    void World::Event_Key(const EV::Event& event) {
        const EV::Params_Key& args = EV::def_Key.GetArgs(event);
        if(!_isGrabbing && 'G' == args.keyCode) {
            if(EV::Params_Key::KEY_DOWN == args.type) {
                _grabPivot.x = mouseX;
                _grabPivot.y = mouseY;
                _isGrabbing = true;


                for(unsigned i = 0; i < _entities.size(); ++i) {
                    if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                        ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                        ph.nodes.oldPositions = ph.nodes.positions;
                    }
                }
            } else _isGrabbing = false;
        }

        if('R' == args.keyCode) _camArcball.ResetRotation();
        if('E' == args.keyCode) _camArcball.Reset();

        if(EV::Params_Key::KEY_DOWN == args.type && !args.autoRepeat) {
            GEN::Pointer<IPhase> phase = ALG::gs_algorithm.GetPhase();
            if(phase.IsValid()) phase->OnKeyPressed((char)args.keyCode);
        }
    }

	World::World(void) : _camArcball(800, 400) /* init values arbitrary */ {
        _isGrabbing = false;
	}

    M::Ray World::PickingRay(const M::Vector2& mouseCoords) {
        M::Matrix4 projectionMat = M::Mat4::Perspective(45.0f, aspect, 0.1f, 1000.0f);
        M::Matrix4 invWorldMat;
        M::TryInvert(_camArcball.GetWorldMatrix(), invWorldMat);
        M::Ray ray;
        ray.origin = M::Transform(invWorldMat, M::Vector3::Zero);
        ray.direction = UnprojectPoint(projectionMat, _camArcball.GetWorldMatrix(), screenWidth, screenHeight, mouseCoords);
        ray.direction.Normalize();
        return ray;
    }

	unsigned World::SpawnPolyhedron(graph_t* const G, leda::node_map<bool>* cachedNodes, leda::edge_map<bool>* cachedEdges) {
        entIdCntMtx.Lock();
        unsigned entId = entIdCnt++;
        entIdCntMtx.Unlock();

        EV::Params_SpawnPolyhedron args;
        args.entId          = entId;
        args.G      		= G;
        args.cachedNodes    = cachedNodes;
        args.cachedEdges 	= cachedEdges;
        Send(EV::def_SpawnPolyhedron.Create(args)); // TODO: block?

        return entId;
	}

    unsigned World::SpawnMesh(R::meshPtr_t meshPtr) {
        entIdCntMtx.Lock();
        unsigned entId = entIdCnt++;
        entIdCntMtx.Unlock();

        EV::Params_SpawnMesh args;
        args.entId      = entId;
        args.meshPtr    = meshPtr;
        Send(EV::def_SpawnMesh.Create(args));

        return entId;
    }

    void World::Update(void) {
        _secsPassed = _timer.Stop();
        _timePassed += _secsPassed;
        _timer.Start();

        SYS::ScopedLock lockEntities(_entitiesMtx);

        HandleEvents();

        if(_isGrabbing) {
            float dx = mouseX - _grabPivot.x;
            float dy = -mouseY + _grabPivot.y;
            dx *= 0.1f;
            dy *= 0.1f;

            const M::Matrix3 M = M::Inverse(M::RotationOf(_camArcball.GetWorldMatrix()));
            const M::Vector3 X = M::Transform(M, M::Vector3(1.0f, 0.0f, 0.0f));
            const M::Vector3 Y = M::Transform(M, M::Vector3(0.0f, 1.0f, 0.0f));
            M::Matrix4 invWorld;
            M::TryInvert(_camArcball.GetWorldMatrix(), invWorld);
            const M::Vector3 rayOrig = M::Transform(invWorld, M::Vector3::Zero);

            for(unsigned i = 0; i < _entities.size(); ++i) {
                if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                    ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                    unsigned numNodes = ph.nodes.positions.size();
                    for(unsigned j = 0; j < numNodes; ++j) {
                        if(ph.selection.nodes[j]) {
                            const M::Vector3 p = ph.nodes.oldPositions[j] + X * dx + Y * dy;
                            const M::Vector3 rayDir = p - rayOrig;
                            assert(0.0f != rayOrig.z - p.z);
                            float t = rayOrig.z / (rayOrig.z - p.z);
                            M::Vector3 q = rayOrig + t * rayDir;
                            q.z = 0.0f;
                            // assert(M::AlmostEqual(q.z, 0.0f));
                            ph.nodes.positions[j] = q;
                            printf("q = (%f, %f, %f)\n", q.x, q.y, q.z);
                            // ph.nodes.positions[j] = ph.nodes.oldPositions[j] + M::Vector3(1.0f, 0.0f, 0.0f) * dx + M::Vector3(0.0f, 1.0f, 0.0f) * dy;
                        }
                    }
                }
            }

            // update
            /* TODO
            for(unsigned i = 0; i < _entities.size(); ++i) {
                if(ENT_POLYHEDRON == _entities[i]->type) {
                    ENT_Polyhedron& ph = *_entities[i]->polyhedron;
                    leda::node n;
                    forall_nodes(n, *ph.G) {
                        if(ph.selection.nodes[n->id()]) {
                            const M::Vector3& p = ph.nodes.positions[n->id()];
                            (*ph.G)[n] = point_t(leda::rational(p.x), leda::rational(p.y), leda::rational(p.z));
                        }
                    }
                }
            }
            GEN::Pointer<IPhase> phase = ALG::gs_algorithm.GetPhase();
            if(phase.IsValid()) phase->OnNodesMoved();
            */
        } // if(isGrabbing)

        for(unsigned i = 0; i < _entities.size(); ++i) {
            if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                Polyhedron_Update(ph);
            }
        }

        for(unsigned i = 0; i < _entities.size(); ++i) {
            if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                for(unsigned j = 0; j < ph.faces.curves.size(); ++j)
                    Polyhedron_UpdateCurve(ph.faces.curves[j], _secsPassed);
            }
        }

        for(unsigned i = 0; i < _entities.size(); ++i) {
            if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                Polyhedron_UpdateFaceColors(ph, _secsPassed);
            }
        }

        for(unsigned i = 0; i < _entities.size(); ++i) {
            GEN::Pointer<Entity> entity = _entities[i];
            if(EntityType::ENT_POLYHEDRON == entity->GetType()) {
                ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                Polyhedron_BuildRenderList(ph, entity->GetFxName());
            }
            if(EntityType::ENT_MESH == entity->GetType()) {
                ENT_Mesh& mesh = static_cast<ENT_Mesh&>(*entity);
                const EntTransform& tf = entity->GetTransform();
                M::Matrix4 transform = 
                    M::Mat4::Translate(tf.position) * 
                    M::Mat4::FromRigidTransform(tf.rotation, M::Vector3::Zero) * 
                    M::Mat4::Scale(tf.scale.x, tf.scale.y, tf.scale.z);
                Mesh_BuildRenderList(mesh, entity->GetFxName(), transform);
            }
        }

        for(unsigned i = 0; i < _entities.size(); ++i) {
            GEN::Pointer<Entity> entity = _entities[i];
            if(EntityType::ENT_GEOMETRY == entity->GetType()) {
                ENT_Geometry& geom = static_cast<ENT_Geometry&>(*entity);
                geom.CompileMesh();
                geom.FrameUpdate();
                geom.BuildRenderList();
            }
        }

    }

    void World::Render(R::RenderList& renderList) {
        SetupLights(renderList);

        renderList.worldMat = _camArcball.GetWorldMatrix();
		renderList.meshJobs.clear();

        for(unsigned i = 0; i < _entities.size(); ++i) {
            if(EntityType::ENT_POLYHEDRON == _entities[i]->GetType()) {
                ENT_Polyhedron& ph = static_cast<ENT_Polyhedron&>(*_entities[i]);
                renderList.meshJobs.insert(renderList.meshJobs.end(),
                    ph.renderList.meshJobs.begin(),
                    ph.renderList.meshJobs.end());
            }
            if(EntityType::ENT_MESH == _entities[i]->GetType()) {
                ENT_Mesh& mesh = static_cast<ENT_Mesh&>(*_entities[i]);
                renderList.meshJobs.insert(renderList.meshJobs.end(),
                    mesh.renderList.meshJobs.begin(),
                    mesh.renderList.meshJobs.end());
            }
            if(EntityType::ENT_GEOMETRY == _entities[i]->GetType()) {
                ENT_Geometry& geom = static_cast<ENT_Geometry&>(*_entities[i]);
                renderList.meshJobs.insert(renderList.meshJobs.end(),
                    geom.GetRenderList().meshJobs.begin(),
                    geom.GetRenderList().meshJobs.end());
            }
        }
    }

    IPolyhedron* World::CreatePolyhedron(void) {
        return new Proxy::Polyhedron(graph_t());
    }

    IGeometry* World::CreateGeometry() {
        entIdCntMtx.Lock();
        unsigned entId = entIdCnt++;
        entIdCntMtx.Unlock();

        ENT_Geometry* geom = new ENT_Geometry;
        geom->SetType(EntityType::ENT_GEOMETRY);
        geom->SetID(entId);
        geom->SetFxName("LitDirectional");

        EntTransform transform;
        transform.position = M::Vector3::Zero;
        transform.scale = M::Vector3(1.0f, 1.0f, 1.0f);
        transform.rotation = M::Mat3::Identity();
        geom->SetTransform(transform);

        printf(">>>>>>>>>>>>>>>>>> BEGIN PUSHING BACK GEOM ENTITY\n");
        EV::Params_LinkEntity args;
        args.entity = geom;
        Send(EV::def_LinkEntity.Create(args));
        printf(">>>>>>>>>>>>>>>>>> END PUSHING BACK GEOM ENTITY\n");

        return geom;
    }

    IMesh* World::CreatePlaneMesh(const PlaneDesc& desc) 
    {
        R::Plane plane(desc);
        unsigned entId = SpawnMesh(R::meshMgr.Create(plane.GetDesc()));
        return new Proxy::Mesh(entId);
    }

    IMesh* World::CreateSphereMesh(const SphereDesc& desc) {
        R::Sphere sphere(desc.numSubdiv, desc.smooth);
        unsigned entId = SpawnMesh(R::meshMgr.Create(sphere.GetDesc()));
        return new Proxy::Mesh(entId);
    }

    IMesh* World::CreateCylinderMesh(const CylinderDesc& desc) {
        R::Cylinder cylinder(desc.radius, desc.height, desc.numSlices, desc.caps);
        unsigned entId = SpawnMesh(R::meshMgr.Create(cylinder.GetDesc()));
        return new Proxy::Mesh(entId);
    }


    void World::SelectGeometry(IGeometry* geom) { 
        _selection.geometry = geom; 
        OP::g_operators.SelectGeometry();
    }

    DWORD World::Thread_Func(void) {
        Polyhedron_InitResources();
        while(true) {
            Update();
            // Sleep(10);
        }
    }

} // namespace W
