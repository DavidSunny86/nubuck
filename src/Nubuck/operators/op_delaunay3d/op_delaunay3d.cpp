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
    SendToOperator(ev_setScale.Tag(value.to_double()));
}

Delaunay3DPanel::Delaunay3DPanel() {
    _ui.setupUi(GetWidget());

    _ui.sbScale->setObjectName("nbw_spinBox"); // important for stylesheet.
    _ui.sbScale->showProgressBar(true);
    _ui.sbScale->setMinimum(0.0);
    _ui.sbScale->setMaximum(1.0);
    _ui.sbScale->setSingleStep(0.025);
    _ui.sbScale->setValue(0.0);

    QObject::connect(_ui.sbScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnScaleChanged(leda::rational)));
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

        leda::nb::RatPolyMesh& graph = NB::GetGraph(_mesh);
        for(int i = 0; i < 4; ++i) {
            // NOTE: carrying out this addition with rat_vectors yields NaNs when converted to float.
            // i have absolutely NO IDEA why. maybe corrupted leda install?
            graph.set_position(simplex.verts[i], ToRatPoint(ToVector(simplex.localPos[i]) + ToVector(center)));
        }
    }
    std::cout << "DONE" << std::endl;
}

Delaunay3D::Delaunay3D() {
    AddEventHandler(ev_setScale, this, &Delaunay3D::Event_SetScale);
}

void Delaunay3D::Register(Invoker& invoker) {
    QAction* action = NB::ObjectMenu()->addAction("Delaunay 3D");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void AddFace(leda::nb::RatPolyMesh& graph, leda::node v0, leda::node v1, leda::node v2, leda::node v3) {
    graph.new_edge(v0, v1);
    graph.new_edge(v0, v3);
    graph.new_edge(v0, v2);

    graph.new_edge(v1, v2);
    graph.new_edge(v1, v3);
    graph.new_edge(v1, v0);

    graph.new_edge(v2, v0);
    graph.new_edge(v2, v3);
    graph.new_edge(v2, v1);

    graph.new_edge(v3, v0);
    graph.new_edge(v3, v1);
    graph.new_edge(v3, v2);
}

bool Delaunay3D::Invoke() {
    typedef leda::d3_rat_point point3_t;

    _simplices.clear();

    if(!NB::FirstSelectedMesh()) {
        NB::LogPrintf("no geometry selected.\n");
        return false;
    }

    NB::SetOperatorName("Delaunay 3D");

    NB::Mesh cloud = NB::FirstSelectedMesh();
    COM_assert(cloud);

    leda::nb::RatPolyMesh& cloudGraph = NB::GetGraph(cloud);
    leda::list<point3_t> L;
    leda::node v;
    forall_nodes(v, cloudGraph) L.push_back(cloudGraph.position_of(v));

    leda::list<leda::fork::simplex_t> S;

    std::cout << "Delaunay3D: calling D3_DELAUNAY ... " << std::flush;
    // NOTE: include d3_delaunay.cpp when building LEDA
    leda::fork::D3_DELAUNAY(L, S);
    std::cout << "DONE" << std::endl;

    _mesh = NB::CreateMesh();
    NB::SetMeshRenderMode(_mesh, NB::RM_ALL);
    NB::SetMeshPosition(_mesh, NB::GetMeshPosition(cloud));
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_mesh);

    std::cout << "Delaunay3D: creating simplex geometries ... " << std::flush;
    leda::list_item it;
    forall_items(it, S) {
        Simplex simplex;

        simplex.center = leda::rat_vector::zero(3);
        leda::rat_vector pos[4];
        for(int i = 0; i < 4; ++i) {
            pos[i] = S[it].verts[i].to_vector();
            simplex.center += pos[i];

            simplex.verts[i] = graph.new_node();
            graph.set_position(simplex.verts[i], pos[i]);
        }
        simplex.center /= 4;
        for(int i = 0; i < 4; ++i) simplex.localPos[i] = pos[i] - simplex.center;
        AddFace(graph, simplex.verts[0], simplex.verts[1], simplex.verts[2], simplex.verts[3]);

        _simplices.push_back(simplex);
    }

    graph.make_map();
    graph.compute_faces();

    NB::DestroyMesh(cloud);
    NB::ClearSelection();

    std::cout << "DONE" << std::endl;

    return true;
}

} // namespace OP