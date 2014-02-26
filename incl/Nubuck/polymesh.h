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
    static const R::Color defaultVertexColor;

    typedef GRAPH<VEC3, int> base_t;

    // vertex attributes
    leda::node_map<float>   _vatt_colorR;
    leda::node_map<float>   _vatt_colorG;
    leda::node_map<float>   _vatt_colorB;

    // face attributes
    leda::face_map<int>     _fatt_visible;
    leda::face_map<float>   _fatt_colorR;
    leda::face_map<float>   _fatt_colorG;
    leda::face_map<float>   _fatt_colorB;

    void InitVertexAttributes();
    void InitFaceAttributes();
public:
    PolyMesh();

    size_t FromObj(const char* filename);

    PolyMesh& operator=(const PolyMesh& other);

    void join(PolyMesh& other);

    const VEC3&     position_of(const node v) const;
    const R::Color& color_of(const node v) const;

    bool            is_visible(const face f) const;
    const R::Color& color_of(const face f) const;

    void set_position(const node v, const VEC3& p);
    void set_color(const node v, const R::Color& color);

    void set_color(const face f, const R::Color& color);

    edge make_triangle(const VEC3& p0, const VEC3& p1, const VEC3& p2);
};

typedef PolyMesh<d3_rat_point> RatPolyMesh;

void D3_HULL(list<d3_rat_point> L, RatPolyMesh& mesh);

template<typename VEC3>
const R::Color PolyMesh<VEC3>::defaultVertexColor = R::Color(0.3f, 0.3f, 0.3f);

template<typename VEC3>
void set_color(PolyMesh<VEC3>& mesh, const R::Color& color) {
    leda::face f;
    forall_faces(f, mesh) mesh.set_color(f, color);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitVertexAttributes() {
    _vatt_colorR.init(*this, defaultVertexColor.r);
    _vatt_colorG.init(*this, defaultVertexColor.g);
    _vatt_colorB.init(*this, defaultVertexColor.b);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitFaceAttributes() {
    _fatt_visible.init(*this, 1);
    _fatt_colorR.init(*this, 1.0f);
    _fatt_colorG.init(*this, 1.0f);
    _fatt_colorB.init(*this, 1.0f);
}

template<typename VEC3>
inline PolyMesh<VEC3>::PolyMesh() {
    InitVertexAttributes();
    InitFaceAttributes();
}

template<typename VEC3>
inline PolyMesh<VEC3>& PolyMesh<VEC3>::operator=(const PolyMesh& other) {
    if(&other != this) {
        base_t::operator=(other);
        InitVertexAttributes();
        InitFaceAttributes();
    }
    return *this;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::join(PolyMesh& other) {
    if(&other != this) {
        base_t::join(other);
        compute_faces();
        InitVertexAttributes();
        InitFaceAttributes();
    }
}

template<typename VEC3>
inline const VEC3& PolyMesh<VEC3>::position_of(const node v) const {
    return LEDA_CONST_ACCESS(VEC3, entry(v));
}

template<typename VEC3>
inline const R::Color& PolyMesh<VEC3>::color_of(const node v) const {
    return R::Color(_vatt_colorR[v], _vatt_colorG[v], _vatt_colorB[v]);
}

template<typename VEC3>
inline bool PolyMesh<VEC3>::is_visible(const face f) const {
    return _fatt_visible[f];
}

template<typename VEC3>
inline const R::Color& PolyMesh<VEC3>::color_of(const face f) const {
    return R::Color(_fatt_colorR[f], _fatt_colorG[f], _fatt_colorB[f]);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_position(const node v, const VEC3& p) {
    LEDA_ACCESS(VEC3, entry(v)) = p;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const node v, const R::Color& color) {
    _vatt_colorR[v] = color.r;
    _vatt_colorG[v] = color.g;
    _vatt_colorB[v] = color.b;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const face f, const R::Color& color) {
    _fatt_colorR[f] = color.r;
    _fatt_colorG[f] = color.g;
    _fatt_colorB[f] = color.b;
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

    _fatt_visible[face_of(f[0])] = true;
    _fatt_visible[face_of(b[0])] = false;

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