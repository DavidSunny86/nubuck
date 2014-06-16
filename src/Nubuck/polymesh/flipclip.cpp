#include "flipclip.h"

namespace {

typedef leda::d3_rat_point          point_t;
typedef leda::GRAPH<point_t, int>   mesh_t;

struct Color {
    enum {
        BLACK = 0,  // convex
        RED,        // not convex
        BLUE        // hull edge
    };
};

typedef leda::node_array<int> cdegs_t[3];

struct Side {
    enum Enum {
        FRONT = 0,
        BACK
    };
};

enum {
    COLOR_MASK      = 3,
    PLANARITY_FLAG  = 4
};

inline const point_t& PositionOf(const mesh_t& mesh, leda::node v) { return mesh[v]; }

inline void SetColorU(mesh_t& mesh, leda::edge e, int color) {
    assert(0 <= color && color < 3);
    const leda::edge r = mesh.reversal(e);
    mesh[e] = mesh[e] & ~COLOR_MASK | color;
    mesh[r] = mesh[r] & ~COLOR_MASK | color;

    assert(Color::RED != color); // use InvalidateU for this
}

inline void InvalidateU(mesh_t& mesh, leda::edge e) {
    mesh[e] = mesh[mesh.reversal(e)] = Color::RED;
}

inline int GetColor(mesh_t& mesh, leda::edge e) {
    return mesh[e] & COLOR_MASK;
}

inline void MarkPlanar(mesh_t& mesh, leda::edge e) {
    mesh[e] |= PLANARITY_FLAG;
}

inline void ClearPlanarFlag(mesh_t& mesh, leda::edge e) {
    mesh[e] &= ~PLANARITY_FLAG;
}

inline bool IsMarkedPlanar(mesh_t& mesh, leda::edge e) {
    return mesh[e] & PLANARITY_FLAG;
}

int ComputeCDeg(mesh_t& mesh, leda::node v, int color) {
    int cdeg = 0;

    leda::edge e;
    forall_out_edges(e, v) {
        if(color == GetColor(mesh, e)) cdeg++;
    }

    return cdeg;
}

inline int GetCDeg(mesh_t& mesh, cdegs_t& cdegs, leda::node v, int color) {
    assert(cdegs[color][v] == ComputeCDeg(mesh, v, color));
    return cdegs[color][v];
}

inline void IncTermCDegs(mesh_t& mesh, cdegs_t& cdegs, leda::edge e) {
    cdegs[GetColor(mesh, e)][leda::source(e)]++;
    cdegs[GetColor(mesh, e)][leda::target(e)]++;
}

inline void DecTermCDegs(mesh_t& mesh, cdegs_t& cdegs, leda::edge e) {
    cdegs[GetColor(mesh, e)][leda::source(e)]--;
    cdegs[GetColor(mesh, e)][leda::target(e)]--;
}

inline bool equal_xy(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
    return 0 == leda::d3_rat_point::cmp_x(lhp, rhp) && 0 == leda::d3_rat_point::cmp_y(lhp, rhp);
}

bool IsCollinear(mesh_t& mesh, leda::edge e) {
    leda::edge r = mesh.reversal(e);

    leda::edge e1 = mesh.face_cycle_succ(r);
    leda::edge e3 = mesh.face_cycle_succ(e);

    const leda::node v0 = leda::source(e1);
    const leda::node v1 = leda::target(e1);
    const leda::node v2 = leda::source(e3);
    const leda::node v3 = leda::target(e3);

    const point_t& p0 = PositionOf(mesh, v0);
    const point_t& p1 = PositionOf(mesh, v1);
    const point_t& p2 = PositionOf(mesh, v2);
    const point_t& p3 = PositionOf(mesh, v3);

    return 0 == leda::orientation(p0, p1, p2, p3);
}

/*
====================
TriangulateXY
    precond: mesh does not contain any edges, L is the sorted node list of mesh.
    triangulates the mesh according to xy-order of vertices, returns edge of hull
====================
*/
// cnf. d3hf13.cpp (LEDA)
leda::edge TriangulateXY(mesh_t& mesh, leda::list<leda::node>& L, int orient) {
    typedef leda::d3_rat_point point_t;

    // assert(0 == mesh.number_of_edges());
    // assert(IsSorted(mesh, L));

    if(L.empty()) return NULL;

    leda::node  last_v  = L.pop_front();
    point_t     last_p  = PositionOf(mesh, last_v);

    while(!L.empty() && equal_xy(last_p, mesh[L.front()])) {
        mesh.del_node(L.pop_front());
    }

    if(!L.empty()) {
        leda::node v = L.pop_front();

        leda::edge e0 = mesh.new_edge(last_v, v, 0);
        leda::edge e1 = mesh.new_edge(v, last_v, 0);
        mesh.set_reversal(e0, e1);
        InvalidateU(mesh, e0);

        last_v = v;
        last_p = PositionOf(mesh, v);
    }

    // scan remaining points

    leda::node v;
    forall(v, L) {
        point_t p = PositionOf(mesh, v);

        if(equal_xy(p, last_p)) {
            mesh.del_node(v);
            continue;
        }

        // walk up to upper tangent
        leda::edge e = mesh.last_edge();
        int orientXY;
        do {
            e = mesh.face_cycle_pred(e);
            orientXY = leda::orientation_xy(
                p,
                PositionOf(mesh, leda::source(e)),
                PositionOf(mesh, leda::target(e)));
        } while(orient == orientXY);

        // walk down to lower tangent and triangulate
        do {
            leda::edge succ = mesh.face_cycle_succ(e);
            leda::edge x = mesh.new_edge(succ, v, 0, leda::after);
            leda::edge y = mesh.new_edge(v, leda::source(succ), 0);
            mesh.set_reversal(x, y);
            InvalidateU(mesh, x);
            e = succ;

            orientXY = leda::orientation_xy(
                p,
                PositionOf(mesh, leda::source(e)),
                PositionOf(mesh, leda::target(e)));
        } while(orient == orientXY);

        last_p = p;
    } // forall nodes in L

    leda::edge hull = mesh.last_edge();
    leda::edge e = hull;
    do {
        SetColorU(mesh, e, Color::BLUE);
        e = mesh.face_cycle_succ(e);
    } while(hull != e);

    return mesh.last_edge();
}

void Triangulate(leda::list<point_t>& points, mesh_t& mesh, leda::edge hullEdges[], leda::node stitchVerts[]) {
    points.sort(); // lexicographic ascending

    leda::list<leda::node> L[2];

    point_t p;
    forall(p, points) {
        // since points are sorted, front list resp. back list is
        // sorted in descending resp. ascending order
        L[Side::FRONT].push_front(mesh.new_node(p));
        L[Side::BACK].push_back(mesh.new_node(p));
    }

    stitchVerts[Side::FRONT] = L[Side::FRONT].head();
    hullEdges[Side::FRONT] = TriangulateXY(mesh, L[Side::FRONT], +1);

    hullEdges[Side::BACK] = TriangulateXY(mesh, L[Side::BACK], -1);
    stitchVerts[Side::BACK] = leda::source(hullEdges[Side::BACK]);
}

inline void CheckCDegs(mesh_t& mesh, cdegs_t& cdegs) {
#ifdef _DEBUG
    leda::node v;
    forall_nodes(v, mesh) {
        for(int i = 0; i < 3; ++i) {
            int d0 = cdegs[i][v];
            int d1 = ComputeCDeg(mesh, v, i);
            assert(d0 == d1);
        }
    }
#endif
}

void InitColorDegs(mesh_t& mesh, leda::edge hullEdges[2], cdegs_t& cdegs) {
    cdegs[0].init(mesh, 0);
    cdegs[1].init(mesh, 0);
    cdegs[2].init(mesh, 0);

    leda::node v;
    leda::edge e;

    forall_nodes(v, mesh) {
        cdegs[Color::RED][v] = mesh.outdeg(v);
    }

    e = hullEdges[Side::FRONT];
    do {
        // note that every hull vertex has two adj. blue edges
        cdegs[Color::RED][leda::source(e)]--;
        cdegs[Color::RED][leda::target(e)]--;
        cdegs[Color::BLUE][leda::source(e)]++;
        cdegs[Color::BLUE][leda::target(e)]++;
        e = mesh.face_cycle_succ(e);
    } while(hullEdges[Side::FRONT] != e);

    e = hullEdges[Side::BACK];
    do {
        // note that every hull vertex has two adj. blue edges
        cdegs[Color::RED][leda::source(e)]--;
        cdegs[Color::RED][leda::target(e)]--;
        cdegs[Color::BLUE][leda::source(e)]++;
        cdegs[Color::BLUE][leda::target(e)]++;
        e = mesh.face_cycle_succ(e);
    } while(hullEdges[Side::BACK] != e);

    CheckCDegs(mesh, cdegs);
}

int Flipping(mesh_t& mesh, cdegs_t& cdegs) {
    // gather red edges
    leda::list<leda::edge> S;
    leda::edge e;
    forall_edges(e, mesh) {
        if(Color::RED == GetColor(mesh, e)) S.push(e);
    }

    int numFlips = 0;

    while(!S.empty()) {
        leda::edge e = S.pop();
        leda::edge r = mesh.reversal(e);

        if(Color::BLUE == GetColor(mesh, e)) {
            continue;
        }

        leda::edge e1 = mesh.face_cycle_succ(r);
        leda::edge e3 = mesh.face_cycle_succ(e);

        const leda::node v0 = leda::source(e1);
        const leda::node v1 = leda::target(e1);
        const leda::node v2 = leda::source(e3);
        const leda::node v3 = leda::target(e3);

        const point_t& p0 = PositionOf(mesh, v0);
        const point_t& p1 = PositionOf(mesh, v1);
        const point_t& p2 = PositionOf(mesh, v2);
        const point_t& p3 = PositionOf(mesh, v3);

        const int orient = leda::orientation(p0, p1, p2, p3);

        DecTermCDegs(mesh, cdegs, e);

        if(0 >= orient) {
            // edge is already convex
            SetColorU(mesh, e, Color::BLACK);

            if(0 == orient) {
                MarkPlanar(mesh, e);
            }
        } else {
            InvalidateU(mesh, e);

            int orient_130 = leda::orientation_xy(p1, p3, p0);
            int orient_132 = leda::orientation_xy(p1, p3, p2);

            if( orient_130 != orient_132 && // is convex quadliteral
                (0 != orient_130 || 4 == mesh.outdeg(v0) || Color::BLUE == GetColor(mesh, e1)) &&
                (0 != orient_132 || 4 == mesh.outdeg(v2) || Color::BLUE == GetColor(mesh, e3)))
            {
                // NOTE: if quadliteral is not stricly convex then it's either part of the hull
                // or a perfect diamond.

                leda::edge e2 = mesh.face_cycle_succ(e1);
                leda::edge e4 = mesh.face_cycle_succ(e3);

                S.push(e1);
                S.push(e2);
                S.push(e3);
                S.push(e4);

                // perform flip

                mesh.move_edge(e, e2, leda::source(e4));
                mesh.move_edge(r, e4, leda::source(e2));

                if( (0 == orient_130 && Color::BLUE == GetColor(mesh, e1)) ||
                    (0 == orient_132 && Color::BLUE == GetColor(mesh, e3)))
                {
                    // quadliteral is part of hull
                    SetColorU(mesh, e, Color::BLUE);
                } else {
                    SetColorU(mesh, e, Color::BLACK);
                }

                numFlips++;

            } // flip possible
        } // edge not convex

        IncTermCDegs(mesh, cdegs, e);

    } // while !S.empty()

    CheckCDegs(mesh, cdegs);

    return numFlips;
}

void SimplifyFace(mesh_t& mesh, cdegs_t& cdegs, leda::node v) {
    if(mesh.outdeg(v) <= 3) return; // nothing to do

    leda::edge e1 = mesh.first_adj_edge(v);
    leda::edge e2 = mesh.cyclic_adj_succ(e1);
    leda::edge e3 = mesh.cyclic_adj_succ(e2);

    // count: number of traversed edges since last flip
    int count = 0;
    while(count++ < mesh.outdeg(v) && mesh.outdeg(v) > 3) {
        const point_t& p0 = PositionOf(mesh, v);
        const point_t& p1 = PositionOf(mesh, leda::target(e1));
        const point_t& p2 = PositionOf(mesh, leda::target(e2));
        const point_t& p3 = PositionOf(mesh, leda::target(e3));

        int orient_130 = leda::orientation_xy(p1, p3, p0);
        int orient_132 = leda::orientation_xy(p1, p3, p2);

        if(orient_130 != orient_132 && orient_132 != 0) {
            DecTermCDegs(mesh, cdegs, e2);
            leda::edge r2 = mesh.reversal(e2);
            mesh.move_edge(e2, mesh.reversal(e1), leda::target(e3), leda::before);
            mesh.move_edge(r2, mesh.reversal(e3), leda::target(e1));
            InvalidateU(mesh, e2);
            IncTermCDegs(mesh, cdegs, e2);
            count = 0;
        } else {
            e1 = e2;
        }

        e2 = e3;
        e3 = mesh.cyclic_adj_succ(e2);
    }

    CheckCDegs(mesh, cdegs);

    assert(mesh.outdeg(v) <= 3);
}

int Clipping(mesh_t& mesh, cdegs_t& cdegs) {
    leda::node_list L;

    leda::node v;
    forall_nodes(v, mesh) {
        if(3 <= GetCDeg(mesh, cdegs, v, Color::RED)) L.push(v);
    }

    int numClips = 0;

    while(!L.empty()) {
        leda::node clipV = L.pop();

        const int bbdeg = GetCDeg(mesh, cdegs, clipV, Color::BLACK) + GetCDeg(mesh, cdegs, clipV, Color::BLUE);
        if(0 != bbdeg) continue;

        leda::edge e;
        forall_out_edges(e, clipV) {
            leda::edge b = mesh.face_cycle_succ(e); // boundary edge
            if(Color::BLACK == GetColor(mesh, b)) {
                DecTermCDegs(mesh, cdegs, b);
                InvalidateU(mesh, b);
                IncTermCDegs(mesh, cdegs, b);
            }
        }

        if(3 < mesh.outdeg(clipV)) SimplifyFace(mesh, cdegs, clipV);

        forall_out_edges(e, clipV) {
            assert(Color::RED == GetColor(mesh, e));
            DecTermCDegs(mesh, cdegs, e);
        }

        mesh.del_node(clipV);
        numClips++;
    }

    CheckCDegs(mesh, cdegs);

    return numClips;
}

void StripTetrahedrons(mesh_t& mesh, cdegs_t& cdegs, leda::node v) {
    if(mesh.outdeg(v) <= 3) return; // nothing to do

    leda::edge e1 = mesh.first_adj_edge(v);
    leda::edge e2 = mesh.cyclic_adj_succ(e1);
    leda::edge e3 = mesh.cyclic_adj_succ(e2);

    // count: number of traversed edges since last flip
    int count = 0;
    while(count++ < mesh.outdeg(v) && mesh.outdeg(v) > 3) {
        const point_t& p0 = PositionOf(mesh, v);
        const point_t& p1 = PositionOf(mesh, leda::target(e1));
        const point_t& p2 = PositionOf(mesh, leda::target(e2));
        const point_t& p3 = PositionOf(mesh, leda::target(e3));

        int orient_130 = leda::orientation_xy(p1, p3, p0);
        int orient_132 = leda::orientation_xy(p1, p3, p2);

        if(Color::BLACK == GetColor(mesh, e2) && orient_130 != orient_132 && orient_132 != 0) {
            DecTermCDegs(mesh, cdegs, e2);
            leda::edge r2 = mesh.reversal(e2);
            mesh.move_edge(e2, mesh.reversal(e1), leda::target(e3), leda::before);
            mesh.move_edge(r2, mesh.reversal(e3), leda::target(e1));
            InvalidateU(mesh, e2);
            IncTermCDegs(mesh, cdegs, e2);
            count = 0;
        } else {
            e1 = e2;
        }

        e2 = e3;
        e3 = mesh.cyclic_adj_succ(e2);
    }
}

int Stripping(mesh_t& mesh, cdegs_t& cdegs) {
    leda::node_list L;
    leda::node v;
    forall_nodes(v, mesh) {
        if(0 == GetCDeg(mesh, cdegs, v, Color::BLUE) && 1 < GetCDeg(mesh, cdegs, v, Color::RED)) {
            L.push(v);
        }
    }

    int numStrips = 0;

    while(!L.empty()) {
        leda::node v = L.pop();

        StripTetrahedrons(mesh, cdegs, v);
        numStrips++;
    }

    CheckCDegs(mesh, cdegs);

    return numStrips;
}

void Stitching(mesh_t& mesh, leda::edge hullEdges[], leda::node stitchVerts[]) {
    leda::edge adv0 = hullEdges[Side::FRONT];
    while(stitchVerts[Side::FRONT] != leda::target(adv0))
        adv0 = mesh.face_cycle_pred(adv0); // here dir is arbitrary

    leda::edge adv1 = hullEdges[Side::BACK];
    while(stitchVerts[Side::BACK] != leda::source(adv1))
        adv1 = mesh.face_cycle_succ(adv1); // here dir is arbitrary

    leda::node v0, v1;
    do {
        v0 = leda::target(adv0);
        v1 = leda::source(adv1);

        assert(equal_xy(PositionOf(mesh, v0), PositionOf(mesh, v1)));

        if(0 != leda::d3_rat_point::cmp_z(PositionOf(mesh, v0), PositionOf(mesh, v1))) {
            // stitch

            int dir = leda::before;
            leda::edge e = mesh.new_edge(mesh.reversal(adv0), v1, dir, dir);
            leda::edge r = mesh.new_edge(mesh.cyclic_adj_succ(adv1), v0, dir, dir);
            mesh.set_reversal(e, r);

            if(IsCollinear(mesh, e)) {
                MarkPlanar(mesh, e);
            }

            adv0 = mesh.face_cycle_pred(adv0);
            adv1 = mesh.face_cycle_succ(adv1);
        } else {
            // collapse

            leda::edge succ0    = mesh.face_cycle_succ(adv0);
            leda::edge term1    = mesh.cyclic_adj_succ(adv1);

            leda::edge r, next, it1;

            it1  = adv1;
            next = mesh.cyclic_adj_pred(it1);

            if(leda::source(adv0) != leda::target(it1)) {
                r = mesh.reversal(it1);
                mesh.move_edge(it1, succ0, leda::target(it1), leda::behind);
                mesh.move_edge(r, leda::source(r), v0);
            }
            it1 = next;

            while(it1 != term1) {
                next = mesh.cyclic_adj_pred(it1);
                r = mesh.reversal(it1);
                mesh.move_edge(it1, succ0, leda::target(it1), leda::behind);
                mesh.move_edge(r, leda::source(r), v0);
                it1 = next;
            }

            if(leda::target(succ0) != leda::target(it1)) {
                r = mesh.reversal(it1);
                mesh.move_edge(it1, succ0, leda::target(it1), leda::behind);
                mesh.move_edge(r, leda::source(r), v0);
            }

            adv0 = mesh.face_cycle_pred(adv0);
            adv1 = mesh.face_cycle_succ(adv1);

            mesh.del_node(v1);
        } // collapse

    } while(stitchVerts[Side::FRONT] != leda::target(adv0));

}

// these nodes somehow mess up rendering
void DeleteInnerNodes(mesh_t& G) {
    leda::node n;
    forall_nodes(n, G) {
        if(2 == G.outdeg(n)) {
            const leda::edge e0 = G.first_adj_edge(n);
            const leda::edge e1 = G.cyclic_adj_succ(e0);

            const leda::node p0 = G.target(e0);
            const leda::node p1 = G.target(e1);

            if(leda::collinear(G[p0], G[n], G[p1])) {
                const leda::edge r0 = G.reversal(e0);
                const leda::edge r1 = G.reversal(e1);

                G.set_reversal(
                    G.new_edge(r0, p1),
                    G.new_edge(r1, p0));

                G.del_edge(e0);
                G.del_edge(e1);
                G.del_edge(r0);
                G.del_edge(r1);
            }
        }
    }
}

void DeleteLooseNodes(mesh_t& G) {
    leda::node n;
    forall_nodes(n, G) {
        if(!G.outdeg(n)) G.del_node(n);
    }
}

void Simplify(mesh_t& mesh) {
    leda::node_array<int> deg(mesh, 0);
    leda::node v;
    forall_nodes(v, mesh) {
        deg[v] = mesh.outdeg(v);
    }

    leda::edge_array<bool> inL(mesh, false);
    leda::list<leda::edge> L;
    leda::edge e;
    forall_edges(e, mesh) {
        bool isPlanar =
            (Color::BLACK == GetColor(mesh, e) && IsMarkedPlanar(mesh, e)) ||
            IsCollinear(mesh, e); // blue edges are unmarked

        if(IsCollinear(mesh, e)) assert(isPlanar);

        if(!inL[e] && isPlanar) {
            assert(IsCollinear(mesh, e));
            L.push(e);
            inL[e] = inL[mesh.reversal(e)] = true;
        }
    }

    leda::edge r;
    forall(e, L) {
        r = mesh.reversal(e);

        mesh.del_edge(r);
        mesh.del_edge(e);

    }

    DeleteInnerNodes(mesh);
    DeleteLooseNodes(mesh);
}

} // unnamed namespace

void FlipClipHull(leda::list<leda::d3_rat_point> points, leda::GRAPH<leda::d3_rat_point, int>& mesh) {
    leda::edge  hullEdges[2];
    leda::node  stitchVerts[2];

    Triangulate(points, mesh, hullEdges, stitchVerts);

    cdegs_t cdegs;

    InitColorDegs(mesh, hullEdges, cdegs);

    bool done = false;
    while(!done) {
        Flipping(mesh, cdegs);
        if(!Clipping(mesh, cdegs)) {
            if(Stripping(mesh, cdegs)) {
                Clipping(mesh, cdegs);
            } else {
                done = true;
            }
        }
    }

    Stitching(mesh, hullEdges, stitchVerts);
    Simplify(mesh);
}