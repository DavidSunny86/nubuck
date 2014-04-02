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
    mesh.merge_nodes(verts[0], verts[1]);
    leda::list<leda::edge> r;
    mesh.make_bidirected(r);
    std::cout << "|r| = " << r.size() << std::endl;
    mesh.compute_faces();
    geom->ClearVertexSelection();
}

} // namespace OP