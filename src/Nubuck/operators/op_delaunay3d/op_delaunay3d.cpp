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

        leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        for(int i = 0; i < 4; ++i) {
            leda::rat_vector pos = simplex.localPos[i] + center;
            mesh.set_position(simplex.verts[i], pos);
        }
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

void AddFace(leda::nb::RatPolyMesh& mesh, leda::node v0, leda::node v1, leda::node v2, leda::node v3) {
    mesh.new_edge(v0, v1);
    mesh.new_edge(v0, v3);
    mesh.new_edge(v0, v2);

    mesh.new_edge(v1, v2);
    mesh.new_edge(v1, v3);
    mesh.new_edge(v1, v0);

    mesh.new_edge(v2, v0);
    mesh.new_edge(v2, v3);
    mesh.new_edge(v2, v1);

    mesh.new_edge(v3, v0);
    mesh.new_edge(v3, v1);
    mesh.new_edge(v3, v2);
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

    geom = _nb.world->CreateGeometry();
    geom->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

    std::cout << "Delaunay3D: creating simplex geometries ... " << std::flush;
    leda::list_item it;
    forall_items(it, S) {
        Simplex simplex;

        simplex.center = leda::rat_vector::zero(3);
        leda::rat_vector pos[4];
        for(int i = 0; i < 4; ++i) {
            pos[i] = S[it].verts[i].to_vector();
            simplex.center += pos[i];

            simplex.verts[i] = mesh.new_node();
            mesh.set_position(simplex.verts[i], pos[i]);
        }
        simplex.center /= 4;
        for(int i = 0; i < 4; ++i) simplex.localPos[i] = pos[i] - simplex.center;
        AddFace(mesh, simplex.verts[0], simplex.verts[1], simplex.verts[2], simplex.verts[3]);

        _simplices.push_back(simplex);
    }

    mesh.make_map();
    mesh.compute_faces();

    cloud->Destroy();
    _nb.world->GetSelection()->Clear();

    std::cout << "DONE" << std::endl;
}

} // namespace OP