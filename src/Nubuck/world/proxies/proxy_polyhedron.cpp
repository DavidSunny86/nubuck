#include <events\event_defs.h>
#include <world\world.h>
#include "proxy_polyhedron.h"

namespace Proxy {

Polyhedron::Polyhedron(const graph_t& G) : _G(G) {
	_entId = W::world.SpawnPolyhedron(&_G);
}

void Polyhedron::Destroy(void) {
    EV::Params_DestroyEntity args;
    args.entId = _entId;
    W::world.Send(EV::def_DestroyEntity.Create(args));
    _entId = 0;
}

void Polyhedron::SetName(const std::string& name) {
    _name = name;
    EV::Params_SetName args;
    args.entId  = _entId;
    args.name   = _name.c_str();
    W::world.Send(EV::def_SetName.Create(args));
}

void Polyhedron::SetEffect(const char* fxName) {
    EV::Params_SetEffect args;
    args.entId = _entId;
    args.fxName = fxName;
    W::world.Send(EV::def_SetEffect.Create(args));
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

void Polyhedron::SetEdgeColor(leda::edge edge, float r, float g, float b) {
    EV::Params_SetEdgeColor args;
    args.entId = _entId;
    args.edge = edge;
    args.color = R::Color(r, g, b, 1.0f);
    W::world.Send(EV::def_SetEdgeColor.Create(args));
}

void Polyhedron::SetFaceColor(leda::edge edge, float r, float g, float b) {
    SetFaceColor(edge, r, g, b, 1.0f);
}

void Polyhedron::SetFaceColor(leda::edge edge, float r, float g, float b, float a) {
    EV::Params_SetFaceColor args;
    args.entId = _entId;
    args.edge = edge;
    args.color = R::Color(r, g, b, a);
    W::world.Send(EV::def_SetFaceColor.Create(args));
}

void Polyhedron::HideFace(leda::edge edge) {
    EV::Params_HideFace args;
    args.entId = _entId;
    args.edge = edge;
    W::world.Send(EV::def_HideFace.Create(args));
}

void Polyhedron::Update(void) {
    EV::Params_Rebuild args;
    args.entId = _entId;
    W::world.Send(EV::def_Rebuild.Create(args));
}

} // namespace Proxy