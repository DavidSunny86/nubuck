#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>
#include <LEDA\geo\d3_delaunay.h>

#include <Nubuck\polymesh.h>
#include <UI\block_signals.h>
#include "d3_delaunay.h"
#include "op_delaunay3d.h"

namespace OP {

void Delaunay3DPanel::OnScaleChanged(int value) {
    EV::Params_OP_Delaunay3D_SetScale args;
    args.value = value;
    args.max = _ui.sldScale->maximum();
    SendToOperator(EV::def_OP_Delaunay3D_SetScale.Create(args));
}

Delaunay3DPanel::Delaunay3DPanel(QWidget* parent) : OperatorPanel(parent) {
    _ui.setupUi(this);
}

void Delaunay3DPanel::Invoke() {
    UI::BlockSignals block(_ui.sldScale);
    _ui.sldScale->setValue(0);
}

void Delaunay3D::Event_SetScale(const EV::Event& event) {
    const EV::Params_OP_Delaunay3D_SetScale& args = EV::def_OP_Delaunay3D_SetScale.GetArgs(event);

    if(_simplices.empty()) return;

    for(unsigned i = 0; i < _simplices.size(); ++i) {
        Simplex& simplex = _simplices[i];

        leda::rational scale = 1 + 5 * leda::rational(args.value, args.max);
        leda::rat_vector center = scale * simplex.center;

        simplex.geom->SetPosition(
            center.xcoord().to_float(),
            center.ycoord().to_float(),
            center.zcoord().to_float());
    }
}

Delaunay3D::Delaunay3D() {
    AddEventHandler(EV::def_OP_Delaunay3D_SetScale, this, &Delaunay3D::Event_SetScale);
}

void Delaunay3D::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetObjectMenu()->addAction("Delaunay 3D");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void Delaunay3D::Invoke() {
    typedef leda::d3_rat_point point3_t;

    _nb.ui->SetOperatorName("Delaunay 3D");

    _simplices.clear();

    std::vector<IGeometry*> geomSel = _nb.world->GetSelection()->GetList();
    if(geomSel.empty()) return;

    IGeometry* cloud = geomSel.front();

    leda::nb::RatPolyMesh& cloudMesh = cloud->GetRatPolyMesh();
    leda::list<point3_t> L;
    leda::node v;
    forall_nodes(v, cloudMesh) L.push_back(cloudMesh.position_of(v));

    leda::list<leda::fork::simplex_t> S;

    std::cout << "Delaunay3D: calling D3_DELAUNAY ... " << std::flush;
    // NOTE: include d3_delaunay.cpp when building LEDA
    leda::fork::D3_DELAUNAY(L, S);
    std::cout << "DONE" << std::endl;

    float x, y, z;
    cloud->GetPosition(x, y, z);

    std::cout << "Delaunay3D: creating simplex geometries ... " << std::flush;
    leda::list_item it;
    forall_items(it, S) {
        IGeometry* geom = _nb.world->CreateGeometry();
        geom->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
        leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

        leda::rat_vector v0 = S[it].verts[0].to_vector();
        leda::rat_vector v1 = S[it].verts[1].to_vector();
        leda::rat_vector v2 = S[it].verts[2].to_vector();
        leda::rat_vector v3 = S[it].verts[3].to_vector();

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
        leda::CONVEX_HULL(L, mesh);
        mesh.compute_faces();

        geom->SetPosition(
            center.xcoord().to_float(),
            center.ycoord().to_float(),
            center.zcoord().to_float());

        Simplex simplex;
        simplex.geom    = geom;
        simplex.center  = center;
        _simplices.push_back(simplex);
    }
    std::cout << "DONE" << std::endl;

    cloud->Destroy();
    _nb.world->GetSelection()->Clear();
}

} // namespace OP