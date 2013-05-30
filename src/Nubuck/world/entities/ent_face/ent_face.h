#pragma once

#include <world\entity.h>

namespace W {

    class ENT_Face : public Entity {
    public:
        void Spawn(const Event& event);
    };

} // namespace W