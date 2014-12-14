#include <maxint.h>

#include <QObject>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>
#include <LEDA\geo\d3_delaunay.h>

#include <Nubuck\polymesh.h>
#include <UI\block_signals.h>
#include "d3_delaunay.h"
#include "op_delaunay3d.h"

static EV::ConcreteEventDef<EV::Arg<double> > ev_setScale;

namespace OP {

void Delaunay3DPanel::OnScaleChanged(leda::rational value) {
    SendToOperator(ev_setScale, EV::Arg<double>(value.to_double()));
}

Delaunay3DPanel::Delaunay3DPanel(QWidget* parent) : OperatorPanel(parent) {
    _ui.setupUi(this);

    _ui.sbScale->setObjectName("nbw_spinBox"); // important for stylesheet.
    _ui.sbScale->showProgressBar(true);
    _ui.sbScale->setMinimum(0.0);
    _ui.sbScale->setMaximum(1.0);
    _ui.sbScale->setSingleStep(0.025);
    _ui.sbScale->setValue(0.0);

    connect(_ui.sbScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnScaleChanged(leda::rational)));
}

void Delaunay3DPanel::Invoke() {
    UI::BlockSignals block(_ui.sbScale);
    _ui.sbScale->setValue(0);
}

namespace {

M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

leda::d3_rat_point ToRatPoint(const M::Vector3& v) {
    return leda::d3_rat_point(leda::d3_point(v.x, v.y, v.z));
}

} // unnamed namespace

void Delaunay3D::Event_SetScale(const EV::Arg<double>& event) {
    if(_simplices.empty()) return;

    std::cout << "Delaunay3D: scaling simplices ... ";
    for(unsigned j = 0; j < _simplices.size(); ++j) {
        Simplex& simplex = _simplices[j];

        leda::rational scale = 1 + 5 * event.value;
        leda::rat_vector center = scale * simplex.center;

        leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);
        for(int i = 0; i < 4; ++i) {
            // NOTE: carrying out this addition with rat_vectors yields NaNs when converted to float.
            // i have absolutely NO IDEA why. maybe corrupted leda install?
            mesh.set_position(simplex.verts[i], ToRatPoint(ToVector(simplex.localPos[i]) + ToVector(center)));
        }
    }
    std::cout << "DONE" << std::endl;
}

Delaunay3D::Delaunay3D() {
    AddEventHandler(ev_setScale, this, &Delaunay3D::Event_SetScale);
}

void Delaunay3D::Register(Invoker& invoker) {
    QAction* action = nubuck().object_menu()->addAction("Delaunay 3D");
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

bool Delaunay3D::Invoke() {
    typedef leda::d3_rat_point point3_t;

    _simplices.clear();

    std::vector<nb::geometry> geomSel = nubuck().selected_geometry();
    if(geomSel.empty()) {
        nubuck().log_printf("no geometry selected.\n");
        return false;
    }

    nubuck().set_operator_name("Delaunay 3D");

    nb::geometry cloud = geomSel.front();

    leda::nb::RatPolyMesh& cloudMesh = nubuck().poly_mesh(cloud);
    leda::list<point3_t> L;
    leda::node v;
    forall_nodes(v, cloudMesh) L.push_back(cloudMesh.position_of(v));

    leda::list<leda::fork::simplex_t> S;

    std::cout << "Delaunay3D: calling D3_DELAUNAY ... " << std::flush;
    // NOTE: include d3_delaunay.cpp when building LEDA
    leda::fork::D3_DELAUNAY(L, S);
    std::cout << "DONE" << std::endl;

    geom = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(geom, Nubuck::RenderMode::NODES | Nubuck::RenderMode::EDGES | Nubuck::RenderMode::FACES);
    nubuck().set_geometry_position(geom, nubuck().geometry_position(cloud));
    leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);

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

    nubuck().destroy_geometry(cloud);
    nubuck().clear_selection();

    std::cout << "DONE" << std::endl;

    return true;
}

} // namespace OP