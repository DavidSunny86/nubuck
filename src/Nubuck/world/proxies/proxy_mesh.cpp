#include <events\event_defs.h>
#include <world\world.h>
#include "proxy_mesh.h"

namespace Proxy {

    void Mesh::SetPosition(float x, float y, float z) {
        EV::Params_SetPosition args;
        args.entId = _entId;
        args.pos = M::Vector3(x, y, z);
        W::world.Send(EV::def_SetPosition.Create(args));
    }

    void Mesh::SetScale(float sx, float sy, float sz) {
        EV::Params_SetScale args;
        args.entId = _entId;
        args.sx = sx;
        args.sy = sy;
        args.sz = sz;
        W::world.Send(EV::def_SetScale.Create(args));
    }

    void Mesh::SetEffect(const char* fxName) {
        EV::Params_SetEffect args;
        args.entId = _entId;
        args.fxName = fxName;
        W::world.Send(EV::def_SetEffect.Create(args));
    }

    void Mesh::SetVisible(bool isVisible) {
        EV::Params_SetVisible args;
        args.entId = _entId;
        args.isVisible = isVisible;
        W::world.Send(EV::def_SetVisible.Create(args));
    }

} // namespace Proxy