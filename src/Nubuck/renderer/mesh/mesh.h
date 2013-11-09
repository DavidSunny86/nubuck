#pragma once

#include <vector>

#include <common\common.h>

#include <generic\pointer.h>
#include <generic\uncopyable.h>

#include <renderer\glew\glew.h>
#include <math\vector3.h>
#include <math\vector2.h>
#include <math\matrix4.h>
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

    struct TriIndices {
        Mesh::Index indices[3];
    };

    struct Triangle {
        float dist;
        bool visible;
        TriIndices bufIndices;
    };
private:
    SYS::SpinLock _mtx;

    Desc    _desc;
    bool    _compiled;

    M::Matrix4  _transform;
    Vertex*     _tfverts;   // transformed vertices

    std::vector<TriIndices> _triangleIndices;

    unsigned _gbHandle;
public:
    Mesh(const Desc& desc); // deep copy
    ~Mesh(void);

    void AppendTriangles(std::vector<Triangle>& tris, const M::Vector3& eye, const M::Matrix4& worldMat); // eye in mesh local space

    // assumes number of vertices and indices is constant
    void Invalidate(Mesh::Vertex* const vertices);
    void Transform(const M::Matrix4& mat);

    // methods prefixed with R_ should only be called by the renderer
    void R_Touch(void);
    void R_Compile(void);
};

} // namespace R
