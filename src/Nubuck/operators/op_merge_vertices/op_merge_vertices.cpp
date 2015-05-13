#include <QMenu>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_merge_vertices.h"

namespace OP {

std::string MergeVertices::PreferredShortcut() const {
    return "M";
}

void MergeVertices::Register(Invoker& invoker) {
    NB::AddMenuItem(NB::VertexMenu(), "Merge", invoker);
}

static bool EdgeEx(leda::nb::RatPolyMesh& graph, leda::node v0, leda::node v1) {
    leda::edge e, e0 = nil;
    forall_out_edges(e, v0) {
        if(graph.target(e) == v1) return true;
    }
    return false;
}

// TODO: optimize me!
static leda::node merge_vertices(leda::nb::RatPolyMesh& graph, leda::node v0, leda::node v1) {
    leda::edge e;

    leda::edge e01 = v0->first_out_edge();
    while(e01 && e01->terminal(1) != v1)
        e01 = e01->next_out_edge();
    COM_assert(e01 && e01->terminal(1) == v1);

    leda::edge e10 = graph.reversal(e01);

    leda::edge i = graph.cyclic_adj_pred(e01);
    leda::edge j = graph.cyclic_adj_pred(e10);

    leda::list<leda::edge> del;
    while(j != e10) {
        leda::node w = leda::target(j);
        leda::edge r = graph.reversal(j);
        if(!EdgeEx(graph, v0, w)) {
            leda::edge e = graph.new_edge(i, w);
            r->set_term(1, v0);
            graph.set_reversal(e, r);
        } else {
            del.push(r);
        }
        j = graph.cyclic_adj_pred(j);
    }

    forall(e, del) graph.del_edge(e);
    graph.del_edge(e01);

    v0->append_adj_list(1, v1, 1);
    graph.del_node(v1);

    assert(graph.is_bidirected());
    assert(graph.is_map());

    return v0;
}

bool MergeVertices::Invoke() {
    if(!NB::FirstSelectedMesh()) {
        std::cout << "MergeVertices: empty selection" << std::endl;
        return false;
    }

    NB::SetOperatorName("Merge");

    NB::Mesh mesh = NB::FirstSelectedMesh();
    std::vector<leda::node> verts = mesh->GetVertexSelection();
    if(2 != verts.size()) {
        std::cout << "MergeVertices: number of selected vertices != 2" << std::endl;
        return false;
    }

    leda::nb::RatPolyMesh& graph = mesh->GetRatPolyMesh();
    assert(EdgeEx(graph, verts[0], verts[1]));
    // graph.merge_nodes(verts[0], verts[1]);
    leda::node vert = merge_vertices(graph, verts[0], verts[1]);
    leda::list<leda::edge> r;
    std::cout << "|r| = " << r.size() << std::endl;
    graph.compute_faces();
    NB::SelectVertex(NB::SM_NEW, mesh, vert);

    return true;
}

} // namespace OP