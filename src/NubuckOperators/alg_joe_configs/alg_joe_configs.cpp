#include <QAction>
#include <QMenu>

#include <LEDA\geo\d3_hull.h>
#include <Nubuck\nubuck.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <Nubuck\operators\standard_algorithm.h>

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
        // total number of configurations. 
        // config1a and config1b are counted as two configurations
        NUM_CONFIGS     = 3,

        // total number of simplices in all configurations
        NUM_SIMPLICES   = 9  
    };

    enum {
        CONF_1A
    };

    Config  _configs[NUM_CONFIGS];
    Simplex _simplices[NUM_SIMPLICES];

    void SetScale(Config& config, const leda::rational scale);
public:
    void Register(const Nubuck& nb, OP::Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

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

void JoeConfigs::Register(const Nubuck& nb, OP::Invoker& invoker) {
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

bool JoeConfigs::Invoke() {
    const int renderAll =
        Nubuck::RenderMode::NODES |
        Nubuck::RenderMode::EDGES |
        Nubuck::RenderMode::FACES;

    Config& config = _configs[CONF_1A];

    config.geom = nubuck().create_geometry();
    nubuck().set_geometry_render_mode(config.geom, renderAll);

    leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(config.geom);

    enum { VA = 0, VB, VC, VD, VE };

    const leda::d3_rat_point positions[] = {
        leda::d3_rat_point(-1,  0, -1), // a
        leda::d3_rat_point( 0,  0,  1), // b
        leda::d3_rat_point( 1,  0, -1), // c
        leda::d3_rat_point( 0,  1,  0), // d
        leda::d3_rat_point( 0, -1,  0)  // e
    };

    int simp_cnt = 0;

    config.simp_first = 0;
    AddSimplex(positions[VA], positions[VB], positions[VC], positions[VD], mesh, _simplices[simp_cnt++]);
    AddSimplex(positions[VA], positions[VB], positions[VC], positions[VE], mesh, _simplices[simp_cnt++]);
    config.simp_last = simp_cnt;

    SetScale(config, 2);

    nb::text text = nubuck().create_text();
    nubuck().set_text_content_scale(text, 'A', 1.0f);
    nubuck().set_text_content(text, "1a");
    M::Vector3 pos = M::Vector3(-0.5f * nubuck().text_content_size(text).x, -1.0f, -1.0f);
    nubuck().set_text_position(text, pos);

    return true;
}

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new OP::ALG::StandardAlgorithmPanel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new JoeConfigs;
}