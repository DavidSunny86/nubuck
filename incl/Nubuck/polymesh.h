#pragma once

#include <vector>

#include <Nubuck\renderer\color\color.h>
#include <Nubuck\math\math.h>
#include <Nubuck\common\common.h>

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
    struct VertexAttribs {
        float       radius;
        R::Color    color;
        char        state;
    };
    leda::node_map<VertexAttribs> _vatt;

    // edge attributes
    struct EdgeAttribs {
        int         mask;
        float       radius;
        R::Color    color;
        char        state;
    };
    leda::edge_map<EdgeAttribs> _eatt;

    // face attributes
    struct FaceAttribs {
        int         visible;
        R::Color    color;
        R::Color    pattern;
        char        state;
    };
    leda::face_map<FaceAttribs> _fatt;

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
    void set_source_color(const edge e, const R::Color& color);
    void set_masked(const edge e);
    void set_unmasked(const edge e);

    void set_visible(const face f, bool visible);
    void set_color(const face f, const R::Color& color);
    void set_pattern(const face f, const R::Color& color);

    edge make_triangle(const VEC3& p0, const VEC3& p1, const VEC3& p2);
};

typedef PolyMesh<d3_rat_point> RatPolyMesh;

NUBUCK_API void     make_leda(RatPolyMesh& mesh);
NUBUCK_API size_t   make_from_obj(const char* filename, RatPolyMesh& mesh);
NUBUCK_API void     make_grid(RatPolyMesh& mesh, const int subdiv, const float size);

void D3_HULL(list<d3_rat_point> L, RatPolyMesh& mesh);

template<typename VEC3>
const R::Color PolyMesh<VEC3>::defaultVertexColor = R::Color(0.3f, 0.3f, 0.3f);

template<typename VEC3>
const float PolyMesh<VEC3>::defaultVertexRadius = 0.02f;

template<typename VEC3>
const R::Color PolyMesh<VEC3>::defaultEdgeColor = R::Color(1.0f, 1.0f, 1.0f);

template<typename VEC3>
const float PolyMesh<VEC3>::defaultEdgeRadius = 0.01f;

template<typename VEC3>
void set_color(PolyMesh<VEC3>& mesh, const R::Color& color) {
    leda::face f;
    forall_faces(f, mesh) mesh.set_color(f, color);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitVertexAttributes() {
    VertexAttribs defaultAttribs;
    defaultAttribs.radius = defaultVertexRadius;
    defaultAttribs.color = defaultVertexColor;
    defaultAttribs.state = State::TOPOLOGY_CHANGED;
    _vatt.init(*this, defaultAttribs);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitEdgeAttributes() {
    EdgeAttribs defaultEdgeAttribs;
    defaultEdgeAttribs.mask = 0;
    defaultEdgeAttribs.radius = defaultEdgeRadius;
    defaultEdgeAttribs.color = defaultEdgeColor;
    defaultEdgeAttribs.state = State::TOPOLOGY_CHANGED;
    _eatt.init(*this, defaultEdgeAttribs);
}

template<typename VEC3>
inline void PolyMesh<VEC3>::InitFaceAttributes() {
    FaceAttribs defaultFaceAttribs;
    defaultFaceAttribs.visible = 1;
    defaultFaceAttribs.color = R::Color::White;
    defaultFaceAttribs.pattern = R::Color(0.0f, 0.0f, 0.0f, 0.0f);
    defaultFaceAttribs.state = State::TOPOLOGY_CHANGED;
    _fatt.init(*this, defaultFaceAttribs);
}

template<typename VEC3>
inline PolyMesh<VEC3>::PolyMesh() {
    InitVertexAttributes();
    InitEdgeAttributes();
    InitFaceAttributes();
}

template<typename VEC3>
inline int PolyMesh<VEC3>::state_of(node v) const {
    return _vatt[v].state;
}

template<typename VEC3>
inline int PolyMesh<VEC3>::state_of(edge e) const {
    return _eatt[e].state;
}

template<typename VEC3>
inline int PolyMesh<VEC3>::state_of(face f) const {
    return _fatt[f].state;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::cache_all() {
    node v;
    edge e;
    face f;
    forall_nodes(v, *this) _vatt[v].state = State::CACHED;
    forall_edges(e, *this) _eatt[e].state = State::CACHED;
    forall_faces(f, *this) _fatt[f].state = State::CACHED;
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
    forall_nodes(v, *this) _vatt[v].state = State::TOPOLOGY_CHANGED;
    forall_edges(e, *this) _eatt[e].state = State::TOPOLOGY_CHANGED;
    forall_faces(f, *this) _fatt[f].state = State::TOPOLOGY_CHANGED;
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
    return _vatt[v].radius;
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::color_of(const node v) const {
    return R::Color(_vatt[v].color);
}

template<typename VEC3>
inline const float PolyMesh<VEC3>::radius_of(const edge e) const {
    return _eatt[e].radius;
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::color_of(const edge e) const {
    return _eatt[e].color;
}

template<typename VEC3>
int PolyMesh<VEC3>::is_masked(const edge e) const {
    return _eatt[e].mask;
}

template<typename VEC3>
inline bool PolyMesh<VEC3>::is_visible(const face f) const {
    return _fatt[f].visible;
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::color_of(const face f) const {
    return _fatt[f].color;
}

template<typename VEC3>
inline R::Color PolyMesh<VEC3>::pattern_of(const face f) const {
    return _fatt[f].pattern;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_position(const node v, const VEC3& p) {
    _vatt[v].state = M::Max(_vatt[v].state, static_cast<char>(State::GEOMETRY_CHANGED));

    leda::face f;
    forall_adj_faces(f, v) _fatt[f].state = M::Max(_fatt[f].state, static_cast<char>(State::GEOMETRY_CHANGED));

    LEDA_ACCESS(VEC3, entry(v)) = p;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_radius(const node v, const float radius) {
    _vatt[v].state = M::Max(_vatt[v].state, static_cast<char>(State::GEOMETRY_CHANGED));
    _vatt[v].radius = radius;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const node v, const R::Color& color) {
    _vatt[v].state = M::Max(_vatt[v].state, static_cast<char>(State::GEOMETRY_CHANGED));
    _vatt[v].color = color;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_radius(const edge e, const float radius) {
    const edge r = reversal(e);
    _eatt[e].radius = _eatt[r].radius = radius;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const edge e, const R::Color& color) {
    const edge r = reversal(e);

    _eatt[e].state = M::Max(_eatt[e].state, static_cast<char>(State::GEOMETRY_CHANGED));
    _eatt[r].state = M::Max(_eatt[r].state, static_cast<char>(State::GEOMETRY_CHANGED));

    _eatt[e].color = color;
    _eatt[r].color = color;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_source_color(const edge e, const R::Color& color) {
    _eatt[e].state = M::Max(_eatt[e].state, static_cast<char>(State::GEOMETRY_CHANGED));
    _eatt[e].color = color;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_masked(const edge e) {
    _eatt[e].mask = _eatt[reversal(e)].mask = 1;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_unmasked(const edge e) {
    _eatt[e].mask = _eatt[reversal(e)].mask = 0;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_visible(const face f, bool visible) {
    _fatt[f].visible = visible;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_color(const face f, const R::Color& color) {
    _fatt[f].state = M::Max(_fatt[f].state, static_cast<char>(State::GEOMETRY_CHANGED));
    _fatt[f].color = color;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_pattern(const face f, const R::Color& pattern) {
    _fatt[f].state = M::Max(_fatt[f].state, static_cast<char>(State::GEOMETRY_CHANGED));
    _fatt[f].pattern = pattern;
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

    _fatt[face_of(f[0])].visible = true;
    _fatt[face_of(b[0])].visible = false;

    return f[0];
}

} // namespace nb

typedef nb::PolyMesh<d3_rat_point> NbGraph; // deprecated

} // namespace leda

namespace NB {

typedef leda::nb::RatPolyMesh   Graph;
typedef leda::d3_rat_point      Point3;

} // namespace NB