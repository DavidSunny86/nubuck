#include <world\world.h>
#include <world\entities\ent_node\ent_node.h>
#include <world\entities\ent_face\ent_face.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "polyhedron.h"

Polyhedron::Polyhedron(graph_t& G) : _G(G), _rebuildSem(0) {
    _nodeEntIDs.init(_G);
    _faceEntIDs.init(_G, 0);

    W::World& world = W::world;
    W::Event event;
    event.sem = NULL;

    event.type = W::ENT_NODE;
    leda::node n = NULL;
    forall_nodes(n, G) {
        W::ENT_Node::SpawnArgs* spawnArgs = (W::ENT_Node::SpawnArgs*)event.args;
        spawnArgs->G    = &_G;
        spawnArgs->node = n;

        _nodeEntIDs[n] = world.Spawn(event);
    }

    event.type = W::ENT_POLYHEDRON;
    W::ENT_Polyhedron::SpawnArgs* spawnArgs = (W::ENT_Polyhedron::SpawnArgs*)event.args;
    spawnArgs->G = &_G;
    _hullID = world.Spawn(event);
}

void Polyhedron::Destroy(void) {
    W::Event event;
    event.id = W::EVENT_DESTROY_ENTITY;
    event.sem = NULL;

    // destroy nodes
    leda::node n;
    forall_nodes(n, _G) {
        event.entityId = _nodeEntIDs[n];
        if(0 < event.entityId) {
            _nodeEntIDs[n] = 0;
            W::world.Send(event);
        }
    }

    // destroy faces
    leda::edge e;
    forall_edges(e, _G) {
        event.entityId = _faceEntIDs[e];
        if(0 < event.entityId) {
            _faceEntIDs[e] = 0;
            W::world.Send(event);
        }
    }

    // destroy hull
    event.entityId = _hullID;
    W::world.Send(event);
}

void Polyhedron::SetNodePosition(leda::node node, const point_t& position) {
    _G[node] = position;

    W::Event event;
    event.id = W::EVENT_UPDATE;
    event.sem = NULL;
    event.entityId = _nodeEntIDs[node];
    W::world.Send(event);
}

void Polyhedron::SetNodeColor(leda::node node, float r, float g, float b) {
    W::Event event;

    event.id        = W::EVENT_CHANGE_COLOR;
    event.entityId  = _nodeEntIDs[node];
    event.sem       = NULL;

    W::ChangeColorArgs* args = (W::ChangeColorArgs*)event.args;
    args->mode = W::ChangeColorArgs::MODE_LERP;
    args->r = r;
    args->g = g;
    args->b = b;

    W::world.Send(event);
}

void Polyhedron::SetFaceColorSolid(leda::edge edge, float r, float g, float b) {
    W::Event event;

    event.id        = W::EVENT_CHANGE_COLOR;
    event.entityId  = _hullID;
    event.sem       = NULL;

    W::ChangeColorArgs* args = (W::ChangeColorArgs*)event.args;
    args->mode = W::ChangeColorArgs::MODE_LERP;
    args->r     = r;
    args->g     = g;
    args->b     = b;
    args->edge  = edge;

    W::world.Send(event);
}

void Polyhedron::SetFaceColorRing(leda::edge edge, float r, float g, float b) {
    // if(0 < _faceEntIDs[edge]) return;

    W::Event event;

    event.id    = W::EVENT_SPAWN_ENTITY;
    event.type  = W::ENT_FACE;
    event.sem   = &_rebuildSem;

    W::ENT_Face::SpawnArgs* args = (W::ENT_Face::SpawnArgs*)event.args;
    args->G     = &_G;
    args->edge  = edge;
    args->r     = r;
    args->g     = g;
    args->b     = b;

    int id = W::world.Spawn(event);

    leda::edge it = edge;
    do {
        _faceEntIDs[it] = id;
    } while(edge != (it = _G.face_cycle_succ(it)));
}

void Polyhedron::SetFaceColor(leda::edge edge, float r, float g, float b) {
    SetFaceColorRing(edge, r, g, b);
}

void Polyhedron::Update(void) {
    W::Event event;

    // destroy faces
    event.id = W::EVENT_DESTROY_ENTITY;
    event.sem = NULL;
    leda::edge e;
    forall_edges(e, _G) {
        event.entityId = _faceEntIDs[e];
        if(0 < event.entityId) {
            _faceEntIDs[e] = 0;
            W::world.Send(event);
        }
    }

    // update hull
    event.id = W::EVENT_REBUILD;
    event.entityId = _hullID;
    event.sem = &_rebuildSem;
    W::world.Send(event);
}