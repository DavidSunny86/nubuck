#include <Nubuck\polymesh.h>
#include "globals.h"
#include "phase_init.h"

/*
====================
IsSorted
    precond: L is node list of mesh
    checks if L is sorted node list of mesh, according to lexicographic
    order of vertex positions
====================
*/
bool IsSorted(const leda::nb::RatPolyMesh& mesh, const leda::list<leda::node>& L) {
    leda::list_item last_it = NULL, it = L.first();
    while(it) {
        assert(&mesh == L[it]->owner());
        if(last_it && 0 < leda::d3_rat_point::cmp_xyz(mesh[L[it]], mesh[L[last_it]]))
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
    precond: mesh does not contain any edges, L is the sorted node list of mesh.
    triangulates the mesh according to xy-order of vertices, returns edge of hull
====================
*/
// cnf. d3hf13.cpp (LEDA)
leda::edge TriangulateXY(leda::nb::RatPolyMesh& mesh, leda::list<leda::node>& L, int orient) {
    typedef leda::d3_rat_point point_t;

    assert(0 == mesh.number_of_edges());
    // assert(IsSorted(mesh, L));

    if(L.empty()) return NULL;

    leda::node  last_v  = L.pop_front();
    point_t     last_p  = mesh.position_of(last_v);

    while(!L.empty() && equal_xy(last_p, mesh[L.front()])) {
        mesh.del_node(L.pop_front());
    }

    if(!L.empty()) {
        leda::node v = L.pop_front();

        leda::edge e0 = mesh.new_edge(last_v, v, 0);
        leda::edge e1 = mesh.new_edge(v, last_v, 0);
        mesh[e0] = mesh[e1] = Color::BLACK;
        mesh.set_reversal(e0, e1);

        last_v = v;
        last_p = mesh.position_of(v);
    }

    // scan remaining points

    leda::node v;
    forall(v, L) {
        point_t p = mesh.position_of(v);

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
                mesh.position_of(leda::source(e)),
                mesh.position_of(leda::target(e)));
        } while(orient == orientXY);

        // walk down to lower tangent and triangulate
        do {
            leda::edge succ = mesh.face_cycle_succ(e);
            leda::edge x = mesh.new_edge(succ, v, 0, leda::after);
            leda::edge y = mesh.new_edge(v, leda::source(succ), 0);
            mesh[x] = mesh[y] = Color::RED;
            mesh.set_reversal(x, y);
            e = succ;

            orientXY = leda::orientation_xy(
                p,
                mesh.position_of(leda::source(e)),
                mesh.position_of(leda::target(e)));
        } while(orient == orientXY);

        last_p = p;
    } // forall nodes in L

    leda::edge hull = mesh.last_edge();
    leda::edge e = hull;
    do {
        leda::edge r = mesh.reversal(e);
        mesh[e] = mesh[r] = Color::BLUE;
        e = mesh.face_cycle_succ(e);
    } while(hull != e);

    return mesh.last_edge();
}

Phase_Init::Phase_Init(Globals& g) : _g(g) { }

struct CompareVertexPositions : leda::leda_cmp_base<leda::node> {
    const leda::nb::RatPolyMesh& mesh;

    CompareVertexPositions(const leda::nb::RatPolyMesh& mesh) : mesh(mesh) { }

    int operator()(const leda::node& lhp, const leda::node& rhp) const override {
        return leda::compare(mesh.position_of(lhp), mesh.position_of(rhp));
    }
};

void Phase_Init::Enter() {
    // choose first selected geometry as input
    ISelection* sel = _g.nb.world->GetSelection();
    std::vector<IGeometry*> geomSel = sel->GetList();
    if(geomSel.empty()) {
        printf("ERROR - no input object selected.\n");
        return;
    }
    _g.geom = geomSel[0];


    leda::nb::RatPolyMesh& mesh = _g.geom->GetRatPolyMesh();

    leda::list<leda::node> L = mesh.all_nodes();
    L.sort(CompareVertexPositions(mesh));

    const unsigned renderAll = IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES;
    _g.geom->SetRenderMode(renderAll);

    leda::edge hull = TriangulateXY(mesh, L, -1);
    mesh.compute_faces();
    mesh.set_visible(mesh.face_of(hull), false);

    R::Color colors[3];
    colors[Color::BLACK]    = R::Color::Black;
    colors[Color::RED]      = R::Color::Red;
    colors[Color::BLUE]     = R::Color::Blue;

    leda::node_array<int> nodeColors(mesh, Color::BLACK);

    leda::edge e;
    forall_edges(e, mesh) {
        mesh.set_color(e, colors[mesh[e]]);
        nodeColors[leda::source(e)] = M::Max(nodeColors[leda::source(e)], mesh[e]);
        nodeColors[leda::target(e)] = M::Max(nodeColors[leda::target(e)], mesh[e]);
    }

    leda::node v;
    forall_nodes(v, mesh) {
        mesh.set_color(v, colors[nodeColors[v]]);
    }
}