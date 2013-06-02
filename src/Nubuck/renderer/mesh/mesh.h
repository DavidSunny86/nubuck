#pragma once

#include <renderer\glew\glew.h>
#include <math\vector3.h>
#include <math\vector2.h>
#include <renderer\color\color.h>

namespace R {

    struct Vertex {
        M::Vector3  position;
        M::Vector3  normal;
        Color       color;
        M::Vector2  texCoords;
    };

    typedef unsigned Index;

#define RESTART_INDEX (unsigned)0xFFFFFFFF

    struct MeshDesc {
        Vertex* vertices;
        int     numVertices;

        Index*  indices;
        int     numIndices;

        GLenum  primType;
    };

} // namespace R
