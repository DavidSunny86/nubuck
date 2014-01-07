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
        size_t prev;
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

    HalfEdge& GetHalfEdge(const size_t idx) { return _edges[idx >> 1].halfEdges[idx & 1]; }
public:
    size_t      V_MapSize() const                               { return _vertices.MapSize(); }
    size_t      V_Begin() const                                 { return _vertices.Begin(); }
    size_t  	V_End() const                                   { return _vertices.End(); }
    size_t  	V_Next(size_t idx) const                        { return _vertices.Next(idx); }
    void        V_SetPosition(size_t idx, const VEC3& position) { _vertices[idx].position = position; }
    const VEC3& V_GetPosition(size_t idx) const                 { return _vertices[idx].position; }

    size_t E_Begin() const                  { return _edges.Begin(); }
    size_t E_End() const                	{ return _edges.End(); }
    size_t E_Next(size_t idx) const     	{ return _edges.Next(idx); }
    size_t E_StartVertex(size_t idx) const  { return _edges[idx].halfEdges[0].vert; }
    size_t E_EndVertex(size_t idx) const    { return _edges[idx].halfEdges[1].vert; }

    size_t F_Begin() const              { return _faces.Begin(); }
    size_t F_End() const                { return _faces.End(); }
    size_t F_Next(size_t idx) const     { return _faces.Next(idx); }
    size_t F_HalfEdge(size_t idx) const { return _faces[idx].halfEdge; }

    size_t H_Begin() const;
    size_t H_End() const;
    size_t H_Next(size_t idx) const;
    size_t H_Reversal(size_t idx) const     { return idx ^ 1; }
    size_t H_FaceSucc(size_t idx) const     { return _edges[idx >> 1].halfEdges[idx & 1].next; }
    size_t H_FacePred(size_t idx) const     { return _edges[idx >> 1].halfEdges[idx & 1].prev; }
    size_t H_StartVertex(size_t idx) const  { return _edges[idx >> 1].halfEdges[idx & 1].vert; }
    size_t H_EndVertex(size_t idx) const    { return H_StartVertex(H_Reversal(idx)); }
    size_t H_StartCCW(size_t idx) const     { return H_Reversal(H_FacePred(idx)); }
    size_t H_StartCW(size_t idx) const      { return H_FaceSucc(H_Reversal(idx)); }
    
    size_t MakeTetrahedron(const VEC3& p0, const VEC3& p1, const VEC3& p2, const VEC3& p3);

    size_t SplitVertex(size_t he0, size_t he1);
    size_t SplitFace(size_t h0, size_t h1);
    size_t SplitHalfEdge(size_t h);
};

typedef PolyMesh<leda::d3_rat_point> RatPolyMesh;

template<typename VEC3>
size_t PolyMesh_H_ComputePrev(const PolyMesh<VEC3>& pm, size_t idx) {
    size_t it = idx;
    while(idx != pm.H_FaceSucc(it)) it = pm.H_FaceSucc(it);
    return it;
}

template<typename VEC3>
void PolyMesh_H_CheckPrev(const PolyMesh<VEC3>& pm) {
    bool valid = true;
    size_t it = pm.H_Begin();
    while(pm.H_End() != it) {
        if(PolyMesh_H_ComputePrev(pm, it) != pm.H_FacePred(it)) {
            printf("ASSERTION FAILED: half edge %d: prev = %d != %d = compute_prev\n",
                it, pm.H_FacePred(it), PolyMesh_H_ComputePrev(pm, it));
            valid = false;
        }
        it = pm.H_Next(it);
    }
    assert(valid);
}

template<typename VEC3>
size_t PolyMesh<VEC3>::MakeTetrahedron(const VEC3& p0, const VEC3& p1, const VEC3& p2, const VEC3& p3) {
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
    _edges[0].halfEdges[0].prev = 10;
    _edges[0].halfEdges[1].vert = 1;
    _edges[0].halfEdges[1].next = 5;
    _edges[0].halfEdges[1].prev = 2;

    _edges[1].halfEdges[0].vert = 2;
    _edges[1].halfEdges[0].next = 1;
    _edges[1].halfEdges[0].prev = 5;
    _edges[1].halfEdges[1].vert = 1;
    _edges[1].halfEdges[1].next = 7;
    _edges[1].halfEdges[1].prev = 8;

    _edges[2].halfEdges[0].vert = 2;
    _edges[2].halfEdges[0].next = 11;
    _edges[2].halfEdges[0].prev = 6;
    _edges[2].halfEdges[1].vert = 0;
    _edges[2].halfEdges[1].next = 2;
    _edges[2].halfEdges[1].prev = 1;

    _edges[3].halfEdges[0].vert = 3;
    _edges[3].halfEdges[0].next = 4;
    _edges[3].halfEdges[0].prev = 11;
    _edges[3].halfEdges[1].vert = 2;
    _edges[3].halfEdges[1].next = 8;
    _edges[3].halfEdges[1].prev = 3;

    _edges[4].halfEdges[0].vert = 3;
    _edges[4].halfEdges[0].next = 3;
    _edges[4].halfEdges[0].prev = 7;
    _edges[4].halfEdges[1].vert = 1;
    _edges[4].halfEdges[1].next = 10;
    _edges[4].halfEdges[1].prev = 0;

    _edges[5].halfEdges[0].vert = 3;
    _edges[5].halfEdges[0].next = 0;
    _edges[5].halfEdges[0].prev = 9;
    _edges[5].halfEdges[1].vert = 0;
    _edges[5].halfEdges[1].next = 6;
    _edges[5].halfEdges[1].prev = 4;

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
    PolyMesh_H_CheckPrev(*this);

    size_t it = V_Begin();
    while(V_End() != it) {
        printf("it = %d\n", it);
        it = V_Next(it);
    }

    it = H_Begin();
    while(H_End() != it) {
        printf("halfedge it = %d\n", it);
        it = H_Next(it);
    }

    return 0;
}

template<typename VEC3>
size_t PolyMesh<VEC3>::SplitVertex(size_t he0, size_t he1) {
    size_t w = _vertices.Alloc();
    size_t e = _edges.Alloc();

    size_t n0 = e << 1 | 0;
    size_t n1 = e << 1 | 1;

    HalfEdge& N0 = GetHalfEdge(n0);
    HalfEdge& N1 = GetHalfEdge(n1);
    HalfEdge& H0 = GetHalfEdge(he0);
    HalfEdge& H1 = GetHalfEdge(he1);

    N0.vert = w;
    N1.vert = H_StartVertex(he0);

    size_t it = H_StartCW(he0);
    while(he1 != it) {
        GetHalfEdge(it).vert = w;
        it = H_StartCW(it);
    }

    N0.prev = H1.prev;
    N0.next = he1;
    GetHalfEdge(H1.prev).next = n0;
    H1.prev = n0;

    N1.next = H_FaceSucc(H_Reversal(he0));
    N1.prev = H_Reversal(he0);
    GetHalfEdge(H_FaceSucc(H_Reversal(he0))).prev = n1;
    GetHalfEdge(H_Reversal(he0)).next = n1;

    PolyMesh_H_CheckPrev(*this);

    return n0;
}

template<typename VEC3>
size_t PolyMesh<VEC3>::SplitFace(size_t h0, size_t h1) {
    size_t e = _edges.Alloc();
    size_t n0 = e << 1 | 0;
    size_t n1 = e << 1 | 1;
    size_t p0 = H_FacePred(h0);
	size_t p1 = H_FacePred(h1);

    HalfEdge& N0 = GetHalfEdge(n0);
    HalfEdge& N1 = GetHalfEdge(n1);
    HalfEdge& H0 = GetHalfEdge(h0);
    HalfEdge& H1 = GetHalfEdge(h1);
    HalfEdge& P0 = GetHalfEdge(p0);
    HalfEdge& P1 = GetHalfEdge(p1);

    N0.vert = H_StartVertex(h1);
    N1.vert = H_StartVertex(h0);

    N0.next = h0;
    N0.prev = p1;
    N1.next = h1;
    N1.prev = p0;
    H0.prev = n0;
    H1.prev = n1;
    P0.next = n1;
    P1.next = n0;

    size_t f = _faces.Alloc();
    _faces[f].halfEdge = n0;

    PolyMesh_H_CheckPrev(*this);

    return n1;
}

template<typename VEC3>
size_t PolyMesh<VEC3>::SplitHalfEdge(size_t h) {
    size_t h0 = H_StartCCW(h);
    size_t h1 = H_StartCW(h);
    return SplitVertex(h0, h1);
}

// ==================================================
// HALF EDGE OPERATIONS
// ==================================================

// ITERATION

template<typename VEC3>
inline size_t PolyMesh<VEC3>::H_Begin() const {
    return _edges.Begin() << 1;
}

template<typename VEC3>
inline size_t PolyMesh<VEC3>::H_End() const {
    return _edges.End() << 1;
}

template<typename VEC3>
inline size_t PolyMesh<VEC3>::H_Next(size_t idx) const {
    if(idx & 1) return _edges.Next(idx >> 1) << 1;
    else return idx | 1;
}

} // namespace NB