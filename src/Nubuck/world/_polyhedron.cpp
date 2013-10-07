#include <world\world.h>
#include "polyhedron.h"

Polyhedron::Polyhedron(graph_t& G) {
	_entId = W::world.SpawnPolyhedron(&G);
}

void Polyhedron::Destroy(void) {
    W::Event event;
    event.type = W::EVENT_DESTROY_POLYHEDRON;
    W::EvArgs_DestroyPolyhedron* args = (W::EvArgs_DestroyPolyhedron*)event.args;
    args->entId = _entId;
    W::world.Send(event);
    _entId = 0;
}

void Polyhedron::SetRenderFlags(int flags) {
    W::Event event;
    event.type = W::EVENT_SET_RENDER_FLAGS;
    W::EvArgs_SetRenderFlags* args = (W::EvArgs_SetRenderFlags*)event.args;
    args->entId = _entId;
    args->flags = flags;
    W::world.Send(event);
}

void Polyhedron::SetPickable(bool isPickable) {
    W::Event event;
    event.type = W::EVENT_SET_PICKABLE;
    W::EvArgs_SetPickable* args = (W::EvArgs_SetPickable*)event.args;
    args->entId = _entId;
    args->isPickable = isPickable;
    W::world.Send(event);
}

void Polyhedron::SetNodeColor(leda::node node, float r, float g, float b) {
    W::Event event;
    event.type = W::EVENT_SET_NODE_COLOR;
    W::EvArgs_SetNodeColor* args = (W::EvArgs_SetNodeColor*)event.args;
    args->entId = _entId;
    args->node = node;
    args->color = R::Color(r, g, b, 1.0f);
    W::world.Send(event);
}

void Polyhedron::SetFaceColor(leda::edge edge, float r, float g, float b) {
    W::Event event;
    event.type = W::EVENT_SET_FACE_COLOR;
    W::EvArgs_SetFaceColor* args = (W::EvArgs_SetFaceColor*)event.args;
    args->entId = _entId;
    args->edge = edge;
    args->color = R::Color(r, g, b, 1.0f);
    W::world.Send(event);
}

void Polyhedron::Update(void) {
    W::Event event;
    event.type = W::EVENT_REBUILD;
    W::EvArgs_Rebuild* args = (W::EvArgs_Rebuild*)event.args;
    args->entId = _entId;
    W::world.Send(event);
}