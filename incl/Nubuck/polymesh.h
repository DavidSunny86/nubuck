#pragma once

#include <Nubuck\renderer\color\color.h>

#include <LEDA\graph\graph.h>
#include <LEDA\graph\face_map.h>
#include <LEDA\geo\d3_rat_point.h>

namespace leda {
namespace nb {

template<typename VEC3>
class PolyMesh : public GRAPH<VEC3, int> {
private:
    typedef GRAPH<VEC3, int> base_t;

    struct FaceAttribs {
        int         visible;
        R::Color    color;

        FaceAttribs() : visible(true), color(R::Color::White) { }
    };

    leda::face_map<FaceAttribs> _faceAttribs;
public:
    PolyMesh();

    size_t FromObj(const char* filename);

    PolyMesh& operator=(const PolyMesh& other);

    void join(PolyMesh& other);

    const VEC3& position_of(const node v);

    bool            is_visible(const face f) const;
    const R::Color& color_of(const face f) const;

    void set_position(const node v, const VEC3& p);

    void set_color(const face f, const R::Color& color);

    edge make_triangle(const VEC3& p0, const VEC3& p1, const VEC3& p2);
};

typedef PolyMesh<d3_rat_point> RatPolyMesh;

void D3_HULL(list<d3_rat_point> L, RatPolyMesh& mesh);

template<typename VEC3>
void set_color(PolyMesh<VEC3>& mesh, const R::Color& color) {
    leda::face f;
    forall_faces(f, mesh) mesh.set_color(f, color);
}

template<typename VEC3>
inline PolyMesh<VEC3>::PolyMesh() {
    _faceAttribs.init(*this);
}

template<typename VEC3>
inline PolyMesh<VEC3>& PolyMesh<VEC3>::operator=(const PolyMesh& other) {
    if(&other != this) {
        base_t::operator=(other);
        _faceAttribs.init(*this);
    }
    return *this;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::join(PolyMesh& other) {
    if(&other != this) {
        base_t::join(other);
        compute_faces();
        _faceAttribs.init(*this);
    }
}

template<typename VEC3>
inline const VEC3& PolyMesh<VEC3>::position_of(const node v) {
    return LEDA_CONST_ACCESS(VEC3, entry(v));
}

template<typename VEC3>
inline bool PolyMesh<VEC3>::is_visible(const face f) const {
    return _faceAttribs[f].visible;
}

template<typename VEC3>
inline const R::Color& PolyMesh<VEC3>::color_of(const face f) const {
    return _faceAttribs[f].color;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_position(const node v, const VEC3& p) {
    LEDA_ACCESS(VEC3, entry(v)) = p;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const face f, const R::Color& color) {
    _faceAttribs[f].color = color;
}

template<typename VEC3>
edge PolyMesh<VEC3>::make_triangle(const VEC3& p0, const VEC3& p1, const VEC3& p2) {
    clear();

    node v[3];
    edge f[3];
    edge b[3];

    v[0] = new_node();
    v[1] = new_node();
    v[2] = new_node();

    set_position(v[0], p0);
    set_position(v[1], p1);
    set_position(v[2], p2);

    f[0] = new_edge(v[0], v[1]);
    b[0] = new_edge(v[1], v[0]);

    f[1] = new_edge(v[1], v[2]);
    b[1] = new_edge(v[2], v[1]);

    f[2] = new_edge(v[2], v[0]);
    b[2] = new_edge(v[0], v[2]);

    for(unsigned i = 0; i < 3; ++i) {
        set_reversal(f[i], b[i]);
    }

    compute_faces();

    _faceAttribs[face_of(f[0])].visible = true;
    _faceAttribs[face_of(b[0])].visible = false;

    return f[0];
}

struct OBJ_Face {
    size_t      face;
    unsigned    vertices[3];
};

template<typename VEC3>
struct FromFloat3 { };

template<> struct FromFloat3<leda::d3_rat_point> {
    static leda::d3_rat_point Conv(float x, float y, float z) {
        return leda::d3_rat_point(leda::rational(x), leda::rational(y), leda::rational(z));
    }
};

template<typename VEC3>
size_t PolyMesh<VEC3>::FromObj(const char* filename) {
    FILE* file = fopen(filename, "r");
    assert(file);

    std::vector<OBJ_Face> obj_faces;
    std::vector<node> verts;

    typedef std::pair<unsigned, unsigned> uedge_t; // edge described by pair of vertices, with e.first() < e.second()
    std::map<uedge_t, edge> edgemap; // maps pairs of vertices to edge handles

    unsigned numVertices = 0, numFaces = 0;
    int r = 0;
    while(!feof(file)) {
        char buffer[512];
        float f[3];
        unsigned d[3];

        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, 512, file);

        if(r = sscanf(buffer, "v %f %f %f", &f[0], &f[1], &f[2])) {
            node v = new_node();
            verts.push_back(v);
            float scale = 1.0f;
            set_position(v, FromFloat3<VEC3>::Conv(scale * f[0], scale * f[1], scale * f[2]));
            numVertices++;
        }
        if(r = sscanf(buffer, "f %d %d %d", &d[0], &d[1], &d[2])) {
            OBJ_Face f;
            f.vertices[0] = d[0] - 1;
            f.vertices[1] = d[1] - 1;
            f.vertices[2] = d[2] - 1;
            obj_faces.push_back(f);
            numFaces++;
        }
    }

    printf("NUM VERTICES: %d, NUM FACES %d\n", numVertices, numFaces);

    for(unsigned i = 0; i < numFaces; ++i) {
        OBJ_Face& f = obj_faces[i];
        for(unsigned j = 0; j < 3; ++j) {
            unsigned v0 = f.vertices[j];
            unsigned v1 = f.vertices[(j + 1) % 3];
            uedge_t uedge = std::make_pair(std::min(v0, v1), std::max(v0, v1));
            edge e = new_edge(verts[v0], verts[v1]);
            if(edgemap.end() == edgemap.find(uedge)) edgemap[uedge] = e;
            else set_reversal(edgemap[uedge], e);
        }
    }
    leda::list<edge> E;
    make_bidirected(E);
    printf("|E| = %d\n", E.size());
    make_planar_map();
    compute_faces();

    fclose(file);
    return 0;
}

} // namespace nb
} // namespace leda