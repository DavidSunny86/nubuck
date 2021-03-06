#include <maxint.h>
#include <Nubuck\UI\nbw_spinbox.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_gen_randompoints.h"

static EV::ConcreteEventDef<RandomPointsUpdate> ev_randomPointsUpdate;

namespace OP {
namespace GEN {

// RandomPointsPanel ---

void RandomPointsPanel::_OnArgsChanged() {
    RandomPointsUpdate event;
    event.domain = _cbDomain->currentIndex();
    event.size   = _sbSize->value().numerator().to_long();
    event.radius = _sbRadius->value().numerator().to_long();
    event.save   = _cbSave->isChecked();
    g_operators.InvokeAction(ev_randomPointsUpdate.Tag(event));
}

void RandomPointsPanel::OnArgsChanged(leda::rational) {
    _OnArgsChanged();
}

void RandomPointsPanel::OnArgsChanged(int) {
    _OnArgsChanged();
}

void RandomPointsPanel::OnArgsChanged(bool) {
    _OnArgsChanged();
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
    domains.push_back("on hemisphere");
    domains.push_back("on paraboloid");
    domains.push_back("in disc");

    _cbDomain = AddComboBox("Domain", domains);
    QObject::connect(_cbDomain, SIGNAL(currentIndexChanged(int)), this, SLOT(OnArgsChanged(int)));

    _sbRadius = AddSpinBox("radius", 1, 1000);
    _sbRadius->setValue(radius);
    QObject::connect(_sbRadius, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnArgsChanged(leda::rational)));

    _sbSize = AddSpinBox("size", 1, 100000);
    _sbSize->setValue(size);
    QObject::connect(_sbSize, SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnArgsChanged(leda::rational)));

    AddVerticalSpace(20);

    _cbSave = AddCheckBox("write to file");
    _cbSave->setChecked(true);
    QObject::connect(_cbSave, SIGNAL(toggled(bool)), this, SLOT(OnArgsChanged(bool)));
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

    if( Domain::IN_BALL == domain ||
        Domain::IN_HEMISPHERE == domain ||
        Domain::ON_SPHERE == domain ||
        Domain::ON_HEMISPHERE == domain)
    {
        if(Domain::IN_HEMISPHERE == domain || Domain::ON_HEMISPHERE == domain) {
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

    if(Domain::IN_DISC == domain) {
        mesh = _discPrefab;

        leda::node v;
        forall_nodes(v, mesh) {
            leda::rat_vector pos = r * mesh.position_of(v).to_vector();
            mesh.set_position(v, pos);
        }
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
        leda::random_d3_rat_points_on_sphere(size, 100 * radius, L);
        forall_items(it, L) L[it] = leda::rational(1, 100) * L[it].to_vector();
        break;
    case Domain::ON_HEMISPHERE:
        leda::random_d3_rat_points_on_sphere(size, 100 * radius, L);
        forall_items(it, L) {
            leda::rat_vector vec = L[it].to_vector();
            L[it] = leda::rational(1, 100) * leda::rat_vector(vec.xcoord(), vec.ycoord(), leda::abs(vec.zcoord()));
        }
        break;
    case Domain::ON_PARABOLOID:
        leda::random_d3_rat_points_on_paraboloid(size, 100 * radius, L);
        forall_items(it, L) L[it] = leda::rational(1, 50) * L[it].to_vector();
        break;
    case Domain::IN_DISC:
        leda::random_d3_rat_points_in_disc(size, 100 * radius, L);
        forall_items(it, L) L[it] = leda::rational(1, 100) * L[it].to_vector();
        break;
    default:
        assert(0 && "UpdateCloud(): unknown domain");
    };

    forall_items(it, L) {
        mesh.set_position(mesh.new_node(), L[it]);
    }
    NB::SelectMesh(NB::SM_NEW, _cloud);

    W::SaveGeometryToFile("last_cloud.geom", _cloudCopy);
}

void RandomPoints::Event_Update(const RandomPointsUpdate& event) {
    Domain::Enum domain = Domain::Enum(event.domain);

    if(_lastDomain != event.domain || _lastRadius != event.radius || _lastSize != event.size) {
        UpdateHull(domain, event.radius);
        UpdateCloud(Domain::Enum(event.domain), event.size, event.radius);
        _cloudCopy->GetRatPolyMesh() = _cloud->GetRatPolyMesh();
    }

    _lastDomain = domain;
    _lastSize   = event.size;
    _lastRadius = event.radius;
    _lastSave   = event.save;
}

RandomPoints::RandomPoints()
    : _hull(NULL)
    , _cloud(NULL)
    , _lastDomain(Domain::Enum(DEFAULT_DOMAIN))
    , _lastSize(DEFAULT_SIZE)
    , _lastRadius(DEFAULT_RADIUS)
    , _lastSave(DEFAULT_SAVE)
{
    AddEventHandler(ev_randomPointsUpdate, this, &RandomPoints::Event_Update);

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

    L.clear();
    leda::random_d3_rat_points_on_circle(1000, 1000, L);
    forall_items(it, L) {
        L[it] = leda::rational(1, 1000) * L[it].to_vector();
    }
    leda::D3_HULL(L, _discPrefab);
    _discPrefab.compute_faces();
}

void RandomPoints::Register(Invoker& invoker) {
    QMenu* sceneMenu = NB::SceneMenu();
    QAction* action = sceneMenu->addAction("Random Points");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool RandomPoints::Invoke() {
    NB::SetOperatorName("Random Points");

    _hull = NB::CreateMesh();
    NB::SetMeshSolid(_hull, false);
    NB::HideMeshOutline(_hull);
    NB::SetMeshRenderMode(_hull, NB::RM_FACES);
    _cloud = NB::CreateMesh();
    NB::SetMeshName(_cloud, "point cloud");
    NB::SetMeshRenderMode(_cloud, NB::RM_NODES);

    _cloudCopy = NB::CreateMesh();
    NB::SetMeshName(_cloudCopy, "point cloud");
    NB::HideMeshOutline(_cloudCopy);

    UpdateHull(_lastDomain, _lastRadius);
    UpdateCloud(Domain::Enum(_lastDomain), _lastSize, _lastRadius);
    _cloudCopy->GetRatPolyMesh() = _cloud->GetRatPolyMesh();

    NB::SelectMesh(NB::SM_NEW, _cloud);

    return true;
}

void RandomPoints::Finish() {
    _hull->Destroy();

    if(_lastSave) {
        W::SaveGeometryToFile("last_saved_cloud.geom", _cloudCopy);
    }

    _cloudCopy->Destroy();
}

} // namespace GEN
} // namespace OP