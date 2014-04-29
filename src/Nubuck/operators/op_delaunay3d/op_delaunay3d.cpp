#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_delaunay.h>

#include <Nubuck\polymesh.h>
#include "op_delaunay3d.h"

namespace OP {

void Delaunay3D::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetObjectMenu()->addAction("Delaunay 3D");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void Delaunay3D::Invoke() {
    typedef leda::d3_rat_point point3_t;

    _nb.ui->SetOperatorName("Delaunay 3D");

    std::vector<IGeometry*> geomSel = _nb.world->GetSelection()->GetList();
    if(geomSel.empty()) return;

    IGeometry* cloud = geomSel.front();

    leda::nb::RatPolyMesh& cloudMesh = cloud->GetRatPolyMesh();
    leda::list<point3_t> L;
    leda::node v;
    forall_nodes(v, cloudMesh) L.push_back(cloudMesh.position_of(v));

    IGeometry* delaunay = _nb.world->CreateGeometry();
    delaunay->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES);
    leda::nb::RatPolyMesh& delaunayMesh = delaunay->GetRatPolyMesh();
    
    std::cout << "Delaunay3D: calling D3_DELAUNAY ... " << std::flush;
    // NOTE: include d3_delaunay.cpp when building LEDA
    leda::D3_DELAUNAY(L, delaunayMesh);
    std::cout << "DONE" << std::endl;

    float x, y, z;
    cloud->GetPosition(x, y, z);
    delaunay->SetPosition(x, y, z);

    cloud->Destroy();
    _nb.world->GetSelection()->Set(delaunay);
}

} // namespace OP