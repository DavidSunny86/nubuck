#include <QMenu>
#include <QAction>
#include <QVBoxLayout>

#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <Nubuck\UI\nbw_spinbox.h>

static EV::ConcreteEventDef<EV::Arg<double> > ev_setScale;

class JoePlontPanel : public QObject, public OP::OperatorPanel {
    Q_OBJECT
private:
    NBW_SpinBox* _sbScale;
private slots:
    void OnScaleChanged(leda::rational value);
public:
    JoePlontPanel();
};

void JoePlontPanel::OnScaleChanged(leda::rational value) {
    OP::SendToOperator(ev_setScale.Tag(value.to_double()));
}

JoePlontPanel::JoePlontPanel() {
    _sbScale = new NBW_SpinBox;
    _sbScale->setObjectName("nbw_spinBox"); // important for stylesheet.
    _sbScale->setText("explosion scale: ");
    _sbScale->showProgressBar(true);
    _sbScale->setMinimum(0.0);
    _sbScale->setMaximum(1.0);
    _sbScale->setSingleStep(0.025);
    _sbScale->setValue(0.0);

    QVBoxLayout* vboxLayout = new QVBoxLayout;
    vboxLayout->addWidget(_sbScale);
    GetWidget()->setLayout(vboxLayout);

    QObject::connect(_sbScale, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnScaleChanged(leda::rational)));
}

struct Simplex {
    leda::node          verts[4];
    leda::rat_vector    localPos[4];
    leda::rat_vector    center;
};

class JoePlont : public OP::Operator {
private:
    NB::Mesh _mesh;
    std::vector<Simplex>    _simplices;

    void Event_SetScale(const EV::Arg<double>& event);
public:
    JoePlont();

    void Register(OP::Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

JoePlont::JoePlont() {
    AddEventHandler(ev_setScale, this, &JoePlont::Event_SetScale);
}

void JoePlont::Register(OP::Invoker& invoker) {
    QAction* action = NB::AlgorithmMenu()->addAction("Joe PLONT");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

/*
==================================================
BuildTetrahedron
adds face cycles of tetrahedron v0v1v2v3 to G.
also returns edges fc_i, where edge fc_i belongs to
the face cycle of the face that is opposite to v_i
==================================================
*/
static void BuildTetrahedron(
    leda::nb::RatPolyMesh& G,
    leda::node v0,
    leda::node v1,
    leda::node v2,
    leda::node v3,
    leda::edge& fc0,
    leda::edge& fc1,
    leda::edge& fc2,
    leda::edge& fc3)
{
    const bool flip = 0 < leda::orientation(G[v0], G[v1], G[v2], G[v3]);

    if(flip) {
        // this assures that v0v1v2 is a face of the tetrahedron
        std::swap(v2, v3);
    }

    // v3
    leda::edge e2   = G.new_edge(v3, v2);
    leda::edge e1   = G.new_edge(v3, v1);
    leda::edge e0   = G.new_edge(v3, v0);

    // v0
    leda::edge e3   = G.new_edge(v0, v2);
    leda::edge e0R  = G.new_edge(v0, v3);
    leda::edge e4   = G.new_edge(v0, v1);

    // v1
    leda::edge e4R  = G.new_edge(v1, v0);
    leda::edge e1R  = G.new_edge(v1, v3);
    leda::edge e5R  = G.new_edge(v1, v2);

    // v2
    leda::edge e5   = G.new_edge(v2, v1);
    leda::edge e2R  = G.new_edge(v2, v3);
    leda::edge e3R  = G.new_edge(v2, v0);

    G.set_reversal(e0, e0R);
    G.set_reversal(e1, e1R);
    G.set_reversal(e2, e2R);
    G.set_reversal(e3, e3R);
    G.set_reversal(e4, e4R);
    G.set_reversal(e5, e5R);

    fc0 = e1R;
    fc1 = e3;
    fc2 = e0R;
    fc3 = e4;

    if(flip) {
        std::swap(fc2, fc3);
    }
}

static void AddTetrahedron(
    leda::nb::RatPolyMesh& graph,
    const leda::d3_rat_point p0,
    const leda::d3_rat_point p1,
    const leda::d3_rat_point p2,
    const leda::d3_rat_point p3,
    leda::node verts[] /* out */)
{
    const leda::node v0 = graph.new_node();
    const leda::node v1 = graph.new_node();
    const leda::node v2 = graph.new_node();
    const leda::node v3 = graph.new_node();

    graph.set_position(v0, p0);
    graph.set_position(v1, p1);
    graph.set_position(v2, p2);
    graph.set_position(v3, p3);

    leda::edge dontCare = 0;
    BuildTetrahedron(graph, v0, v1, v2, v3,
        dontCare, dontCare, dontCare, dontCare);

    verts[0] = v0;
    verts[1] = v1;
    verts[2] = v2;
    verts[3] = v3;
}

static M::Vector3 ToVector(const leda::d3_rat_point& p) {
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

static leda::d3_rat_point ToRatPoint(const M::Vector3& v) {
    return leda::d3_rat_point(leda::d3_point(v.x, v.y, v.z));
}

void JoePlont::Event_SetScale(const EV::Arg<double>& event) {
    if(_simplices.empty()) return;

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
}

static void ScaleAndCenter(M::Vector3 points[], unsigned numPoints, float scale) {
    M::Vector3 center(0.0f, 0.0f, 0.0f);
    for(unsigned i = 0; i < numPoints; ++i) {
        points[i] *= scale;
        center += points[i];
    }
    center /= numPoints;
    for(unsigned i = 0; i < numPoints; ++i) points[i] -= center;
}

bool JoePlont::Invoke() {
    NB::SetOperatorName("Joe PLONT example");

    _simplices.clear();

    _mesh = NB::CreateMesh();
    NB::SetMeshName(_mesh, "Joe PLONT");
    NB::SetMeshRenderMode(_mesh, NB::RM_ALL);

    leda::nb::RatPolyMesh& graph = NB::GetGraph(_mesh);

    M::Vector3 fpos[] = {
        M::Vector3(0.054f, 0.099f, 0.993f),
        M::Vector3(0.066f, 0.756f, 0.910f),
        M::Vector3(0.076f, 0.578f, 0.408f),
        M::Vector3(0.081f, 0.036f, 0.954f),
        M::Vector3(0.082f, 0.600f, 0.726f),
        M::Vector3(0.085f, 0.327f, 0.731f),
        M::Vector3(0.123f, 0.666f, 0.842f),
        M::Vector3(0.161f, 0.303f, 0.975f)
    };
    const unsigned numVerts = sizeof(fpos) / sizeof(fpos[0]);
    ScaleAndCenter(fpos, numVerts, 10.0f);
    leda::d3_rat_point pos[numVerts];
    for(unsigned i = 0; i < numVerts; ++i) {
        pos[i] = ToRatPoint(fpos[i]);
    }

    struct SimpIndices { int a, b, c, d; } simplices[] = {
        { 1, 2, 3, 5 },
        { 1, 2, 4, 6 },
        { 1, 2, 4, 7 },
        { 1, 2, 5, 6 },
        { 1, 2, 7, 8 },
        { 1, 3, 4, 6 },
        { 1, 3, 5, 6 },
        { 1, 4, 7, 8 },
        { 2, 3, 5, 7 },
        { 2, 4, 5, 6 },
        { 2, 4, 5, 7 },
        { 3, 4, 6, 8 },
        { 3, 5, 6, 7 },
        { 3, 6, 7, 8 },
        { 4, 5, 6, 8 },
        { 4, 5, 7, 8 },
        { 5, 6, 7, 8 }
    };
    const unsigned numSimplices = sizeof(simplices) / sizeof(simplices[0]);

    for(unsigned i = 0; i < numSimplices; ++i) {
        Simplex s;
        SimpIndices& si = simplices[i];
        AddTetrahedron(graph,
            pos[si.a - 1],
            pos[si.b - 1],
            pos[si.c - 1],
            pos[si.d - 1],
            s.verts);
        s.center = leda::rat_vector::zero(3);
        leda::rat_vector pos[4];
        for(int i = 0; i < 4; ++i) {
            pos[i] = graph.position_of(s.verts[i]).to_vector();
            s.center += pos[i];
        }
        s.center /= 4;
        for(int i = 0; i < 4; ++i) s.localPos[i] = pos[i] - s.center;
        _simplices.push_back(s);
    }
    graph.compute_faces();

    return true;
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() { return new JoePlontPanel; }

NUBUCK_OPERATOR OP::Operator* CreateOperator() { return new JoePlont; }

#include "alg_joe_plont.moc"