#include <maxint.h>

#include <QAction>
#include <QMenu>
#include <QComboBox>

#include <QFormLayout>

#include <LEDA\geo\d3_hull.h>

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include "op_gen_platonic_solids.h"

typedef leda::d3_rat_point point_t;

static EV::ConcreteEventDef<EV::Arg<int> > ev_createPlatonicSolid;

namespace OP {
// namespace GEN {

void PlatonicSolidsPanel::OnNameChanged(int idx) {
    OP::SendToOperator(ev_createPlatonicSolid.Tag(idx));
}

PlatonicSolidsPanel::PlatonicSolidsPanel() {
    _names = new QComboBox;
    _names->addItem("tetrahedron");
    _names->addItem("hexahedron (cube)");
    _names->addItem("octahedron");
    _names->addItem("dodecahedron");
    _names->addItem("icosahedron");
    QObject::connect(_names, SIGNAL(currentIndexChanged(int)), this, SLOT(OnNameChanged(int)));

    QFormLayout* layout = new QFormLayout;
    layout->addRow("name", _names);
    GetWidget()->setLayout(layout);
}

void PlatonicSolidsPanel::Invoke() {
    _names->blockSignals(true);
    _names->setCurrentIndex(0);
    _names->blockSignals(false);
}

// vertex coordinates taken from "https://processing.org/discourse/beta/num_1272064104.html"
// (yeah, underscore identifiers are also their idea)

static void CreateTetrahedronVertices(leda::list<point_t>& L) {
    L.push(point_t( 1, 1, 1 ));
    L.push(point_t(-1, -1, 1 ));
    L.push(point_t(-1, 1,-1 ));
    L.push(point_t( 1, -1, -1 ));
}

static void CreateOctahedronVertices(leda::list<point_t>& L) {
    L.push(point_t( 1, 0, 0 ));
    L.push(point_t( 0, 1, 0 ));
    L.push(point_t( 0, 0, 1 ));
    L.push(point_t( -1, 0, 0 ));
    L.push(point_t( 0, -1, 0 ));
    L.push(point_t( 0, 0, -1 ));
}

static void CreateHexahedronVertices(leda::list<point_t>& L) {
    L.push(point_t( 1, 1, 1 ));
    L.push(point_t( -1, 1, 1 ));
    L.push(point_t( -1, -1, 1 ));
    L.push(point_t( 1, -1, 1 ));
    L.push(point_t( 1, 1, -1 ));
    L.push(point_t( -1, 1, -1 ));
    L.push(point_t( -1, -1, -1 ));
    L.push(point_t( 1, -1, -1 ));
}

static void CreateIcosahedronVertices(leda::list<point_t>& L) {
    leda::rational _ = leda::rational(0.525731);
    leda::rational __ = leda::rational(0.850650);
    L.push(point_t(-_, 0, __));
    L.push(point_t(_, 0, __));
    L.push(point_t(-_, 0, -__));
    L.push(point_t(_, 0, -__));
    L.push(point_t(0, __, _));
    L.push(point_t(0, __, -_));
    L.push(point_t(0, -__, _));
    L.push(point_t(0, -__, -_));
    L.push(point_t(__, _, 0));
    L.push(point_t(-__, _, 0));
    L.push(point_t(__, -_, 0));
    L.push(point_t(-__, -_, 0));
}

static void CreateDodecahedronVertices(leda::list<point_t>& L) {
    leda::rational _ = leda::rational(1.618033); //golden mean
    leda::rational __ = leda::rational(0.618033);
    L.push(point_t(0, __, _));
    L.push(point_t(0, __, -_));
    L.push(point_t(0, -__, _));
    L.push(point_t(0, -__, -_));
    L.push(point_t(_, 0, __));
    L.push(point_t(_, 0, -__));
    L.push(point_t(-_, 0, __));
    L.push(point_t(-_, 0, -__));
    L.push(point_t(__, _, 0));
    L.push(point_t(__, -_, 0));
    L.push(point_t(-__, _, 0));
    L.push(point_t(-__, -_, 0));
    L.push(point_t(1, 1, 1));
    L.push(point_t(1, 1, -1));
    L.push(point_t(1, -1, 1));
    L.push(point_t(1, -1, -1));
    L.push(point_t(-1, 1, 1));
    L.push(point_t(-1, 1, -1));
    L.push(point_t(-1, -1, 1));
    L.push(point_t(-1, -1, -1));
}

static M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

static M::Vector3 ComputeFaceNormal(const leda::nb::RatPolyMesh& mesh, leda::edge e) {
    M::Vector3 v0 = ToVector(mesh.position_of(leda::source(e)));
    M::Vector3 v1 = ToVector(mesh.position_of(leda::target(e)));
    M::Vector3 v2 = ToVector(mesh.position_of(leda::target(mesh.face_cycle_succ(e))));
    return M::Normalize(M::Cross(v1 - v0, v2 - v0));
}

static bool AlmostEqual(float lhp, float rhp, float epsilon) {
    return M::Abs(lhp - rhp) < epsilon;
}

void PlatonicSolids::CreateMesh(int type) {
    typedef void (*createFunc_t)(leda::list<point_t>& L);

    createFunc_t createFuncs[] = {
        CreateTetrahedronVertices,
        CreateHexahedronVertices,
        CreateOctahedronVertices,
        CreateDodecahedronVertices,
        CreateIcosahedronVertices
    };

    leda::list<point_t> L;
    createFunc_t func = createFuncs[type];
    func(L);
    leda::nb::RatPolyMesh& graph = NB::GetGraph(_mesh);
    graph.clear();
    leda::D3_HULL(L, graph);

    if(3 == type) {
        // postprocessing of dodecahedron due to precision errors:
        // remove edges shared by nearly-coplanar faces
        leda::edge e;
        leda::list<leda::edge> deathList;
        leda::edge_array<bool> sentencedToDeath(graph, false);
        forall_edges(e, graph) {
            if(sentencedToDeath[e]) continue;
            leda::edge r = graph.reversal(e);
            M::Vector3 n0 = ComputeFaceNormal(graph, e);
            M::Vector3 n1 = ComputeFaceNormal(graph, r);
            if(AlmostEqual(1.0f, M::Dot(n0, n1), 0.1f)) {
                graph.set_color(e, R::Color::Red);
                deathList.push(e);
                sentencedToDeath[e] = true;
                sentencedToDeath[r] = true;
            }
        }
        forall(e, deathList) {
            graph.del_edge(graph.reversal(e));
            graph.del_edge(e);
        }
    }

    graph.compute_faces();
}

void PlatonicSolids::Event_CreatePlatonicSolid(const EV::Arg<int>& event) {
    CreateMesh(event.value);
}

PlatonicSolids::PlatonicSolids() {
    AddEventHandler(ev_createPlatonicSolid, this, &PlatonicSolids::Event_CreatePlatonicSolid);
}

void PlatonicSolids::Register(Invoker& invoker) {
    QAction* action = NB::SceneMenu()->addAction("Platonic Solids");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool PlatonicSolids::Invoke() {
    NB::SetOperatorName("Create Platonic Solid");

    _mesh = NB::CreateMesh();
    NB::SetMeshName(_mesh, "platonic solid");
    NB::SetMeshRenderMode(_mesh, NB::RM_ALL);
    CreateMesh(0 /* tetrahedron */);

    return true;
}

// } // namespace GEN
} // namespace OP
