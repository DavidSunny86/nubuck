#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_gen_randompoints.h"

namespace OP {
namespace GEN {

// RandomPointsPanel ---

void RandomPointsPanel::OnArgsChanged(int) {
    EV::Params_OP_RandomPoints_Update args;
    args.domain = _cbDomain->currentIndex();
    args.size   = _sbSize->value();
    args.radius = _sbRadius->value();
    g_operators.InvokeAction(EV::def_OP_RandomPoints_Update.Create(args));
}

RandomPointsPanel::RandomPointsPanel(QWidget* parent) : SimplePanel(parent) {
    const int size = RandomPoints::DEFAULT_SIZE;
    const int radius = RandomPoints::DEFAULT_RADIUS;

    AddLabel("Random Points in Cube");

    std::vector<QString> domains;
    domains.push_back("in ball");
    domains.push_back("in cube");
    domains.push_back("in hemisphere");
    domains.push_back("on sphere");
    domains.push_back("on paraboloid");

    _cbDomain = AddComboBox("Domain", domains);
    connect(_cbDomain, SIGNAL(currentIndexChanged(int)), this, SLOT(OnArgsChanged(int)));

    _sbRadius = AddSpinBox("radius", 1, 1000);
    _sbRadius->setValue(radius);
    connect(_sbRadius, SIGNAL(valueChanged(int)), this, SLOT(OnArgsChanged(int)));

    _sbSize = AddSpinBox("size", 1, 10000);
    _sbSize->setValue(size);
    connect(_sbSize, SIGNAL(valueChanged(int)), this, SLOT(OnArgsChanged(int)));
}

// --- RandomPointsPanel

namespace {

leda::d3_rat_point MapOnParaboloid(int radius, const leda::rational& x, const leda::rational& y) {
    // cnf. leda manual, random_points_on_paraboloid
    static const leda::rational f0 = leda::rational(4, 1000);
    static const leda::rational f1 = leda::rational(125, 100);
    return leda::d3_rat_point(x, y, f0 * (x * x + y * y) - f1 * radius);
}

} // unnamed namespace

void RandomPoints::UpdateHull(Domain::Enum domain, int radius) {
    leda::nb::RatPolyMesh& mesh = _hull->GetRatPolyMesh();
    mesh.clear();

    int r = radius;
    leda::list<point3_t> L;

    if(Domain::IN_CUBE == domain) {
        L.push(point3_t(-r, -r, -r));
        L.push(point3_t(-r, -r,  r));
        L.push(point3_t( r, -r,  r));
        L.push(point3_t( r, -r, -r));
        L.push(point3_t(-r,  r, -r));
        L.push(point3_t(-r,  r,  r));
        L.push(point3_t( r,  r,  r));
        L.push(point3_t( r,  r, -r));

        leda::D3_HULL(L, mesh);
        mesh.compute_faces();
    }

    if(Domain::IN_BALL == domain || Domain::ON_SPHERE == domain || Domain::IN_HEMISPHERE == domain) {
        if(Domain::IN_HEMISPHERE == domain) {
            mesh = _hemispherePrefab;
        } else {
            mesh = _spherePrefab;
        }
        leda::node v;
        forall_nodes(v, mesh) {
            leda::rat_vector pos = r * mesh.position_of(v).to_vector();
            mesh.set_position(v, pos);
        }
    }

    if(Domain::ON_PARABOLOID == domain) {
        int r = 100 * radius;
        leda::random_points_on_paraboloid(1000, 100 * radius, L);
        L.push(MapOnParaboloid(r,  r,  r));
        L.push(MapOnParaboloid(r,  r, -r));
        L.push(MapOnParaboloid(r, -r,  r));
        L.push(MapOnParaboloid(r, -r, -r));
        leda::list_item it;
        forall_items(it, L) L[it] = leda::rational(1, 50) * L[it].to_vector();
        leda::D3_HULL(L, mesh);
        mesh.compute_faces();
    }

    leda::nb::set_color(_hull->GetRatPolyMesh(), R::Color::Red);
    ((W::ENT_Geometry*)_hull)->SetTransparency(0.2f); // HACK, incomplete iface
}

void RandomPoints::UpdateCloud(Domain::Enum domain, int size, int radius) {
    leda::nb::RatPolyMesh& mesh = _cloud->GetRatPolyMesh();
    mesh.clear();

    leda::list<point3_t> L;
    leda::list_item it;

    switch(domain) {
    case Domain::IN_BALL:
        leda::random_d3_rat_points_in_ball(size, 100 * radius, L);
        forall_items(it, L) L[it] = leda::rational(1, 100) * L[it].to_vector();
        break;
    case Domain::IN_CUBE:
        leda::random_d3_rat_points_in_cube(size, 100 * radius, L);
        forall_items(it, L) L[it] = leda::rational(1, 100) * L[it].to_vector();
        break;
    case Domain::IN_HEMISPHERE:
        leda::random_d3_rat_points_in_ball(size, 100 * radius, L);
        forall_items(it, L) {
            leda::rat_vector vec = L[it].to_vector();
            L[it] = leda::rational(1, 100) * leda::rat_vector(vec.xcoord(), vec.ycoord(), leda::abs(vec.zcoord()));
        }
        break;
    case Domain::ON_SPHERE:
        leda::random_d3_rat_points_on_sphere(size, radius, L);
        break;
    case Domain::ON_PARABOLOID:
        leda::random_d3_rat_points_on_paraboloid(size, 100 * radius, L);
        forall_items(it, L) L[it] = leda::rational(1, 50) * L[it].to_vector();
        break;
    default:
        assert(0 && "UpdateCloud(): unknown domain");
    };

    forall_items(it, L) {
        mesh.set_position(mesh.new_node(), L[it]);
    }
    _nb.world->GetSelection()->Set(_cloud);
}

void RandomPoints::Event_Update(const EV::Event& event) {
    const EV::Params_OP_RandomPoints_Update& args = EV::def_OP_RandomPoints_Update.GetArgs(event);

    Domain::Enum domain = Domain::Enum(args.domain);

    if(_lastDomain != args.domain || _lastRadius != args.radius)
        UpdateHull(domain, args.radius);
    UpdateCloud(Domain::Enum(args.domain), args.size, args.radius);

    _lastDomain = domain;
    _lastSize   = args.size;
    _lastRadius = args.radius;
}

RandomPoints::RandomPoints()
    : _hull(NULL)
    , _cloud(NULL)
    , _lastDomain(Domain::Enum(DEFAULT_DOMAIN))
    , _lastSize(DEFAULT_SIZE)
    , _lastRadius(DEFAULT_RADIUS)
{
    AddEventHandler(EV::def_OP_RandomPoints_Update, this, &RandomPoints::Event_Update);

    leda::list<point3_t> L;
    leda::random_d3_rat_points_on_sphere(1000, 1, L);

    leda::D3_HULL(L, _spherePrefab);
    _spherePrefab.compute_faces();

    leda::list_item it;
    leda::rational zero(0, 1);
    forall_items(it, L) {
        leda::rat_vector vec = L[it].to_vector();
        L[it] = leda::rat_vector(vec.xcoord(), vec.ycoord(), leda::max(zero, vec.zcoord()));
    }
    leda::D3_HULL(L, _hemispherePrefab);
    _hemispherePrefab.compute_faces();
}

void RandomPoints::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QMenu* sceneMenu = _nb.ui->GetSceneMenu();
    QAction* action = sceneMenu->addAction("Random Points");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void RandomPoints::Invoke() {
    _nb.ui->SetOperatorName("Random Points");

    _hull = _nb.world->CreateGeometry();
    _hull->SetSolid(false);
    _hull->HideOutline();
    _hull->SetRenderMode(IGeometry::RenderMode::FACES);
    _cloud = _nb.world->CreateGeometry();
    _cloud->SetName("point cloud");
    _cloud->SetRenderMode(IGeometry::RenderMode::NODES);

    UpdateHull(_lastDomain, _lastRadius);
    UpdateCloud(Domain::Enum(_lastDomain), _lastSize, _lastRadius);

    _nb.world->GetSelection()->Set(_cloud);
}

void RandomPoints::Finish() {
    _hull->Destroy();
}

} // namespace GEN
} // namespace OP