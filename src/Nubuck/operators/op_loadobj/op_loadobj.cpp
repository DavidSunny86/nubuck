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

LoadOBJPanel::LoadOBJPanel(QWidget* parent) : QWidget(parent) {
	_ui.setupUi(this);
	QObject::connect(_ui.btnChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFilename()));
	QObject::connect(_ui.btnLoadScene, SIGNAL(clicked()), this, SLOT(OnLoadScene()));
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

struct TypeA { int val; };
struct TypeB { int val0; bool val1; };

template<typename FTYPE>
class MyGraph : public leda::GRAPH<leda::d3_rat_point, int> { 
	typedef leda::GRAPH<leda::d3_rat_point, int> base_t;

	leda::edge_map<FTYPE> _attribs;
public:
	MyGraph() { 
		_attribs.init(*this);
	}

    ~MyGraph() {
        // ...
	}

    MyGraph& operator=(const MyGraph& other) {
        base_t::operator=(other);
        return *this;
	}
};

template<typename FTYPE>
void DoSomeWork() {
	MyGraph<FTYPE> G[3];
    leda::list<leda::edge> E;
    printf(">>>>>>>> DoSomeWork: BEGIN\n");
    for(int i = 0; i < 3; ++i) {
		leda::maximal_planar_graph(G[i], 10000);
		G[i].make_bidirected(E);
		G[i].make_planar_map();
		G[i].compute_faces();
		printf("|E| = %d\n", E.size());
	}
	G[0].join(G[1]);
	G[0].join(G[2]);
    printf(">>>>>>>> DoSomeWork: END\n");
}

void LoadOBJ::Event_LoadScene(const EV::Event& event) {
	SYS::Timer timer;
    float secsPassed = 0.0f;

	timer.Start();
    DoSomeWork<TypeA>();
	secsPassed = timer.Stop();
	printf("#### TypeA: %fs\n", secsPassed);

	timer.Start();
    DoSomeWork<TypeB>();
	secsPassed = timer.Stop();
	printf("#### TypeB: %fs\n", secsPassed);
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
}

} // namespace OP