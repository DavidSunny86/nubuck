#include <common\common.h>
#include <events\event_defs.h>
#include <world\world.h>
#include "entity.h"

namespace W {

void Entity::Destroy() {
    EV::Params_DestroyEntity args;
    args.entId = GetID();
    W::world.Send(EV::def_DestroyEntity.Create(args));
}

} // namespace W