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

    nb::geometry geom = nubuck().create_geometry();
    const int renderAll =
        Nubuck::RenderMode::NODES |
        Nubuck::RenderMode::EDGES |
        Nubuck::RenderMode::FACES;
    nubuck().set_geometry_render_mode(geom, renderAll);
    W::LoadGeometryFromFile(filename, geom);
    nubuck().poly_mesh(geom).compute_faces();

    delete event.value;
}

LoadGeom::LoadGeom() {
    AddEventHandler(ev_load, this, &LoadGeom::Event_Load);
}

void LoadGeom::Register(Invoker& invoker) {
    QAction* action = nubuck().scene_menu()->addAction("Load .geom file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool LoadGeom::Invoke() {
    nubuck().set_operator_name("Load (.geom)");
    return true;
}

} // namespace OP