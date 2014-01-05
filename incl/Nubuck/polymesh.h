#pragma once

#include <vector>
#include <LEDA\geo\d3_rat_point.h>

namespace NB {

template<typename TYPE>
class ObjectPool {
private:
    enum { INVALID_IDX = (size_t)-1 };

    std::vector<TYPE>   _objs;
    size_t              _used;
    size_t              _free;
    size_t              _rend;

    ObjectPool(const ObjectPool& other);
    ObjectPool& operator=(const ObjectPool& other);
public:
    ObjectPool() : _objs(), _used(INVALID_IDX), _free(INVALID_IDX), _rend(0) { }

    const TYPE& operator[](const size_t idx) const  { return _objs[idx]; }
    TYPE&       operator[](const size_t idx)        { return _objs[idx]; }

    size_t      Alloc();
    void        Free(size_t idx);

    size_t      MapSize() const                 { return _rend; }
    size_t      Begin() const                   { return _used; }
    size_t      End() const                     { return INVALID_IDX; }
    size_t      Next(const size_t idx) const    { return _objs[idx].next; }
};

template<typename TYPE>
inline size_t ObjectPool<TYPE>::Alloc() {
    if(INVALID_IDX != _free) {
        TYPE& obj = _objs[_free];
        size_t tmp = obj.next;
        obj.next = _used;
        _used = _free;
        _free = tmp;
        return _used;
    }

    _objs.resize(_rend + 1);

    TYPE& obj = _objs[_rend];
    obj.next = _used;
    _used = _rend;
    return _rend++;
}

template<typename TYPE>
inline void ObjectPool<TYPE>::Free(size_t idx) {
    TYPE& obj = _objs[idx];
    _used = obj.next;
    obj.next = _free;
    _free = idx;
}

template<typename VEC3>
class PolyMesh {
private:
    struct Vertex {
        size_t  next;
        size_t  halfEdge;
        VEC3    position;
    };

    struct HalfEdge {
        size_t vert;
        size_t next;
    };

    struct Edge {
        size_t      next;
        HalfEdge    halfEdges[2];
    };

    struct Face {
        size_t next; 
        size_t halfEdge;
    };

    ObjectPool<Vertex>  _vertices;
    ObjectPool<Edge>    _edges;
    ObjectPool<Face>    _faces;
public:
    size_t      V_MapSize() const                               { return _vertices.MapSize(); }
    size_t      V_Begin() const                                 { return _vertices.Begin(); }
    size_t  	V_End() const                                   { return _vertices.End(); }
    size_t  	V_Next(size_t idx) const                        { return _vertices.Next(idx); }
    void        V_SetPosition(size_t idx, const VEC3& position) { _vertices[idx].position = position; }
    const VEC3& V_GetPosition(size_t idx) const                 { return _vertices[idx].position; }

    size_t F_Begin() const              { return _faces.Begin(); }
    size_t F_End() const                { return _faces.End(); }
    size_t F_Next(size_t idx) const     { return _faces.Next(idx); }
    size_t F_HalfEdge(size_t idx) const { return _faces[idx].halfEdge; }

    size_t H_Reversal(size_t idx) const     { return idx ^ 1; }
    size_t H_FaceSucc(size_t idx) const     { return _edges[idx >> 1].halfEdges[idx & 1].next; }
    size_t H_StartVertex(size_t idx) const  { return _edges[idx >> 1].halfEdges[idx & 1].vert; }
    size_t H_EndVertex(size_t idx) const    { return H_StartVertex(H_Reversal(idx)); }
    
    void MakeTetrahedron(const VEC3& p0, const VEC3& p1, const VEC3& p2, const VEC3& p3);
};

typedef PolyMesh<leda::d3_rat_point> RatPolyMesh;

template<typename VEC3>
void PolyMesh<VEC3>::MakeTetrahedron(const VEC3& p0, const VEC3& p1, const VEC3& p2, const VEC3& p3) {
    size_t v0 = _vertices.Alloc();
    size_t v1 = _vertices.Alloc();
    size_t v2 = _vertices.Alloc();
    size_t v3 = _vertices.Alloc();

    V_SetPosition(v0, p0);
    V_SetPosition(v1, p1);
    V_SetPosition(v2, p2);
    V_SetPosition(v3, p3);

    for(unsigned i = 0; i < 6; ++i) _edges.Alloc();

    _edges[0].halfEdges[0].vert = 0;
    _edges[0].halfEdges[0].next = 9;
    _edges[0].halfEdges[1].vert = 1;
    _edges[0].halfEdges[1].next = 5;

    _edges[1].halfEdges[0].vert = 2;
    _edges[1].halfEdges[0].next = 1;
    _edges[1].halfEdges[1].vert = 1;
    _edges[1].halfEdges[1].next = 7;

    _edges[2].halfEdges[0].vert = 2;
    _edges[2].halfEdges[0].next = 11;
    _edges[2].halfEdges[1].vert = 0;
    _edges[2].halfEdges[1].next = 2;

    _edges[3].halfEdges[0].vert = 3;
    _edges[3].halfEdges[0].next = 4;
    _edges[3].halfEdges[1].vert = 2;
    _edges[3].halfEdges[1].next = 8;

    _edges[4].halfEdges[0].vert = 3;
    _edges[4].halfEdges[0].next = 3;
    _edges[4].halfEdges[1].vert = 1;
    _edges[4].halfEdges[1].next = 10;

    _edges[5].halfEdges[0].vert = 3;
    _edges[5].halfEdges[0].next = 0;
    _edges[5].halfEdges[1].vert = 0;
    _edges[5].halfEdges[1].next = 6;

    for(unsigned i = 0; i < 4; ++i) _faces.Alloc();

    _faces[0].halfEdge = 0;
    _faces[1].halfEdge = 3;
    _faces[2].halfEdge = 4;
    _faces[3].halfEdge = 2;

    // removeme
    assert(H_FaceSucc(0) == 9);
    assert(H_FaceSucc(9) == 10);
    assert(H_StartVertex(5) == 0);
    assert(H_EndVertex(5) == 2);

    size_t it = V_Begin();
    while(V_End() != it) {
        printf("it = %d\n", it);
        it = V_Next(it);
    }
}

} // namespace NB