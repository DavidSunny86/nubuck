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
        struct Desc {
            const Vertex*   vertices;
            int             numVertices;

            const Index*    indices;
            int             numIndices;

            GLenum          primType;
        };
        virtual ~MeshDesc(void) { }
        virtual Desc GetDesc(void) const = 0;
    };

    struct SimpleMeshDesc : MeshDesc {
        Desc desc;
        explicit SimpleMeshDesc(const Desc& desc) : desc(desc) { }
        ~SimpleMeshDesc(void) {
            if(desc.vertices) delete[] desc.vertices;
            if(desc.indices) delete[] desc.indices;
        }
        Desc GetDesc(void) const override { return desc; }
    };

    struct Instance {
        M::Vector3  position;
    };

    class Mesh {
    private:
        // NOTE: a normal pointer should be fine here,
        // since a mesh is only ever deleted by the mgr
        // if there are no external references to it
        GEN::Pointer<MeshDesc> _meshDesc;

        GEN::Pointer<StaticBuffer> _vertexBuffer;
        GEN::Pointer<StaticBuffer> _indexBuffer;
        bool _compiled;
    public:
        explicit Mesh(const GEN::Pointer<MeshDesc>& meshDesc);
        ~Mesh(void);

        void Bind(void) const;
        void Draw(void) const;

        void Invalidate(void);
        void Compile(void);
    };

} // namespace R
