#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>
#include <LEDA\geo\d3_delaunay.h>

#include <Nubuck\polymesh.h>
#include "d3_delaunay.h"
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

    leda::list<leda::fork::simplex_t> simplices;

    std::cout << "Delaunay3D: calling D3_DELAUNAY ... " << std::flush;
    // NOTE: include d3_delaunay.cpp when building LEDA
    leda::fork::D3_DELAUNAY(L, simplices);
    std::cout << "DONE" << std::endl;

    float x, y, z;
    cloud->GetPosition(x, y, z);

    std::cout << "Delaunay3D: creating simplex geometries ... " << std::flush;
    leda::list_item it;
    forall_items(it, simplices) {
        IGeometry* simplex = _nb.world->CreateGeometry();
        simplex->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
        leda::nb::RatPolyMesh& simplexMesh = simplex->GetRatPolyMesh();

        leda::rat_vector v0 = simplices[it].verts[0].to_vector();
        leda::rat_vector v1 = simplices[it].verts[1].to_vector();
        leda::rat_vector v2 = simplices[it].verts[2].to_vector();
        leda::rat_vector v3 = simplices[it].verts[3].to_vector();

        leda::rat_vector center = leda::rat_vector::zero(3);
        center += v0;
        center += v1;
        center += v2;
        center += v3;
        center /= 4;

        leda::list<leda::d3_rat_point> L;
        L.push(v0 - center);
        L.push(v1 - center);
        L.push(v2 - center);
        L.push(v3 - center);
        leda::CONVEX_HULL(L, simplexMesh);
        simplexMesh.compute_faces();

        center *= 2;
        simplex->SetPosition(
            center.xcoord().to_float(),
            center.ycoord().to_float(),
            center.zcoord().to_float());
    }
    std::cout << "DONE" << std::endl;

    cloud->Destroy();
    _nb.world->GetSelection()->Clear();
}

} // namespace OP