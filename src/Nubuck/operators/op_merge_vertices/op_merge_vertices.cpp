#include <QMenu>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_merge_vertices.h"

namespace OP {

void MergeVertices::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetVertexMenu()->addAction("Merge");
    action->setShortcut(QKeySequence("M"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

static bool EdgeEx(leda::nb::RatPolyMesh& mesh, leda::node v0, leda::node v1) {
    leda::edge e, e0 = nil;
    forall_out_edges(e, v0) {
        if(mesh.target(e) == v1) return true;
    }
    return false;
}

// TODO: optimize me!
static leda::node merge_vertices(leda::nb::RatPolyMesh& mesh, leda::node v0, leda::node v1) {
    leda::edge e;

    leda::edge e01 = v0->first_out_edge();
    while(e01 && e01->terminal(1) != v1)
        e01 = e01->next_out_edge();
    COM_assert(e01 && e01->terminal(1) == v1);

    leda::edge e10 = mesh.reversal(e01);

    leda::edge i = mesh.cyclic_adj_pred(e01);
    leda::edge j = mesh.cyclic_adj_pred(e10);

    leda::list<leda::edge> del;
    while(j != e10) {
        leda::node w = leda::target(j);
        leda::edge r = mesh.reversal(j);
        if(!EdgeEx(mesh, v0, w)) {
            leda::edge e = mesh.new_edge(i, w);
            r->set_term(1, v0);
            mesh.set_reversal(e, r);
        } else {
            del.push(r);
        }
        j = mesh.cyclic_adj_pred(j);
    }

    forall(e, del) mesh.del_edge(e);
    mesh.del_edge(e01);

    v0->append_adj_list(1, v1, 1);
    mesh.del_node(v1);

    assert(mesh.is_bidirected());
    assert(mesh.is_map());

    return v0;
}

void MergeVertices::Invoke() {
    _nb.ui->SetOperatorName("Merge");

    ISelection* sel = _nb.world->GetSelection();
    if(sel->GetList().empty()) {
        std::cout << "MergeVertices: empty selection" << std::endl;
        return;
    }
    W::ENT_Geometry* geom = (W::ENT_Geometry*)sel->GetList().front();
    std::vector<leda::node> verts = geom->GetVertexSelection();
    if(2 != verts.size()) {
        std::cout << "MergeVertices: number of selected vertices != 2" << std::endl;
        return;
    }

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
    assert(EdgeEx(mesh, verts[0], verts[1]));
    // mesh.merge_nodes(verts[0], verts[1]);
    leda::node vert = merge_vertices(mesh, verts[0], verts[1]);
    leda::list<leda::edge> r;
    std::cout << "|r| = " << r.size() << std::endl;
    mesh.compute_faces();
    sel->SelectVertex(ISelection::SELECT_NEW, geom, vert);
}

} // namespace OP