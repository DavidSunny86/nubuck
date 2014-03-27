#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include "op_chull.h"

namespace OP {

void ConvexHull::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetObjectMenu()->addAction("Convex Hull");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void ConvexHull::Invoke() {
    _nb.ui->SetOperatorName("Convex Hull");

	IGeometry* cloud = _nb.world->GetSelection()->GetList().front();
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

    float x, y, z;
    cloud->GetPosition(x, y, z);
    chull->SetPosition(x, y, z);

    cloud->Destroy();
	_nb.world->GetSelection()->Set(chull);
}

} // namespace OP
