#include <QLabel>
#include <QGridLayout>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\polymesh.h>
#include "alg_d2_delaunay_bruteforce.h"
#include "phase0.h"

// ================================================================================
// Panel Impl
// ================================================================================

void D2_Delaunay_BruteForce_Panel::OnToggleParaboloid() {
    OP::SendToOperator(def_ToggleParaboloid.Create(Params_ToggleParaboloid()));
}

void D2_Delaunay_BruteForce_Panel::OnToggleConvexHull() {
    OP::SendToOperator(def_ToggleConvexHull.Create(Params_ToggleConvexHull()));
}

void D2_Delaunay_BruteForce_Panel::OnConvexHullScaleChanged(int) {
    float scale = static_cast<float>(_sldConvexHullScale->value()) / _sldConvexHullScale->maximum();

    Params_SetConvexHullScale args;
    args.scale = scale;
    OP::SendToOperator(def_SetConvexHullScale.Create(args));
}

D2_Delaunay_BruteForce_Panel::D2_Delaunay_BruteForce_Panel() {
    QGridLayout* gridLayout = new QGridLayout();

    _btnToggleParaboloid = new QPushButton("Toggle Paraboloid");
    connect(_btnToggleParaboloid, SIGNAL(clicked()), this, SLOT(OnToggleParaboloid()));

    gridLayout->addWidget(_btnToggleParaboloid, 0, 0, 1, 2);

    _btnToggleConvexHull = new QPushButton("Toggle Convex Hull");
    connect(_btnToggleConvexHull, SIGNAL(clicked()), this, SLOT(OnToggleConvexHull()));

    gridLayout->addWidget(_btnToggleConvexHull, 1, 0, 1, 2);

    _sldConvexHullScale = new QSlider(Qt::Horizontal);
    _sldConvexHullScale->setMaximum(1000);
    _sldConvexHullScale->setMinimum(1);
    _sldConvexHullScale->setValue(1000);
    connect(_sldConvexHullScale, SIGNAL(valueChanged(int)), this, SLOT(OnConvexHullScaleChanged(int)));

    gridLayout->addWidget(new QLabel("z-scale:"), 2, 0, 1, 1);
    gridLayout->addWidget(_sldConvexHullScale, 2, 1, 1, 1);

    QWidget* dummy = new QWidget;
    dummy->setLayout(gridLayout);

    layout()->addWidget(dummy);
}

void D2_Delaunay_BruteForce_Panel::Invoke() {
    _sldConvexHullScale->blockSignals(true);
    _sldConvexHullScale->setValue(1000);
    _sldConvexHullScale->blockSignals(false);
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

IGeometry* CreateCircle(IWorld* world) {
    IGeometry* geom = world->CreateGeometry();

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

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

    geom->SetRenderMode(IGeometry::RenderMode::FACES);
    geom->SetName("circle");
    geom->Hide();

    return geom;
}

} // unnamed namespace

void D2_Delaunay_BruteForce::Event_ToggleParaboloid(const EV::Event&) {
    if(_isParaboloidVisible) {
        _g.paraboloid->Hide();
    }
    else {
        _g.paraboloid->Show();
    }
    _isParaboloidVisible = !_isParaboloidVisible;
}

void D2_Delaunay_BruteForce::Event_ToggleConvexHull(const EV::Event&) {
    if(_isConvexHullVisible) {
        _g.chull->Hide();
    }
    else {
        _g.chull->Show();
    }
    _isConvexHullVisible = !_isConvexHullVisible;
}

void D2_Delaunay_BruteForce::Event_SetConvexHullScale(const EV::Event& event) {
    const Params_SetConvexHullScale& args = def_SetConvexHullScale.GetArgs(event);
    _g.chull->SetScale(M::Vector3(1.0f, 1.0f, args.scale));
    _g.paraboloid->SetScale(M::Vector3(1.0f, 1.0f, args.scale));
}

const char* D2_Delaunay_BruteForce::GetName() const {
    return "Delaunay Triangulation (brute force)";
}

OP::ALG::Phase* D2_Delaunay_BruteForce::Init(const Nubuck& nb) {
    _isParaboloidVisible = false;
    _isConvexHullVisible = false;

    _g.nb = nb;

    std::vector<IGeometry*> geomSel = _g.nb.world->GetSelection()->GetList();
    if(geomSel.empty()) {
        _g.nb.log->printf("ERROR - no input mesh selected.\n");
        return NULL;
    }

    _g.delaunay = geomSel[0];

    _g.circle = CreateCircle(_g.nb.world);

    return new Phase0(_g);
}

D2_Delaunay_BruteForce::D2_Delaunay_BruteForce() {
    AddEventHandler(def_ToggleParaboloid, this, &D2_Delaunay_BruteForce::Event_ToggleParaboloid);
    AddEventHandler(def_ToggleConvexHull, this, &D2_Delaunay_BruteForce::Event_ToggleConvexHull);
    AddEventHandler(def_SetConvexHullScale, this, &D2_Delaunay_BruteForce::Event_SetConvexHullScale);
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new D2_Delaunay_BruteForce_Panel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new D2_Delaunay_BruteForce;
}