#pragma once

#include <LEDA\graph\graph.h>
#include <LEDA\graph\face_map.h>
#include <LEDA\geo\d3_rat_point.h>

namespace leda {
namespace nb {

template<typename VEC3>
class PolyMesh : public GRAPH<VEC3, int> {
private:
    struct FaceAttribs {
        int visible;

        FaceAttribs() : visible(true) { }
    };

    leda::face_map<FaceAttribs> _faceAttribs;
public:
    PolyMesh();

    const VEC3& position_of(const node v);
    bool        is_visible(const face f);

    void set_position(const node v, const VEC3& p);

    edge make_triangle(const VEC3& p0, const VEC3& p1, const VEC3& p2);
};

typedef PolyMesh<d3_rat_point> RatPolyMesh;

void D3_HULL(list<d3_rat_point> L, RatPolyMesh& mesh);

template<typename VEC3>
inline PolyMesh<VEC3>::PolyMesh() {
    _faceAttribs.init(*this);
}

template<typename VEC3>
inline const VEC3& PolyMesh<VEC3>::position_of(const node v) {
    return LEDA_CONST_ACCESS(VEC3, entry(v));
}

template<typename VEC3>
inline bool PolyMesh<VEC3>::is_visible(const face f) {
    return _faceAttribs[f].visible;
}

template<typename VEC3>
inline void PolyMesh<VEC3>::set_position(const node v, const VEC3& p) {
    LEDA_ACCESS(VEC3, entry(v)) = p;
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

} // namespace nb
} // namespace leda