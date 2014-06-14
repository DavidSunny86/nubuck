#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include "op_chull.h"

namespace OP {

void ConvexHull::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetObjectMenu()->addAction("Convex Hull");
    action->setShortcut(QKeySequence("C"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool ConvexHull::Invoke() {
    ISelection* sel = _nb.world->GetSelection();
    std::vector<IGeometry*> geomSel = sel->GetList();
    if(geomSel.empty()) {
        _nb.log->printf("no geometry selected.\n");
        return false;
    }

    _nb.ui->SetOperatorName("Convex Hull");

	IGeometry* cloud = geomSel[0];
    assert(cloud);

    leda::nb::RatPolyMesh& cloudMesh = cloud->GetRatPolyMesh();
    leda::list<point3_t> L;
    leda::node v;
    forall_nodes(v, cloudMesh) L.push_back(cloudMesh.position_of(v));

    IGeometry* chull = _nb.world->CreateGeometry();
    chull->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
    chull->SetName(std::string("CH(") + cloud->GetName() + ")");
    leda::nb::RatPolyMesh& chullMesh = chull->GetRatPolyMesh();
    leda::CONVEX_HULL(L, chullMesh);
    chullMesh.compute_faces();

    chull->SetPosition(cloud->GetPosition());

    cloud->Destroy();
	_nb.world->GetSelection()->Set(chull);

    _nb.log->printf("convex hull:\n");
    _nb.log->printf("... |V| = %d\n", chullMesh.number_of_nodes());
    _nb.log->printf("... |E| = %d\n", chullMesh.number_of_edges());
    _nb.log->printf("... |F| = %d\n", chullMesh.number_of_faces());

    return true;
}

} // namespace OP
