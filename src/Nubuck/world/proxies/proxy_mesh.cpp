#include <events\event_defs.h>
#include <world\world.h>
#include "proxy_mesh.h"

namespace Proxy {

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