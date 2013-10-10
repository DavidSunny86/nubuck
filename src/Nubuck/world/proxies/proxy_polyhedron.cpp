#include <world\world.h>
#include "proxy_polyhedron.h"

namespace Proxy {

Polyhedron::Polyhedron(graph_t& G) {
	_entId = W::world.SpawnPolyhedron(&G);
}

void Polyhedron::Destroy(void) {
    EV::Params_DestroyPolyhedron args;
    args.entId = _entId;
    W::world.Send(EV::def_DestroyPolyhedron.Create(args));
    _entId = 0;
}

void Polyhedron::SetRenderFlags(int flags) {
    EV::Params_SetRenderFlags args;
    args.entId = _entId;
    args.flags = flags;
    W::world.Send(EV::def_SetRenderFlags.Create(args));
}

void Polyhedron::SetPickable(bool isPickable) {
    EV::Params_SetPickable args;
    args.entId = _entId;
    args.isPickable = isPickable;
    W::world.Send(EV::def_SetPickable.Create(args));
}

void Polyhedron::SetNodeColor(leda::node node, float r, float g, float b) {
    EV::Params_SetNodeColor args;
    args.entId = _entId;
    args.node = node;
    args.color = R::Color(r, g, b, 1.0f);
    W::world.Send(EV::def_SetNodeColor.Create(args));
}

void Polyhedron::SetFaceColor(leda::edge edge, float r, float g, float b) {
    EV::Params_SetFaceColor args;
    args.entId = _entId;
    args.edge = edge;
    args.color = R::Color(r, g, b, 1.0f);
    W::world.Send(EV::def_SetFaceColor.Create(args));
}

void Polyhedron::Update(void) {
    EV::Params_Rebuild args;
    args.entId = _entId;
    W::world.Send(EV::def_Rebuild.Create(args));
}

} // namespace Proxy