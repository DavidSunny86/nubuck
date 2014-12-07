#include <QObject>
#include <QMenu>
#include <QFileDialog>

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_loadgeom.h"

namespace OP {

void LoadGeomPanel::OnChooseFilename() {
	QString filename = QFileDialog::getOpenFileName(
        this,
        "Choose a .obj file",
		QDir::currentPath(),
        tr("Geometry (*.geom)"));
	if(!filename.isNull()) {
		_leFilename->setText(filename);

        EV::Params_OP_LoadGeom_Load args = { new std::string(filename.toStdString()) };
		SendToOperator(EV::def_OP_LoadGeom_Load.Create(args));
	}
}

LoadGeomPanel::LoadGeomPanel(QWidget* parent) : OperatorPanel(NULL) {
    QFormLayout* layout = new QFormLayout;

    _leFilename = new QLineEdit("[...]");
    _leFilename->setReadOnly(true);
    layout->addRow(new QLabel("Filename"), _leFilename);

    QPushButton* btnChooseFilename = new QPushButton("Choose Filename");
    connect(btnChooseFilename, SIGNAL(clicked()), this, SLOT(OnChooseFilename()));
    layout->addRow(btnChooseFilename);

    setLayout(layout);
}

void LoadGeom::Event_Load(const EV::Event& event) {
    const EV::Params_OP_LoadGeom_Load& args = EV::def_OP_LoadGeom_Load.GetArgs(event);
    const std::string& filename(*args.filename);

    nb::geometry geom = nubuck().create_geometry();
    const int renderAll =
        Nubuck::RenderMode::NODES |
        Nubuck::RenderMode::EDGES |
        Nubuck::RenderMode::FACES;
    nubuck().set_geometry_render_mode(geom, renderAll);
    W::LoadGeometryFromFile(filename, geom);
    nubuck().poly_mesh(geom).compute_faces();

    delete args.filename;
}

LoadGeom::LoadGeom() {
    AddEventHandler(EV::def_OP_LoadGeom_Load, this, &LoadGeom::Event_Load);
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