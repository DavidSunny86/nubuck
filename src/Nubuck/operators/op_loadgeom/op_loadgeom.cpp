#include <QObject>
#include <QMenu>
#include <QFileDialog>

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_loadgeom.h"

static EV::ConcreteEventDef<EV::Arg<std::string*> > ev_load;

namespace OP {

void LoadGeomPanel::OnChooseFilename() {
	QString filename = QFileDialog::getOpenFileName(
        GetWidget(),
        "Choose a .obj file",
		QDir::currentPath(),
        tr("Geometry (*.geom)"));
	if(!filename.isNull()) {
		_leFilename->setText(filename);

        EV::Arg<std::string*> event(new std::string(filename.toStdString()));
		SendToOperator(ev_load.Tag(event));
	}
}

LoadGeomPanel::LoadGeomPanel() {
    QFormLayout* layout = new QFormLayout;

    _leFilename = new QLineEdit("[...]");
    _leFilename->setReadOnly(true);
    layout->addRow(new QLabel("Filename"), _leFilename);

    QPushButton* btnChooseFilename = new QPushButton("Choose Filename");
    QObject::connect(btnChooseFilename, SIGNAL(clicked()), this, SLOT(OnChooseFilename()));
    layout->addRow(btnChooseFilename);

    GetWidget()->setLayout(layout);
}

void LoadGeom::Event_Load(const EV::Arg<std::string*>& event) {
    const std::string& filename(*event.value);

    NB::Mesh mesh = NB::CreateMesh();
    NB::SetMeshRenderMode(mesh, NB::RM_ALL);
    W::LoadGeometryFromFile(filename, mesh);
    NB::GetGraph(mesh).compute_faces();

    delete event.value;
}

LoadGeom::LoadGeom() {
    AddEventHandler(ev_load, this, &LoadGeom::Event_Load);
}

void LoadGeom::Register(Invoker& invoker) {
    QAction* action = NB::SceneMenu()->addAction("Load .geom file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool LoadGeom::Invoke() {
    NB::SetOperatorName("Load (.geom)");
    return true;
}

} // namespace OP