#pragma once

#include "..\mesh.h"

namespace R {

    Mesh::Desc CreateQuadDesc(
        float edgeLength = 1.0f,
        const M::Vector2& texCoords_lowerLeft = M::Vector2(0.0f, 0.0f),
        const M::Vector2& texCoords_upperRight = M::Vector2(1.0, 1.0f));

} // namespace R
