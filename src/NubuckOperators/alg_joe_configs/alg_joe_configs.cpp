#include <QAction>
#include <QMenu>
#include <QVBoxLayout>
#include <QSignalMapper>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\nubuck.h>
#include <Nubuck\UI\nbw_spinbox.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>

enum JoeConfig {
    CONF_1A = 0,
    CONF_1B,
    CONF_2,
    CONF_3A,
    CONF_3B,
    CONF_4,
    CONF_5,

    NUM_CONFIGS
};

struct SetConfigScale : EV::Event {
    EVENT_TYPE(SetConfigScale)

    int     config;
    double  scale;
};

static EV::ConcreteEventDef<SetConfigScale> ev_setConfigScale;

class NBW_SpinBox;

class JoeConfigsPanel : public QObject, public OP::OperatorPanel {
    Q_OBJECT
private:
    NBW_SpinBox* _sbScales[NUM_CONFIGS];

    QSignalMapper _sigMap;
private slots:
    void OnConfigScaleChanged(int which);
public:
    JoeConfigsPanel();
};

void JoeConfigsPanel::OnConfigScaleChanged(int which) {
    SetConfigScale event;
    event.config = which;
    event.scale = _sbScales[which]->value().to_double();
    OP::SendToOperator(ev_setConfigScale.Tag(event));
}

JoeConfigsPanel::JoeConfigsPanel() {
    QString texts[] = {
        "1a: ",
        "1b: ",
        "2: ",
        "3a: ",
        "3b: ",
        "4: ",
        "5: "
    };

    QVBoxLayout* layout = new QVBoxLayout;

    for(int i = 0; i < NUM_CONFIGS; ++i) {
        NBW_SpinBox* sb = new NBW_SpinBox;
        sb->setText(texts[i]);
        sb->showProgressBar(true);
        sb->setMinimum(1.0);
        sb->setMaximum(5.0);
        sb->setSingleStep(0.05);

        // NOTE: it's okay for the slot to discard arguments
        QObject::connect(sb, SIGNAL(SigValueChanged(leda::rational)), &_sigMap, SLOT(map()));
        _sigMap.setMapping(sb, i);
        layout->addWidget(sb);

        _sbScales[i] = sb;
    }

    GetWidget()->setLayout(layout);

    QObject::connect(&_sigMap, SIGNAL(mapped(int)), this, SLOT(OnConfigScaleChanged(int)));
}

struct Config {
    nb::geometry    geom;
    int             simp_first;
    int             simp_last; // exclusive
};

struct Simplex {
    leda::node          verts[4];
    leda::rat_vector    localPos[4];
    leda::rat_vector    center;
};

class JoeConfigs : public OP::Operator {
private:
    enum {
        // total number of simplices in all configurations
        NUM_SIMPLICES   = 18
    };

    Config  _configs[NUM_CONFIGS];
    Simplex _simplices[NUM_SIMPLICES];

    void SetScale(Config& config, const leda::rational scale);

    // construction
    int                         _cs_simp_cnt;
    int                         _cs_config;
    const leda::d3_rat_point*   _cs_positions;

    void BeginSimplices(const int config, const leda::d3_rat_point* positions);
    void AddSimplex(int i0, int i1, int i2, int i3);
    void EndSimplices();

    void Event_SetConfigScale(const SetConfigScale& event);
public:
    JoeConfigs();

    void Register(OP::Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

void JoeConfigs::Event_SetConfigScale(const SetConfigScale& event) {
    SetScale(_configs[event.config], event.scale);
}

JoeConfigs::JoeConfigs() : _cs_simp_cnt(0) {
    AddEventHandler(ev_setConfigScale, this, &JoeConfigs::Event_SetConfigScale);
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

void JoeConfigs::SetScale(Config& config, const leda::rational scale) {
    leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(config.geom);

    for(int i = config.simp_first; i < config.simp_last; ++i) {
        Simplex& simplex = _simplices[i];

        leda::rat_vector center = scale * simplex.center;

        for(int i = 0; i < 4; ++i) {
            // NOTE: carrying out this addition with rat_vectors yields NaNs when converted to float.
            // i have absolutely NO IDEA why. maybe corrupted leda install?
            mesh.set_position(simplex.verts[i], ToRatPoint(ToVector(simplex.localPos[i]) + ToVector(center)));
        }
    }
}

void JoeConfigs::Register(OP::Invoker& invoker) {
    QAction* action = nubuck().algorithm_menu()->addAction("Joe Configs");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

template<typename VECTOR>
void BuildTetrahedron(leda::nb::PolyMesh<VECTOR>& G,
        leda::node base0,
        leda::node base1,
        leda::node base2,
        leda::node apex)
{
    // apex
    leda::edge e2   = G.new_edge(apex, base2);
    leda::edge e1   = G.new_edge(apex, base1);
    leda::edge e0   = G.new_edge(apex, base0);

    // base0
    leda::edge e3   = G.new_edge(base0, base2);
    leda::edge e0R  = G.new_edge(base0, apex);
    leda::edge e4   = G.new_edge(base0, base1);

    // base1
    leda::edge e4R  = G.new_edge(base1, base0);
    leda::edge e1R  = G.new_edge(base1, apex);
    leda::edge e5R  = G.new_edge(base1, base2);

    // base2
    leda::edge e5   = G.new_edge(base2, base1);
    leda::edge e2R  = G.new_edge(base2, apex);
    leda::edge e3R  = G.new_edge(base2, base0);

    G.set_reversal(e0, e0R);
    G.set_reversal(e1, e1R);
    G.set_reversal(e2, e2R);
    G.set_reversal(e3, e3R);
    G.set_reversal(e4, e4R);
    G.set_reversal(e5, e5R);

    G.compute_faces();
}

void AddSimplex(
    const leda::d3_rat_point&   p0,
    const leda::d3_rat_point&   p1,
    const leda::d3_rat_point&   p2,
    const leda::d3_rat_point&   p3,
    leda::nb::RatPolyMesh&      mesh,
    Simplex&                    simplex)
{
    for(int i = 0; i < 4; ++i) simplex.verts[i] = mesh.new_node();

    mesh.set_position(simplex.verts[0], p0);
    mesh.set_position(simplex.verts[1], p1);
    mesh.set_position(simplex.verts[2], p2);
    mesh.set_position(simplex.verts[3], p3);

    simplex.center = leda::rat_vector::zero(3);
    leda::rat_vector pos[4];
    for(int i = 0; i < 4; ++i) {
        pos[i] = mesh.position_of(simplex.verts[i]).to_vector();
        simplex.center += pos[i];
    }
    simplex.center /= 4;

    for(int i = 0; i < 4; ++i) simplex.localPos[i] = pos[i] - simplex.center;

    BuildTetrahedron(
        mesh,
        simplex.verts[0],
        simplex.verts[1],
        simplex.verts[2],
        simplex.verts[3]);
}

std::ostream& operator<<(std::ostream& stream, const M::Vector2& v) {
    stream << "vec2[ " << v.x << ", " << v.y << " ]";
    return stream;
}

void JoeConfigs::BeginSimplices(const int config, const leda::d3_rat_point* positions) {
    _cs_config = config;
    _cs_positions = positions;

    _configs[_cs_config].simp_first = _cs_simp_cnt;
}

void JoeConfigs::EndSimplices() {
    _configs[_cs_config].simp_last = _cs_simp_cnt;
}

void JoeConfigs::AddSimplex(int i0, int i1, int i2, int i3) {
    leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(_configs[_cs_config].geom);
    ::AddSimplex(
        _cs_positions[i0],
        _cs_positions[i1],
        _cs_positions[i2],
        _cs_positions[i3],
        mesh,
        _simplices[_cs_simp_cnt++]);
}

bool JoeConfigs::Invoke() {
    nubuck().set_operator_name("Joe's Configurations");

    const int renderAll =
        Nubuck::RenderMode::NODES |
        Nubuck::RenderMode::EDGES |
        Nubuck::RenderMode::FACES;

    enum { VA = 0, VB, VC, VD, VE };

    const leda::d3_rat_point vertPos1[] = {
        leda::d3_rat_point(-1,  0, -1), // a
        leda::d3_rat_point( 0,  0,  1), // b
        leda::d3_rat_point( 1,  0, -1), // c
        leda::d3_rat_point( 0,  1,  0), // d
        leda::d3_rat_point( 0, -1,  0)  // e
    };

    const leda::rational r(1, 2);
    const leda::d3_rat_point vertPos2[] = {
        leda::d3_rat_point( 0,  0,  0), // a
        leda::d3_rat_point( 0,  r,  0), // b
        leda::d3_rat_point( 0, -r,  1), // c
        leda::d3_rat_point(-1, -r, -1), // d
        leda::d3_rat_point( 1, -r, -1)  // e
    };

    const leda::d3_rat_point vertPos3[] = {
        leda::d3_rat_point( 0, -r, -1), // a
        leda::d3_rat_point( 0, -r,  1), // b
        leda::d3_rat_point( 0,  r,  0), // c
        leda::d3_rat_point(-1, -r,  0), // d
        leda::d3_rat_point( 1, -r,  0)  // e
    };

    const leda::d3_rat_point vertPos4[] = {
        leda::d3_rat_point( 0, -r, -1), // a
        leda::d3_rat_point( 0, -r,  1), // b
        leda::d3_rat_point( 0,  r,  0), // c
        leda::d3_rat_point(-1, -r, -1), // d
        leda::d3_rat_point( 1, -r, -1)  // e
    };

    const leda::d3_rat_point vertPos5[] = {
        leda::d3_rat_point( 0, -r,  0), // a
        leda::d3_rat_point( 0, -r,  1), // b
        leda::d3_rat_point( 0,  r,  0), // c
        leda::d3_rat_point(-1, -r, -1), // d
        leda::d3_rat_point( 1, -r, -1)  // e
    };

    const M::Vector3 configPos[] = {
        M::Vector3(-4.0f,  2.0f, 0.0f), // 1a
        M::Vector3( 0.0f,  2.0f, 0.0f), // 1b
        M::Vector3( 4.0f,  2.0f, 0.0f), // 2
        M::Vector3(-6.0f, -2.0f, 0.0f), // 3a
        M::Vector3(-2.0f, -2.0f, 0.0f), // 3b
        M::Vector3( 2.0f, -2.0f, 0.0f), // 4
        M::Vector3( 6.0f, -2.0f, 0.0f)  // 5
    };

    const std::string configNames[] = {
        "1a",
        "1b",
        "2",
        "3a",
        "3b",
        "4",
        "5"
    };

    for(int i = 0; i < NUM_CONFIGS; ++i) {
        Config& config = _configs[i];

        config.geom = nubuck().create_geometry();
        nubuck().set_geometry_name(config.geom, std::string("Config ") + configNames[i]);
        nubuck().set_geometry_render_mode(config.geom, renderAll);
        nubuck().set_geometry_position(config.geom, configPos[i]);

        nb::text text = nubuck().create_text();
        nubuck().set_text_content_scale(text, 'A', 1.0f);
        nubuck().set_text_content(text, configNames[i]);
        M::Vector3 pos = configPos[i] + M::Vector3(-0.5f * nubuck().text_content_size(text).x, -1.0f, -1.0f);
        nubuck().set_text_position(text, pos);
    }

    // configuration 1a
    BeginSimplices(CONF_1A, vertPos1);
    AddSimplex(VA, VB, VC, VD);
    AddSimplex(VA, VB, VC, VE);
    EndSimplices();

    // configuration 1b
    BeginSimplices(CONF_1B, vertPos1);
    AddSimplex(VA, VB, VD, VE);
    AddSimplex(VA, VC, VD, VE);
    AddSimplex(VB, VC, VD, VE);
    EndSimplices();

    // configuration 2
    BeginSimplices(CONF_2, vertPos2);
    AddSimplex(VA, VB, VC, VD);
    AddSimplex(VA, VB, VC, VE);
    AddSimplex(VA, VB, VD, VE);
    AddSimplex(VA, VC, VD, VE);
    EndSimplices();

    // configuration 3a
    BeginSimplices(CONF_3A, vertPos3);
    AddSimplex(VA, VB, VC, VD);
    AddSimplex(VA, VB, VC, VE);
    EndSimplices();

    // configuration 3b
    BeginSimplices(CONF_3B, vertPos3);
    AddSimplex(VA, VC, VD, VE);
    AddSimplex(VB, VC, VD, VE);
    EndSimplices();

    // configuration 4
    BeginSimplices(CONF_4, vertPos4);
    AddSimplex(VA, VB, VC, VD);
    AddSimplex(VA, VB, VC, VE);
    EndSimplices();

    // configuration 5
    BeginSimplices(CONF_5, vertPos5);
    AddSimplex(VA, VB, VC, VD);
    AddSimplex(VA, VB, VC, VE);
    AddSimplex(VA, VC, VD, VE);
    EndSimplices();

    return true;
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new JoeConfigsPanel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new JoeConfigs;
}

#include "alg_joe_configs.moc"