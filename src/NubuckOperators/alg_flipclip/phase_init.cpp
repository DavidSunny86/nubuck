#include <Nubuck\polymesh.h>
#include "globals.h"
#include "phase_flip.h"
#include "phase_init.h"

/*
====================
IsSorted
    precond: L is node list of graph
    checks if L is sorted node list of graph, according to lexicographic
    order of vertex positions
====================
*/
bool IsSorted(const leda::nb::RatPolyMesh& graph, const leda::list<leda::node>& L) {
    leda::list_item last_it = NULL, it = L.first();
    while(it) {
        assert(&graph == L[it]->owner());
        if(last_it && 0 < leda::d3_rat_point::cmp_xyz(graph[L[it]], graph[L[last_it]]))
            return false;
        last_it = it;
        it = L.next_item(it);
    }
    return true;
}

inline bool equal_xy(const leda::d3_rat_point& lhp, const leda::d3_rat_point& rhp) {
    return 0 == leda::d3_rat_point::cmp_x(lhp, rhp) && 0 == leda::d3_rat_point::cmp_y(lhp, rhp);
}

/*
====================
TriangulateXY
    precond: graph does not contain any edges, L is the sorted node list of graph.
    triangulates the graph according to xy-order of vertices, returns edge of hull
====================
*/
// cnf. d3hf13.cpp (LEDA)
leda::edge TriangulateXY(leda::nb::RatPolyMesh& graph, leda::list<leda::node>& L, int orient) {
    typedef leda::d3_rat_point point_t;

    // assert(0 == graph.number_of_edges());
    // assert(IsSorted(graph, L));

    if(L.empty()) return NULL;

    leda::node  last_v  = L.pop_front();
    point_t     last_p  = graph.position_of(last_v);

    while(!L.empty() && equal_xy(last_p, graph[L.front()])) {
        graph.del_node(L.pop_front());
    }

    if(!L.empty()) {
        leda::node v = L.pop_front();

        leda::edge e0 = graph.new_edge(last_v, v, 0);
        leda::edge e1 = graph.new_edge(v, last_v, 0);
        graph.set_reversal(e0, e1);
        InvalidateU(graph, e0);

        last_v = v;
        last_p = graph.position_of(v);
    }

    // scan remaining points

    leda::node v;
    forall(v, L) {
        point_t p = graph.position_of(v);

        if(equal_xy(p, last_p)) {
            graph.del_node(v);
            continue;
        }

        // walk up to upper tangent
        leda::edge e = graph.last_edge();
        int orientXY;
        do {
            e = graph.face_cycle_pred(e);
            orientXY = leda::orientation_xy(
                p,
                graph.position_of(leda::source(e)),
                graph.position_of(leda::target(e)));
        } while(orient == orientXY);

        // walk down to lower tangent and triangulate
        do {
            leda::edge succ = graph.face_cycle_succ(e);
            leda::edge x = graph.new_edge(succ, v, 0, leda::after);
            leda::edge y = graph.new_edge(v, leda::source(succ), 0);
            graph.set_reversal(x, y);
            InvalidateU(graph, x);
            e = succ;

            orientXY = leda::orientation_xy(
                p,
                graph.position_of(leda::source(e)),
                graph.position_of(leda::target(e)));
        } while(orient == orientXY);

        last_p = p;
    } // forall nodes in L

    leda::edge hull = graph.last_edge();
    leda::edge e = hull;
    do {
        SetColorU(graph, e, Color::BLUE);
        e = graph.face_cycle_succ(e);
    } while(hull != e);

    return graph.last_edge();
}

Phase_Init::Phase_Init(Globals& g) : _g(g) { }

struct CompareVertexPositionsAscending : leda::leda_cmp_base<leda::node> {
    const leda::nb::RatPolyMesh& graph;

    CompareVertexPositionsAscending(const leda::nb::RatPolyMesh& graph) : graph(graph) { }

    int operator()(const leda::node& lhp, const leda::node& rhp) const override {
        return leda::compare(graph.position_of(lhp), graph.position_of(rhp));
    }
};

struct CompareVertexPositionsDescending : leda::leda_cmp_base<leda::node> {
    const leda::nb::RatPolyMesh& graph;

    CompareVertexPositionsDescending(const leda::nb::RatPolyMesh& graph) : graph(graph) { }

    int operator()(const leda::node& lhp, const leda::node& rhp) const override {
        return -leda::compare(graph.position_of(lhp), graph.position_of(rhp));
    }
};

void Phase_Init::Enter() {
    NB::LogPrintf("entering phase 'init'\n");
}

Phase_Init::StepRet::Enum Phase_Init::Step() {
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_g.meshes[_g.side]);

    if(Side::FRONT == _g.side) {
        leda::list<leda::node>& L = _g.L[Side::FRONT];
        L.sort(CompareVertexPositionsDescending(graph));
        _g.stitchVerts[Side::FRONT] = L.head();
        _g.hullEdges[Side::FRONT] = TriangulateXY(graph, L, +1);
    } else {
        leda::list<leda::node>& L = _g.L[Side::BACK];
        L.sort(CompareVertexPositionsAscending(graph));
        _g.hullEdges[Side::BACK] = TriangulateXY(graph, L, -1);
        _g.stitchVerts[Side::BACK] = leda::source(_g.hullEdges[Side::BACK]);
    }

    graph.compute_faces();
    if(_g.hullEdges[Side::FRONT]) graph.set_visible(graph.face_of(_g.hullEdges[Side::FRONT]), false);
    if(_g.hullEdges[Side::BACK]) graph.set_visible(graph.face_of(_g.hullEdges[Side::BACK]), false);

    ApplyEdgeColors(graph);

    return StepRet::DONE;
}

GEN::Pointer<OP::ALG::Phase> Phase_Init::NextPhase() {
    return GEN::MakePtr(new Phase_Flip(_g));
}