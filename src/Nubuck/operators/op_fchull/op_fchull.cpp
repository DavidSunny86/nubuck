#include <QMenu>
#include <QAction>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\polymesh.h>
#include <polymesh\flipclip.h>
#include "op_fchull.h"

namespace OP {

void FlipClip::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetObjectMenu()->addAction("FlipClip Hull");
    action->setShortcut(QKeySequence("Shift+C"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool FlipClip::Invoke() {
    ISelection* sel = _nb.world->GetSelection();
    std::vector<IGeometry*> geomSel = sel->GetList();
    if(geomSel.empty()) {
        _nb.log->printf("no geometry selected.\n");
        return false;
    }

    _nb.ui->SetOperatorName("FlipClip Hull");

    _nb.log->printf("FlipClip Hull:\n");

	IGeometry* cloud = geomSel[0];
    assert(cloud);

    leda::nb::RatPolyMesh& cloudMesh = cloud->GetRatPolyMesh();
    leda::list<leda::d3_rat_point> L0, L1;
    leda::node v;
    forall_nodes(v, cloudMesh) L0.push_back(cloudMesh.position_of(v));
    L1 = L0;

    IGeometry* chull = _nb.world->CreateGeometry();
    chull->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
    chull->SetName(std::string("CH(") + cloud->GetName() + ")");
    leda::nb::RatPolyMesh& chullMesh = chull->GetRatPolyMesh();

    SYS::Timer  timer;
    float       secsPassed;

    timer.Start();
    leda::CONVEX_HULL(L0, chullMesh);
    secsPassed = timer.Stop();
    _nb.log->printf("... CONVEX_HULL: %fs\n", secsPassed);

    chullMesh.clear();

    timer.Start();
    FlipClipHull(L1, chullMesh);
    secsPassed = timer.Stop();
    _nb.log->printf("... FlipClip: %fs\n", secsPassed);

    _nb.log->printf("... CHECK_HULL: ");
    bool isConvex = leda::CHECK_HULL(chullMesh);
    _nb.log->printf(isConvex ? "true" : "false");
    _nb.log->printf("\n");

    chullMesh.compute_faces();

    chull->SetPosition(cloud->GetPosition());

    cloud->Destroy();
	_nb.world->GetSelection()->Set(chull);

    _nb.log->printf("convex hull:\n");
    _nb.log->printf("... |V| = %d\n", chullMesh.number_of_nodes());
    _nb.log->printf("... |E| = %d\n", chullMesh.number_of_edges());
    _nb.log->printf("... |F| = %d\n", chullMesh.number_of_faces());
}

} // namespace OP