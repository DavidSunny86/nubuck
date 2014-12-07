#pragma once

#include <Nubuck\renderer\color\color.h>
#include <Nubuck\math\math.h>

#include <LEDA\graph\graph.h>
#include <LEDA\graph\face_map.h>
#include <LEDA\geo\d3_rat_point.h>

namespace leda {
namespace nb {

template<typename VEC3>
class PolyMesh : public GRAPH<VEC3, int> {
public:
    struct State {
        enum {
            CACHED = 0,
            GEOMETRY_CHANGED,
            TOPOLOGY_CHANGED,
        };
    };
private:
    static const R::Color   defaultVertexColor;
    static const float      defaultVertexRadius;
    static const R::Color 	defaultEdgeColor;
    static const float      defaultEdgeRadius;

    typedef GRAPH<VEC3, int> base_t;

    // vertex attributes
    leda::node_map<char>    _vatt_state;
    leda::node_map<float>   _vatt_colorR;
    leda::node_map<float>   _vatt_colorG;
    leda::node_map<float>   _vatt_colorB;
    leda::node_map<float>   _vatt_radius;

    // edge attributes
    leda::edge_map<char>    _eatt_state;
    leda::edge_map<int>     _eatt_mask;
    leda::edge_map<float>   _eatt_colorR;
    leda::edge_map<float>   _eatt_colorG;
    leda::edge_map<float>   _eatt_colorB;
    leda::edge_map<float>   _eatt_radius;

    // face attributes
    leda::face_map<char>    _fatt_state;
    leda::face_map<int>     _fatt_visible;
    leda::face_map<float>   _fatt_colorR;
    leda::face_map<float>   _fatt_colorG;
    leda::face_map<float>   _fatt_colorB;
    leda::face_map<float>   _fatt_colorA;
    leda::face_map<float>   _fatt_patternR;
    leda::face_map<float>   _fatt_patternG;
    leda::face_map<float>   _fatt_patternB;
    leda::face_map<float>   _fatt_patternA;

    void InitVertexAttributes();
    void InitEdgeAttributes();
    void InitFaceAttributes();
public:
    PolyMesh();

    // called by ENT_Geometry only!
    int     state_of(node v) const;
    int     state_of(edge e) const;
    int     state_of(face f) const;
    void    cache_all();

    size_t FromObj(const char* filename);

    PolyMesh& operator=(const PolyMesh& other);

    void force_rebuild();

    void join(PolyMesh& other);

    edge split(const edge e);
    edge split(const edge e0, const edge e1);

    const VEC3&     position_of(const node v) const;
    const float     radius_of(const node v) const;
    R::Color        color_of(const node v) const;

    const float     radius_of(const edge e) const;
    R::Color        color_of(const edge e) const;
    int             is_masked(const edge e) const;

    bool            is_visible(const face f) const;
    R::Color        color_of(const face f) const;
    R::Color        pattern_of(const face f) const;

    void set_position(const node v, const VEC3& p);
    void set_radius(const node v, const float radius);
    void set_color(const node v, const R::Color& color);

    void set_radius(const edge e, const float radius);
    void set_color(const edge e, const R::Color& color);
    void set_masked(const edge e);
    void set_unmasked(const edge e);

    void set_visible(const face f, bool visible);
    void set_color(const face f, const R::Color& color);
    void set_pattern(const face f, const R::Color& color);

    edge make_triangle(const VEC3& p0, const VEC3& p1, const VEC3& p2);
};

typedef PolyMesh<d3_rat_point> RatPolyMesh;

void D3_HULL(list<d3_rat_point> L, RatPolyMesh& mesh);

template<typename VEC3>
const R::Color PolyMesh<VEC3>::defaultVertexColor = R::Color(0.3f, 0.3f, 0.3f);

template<typename VEC3>
const float PolyMesh<VEC3>::defaultVertexRadius = 0.02f;

template<typename VEC3>
const R::Color PolyMesh<VEC3>::defaultEdgeColor = R::Color(0.3f, 0.3f, 0.3f);

template<typename VEC3>
const float PolyMesh<VEC3>::defaultEdgeRadius = 0.01f;

template<typename VEC3>
void set_color(PolyMesh<VEC3>& mesh, const R::Color& color) {
    leda::face f;
    forall_faces(f, mesh) mesh.set_color(f, color);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitVertexAttributes() {
    _vatt_state.init(*this, State::TOPOLOGY_CHANGED);
    _vatt_colorR.init(*this, defaultVertexColor.r);
    _vatt_colorG.init(*this, defaultVertexColor.g);
    _vatt_colorB.init(*this, defaultVertexColor.b);
    _vatt_radius.init(*this, defaultVertexRadius);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitEdgeAttributes() {
    _eatt_state.init(*this, State::TOPOLOGY_CHANGED);
    _eatt_mask.init(*this, 0);
    _eatt_colorR.init(*this, defaultEdgeColor.r);
    _eatt_colorG.init(*this, defaultEdgeColor.g);
    _eatt_colorB.init(*this, defaultEdgeColor.b);
    _eatt_radius.init(*this, defaultEdgeRadius);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitFaceAttributes() {
    _fatt_state.init(*this, State::TOPOLOGY_CHANGED);
    _fatt_visible.init(*this, 1);
    _fatt_colorR.init(*this, 1.0f);
    _fatt_colorG.init(*this, 1.0f);
    _fatt_colorB.init(*this, 1.0f);
    _fatt_colorA.init(*this, 1.0f);
    _fatt_patternR.init(*this, 0.0f);
    _fatt_patternG.init(*this, 0.0f);
    _fatt_patternB.init(*this, 0.0f);
    _fatt_patternA.init(*this, 0.0f);
}

template<typename VEC3>
inline PolyMesh<VEC3>::PolyMesh() {
    InitVertexAttributes();
    InitEdgeAttributes();
    InitFaceAttributes();
}

template<typename VEC3>
inline int PolyMesh<VEC3>::state_of(node v) const {
    return _vatt_state[v];
}

template<typename VEC3>
inline int PolyMesh<VEC3>::state_of(edge e) const {
    return _eatt_state[e];
}

template<typename VEC3>
inline int PolyMesh<VEC3>::state_of(face f) const {
    return _fatt_state[f];
}

template<typename VEC3>
inline void PolyMesh<VEC3>::cache_all() {
    node v;
    edge e;
    face f;
    forall_nodes(v, *this) _vatt_state[v] = State::CACHED;
    forall_edges(e, *this) _eatt_state[e] = State::CACHED;
    forall_faces(f, *this) _fatt_state[f] = State::CACHED;
}

template<typename VEC3>
inline PolyMesh<VEC3>& PolyMesh<VEC3>::operator=(const PolyMesh& other) {
    if(&other != this) {
        base_t::operator=(other);
        compute_faces();
        InitVertexAttributes();
        InitEdgeAttributes();
        InitFaceAttributes();
    }
    return *this;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::force_rebuild() {
    node v;
    edge e;
    face f;
    forall_nodes(v, *this) _vatt_state[v] = State::TOPOLOGY_CHANGED;
    forall_edges(e, *this) _eatt_state[e] = State::TOPOLOGY_CHANGED;
    forall_faces(f, *this) _fatt_state[f] = State::TOPOLOGY_CHANGED;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::join(PolyMesh& other) {
    if(&other != this) {
        base_t::join(other);
        compute_faces();
        InitVertexAttributes();
        InitEdgeAttributes();
        InitFaceAttributes();
    }
}

template<typename VEC3>
inline edge PolyMesh<VEC3>::split(const edge e) {
    return base_t::split_map_edge(e);
}

template<typename VEC3>
inline edge PolyMesh<VEC3>::split(const edge e0, const edge e1) {
    return base_t::split_face(e0, e1);
}

template<typename VEC3>
inline const VEC3& PolyMesh<VEC3>::position_of(const node v) const {
    return LEDA_CONST_ACCESS(VEC3, entry(v));
}

template<typename VEC3>
inline const float PolyMesh<VEC3>::radius_of(const node v) const {
    return _vatt_radius[v];
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::color_of(const node v) const {
    return R::Color(_vatt_colorR[v], _vatt_colorG[v], _vatt_colorB[v]);
}

template<typename VEC3>
inline const float PolyMesh<VEC3>::radius_of(const edge e) const {
    return _eatt_radius[e];
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::color_of(const edge e) const {
    return R::Color(_eatt_colorR[e], _eatt_colorG[e], _eatt_colorB[e]);
}

template<typename VEC3>
int PolyMesh<VEC3>::is_masked(const edge e) const {
    return _eatt_mask[e];
}

template<typename VEC3>
inline bool PolyMesh<VEC3>::is_visible(const face f) const {
    return _fatt_visible[f];
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::color_of(const face f) const {
    return R::Color(_fatt_colorR[f], _fatt_colorG[f], _fatt_colorB[f], _fatt_colorA[f]);
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::pattern_of(const face f) const {
    return R::Color(_fatt_patternR[f], _fatt_patternG[f], _fatt_patternB[f], _fatt_patternA[f]);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_position(const node v, const VEC3& p) {
    _vatt_state[v] = M::Max(_vatt_state[v], static_cast<char>(State::GEOMETRY_CHANGED));

    leda::face f;
    forall_adj_faces(f, v) _fatt_state[f] = M::Max(_fatt_state[f], static_cast<char>(State::GEOMETRY_CHANGED));

    LEDA_ACCESS(VEC3, entry(v)) = p;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_radius(const node v, const float radius) {
    _vatt_state[v] = M::Max(_vatt_state[v], static_cast<char>(State::GEOMETRY_CHANGED));
    _vatt_radius[v] = radius;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const node v, const R::Color& color) {
    _vatt_state[v] = M::Max(_vatt_state[v], static_cast<char>(State::GEOMETRY_CHANGED));

    _vatt_colorR[v] = color.r;
    _vatt_colorG[v] = color.g;
    _vatt_colorB[v] = color.b;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_radius(const edge e, const float radius) {
    const edge r = reversal(e);
    _eatt_radius[e] = _eatt_radius[r] = radius;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const edge e, const R::Color& color) {
    const edge r = reversal(e);

    _eatt_state[e] = M::Max(_eatt_state[e], static_cast<char>(State::GEOMETRY_CHANGED));
    _eatt_state[r] = M::Max(_eatt_state[r], static_cast<char>(State::GEOMETRY_CHANGED));

    _eatt_colorR[e] = color.r;
    _eatt_colorG[e] = color.g;
    _eatt_colorB[e] = color.b;

    _eatt_colorR[r] = color.r;
    _eatt_colorG[r] = color.g;
    _eatt_colorB[r] = color.b;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_masked(const edge e) {
    _eatt_mask[e] = _eatt_mask[reversal(e)] = 1;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_unmasked(const edge e) {
    _eatt_mask[e] = _eatt_mask[reversal(e)] = 0;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_visible(const face f, bool visible) {
    _fatt_visible[f] = visible;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const face f, const R::Color& color) {
    _fatt_state[f] = M::Max(_fatt_state[f], static_cast<char>(State::GEOMETRY_CHANGED));

    _fatt_colorR[f] = color.r;
    _fatt_colorG[f] = color.g;
    _fatt_colorB[f] = color.b;
    _fatt_colorA[f] = color.a;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_pattern(const face f, const R::Color& color) {
    _fatt_state[f] = M::Max(_fatt_state[f], static_cast<char>(State::GEOMETRY_CHANGED));

    _fatt_patternR[f] = color.r;
    _fatt_patternG[f] = color.g;
    _fatt_patternB[f] = color.b;
    _fatt_patternA[f] = color.a;
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
    while(!feof(file)) {
        char buffer[512];
        float f[3];
        unsigned d[3];

        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, 512, file);

        if(3 == sscanf(buffer, "v %f %f %f", &f[0], &f[1], &f[2])) {
            node v = new_node();
            verts.push_back(v);
            float scale = 1.0f;
            set_position(v, FromFloat3<VEC3>::Conv(scale * f[0], scale * f[1], scale * f[2]));
            numVertices++;
        }
        if(3 == sscanf(buffer, "f %d %d %d", &d[0], &d[1], &d[2]) ||
           3 == sscanf(buffer, "f %d/%*d %d/%*d %d/%*d", &d[0], &d[1], &d[2]) ||
           3 == sscanf(buffer, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &d[0], &d[1], &d[2]) ||
           3 == sscanf(buffer, "f %d//%*d %d//%*d %d//%*d", &d[0], &d[1], &d[2]))
        {
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
            else {
                if(leda::target(edgemap[uedge]) == leda::source(e) &&
                   leda::source(edgemap[uedge]) == leda::target(e))
                {
                    set_reversal(edgemap[uedge], e);
                }
            }
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

typedef nb::PolyMesh<d3_rat_point> NbGraph;

} // namespace leda