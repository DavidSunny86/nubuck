#pragma once

#include "world.h"

namespace {

    template<typename TYPE>
    W::Entity* AllocateEntity(void) {
        return new TYPE();
    }

} // unnamed namespace

namespace W {

    template<typename TYPE>
    void World::RegisterEntity(EntityType type) {
        _entityAllocs[type] = AllocateEntity<TYPE>;
    }

} // namespace W