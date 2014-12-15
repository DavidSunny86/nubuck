#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\UI\nbw_spinbox.h>
#include "alg_d2_delaunay_bruteforce.h"
#include "phase0.h"

EV::ConcreteEventDef<EV::Event> ev_toggleParaboloid;
EV::ConcreteEventDef<EV::Event> ev_toggleConvexHull;
EV::ConcreteEventDef<EV::Arg<float> > ev_setConvexHullScale;

// ================================================================================
// Panel Impl
// ================================================================================

void D2_Delaunay_BruteForce_Panel::OnToggleParaboloid() {
    OP::SendToOperator(ev_toggleParaboloid.Tag());
}

void D2_Delaunay_BruteForce_Panel::OnToggleConvexHull() {
    OP::SendToOperator(ev_toggleConvexHull.Tag());
}

void D2_Delaunay_BruteForce_Panel::OnConvexHullScaleChanged(leda::rational) {
    float scale = _sbConvexHullScale->value().to_float() / _sbConvexHullScale->maximum().to_float();

    OP::SendToOperator(ev_setConvexHullScale.Tag(scale));
}

D2_Delaunay_BruteForce_Panel::D2_Delaunay_BruteForce_Panel() {
    QHBoxLayout* row0 = new QHBoxLayout();
    QHBoxLayout* row1 = new QHBoxLayout();
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    _btnToggleParaboloid = new QCheckBox("Paraboloid");
    connect(_btnToggleParaboloid, SIGNAL(clicked()), this, SLOT(OnToggleParaboloid()));

    row0->addWidget(_btnToggleParaboloid);

    _btnToggleConvexHull = new QCheckBox("Convex Hull");
    connect(_btnToggleConvexHull, SIGNAL(clicked()), this, SLOT(OnToggleConvexHull()));

    row0->addWidget(_btnToggleConvexHull);

    _sbConvexHullScale = new NBW_SpinBox;
    _sbConvexHullScale->setText("z-scale: ");
    _sbConvexHullScale->showProgressBar(true);
    _sbConvexHullScale->setMaximum(1000);
    _sbConvexHullScale->setMinimum(1);
    _sbConvexHullScale->setValue(1000);
    _sbConvexHullScale->setSingleStep(10);
    connect(_sbConvexHullScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnConvexHullScaleChanged(leda::rational)));

    row1->addWidget(_sbConvexHullScale);

    vboxLayout->addLayout(row0);
    vboxLayout->addLayout(row1);

    QWidget* dummy = new QWidget;
    dummy->setLayout(vboxLayout);

    layout()->addWidget(dummy);
}

void D2_Delaunay_BruteForce_Panel::Invoke() {
    _btnToggleParaboloid->setChecked(false);
    _btnToggleConvexHull->setChecked(false);

    _sbConvexHullScale->blockSignals(true);
    _sbConvexHullScale->setValue(1000);
    _sbConvexHullScale->blockSignals(false);
}

// ================================================================================
// Algorithm Impl
// ================================================================================

namespace {

/*
====================
BackFaceEdgeXY
    returns an edge incident to the backface. assumes 'mesh'
    lies in xy plane and faces +z direction
====================
*/
leda::edge BackFaceEdgeXY(const leda::nb::RatPolyMesh& mesh) {
    // find hull vertex
    leda::node hullVert = mesh.first_node();
    leda::node v;
    forall_nodes(v, mesh) {
        if(0 < leda::compare(mesh.position_of(hullVert), mesh.position_of(v))) {
            hullVert = v;
        }
    }

    // find hull edge
    leda::edge hullEdge = mesh.first_out_edge(hullVert);
    do {
        leda::edge succ = mesh.cyclic_adj_pred(hullEdge);

        const leda::d3_rat_point& p0 = mesh.position_of(hullVert);
        const leda::d3_rat_point& p1 = mesh.position_of(leda::target(hullEdge));
        const leda::d3_rat_point& p2 = mesh.position_of(leda::target(succ));

        if(0 <= leda::orientation_xy(p0, p1, p2)) break;

        hullEdge = succ;

    } while(true);

    return mesh.reversal(hullEdge);
}

nb::geometry CreateCircle() {
    nb::geometry geom = nubuck().create_geometry();

    leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);

    leda::list<leda::d3_rat_point> L;
    leda::random_d3_rat_points_on_circle(500, 1000, L);
    leda::list_item it;
    forall_items(it, L) {
        L[it] = leda::rational(1, 1000) * L[it].to_vector();
    }
    leda::D3_HULL(L, mesh);
    mesh.compute_faces();

    leda::edge bfaceEdge = BackFaceEdgeXY(mesh);
    mesh.set_visible(mesh.face_of(bfaceEdge), false);

    leda::nb::set_color(mesh, R::Color::Red);

    nubuck().set_geometry_render_mode(geom, Nubuck::RenderMode::FACES);
    nubuck().set_geometry_name(geom, "circle");
    nubuck().hide_geometry(geom);

    return geom;
}

} // unnamed namespace

void D2_Delaunay_BruteForce::Event_ToggleParaboloid(const EV::Event&) {
    if(_isParaboloidVisible) {
        nubuck().hide_geometry(_g.paraboloid);
    }
    else {
        nubuck().show_geometry(_g.paraboloid);
    }
    _isParaboloidVisible = !_isParaboloidVisible;
}

void D2_Delaunay_BruteForce::Event_ToggleConvexHull(const EV::Event&) {
    if(_isConvexHullVisible) {
        nubuck().hide_geometry(_g.chull);
    }
    else {
        nubuck().show_geometry(_g.chull);
    }
    _isConvexHullVisible = !_isConvexHullVisible;
}

void D2_Delaunay_BruteForce::Event_SetConvexHullScale(const EV::Arg<float>& event) {
    float scale = event.value;
    nubuck().set_geometry_scale(_g.chull, M::Vector3(1.0f, 1.0f, scale));
    nubuck().set_geometry_scale(_g.paraboloid, M::Vector3(1.0f, 1.0f, scale));
}

const char* D2_Delaunay_BruteForce::GetName() const {
    return "Delaunay Triangulation (brute force)";
}

OP::ALG::Phase* D2_Delaunay_BruteForce::Init() {
    _isParaboloidVisible = false;
    _isConvexHullVisible = false;

    std::vector<nb::geometry> geomSel = nubuck().selected_geometry();
    if(geomSel.empty()) {
        nubuck().log_printf("ERROR - no input mesh selected.\n");
        return NULL;
    }

    _g.delaunay = geomSel[0];

    _g.circle = CreateCircle();

    return new Phase0(_g);
}

D2_Delaunay_BruteForce::D2_Delaunay_BruteForce() {
    AddEventHandler(ev_toggleParaboloid, this, &D2_Delaunay_BruteForce::Event_ToggleParaboloid);
    AddEventHandler(ev_toggleConvexHull, this, &D2_Delaunay_BruteForce::Event_ToggleConvexHull);
    AddEventHandler(ev_setConvexHullScale, this, &D2_Delaunay_BruteForce::Event_SetConvexHullScale);
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new D2_Delaunay_BruteForce_Panel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new D2_Delaunay_BruteForce;
}