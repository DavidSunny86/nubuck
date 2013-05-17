#include <world\world.h>
#include <world\entities\ent_node\ent_node.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "polyhedron.h"

Polyhedron::Polyhedron(const graph_t& G) : _G(G), _rebuildSem(0) {
    _nodeEntIDs.init(_G);

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

void Polyhedron::Update(void) {
    W::Event event;

    event.id = W::EVENT_REBUILD;
    event.entityId = _hullID;
    event.sem = &_rebuildSem;

    W::world.Send(event);
}