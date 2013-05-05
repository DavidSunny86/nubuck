#pragma once

#include <generic\pointer.h>
#include <math\vector3.h>
#include <renderer\glew\glew.h>
#include <renderer\color\color.h>

namespace R {

    class Program;
    class StaticBuffer;

    struct Vertex {
        M::Vector3  position;
        M::Vector3  normal;
        Color       color;
    };

    typedef unsigned Index;

#define RESTART_INDEX (unsigned)0xFFFFFFFF

    struct MeshDesc {
        const Vertex*   vertices;
        int             numVertices;

        const Index*    indices;
        int             numIndices;

        GLenum          primType;
    };

    struct Instance {
        M::Vector3  position;
    };

    class Mesh {
    private:
        MeshDesc _desc;

        GEN::Pointer<StaticBuffer> _vertexBuffer;
        GEN::Pointer<StaticBuffer> _indexBuffer;
        bool _compiled;
    public:
        explicit Mesh(const MeshDesc& desc);
        ~Mesh(void);

        void Bind(void) const;
        void Draw(void) const;

        void Invalidate(void);
        void Compile(void);
    };

} // namespace R
