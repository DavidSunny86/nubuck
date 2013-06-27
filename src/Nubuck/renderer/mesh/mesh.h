#pragma once

#include <generic\pointer.h>
#include <generic\uncopyable.h>

#include <renderer\glew\glew.h>
#include <math\vector3.h>
#include <math\vector2.h>
#include <renderer\color\color.h>
#include <renderer\mesh\staticbuffer.h>
#include <system\locks\spinlock.h>

#ifndef NULL
#define NULL 0
#endif

namespace R {

    class Mesh : private GEN::Uncopyable {
    public:
        struct Vertex {
            M::Vector3  position;
            M::Vector3  normal;
            Color       color;
            M::Vector2  texCoords;
        };

        typedef unsigned Index;

        enum { RESTART_INDEX = 0xFFFFFFFFu };

        struct Desc {
            Vertex* vertices;
            unsigned numVertices;

            Index* indices;
            unsigned numIndices;

            GLenum primType;

            Desc(void) : vertices(NULL), numVertices(0), indices(NULL), numIndices(0), primType(0) { }
        };
    private:
        Desc _desc;

        GEN::Pointer<StaticBuffer> _vertexBuffer;
        GEN::Pointer<StaticBuffer> _indexBuffer;
        bool _compiled;
    public:
        struct MgrLink { // maintained my MeshMgr
            Mesh *prev, *next;
            unsigned refCount;
            SYS::SpinLock mtx; // locks ref count
        } mgrLink;

        Mesh(const Desc& desc); // deep copy
        ~Mesh(void);

        unsigned NumIndices(void) const { return _desc.numIndices; }
        GLenum PrimitiveType(void) const { return _desc.primType; }

        void R_Compile(void);
        void R_Bind(void);
    };

} // namespace R
