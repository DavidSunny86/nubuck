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

LoadOBJPanel::LoadOBJPanel() {
	_ui.setupUi(GetWidget());
	QObject::connect(_ui.btnChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFilename()));
}

void LoadOBJPanel::Invoke() {
    _ui.lneFilename->setText("[...]");
}

// --- LoadOBJPanel impl

void LoadOBJ::Event_Load(const EV::Arg<QString*>& event) {
    NB::ClearSelection();
	if(_mesh) NB::DestroyMesh(_mesh);

    _mesh = NB::CreateMesh();
    NB::SetMeshRenderMode(_mesh, NB::RM_ALL);
	leda::nb::RatPolyMesh& graph = NB::GetGraph(_mesh);
    leda::nb::make_from_obj(event.value->toAscii(), graph);

    NB::SelectMesh(NB::SM_NEW, _mesh);

    NB::LogPrintf("loaded object: |V| = %d, |E| = %d, |F| = %d\n",
        graph.number_of_nodes(), graph.number_of_edges(), graph.number_of_faces());

    delete event.value;
}

LoadOBJ::LoadOBJ() : _mesh(NULL) {
	AddEventHandler(ev_load, this, &LoadOBJ::Event_Load);
}

void LoadOBJ::Register(Invoker& invoker) {
    QAction* action = NB::SceneMenu()->addAction("Load .obj file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool LoadOBJ::Invoke() {
    _mesh = NULL;
    return true;
}

} // namespace OP