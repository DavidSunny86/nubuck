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

static EV::ConcreteEventDef<EV::Arg<QString*> >     ev_load;
static EV::ConcreteEventDef<EV::Event>              ev_loadScene;

namespace OP {

// LoadOBJPanel impl ---

void LoadOBJPanel::OnChooseFilename() {
	QString filename = QFileDialog::getOpenFileName(
        GetWidget(),
        "Choose a .obj file",
		QDir::currentPath(),
        tr("Models (*.obj)"));
	if(!filename.isNull()) {
		_ui.lneFilename->setText(filename);

        EV::Arg<QString*> event(new QString(filename));
		g_operators.InvokeAction(ev_load.Tag(event));
	}
}

void LoadOBJPanel::OnLoadScene() {
    OP::SendToOperator(ev_loadScene.Tag());
}

LoadOBJPanel::LoadOBJPanel() {
	_ui.setupUi(GetWidget());
	QObject::connect(_ui.btnChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFilename()));
	QObject::connect(_ui.btnLoadScene, SIGNAL(clicked()), this, SLOT(OnLoadScene()));
}

void LoadOBJPanel::Invoke() {
    _ui.lneFilename->setText("[...]");
}

// --- LoadOBJPanel impl

static leda::nb::RatPolyMesh preload;

void LoadOBJ::Event_Load(const EV::Arg<QString*>& event) {
    NB::ClearSelection();
	if(_mesh) NB::DestroyMesh(_mesh);

    _mesh = NB::CreateMesh();
    NB::SetMeshRenderMode(_mesh, NB::RM_ALL);
	leda::nb::RatPolyMesh& graph = NB::GetGraph(_mesh);
    graph.FromObj(event.value->toAscii());

    NB::SelectMesh(NB::SM_NEW, _mesh);

    NB::LogPrintf("loaded object: |V| = %d, |E| = %d, |F| = %d\n",
        graph.number_of_nodes(), graph.number_of_edges(), graph.number_of_faces());

    delete event.value;
}

void LoadOBJ::Event_LoadScene(const EV::Event& event) {
    leda::nb::RatPolyMesh graph0, graph1;
    graph0 = graph1 = preload;

    leda::node v;
    forall_nodes(v, graph0) graph0.set_position(v, graph0.position_of(v).translate(-10, 0, 0));
    forall_nodes(v, graph1) graph1.set_position(v, graph1.position_of(v).translate( 10, 0, 0));

    graph0.join(graph1);

    NB::Mesh mesh = NB::CreateMesh();
    NB::SetMeshRenderMode(mesh, NB::RM_ALL);
    NB::SetGraph(mesh, graph0);

    NB::SelectMesh(NB::SM_NEW, mesh);

    NB::LogPrintf("LoadOBJ: loading scene finished\n");
}

LoadOBJ::LoadOBJ() : _mesh(NULL) {
	AddEventHandler(ev_load, this, &LoadOBJ::Event_Load);
	AddEventHandler(ev_loadScene, this, &LoadOBJ::Event_LoadScene);
}

void LoadOBJ::Register(Invoker& invoker) {
    NB::LogPrintf("LoadOBJ: preloading mesh\n");
    std::string filename = common.BaseDir() + "Meshes\\laurana_hp.obj";
    preload.FromObj(filename.c_str());

    QAction* action = NB::SceneMenu()->addAction("Load .obj file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool LoadOBJ::Invoke() {
    _mesh = NULL;
    return true;
}

} // namespace OP