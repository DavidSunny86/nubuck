#pragma once

#include <world\entity.h>

namespace W {

    class ENT_Face : public Entity {
    private:
        Mesh _mesh;
    public:
        void Spawn(const Event& event);

        void Render(std::vector<R::RenderJob>& renderList) override;
    };

} // namespace W