#pragma once

#include <vector>

#include <Nubuck\renderer\color\color.h>

#include <Nubuck\common\common.h>

#include <Nubuck\generic\pointer.h>
#include <Nubuck\generic\uncopyable.h>

#include <renderer\glew\glew.h>
#include <Nubuck\math\vector3.h>
#include <Nubuck\math\vector2.h>
#include <Nubuck\math\matrix4.h>
#include <renderer\mesh\staticbuffer.h>
#include <Nubuck\system\locks\spinlock.h>

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
        M::Vector3  A[4];
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
        float       dist;
        float       viewAngle;
        TriIndices  bufIndices;
    };
private:
    SYS::SpinLock _mtx;

    std::vector<Vertex> _vertices;
    std::vector<Index>  _indices;
    std::vector<Index>  _offIndices;
    GLenum              _primType;

    M::Vector3 _center;

    void ComputeCenter();

    std::vector<TriIndices> _triangleIndices;

    bool     _invalidate;
    unsigned _gbHandle;
public:
    Mesh(const Desc& desc); // deep copy
    ~Mesh();

    const M::Vector3 GetLocalCenter() const;

    bool            IsCached() const;

    bool            IsSolid() const;
    GLenum      	PrimitiveType() const;
    unsigned    	NumIndices() const;
    const Index*    Indices() const;
    const Index*    OffIndices() const;

    const std::vector<Mesh::Vertex>& GetVertices() const { return _vertices; }

    void Invalidate(Mesh::Vertex* const vertices); // assumes number of vertices and indices is constant
    void Invalidate(Mesh::Vertex* const vertices, unsigned offset, unsigned size);
    void Invalidate(Mesh::Index* const indices, unsigned numIndices);

    // methods prefixed with R_ should only be called by the renderer
    void R_AppendTriangles(std::vector<Triangle>& tris, const M::Vector3& eye, unsigned idxOff); // eye in mesh local space
    unsigned R_IndexOff() const;
    void R_AllocBuffer();
    void R_TouchBuffer();
};

} // namespace R
