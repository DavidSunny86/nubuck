#include "op_gen_incube.h"

namespace OP {
namespace GEN {

// InCubePanel ---

void InCubePanel::OnArgsChanged(int) {
    EV::Params_OP_InCube_Update args;
    args.size = _sbSize->value();
    args.radius = _sbRadius->value();
    g_operators.InvokeAction(EV::def_OP_InCube_Update.Create(args));
}

InCubePanel::InCubePanel(QWidget* parent) : SimplePanel(parent) {
    const int size = 5;
    const int radius = 2;

    AddLabel("Random Points in Cube");

    _sbRadius = AddSpinBox("radius", 1, 100);
    _sbRadius->setValue(radius);
    connect(_sbRadius, SIGNAL(valueChanged(int)), this, SLOT(OnArgsChanged(int)));

    _sbSize = AddSpinBox("size", 1, 10000);
    _sbSize->setValue(size);
    connect(_sbSize, SIGNAL(valueChanged(int)), this, SLOT(OnArgsChanged(int)));
}

// --- InCubePanel 

void InCube::UpdateBBox(int radius) {
    leda::nb::RatPolyMesh& mesh = _bbox->GetRatPolyMesh();
    mesh.clear();

    int r = radius;
    leda::list<point3_t> L;
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

void InCube::UpdateCloud(int size, int radius) {
    leda::nb::RatPolyMesh& mesh = _cloud->GetRatPolyMesh();
    mesh.clear();

    leda::list<point3_t> L;
    leda::random_d3_rat_points_in_cube(size, radius, L);

    leda::list_item it;
    forall_items(it, L) {
        mesh.set_position(mesh.new_node(), L[it]);
    }
    _nb.world->GetSelection()->Set(_cloud);
}

void InCube::Update(int size, int radius) {
    UpdateBBox(radius);
    UpdateCloud(size, radius);
}

void InCube::Event_Update(const EV::Event& event) {
    const EV::Params_OP_InCube_Update& args = EV::def_OP_InCube_Update.GetArgs(event);
    Update(args.size, args.radius);
}

InCube::InCube() : _bbox(NULL), _cloud(NULL) { 
    AddEventHandler(EV::def_OP_InCube_Update, this, &InCube::Event_Update);
}

void InCube::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QMenu* sceneMenu = _nb.ui->GetSceneMenu();
    QAction* action = sceneMenu->addAction("Random Points in Cube");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void InCube::Invoke() {
    _nb.ui->SetOperatorName("Random Points in Cube");

    _bbox = _nb.world->CreateGeometry();
    _bbox->SetSolid(false);
    _bbox->HideOutline();
    _bbox->SetRenderMode(IGeometry::RenderMode::EDGES);
    // _bbox->SetShadingMode(IGeometry::ShadingMode::FAST);
    _cloud = _nb.world->CreateGeometry();
    _cloud->SetName("point cloud");
    _cloud->SetRenderMode(IGeometry::RenderMode::NODES);

    Update(5, 2);

    _nb.world->GetSelection()->Set(_cloud);
}

void InCube::Finish() {
    _bbox->Destroy();
}

} // namespace GEN
} // namespace OP