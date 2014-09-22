#include <maxint.h>

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

	nubuck().clear_selection();
	if(_geom) _geom->Destroy();

    _geom = (W::ENT_Geometry*)nubuck().create_geometry();
	_geom->SetRenderMode(Nubuck::RenderMode::NODES | Nubuck::RenderMode::EDGES | Nubuck::RenderMode::FACES);
	leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();
	mesh.FromObj(args.filename->toAscii());

    nubuck().select_geometry(Nubuck::SELECT_MODE_NEW, _geom);

    nubuck().log_printf("loaded object: |V| = %d, |E| = %d, |F| = %d\n",
        mesh.number_of_nodes(), mesh.number_of_edges(), mesh.number_of_faces());

	delete args.filename;
}

void LoadOBJ::Event_LoadScene(const EV::Event& event) {
    leda::nb::RatPolyMesh mesh0, mesh1;
    mesh0 = mesh1 = preload;

    leda::node v;
    forall_nodes(v, mesh0) mesh0.set_position(v, mesh0.position_of(v).translate(-10, 0, 0));
    forall_nodes(v, mesh1) mesh1.set_position(v, mesh1.position_of(v).translate( 10, 0, 0));

    mesh0.join(mesh1);

    const int renderAll = Nubuck::RenderMode::NODES | Nubuck::RenderMode::EDGES | Nubuck::RenderMode::FACES;
    W::ENT_Geometry* geom = (W::ENT_Geometry*)nubuck().create_geometry();
    geom->SetRenderMode(renderAll);
    geom->GetRatPolyMesh() = mesh0;

    nubuck().select_geometry(Nubuck::SELECT_MODE_NEW, geom);

    nubuck().log_printf("LoadOBJ: loading scene finished\n");
}

LoadOBJ::LoadOBJ() : _geom(NULL) {
	AddEventHandler(EV::def_OP_LoadOBJ_Load, this, &LoadOBJ::Event_Load);
	AddEventHandler(EV::def_OP_LoadOBJ_LoadScene, this, &LoadOBJ::Event_LoadScene);
}

void LoadOBJ::Register(const Nubuck& nb, Invoker& invoker) {
    nubuck().log_printf("LoadOBJ: preloading mesh\n");
    std::string filename = common.BaseDir() + "Meshes\\laurana_hp.obj";
    preload.FromObj(filename.c_str());

    QAction* action = nubuck().scene_menu()->addAction("Load .obj file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool LoadOBJ::Invoke() {
    _geom = NULL;
    return true;
}

} // namespace OP