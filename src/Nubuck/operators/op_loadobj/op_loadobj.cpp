#include <QFileDialog>
#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>

#include <LEDA\graph\graph_gen.h>
#include <LEDA\graph\planar_map.h>

#include <world\entities\ent_geometry\ent_geometry.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include "op_loadobj.h"

namespace OP {

// LoadOBJPanel impl ---

void LoadOBJPanel::OnChooseFilename() {
	QString filename = QFileDialog::getOpenFileName(
        this,
        "Choose a .obj file",
		QDir::currentPath(),
        tr("Models (*.obj)"));
	if(!filename.isNull()) {
		_ui.lneFilename->setText(filename);

		EV::Params_OP_LoadOBJ_Load args = { new QString(filename) };
		g_operators.InvokeAction(EV::def_OP_LoadOBJ_Load.Create(args));
	}
}

void LoadOBJPanel::OnLoadScene() {
	g_operators.InvokeAction(EV::def_OP_LoadOBJ_LoadScene.Create(EV::Params_OP_LoadOBJ_LoadScene()));
}

LoadOBJPanel::LoadOBJPanel(QWidget* parent) : OperatorPanel(parent) {
	_ui.setupUi(this);
	QObject::connect(_ui.btnChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFilename()));
	QObject::connect(_ui.btnLoadScene, SIGNAL(clicked()), this, SLOT(OnLoadScene()));
}

void LoadOBJPanel::Invoke() {
    _ui.lneFilename->setText("[...]");
}

// --- LoadOBJPanel impl

static leda::nb::RatPolyMesh preload;

void LoadOBJ::Event_Load(const EV::Event& event) {
	const EV::Params_OP_LoadOBJ_Load& args = EV::def_OP_LoadOBJ_Load.GetArgs(event);

	_nb.world->GetSelection()->Clear();
	if(_geom) _geom->Destroy();

	_geom = (W::ENT_Geometry*)_nb.world->CreateGeometry();
	_geom->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
	leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();
	mesh.FromObj(args.filename->toAscii());
	_geom->Update();

	_nb.world->GetSelection()->Set(_geom);

	delete args.filename;
}

void LoadOBJ::Event_LoadScene(const EV::Event& event) {
    leda::nb::RatPolyMesh mesh0, mesh1;
    mesh0 = mesh1 = preload;

    leda::node v;
    forall_nodes(v, mesh0) mesh0.set_position(v, mesh0.position_of(v).translate(-10, 0, 0));
    forall_nodes(v, mesh1) mesh1.set_position(v, mesh1.position_of(v).translate( 10, 0, 0));

    mesh0.join(mesh1);

    const int renderAll = IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES;
    W::ENT_Geometry* geom = (W::ENT_Geometry*)_nb.world->CreateGeometry();
    geom->SetRenderMode(renderAll);
    geom->GetRatPolyMesh() = mesh0;
    geom->Update();

    _nb.world->GetSelection()->Set(geom);

    printf("### LoadOBJ: loading scene finished\n");
}

LoadOBJ::LoadOBJ() : _geom(NULL) {
	AddEventHandler(EV::def_OP_LoadOBJ_Load, this, &LoadOBJ::Event_Load);
	AddEventHandler(EV::def_OP_LoadOBJ_LoadScene, this, &LoadOBJ::Event_LoadScene);
}

void LoadOBJ::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    printf("LoadOBJ: preloading mesh\n");
    const char* filename = "C:\\Libraries\\LEDA\\LEDA-6.4\\vs_nubuck\\demo_flipclip0\\laurana_hp.obj";
	preload.FromObj(filename);

    QAction* action = _nb.ui->GetSceneMenu()->addAction("Load .obj file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void LoadOBJ::Invoke() {
    printf("LoadOBJ::Invoke\n");
    _geom = NULL;

    // REMOVEME
    const int renderAll =
		IGeometry::RenderMode::NODES |
		IGeometry::RenderMode::EDGES |
		IGeometry::RenderMode::FACES;
	IGeometry* geom = _nb.world->CreateGeometry();
	geom->SetRenderMode(renderAll);
    geom->HideOutline();
	leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
	leda::edge e0 = mesh.make_triangle(point_t(-1, -1, 0), point_t(1, -1, 0), point_t(1, 1, 0));
	leda::edge e1 = mesh.face_cycle_pred(e0);
    leda::edge e = e0;
	e = mesh.face_cycle_pred(e);
    e = mesh.split(e);
	mesh.set_position(leda::source(e), point_t(-1, 1, 1));
	mesh.split(e0, e1);
	geom->Update();
}

} // namespace OP